/*
**	D3DX8 shim implementations (see dx8compat.h).
**	Texture/surface/shader helpers are stubbed: they return "not implemented" so call
**	sites take their existing error paths. A Vulkan-based renderer replaces this layer.
*/

#include "dx8compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char error_string_buffer[256];

LPCSTR __cdecl D3DXGetErrorStringA(HRESULT hr)
{
	switch (hr) {
		case D3D_OK: return "D3D_OK";
		case D3DERR_CONFLICTINGTEXTUREFILTER: return "D3DERR_CONFLICTINGTEXTUREFILTER";
		case D3DERR_CONFLICTINGRENDERSTATE: return "D3DERR_CONFLICTINGRENDERSTATE";
		case D3DERR_DEVICELOST: return "D3DERR_DEVICELOST";
		case D3DERR_DRIVERINTERNALERROR: return "D3DERR_DRIVERINTERNALERROR";
		case D3DERR_DEVICENOTRESET: return "D3DERR_DEVICENOTRESET";
		case D3DERR_DRIVERINVALIDCALL: return "D3DERR_DRIVERINVALIDCALL";
		case D3DERR_CONFLICTINGTEXTUREPALETTE: return "D3DERR_CONFLICTINGTEXTUREPALETTE";
		case D3DERR_OUTOFVIDEOMEMORY: return "D3DERR_OUTOFVIDEOMEMORY";
		case D3DERR_TOOMANYOPERATIONS: return "D3DERR_TOOMANYOPERATIONS";
		case D3DERR_UNSUPPORTEDALPHAARG: return "D3DERR_UNSUPPORTEDALPHAARG";
		case D3DERR_UNSUPPORTEDALPHAOPERATION: return "D3DERR_UNSUPPORTEDALPHAOPERATION";
		case D3DERR_UNSUPPORTEDCOLORARG: return "D3DERR_UNSUPPORTEDCOLORARG";
		case D3DERR_UNSUPPORTEDCOLOROPERATION: return "D3DERR_UNSUPPORTEDCOLOROPERATION";
		case D3DERR_UNSUPPORTEDFACTORVALUE: return "D3DERR_UNSUPPORTEDFACTORVALUE";
		case D3DERR_UNSUPPORTEDTEXTUREFILTER: return "D3DERR_UNSUPPORTEDTEXTUREFILTER";
		case D3DERR_WRONGTEXTUREFORMAT: return "D3DERR_WRONGTEXTUREFORMAT";
		case D3DERR_INVALIDCALL: return "D3DERR_INVALIDCALL";
		case D3DERR_INVALIDDEVICE: return "D3DERR_INVALIDDEVICE";
		case D3DERR_NOTAVAILABLE: return "D3DERR_NOTAVAILABLE";
		case D3DERR_NOTFOUND: return "D3DERR_NOTFOUND";
		case D3DERR_MOREDATA: return "D3DERR_MOREDATA";
		default:
			snprintf(error_string_buffer, sizeof(error_string_buffer), "0x%08X", (unsigned int)hr);
			return error_string_buffer;
	}
}

HRESULT __cdecl D3DXGetErrorStringA(HRESULT hr, LPSTR pBuffer, DWORD BufferLen)
{
	if (pBuffer && BufferLen) {
		strncpy(pBuffer, D3DXGetErrorStringA(hr), BufferLen - 1);
		pBuffer[BufferLen - 1] = 0;
	}
	return D3D_OK;
}

HRESULT __cdecl D3DXCreateTexture(
	VKDevice *pDevice,
	unsigned int Width, unsigned int Height, unsigned int MipLevels,
	DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
	VKTexture **ppTexture)
{
	(void)pDevice; (void)Width; (void)Height; (void)MipLevels;
	(void)Usage; (void)Format; (void)Pool;
	if (ppTexture) *ppTexture = NULL;
	return D3DERR_NOTAVAILABLE;
}

HRESULT __cdecl D3DXCreateTextureFromFileExA(
	VKDevice *pDevice,
	LPCSTR pSrcFile,
	unsigned int Width, unsigned int Height, unsigned int MipLevels,
	DWORD Usage,
	D3DFORMAT Format, D3DPOOL Pool,
	DWORD Filter, DWORD MipFilter, DWORD ColorKey,
	void *pSrcInfo, void *pPalette,
	VKTexture **ppTexture)
{
	(void)pDevice; (void)pSrcFile; (void)Width; (void)Height; (void)MipLevels;
	(void)Usage; (void)Format; (void)Pool; (void)Filter; (void)MipFilter;
	(void)ColorKey; (void)pSrcInfo; (void)pPalette;
	if (ppTexture) *ppTexture = NULL;
	return D3DERR_NOTAVAILABLE;
}

