# Test Plan: CnC Generals Zero Hour — Vulkan / x64 Rewrite

| Field | Value |
|-------|-------|
| **Project** | CnC Generals Zero Hour — Vulkan/x64 Rewrite |
| **Target Platform** | Windows 10+ (x64) |
| **Build System** | CMake 3.20+ / MSVC 2022 |
| **Renderer** | Vulkan 1.3+ |
| **Test Plan Version** | 1.0 |
| **Date** | 2026-10-01 |

---

## 1. Build Verification

### 1.1 CMake Configuration

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| BV-01 | Configure with MSVC 2022 (Release) | `cmake --preset windows-x64-msvc` | Configuration completes with no errors |
| BV-02 | Configure with MSVC 2022 (Debug) | `cmake --preset windows-x64-msvc-debug` | Configuration completes with no errors |
| BV-03 | Verify x64 architecture flag | Check `CMakeCache.txt` for `CMAKE_SIZEOF_VOID_P=8` | Value is `8` (64-bit) |
| BV-04 | Verify generator platform | Check `CMakeCache.txt` for `CMAKE_GENERATOR_PLATFORM` | Value is `x64` |
| BV-05 | Verify C++ standard | Check `CMakeCache.txt` for `CMAKE_CXX_STANDARD` | Value is `17` or higher |

### 1.2 Library Builds

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| BV-06 | Build WWMath | `cmake --build build/windows-x64-msvc --target WWMath` | Compiles and archives as `WWMath.lib` (x64) |
| BV-07 | Build WWLib | `cmake --build build/windows-x64-msvc --target WWLib` | Compiles and archives as `WWLib.lib` (x64) |
| BV-08 | Build WWDebug | `cmake --build build/windows-x64-msvc --target WWDebug` | Compiles and archives as `WWDebug.lib` (x64) |
| BV-09 | Build WWSaveLoad | `cmake --build build/windows-x64-msvc --target WWSaveLoad` | Compiles and archives as `WWSaveLoad.lib` (x64) |
| BV-10 | Build WW3D2 | `cmake --build build/windows-x64-msvc --target WW3D2` | Compiles and archives as `WW3D2.lib` (x64) |
| BV-11 | Build VulkanRenderer | `cmake --build build/windows-x64-msvc --target VulkanRenderer` | Compiles and archives as `VulkanRenderer.lib` (x64) |
| BV-12 | Build WWAudio | `cmake --build build/windows-x64-msvc --target WWAudio` | Compiles and archives as `WWAudio.lib` (x64) |
| BV-13 | Build Compression | `cmake --build build/windows-x64-msvc --target Compression` | Compiles and archives as `Compression.lib` (x64) |
| BV-14 | Build GameEngine | `cmake --build build/windows-x64-msvc --target GameEngine` | Compiles and archives as `GameEngine.lib` (x64) |
| BV-15 | Build GameEngineDevice | `cmake --build build/windows-x64-msvc --target GameEngineDevice` | Compiles and archives as `GameEngineDevice.lib` (x64) |

### 1.3 Final Link

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| BV-16 | Build RTS.exe | `cmake --build build/windows-x64-msvc --target RTS` | Links successfully, `RTS.exe` produced in `build/windows-x64-msvc/bin/` |
| BV-17 | Verify machine type | `dumpbin /headers RTS.exe` | Output contains `machine (x64)` — **not** `machine (x86)` |
| BV-18 | Verify no x86 dependencies | `dumpbin /dependents RTS.exe` | No `*.dll` dependencies that are x86-only (check each with `dumpbin /headers`) |
| BV-19 | Verify subsystem | `dumpbin /headers RTS.exe` | Subsystem is `WINDOWS` (GUI), entry point is x64 |
| BV-20 | Verify no STLport dependency | `dumpbin /dependents RTS.exe` | No `stlport*.dll` or `stlport*.lib` references |
| BV-21 | Verify no D3D8 dependency | `dumpbin /dependents RTS.exe` | No `d3d8.dll` or `d3dx8.dll` references |
| BV-22 | Verify no Miles dependency | `dumpbin /dependents RTS.exe` | No `mss32.dll` or `mss*.lib` references |
| BV-23 | Verify no Bink dependency | `dumpbin /dependents RTS.exe` | No `binkw32.dll` references |
| BV-24 | Verify no DirectInput dependency | `dumpbin /dependents RTS.exe` | No `dinput8.dll` references |
| BV-25 | Verify Vulkan loader present | `dumpbin /dependents RTS.exe` | `vulkan-1.dll` is listed as a dependency |

