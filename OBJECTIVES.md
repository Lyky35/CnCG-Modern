# Objectives: Vulkan / Windows 10+ / x64 Rewrite

## Current State Assessment

| Aspect | Current | Target |
|--------|---------|--------|
| Renderer | Direct3D 8 (`dx8wrapper.cpp`, fixed-function pipeline) | Vulkan 1.3+ |
| Architecture | x86 only (`/machine:I386`, VS6 `.dsp` projects) | x64 (AMD64) |
| OS Support | Windows XP-era APIs | Windows 10+ |
| Build System | Visual Studio 6.0 (`.dsp`/`.dsw`) | CMake + MSVC 2022+ |
| C++ STL | STLport-4.5.3 | MSVC STL (C++17/20) |
| Audio | Miles Sound System (`mss32.lib`) | XAudio2 or OpenAL Soft |
| Input | DirectInput 8 (`dinput8.lib`) | Raw Input + XInput |
| Video | Bink Video (`binkw32.lib`) | FFmpeg or custom |
| Assembly | x86 inline asm in 10+ files | C++ or x64 intrinsics |

---

## Phase 0: Audit & Infrastructure

### 0.1 — Code Audit
- [ ] Catalog all x86 assembly / inline asm locations (`PerfTimer.h`, `StackDump.cpp`, `W3DBridgeBuffer.cpp`, `BaseType.h`, `dx8wrapper.h`, `wwdebug.h`, `wwprofile.cpp`, `blitblit.h`, `cpudetect.cpp`, `keyboard.cpp`)
- [ ] Identify all pointer-truncation risks (`DWORD` for handles, `int` for pointer diffs, `LONG` for pointer storage)
- [ ] Inventory all external library dependencies and their x64/Vulkan availability
- [ ] Map the full WW3D2 rendering API surface (vertex buffers, index buffers, textures, shaders, render states)
- [ ] Document the W3DDevice → GameEngine interface boundary

### 0.2 — Build System Modernization
- [ ] Create top-level `CMakeLists.txt` with x64 target (`CMAKE_SYSTEM_PROCESSOR=x64`)
- [ ] Migrate all `.dsp` projects to CMake targets (GameEngine, GameEngineDevice, WW3D2, WWLib, WWMath, WWDebug, WWSaveLoad, WWAudio, Compression, RTS)
- [ ] Set C++ standard to C++17 minimum (C++20 preferred)
- [ ] Configure MSVC 2022+ as the only supported compiler
- [ ] Remove `/G6`, `/GX`, `/YX` and other VS6-era flags
- [ ] Set up CI pipeline (GitHub Actions) for x64 build verification

### 0.3 — Dependency Resolution
- [ ] Replace STLport-4.5.3 with MSVC STL
- [ ] Replace `d3d8.lib` / `d3dx8.lib` — no longer needed (Vulkan replaces)
- [ ] Replace `dinput8.lib` with Raw Input / XInput
- [ ] Replace `mss32.lib` (Miles Sound) with XAudio2 or OpenAL Soft
- [ ] Replace `binkw32.lib` with FFmpeg or custom video decoder
- [ ] Update GameSpy SDK or replace with modern networking
- [ ] Verify ZLib and LZH-Light have x64 builds

---

## Phase 1: x64 Port

### 1.1 — Type Safety & Pointer Fixes
- [ ] Replace `DWORD` for pointer storage with `DWORD_PTR` / `uintptr_t`
- [ ] Replace `LONG` for pointer storage with `LONG_PTR`
- [ ] Fix all `int` → pointer and pointer → `int` casts
- [ ] Audit `sizeof(pointer)` assumptions in serialization/save-load
- [ ] Fix `WPARAM`/`LPARAM` usage for 64-bit correctness
- [ ] Replace `GetWindowLong` with `GetWindowLongPtr` (and `SetWindowLong` → `SetWindowLongPtr`)

### 1.2 — Assembly Removal
- [ ] Port `PerfTimer.h` RDTSC asm to `__rdtsc()` intrinsic or `std::chrono`
- [ ] Port `StackDump.cpp` asm to Windows x64 unwinding (`RtlCaptureContext`, `StackWalk64`)
- [ ] Port `W3DBridgeBuffer.cpp` asm to C++/intrinsics
- [ ] Port `BaseType.h` asm to C++/intrinsics
- [ ] Port `dx8wrapper.h` asm to C++/intrinsics
- [ ] Port `wwdebug.h` / `wwprofile.cpp` asm to C++/intrinsics
- [ ] Port `blitblit.h` asm to C++/intrinsics
- [ ] Port `cpudetect.cpp` asm to `__cpuid()` intrinsic
- [ ] Port `keyboard.cpp` asm to C++/intrinsics

