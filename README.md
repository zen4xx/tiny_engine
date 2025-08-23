# Tiny Engine 🎮
**A lightweight, cross-platform 3D game engine in C++**


## About
Tiny Engine is **designed for medium-scale and high-scale projects**. It’s written in modern C++ and keeps dependencies minimal so you can easily explore engine internals and customize subsystems. Supported platforms include **Windows**, **Linux**, and **macOS**.

---

## Features

### Graphics
- **Vulkan** graphics subsystem for low-level control and performance  
- **Physically Based Rendering (PBR)** with metallic-roughness workflow  
- **Normal mapping** to simulate surface detail  
- **Multisample anti-aliasing (MSAA)** for smoother edges  

### Scene and Culling
- **Scene graph** and object hierarchy  
- **Frustum culling** to skip rendering off-camera objects  
- **Multithreaded rendering** to leverage multi-core CPUs  

### Assets
- **glTF** model loader (including textures)  
- Texture formats: PNG, JPEG

### Input and Interaction
- **Keyboard and mouse** input processing  

## Screenshots

| ![Demo](screenshots/screenshot1.png) | ![Demo](screenshots/screenshot2.png) |
| --------------------------------------- | --------------------------------------- |

## Download & Build

1. **Clone the repository**  
   ```bash
   git clone https://github.com/zen4xx/tiny_engine.git
   cd tiny_engine
   ```

2. **Build**  
   ```bash
   cmake CMakeLists.txt
   make
   ```

## Download & Build using cmake

 ```cmake
   cmake_minimum_required(VERSION 3.14)
   project(myApp)

   include(FetchContent)

   set(TINY_ENGINE_NO_EXAMPLES ON CACHE BOOL "")

   FetchContent_Declare(
      Tiny_engine
      GIT_REPOSITORY https://github.com/zen4xx/tiny_engine.git
      GIT_TAG        master
   )

   FetchContent_MakeAvailable(Tiny_engine)

   set(CMAKE_DIR "${tiny_engine_SOURCE_DIR}/assets")
   set(TARGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
   file(COPY ${CMAKE_DIR} DESTINATION ${TARGET_DIR}) # for nessasery tiny_engine assets

   add_executable(myApp main.cpp)

   target_include_directories(myApp PRIVATE ${tiny_engine_SOURCE_DIR})

target_link_libraries(myApp PRIVATE Tiny_engine)
 ```

## Wiki: https://github.com/zen4xx/tiny_engine/blob/master/wiki.md