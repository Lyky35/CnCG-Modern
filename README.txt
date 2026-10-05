RTS.exe - Command & Conquer Generals (x64, Vulkan renderer build)
=====================================================================

This build renders via Vulkan (vulkan-1.dll, loaded from your GPU driver).
Direct3D 8 is no longer used at runtime.

Files in this archive:
  RTS.exe              the game executable (x86-64 Windows)
  libgcc_s_seh-1.dll   MinGW runtime
  libstdc++-6.dll      MinGW runtime
  libwinpthread-1.dll  MinGW runtime (pulled in by libstdc++)

To run:
  1. Place RTS.exe (and the three DLLs) next to the game data, i.e. a folder
     containing the original Generals/Zero Hour Data directory:
         RTS.exe
         Data\INI, Data\Big, ... (from your game install)
  2. Run RTS.exe.

Notes:
  - Single-player only: GameSpy multiplayer services are stubbed out.
  - Movies are disabled in this build (no FFmpeg for the target).
  - Some fixed-function effects (fog, DOT3, ProcessVertices-based paths)
    degrade and are logged; first visual-pass bugs are expected - file them
    on the GitHub issues page.

Repository: https://github.com/Lyky35/CnCG-Modern
