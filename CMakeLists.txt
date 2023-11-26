cmake_minimum_required(VERSION 3.20)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

project(infinitown)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE EXTERNAL_FILES "external/include/*.h" "external/src/*.c")
file(GLOB_RECURSE INFINITOWN_SRC_FILES "src/*.cpp")
file(GLOB_RECURSE INFINITOWN_HEADER_FILES "src/*.h")

find_package(Catch2 CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_subdirectory(tests)

add_executable(infinitown ${INFINITOWN_SRC_FILES} ${INFINITOWN_HEADER_FILES} ${EXTERNAL_FILES})
target_include_directories(infinitown PRIVATE "include" "external/include")
target_link_libraries(infinitown PRIVATE glfw glm::glm)
if(MSVC)
    target_compile_options(infinitown PRIVATE /W4 /WX)
else()
    target_compile_options(infinitown PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Add Metal extension for Metal surface creation on MacOS
if(APPLE)
    target_compile_definitions(infinitown PRIVATE "-DVK_USE_PLATFORM_METAL_EXT")
endif()