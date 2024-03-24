#include "generator.h"
#include "gfx/geometry.h"

#include <cmath>
#include <stdexcept>
#include <functional>
#include <unordered_map>

namespace inf {

    WorldGenerator::WorldGenerator(RandomGenerator& random_engine, const gfx::Renderer& renderer) :
        random_engine(random_engine), renderer(renderer) {}

    World WorldGenerator::generate_initial() {
        World world;
        auto& district = world.districts.emplace_back(DistrictType::RESIDENTAL);
        std::deque<DistrictBuilding*> to_process;
        place_initial_building(district, to_process);
        populate_district(district, to_process);
        return world;
    }

    void WorldGenerator::populate_district(District& district) {
        auto& buildings = district.get_buildings();
        std::deque<DistrictBuilding*> to_process;
        // If there are no buildings that likely means that everything has been culled (e.g. looking up in freecam)
        if (buildings.empty()) {
            place_initial_building(district, to_process);
        }
        // Only start with buildings that have a missing neighbor
        else {
            for (auto& entry : buildings) {
                auto& building = entry.second;
                if (!building.top || !building.right || !building.bottom || !building.left) {
                    to_process.emplace_back(&building);
                }
            }
        }
        populate_district(district,  to_process);
    }

    wfc::Building WorldGenerator::generate_building() {
        return wfc::BuildingPatterns::get_pattern("house")->instantiate(
            random_engine,
            &renderer.get_logical_device(),
            &renderer.get_memory_allocator());
    }

    void WorldGenerator::place_initial_building(District& district, std::deque<DistrictBuilding*>& to_process) {
        const auto& camera = renderer.get_camera();
        const gfx::Ray camera_ray(camera.get_position(), camera.get_direction());
        const gfx::Plane ground_plane(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), 0.0f);
        const auto maybe_intersection = camera_ray.intersect(ground_plane);
        if (!maybe_intersection) {
            return;
        }
        const auto intersection = glm::floor(camera_ray.point_at(maybe_intersection.value()));

        // Place the initial building around which we'll start the generation
        {
            auto initial_building = generate_building();
            initial_building.set_position(intersection);
            if (auto building = district.add_building(glm::ivec2(0, 0), std::move(initial_building)); building) {
                to_process.emplace_back(building);
            }
        }
    }

    void WorldGenerator::populate_district(District& district, std::deque<DistrictBuilding*>& to_process) {
        auto place_building = [&](const glm::ivec2& position, const std::function<glm::vec3(const wfc::Building&)>& position_calculator) {
            auto building = generate_building();
            building.set_position(position_calculator(building));
            if (auto ptr = district.add_building(position, std::move(building)); ptr) {
                auto& buildings = district.get_buildings();
                std::vector<std::pair<glm::ivec2, std::function<void(DistrictBuilding&)>>> to_update{
                    { { 1, 0 }, [ptr](DistrictBuilding& right_neighbor) {
                        ptr->right = &right_neighbor;
                        right_neighbor.left = ptr;
                    } },
                    { { -1, 0 }, [ptr](DistrictBuilding& left_neighbor) {
                        ptr->left = &left_neighbor;
                        left_neighbor.right = ptr;
                    } },
                    { { 0, 1 }, [ptr](DistrictBuilding& top_neighbor) {
                        ptr->top = &top_neighbor;
                        top_neighbor.bottom = ptr;
                    } },
                    { { 0, -1 }, [ptr](DistrictBuilding& bottom_neighbor) {
                        ptr->bottom = &bottom_neighbor;
                        bottom_neighbor.top = ptr;
                    } }
                };
                for (const auto& entry : to_update) {
                    const auto entry_pos = position + entry.first;
                    if (auto it = buildings.find(entry_pos); it != buildings.cend()) {
                        entry.second(it->second);
                    }
                }
                to_process.emplace_back(ptr);
            }
        };

        static constexpr float gap = 2.0f;
        while (!to_process.empty()) {
            auto entry = to_process.front();
            to_process.pop_front();
            const auto& building = entry->building;
            const auto& position = building.get_position();
            const auto bb = building.get_bounding_box();
            const auto width = bb.width();
            const auto depth = bb.depth();
            if (!entry->left && renderer.is_in_view(bb.get_block_to_the_left())) {
                place_building(entry->position + glm::ivec2(-1, 0), [&position](const wfc::Building& placed) {
                    const auto width = placed.get_bounding_box().width();
                    return glm::floor(position - glm::vec3(width + gap, 0.0f, 0.0f));
                });
            }
            if (!entry->right && renderer.is_in_view(bb.get_block_to_the_right())) {
                place_building(entry->position + glm::ivec2(1, 0), [&position, &width](const wfc::Building&) {
                    return position + glm::vec3(width + gap, 0.0f, 0.0f);
                });
            }
            if (!entry->top && renderer.is_in_view(bb.get_block_above())) {
                place_building(entry->position + glm::ivec2(0, 1), [&position, &depth](const wfc::Building&) {
                    return position - glm::vec3(0.0f, 0.0f, depth + gap);
                });
            }
            if (!entry->bottom && renderer.is_in_view(bb.get_block_below())) {
                place_building(entry->position + glm::ivec2(0, -1), [&position](const wfc::Building& placed) {
                    const auto depth = placed.get_bounding_box().depth();
                    return position + glm::vec3(0.0f, 0.0f, depth + gap);
                });
            }
        }
    }

}