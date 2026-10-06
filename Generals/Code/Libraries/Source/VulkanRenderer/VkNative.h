#pragma once
#include <windows.h>
#include <d3d8.h>

class VKVolumeTexture; class VKVolume; class VKCubeTexture;
class VKRoot; class VKDevice; class VKVertexBuffer; class VKIndexBuffer;
class VKBaseTexture; class VKTexture; class VKSurface; class VKSwapChain;
// Native Vulkan backend classes replacing the D3D8 COM interfaces. Plain C++, no vtables/COM.
// Generated from <d3d8.h> declaration order. Method bodies live in VkNative.cpp.

class VKRoot
{
public:
	HRESULT QueryInterface(REFIID riid, void** ppvObject);
	ULONG AddRef();
	ULONG Release();
	HRESULT RegisterSoftwareDevice(void * pInitializeFunction);
	UINT GetAdapterCount();
	HRESULT GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 * pIdentifier);
	UINT GetAdapterModeCount(UINT Adapter);
	HRESULT EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE * pMode);
	HRESULT GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE * pMode);
	HRESULT CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, WINBOOL Windowed);
	HRESULT CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
	HRESULT CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, WINBOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType);
	HRESULT CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
	HRESULT GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8 * pCaps);
	HMONITOR GetAdapterMonitor(UINT Adapter);
	HRESULT CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType,HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS * pPresentationParameters, struct VKDevice ** ppReturnedDeviceInterface);
public:
	static VKRoot* Create();
};

