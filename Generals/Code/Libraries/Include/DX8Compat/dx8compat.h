/*
**	D3DX8 compatibility shim.
**	DX8-era D3DX helper library (d3dx8_*.dll/.lib) is no longer distributed with
**	Windows or the modern Windows SDKs. This shim provides the small subset of the
**	D3DX8 helper surface that WW3D2/GameEngineDevice use:
**	  - full software implementations of the math/FVF/error-string helpers
**	  - error-returning stubs for the texture/surface/shader-assembly helpers
**	See dx8compat.cpp for the stub implementations.
*/
#ifndef DX8COMPAT_H
#define DX8COMPAT_H

#include <d3d8.h>
#include <string.h>
#include <math.h>

#ifndef D3DX_PI
#define D3DX_PI 3.14159265358979323846f
#endif

#define D3DX_DEFAULT ((unsigned int)-1)

#ifndef D3DCURSOR_IMMEDIATE_UPDATE
#define D3DCURSOR_IMMEDIATE_UPDATE 0x00000001L
#endif

#define D3DX_FILTER_NONE     0x00000001
#define D3DX_FILTER_POINT    0x00000002
#define D3DX_FILTER_LINEAR   0x00000004
#define D3DX_FILTER_BOX      0x00000008
#define D3DX_FILTER_TRIANGLE 0x00000010

typedef struct _D3DXVECTOR3
{
	float x, y, z;
	_D3DXVECTOR3() : x(0), y(0), z(0) {}
	_D3DXVECTOR3(float fx, float fy, float fz) : x(fx), y(fy), z(fz) {}
	float & operator[](int i) { return (&x)[i]; }
	const float & operator[](int i) const { return (&x)[i]; }
} D3DXVECTOR3;

typedef struct _D3DXVECTOR4
{
	union
	{
		struct { float x, y, z, w; };
		float C[4];
	};
	_D3DXVECTOR4() : x(0), y(0), z(0), w(0) {}
	_D3DXVECTOR4(float fx, float fy, float fz, float fw) : x(fx), y(fy), z(fz), w(fw) {}
	float & operator[](int i) { return C[i]; }
	const float & operator[](int i) const { return C[i]; }
} D3DXVECTOR4;



typedef struct _D3DXMATRIX
{
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];
	};
	_D3DXMATRIX()
	{
		memset(m, 0, sizeof(m));
	}
	_D3DXMATRIX(float m11, float m12, float m13, float m14,
	            float m21, float m22, float m23, float m24,
	            float m31, float m32, float m33, float m34,
	            float m41, float m42, float m43, float m44)
	{
		_11 = m11; _12 = m12; _13 = m13; _14 = m14;
		_21 = m21; _22 = m22; _23 = m23; _24 = m24;
		_31 = m31; _32 = m32; _33 = m33; _34 = m34;
		_41 = m41; _42 = m42; _43 = m43; _44 = m44;
	}
	float * operator[](int row) { return m[row]; }
	const float * operator[](int row) const { return m[row]; }
	_D3DXMATRIX & operator *=(const _D3DXMATRIX & b)
	{
		_D3DXMATRIX t;
		for (int r = 0; r < 4; ++r)
			for (int c = 0; c < 4; ++c)
				t.m[r][c] = m[r][0] * b.m[0][c] + m[r][1] * b.m[1][c] + m[r][2] * b.m[2][c] + m[r][3] * b.m[3][c];
		*this = t;
		return *this;
	}
} D3DXMATRIX;


typedef struct D3DXBuffer ID3DXBuffer;

/* Minimal buffer object matching the D3DXBUFFER call pattern used in the code */
struct D3DXBuffer
{
	unsigned char *data;
	unsigned size;
	unsigned refs;

	void *GetBufferPointer() { return data; }
	unsigned GetBufferSize() { return size; }
	unsigned AddRef() { return ++refs; }
	unsigned Release()
	{
		if (--refs == 0) { delete[] data; delete this; return 0; }
		return refs;
	}
};

#ifdef __cplusplus
extern "C" {
#endif

/* ---- math helpers (implemented inline in software) ---- */

static inline D3DXMATRIX * __cdecl D3DXMatrixIdentity(D3DXMATRIX *pOut)
{
	memset(pOut->m, 0, sizeof(pOut->m));
	pOut->_11 = pOut->_22 = pOut->_33 = pOut->_44 = 1.0f;
	return pOut;
}

static inline D3DXMATRIX * __cdecl D3DXMatrixMultiply(D3DXMATRIX *pOut, const D3DXMATRIX *pM1, const D3DXMATRIX *pM2)
{
	D3DXMATRIX tmp;
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++) {
			tmp.m[r][c] =
				pM1->m[r][0] * pM2->m[0][c] +
				pM1->m[r][1] * pM2->m[1][c] +
				pM1->m[r][2] * pM2->m[2][c] +
				pM1->m[r][3] * pM2->m[3][c];
		}
	*pOut = tmp;
	return pOut;
}

