Tiny Engine — API Reference

This document describes the methods exposed by the engine interface. It covers purpose, parameters, return values, usage notes and examples.
Table of Contents

    Overview
    Lifecycle
        isWindowOpen
        update
    Scene management
        createScene
        addObject (three overloads)
        updateScene
        drawScene
        setView
        setDrawDistance
    Object manipulation
        moveObject (two overloads)
    Lighting & environment
        setAmbient
        setDirLight
        setDirLightColor
        setPointLight
        setPointLightColor
        setPointLightsCount
    Input & timing
        isKeyPressed
        getDeltaTime
        getFPSCount
    Window access
        getWindow
    Notes, thread-safety & lifetime
    Examples
        Basic render loop
        Loading a glTF model
    Changelog

Overview

This wrapper exposes inline helper methods that forward to an internal renderer and window objects. Methods are simple thin-forwarders and assume the renderer and window are properly initialized before use.
Lifecycle
isWindowOpen

Signature:
```cpp
inline bool isWindowOpen()
```
Returns true while the engine window is still open (i.e., not requested to close).

Return

    bool — true if window is open, false if GLFW signalled window should close.

Usage notes

    Calls glfwWindowShouldClose(m_window->GetWindow()) and returns the negation.
    Poll the event loop while this is true.

update

Signature:
```cpp
inline void update()
```
Polls OS events.

Behavior

    Calls glfwPollEvents() to process pending input and window events.
    Should be called every frame before drawing.

Scene management
createScene

Signature:
```cpp
inline void createScene(const std::string &scene_name)
```
Creates a new named scene in the renderer.

Parameters

    scene_name — unique name for the scene. If a scene with the same name exists, behavior depends on renderer implementation (may overwrite, ignore, or error).

addObject (vertices + indices)

Signature:
```cpp
inline void addObject(const std::string &scene_name, const std::string &obj_name, const std::vector &vertices, const std::vector<uint32_t> &indices, glm::mat4 pos, const std::string &albedo_path="_default", const std::string &mr_path="_default", const std::string &normal_path="_default")
```
Description

    Adds a mesh built from raw vertices and indices to the named scene.

Parameters

    scene_name — target scene.
    obj_name — unique object identifier inside the scene.
    vertices — vertex buffer (positions, normals, UVs, etc.).
    indices — index buffer.
    pos — model matrix (glm::mat4) describing object transform.
    albedo_path — path to albedo (diffuse) texture or "_default" to use engine default.
    mr_path — path to metalness-roughness texture or "_default".
    normal_path — path to normal map or "_default".

Notes

    Copies of the vectors are forwarded; prefer const & to avoid extra allocations (already used).
    Ensure vertex format matches renderer expectations.

addObject (gltf)

Signature:
```cpp
inline void addObject(const std::string &scene_name, const std::string &obj_name, const std::string &gltf_model_path, glm::mat4 pos, const std::string &albedo_path="_default", const std::string &mr_path="_default", const std::string &normal_path="_default")
```
Description

    Loads a glTF model from file and adds it to the named scene.

Parameters

    gltf_model_path — file system path to .gltf.


addObject (Object)

Signature:
```cpp
inline void addObject(const tiny_engine::Object &obj)
```
Description

    Adds an already-constructed Object instance to the renderer.

Parameters

    obj — object containing mesh, material and transform data.

Notes

    Ownership semantics depend on tiny_engine::Object implementation.

updateScene

Signature:
```cpp
inline void updateScene(const std::string& scene_name)
```
Description

    Must be called when objects are added before calling drawScene.

Parameters

    scene_name — target scene to update.

drawScene

Signature:
```cpp
inline void drawScene(const std::string &scene_name)
```
Description

    Renders the named scene to the currently bound window.

Parameters

    scene_name — target scene.

Usage notes

    Call after updateScene if you changed objects that frame.
    Ensure view/projection matrices are set.

setView

