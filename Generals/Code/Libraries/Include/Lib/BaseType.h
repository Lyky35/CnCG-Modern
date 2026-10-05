/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// FILE: BaseType.h ///////////////////////////////////////////////////////////
//
// Project:  RTS3
//
// Basic types and constants
// Author: Michael S. Booth, January 1995, September 2000
//
///////////////////////////////////////////////////////////////////////////////

// tell the compiler to only load this file once
#pragma once


#ifndef _BASE_TYPE_H_
#define _BASE_TYPE_H_

#include <math.h>
#include <string.h>

/*
**	Turn off some unneeded warnings.
**	Within the windows headers themselves, Microsoft has disabled the warnings 4290, 4514, 
**	4069, 4200, 4237, 4103, 4001, 4035, 4164. Makes you wonder, eh?
*/

// "unreferenced inline function has been removed" Yea, so what?
#pragma warning(disable : 4514)

// Unreferenced local function removed.
#pragma warning(disable : 4505)

// 'unreferenced formal parameter'
#pragma warning(disable : 4100)

// 'identifier was truncated to '255' characters in the browser information':
// Tempates create LLLOOONNNGGG identifiers!
#pragma warning(disable : 4786)

// 'function selected for automatic inline expansion'.  Cool, but since we're treating
// warnings as errors, don't warn me about this!
#pragma warning(disable : 4711)

#if 0
// 'assignment within condition expression'. actually a pretty useful warning, 
// but way too much existing code violates it.
//#pragma warning(disable : 4706)
#else
// actually, it turned out not to be too bad, so this is now ENABLED. (srj)
#pragma warning(error : 4706)
#endif

// 'conditional expression is constant'. used lots in debug builds.
#pragma warning(disable : 4127)

// 'nonstandard extension used : nameless struct/union'. MS headers violate this...
#pragma warning(disable : 4201)

// 'unreachable code'. STL violates this...
#pragma warning(disable : 4702)

// 'local variable is initialized but not referenced'. good thing to know about...
#pragma warning(error : 4189)

// 'unreferenced local variable'. good thing to know about...
#pragma warning(error : 4101)

#ifndef PI
#define PI     3.14159265359f
#define TWO_PI 6.28318530718f
#endif

#ifndef NULL
//#define NULL ((void *)0)
#define NULL 0						// C++ doesn't like casting void *'s into other pointers
#endif

// MSVC math.h defines overloaded functions with this name...
//#ifndef abs
//#define abs(x) (((x) < 0) ? -(x) : (x))
//#endif

#ifdef __cplusplus
// The legacy min/max macros corrupt C++ standard library headers (std headers use
// min/max as identifiers). In C++ we provide type-safe templated equivalents that
// accept mixed argument types, matching the macro's value semantics.
// MSVC's SEH-to-C++ translator hook used by the GameSpy threads; not available
// under MinGW, where asynchronous exceptions are disabled anyway: accept and ignore.
#ifndef _MSC_VER
#ifndef __BASETYPE_SEH_SHIM__
#define __BASETYPE_SEH_SHIM__
struct _EXCEPTION_POINTERS;
typedef void (*_se_translator_function)(unsigned int code, struct _EXCEPTION_POINTERS *info);
inline _se_translator_function _set_se_translator(_se_translator_function f) { (void)f; return 0; }
#endif
#endif


#ifndef WWLIB_MIXED_MINMAX_DEFINED
#define WWLIB_MIXED_MINMAX_DEFINED
#include <type_traits>

template <class A, class B>
inline typename std::common_type<A, B>::type min(A a, B b)
{
	return (a < b) ? a : b;
}

template <class A, class B>
inline typename std::common_type<A, B>::type max(A a, B b)
{
	return (a > b) ? a : b;
}
#endif
#else

#ifndef min
#define min(x,y) (((x)<(y)) ? (x) : (y))
#endif

#ifndef max
#define max(x,y) (((x)>(y)) ? (x) : (y))
#endif

#endif

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

