/*
**	DX8Vk bridge - a Vulkan-backed implementation of the IDirect3D8/IDirect3DDevice8
**	COM surface used by WW3D2. Injected at the single point where DX8Wrapper calls
**	Direct3DCreate8(); everything above (WW3D2, W3DDevice, W3DShaderManager) then
**	drives Vulkan without source changes.
**
**	Enable at runtime with CNC_VULKAN=1 (see DX8Vk_ShouldUseBridge()).
**	Scope: fixed-function triangle/line/point rendering, FVF re-staging, two texture
**	stages, common blend/depth states, render targets. Unsupported calls degrade with
**	logs rather than crashing.
*/
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

struct IDirect3D8;

// Returns a fake IDirect3D8* (refcount 1) or NULL on init failure.
IDirect3D8* DX8Vk_CreateD3D8(void);

// True when the bridge should be used instead of the real Direct3D8 runtime.
bool DX8Vk_ShouldUseBridge(void);
