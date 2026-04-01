# AGENTS.md

## PROJECT: yuchengImGUI

Dear ImGui fork with Windows CMake build support. C++ library outputting vertex buffers for 3D rendering. Self-contained, no external deps beyond optionally GLFW.

**Commit**: 54d4b15 | **Branch**: main

---

## QUICK START

```bash
# Build library (installs to ./install/)
compile.bat      # Windows
./compile.sh    # Linux/macOS

# Build demos (requires library installed first)
ecompile.bat    # Windows
./ecompile.sh   # Linux/macOS
```

---

## DIRECTORY LAYOUT

| Directory | Purpose |
|-----------|---------|
| `./` | Core library - `imgui*.cpp/h`, `imconfig.h` |
| `backends/` | Platform/renderer backends (`imgui_impl_*.cpp/h`) |
| `demo/` | Standalone demos (GLFW+OpenGL/Vulkan) |
| `examples/` | Example apps (25+ backend combos) |
| `misc/` | Optional: freetype, stdlib |
| `docs/` | README, FAQ, CONTRIBUTING |
| `cmake/` | InstallLib.cmake |
| `3rdlibs/` | Bundled: GLFW 3.4, CMake 3.31 |

---

## CODE MAP

### Core Library (Root)
- `imgui.cpp`, `imgui_widgets.cpp`, `imgui_draw.cpp`, etc.
- Entry points: `ImGui::Text()`, `ImGui::Button()`, `ImGui::Begin()`
- Demo: `ImGui::ShowDemoWindow()` in `imgui_demo.cpp`
- Key types: `ImDrawList`, `ImGuiIO`, `ImGuiStyle`

### Configuration
- `imconfig.h` - 140+ compile-time flags (e.g., `IMGUI_DISABLE_OBSOLETE_FUNCTIONS`)

### Build System
- `CMakeLists.txt` - CMake 3.30+ required
- Platform libs: `WIN32=STATIC`, `Unix=SHARED`
- Hardcoded: `--parallel 32`

---

## BACKENDS

Naming convention: `imgui_impl_{platform}_{renderer}.cpp`

20+ backends in `backends/`:
- win32, dx9-dx12, opengl2/3, vulkan, sdl2/3, glfw, metal, wgpu, allegro5, android, glut

---

## ANTI-PATTERNS

- **Do NOT modify `3rdlibs/`** - external code, commit as-is
- **Do NOT remove `imgui.ini` from .gitignore** - runtime config
- **Do NOT add CI to `3rdlibs/glfw-3.4/.github/workflows/`** - third party

---

## CONVENTIONS

| Aspect | Rule |
|--------|------|
| C++ standard | C++11 for library, C++17 for demo |
| Indentation | 4 spaces |
| Style | C-style casts, minimal C++11 features |
| Naming | `snake_case` vars, `PascalCase` functions |
| macOS | Vulkan excluded from build (demo/CMakeLists.txt) |
| No project-level .editorconfig | Only in `3rdlibs/glfw-3.4/` |

---

## BUILD OUTPUTS

```
install/lib/cmake/imgui/    # CMake configs
install/include/imgui/       # Headers
demo/build/opengl/Release/demoGlfwOpenGL.exe
demo/build/vulkan/Release/demoGlfwVulkan.exe
```

---

## TESTING

No formal test suite. `imgui_demo.cpp` IS the functional verification - run `ShowDemoWindow()` to validate.

---

## COMMON TASKS

| Task | Location | Notes |
|------|----------|-------|
| Add renderer backend | `backends/` | Copy `imgui_impl_*.cpp/h` pattern |
| New platform example | `examples/` | Copy existing `main.cpp` pattern |
| Core library bugs | Root `.cpp` files | `imgui.cpp`, `imgui_widgets.cpp`, etc. |
| Config/compile options | `imconfig.h` | 140+ compile-time flags |
| Build issues | `CMakeLists.txt` | Requires CMake 3.30+ |