//--------------------------------------------------------------------
// Fundamental type definitions
//--------------------------------------------------------------------
typedef float							Real;							// 4 bytes 
typedef int								Int;							// 4 bytes 
typedef unsigned int			UnsignedInt;	  	// 4 bytes 
typedef unsigned short		UnsignedShort;		// 2 bytes 
typedef short							Short;					  // 2 bytes 
typedef unsigned char			UnsignedByte;			// 1 byte		USED TO BE "Byte"
typedef char							Byte;							// 1 byte		USED TO BE "SignedByte"
typedef char							Char;							// 1 byte of text
typedef bool							Bool;							// 
// note, the types below should use "long long", but MSVC doesn't support it yet
typedef __int64						Int64;							// 8 bytes 
typedef unsigned __int64	UnsignedInt64;	  	// 8 bytes 

#include "Lib/trig.h"

//-----------------------------------------------------------------------------
#if defined(__GNUC__) && !defined(_MSC_VER)
// MSVC's wchar_t is 16-bit (UTF-16 code units), which the engine's string
// layer assumes. MinGW's wchar_t is 32-bit, so use unsigned short there.
typedef unsigned short WideChar;  ///< multi-byte character representations
#else
typedef wchar_t WideChar;  ///< multi-byte character representations
#endif

#if defined(__GNUC__) && !defined(_MSC_VER)
// The engine's string layer uses WideChar (16-bit UTF-16 code units) pointers with
// the standard wide-char helpers. MinGW's wchar_t is 32-bit, so provide overloads
// that operate on unsigned-short sequences.
#include <stddef.h>
static inline size_t wcslen(const WideChar* s)
{
	size_t n = 0;
	while (s[n]) ++n;
	return n;
}
static inline int wcscmp(const WideChar* a, const WideChar* b)
{
	while (*a && *a == *b) { ++a; ++b; }
	return (int)*a - (int)*b;
}
static inline int wcsncmp(const WideChar* a, const WideChar* b, size_t n)
{
	for (size_t i = 0; i < n; ++i) {
		if (!*a || *a != *b) return (int)*a - (int)*b;
		++a; ++b;
	}
	return 0;
}
static inline WideChar* wcscpy(WideChar* d, const WideChar* s)
{
	WideChar* r = d;
	while ((*d++ = *s++)) ;
	return r;
}
static inline WideChar* wcsncpy(WideChar* d, const WideChar* s, size_t n)
{
	WideChar* r = d;
	while (n && (*d++ = *s++)) --n;
	while (n--) *d++ = 0;
	return r;
}
static inline WideChar* wcscat(WideChar* d, const WideChar* s)
{
	WideChar* r = d;
	while (*d) ++d;
	while ((*d++ = *s++)) ;
	return r;
}
static inline int wcscmp(const WideChar* a, const wchar_t* b)
{
	for (;; ++a, ++b) {
		wchar_t ca = (wchar_t)*a;
		if (ca != *b) return (int)ca - (int)*b;
		if (!ca) return 0;
	}
}
static inline WideChar* wcsncpy(WideChar* d, const wchar_t* s, size_t n)
{
	size_t i = 0;
	for (; i < n && s[i]; ++i) d[i] = (WideChar)s[i];
	while (i < n) d[i++] = 0;
	return d;
}
static inline const WideChar* wcschr(const WideChar* s, WideChar c)
{
	while (*s && *s != c) ++s;
	return *s ? s : 0;
}
static inline WideChar* wcschr(WideChar* s, WideChar c)
{
	return (WideChar*)wcschr((const WideChar*)s, c);
}
static inline int ww_wcsicmp(const WideChar* a, const WideChar* b)
{
	for (;;) {
		WideChar ca = *a, cb = *b;
		if (ca >= 'A' && ca <= 'Z') ca += 32;
		if (cb >= 'A' && cb <= 'Z') cb += 32;
		if (ca != cb) return (int)ca - (int)cb;
		if (!ca) return 0;
		++a; ++b;
	}
}

static inline int wcsncmp_simple(const WideChar* a, const WideChar* b) { return wcscmp(a, b); }
#endif

//-----------------------------------------------------------------------------
template <typename NUM>
inline NUM sqr(NUM x)
{
	return x*x;
}

template <typename NUM>
inline NUM clamp(NUM lo, NUM val, NUM hi)
{
	if (val < lo) return lo;
	else if (val > hi) return hi;
	else return val;
}

template <typename NUM>
inline int sign(NUM x)
{
	if (x > 0) return 1;
	else if (x < 0) return -1;
	else return 0;
}

