/*
**	LZHL stub implementation on top of zlib (see CompLibHeader/lzhl.h for background).
**	Block format produced by LZHLCompress:
**		[UnsignedInt rawLen][UnsignedInt compLen][zlib deflate payload]
*/

#include <cstdlib>
#include <cstring>

#include "Lib/BaseType.h"
#include "CompLibHeader/lzhl.h"
// Direct prototypes instead of zlib.h: the game's BaseType.h "Byte" typedef clashes
// with zlib's own "Byte".
#define LZHL_Z_OK 0

extern "C" {
int compress2(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen, int level);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);
unsigned long compressBound(unsigned long sourceLen);
}

struct LZHL_Compressor
{
	int dummy;
};

struct LZHL_Decompressor
{
	int dummy;
};

static void WriteU32(UnsignedByte *p, UnsignedInt v)
{
	p[0] = (UnsignedByte)(v & 0xFF);
	p[1] = (UnsignedByte)((v >> 8) & 0xFF);
	p[2] = (UnsignedByte)((v >> 16) & 0xFF);
	p[3] = (UnsignedByte)((v >> 24) & 0xFF);
}

static UnsignedInt ReadU32(const UnsignedByte *p)
{
	return (UnsignedInt)p[0] | ((UnsignedInt)p[1] << 8) | ((UnsignedInt)p[2] << 16) | ((UnsignedInt)p[3] << 24);
}

extern "C" {

LZHL_CHANDLE LZHLCreateCompressor(void)
{
	return new LZHL_Compressor;
}

void LZHLDestroyCompressor(LZHL_CHANDLE handle)
{
	delete handle;
}

UnsignedInt LZHLCompress(LZHL_CHANDLE handle, void *dstVoid, const void *srcVoid, UnsignedInt srcLen)
{
	UnsignedByte *dst = (UnsignedByte *)dstVoid;
	const UnsignedByte *src = (const UnsignedByte *)srcVoid;

	if (!handle || !dst || (!src && srcLen))
		return 0;

	unsigned long compLen = compressBound(srcLen);
	if (compress2(dst + 8, &compLen, src, srcLen, 9) != LZHL_Z_OK)
		return 0;

	WriteU32(dst, srcLen);
	WriteU32(dst + 4, (UnsignedInt)compLen);
	return 8 + (UnsignedInt)compLen;
}

UnsignedInt LZHLCompressorCalcMaxBuf(UnsignedInt rawSize)
{
	// Worst case: rawSize blocks of 500000 plus deflate overhead.
	UnsignedInt blocks = rawSize / 500000 + 1;
	return rawSize + rawSize / 8 + 1024 + blocks * 64;
}

LZHL_DHANDLE LZHLCreateDecompressor(void)
{
	return new LZHL_Decompressor;
}

void LZHLDestroyDecompressor(LZHL_DHANDLE handle)
{
	delete handle;
}

Int LZHLDecompress(LZHL_DHANDLE handle, void *dstVoid, UnsignedInt *dstSize, const void *srcVoid, UnsignedInt *srcSize)
{
	UnsignedByte *dst = (UnsignedByte *)dstVoid;
	const UnsignedByte *src = (const UnsignedByte *)srcVoid;

	if (!handle || !dst || !src || !dstSize || !srcSize)
		return 0;

	if (*srcSize < 8)
		return 0;

	UnsignedInt rawLen = ReadU32(src);
	UnsignedInt compLen = ReadU32(src + 4);

	if (*srcSize < 8 + compLen || *dstSize < rawLen)
		return 0;

	unsigned long outLen = rawLen;
	if (uncompress(dst, &outLen, src + 8, compLen) != LZHL_Z_OK || outLen != rawLen)
		return 0;

	*srcSize -= 8 + compLen;
	*dstSize -= rawLen;
	return 1;
}

}
