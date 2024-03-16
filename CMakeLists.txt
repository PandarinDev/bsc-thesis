cmake_minimum_required(VERSION 3.20)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

project(infinitown)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE EXTERNAL_FILES "external/include/*.h" "external/src/*.c" "external/src/*.cpp")
file(GLOB_RECURSE INFINITOWN_SRC_FILES "src/*.cpp")
file(GLOB_RECURSE INFINITOWN_HEADER_FILES "src/*.h")

find_package(Catch2 CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_subdirectory(tests)

# Look for "glslc" on the PATH in order to compile shaders at compile time
find_program(GLSLC_BIN_PATH glslc)
if (${GLSLC_BIN_PATH} STREQUAL "GLSLC_BIN_PATH-NOTFOUND")
    message(FATAL_ERROR "The binary 'glslc' was not found on PATH. This binary is required to shaders and is part of the official Vulkan SDK.")
endif()

# Define a custom command for each shader binary
file(GLOB_RECURSE INFINITOWN_SHADER_SRC_FILES "assets/shaders/*.vert" "assets/shaders/*.frag")
foreach(SHADER_FILE IN LISTS INFINITOWN_SHADER_SRC_FILES)
    message(STATUS "Compiling ${SHADER_FILE}: glslc ${SHADER_FILE} -o ${SHADER_FILE}.bin")
    list(APPEND INFINITOWN_SHADER_BINARY_FILES "${SHADER_FILE}.bin")
    add_custom_command(
        OUTPUT ${SHADER_FILE}.bin
        COMMAND glslc
        ARGS ${SHADER_FILE} -o ${SHADER_FILE}.bin
        DEPENDS ${SHADER_FILE})
endforeach()

# Add a custom target for each shader binary
add_custom_target(infinitown-shaders DEPENDS ${INFINITOWN_SHADER_BINARY_FILES})

add_executable(infinitown ${INFINITOWN_SRC_FILES} ${INFINITOWN_HEADER_FILES} ${INFINITOWN_SHADER_FILES} ${EXTERNAL_FILES})
target_include_directories(infinitown PRIVATE "include" "external/include")
target_link_libraries(infinitown PRIVATE glfw glm::glm GPUOpen::VulkanMemoryAllocator imgui::imgui)
add_dependencies(infinitown infinitown-shaders)
if(MSVC)
    # TODO: Add back /WX and disable C4458 warning
    target_compile_options(infinitown PRIVATE /W4 /MP)
else()
    target_compile_options(infinitown PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()