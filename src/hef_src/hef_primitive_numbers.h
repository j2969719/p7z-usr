
#ifndef _HEF_PRIMITIVE_NUMBERS_H_
#define _HEF_PRIMITIVE_NUMBERS_H_
namespace hef{
;
#ifdef _MSC_VER
#	pragma warning ( disable: 4786 )
#endif //_MSC_VER

// TODO: replace typedefs with ones from "inttypes.h".
//       definitions incorrect on some platforms, fe. x86_64

//typedef float          F32;
//typedef char           S08;
//typedef unsigned char  U08;
//typedef short          S16;
//typedef unsigned short U16;
//typedef int            S32;
//typedef unsigned int   U32;
#ifdef _MSC_VER
//	typedef __int64          S64;
//	typedef unsigned __int64 U64;
#else
	// ref:
	//     Integer sizes in C on 32 bit and 64 bit Linux
	//     https://usrmisc.wordpress.com/2012/12/27/integer-sizes-in-c-on-32-bit-and-64-bit-linux/
//	typedef long long          S64;
//	typedef unsigned long long U64;
#endif //_MSC_VER

// TODO: remove min/max fncs, accessing std ones should be sufficient.

template<class T> T hf_min( const T& a, const T& b )
{
	return a < b ? a : b;
}
template<class T> T hf_max( const T& a, const T& b )
{
	return a < b ? b : a;
}
/// Returns true if 'a' is inside inclusive range beetwen 'low' and 'high'.
template<class T>
bool hf_isInRangeInclusive( T a, T low, T high )
{
	return a >= low && a <= high;
}

//hf_assert()
//assert2hf()


} // end namespace hef

#endif //_HEF_PRIMITIVE_NUMBERS_H_