### 1.4 Full Build

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| BV-26 | Full build (Release) | `cmake --build build/windows-x64-msvc` | All targets build with zero errors |
| BV-27 | Full build (Debug) | `cmake --build build/windows-x64-msvc-debug` | All targets build with zero errors |

---

## 2. Launch Test

### 2.1 Basic Launch

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| LT-01 | Launch RTS.exe | Double-click `RTS.exe` or run from command line | Process starts, no immediate crash |
| LT-02 | Window appears | Observe desktop | Game window appears within 10 seconds |
| LT-03 | Window title | Check window title bar | Title reads "Command & Conquer: Generals — Zero Hour" (or appropriate) |
| LT-04 | Process stays running | Wait 30 seconds after launch | Process is still running (check Task Manager) |
| LT-05 | No crash on launch | Check Windows Event Viewer | No application error events for `RTS.exe` |

### 2.2 Legacy Dependency Checks

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| LT-06 | No SafeDisc error | Launch and observe | No "SafeDisc" or "CD/DVD copy protection" error dialog |
| LT-07 | No CD check | Launch with no disc in drive | Game launches normally, no "insert CD" prompt |
| LT-08 | No DirectInput error | Launch and observe | No "DirectInput" initialization error in log or dialog |
| LT-09 | No Miles Sound error | Launch and observe | No "Miles Sound System" initialization error |
| LT-10 | No Bink Video error | Launch and observe | No "Bink Video" initialization error |
| LT-11 | No STLport error | Launch and observe | No STLport runtime error or assertion |

### 2.3 Log File Verification

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| LT-12 | Check debug log | Open `debug.log` or equivalent | Log shows successful initialization of all subsystems |
| LT-13 | Check for x64 indicators | Search log for architecture markers | Log confirms x64 build (e.g., pointer size, module addresses > 4GB) |
| LT-14 | Check Vulkan initialization | Search log for Vulkan messages | Log shows Vulkan instance, device, and swapchain creation |

---

## 3. Vulkan Renderer Test

### 3.1 Device Initialization

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| VR-01 | Vulkan instance creation | Launch game (debug build with validation layers) | Instance created, no validation errors |
| VR-02 | Physical device selection | Check log for selected GPU | Discrete GPU preferred; falls back to integrated if needed |
| VR-03 | Logical device creation | Check log | Logical device created with graphics, compute, and transfer queues |
| VR-04 | Swapchain creation | Check log | Swapchain created with correct format (B8G8R8A8_UNORM or R8G8B8A8_UNORM) |
| VR-05 | Swapchain image count | Check log | At least 2 images (double buffering), typically 3 (triple buffering) |
| VR-06 | VMA initialization | Check log | Vulkan Memory Allocator initialized successfully |

### 3.2 Validation Layer Tests (Debug Build Only)

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| VR-07 | No validation errors | Launch debug build, play for 5 minutes | Zero Vulkan validation layer errors in debug output |
| VR-08 | No validation warnings | Launch debug build, play for 5 minutes | Zero (or only acceptable/known) validation warnings |
| VR-09 | No VUID violations | Check debug output | No `VUID-*` violations reported |
| VR-10 | No descriptor leaks | Play for 10 minutes, check memory | Descriptor sets are properly freed |
| VR-11 | No buffer leaks | Play for 10 minutes, check VMA stats | No unreleased VMA allocations |

### 3.3 Main Menu Rendering

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| VR-12 | Main menu displays | Launch game, wait for menu | Main menu renders correctly with all UI elements visible |
| VR-13 | Menu background | Observe menu background | Background renders correctly (no black screen, no corruption) |
| VR-14 | Menu text rendering | Observe all text in menu | All text is legible, no missing glyphs |
| VR-15 | Menu button rendering | Hover over buttons | Buttons show hover state correctly |
| VR-16 | Menu animations | Observe menu for 30 seconds | Animations play smoothly, no stuttering |
| VR-17 | Frame rate in menu | Measure FPS (in-game counter or external tool) | Stable 60 FPS (or vsync-capped rate) |

