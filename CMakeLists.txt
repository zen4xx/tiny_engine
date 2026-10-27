cmake_minimum_required(VERSION 3.14)
project(Tiny_engine)

add_compile_options(-Wall)

# C++ standard 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan SDK: ${Vulkan_LIBRARY}")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) 
option(GLFW_BUILD_DOCS "Build documentation" OFF)
option(GLFW_BUILD_TESTS "Build tests" OFF)
option(GLFW_BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

include(FetchContent)

message("fetching glfw")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

message("fetching glm")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

file(GLOB CORE "core/*.cpp")
file(GLOB WINDOW "window/*.cpp")
file(GLOB RENDERER "renderer/*.cpp")
file(GLOB SCENE "scene/*.cpp")
file(GLOB ERR_HANDLER "error_handler/error_handler.cpp")

set(SOURCE_FILES ${CORE} ${WINDOW} ${RENDERER} ${SCENE} ${ERR_HANDLER})

if(NOT TINY_ENGINE_NO_EXAMPLES)
    list(APPEND SOURCE_FILES example.cpp)
endif()

if(NOT TINY_ENGINE_NO_EXAMPLES)
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
endif()

if (DEFINED TINY_ENGINE_NO_SECONDARY_CMD_BUFFERS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TINY_ENGINE_NO_SECONDARY_CMD_BUFFERS)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
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
)

find_program(GLSLC_EXECUTABLE glslc)
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found! Please install Vulkan SDK.")
endif()

set(DEFAULT_SHADER_DEFINES
    USE_NORMAL_MAP
    TONE_MAPPING
)

set(SHADER_DEFINES "${DEFAULT_SHADER_DEFINES}" CACHE STRING "glsl shaders")

message(STATUS "Shader defines: ${SHADER_DEFINES}")

set(SHADER_DEFINE_FLAGS "")
foreach(DEF IN LISTS SHADER_DEFINES)
    list(APPEND SHADER_DEFINE_FLAGS "-D${DEF}")
endforeach()

set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/renderer/shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tiny_engine_assets/shaders")

file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

file(GLOB SHADER_SOURCES 
    "${SHADER_SOURCE_DIR}/*.vert"
    "${SHADER_SOURCE_DIR}/*.frag"
)

set(SHADER_OUTPUTS "")

foreach(SHADER_SOURCE ${SHADER_SOURCES})
    get_filename_component(SHADER_EXT ${SHADER_SOURCE} EXT)
    
    if(SHADER_EXT STREQUAL ".vert")
        set(SHADER_OUTPUT "${SHADER_OUTPUT_DIR}/vert.spv")
    elseif(SHADER_EXT STREQUAL ".frag")
        set(SHADER_OUTPUT "${SHADER_OUTPUT_DIR}/frag.spv")
    else()
        get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)
        set(SHADER_OUTPUT "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")
    endif()
    
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER_DEFINE_FLAGS} "${SHADER_SOURCE}" -o "${SHADER_OUTPUT}"
        DEPENDS ${SHADER_SOURCE}
        COMMENT "Compiling shader: ${SHADER_SOURCE} -> ${SHADER_OUTPUT}"
    )
    
    list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
endforeach()

list(REMOVE_DUPLICATES SHADER_OUTPUTS)

add_custom_target(CompileShaders ALL DEPENDS ${SHADER_OUTPUTS})
add_dependencies(${PROJECT_NAME} CompileShaders)