//-----------------------------------------------------------------------------
inline Real rad2deg(Real rad) { return rad * (180/PI); }
inline Real deg2rad(Real rad) { return rad * (PI/180); }

//-----------------------------------------------------------------------------
// For twiddling bits
//-----------------------------------------------------------------------------
// winnt.h (x86 intrinsics section) #defines BitTest to _bittest; make sure the
// game's macro wins regardless of include order.
#ifdef BitTest
#undef BitTest
#endif
#define WWBitTest( x, i ) ( ( (x) & (i) ) != 0 )
#define BitSet( x, i ) ( (x) |= (i) )
#define BitClear( x, i ) ( (x ) &= ~(i) )
#define BitToggle( x, i ) ( (x) ^= (i) )

//-------------------------------------------------------------------------------------------------

// note, this function depends on the cpu rounding mode, which we set to CHOP every frame, 
// but apparently tends to be left in unpredictable modes by various system bits of
// code, so use this function with caution -- it might not round in the way you want.
__forceinline long fast_float2long_round(float f)
{
	return static_cast<long>(f);
}

//-------------------------------------------------------------------------------------------------
#define REAL_TO_INT(x)						((Int)(x))
#define REAL_TO_UNSIGNEDINT(x)		((UnsignedInt)(x))
#define REAL_TO_SHORT(x)					((Short)(x))
#define REAL_TO_UNSIGNEDSHORT(x)	((UnsignedShort)(x))
#define REAL_TO_BYTE(x)						((Byte)(x))
#define REAL_TO_UNSIGNEDBYTE(x)		((UnsignedByte)(x))
#define REAL_TO_CHAR(x)						((Char)(x))
#define DOUBLE_TO_REAL(x)					((Real) (x))
#define DOUBLE_TO_INT(x)					((Int) (x))
#define INT_TO_REAL(x)						((Real) (x))

// once we've ceiled/floored, trunc and round are identical, and currently, round is faster... (srj)
#define REAL_TO_INT_CEIL(x)				(fast_float2long_round(ceilf(x)))
#define REAL_TO_INT_FLOOR(x)			(fast_float2long_round(floorf(x)))

//--------------------------------------------------------------------
// Derived type definitions
//--------------------------------------------------------------------

// NOTE: Keep these derived types simple, and avoid constructors and destructors
// so they can be used within unions.

// real-valued range defined by low and high values
struct RealRange 
{
	Real lo, hi;							// low and high values of the range

	// combine the given range with us such that we now encompass
	// both ranges
	void combine( RealRange &other )
	{
		lo = min( lo, other.lo );
		hi = max( hi, other.hi );
	}
};

struct Coord2D 
{
	Real x, y;

	Real length( void ) const { return (Real)sqrt( x*x + y*y ); }

	void normalize( void )
	{
		Real len = length();
		if( len != 0 )
		{
			x /= len;
			y /= len;
		}
	}
	
	Real toAngle( void );  ///< turn 2D vector into angle (where angle 0 is down the +x axis)

};

inline Real Coord2D::toAngle( void )
{
	Coord2D vector;

	vector.x = x;
	vector.y = y;

	Real dist = (Real)sqrt(vector.x * vector.x + vector.y * vector.y);

	// normalize
	if (dist == 0.0f)
		return 0.0f;

	Coord2D dir;
	dir.x = 1.0f;
	dir.y = 0.0f;

	Real distInv = 1.0f / dist;
	vector.x *= distInv;
	vector.y *= distInv;

	// dot of two unit vectors is cos of angle
	Real c = dir.x*vector.x + dir.y*vector.y;

	// bound it in case of numerical error
	if (c < -1.0)
		c = -1.0;
	else if (c > 1.0)
		c = 1.0;

	Real value = (Real)ACos( (Real)c );

	// Determine sign by checking Z component of dir cross vector
	// Note this is assumes 2D, and is identical to dotting the perpendicular of v with dir
	Real perpZ = dir.x * vector.y - dir.y * vector.x;
	if (perpZ < 0.0f)
		value = -value;

	// note: to make this 3D, 'dir' and 'vector' can be normalized and dotted just as they are
	// to test sign, compute N = dir X vector, then P = N x dir, then S = P . vector, where sign of
	// S is sign of angle - MSB

	return value;

}  // end toAngle