### 3.4 Swapchain & Presentation

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| VR-18 | Present mode | Check log | Present mode is `VK_PRESENT_MODE_FIFO_KHR` (vsync) or `MAILBOX` (triple buffering) |
| VR-19 | Swapchain resize | Resize window (windowed mode) | Swapchain recreates correctly, rendering continues |
| VR-20 | Swapchain out-of-date handling | Minimize then restore window | `VK_ERROR_OUT_OF_DATE_KHR` is handled, swapchain recreated |
| VR-21 | Swapchain suboptimal handling | Move window between monitors with different refresh rates | `VK_SUBOPTIMAL_KHR` is handled correctly |

---

## 4. Gameplay Rendering Test

### 4.1 Terrain

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| GR-01 | Terrain loads | Start a skirmish or campaign mission | Terrain heightmap renders correctly |
| GR-02 | Terrain textures | Zoom in on terrain | Texture splatting is correct, no missing or corrupted textures |
| GR-03 | Terrain lightmap | Observe terrain lighting | Lightmap is applied correctly, no over/under-exposure |
| GR-04 | Terrain LOD | Zoom in and out | Terrain LOD transitions are smooth, no popping |
| GR-05 | Terrain cliffs | Navigate to cliff areas | Cliffs render correctly with proper texturing |
| GR-06 | Terrain waterline | Navigate to water edges | Waterline transition is smooth, no z-fighting |

### 4.2 Units & Buildings

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| GR-07 | Infantry units | Spawn infantry units | Models render correctly with proper textures |
| GR-08 | Vehicle units | Spawn vehicles (tanks, humvees) | Models render correctly, wheels/tracks animate |
| GR-09 | Buildings | Place buildings | Buildings render correctly with proper textures |
| GR-10 | Building animations | Construct a building | Construction animation plays correctly |
| GR-11 | Unit animations | Order units to move/attack | Animations play correctly (walk, attack, death) |
| GR-12 | Unit selection | Select units | Selection indicator renders correctly |
| GR-13 | Health bars | Damage a unit | Health bar appears and updates correctly |
| GR-14 | Building damage states | Damage a building | Damage states (smoke, fire) render correctly |
| GR-15 | Submarine units | Deploy submarine (if available) | Submerge/surface rendering is correct |
| GR-16 | Stealth units | Deploy stealth unit | Stealth shader effect renders correctly |

### 4.3 Shadows

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| GR-17 | Unit shadows | Observe units in sunlight | Shadows are cast on terrain, correct direction |
| GR-18 | Building shadows | Observe buildings | Building shadows render correctly |
| GR-19 | Shadow quality | Observe shadow edges | Shadows have acceptable quality (PCF filtering, no severe aliasing) |
| GR-20 | Shadow map resolution | Check config/shadow settings | Shadow map resolution is appropriate (2048+) |
| GR-21 | Dynamic shadows | Move units, observe shadows | Shadows update in real-time as units move |
| GR-22 | Shadow acne | Observe terrain in shadowed areas | No shadow acne or peter-panning |

### 4.4 Water

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| GR-23 | Water rendering | Navigate to water area | Water surface renders with correct color and transparency |
| GR-24 | Water reflections | Observe water surface | Reflections render correctly (if enabled) |
| GR-25 | Water waves | Observe water over time | Wave animation plays smoothly |
| GR-26 | Water shoreline | Observe water-land boundary | Shoreline transition is smooth |
| GR-27 | Units in water | Order amphibious units into water | Units render correctly in water, wake effects visible |
| GR-28 | Underwater effects | Submerge submarine | Underwater visual effects render correctly |

### 4.5 Particles

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| GR-29 | Explosion particles | Cause an explosion | Explosion particle effect renders correctly |
| GR-30 | Smoke particles | Damage a building | Smoke particles rise and fade correctly |
| GR-31 | Fire particles | Destroy a vehicle | Fire particles render correctly |
| GR-32 | Muzzle flash | Order units to attack | Muzzle flash particles render correctly |
| GR-33 | Projectile trails | Fire a missile | Projectile trail renders correctly |
| GR-34 | Weather particles | Play in a map with weather (rain/snow) | Weather particles render correctly |
| GR-35 | Particle performance | Trigger many explosions simultaneously | Frame rate remains acceptable, no particle system crash |

