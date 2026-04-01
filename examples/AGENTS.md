# AGENTS.md - examples/

Platform/renderer example applications. 25+ standalone apps demonstrating Dear ImGui integration.

## OVERVIEW

Each example is a complete, compilable app pairing a platform (GLFW, SDL2, Win32) with a renderer (OpenGL, Vulkan, DirectX). Best reference: `example_glfw_opengl3/`.

## STRUCTURE

```
examples/
├── example_glfw_opengl3/        # Best portable reference (CMake + Makefile)
├── example_glfw_vulkan/         # Cross-platform Vulkan
├── example_win32_directx11/      # Pure Win32, no SDL/GLFW
├── example_android_opengl3/     # Android (Gradle + CMake)
├── example_null/                # Headless, no GPU (CI/testing)
├── libs/                        # Example-only GLFW/uSynergy (NOT 3rdlibs/)
└── imgui_examples.sln          # Visual Studio solution
```

## WHERE TO LOOK

| Task | Location |
|------|----------|
| Best portable ref | `example_glfw_opengl3/` |
| Pure Win32 | `example_win32_directx11/` |
| Android | `example_android_opengl3/android/` |
| Cross-platform Vulkan | `example_glfw_vulkan/` |
| Headless CI | `example_null/` |

## BUILD

```bash
# CMake (example_android_opengl3)
cd example_android_opengl3 && mkdir build && cd build && cmake ..

# Makefile (per-example)
cd example_glfw_opengl3 && make

# Visual Studio
# Open imgui_examples.sln

# Android
cd example_android_opengl3/android && ./gradlew assembleDebug
```

## CONVENTIONS

- **Naming**: `example_{platform}_{renderer}/main.cpp`
- **Apple**: Uses `.mm` (Objective-C++)
- **Emscripten**: `Makefile.emscripten` variants for WASM
- **libs/**: Example-specific copies, separate from `3rdlibs/`

## ANTI-PATTERNS

- Do NOT modify `libs/` - not the `3rdlibs/` versions
- Do NOT use Makefile for new projects - use CMake like `demo/`
- Do NOT copy example backends - use `backends/` directly
- Do NOT expect all examples on all platforms - macOS has no Vulkan

## BACKEND INTEGRATION

```
example_glfw_opengl3   → backends/imgui_impl_glfw.cpp + imgui_impl_opengl3.cpp
example_win32_directx11 → backends/imgui_impl_win32.cpp + imgui_impl_dx11.cpp
```