### 1.3 — Library x64 Builds
- [ ] Build WWLib, WWMath, WWDebug, WWUtil, WWSaveLoad as x64 static libs
- [ ] Build Compression (ZLib, LZH-Light) as x64
- [ ] Build WWAudio replacement as x64
- [ ] Build GameEngine and GameEngineDevice as x64
- [ ] Link all into x64 RTS.exe

---

## Phase 2: Windows 10+ Compatibility

### 2.1 — Win32 API Modernization
- [ ] Remove SafeDisc / CD copy-protection checks (`CopyProtection.h`, `CdaPfn.h`)
- [ ] Remove `Win32CDManager` CD-ROM dependency
- [ ] Add DPI awareness manifest (`SetProcessDpiAwarenessContext`)
- [ ] Update window class registration for Win10+ (no `CS_OWNDC` issues)
- [ ] Handle fullscreen borderless windowed mode properly on Win10+
- [ ] Remove `vfw32.lib` (Video for Windows) dependency
- [ ] Update `imm32` IME handling for Win10+ if needed

### 2.2 — Window & Message Loop
- [ ] Modernize `WinMain.cpp` entry point for x64
- [ ] Update `Win32GameEngine` for Win10+ window management
- [ ] Handle high-DPI scaling in `Win32OSDisplay`
- [ ] Support windowed, borderless, and exclusive fullscreen modes
- [ ] Handle Alt+Enter, Alt+Tab, and monitor hot-plug correctly

---

## Phase 3: Vulkan Renderer

### 3.1 — Vulkan Device Layer
- [ ] Create `VulkanDevice` class (replaces `dx8wrapper.cpp` / `dx8wrapper.h`)
- [ ] Implement Vulkan instance creation with validation layers (debug) / without (release)
- [ ] Implement physical device selection (discrete GPU preferred, fallback to integrated)
- [ ] Create logical device with graphics + compute + transfer queues
- [ ] Set up swapchain with `VK_KHR_swapchain`, handle resize/minimize
- [ ] Implement frame synchronization (semaphores, fences, `VK_KHR_present_wait`)
- [ ] Create VMA (Vulkan Memory Allocator) or custom allocator for buffer/texture memory

### 3.2 — Resource Management
- [ ] Port `dx8vertexbuffer` → Vulkan vertex buffers (VMA-backed)
- [ ] Port `dx8indexbuffer` → Vulkan index buffers
- [ ] Port `texture.cpp` / `texman` → Vulkan images with optimal tiling
- [ ] Port `surfaceclass.cpp` → Vulkan image views
- [ ] Port `ddsfile.cpp` → Vulkan-compatible texture loading (BCn compressed formats)
- [ ] Implement descriptor set management (replaces fixed-function state)
- [ ] Port `dx8caps.cpp` → Vulkan feature/query system

### 3.3 — Shader System
- [ ] Create GLSL/HLSL shader set replacing fixed-function pipeline:
  - [ ] Terrain shader (heightmap, texture splatting, lightmap)
  - [ ] Object shader (W3D models, skinning, fog)
  - [ ] Particle shader (point sprites, additive blending)
  - [ ] Water shader (reflection, refraction, normal mapping)
  - [ ] Shadow shader (depth rendering, PCF sampling)
  - [ ] Bridge/road shader
  - [ ] Tree/billboard shader
  - [ ] shroud/fog shader
- [ ] Implement shader permutation system for feature combinations
- [ ] Set up SPIR-V compilation pipeline (glslangValidator or DXC)
- [ ] Port `W3DShaderManager` to Vulkan pipeline cache

### 3.4 — Render State & Pipeline
- [ ] Map all D3D8 render states to Vulkan pipeline state objects
- [ ] Implement blend state mapping (alpha, additive, multiplicative)
- [ ] Implement depth/stencil state mapping
- [ ] Implement rasterizer state mapping (cull mode, fill mode, scissor)
- [ ] Port `sortingrenderer.cpp` → Vulkan sorted draw calls
- [ ] Port `seglinerenderer.cpp` → Vulkan line rendering
- [ ] Port `streakRender.cpp` → Vulkan streak rendering
- [ ] Port `bwrender.cpp` / `bw_render.cpp` → Vulkan 2D rendering