### 4.6 UI / HUD

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| GR-36 | Command bar | Observe bottom UI | Command bar renders with all buttons visible |
| GR-37 | Minimap | Observe minimap | Minimap renders terrain, units, and camera view |
| GR-38 | Minimap interaction | Click on minimap | Camera moves to clicked location |
| GR-39 | Resource counter | Observe top-right UI | Resource count displays and updates correctly |
| GR-40 | Tooltip display | Hover over command button | Tooltip appears with correct text |
| GR-41 | Selection info | Select a unit | Unit info panel displays correctly |
| GR-42 | Game messages | Trigger in-game messages | Messages display correctly in the message area |
| GR-43 | Score screen | Open score screen (if available) | Score screen renders with all player stats |

### 4.7 2D Sprites

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| GR-44 | Cursor rendering | Move mouse | Custom cursor renders correctly |
| GR-45 | Cursor states | Perform different actions | Cursor changes correctly (attack, move, select) |
| GR-46 | Health bar sprites | Select damaged unit | Health bar sprite renders at correct position |
| GR-47 | Waypoint markers | Set move order with shift | Waypoint markers render correctly |
| GR-48 | Attack cursor | Order attack on enemy | Attack cursor renders over valid targets |
| GR-49 | Rally point | Set rally point for building | Rally point flag renders correctly |

---

## 5. Window Mode Tests

### 5.1 Windowed Mode

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| WM-01 | Launch in windowed mode | Set `Windowed=1` in config, launch | Game runs in a window, not fullscreen |
| WM-02 | Window drag | Drag window by title bar | Window moves, rendering continues |
| WM-03 | Window resize | Drag window edge to resize | Window resizes, rendering adapts correctly |
| WM-04 | Window minimize | Click minimize button | Game minimizes, no crash |
| WM-05 | Window restore | Restore from taskbar | Game restores, rendering continues |
| WM-06 | Window maximize | Click maximize button | Game fills screen, rendering correct |

### 5.2 Borderless Fullscreen

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| WM-07 | Launch borderless | Set borderless mode in config, launch | Game fills screen with no window border |
| WM-08 | Borderless rendering | Observe rendering | Rendering is correct, no black bars |
| WM-09 | Borderless Alt+Tab | Press Alt+Tab | Game switches to background, then back without crash |
| WM-10 | Borderless task switching | Switch to another app and back | Game remains stable |

### 5.3 Exclusive Fullscreen

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| WM-11 | Launch exclusive fullscreen | Set exclusive fullscreen in config, launch | Game takes over display exclusively |
| WM-12 | Exclusive rendering | Observe rendering | Rendering is correct, no compositing artifacts |
| WM-13 | Exclusive Alt+Tab | Press Alt+Tab | Game minimizes, display returns to desktop |
| WM-14 | Exclusive restore | Switch back to game | Game returns to exclusive fullscreen |

### 5.4 Mode Switching

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| WM-15 | Alt+Enter toggle | Press Alt+Enter in windowed mode | Game switches to fullscreen |
| WM-16 | Alt+Enter toggle back | Press Alt+Enter in fullscreen mode | Game switches to windowed |
| WM-17 | Alt+Enter in borderless | Press Alt+Enter in borderless mode | Game switches to windowed or exclusive |
| WM-18 | Rapid mode switching | Press Alt+Enter multiple times quickly | Game does not crash or hang |

### 5.5 Alt+Tab

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| WM-19 | Alt+Tab from windowed | Press Alt+Tab | Game loses focus, no crash |
| WM-20 | Alt+Tab back to game | Select game from Alt+Tab menu | Game regains focus, rendering continues |
| WM-21 | Alt+Tab from fullscreen | Press Alt+Tab in fullscreen | Game minimizes, desktop visible |
| WM-22 | Alt+Tab back to fullscreen | Switch back to game | Game returns to fullscreen, rendering correct |
| WM-23 | Alt+Tab during gameplay | Press Alt+Tab during active game | Game pauses or continues without crash |

### 5.6 Monitor Hot-Plug

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| WM-24 | Disconnect secondary monitor | Play in windowed mode, disconnect 2nd monitor | `WM_DISPLAYCHANGE` handled, game does not crash |
| WM-25 | Reconnect secondary monitor | Reconnect 2nd monitor | Game detects new monitor, rendering correct |
| WM-26 | Change primary monitor | Change primary display in Windows settings | Game adapts to new primary monitor |
| WM-27 | Change resolution | Change desktop resolution while game runs | Game adapts to new resolution |
| WM-28 | Change refresh rate | Change monitor refresh rate | Game adapts to new refresh rate |

