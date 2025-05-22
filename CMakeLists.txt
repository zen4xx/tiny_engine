cmake_minimum_required(VERSION 3.14)
project(Tiny_engine)

# c++ standart 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan SDK: ${Vulkan_LIBRARY}")

option(GLFW_BUILD_DOCS "Build documentation" OFF)
option(GLFW_BUILD_TESTS "Build tests" OFF)
option(GLFW_BUILD_EXAMPLES "Build examples" OFF)

find_package(glfw3 REQUIRED)

add_executable(${PROJECT_NAME} main.cpp
    window/window.cpp
    core/core.cpp
    renderer/renderer.cpp
)


include_directories(${PROJECT_SOURCE_DIR}/external/glm)  # Для GLM
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/window
    ${PROJECT_SOURCE_DIR}/physic
    ${PROJECT_SOURCE_DIR}/audio
    ${PROJECT_SOURCE_DIR}/UI

)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Vulkan::Vulkan
    glfw
)