class VKDevice
{
public:
	HRESULT QueryInterface(REFIID riid, void** ppvObject);
	ULONG AddRef();
	ULONG Release();
	HRESULT TestCooperativeLevel();
	UINT GetAvailableTextureMem();
	HRESULT ResourceManagerDiscardBytes(DWORD Bytes);
	HRESULT GetDirect3D(VKRoot ** ppD3D8);
	HRESULT GetDeviceCaps(D3DCAPS8 * pCaps);
	HRESULT GetDisplayMode(D3DDISPLAYMODE * pMode);
	HRESULT GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS * pParameters);
	HRESULT SetCursorProperties(UINT XHotSpot, UINT YHotSpot, VKSurface * pCursorBitmap);
	void SetCursorPosition(UINT XScreenSpace, UINT YScreenSpace,DWORD Flags);
	WINBOOL ShowCursor(WINBOOL bShow);
	HRESULT CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS * pPresentationParameters, VKSwapChain ** pSwapChain);
	HRESULT Reset(D3DPRESENT_PARAMETERS * pPresentationParameters);
	HRESULT Present(const RECT *src_rect, const RECT *dst_rect, HWND dst_window_override, const RGNDATA *dirty_region);
	HRESULT GetBackBuffer(UINT BackBuffer,D3DBACKBUFFER_TYPE Type,VKSurface ** ppBackBuffer);
	HRESULT GetRasterStatus(D3DRASTER_STATUS * pRasterStatus);
	void SetGammaRamp(DWORD flags, const D3DGAMMARAMP *ramp);
	void GetGammaRamp(D3DGAMMARAMP * pRamp);
	HRESULT CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,VKTexture ** ppTexture);
	HRESULT CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,VKVolumeTexture ** ppVolumeTexture);
	HRESULT CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,VKCubeTexture ** ppCubeTexture);
	HRESULT CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,VKVertexBuffer ** ppVertexBuffer);
	HRESULT CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,VKIndexBuffer ** ppIndexBuffer);
	HRESULT CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,WINBOOL Lockable,VKSurface ** ppSurface);
	HRESULT CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,VKSurface ** ppSurface);
	HRESULT CreateImageSurface(UINT Width,UINT Height,D3DFORMAT Format,VKSurface ** ppSurface);
	HRESULT CopyRects(VKSurface *src_surface, const RECT *src_rects, UINT rect_count, VKSurface *dst_surface, const POINT *dst_points);
	HRESULT UpdateTexture(VKBaseTexture * pSourceTexture,VKBaseTexture * pDestinationTexture);
	HRESULT GetFrontBuffer(VKSurface * pDestSurface);
	HRESULT SetRenderTarget(VKSurface * pRenderTarget,VKSurface * pNewZStencil);
	HRESULT GetRenderTarget(VKSurface ** ppRenderTarget);
	HRESULT GetDepthStencilSurface(VKSurface ** ppZStencilSurface);
	HRESULT BeginScene();
	HRESULT EndScene();
	HRESULT Clear(DWORD rect_count, const D3DRECT *rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil);
	HRESULT SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX *matrix);
	HRESULT GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX * pMatrix);
	HRESULT MultiplyTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX *matrix);
	HRESULT SetViewport(const D3DVIEWPORT8 *viewport);
	HRESULT GetViewport(D3DVIEWPORT8 * pViewport);
	HRESULT SetMaterial(const D3DMATERIAL8 *material);
	HRESULT GetMaterial(D3DMATERIAL8 *pMaterial);
	HRESULT SetLight(DWORD index, const D3DLIGHT8 *light);
	HRESULT GetLight(DWORD Index,D3DLIGHT8 * pLight);
	HRESULT LightEnable(DWORD Index,WINBOOL Enable);
	HRESULT GetLightEnable(DWORD Index,WINBOOL * pEnable);
	HRESULT SetClipPlane(DWORD index, const float *plane);
	HRESULT GetClipPlane(DWORD Index,float * pPlane);
	HRESULT SetRenderState(D3DRENDERSTATETYPE State,DWORD Value);
	HRESULT GetRenderState(D3DRENDERSTATETYPE State,DWORD * pValue);
	HRESULT BeginStateBlock();
	HRESULT EndStateBlock(DWORD * pToken);
	HRESULT ApplyStateBlock(DWORD Token);
	HRESULT CaptureStateBlock(DWORD Token);
	HRESULT DeleteStateBlock(DWORD Token);
	HRESULT CreateStateBlock(D3DSTATEBLOCKTYPE Type,DWORD * pToken);
	HRESULT SetClipStatus(const D3DCLIPSTATUS8 *clip_status);
	HRESULT GetClipStatus(D3DCLIPSTATUS8 * pClipStatus);
	HRESULT GetTexture(DWORD Stage,VKBaseTexture ** ppTexture);
	HRESULT SetTexture(DWORD Stage,VKBaseTexture * pTexture);
	HRESULT GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD * pValue);
	HRESULT SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value);
	HRESULT ValidateDevice(DWORD * pNumPasses);
	HRESULT GetInfo(DWORD DevInfoID,void * pDevInfoStruct,DWORD DevInfoStructSize);
	HRESULT SetPaletteEntries(UINT palette_idx, const PALETTEENTRY *entries);
	HRESULT GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY * pEntries);
	HRESULT SetCurrentTexturePalette(UINT PaletteNumber);
	HRESULT GetCurrentTexturePalette(UINT * PaletteNumber);
	HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount);
	HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT minIndex,UINT NumVertices,UINT startIndex,UINT primCount);
	HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE primitive_type, UINT primitive_count, const void *data, UINT stride);
	HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE primitive_type, UINT min_vertex_idx, UINT vertex_count, UINT primitive_count, const void *index_data, D3DFORMAT index_format, const void *data, UINT stride);
	HRESULT ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,VKVertexBuffer * pDestBuffer,DWORD Flags);
	HRESULT CreateVertexShader(const DWORD *declaration, const DWORD *byte_code, DWORD *shader, DWORD usage);
	HRESULT SetVertexShader(DWORD Handle);
	HRESULT GetVertexShader(DWORD * pHandle);
	HRESULT DeleteVertexShader(DWORD Handle);
	HRESULT SetVertexShaderConstant(DWORD reg_idx, const void *data, DWORD count);
	HRESULT GetVertexShaderConstant(DWORD Register,void * pConstantData,DWORD ConstantCount);
	HRESULT GetVertexShaderDeclaration(DWORD Handle,void * pData,DWORD * pSizeOfData);
	HRESULT GetVertexShaderFunction(DWORD Handle,void * pData,DWORD * pSizeOfData);
	HRESULT SetStreamSource(UINT StreamNumber,VKVertexBuffer * pStreamData,UINT Stride);
	HRESULT GetStreamSource(UINT StreamNumber,VKVertexBuffer ** ppStreamData,UINT * pStride);
	HRESULT SetIndices(VKIndexBuffer * pIndexData,UINT BaseVertexIndex);
	HRESULT GetIndices(VKIndexBuffer ** ppIndexData,UINT * pBaseVertexIndex);
	HRESULT CreatePixelShader(const DWORD *byte_code, DWORD *shader);
	HRESULT SetPixelShader(DWORD Handle);
	HRESULT GetPixelShader(DWORD * pHandle);
	HRESULT DeletePixelShader(DWORD Handle);
	HRESULT SetPixelShaderConstant(DWORD reg_idx, const void *data, DWORD count);
	HRESULT GetPixelShaderConstant(DWORD Register,void * pConstantData,DWORD ConstantCount);
	HRESULT GetPixelShaderFunction(DWORD Handle,void * pData,DWORD * pSizeOfData);
	HRESULT DrawRectPatch(UINT handle, const float *segment_count, const D3DRECTPATCH_INFO *patch_info);
	HRESULT DrawTriPatch(UINT handle, const float *segment_count, const D3DTRIPATCH_INFO *patch_info);
	HRESULT DeletePatch(UINT Handle);
public:
	static VKDevice* Create();
};