### 5.7 DPI Scaling

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| WM-29 | 100% DPI | Set display scaling to 100%, launch game | Game renders at native resolution |
| WM-30 | 125% DPI | Set display scaling to 125%, launch game | Game renders correctly, UI is not blurry |
| WM-31 | 150% DPI | Set display scaling to 150%, launch game | Game renders correctly, UI scales properly |
| WM-32 | 200% DPI | Set display scaling to 200%, launch game | Game renders correctly, UI is legible |
| WM-33 | Per-monitor DPI | Move window between monitors with different DPI | Game adjusts DPI scaling correctly |
| WM-34 | DPI change while running | Change DPI scaling while game is running | Game handles `WM_DPICHANGED` correctly |

---

## 6. Performance Test

### 6.1 Frame Rate

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| PF-01 | 1080p menu FPS | Measure FPS in main menu at 1920x1080 | Stable 60 FPS (or vsync-capped) |
| PF-02 | 1080p gameplay FPS | Measure FPS during skirmish at 1920x1080 | Average >= 60 FPS, 1% low >= 45 FPS |
| PF-03 | 1440p gameplay FPS | Measure FPS during skirmish at 2560x1440 | Average >= 60 FPS on mid-range GPU |
| PF-04 | 4K gameplay FPS | Measure FPS during skirmish at 3840x2160 | Playable frame rate (>= 30 FPS) on high-end GPU |
| PF-05 | Large battle FPS | Measure FPS with 100+ units on screen | Average >= 45 FPS, no severe drops |
| PF-06 | Particle-heavy FPS | Measure FPS with many explosions/effects | Average >= 45 FPS |
| PF-07 | Frame time consistency | Measure frame time distribution | No frame time spikes > 50ms (excluding loading) |

### 6.2 Memory

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| PF-08 | Initial memory usage | Measure RAM usage after launch | Reasonable baseline (< 2 GB) |
| PF-09 | Gameplay memory usage | Measure RAM usage during gameplay | Stable, no continuous growth |
| PF-10 | 1-hour leak test | Play for 1 hour, measure RAM every 10 minutes | Memory growth < 50 MB over 1 hour |
| PF-11 | GPU memory usage | Measure VRAM usage during gameplay | Reasonable for resolution (< 4 GB at 1080p) |
| PF-12 | GPU memory stability | Measure VRAM over 1 hour | No continuous VRAM growth |
| PF-13 | Texture memory | Measure VRAM with texture streaming | Textures load/unload correctly, no VRAM bloat |

### 6.3 CPU

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| PF-14 | CPU usage (idle menu) | Measure CPU usage in main menu | Low CPU usage (< 10% on modern CPU) |
| PF-15 | CPU usage (gameplay) | Measure CPU usage during gameplay | Reasonable CPU usage (< 50% on modern quad-core) |
| PF-16 | CPU usage (large battle) | Measure CPU usage with 100+ units | CPU usage scales reasonably, no bottleneck |
| PF-17 | Multi-core utilization | Check CPU core distribution | Work is distributed across multiple cores |
| PF-18 | Main thread frame time | Profile main thread | Main thread frame time < 16ms (60 FPS target) |

### 6.4 Loading Times

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| PF-19 | Initial launch time | Measure time from double-click to main menu | < 30 seconds on SSD |
| PF-20 | Map load time | Measure time from mission start to playable | < 30 seconds for standard map |
| PF-21 | Save game load time | Measure time from load start to playable | < 15 seconds |

---

## 7. Save/Load Test

### 7.1 Save Game

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| SL-01 | Save game | During gameplay, save the game | Save completes without error, file is created |
| SL-02 | Save file integrity | Open save file in hex editor | File has valid header and structure |
| SL-03 | Save file size | Check save file size | Size is reasonable (< 50 MB for typical game) |
| SL-04 | Save during combat | Save during active combat | Save completes, no crash |
| SL-05 | Save with many units | Save with 100+ units on map | Save completes, all unit data preserved |
| SL-06 | Save filename | Save with custom filename | File is created with correct name |

