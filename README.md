# **Minecraft-Clone**

A voxel sandbox engine and game built in **C/C++20** with **OpenGL**, structured into a clean **Engine DLL** and **Game executable**. The goal is to recreate the foundations of Minecraft-style rendering and world logic while keeping the architecture modular and scalable.

This project uses a modern CMake workflow, vendor-managed dependencies, and a layered system that separates engine functionality from game-specific logic.

---

## **Overview**

The project is split into two modules:

### **Engine (DLL)**

Located in `Engine/`.
Provides core low-level systems:

* Window creation and management (GLFW)
* OpenGL context initialization (GLAD + OpenGL 4.3)
* Core application class
* Update and render loop
* Vendor-managed dependencies (glfw, glad, glm, stb, spdlog, yaml-cpp, entt)

### **Game (EXE)**

Located in `Game/`.
Links against the Engine and provides:

* Custom `GameLayer` for high-level game-specific logic
* Asset folder auto-copied at build time
* Entry point (`main`) that launches the Engine’s application

---

## **Project Structure**

```
Minecraft-Clone/
│
├─ bin/                                 # Build artifacts (exe, dll, assets)
│
├─ Engine/                              # Engine DLL
│   ├─ src/
│   │   └─ Engine/
│   │       ├─ Application.cpp/.h
│   │       └─ Window/
│   │           ├─ Window.cpp/.h
│   ├─ vendor/                           # Third-party libraries
│   │   ├─ entt
│   │   ├─ glad
│   │   ├─ glfw
│   │   ├─ glm
│   │   ├─ spdlog
│   │   ├─ stb
│   │   └─ yaml-cpp
│   └─ CMakeLists.txt
│
├─ Game/                                 # Game EXE
│   ├─ Assets/
│   ├─ src/
│   │   ├─ GameLayer.cpp/.h
│   │   └─ EntryPoint.cpp
│   └─ CMakeLists.txt
│
├─ out/                                  # Build system output (CMake presets)
│
├─ CMakeLists.txt                        # Root build script
├─ CMakePresets.json                     # Build configurations
├─ .gitignore
├─ .gitmodules
├─ LICENSE
└─ README.md
```

---

## **Features**

### **Engine**

* GLFW window creation (1080p default)
* OpenGL context setup (4.3 core profile)
* GLAD function loading
* Main application loop
* Safe initialization + shutdown paths
* DLL export for engine symbols

### **Game**

* Simple layer-based architecture
* Game runtime built on engine loop
* Assets auto-copied to binary directory
* Clean separation from engine code

### **Rendering**

Current:

* Basic window creation
* Context handling
* Swap buffers + event polling

Upcoming:

* Vertex buffers and shader system
* Chunk meshing
* Texture atlas
* Player camera
* Block registry and world generation

---

## **Dependencies**

Already included in `Engine/vendor/`:

| Library  | Purpose                     |
| -------- | --------------------------- |
| GLFW     | Window + input              |
| GLAD     | OpenGL function loader      |
| glm      | Math (vec, mat, transforms) |
| spdlog   | Logging                     |
| stb      | Image loading               |
| yaml-cpp | Serialization               |
| entt     | ECS                         |

No external downloads needed.

---

## **Build Instructions**

### **Requirements**

* CMake 3.25+
* Visual Studio 2022 (MSVC)
* Ninja (optional but recommended)
* Windows 10/11

### **Build**

```
git clone --recursive https://github.com/ThatTanishqTak/Minecraft-Clone.git
```

```
cmake --preset x64-debug
cmake --build --preset x64-debug
```

Or manually:

```
mkdir build
cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
ninja
```

Build output will appear in:

```
bin/Windows-Debug-x64/Minecraft-Clone/
```

Containing:

* `Game.exe`
* `Engined.dll`
* Copied `Assets/`

---

## **Roadmap**

### Rendering

* Triangle → cube → voxel chunk rendering
* Greedy meshing
* Face culling
* Camera system
* Wireframe + debug modes

### World

* Infinite terrain system
* Block registry
* Biome-based noise generation
* CPU meshing → GPU instancing options

### Engine

* Input system abstraction
* Event system
* Editor layer (ImGui)
* Scene serialization

---

## **License**

Apache-2.0 license.

---
