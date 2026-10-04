# Dependency Resolution Status

## Summary

| Dependency | Category | x64 Available | Action |
|------------|----------|---------------|--------|
| STLport-4.5.3 | C++ STL | No | **REPLACED** — MSVC STL (C++17) |
| Direct3D 8 (`d3d8.lib`, `d3dx8.lib`) | Graphics | Yes | Keep for now — Phase 3 replaces with Vulkan |
| DirectInput 8 (`dinput8.lib`) | Input | Yes | Keep for now — Phase 4 replaces with Raw Input |
| DirectSound (`dsound.lib`) | Audio | Yes | Keep for now — Phase 4 replaces |
| Miles Sound System (`mss32.lib`) | Audio | Yes | Keep for now — Phase 4 replaces |
| Bink Video (`binkw32.lib`) | Video | Yes | Keep for now — Phase 4 replaces |
| GameSpy SDK | Networking | **Uncertain** | **NEEDS REPLACEMENT** — service shut down 2014 |
| 3ds Max 4 SDK | 3D Tool | No | Tools only — not needed for game build |
| BrowserEngine.DLL | Web Browser | **Uncertain** | **NEEDS REPLACEMENT** — proprietary EA DLL |
| ZLib | Compression | Yes | Keep — has x64 builds |
| LZH-Light | Compression | Yes | Keep — has x64 builds |
| Granny SDK | Animation | Yes | Keep — has x64 builds |
| Windows SDK (system libs) | OS | Yes | Keep — all have x64 |

## Immediate Actions

### 1. STLport → MSVC STL ✅ DONE
- Removed STLport-4.5.3 dependency
- Set `CMAKE_CXX_STANDARD 17` in CMakeLists.txt
- MSVC STL is x64-compatible

### 2. GameSpy SDK — Needs Replacement
- GameSpy service shut down in 2014
- SDK availability uncertain for x64
- **Options:**
  - a) Replace with Steamworks SDK (if targeting Steam)
  - b) Replace with custom matchmaking using REST API
  - c) Stub out multiplayer for single-player focus
  - d) Use open-source GameSpy replacement (OpenSpy)

### 3. BrowserEngine.DLL — Needs Replacement
- Proprietary EA web browser component
- Used for Westwood Online login screens
- **Options:**
  - a) Replace with WebView2 (Microsoft Edge-based, x64 native)
  - b) Replace with CEF (Chromium Embedded Framework)
  - c) Remove web browser functionality (single-player focus)

### 4. 3ds Max 4 SDK — Tools Only
- Only needed for Max2W3D export plugin
- Not needed for game runtime or build
- **Action:** Exclude from CMake build, document as separate tool

## Build Instructions

### Windows 10+ with MSVC 2022
```powershell
cmake --preset windows-x64-msvc
cmake --build --preset windows-x64-msvc
```

### Windows 10+ with MinGW-w64 (cross-compile from Linux)
```bash
sudo apt install mingw-w64
cmake --preset windows-x64-mingw
cmake --build --preset windows-x64-mingw
```

### Output
- Executable: `build/windows-x64-msvc/bin/RTS.exe`
- Libraries: `build/windows-x64-msvc/lib/`