Signature:
```cpp
inline void setView(const std::string &scene_name, glm::mat4 view)
```
Description

    Sets camera view (likely combined view-projection or view matrix depending on renderer).

Parameters

    view — camera view matrix.

Notes

    You usually set view each frame to reflect camera motion.

setDrawDistance

Signature:
```cpp
inline void setDrawDistance(const std::string &scene_name, float distance)
```
Description

    Sets maximum draw distance / far plane for frustum culling.

Parameters

    distance — maximum distance in world units (renderer-dependent).

Object manipulation
moveObject (by name)

Signature:
```cpp
inline void moveObject(const std::string &scene_name, const std::string &obj_name, glm::mat4 model)
```
Description

    Updates transform for the named object in the given scene.

Parameters

    model — new model matrix.

Notes

    Call updateScene after moving objects before drawing.

moveObject (by Object)

Signature:
```cpp
inline void moveObject(const tiny_engine::Object &obj, glm::mat4 model)
```
Description

    Move an object reference or instance to a new transform.

Parameters

    obj — object handle/reference owned/managed by renderer.

Lighting & environment

All lighting functions operate per-scene.
setAmbient

Signature:
```cpp
inline void setAmbient(const std::string &scene_name, glm::vec3 ambient)
```
Description

    Set ambient (ambient light color/intensity) for the scene.

Parameters

    ambient — RGB color vector.

setDirLight

Signature:
```cpp
inline void setDirLight(const std::string &scene_name, glm::vec3 dir)
```
Description

    Set directional light direction vector (should be normalized).

Parameters

    dir — direction from which light shines (world-space).

setDirLightColor

Signature:
```cpp
inline void setDirLightColor(const std::string &scene_name, glm::vec3 color)
```
Description

    Set color/intensity for directional light.

setPointLight

Signature:
```cpp
inline void setPointLight(const std::string &scene_name, glm::vec3 pos, uint8_t index)
```
Description

    Place a point light at position for the scene.

Parameters

    pos — world-space position.
    index — point light slot index (0..N-1), must be < count set via setPointLightsCount.

setPointLightColor

Signature:
```cpp
inline void setPointLightColor(const std::string &scene_name, glm::vec3 color, uint8_t index)
```
Description

    Set color for indexed point light.

setPointLightsCount

Signature:
```cpp
inline void setPointLightsCount(const std::string &scene_name, uint8_t count)
```
Description

    Allocate number of point light slots available to the scene.

Notes

    Changing count may reallocate renderer resources; call sparingly.

Input & timing
isKeyPressed

Signature:
```cpp
inline bool isKeyPressed(int key)
```
Description

    Query whether a key is currently pressed.

Parameters

    key — GLFW key code (e.g., GLFW_KEY_W).

Return

    bool — true if pressed.

Notes

    For edge-triggered input, compare previous frame state.

getDeltaTime

Signature:
```cpp
inline float getDeltaTime()
```
Description

    Returns time delta in seconds between last two frames as calculated by renderer.

Return

    float — seconds.

getFPSCount

Signature:
```cpp
inline float getFPSCount()
```
Description

    Returns frames-per-second measurement from renderer.

Return

    float — FPS.

Window access
getWindow

Signature:
```cpp
inline GLFWwindow* getWindow()
```
Description

    Returns raw GLFWwindow pointer managed by the engine.

Notes

    Use with care; the engine retains ownership. Do not destroy the window yourself.
    Use only for GLFW calls not wrapped by the engine.

Notes, thread-safety & lifetime

    These inline methods forward directly to internal renderer/window. Ensure the engine object outlives all use.
    Not thread-safe: calling renderer/window APIs from different threads without synchronization may cause undefined behavior. Protect calls with mutexes or perform all rendering-related calls on a single thread (recommended).
    Many functions accept and copy std::string and std::vector parameters. To avoid allocations, construct objects prior to per-frame calls or modify the API to accept rvalue references / spans.
    Resource loading (gltf, textures) may be synchronous. For responsive apps, load large assets on background threads and only add to renderer on the render thread.