static inline D3DXMATRIX * __cdecl D3DXMatrixTranspose(D3DXMATRIX *pOut, const D3DXMATRIX *pM)
{
	D3DXMATRIX tmp = *pM;
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 4; c++)
			pOut->m[r][c] = tmp.m[c][r];
	return pOut;
}

static inline D3DXMATRIX * __cdecl D3DXMatrixScaling(D3DXMATRIX *pOut, float sx, float sy, float sz)
{
	D3DXMatrixIdentity(pOut);
	pOut->_11 = sx; pOut->_22 = sy; pOut->_33 = sz;
	return pOut;
}

static inline D3DXMATRIX * __cdecl D3DXMatrixTranslation(D3DXMATRIX *pOut, float x, float y, float z)
{
	D3DXMatrixIdentity(pOut);
	pOut->_41 = x; pOut->_42 = y; pOut->_43 = z;
	return pOut;
}

static inline D3DXMATRIX * __cdecl D3DXMatrixRotationZ(D3DXMATRIX *pOut, float angle)
{
	D3DXMatrixIdentity(pOut);
	float c = cosf(angle), s = sinf(angle);
	pOut->_11 = c; pOut->_12 = s;
	pOut->_21 = -s; pOut->_22 = c;
	return pOut;
}

static inline D3DXMATRIX * __cdecl D3DXMatrixInverse(D3DXMATRIX *pOut, float *pDeterminant, const D3DXMATRIX *pM)
{
	const float *s = &pM->_11;

	float c00 = s[10] * s[15] - s[11] * s[14];
	float c01 = s[9]  * s[15] - s[11] * s[13];
	float c02 = s[9]  * s[14] - s[10] * s[13];
	float c03 = s[8]  * s[15] - s[11] * s[12];
	float c04 = s[8]  * s[14] - s[10] * s[12];
	float c05 = s[8]  * s[13] - s[9]  * s[12];

	float c06 = s[6]  * s[11] - s[7]  * s[10];
	float c07 = s[5]  * s[11] - s[7]  * s[9];
	float c08 = s[5]  * s[10] - s[6]  * s[9];
	float c09 = s[4]  * s[11] - s[7]  * s[8];
	float c10 = s[4]  * s[10] - s[6]  * s[8];
	float c11 = s[4]  * s[9]  - s[5]  * s[8];

	float det = s[0] * c00 - s[1] * c01 + s[2] * c02 - s[3] * c03 + s[4] * c06 - s[5] * c07;

	if (pDeterminant) *pDeterminant = det;
	if (det == 0.0f) {
		D3DXMatrixIdentity(pOut);
		return pOut;
	}

	float rdet = 1.0f / det;

	pOut->_11 = ( s[5] * c10 - s[6] * c08 + s[7] * c07) * rdet;
	pOut->_12 = (-s[1] * c10 + s[2] * c08 - s[3] * c07) * rdet;
	pOut->_13 = ( s[13] * c04 - s[14] * c02 + s[15] * c01) * rdet;
	pOut->_14 = (-s[9]  * c04 + s[10] * c02 - s[11] * c01) * rdet;

	pOut->_21 = (-s[4] * c10 + s[6] * c09 - s[7] * c11) * rdet;
	pOut->_22 = ( s[0] * c10 - s[2] * c09 + s[3] * c11) * rdet;
	pOut->_23 = (-s[12] * c04 + s[14] * c03 - s[15] * c05) * rdet;
	pOut->_24 = ( s[8] * c04 - s[10] * c03 + s[11] * c05) * rdet;

	pOut->_31 = ( s[4] * c08 - s[5] * c09 + s[7] * c06) * rdet;
	pOut->_32 = (-s[0] * c08 + s[1] * c09 - s[3] * c06) * rdet;
	pOut->_33 = ( s[12] * c02 - s[13] * c03 + s[15] * c00) * rdet;
	pOut->_34 = (-s[8] * c02 + s[9] * c03 - s[11] * c00) * rdet;

	pOut->_41 = (-s[4] * c07 + s[5] * c06 - s[6] * c11) * rdet;
	pOut->_42 = ( s[0] * c07 - s[1] * c06 + s[2] * c11) * rdet;
	pOut->_43 = (-s[12] * c01 + s[13] * c05 - s[14] * c00) * rdet;
	pOut->_44 = ( s[8] * c01 - s[9] * c05 + s[10] * c00) * rdet;

	return pOut;
}

static inline D3DXVECTOR4 * __cdecl D3DXVec3Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR3 *pV, const D3DXMATRIX *pM)
{
	pOut->x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pM->_41;
	pOut->y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pM->_42;
	pOut->z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pM->_43;
	pOut->w = pV->x * pM->_14 + pV->y * pM->_24 + pV->z * pM->_34 + pM->_44;
	return pOut;
}

