
#include "hef_utf8_str.h"
#include <stdio.h>
#include <cstdlib>
#include <assert.h>
#include <cstring>

namespace hef {

/**
	\defgroup GP_utf8_routines UTF-8 Functions
	...
	hf_Utf8DecodeToAny() \n
	hf_Utf8Decode16() \n
	hf_Utf8Decode32() \n
	hf_Utf8DecodeToWStr() \n
	hf_Utf8EncodeFromAny() \n
	hf_Utf8Encode16() \n
	hf_Utf8Encode32() \n
	hf_Utf8EncodeFromWStr() \n
	hf_Utf8StrPrintUPlusStyle() \n
*/

/// Decodes UTF-8 sequence returning decoded character as 32-bit value and
/// number of bytes consumed.
/// In case of an error result, only output value 'nBytes' can be used to attempt
/// to continue decoding remaining part of an input string.
bool hf_DecodeUtf8EncodedChar( const uint8_t* ptrFirstByte, size_t lenMax,
							  size_t* nBytes, size_t* nBits, uint32_t* uuChar, std::string& err )
{
	const uint8_t* ptr = ptrFirstByte;
	size_t i, nBytes2 = 0;
	uint32_t chr = 0, mask = 0;
	uint8_t ch2;
	char bfr[128];

	//hf_assert( lenMax > 0 );
	assert( lenMax > 0 );

	*nBytes = *nBits = 0;
	*uuChar = 0;
	err = "";

	// make sure leading byte is starting woth 11xxxxxx (in binary form).
	if(!(*ptr & 0x80) ){
		err = "Invalid bit Xxxxxxxx in leading byte, expected it to be 1.";
		return 0;
	}
	if(!(*ptr & 0x40) ){
		err = "Invalid bit xXxxxxxx in leading byte, expected it to be 1.";
		return 0;
	}
	size_t nFirstByteBits = 5;
	size_t nConsecutiveBytes = 1;
	for( i=0; i<4; i++, nConsecutiveBytes++, nFirstByteBits-- ){
		if( !(*ptr & (0x20 >> i)) )
			break;
	}
	//hf_assert( nFirstByteBits > 0 );
	assert( nFirstByteBits > 0 );
	*nBits = nFirstByteBits + nConsecutiveBytes * 6;
	if( nFirstByteBits == 1 ){
		// if first byte yelded only one bit, ensure that
		// next to last bit is 0.
		if( *ptr & 0x02 ){
			err = "Invalid bit xxxxxxXx in leading byte. Expected it to be 0.";
			return 0;
		}
	}
	*nBytes = nBytes2 = 1 + nConsecutiveBytes;
	if( nBytes2 > lenMax ){
		sprintf( bfr,"Not enough bytes in multibyte string, needed %u, were: %u.",
				(unsigned int)nBytes2, (unsigned int)lenMax );
		err = bfr;
		*nBytes = lenMax;
		return 0;
	}
	mask = 0;
	mask = ~mask;
	mask <<= nFirstByteBits;
	mask = ~mask;
	chr |= ((uint32_t)(*ptr)) & mask;

	for( i=1; i<lenMax && i<1+nConsecutiveBytes; i++ ){	//for each consecutive byte.
		ch2 = ptr[i];
		// make sure consecutive byte is starting woth 10xxxxxx (in binary form).
		if( !!(ch2 & 0x80) != 1 ){
			sprintf( bfr,"Invalid bit Xxxxxxxx in unicode sequence byte #%u. Expected it to be 1.", (unsigned int)i );
			err = bfr;
			return 0;
		}
		if( !!(ch2 & 0x40) != 0 ){
			sprintf( bfr,"Invalid bit xXxxxxxx in unicode sequence byte #%u. Expected it to be 0.", (unsigned int)i );
			err = bfr;
			return 0;
		}
		chr <<= 6;
		chr |= (ch2 & 0x3F);	// AND-ing it with 00111111 (binary) (6 least significiant bits set).
	}
	// maximum expected value for unicode character: 0x7FFFFFFF.
	if( chr > 0x7FFFFFFF ){
		// 5*6+1=31 // 1111 1111 1111 1111 1111 1111 1111 111
		// 4*6+2=26
		sprintf( bfr,"unicode sequence yielded value greater than 0x7FFFFFFF." );
		err = bfr;
		return 0;
	}
	*uuChar = chr;
	return 1;
}

/// String prints 32-bit characters array using ANSI characters when value is <= 127 and
/// prints others by using (U+NNNNN) notation, where NNNNN is replaced by hexadecimal
/// unicode character value.
/// \sa GP_utf8_routines
std::string hf_Utf8StrPrintUPlusStyle( const uint32_t* in, int len )
{
	if( len < 0 )
		len = _hf_utf8_strlen(in);
	size_t i, len2 = (size_t)len;
	std::string strX;
	uint32_t chr; char bfr[64];
	for( i=0; i<len2; i++ ){
		chr = in[i];
		if( chr <= 127 ){
			strX += (char)chr;
		}else{
			sprintf(bfr,"(U+%X)", (unsigned int)chr );
			strX += bfr;
		}
	}
	return strX;
}

/// \cond DOXYGEN_SKIP

/// Decodes UTF-8 c-string (aka. multibyte, null terminated string) by calling
/// HfCUCSink::newCharacter() for each consecutive character.
/// Call is done for eavery character, this includes ANSI compatible characters as well.
/// In case of encountered errors in encoding, HfCUCSink::newCharacter() is called with
/// HfSTravesalChar::nBytes member containing actual number of bytes that were omited.
/// References:
/// * http://en.wikipedia.org/wiki/UTF-8
/// * "rfc2044 - UTF-8, a transformation format of Unicode and ISO 10646.txt"
bool hf_Utf8UniversalDecode( const char* in, int length, HfCUCSink& cUCSink )
{
	if( length < 0 )
		length = strlen(in);
	size_t i, length2 = (size_t)length;
	const unsigned char* in2 = (const unsigned char*)in;
	unsigned char ch;
	std::string err;
	size_t nBytes2 = 0, nBits = 0;
	bool rs, retv = 1;
	//
	for( i=0; i<length2; ){
		ch = in2[i];
		if( ch & 0x80 ){ //if start of unicode sequence, i.e. most sgnfcnt bit in ASCII byte is set.
			uint32_t chr = 0;
			nBytes2 = 0;
			rs = hf_DecodeUtf8EncodedChar( &in2[i], length2-i, &nBytes2, &nBits, &chr, err );
			if(!rs){
				HfSTravesalChar chr2;
				chr2.bValid  = 0;
				chr2.szError = err.c_str();
				chr2.nBytes  = nBytes2;
				chr2.szData  = (const char*)&in2[i];
				if(!cUCSink.newCharacter( chr2 ))
					break;
				i += std::max<size_t>( nBytes2, 1 );
			}else{
				HfSTravesalChar chr3;
				chr3.bValid = 1;
				chr3.uValue = chr;
				chr3.nBits  = nBits;
				chr3.nBytes = nBytes2;
				chr3.szData = (const char*)&in2[i];
				if(!cUCSink.newCharacter( chr3 ))
					break;
				i += nBytes2;
			}
		}else{
			HfSTravesalChar chr4;
			chr4.bValid = 1;
			chr4.uValue = ch;
			chr4.nBits  = 7;
			chr4.nBytes = 1;
			chr4.szData = (const char*)&in2[i];
			if(!cUCSink.newCharacter( chr4 ))
				break;
			i++;
		}
	}
	return retv;
}

std::basic_string<uint16_t> hf_Utf8_MbcsToWcharU16( const char* in )
{
	std::basic_string<uint16_t> z;
	for(; *in; in++ )
		z += *in;
	return z;
}

class HfCUCSink16 : public HfCUCSink {
public:
	std::basic_string<uint16_t>* outp2;
	size_t flags2;
	char bfr[64];
	virtual bool newCharacter( const HfSTravesalChar& chr ){
		if( chr.bValid ){
			if( chr.nBits <= 16 || chr.uValue == (uint16_t)chr.uValue ){
				outp2->push_back( (uint16_t) chr.uValue );
			}else{
				if( flags2 & HFE_UTF8DF_UseUPlusInsertion ){
					//sprintf(bfr,"(U+%X)", (long) chr.uValue );
					//*outp2 += hf_Utf8_MbcsToWcharU16(bfr);
					*outp2 += hf_Utf8_MbcsToWcharU16(hf_Utf8StrPrintUPlusStyle( &chr.uValue, 1 ).c_str());
				}else{
					// Unicode replacement character.
					// http://www.unicode.org/charts/PDF/UFFF0.pdf
					// 0xFFFD = qustion mark inside a diamond.
					//          (REPLACEMENT CHARACTER).
					outp2->push_back(0xFFFD);
				}
			}
		}else{
			if(!( flags2 & HFE_UTF8DF_ContinueOnError ) )
				return 0;
			outp2->push_back(0xFFFD);
		}
		return 1;
	}
};

class HfCUCSink32 : public HfCUCSink {
public:
	std::basic_string<uint32_t>* outp3;
	size_t flags3;
	virtual bool newCharacter( const HfSTravesalChar& chr ){
		if( chr.bValid ){
			outp3->push_back( chr.uValue );
		}else{
			if(!( flags3 & HFE_UTF8DF_ContinueOnError ) )
				return 0;
			outp3->push_back('?');
		}
		return 1;
	}
};

void hf_Utf8UniversalEncode( const void* chars, bool b32Bit,
							size_t len, std::string* outp, size_t* lengthOut )
{
	size_t i;
	uint32_t chr;
	outp->clear();
	std::vector<uint8_t> data; std::vector<uint8_t>::const_iterator a;
	for( i=0; i<len; i++ ){
		data.clear();
		if( b32Bit ){
			chr = ((uint32_t*)chars)[i];
		}else{
			chr = ((uint16_t*)chars)[i];
		}
		if(0){
		}else if( chr <= 0x7F ){		//1-byte // 0x7F = 127
			data.assign( 1, (uint8_t)chr );
		}else if( chr <= 0x000007FF ){	//2-bytes // 110xxxxx 10xxxxxx
			data.assign( 2, 0 );
			data[1] = 0x80 | ( ((uint8_t)(chr>>0)) & 0x3F );	//0x3F = 00111111 (binary)
			data[0] = 0xC0 | ( ((uint8_t)(chr>>6)) & 0x1F );	//0x1F = 00011111 (binary)
		}else if( chr <= 0x0000FFFF ){	//3-bytes
			data.assign( 3, 0 );
			data[2] = 0x80 | ( ((uint8_t)(chr>>0 )) & 0x3F );
			data[1] = 0x80 | ( ((uint8_t)(chr>>6 )) & 0x3F );
			data[0] = 0xE0 | ( ((uint8_t)(chr>>12)) & 0x0F );
		}else if( chr <= 0x001FFFFF ){	//4-bytes
			data.assign( 4, 0 );
			data[3] = 0x80 | ( ((uint8_t)(chr>>0 )) & 0x3F );
			data[2] = 0x80 | ( ((uint8_t)(chr>>6 )) & 0x3F );
			data[1] = 0x80 | ( ((uint8_t)(chr>>12)) & 0x3F );
			data[0] = 0xF0 | ( ((uint8_t)(chr>>18)) & 0x07 );	//0x07 = 00000111
		}else if( chr <= 0x03FFFFFF ){	//5-bytes
			data.assign( 5, 0 );
			data[4] = 0x80 | ( ((uint8_t)(chr>>0 )) & 0x3F );
			data[3] = 0x80 | ( ((uint8_t)(chr>>6 )) & 0x3F );
			data[2] = 0x80 | ( ((uint8_t)(chr>>12)) & 0x3F );
			data[1] = 0x80 | ( ((uint8_t)(chr>>18)) & 0x3F );
			data[0] = 0xF8 | ( ((uint8_t)(chr>>24)) & 0x03 );	//0xF8=11111000
		}else if( chr <= 0x7FFFFFFF ){	//6-bytes
			data.assign( 6, 0 );
			data[5] = 0x80 | ( ((uint8_t)(chr>>0 )) & 0x3F );
			data[4] = 0x80 | ( ((uint8_t)(chr>>6 )) & 0x3F );
			data[3] = 0x80 | ( ((uint8_t)(chr>>12)) & 0x3F );
			data[2] = 0x80 | ( ((uint8_t)(chr>>18)) & 0x3F );
			data[1] = 0x80 | ( ((uint8_t)(chr>>24)) & 0x3F );
			data[0] = 0xFC | ( ((uint8_t)(chr>>30)) & 0x01 );	//0xFC=11111100
		}else{
			// error, unicode value is too large for what UTF-8 can support.
			data.assign( 1, (uint8_t)'?' );
		}
		if(outp){
			for( a = data.begin(); a != data.end(); ++a )
				*outp += (char)*a;
		}
		if(lengthOut)
			*lengthOut += data.size();
	}
}
/// \endcond //DOXYGEN_SKIP

/// Decodes to unsigned 16-bit string, aka. wide-char (wchar_t on Windows) string, aka. STD-W-String.
/// \param length - may be specified -1 to indicate null-terminated input.
/// \param flags2 - flags, f.e. HFE_UTF8DF_ContinueOnError.
/// \sa GP_utf8_routines
void hf_Utf8Decode16( const char* in, int length, std::basic_string<uint16_t>& outp, size_t flags2 )
{
	HfCUCSink16 sink16;
	sink16.outp2 = &outp;
	sink16.flags2 = flags2;
	hf_Utf8UniversalDecode( in, length, sink16 );
}
void hf_Utf8Decode32( const char* in, int length, std::basic_string<uint32_t>& outp, size_t flags2 )
{
	HfCUCSink32 sink32;
	sink32.outp3 = &outp;
	sink32.flags3 = flags2;
	hf_Utf8UniversalDecode( in, length, sink32 );
}

std::basic_string<uint16_t> hf_Utf8DecodeToWStr( const char* in, int length, size_t flags2 )
{
	std::basic_string<uint16_t> outp;
	hf_Utf8Decode16( in, length, outp, flags2 );
	return outp;
}

void hf_Utf8Encode16( const uint16_t* chars, int len, std::string* outp, size_t* lengthOut )
{
	if( len < 0 )
		len = _hf_utf8_strlen(chars);
	size_t len2 = (size_t)len;
	hf_Utf8UniversalEncode( chars, 0, len2, outp, lengthOut );
}
/**
	Encode from unicode array or C-string.
	\param chars - data to encode from, 32-bit characters.
	\param len   - optional, number of unicode characters, -1 to indicate null terminated.
	\param outp  - output result of encoding. optional, if set to null,
	               result is not stored, instead, length parameter ('lengthOut')
	               can be used to determine length of encoding alone.
	\param lengthOut - optional, output length in 8-bit characters.
*/
void hf_Utf8Encode32( const uint32_t* chars, int len, std::string* outp, size_t* lengthOut )
{
	if( len < 0 )
		len = _hf_utf8_strlen(chars);
	size_t len2 = (size_t)len;
	//outp.clear();
	hf_Utf8UniversalEncode( chars, 1, len2, outp, lengthOut );
}
std::string hf_Utf8EncodeFromWStr( const uint16_t* chars, int len )
{
	std::string z;
	hf_Utf8Encode16( chars, len, &z, 0 );
	return z;
}

} // end namespace hef