### 7.2 Load Game

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| SL-07 | Load saved game | Load the saved game file | Load completes without error |
| SL-08 | Game state — units | After load, check unit positions | All units are at correct positions |
| SL-09 | Game state — buildings | After load, check buildings | All buildings are at correct positions with correct health |
| SL-10 | Game state — resources | After load, check player resources | Resource counts match saved state |
| SL-11 | Game state — fog of war | After load, check fog of war | Fog of war matches saved state |
| SL-12 | Game state — research | After load, check researched upgrades | Research state matches saved state |
| SL-13 | Game state — objectives | After load, check mission objectives | Objectives match saved state |
| SL-14 | Game state — minimap | After load, check minimap | Minimap matches saved state |

### 7.3 Pointer Integrity

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| SL-15 | No pointer corruption | Load game, play for 5 minutes | No crashes or access violations |
| SL-16 | Unit pointer validity | Load game, select and order units | Unit pointers are valid, no crashes |
| SL-17 | Building pointer validity | Load game, interact with buildings | Building pointers are valid, no crashes |
| SL-18 | Object list integrity | Load game, spawn new units | New objects integrate correctly with loaded state |
| SL-19 | Save-load-save cycle | Save, load, save again, load again | Second load produces identical state |
| SL-20 | Multiple save slots | Save to multiple slots, load each | Each slot loads correctly with correct state |

---

## 8. Zero Hour Variant Test

### 8.1 Build

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| ZH-01 | Configure GeneralsMD | `cmake` with `GeneralsMD` source directory | Configuration completes with no errors |
| ZH-02 | Build GeneralsMD RTS.exe | `cmake --build` for GeneralsMD target | Links successfully, `RTS.exe` produced |
| ZH-03 | Verify machine type | `dumpbin /headers RTS.exe` (GeneralsMD) | Output contains `machine (x64)` |
| ZH-04 | Verify no x86 dependencies | `dumpbin /dependents RTS.exe` (GeneralsMD) | No x86-only dependencies |

### 8.2 Content Rendering

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| ZH-05 | Zero Hour main menu | Launch GeneralsMD RTS.exe | Zero Hour main menu renders correctly |
| ZH-06 | Generals faction selection | Start a skirmish with Generals faction | Faction selection screen renders correctly |
| ZH-07 | Superweapon general | Play as Superweapon General | Superweapon general content renders correctly |
| ZH-08 | Laser general | Play as Laser General | Laser general content renders correctly |
| ZH-09 | Air general | Play as Air General | Air general content renders correctly |
| ZH-10 | Stealth general | Play as Stealth General | Stealth general content renders correctly |
| ZH-11 | Zero Hour units | Spawn all Zero Hour-specific units | All units render correctly with proper models |
| ZH-12 | Zero Hour buildings | Place all Zero Hour-specific buildings | All buildings render correctly with proper models |
| ZH-13 | Zero Hour upgrades | Research Zero Hour upgrades | Upgrades apply correctly, visual changes visible |
| ZH-14 | Zero Hour particle effects | Use Zero Hour-specific abilities | Particle effects render correctly |

### 8.3 Zero Hour Gameplay

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| ZH-15 | Zero Hour campaign mission | Start a Zero Hour campaign mission | Mission loads and plays correctly |
| ZH-16 | Zero Hour skirmish | Play a Zero Hour skirmish map | Gameplay is stable, no crashes |
| ZH-17 | Zero Hour AI | Play against Zero Hour AI | AI behaves correctly, no crashes |
| ZH-18 | Zero Hour audio | Play Zero Hour content | Audio plays correctly (XAudio2 backend) |
| ZH-19 | Zero Hour video | Trigger Zero Hour cutscene | Video plays correctly (FFmpeg backend) |

---

## 9. Clean Build Test

### 9.1 Clean Build Procedure

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| CB-01 | Delete build directory | `rm -rf build/windows-x64-msvc` | Directory is deleted |
| CB-02 | Reconfigure | `cmake --preset windows-x64-msvc` | Configuration completes with no errors |
| CB-03 | Full rebuild | `cmake --build build/windows-x64-msvc` | All targets build from scratch |
| CB-04 | Verify RTS.exe | Check `build/windows-x64-msvc/bin/RTS.exe` | Executable exists and is x64 |
| CB-05 | Verify no cached artifacts | Check build directory | No stale object files from previous build |

