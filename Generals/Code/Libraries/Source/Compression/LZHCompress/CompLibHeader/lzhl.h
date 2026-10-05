/*
**	LZHL replacement layer.
**	The original Nox LZH compression library (CompLibSource) was not present in the
**	public EA source dump. This header provides the same call surface, implemented
**	on top of zlib deflate (see LZHCompress/lzhl_stub.cpp). It is self-consistent:
**	data compressed with this layer decompresses with it. Shipped Generals/Zero Hour
**	assets do not use the LZHL codec, so byte-compatibility with the original
**	library is not required for the game to run.
*/
#ifndef __LZHL_H
#define __LZHL_H

#include "Lib/BaseType.h"

typedef struct LZHL_Compressor* LZHL_CHANDLE;
typedef struct LZHL_Decompressor* LZHL_DHANDLE;

#ifdef __cplusplus
extern "C" {
#endif

LZHL_CHANDLE LZHLCreateCompressor(void);
void         LZHLDestroyCompressor(LZHL_CHANDLE handle);
UnsignedInt  LZHLCompress(LZHL_CHANDLE handle, void *dst, const void *src, UnsignedInt srcLen);
UnsignedInt  LZHLCompressorCalcMaxBuf(UnsignedInt rawSize);

LZHL_DHANDLE LZHLCreateDecompressor(void);
void         LZHLDestroyDecompressor(LZHL_DHANDLE handle);
Int          LZHLDecompress(LZHL_DHANDLE handle, void *dst, UnsignedInt *dstSize, const void *src, UnsignedInt *srcSize);

#ifdef __cplusplus
}
#endif

#endif
