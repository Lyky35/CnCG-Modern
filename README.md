
# CnC Generals Zero Hour — Modern Rewrite

This repository includes source code for Command & Conquer Generals, and its expansion pack Zero Hour, rewritten for modern Windows (10+), x64, and Vulkan.

## What Changed

| Aspect | Original | Rewritten |
|--------|----------|-----------|
| Renderer | Direct3D 8 (fixed-function) | Vulkan 1.3 |
| Architecture | x86 (32-bit) | x64 (64-bit) |
| OS Support | Windows XP-era | Windows 10+ |
| Build System | Visual Studio 6.0 (.dsp/.dsw) | CMake + MSVC 2022 |
| C++ STL | STLport-4.5.3 | MSVC STL (C++17) |
| Audio | Miles Sound System | XAudio2 |
| Input | DirectInput 8 | Raw Input + XInput |
| Video | Bink Video | FFmpeg |
| Copy Protection | SafeDisc | Removed |
| Web Browser | BrowserEngine.DLL | Removed |
| Multiplayer | GameSpy SDK | Stubbed out (single-player) |

## Build Requirements

- Windows 10 or later
- Visual Studio 2022 (17.x) with C++ workload
- CMake 3.20 or later
- Vulkan SDK (for debug validation layers)

## Building

### Using the build script (recommended)

```powershell
.\build-windows.ps1
```

### Using CMake presets

```powershell
cmake --preset windows-x64-msvc
cmake --build --preset windows-x64-msvc
```

### Output

- Executable: `build/windows-x64-msvc/bin/RTS.exe`

## Project Structure

```
CMakeLists.txt              # Top-level build
CMakePresets.json            # Build presets (MSVC, MinGW)
build-windows.ps1            # One-click build script
OBJECTIVES.md                # Detailed phase-by-phase plan
DEPENDENCIES.md              # Dependency resolution status
TEST_PLAN.md                 # Test plan (221 test cases)
Generals/Code/
  Main/                      # WinMain entry point
  GameEngine/                # Game engine core
  GameEngineDevice/          # Device layer (Win32, W3D, Video)
  Libraries/
    VulkanRenderer/          # Vulkan 1.3 renderer
    XAudio2/                 # XAudio2 audio engine
    FFmpegVideo/             # FFmpeg video player
    GameSpy/Stub/            # GameSpy SDK stubs (no-op)
    WWVegas/                 # WW3D2, WWLib, WWMath, etc.
GeneralsMD/Code/             # Zero Hour variant (same structure)
```

## Vulkan Renderer

The Vulkan renderer replaces the legacy Direct3D 8 fixed-function pipeline with Vulkan 1.3:

- **Device**: Instance, physical/logical device, swapchain, synchronization
- **Resources**: Buffer, texture, descriptor set management
- **Shaders**: GLSL vertex/fragment shaders for terrain, objects, sprites
- **Pipelines**: Graphics pipeline state objects (blend, depth, rasterizer)
- **Scene**: Camera matrices, scene graph, 2D orthographic rendering
- **Advanced**: Shadow mapping, terrain rendering, water, particles

See `Libraries/Source/VulkanRenderer/README.md` for details.

## Audio

XAudio2 replaces the Miles Sound System. The `XAudio2Engine` class provides:
- Master voice management
- Sound file playback (WAV)
- Volume control

## Input

Raw Input API replaces DirectInput 8:
- `WM_INPUT` message handling for mouse/keyboard
- Device hot-plug (`WM_INPUT_DEVICE_CHANGE`)
- XInput gamepad support

## Video

FFmpeg replaces Bink Video:
- Modern codec support (H.264, H.265, etc.)
- Seeking and playback control

## Zero Hour Support

The `GeneralsMD/` directory contains the Zero Hour expansion variant with the same x64/Vulkan/modernization changes applied. It builds a separate `RTS.exe`.

## Known Limitations

- Multiplayer is stubbed out (GameSpy SDK not available)
- Web browser functionality removed (BrowserEngine.DLL not available)
- 3ds Max export tool not ported (3ds Max 4 SDK is 32-bit only)

## Contributing

This repository will not be accepting contributions (pull requests, issues, etc). If you wish to create changes to the source code and encourage collaboration, please create a fork of the repository under your GitHub user/organization space.

## Support

This repository is for preservation purposes only and is archived without support.

## License

This repository and its contents are licensed under the GPL v3 license, with additional terms applied. Please see [LICENSE.md](LICENSE.md) for details.
