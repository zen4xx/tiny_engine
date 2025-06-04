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

include(FetchContent)

#GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.9
)
message("fetching glfw")
FetchContent_MakeAvailable(glfw)
message("fetching glm")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git 
    GIT_TAG        master  
)
FetchContent_MakeAvailable(VulkanMemoryAllocator)

include_directories(${VulkanMemoryAllocator_SOURCE_DIR}/include)

set(SOURCE_FILES main.cpp)

file(GLOB CORE "core/*.cpp")
list(APPEND SOURCE_FILES ${CORE})

file(GLOB WINDOW "window/*.cpp")
list(APPEND SOURCE_FILES ${WINDOW})

file(GLOB RENDERER "renderer/*.cpp")
list(APPEND SOURCE_FILES ${RENDERER})

file(GLOB SCENE "scene/*.cpp")
list(APPEND SOURCE_FILES ${SCENE})

file(GLOB ERR_HANDLER "error_handler/error_handler.cpp")
list(APPEND SOURCE_FILES ${ERR_HANDLER})

#add_executable(${PROJECT_NAME} main.cpp
#    window/window.cpp
#    core/core.cpp
#    renderer/renderer.cpp
#)

set(SOURCE_FILES ${SOURCE_FILES} ${CORE} ${WINDOW} ${RENDERER} ${SCENE} ${ERR_HANDLER}) 

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

include_directories(${PROJECT_SOURCE_DIR}/external/glm)  # for GLM
include_directories(${VulkanMemoryAllocator_SOURCE_DIR}/include) # for vma

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/window
    ${PROJECT_SOURCE_DIR}/renderer
    ${PROJECT_SOURCE_DIR}/scene
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Vulkan::Vulkan
    glfw
    glm
    VulkanMemoryAllocator
)