HRESULT __cdecl D3DXFilterTexture(
	VKBaseTexture *pTexture,
	const RECT *pSrcRect,
	unsigned int SrcLevel,
	DWORD Filter)
{
	(void)pTexture; (void)pSrcRect; (void)SrcLevel; (void)Filter;
	return D3DERR_NOTAVAILABLE;
}

static HRESULT surface_to_surface_software(
	VKSurface *pDest, const RECT *pDestRect,
	VKSurface *pSrc, const RECT *pSrcRect,
	D3DCOLOR colorkey)
{
	D3DSURFACE_DESC desc, sdesc;
	RECT dr, sr;

	if (!pDest || !pSrc) return D3DERR_INVALIDCALL;
	if (FAILED(pDest->GetDesc(&desc))) return D3DERR_INVALIDCALL;
	if (FAILED(pSrc->GetDesc(&sdesc))) return D3DERR_INVALIDCALL;

	dr = pDestRect ? *pDestRect : (RECT){0, 0, (LONG)desc.Width, (LONG)desc.Height};
	sr = pSrcRect ? *pSrcRect : (RECT){0, 0, (LONG)sdesc.Width, (LONG)sdesc.Height};

	if (desc.Format != sdesc.Format) return D3DERR_INVALIDCALL;

	unsigned int w = (unsigned int)(dr.right - dr.left);
	unsigned int h = (unsigned int)(dr.bottom - dr.top);
	unsigned int sw = (unsigned int)(sr.right - sr.left);
	if (w != sw) return D3DERR_INVALIDCALL;
	if (h != (unsigned int)(sr.bottom - sr.top)) return D3DERR_INVALIDCALL;

	D3DLOCKED_RECT dlock, slock;
	if (FAILED(pDest->LockRect(&dlock, &dr, 0))) return D3DERR_INVALIDCALL;
	if (FAILED(pSrc->LockRect(&slock, &sr, D3DLOCK_READONLY))) { pDest->UnlockRect(); return D3DERR_INVALIDCALL; }

	unsigned int bpp = 4;
	if (desc.Format == D3DFMT_R5G6B5 || desc.Format == D3DFMT_A1R5G5B5 || desc.Format == D3DFMT_X1R5G5B5) bpp = 2;
	else if (desc.Format == D3DFMT_A8R8G8B8 || desc.Format == D3DFMT_X8R8G8B8) bpp = 4;
	else if (desc.Format == D3DFMT_A8) bpp = 1;

	for (unsigned int y = 0; y < h; y++) {
		const unsigned char *srow = (const unsigned char *)slock.pBits + y * slock.Pitch;
		unsigned char *drow = (unsigned char *)dlock.pBits + y * dlock.Pitch;
		memcpy(drow, srow, (size_t)w * bpp);
	}
	(void)colorkey;

	pSrc->UnlockRect();
	pDest->UnlockRect();
	return D3D_OK;
}

HRESULT __cdecl D3DXLoadSurfaceFromSurface(
	VKSurface *pDestSurface,
	void *pDestPalette,
	const RECT *pDestRect,
	VKSurface *pSrcSurface,
	void *pSrcPalette,
	const RECT *pSrcRect,
	DWORD Filter,
	D3DCOLOR ColorKey)
{
	(void)pDestPalette; (void)pSrcPalette; (void)Filter;
	if (Filter != D3DX_FILTER_NONE && pDestRect && pSrcRect) {
		int dw = pDestRect->right - pDestRect->left;
		int sw = pSrcRect->right - pSrcRect->left;
		int dh = pDestRect->bottom - pDestRect->top;
		int sh = pSrcRect->bottom - pSrcRect->top;
		if (dw != sw || dh != sh) {
			/* scaling requires real D3DX filtering; unsupported in the shim */
			return D3DERR_NOTAVAILABLE;
		}
	}
	return surface_to_surface_software(pDestSurface, pDestRect, pSrcSurface, pSrcRect, ColorKey);
}

HRESULT __cdecl D3DXAssembleShader(
	LPCSTR pSrcData,
	unsigned int SrcDataLen,
	DWORD Flags,
	const void *pDefines,
	D3DXBuffer **ppCompiledShader,
	D3DXBuffer **ppErrorMessages)
{
	(void)pSrcData; (void)SrcDataLen; (void)Flags; (void)pDefines;
	if (ppCompiledShader) *ppCompiledShader = NULL;
	if (ppErrorMessages) *ppErrorMessages = NULL;
	return D3DERR_NOTAVAILABLE;
}
