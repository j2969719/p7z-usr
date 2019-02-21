
#ifndef HEF_UTF8_STR_H_
#define HEF_UTF8_STR_H_

#ifdef _MSC_VER
#	pragma warning(disable:4786)
#endif

#include <vector>
#include <string>
#include <stdint.h>

namespace hef {

class HfCUCSink;

std::basic_string<uint16_t> hf_Utf8DecodeToWStr( const char* in, int length = -1, size_t flags2 = 0 );
void                       hf_Utf8Decode16( const char* in, int length, std::basic_string<uint16_t>& outp, size_t flags2 = 0 );
void                       hf_Utf8Decode32( const char* in, int length, std::basic_string<uint32_t>& outp, size_t flags2 = 0 );
std::string                hf_Utf8EncodeFromWStr( const uint16_t* chars, int len = -1 );
void                       hf_Utf8Encode16( const uint16_t* chars, int len, std::string* outp = 0, size_t* lengthOut = 0 );
void                       hf_Utf8Encode32( const uint32_t* chars, int len, std::string* outp, size_t* lengthOut );
std::string                hf_Utf8StrPrintUPlusStyle( const uint32_t* in, int len = -1 );

/// Flags for 'decode' functions, f.e. hf_Utf8Decode16().
enum {
	/// Normally decodings stop if there were errors encountered and remaining
	/// string portion is discarded. Setting this flag causes conversion to continue and
	/// invalid characters are replaced with placeholder character.
	HFE_UTF8DF_ContinueOnError = 0x1,
	/// Characters that are made out of more than 16 bits are
	/// replaced with (U+NNNNN) notation, where NNNNN stands for unicode hexadecimal
	/// value. Valid for 16-bit decoding.
	HFE_UTF8DF_UseUPlusInsertion = 0x2,
};

/// \cond DOXYGEN_SKIP  // tag_wUulEO5m2IU{

bool                       hf_Utf8UniversalDecode( const char* in, int length, HfCUCSink& cUCSink );
std::basic_string<uint16_t> hf_Utf8_MbcsToWcharU16( const char* in );
void                       hf_Utf8UniversalEncode( const void* chars, bool b32Bit, size_t len, std::string* outp = 0, size_t* lengthOut = 0 );

/// Struct used intarnally with hf_Utf8UniversalDecode().
struct HfSTravesalChar {
	bool      bValid;
	uint32_t   uValue;
	size_t    nBits;
	size_t    nBytes;
	const char* szError;
	const char* szData;
	HfSTravesalChar() : bValid(0), uValue(0), nBits(0), nBytes(0), szError(0), szData(0) {}
};
/// Class used intarnally with hf_Utf8UniversalDecode().
class HfCUCSink {
public:
	virtual bool newCharacter( const HfSTravesalChar& chr ) = 0;
};

/// Determines length of c-string (null-terminated string).
template<class T>
uint32_t _hf_utf8_strlen( const T* in )
{
	const T* sz = in;
	for(; *sz; sz++ );
	return sz - in;
}

/// \endcond //DOXYGEN_SKIP  // }tag_wUulEO5m2IU

template <class T>
void hf_Utf8DecodeToAny( const char* inp, int length, T& outp, size_t flags2 )
{
	std::basic_string<uint32_t>::const_iterator a;
	std::basic_string<uint32_t> str;
	outp.clear();
	hf_Utf8Decode32( inp, length, str, flags2 );
	for( a = str.begin(); a != str.end(); ++a ){
		outp.push_back( (typename T::value_type)( *a ) );
	}
}
template <class T>
void hf_Utf8EncodeFromAny( const T* chars, int len, std::string* outp, size_t* lengthOut )
{
	std::vector<uint32_t> str;
	len = ( len == -1 ? _hf_utf8_strlen(chars) : len );
	for( int i=0; i != len; i++ ){
		str.push_back( (uint32_t)( chars[i] ) );
	}
	hf_Utf8Encode32( &str[0], (int)str.size(), outp, lengthOut );
}


} // end namespace hef

#endif //HEF_UTF8_STR_H_