### 3.5 — Scene & Camera
- [ ] Port `camera.cpp` → Vulkan view/projection matrices
- [ ] Port `render2d.cpp` → Vulkan 2D orthographic rendering
- [ ] Port `render2dsentence.cpp` → Vulkan text rendering
- [ ] Port `renderobjectrecycler.cpp` → Vulkan render object management
- [ ] Port `W3DView` → Vulkan viewport/swapchain management
- [ ] Port `W3DDisplay` → Vulkan display surface

### 3.6 — Advanced Rendering
- [ ] Port shadow mapping (`W3DShadow`, `W3DProjectedShadow`, `W3DVolumetricShadow`)
- [ ] Port decal system (`decalsys.cpp`, `decalmsh.cpp`)
- [ ] Port dazzle rendering (`dazzle.cpp`)
- [ ] Port terrain rendering (`TerrainVisual`, `HeightMap`, `TileData`)
- [ ] Port bridge rendering (`W3DBridgeBuffer`)
- [ ] Port road rendering (`W3DRoadBuffer`)
- [ ] Port tree rendering (`W3TreeBuffer`)
- [ ] Port water rendering (`W3DWater`, `W3DWaterTracks`)
- [ ] Port shroud rendering (`W3DShroud`)
- [ ] Port particle system (`W3DParticleSys`)
- [ ] Port projected shadows and volumetric shadows

---

## Phase 4: Audio, Input & Video

### 4.1 — Audio
- [ ] Replace Miles Sound System with XAudio2 or OpenAL Soft
- [ ] Port `WWAudio` and `WPAudio` to new backend
- [ ] Maintain API compatibility for `GameSounds`, `AudioEvent`, etc.
- [ ] Support x64 audio playback with low latency

### 4.2 — Input
- [ ] Replace DirectInput 8 with Raw Input API
- [ ] Implement XInput for gamepad support
- [ ] Port `Win32Mouse` to Raw Input mouse
- [ ] Port `keyboard.cpp` to Raw Input keyboard
- [ ] Maintain `Mouse.h` / `IMEManager` API compatibility

### 4.3 — Video
- [ ] Replace Bink Video with FFmpeg or custom decoder
- [ ] Port video playback used in intro/cutscenes
- [ ] Support modern video codecs (H.264/H.265)

---

## Phase 5: Integration & Testing

### 5.1 — Final Linking
- [ ] Link all x64 libraries into final `RTS.exe`
- [ ] Verify no x86 dependencies remain (`dumpbin /headers`)
- [ ] Set up `Run/` directory with required game data files
- [ ] Create installer or portable zip

### 5.2 — Testing
- [ ] Launch on Windows 10 x64 — verify window creation
- [ ] Verify Vulkan renderer initializes and displays main menu
- [ ] Test gameplay rendering (terrain, units, particles, shadows, water)
- [ ] Test windowed / borderless / fullscreen modes
- [ ] Test alt-tab and monitor hot-plug
- [ ] Performance test: maintain 60 FPS at 1080p on mid-range GPU
- [ ] Memory test: verify no leaks over extended play session
- [ ] Save/load game test (pointer serialization correctness)

### 5.3 — Zero Hour (GeneralsMD)
- [ ] Apply all phases to `GeneralsMD/` variant
- [ ] Verify Zero Hour specific content renders correctly
- [ ] Build `RTS.exe` for Zero Hour

---

## Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| WW3D2 fixed-function → Vulkan shader translation is 1:1 impossible | High | High | Write new shaders from scratch matching visual output |
| Pointer truncation bugs subtle and hard to find | High | High | Static analysis + runtime checks + x64 ASAN |
| External libs (Miles, Bink, GameSpy) have no x64 equivalent | Medium | High | Replace with open-source alternatives |
| Save-game format breaks with pointer size change | Medium | Medium | Version the save format, migrate old saves |
| Assembly routines have no direct C++ equivalent | Low | Medium | Profile-guided rewrite, verify output matches |
| Scope is enormous (238 files in WW3D2 alone) | High | High | Phased approach, get x64 D3D8 build first, then Vulkan |

---

## Recommended Execution Order

1. **Phase 0** — Audit + CMake + dependency resolution
2. **Phase 1** — x64 port with existing D3D8 renderer (get a working x64 exe first)
3. **Phase 2** — Windows 10+ compatibility fixes
4. **Phase 3** — Vulkan renderer (largest phase, do incrementally)
5. **Phase 4** — Audio/input/video modernization
6. **Phase 5** — Integration, testing, release

> **Key principle:** Get a working x64 executable with the existing D3D8 renderer on Windows 10 *before* attempting the Vulkan rewrite. This de-risks the port and gives a fallback.