struct ICoord2D 
{
	Int x, y;

	Int length( void ) const { return (Int)sqrt( (double)(x*x + y*y) ); }
};

struct Region2D
{
	Coord2D lo, hi;						// bounds of 2D rectangular region

	Real width( void ) const { return hi.x - lo.x; }
	Real height( void ) const { return hi.y - lo.y; }
};

struct IRegion2D
{
	ICoord2D lo, hi;					// bounds of 2D rectangular region

	Int width( void ) const { return hi.x - lo.x; }
	Int height( void ) const { return hi.y - lo.y; }
};


struct Coord3D 
{
	Real x, y, z;

	Real length( void ) const { return (Real)sqrt( x*x + y*y + z*z ); }
	Real lengthSqr( void ) const { return ( x*x + y*y + z*z ); }

	void normalize( void )
	{
		Real len = length();

		if( len != 0 )
		{
			x /= len;
			y /= len;
			z /= len;
		}
	}
	
	static void crossProduct( const Coord3D *a, const Coord3D *b, Coord3D *r )
	{
		r->x = (a->y * b->z - a->z * b->y);
		r->y = (a->z * b->x - a->x * b->z);
		r->z = (a->x * b->y - a->y * b->x);
	}
	
	void zero( void )
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	void add( const Coord3D *a )
	{
		x += a->x;
		y += a->y;
		z += a->z;
	}
	
	void sub( const Coord3D *a )
	{
		x -= a->x;
		y -= a->y;
		z -= a->z;
	}
	
	void set( const Coord3D *a )
	{
		x = a->x;
		y = a->y;
		z = a->z;
	}
	
	void set( Real ax, Real ay, Real az )
	{
		x = ax;
		y = ay;
		z = az;
	}

	void scale( Real scale )
	{
		x *= scale;
		y *= scale;
		z *= scale;
	}

	Bool equals( const Coord3D &r )
	{
		return (x == r.x && 
						y == r.y &&
						z == r.z);
	}

	Bool operator==( const Coord3D &r )
	{
		return (x == r.x &&
						y == r.y &&
						z == r.z);
	}
};

struct ICoord3D 
{
	Int x, y, z;

	Int length( void ) const { return (Int)sqrt( (double)(x*x + y*y + z*z) ); }
	void zero( void )
	{

		x = 0;
		y = 0;
		z = 0;
	}
};

struct Region3D
{
	Coord3D lo, hi;						// axis-aligned bounding box

	Real width( void ) const { return hi.x - lo.x; }
	Real height( void ) const { return hi.y - lo.y; }
	Real depth( void ) const { return hi.z - lo.z; }

	void zero() { lo.zero(); hi.zero(); }
	Bool isInRegionNoZ( const Coord3D *query ) const
	{
		return (lo.x < query->x) && (query->x < hi.x) 
						&& (lo.y < query->y) && (query->y < hi.y);
	}
	Bool isInRegionWithZ( const Coord3D *query ) const
	{
		return (lo.x < query->x) && (query->x < hi.x) 
						&& (lo.y < query->y) && (query->y < hi.y)
						&& (lo.z < query->z) && (query->z < hi.z);
	}
};

struct IRegion3D
{
	ICoord3D lo, hi;					// axis-aligned bounding box

	Int width( void ) const { return hi.x - lo.x; }
	Int height( void ) const { return hi.y - lo.y; }
	Int depth( void ) const { return hi.z - lo.z; }
};


struct RGBColor
{
	Real red, green, blue;		// range between 0 and 1

	inline Int getAsInt() const
	{
		return
			((Int)(red * 255.0) << 16) |
			((Int)(green * 255.0) << 8) |
			((Int)(blue * 255.0) << 0);
	}

	inline void setFromInt(Int c)
	{
		red = ((c >> 16) & 0xff) / 255.0f;
		green = ((c >>  8) & 0xff) / 255.0f;
		blue = ((c >>  0) & 0xff) / 255.0f;
	}

};

struct RGBAColorReal
{

	Real red, green, blue, alpha;  // range between 0.0 and 1.0

};

struct RGBAColorInt
{

	UnsignedInt red, green, blue, alpha;  // range between 0 and 255

};

#endif // _BASE_TYPE_H_