class VKVertexBuffer
{
public:
	HRESULT QueryInterface(REFIID riid, void** ppvObject);
	ULONG AddRef();
	ULONG Release();
	HRESULT GetDevice(struct VKDevice ** ppDevice);
	HRESULT SetPrivateData(REFGUID refguid, const void *data, DWORD data_size, DWORD flags);
	HRESULT GetPrivateData(REFGUID refguid, void * pData, DWORD * pSizeOfData);
	HRESULT FreePrivateData(REFGUID refguid);
	DWORD SetPriority(DWORD PriorityNew);
	DWORD GetPriority();
	void PreLoad();
	D3DRESOURCETYPE GetType();
	HRESULT Lock(UINT OffsetToLock, UINT SizeToLock, BYTE ** ppbData, DWORD Flags);
	HRESULT Unlock();
	HRESULT GetDesc(D3DVERTEXBUFFER_DESC * pDesc);
public:
	static VKVertexBuffer* Create();
};

class VKIndexBuffer
{
public:
	HRESULT QueryInterface(REFIID riid, void** ppvObject);
	ULONG AddRef();
	ULONG Release();
	HRESULT GetDevice(struct VKDevice ** ppDevice);
	HRESULT SetPrivateData(REFGUID refguid, const void *data, DWORD data_size, DWORD flags);
	HRESULT GetPrivateData(REFGUID refguid, void * pData, DWORD * pSizeOfData);
	HRESULT FreePrivateData(REFGUID refguid);
	DWORD SetPriority(DWORD PriorityNew);
	DWORD GetPriority();
	void PreLoad();
	D3DRESOURCETYPE GetType();
	HRESULT Lock(UINT OffsetToLock, UINT SizeToLock, BYTE ** ppbData, DWORD Flags);
	HRESULT Unlock();
	HRESULT GetDesc(D3DINDEXBUFFER_DESC * pDesc);
public:
	static VKIndexBuffer* Create();
};

class VKBaseTexture
{
public:
	HRESULT QueryInterface(REFIID riid, void** ppvObject);
	ULONG AddRef();
	ULONG Release();
	HRESULT GetDevice(struct VKDevice ** ppDevice);
	HRESULT SetPrivateData(REFGUID refguid, const void *data, DWORD data_size, DWORD flags);
	HRESULT GetPrivateData(REFGUID refguid, void * pData, DWORD * pSizeOfData);
	HRESULT FreePrivateData(REFGUID refguid);
	DWORD SetPriority(DWORD PriorityNew);
	DWORD GetPriority();
	void PreLoad();
	D3DRESOURCETYPE GetType();
	DWORD SetLOD(DWORD LODNew);
	DWORD GetLOD();
	DWORD GetLevelCount();
public:
	static VKBaseTexture* Create();
};

class VKTexture : public VKBaseTexture
{
public:
	HRESULT QueryInterface(REFIID riid, void** ppvObject);
	ULONG AddRef();
	ULONG Release();
	HRESULT GetDevice(struct VKDevice ** ppDevice);
	HRESULT SetPrivateData(REFGUID refguid, const void *data, DWORD data_size, DWORD flags);
	HRESULT GetPrivateData(REFGUID refguid, void * pData, DWORD * pSizeOfData);
	HRESULT FreePrivateData(REFGUID refguid);
	DWORD SetPriority(DWORD PriorityNew);
	DWORD GetPriority();
	void PreLoad();
	D3DRESOURCETYPE GetType();
	DWORD SetLOD(DWORD LODNew);
	DWORD GetLOD();
	DWORD GetLevelCount();
	HRESULT GetLevelDesc(UINT Level,D3DSURFACE_DESC * pDesc);
	HRESULT GetSurfaceLevel(UINT Level,VKSurface ** ppSurfaceLevel);
	HRESULT LockRect(UINT level, D3DLOCKED_RECT *locked_rect, const RECT *rect, DWORD flags);
	HRESULT UnlockRect(UINT Level);
	HRESULT AddDirtyRect(const RECT *dirty_rect);
public:
	static VKTexture* Create();
};

class VKSurface
{
public:
	HRESULT QueryInterface(REFIID riid, void** ppvObject);
	ULONG AddRef();
	ULONG Release();
	HRESULT GetDevice(struct VKDevice ** ppDevice);
	HRESULT SetPrivateData(REFGUID refguid, const void *data, DWORD data_size, DWORD flags);
	HRESULT GetPrivateData(REFGUID refguid,void * pData,DWORD * pSizeOfData);
	HRESULT FreePrivateData(REFGUID refguid);
	HRESULT GetContainer(REFIID riid, void ** ppContainer);
	HRESULT GetDesc(D3DSURFACE_DESC * pDesc);
	HRESULT LockRect(D3DLOCKED_RECT *locked_rect, const RECT *rect, DWORD flags);
	HRESULT UnlockRect();
public:
	static VKSurface* Create();
};

class VKSwapChain
{
public:
	HRESULT QueryInterface(REFIID riid, void** ppvObject);
	ULONG AddRef();
	ULONG Release();
	HRESULT Present(const RECT *src_rect, const RECT *dst_rect, HWND dst_window_override, const RGNDATA *dirty_region);
	HRESULT GetBackBuffer(UINT BackBuffer, D3DBACKBUFFER_TYPE Type, struct VKSurface ** ppBackBuffer);
public:
	static VKSwapChain* Create();
};

void VK_ShutdownAll();
