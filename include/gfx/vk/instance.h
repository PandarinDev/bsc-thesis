#pragma once

#include <glad/vulkan.h>

#include <memory>
#include <string_view>

namespace inf::gfx::vk {

    struct Instance {

        static Instance create_instance(std::string_view application_name);

        Instance(std::unique_ptr<VkInstance> instance);
        ~Instance();
        Instance(const Instance&) = delete;
        Instance& operator=(const Instance&) = delete;
        Instance(Instance&&);
        Instance& operator=(Instance&&);

        const VkInstance& get_instance() const;

    private:
        
        std::unique_ptr<VkInstance> instance;

    };

}