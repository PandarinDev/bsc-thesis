file(GLOB_RECURSE INFINITOWN_TEST_SRC_FILES "*.cpp")

add_executable(infinitown-tests ${INFINITOWN_TEST_SRC_FILES}
    "../src/utils/string_utils.cpp"
    "../src/bounding_box.cpp")
target_include_directories(infinitown-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_link_libraries(infinitown-tests PRIVATE Catch2::Catch2WithMain glm::glm)
if(MSVC)
    target_compile_options(infinitown-tests PRIVATE /W4 /WX)
else()
    target_compile_options(infinitown-tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()