### 9.2 Warning Check

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| CB-06 | Count warnings | Build with `/W3` and count warnings | Total warnings < 100 (or documented acceptable list) |
| CB-07 | Review warning types | Categorize all warnings | No warnings in critical categories (uninitialized variables, deprecated APIs, security) |
| CB-08 | No new warnings | Compare warning list to baseline | No new warnings introduced since last build |
| CB-09 | Acceptable warnings documented | Check `WARNINGS.md` or equivalent | All remaining warnings are documented and accepted |

### 9.3 Error Check

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| CB-10 | Zero errors | Full clean build | Zero compilation or linking errors |
| CB-11 | No linker warnings | Check linker output | No LNK warnings (unresolved symbols, etc.) |
| CB-12 | No manifest errors | Check manifest generation | Manifest is generated correctly |
| CB-13 | No resource errors | Check resource compilation | All resources compile correctly |

### 9.4 Reproducibility

| ID | Test Case | Steps | Expected Result |
|----|-----------|-------|-----------------|
| CB-14 | Build twice | Run full clean build twice | Both builds succeed with identical results |
| CB-15 | Binary comparison | Compare `RTS.exe` from two builds | Binaries are identical (or differ only by timestamp/PDB) |
| CB-16 | Build from different directory | Build from a different working directory | Build succeeds, output is correct |

---

## Appendix A: Test Environment

### A.1 Minimum Test Hardware

| Component | Specification |
|-----------|---------------|
| CPU | Intel Core i5-10400 / AMD Ryzen 5 3600 or better |
| GPU | NVIDIA GTX 1060 6GB / AMD RX 580 8GB or better (Vulkan 1.3 compatible) |
| RAM | 16 GB DDR4 |
| Storage | SSD (NVMe recommended) |
| OS | Windows 10 22H2 x64 or Windows 11 x64 |

### A.2 Recommended Test Hardware

| Component | Specification |
|-----------|---------------|
| CPU | Intel Core i7-12700 / AMD Ryzen 7 5800X or better |
| GPU | NVIDIA RTX 3060 / AMD RX 6700 XT or better |
| RAM | 32 GB DDR4/DDR5 |
| Storage | NVMe SSD |
| OS | Windows 11 23H2 x64 |

### A.3 Software Dependencies

| Software | Version | Purpose |
|----------|---------|---------|
| Visual Studio 2022 | 17.8+ | Compiler and IDE |
| CMake | 3.20+ | Build system |
| Vulkan SDK | 1.3+ | Vulkan headers, validation layers, SPIR-V tools |
| Windows SDK | 10.0.22000+ | Windows API headers |
| dumpbin | (included with MSVC) | Binary inspection |

### A.4 Test Tools

| Tool | Purpose |
|------|---------|
| RenderDoc | Vulkan frame capture and analysis |
| NVIDIA Nsight / AMD Radeon GPU Profiler | GPU profiling |
| Windows Performance Recorder | CPU and system profiling |
| Application Verifier | Memory and handle leak detection |
| DebugView | Real-time debug output capture |
| PresentMon | Frame time and FPS measurement |

---

## Appendix B: Test Execution Summary

| Section | Test Cases | Passed | Failed | Blocked | Not Run |
|---------|-----------|--------|--------|---------|---------|
| 1. Build Verification | 27 | | | | |
| 2. Launch Test | 14 | | | | |
| 3. Vulkan Renderer Test | 21 | | | | |
| 4. Gameplay Rendering Test | 49 | | | | |
| 5. Window Mode Tests | 34 | | | | |
| 6. Performance Test | 21 | | | | |
| 7. Save/Load Test | 20 | | | | |
| 8. Zero Hour Variant Test | 19 | | | | |
| 9. Clean Build Test | 16 | | | | |
| **Total** | **221** | | | | |

---

## Appendix C: Known Issues & Acceptable Warnings

### C.1 Acceptable Warnings

| Warning | Reason | Action |
|---------|--------|--------|
| `C4996` (deprecated function) | Legacy Win32 APIs still in use | Suppress with `_CRT_SECURE_NO_WARNINGS`; migrate in future |
| `C4244` (conversion loss) | Legacy code with implicit conversions | Review case-by-case; most are benign in game context |

### C.2 Known Issues

| Issue | Severity | Workaround | Planned Fix |
|-------|----------|------------|-------------|
| None | — | — | — |

---

## Appendix D: Sign-Off

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Test Lead | | | |
| Build Engineer | | | |
| QA Engineer | | | |
| Project Lead | | | |