static inline D3DXVECTOR4 * __cdecl D3DXVec4Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV, const D3DXMATRIX *pM)
{
	D3DXVECTOR4 tmp;
	tmp.x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pV->w * pM->_41;
	tmp.y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pV->w * pM->_42;
	tmp.z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pV->w * pM->_43;
	tmp.w = pV->x * pM->_14 + pV->y * pM->_24 + pV->z * pM->_34 + pV->w * pM->_44;
	*pOut = tmp;
	return pOut;
}

static inline float __cdecl D3DXVec4Dot(const D3DXVECTOR4 *pA, const D3DXVECTOR4 *pB)
{
	return pA->x * pB->x + pA->y * pB->y + pA->z * pB->z + pA->w * pB->w;
}

static inline unsigned int __cdecl D3DXGetFVFVertexSize(unsigned int FVF)
{
	unsigned int vSize = 0;
	unsigned int pos = FVF & D3DFVF_POSITION_MASK;

	if (pos == D3DFVF_XYZRHW) {
		vSize += 4 * sizeof(float);
	} else if (pos & D3DFVF_XYZ) {
		vSize += 3 * sizeof(float);
		switch (pos) {
			case D3DFVF_XYZB1: vSize += 1 * sizeof(float); break;
			case D3DFVF_XYZB2: vSize += 2 * sizeof(float); break;
			case D3DFVF_XYZB3: vSize += 3 * sizeof(float); break;
			case D3DFVF_XYZB4: vSize += 4 * sizeof(float); break;
			default: break;
		}
	}

	if (FVF & D3DFVF_NORMAL)   vSize += 3 * sizeof(float);
	if (FVF & D3DFVF_PSIZE)    vSize += sizeof(float);
	if (FVF & D3DFVF_DIFFUSE)  vSize += sizeof(unsigned int);
	if (FVF & D3DFVF_SPECULAR) vSize += sizeof(unsigned int);

	unsigned int texCount = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	for (unsigned int i = 0; i < texCount && i < 4; i++) {
		unsigned int fmt = (FVF >> (16 + i * 2)) & 3;
		unsigned int comps = (fmt == D3DFVF_TEXTUREFORMAT2) ? 2 :
		                     (fmt == D3DFVF_TEXTUREFORMAT1) ? 1 :
		                     (fmt == D3DFVF_TEXTUREFORMAT3) ? 3 : 4;
		vSize += comps * sizeof(float);
	}

	return vSize;
}


static inline D3DXMATRIX operator * (const D3DXMATRIX & a, const D3DXMATRIX & b)
{
	D3DXMATRIX r;
	D3DXMatrixMultiply(&r, &a, &b);
	return r;
}

LPCSTR __cdecl D3DXGetErrorStringA(HRESULT hr);

/* Gamma ramp calibration flags (d3d8 enum was incomplete in some SDKs) */
#define D3DSGR_NO_CALIBRATION 0x00000000L
#define D3DSGR_CALIBRATE      0x00000001L

/* ---- texture / surface / shader helpers (stubs, see dx8compat.cpp) ---- */

HRESULT __cdecl D3DXCreateTexture(
	IDirect3DDevice8 *pDevice,
	unsigned int Width, unsigned int Height, unsigned int MipLevels,
	DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
	IDirect3DTexture8 **ppTexture);

HRESULT __cdecl D3DXCreateTextureFromFileExA(
	IDirect3DDevice8 *pDevice,
	LPCSTR pSrcFile,
	unsigned int Width, unsigned int Height, unsigned int MipLevels,
	DWORD Usage,
	D3DFORMAT Format, D3DPOOL Pool,
	DWORD Filter, DWORD MipFilter, DWORD ColorKey,
	void *pSrcInfo, void *pPalette,
	IDirect3DTexture8 **ppTexture);

HRESULT __cdecl D3DXFilterTexture(
	IDirect3DBaseTexture8 *pTexture,
	const RECT *pSrcRect,
	unsigned int SrcLevel,
	DWORD Filter);

HRESULT __cdecl D3DXLoadSurfaceFromSurface(
	IDirect3DSurface8 *pDestSurface,
	void *pDestPalette,
	const RECT *pDestRect,
	IDirect3DSurface8 *pSrcSurface,
	void *pSrcPalette,
	const RECT *pSrcRect,
	DWORD Filter,
	D3DCOLOR ColorKey);

HRESULT __cdecl D3DXAssembleShader(
	LPCSTR pSrcData,
	unsigned int SrcDataLen,
	DWORD Flags,
	const void *pDefines,
	D3DXBuffer **ppCompiledShader,
	D3DXBuffer **ppErrorMessages);

#ifdef __cplusplus
}

/* DX8-style buffered variant. Declared with C++ linkage because C linkage
** forbids overloading the name above. */
HRESULT __cdecl D3DXGetErrorStringA(HRESULT hr, LPSTR pBuffer, DWORD BufferLen);
#endif

#endif /* DX8COMPAT_H */
