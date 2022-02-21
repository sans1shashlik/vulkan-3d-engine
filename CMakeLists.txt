cmake_minimum_required(VERSION 3.21)
project(vulkan_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

find_package(Vulkan REQUIRED)
find_package(glfw3)
find_package(fmt)

add_executable(vulkan_test
        src/main.cpp
        src/Program.cpp
        src/engine/VulkanRenderer.cpp
        src/engine/Camera.cpp
        src/engine/VertexObject.cpp
)

target_link_libraries(vulkan_test Vulkan::Vulkan)
target_link_libraries(vulkan_test Vulkan::Headers)
target_link_libraries(vulkan_test glfw)
target_link_libraries(vulkan_test glslang)
target_link_libraries(vulkan_test SPIRV)
target_link_libraries(vulkan_test fmt::fmt-header-only)
target_link_libraries(vulkan_test ${CMAKE_DL_LIBS})

