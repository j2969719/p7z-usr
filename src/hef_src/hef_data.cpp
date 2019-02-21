
#include "hef_data.h"
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <cstring>
#ifdef __GNUC__
#	include <sys/time.h>		//gettimeofday()
#endif

namespace hef{
;
/// \cond DOXYGEN_SKIP

#ifdef HEF_DEBUG //[XwCnFprPZlQA]
;
void data_general_tester()
{
	printf("\n""data_general_tester()\n");
	{
		printf("\n""TEST_01 string to hex encoiding.\n");
		const char* data = "abc_123\t""456";
		size_t len = hf_strlen(data);
		std::string strHex = hf_hexEncode( data, len );
		printf("data string: %s\n", data );
		printf("data length: %u\n", (unsigned int)len );
		printf("hex-encoded: %s\n", strHex.c_str() );
		std::vector<uint8_t> dataDec = hf_hexDecode( strHex.c_str(), -1 );
		printf("decoded length: %u\n", (unsigned int)dataDec.size() );
		std::vector<uint8_t> dataDec2 = dataDec;
		dataDec2.push_back(0);
		printf("decoded back as str: %s\n", &dataDec2[0] );
	}
	{
		printf("\n""TEST_05 hf_StrPrintAny().\n");
		std::string str = hf_StrPrintAny<int64_t>( -5000000201, 10, 3, ",", 0, 0 );
		printf("result 1: %s\n", str.c_str() );

		str = hf_StrPrintAny<uint64_t>( 5000000301, 10, 0, "", 16, '0' );
		printf("result 2: %s\n", str.c_str() );

		str = hf_StrPrintAny<uint64_t>( 5000000301, 16 );	//5000000301->0x12A05F32D
		printf("result 3: %s\n", str.c_str() );

		uint32_t a = 0xFFFFFFFF;
		str = hf_StrPrintAny<uint32_t>( a, 10 );
		printf("result 4: %s\n", str.c_str() );

		str = hf_StrPrintAny<uint32_t>( a, 2 );
		printf("result 5: %s, strsize: %u\n", str.c_str(), (unsigned int)str.size() );

		str = hf_StrPrintAny<uint32_t>( a, 32 );
		printf("result 6a: %s, strsize: %u\n", str.c_str(), (unsigned int)str.size() );
		printf("result 6b: %s\n",
			hf_StrPrintAny<uint64_t>( hf_AsciiToU64( str.c_str(), 32 ), 10 ).c_str() );
		printf("result 6c: %s\n",
			hf_StrPrintAny<uint64_t>( hf_AsciiToU64( str.c_str(), 32 ), 16, 2,"," ).c_str() );
	}
	{
		printf("\n""TEST_06 hf_AsciiToU64() with 64 bit with base >16 value.\n");
		uint64_t a = 5000000000;
		std::string strBase32 = hf_StrPrintAny<uint64_t>( a, 32 );
		printf("result base32 1x: %s\n", strBase32.c_str() );
		printf("result base32 2x: %s\n",
			hf_StrPrintAny<uint64_t>( hf_AsciiToU64( strBase32.c_str(), 32 ), 32 ).c_str() );
		printf("result base10 2x: %s\n",
			hf_StrPrintAny<uint64_t>( hf_AsciiToU64( strBase32.c_str(), 32 ), 10 ).c_str() );
		printf("result base16 2x: %s\n",
			hf_StrPrintAny<uint64_t>( hf_AsciiToU64( strBase32.c_str(), 32 ), 16 ).c_str() );
	}
}
#endif //HEF_DEBUG //[XwCnFprPZlQA]
/// \endcond //DOXYGEN_SKIP
;

/**
	Converts data to hex-view multiline string.
	For example:
	\code
		const char* data = "abcdefgh\r\n11111122222222333";
		//  result:
		//    61 62 63 64 65 66 67 68 abcdefgh
		//    0D 0A 31 31 31 31 31 31 ..111111
		//    32 32 32 32 32 32 32 32 22222222
		//    33 33 33 .. .. .. .. .. 333
	\endcode
	\sa GP_hex_encoding
*/
std::string
hf_dataToHexViewAsLines( const void* data, size_t size, size_t uLineLen,
                        const char* szNewline, const char* szEachLinePrefix,
                        const char szFillerHex[3], const char szFillerChar[2] )
{
	std::string z;
	const uint8_t* data2 = (uint8_t*)data;
	size_t i=0, nLnLenActual=0, k=0;
	size_t numLines = size / uLineLen + !!( size % uLineLen );
	char bfr[16]; char chr;
	for( i=0; i<numLines; i++ ){
		nLnLenActual = uLineLen;
		if( i+1 == numLines ){	//if last line.
			nLnLenActual = size - i * uLineLen;
		}
		z += szEachLinePrefix;
		// show hex-encoded stirng part.
		for( k=0; k<uLineLen; k++ ){
			if( k < nLnLenActual ){
				chr = data2[ i * uLineLen + k ];
				sprintf( bfr, "%02X ", (unsigned char)chr );
				z += bfr;
			}else{
				z += szFillerHex;	//".."
				z += " ";
			}
		}
		// show ASCII stirng part.
		for( k=0; k<uLineLen; k++ ){
			if( k < nLnLenActual ){
				chr = data2[ i * uLineLen + k ];
				if( isprint(chr) ){
					z += chr;
				}else{
					z += '.';
				}
			}else{
				z += szFillerChar;	//" "
			}
		}
		z += szNewline;
	}
	return z;
}

/// Converts data to ASCII c-string with new line characters shown (\\r, \\t and \\n).
/// \sa GP_hex_encoding
std::string hf_dataToAsciiNlSpec( const uint8_t* data, size_t size )
{
	std::string z;
	size_t i; char c;
	for( i=0; i<size; i++ ){
		c = (char) data[i];
		if( isprint( c ) ){
			z += c;
		}else if( c=='\r' || c=='\n' || c=='\t' ){
			z += c;
		}else{
			z += '.';
		}
	}
	return z;
}
/// Converts c-string to ASCII string with new line characters shown (\\r and \\n).
/// \sa GP_hex_encoding
std::string hf_strWithCrLfShown( const char* in, bool bInclLineBreaks )
{
	std::string z;

	for(; *in; in++ ){
		switch(*in){
		case '\r':
			z += "\\r";
			break;
		case '\n':
			z += "\\n";
			if(bInclLineBreaks)
				z += '\n';
			break;
		default:
			z += *in;
			break;
		}
	}
	return z;
}

size_t hf_ConvertBytesArrayToU32LE( void* addrArray4 )
{
	uint8_t* addr = (uint8_t*)addrArray4;
	size_t a = 0;
	size_t b0 = addr[0];
	size_t b1 = addr[1];
	size_t b2 = addr[2];
	size_t b3 = addr[3];
	a |= (b0 << 0);
	a |= (b1 << 8);
	a |= (b2 << 16);
	a |= (b3 << 24);
	return a;
}

/**
	\defgroup GP_rw_file_contents Read-Write File contents functions.
	...
	hf_ReadFileU32LE() \n
	hf_ReadFileBytes() \n
	hf_WriteFileBytes() \n
	hf_PutFileBytes() \n
	hf_GetFileStrContents64() \n
	hf_ReadFileBytes64() \n
*/
;

/// \sa GP_rw_file_contents
size_t hf_ReadFileU32LE( FILE* fp, size_t addrInFile )
{
	fseek( fp, (size_t)addrInFile, SEEK_SET );
	uint8_t* bfr[4] = {0,0,0,0,};
	//size_t readd = (size_t)fread( bfr, 1, 4, fp );
	size_t readd = fread( bfr, 1, 4, fp );
	readd=readd;
	size_t a = hf_ConvertBytesArrayToU32LE( bfr );
	return a;
}
/// \sa GP_rw_file_contents
std::vector<uint8_t> hf_ReadFileBytes( FILE* fp, size_t addrInFile, size_t numBytes )
{
	std::vector<uint8_t> out;
	size_t i;
	for( i=0; i<numBytes; i++ )
		out.push_back('W');
	if(fseek( fp, (size_t)addrInFile, SEEK_SET ))
		return out;
	//size_t readd = (size_t)fread( &out[0], 1, numBytes, fp );
	size_t readd = fread( &out[0], 1, numBytes, fp );
	readd = readd;
	return out;
}
/// \sa GP_rw_file_contents
bool hf_WriteFileBytes( FILE* fp, size_t addrInFile, const void* bytes, size_t size )
{
	if( fseek( fp, (size_t)addrInFile, SEEK_SET ) )
		return 0;
	size_t written = (size_t) fwrite( bytes, 1, size, fp );
	if( !written || written != size )
		return 0;
	return 1;
}
/// Writes file bytes by either replacing entire contents or appending data at end of file.
/// \param fn - File name.
/// \param bytes - Pointer to data.
/// \param size - Size of data in bytes. If set to -1 then data is assumed to be the C-string.
/// \param flags - See \ref HF_EPFBF_NOFLAGS.
/// \sa GP_rw_file_contents
bool hf_PutFileBytes( const char* fn, const void* bytes, int size, size_t flags )
{
	FILE* fp2 = 0;
	if( flags & HF_EPFBF_APPEND ){
		fp2 = fopen( fn, "ab" );
	}else{
		fp2 = fopen( fn, "wb" );
	}
	if(fp2){
		fseek( fp2, 0, SEEK_END );
		if( size == -1 )
			size = strlen( (char*)bytes );
		fwrite( bytes, size, 1, fp2 );
		fclose(fp2);
		return 1;
	}
	return 0;
}
/**
	\defgroup GP_strprint_value Value-String converting
	...
	hf_StrPrintU64() \n
	hf_StrPrintS64() \n
	hf_AsciiToU64() \n
	hf_StrPrintAny() \n
	hf_convCompilerDateYMD() \n
	HfTArgs::toUint64() \n
	HfTArgs::toInt64() \n
	HfTArgs::arg() (3+ methods) \n
	HfTArgs::arg2() \n
	HfTArgs::arg3() \n
	HfTArgs::arg(const T*) \n

*/
;
/// \sa GP_strprint_value
std::string
hf_StrPrintU64( uint64_t inp, uint32_t uBase, uint32_t uDigitGrouping, const char* szGlue,
               size_t uLeadingCharsCount, char leading_char )
{
	char bfr[128];
	int digit_grouping = (int)uDigitGrouping;
	digit_grouping = digit_grouping ? digit_grouping : 3;
	szGlue = (szGlue ? szGlue : "");

	_hf_sprintUT<uint64_t>( inp, bfr, szGlue, uBase, digit_grouping,
				uLeadingCharsCount,
				leading_char );
	if( !uDigitGrouping ){
		// remove any separators ('szGlue') in default, 3-character
		// digit grouping performed.
		std::vector<std::string> pcs;
		hf_explode( bfr, szGlue, pcs );
		return hf_implode( pcs, "" );
	}
	return bfr;

/*	/// \todo Error in code, 'uBase' parameter is not used. i.e. always is assumed 10.
	/// implement _hf_sprintUT() similarrly to how its done in hf_StrPrintS64().
	std::string out;
	uint64_t a = inp;
	size_t i;
	char bfr1[64];
	for( i=0; i<sizeof(a); i++ ){
		//_hf_sprintUT()
		int reminder = (int)(uint64_t)( a % 1000 );
		a /= 1000;
		if(a){
			sprintf( bfr1, "%03d", reminder );
		}else{
			sprintf( bfr1, "%d", reminder );
		}
		out.insert( 0, bfr1 );
		if(!a)
			break;
	}
	if(uDigitGrouping){
		int spliteach = (int)uDigitGrouping;
		HfCStrPiece<char>::LsStrPiece lsSp;
		lsSp = HfCStrPiece<char>(out.c_str()).split( -spliteach );
		out = HfCStrPiece<char>::implode( lsSp, glue );
	}
	if( uLeadingCharsCount && leading_char && out.size() < uLeadingCharsCount ){
		size_t diff = uLeadingCharsCount - out.size();
		out.insert( out.begin(), diff, leading_char );
	}
	return out;
	//*/
}

/// \sa GP_strprint_value
std::string hf_StrPrintS64( int64_t inp, uint32_t base, uint32_t uDigitGrouping,
                           const char* szGlue, size_t uLeadingCharsCount,
                           char leading_char )
{
	char bfr[128];
	szGlue = ( szGlue ? szGlue : "" );
	int digit_grouping = (int)uDigitGrouping;
	digit_grouping = digit_grouping ? digit_grouping : 3;
	bool bMinus = inp < 0;
	int64_t val2 = (int64_t) (bMinus ? inp * -1 : inp);
	_hf_sprintUT<int64_t>( val2, bfr,
			( uDigitGrouping ? szGlue : "" ),
			base, digit_grouping,
			((bMinus && uLeadingCharsCount) ? uLeadingCharsCount-1: uLeadingCharsCount),
			leading_char );
	std::string str = bfr;
	return ( bMinus ? "-" + str : str );
}

/**
	\defgroup GP_hex_encoding Hex encoding etc. functions.
	...
	hf_hexEncode() \n
	hf_hexDecode() \n
	hf_dataToHexViewAsLines() \n
	hf_dataToAsciiNlSpec() \n
	hf_strWithCrLfShown() \n
*/
;

/// \sa GP_hex_encoding
std::string hf_hexEncode( const void* data, uint32_t size )
{
	const uint8_t* data2 = (uint8_t*) data;
	std::string z; uint8_t chr; char bfr[32];
	for( uint32_t i=0; i<size; i++ ){
		chr = data2[i];
		sprintf( bfr, "%02X", (int)chr );
		z += bfr;
	}
	return z;
}
/// \sa GP_hex_encoding
std::vector<uint8_t> hf_hexDecode( const char* str, int len )
{
	std::vector<uint8_t> z;
	if( len == -1 )
		len = hf_strlen( str );
	int lenSafe = ((int)(len / 2)) * 2;
	char* tmp, bfr3[3] = {0,0,0,};
	uint8_t val;
	for( int i=0; i<lenSafe; ){
		bfr3[0] = str[i++];
		bfr3[1] = str[i++];
		val = (uint8_t) strtoul( bfr3, &tmp, 16 );
		z.push_back( val );
	}
	return z;
}
/// \cond DOXYGEN_SKIP
;
// indiced array for finding value of a character.
const char* _hf_indiced_characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void _hf_SprintThousand( unsigned int in, char* Buffer, int base, const char* indiced_characters )
{
	unsigned int a = in;
	*Buffer = 0;
	int reminder, i;
	for( i=0;; i++ ){
		reminder = a % base;
		a /= base;
		Buffer[i] = indiced_characters[reminder];
		Buffer[i+1] = 0;
		if(!a)
			break;
	}
	hf_strrev( Buffer );	//_tcsrev()
}
bool _hf_CharacterToValue( int chr, int base, int* value, const char* indiced_characters )
{
	*value = 0;
	//char bfr[2] = { (char)chr, 0, };
	//_strupr( bfr );
	//chr = bfr[0];
	chr = (char)toupper( chr );
	const char* szChr = hf_strchr<char>( indiced_characters, chr );
	if( szChr ){
		int index = (int) (szChr - indiced_characters);
		if( index < base ){
			*value = index;
			return 1;
		}
	}
	return 0;
}
/// \endcond //DOXYGEN_SKIP
;

/// Converts value while being aware that input string may be prefixed with "0x".
uint64_t hf_AsciiToU64With0x( const char* inp, bool* u64overflow, const char* indiced_characters )
{
	// skip any leading witespaces.
	for(; strchr("\x20\r\n\t", *inp ); inp++ );
	if( hf_strlen(inp) >= 2 ){
		if( !hf_strcasecmp("0x", inp, 2 ) ){
			inp += 2;
			return hf_AsciiToU64( inp, 16, u64overflow, indiced_characters );
		}
	}
	return hf_AsciiToU64( inp, 10, u64overflow, indiced_characters );
}
/// Converts input ASCII string to 64-bit unsigned integer.
/// In case if input is expected to be just 32-bit, return value can be
/// downcasted to desired 32-bit type.
/// Parameter 'u64overflow' can be set to null to ignore.
/// Parameter 'indiced_characters' has default value assigned and its
/// preferred to remain as one.
/// Default is set to string that contains characters '0' through 'Z'.
/// \sa GP_strprint_value
uint64_t hf_AsciiToU64( const char* in, int base, bool* u64overflow, const char* indiced_characters )
{
	// skip any leading whitespaces.
	for(; *in && hf_strchr( "\x20\r\n\t", *in ); in++ );

	const char* bgn = in;
	int i, value;

	// scan for end.
	const char* end; int dummy;
	for( end = bgn, i=0; *end && _hf_CharacterToValue( *end, base, &dummy,
		indiced_characters ); end++, i++ );

	if( bgn == end )
		return 0;

	uint64_t a = 0, weight = 1, tmp;
	const char* rbgn;
	char ch;
	for( rbgn = end-1, i=0; rbgn >= bgn; rbgn--, i++, weight *= base ){
		ch = *rbgn;
		_hf_CharacterToValue( (int) ch, base, &value, indiced_characters );
		// detect overflow.
		tmp = a + value * weight;
		if( tmp < a ){
			if( u64overflow )
				*u64overflow = 1;
			break;
		}
		a = tmp;
	}
	return a;
}

int32_t hf_AsciiToS64( const char* in, int base, bool* u64overflow, const char* indiced_characters )
{
	// skip any leading whitespaces.
	for(; *in && hf_strchr( " \r\n\t", *in ); in++ );
	// check if negated.
	bool neg = 0;
	if( *in == '-' ){
		neg = 1;
		in++;
	}
	uint64_t tmp = hf_AsciiToU64( in, base, u64overflow, indiced_characters );
	int32_t out = (int32_t)tmp;
	if(neg){
		out *= -1;
	}
	return out;
}

uint32_t HfOwnDataVerifier::CurULongVar = 1013;
HfOwnDataVerifier::HfOwnDataVerifier()
{
	CurULongVar++;
	const int32_t cx = (int32_t) 4000001013;
	if( CurULongVar > (uint32_t) cx )	//just prevent uint32_t overflow.
		CurULongVar = 1013;
	VarA = CurULongVar;
	VarB = ~VarA;
}
HfOwnDataVerifier::~HfOwnDataVerifier()
{
	VarA = 0;
	VarB = 0xFF;
}
bool HfOwnDataVerifier::verify()const
{
	return VarA == ~VarB;
}

/// Percent encode character string.
/// Also known as URL-encode / URI-encode.
/// By default encodes all non alphanumeric character into %HH sequences.
/// \param inp - input c-string.
/// \param num - optional, number of characters to encode, -1 = c-string.
/// \param szAllowed - optional, characters, beside alphanumerics, that womt
///                    be encoded as %HH-s (specified as c-string).
std::string hf_PercentEncode( const char* inp, int num, const char* szAllowed )
{
	std::string str; char bfr[32]; const char* sz2;
	for( sz2=inp; num && *sz2; sz2++, num-- ){
		if( isalnum( *sz2 ) || strchr( szAllowed, *sz2 ) ){
			str.append( sz2, 1 );
		}else{
			*bfr = 0;
			sprintf( bfr,"%02X", (unsigned int) (unsigned long) (unsigned char) *sz2 );
			str += "%";
			str += bfr;
		}
	}
	return str;
}

/**
	Converts compiler date that comes from "__DATE__" preprocessor constant into
	all numeric componenrs, uses format.
	\param szUudateuu - must be in "Month Day Year" format, f.e. "Jul 27 2015".
						this is exact format of "__DATE__" string.
	\param fmt - format,
		\code
			fe. "%d-%m-%Y".
			%d	Two-digit day of the month (with leading zeros) (01-32)
			%m	Two digit representation of the month	01 (for January) through 12 (for December)
			%Y	Four digit representation for the year	Example: 2038
		\endcode
	\param y2 - y2, m2, d2 - optional, retrieve components as integers.

	\code
		// Example:
		const char* szDt2 = "Dec 08 2002";
		str = hf_convCompilerDateYMD( szDt2, "%Y-%m-%d" );
		printf("[%s]\n", str.c_str() ); //output "[2002-12-08]"
	\endcode
	\sa GP_strprint_value
*/
std::string
hf_convCompilerDateYMD( const char* szUudateuu, const char* fmt, int* y2, int* m2, int* d2 )
{
	char bfrMn[64] = "";
	const char* months2[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", 0, };
	int y3, m3, d3;
	int& y = ( y2 ? *y2 : y3 );
	int& m = ( m2 ? *m2 : m3 );
	int& d = ( d2 ? *d2 : d3 );
	y = d = 1;
	m = -1;
	sscanf( szUudateuu, "%s %d %d", bfrMn, &d, &y );
	for( int i=0; months2[i]; i++ ){
		if( !hf_strcasecmp( bfrMn, months2[i] ) ){
			m = i + 1;
			break;
		}
	}
	if( m != -1 ){
		std::string str = fmt;
		char bfr2[16], bfr3[16], bfr4[16];
		sprintf( bfr2, "%04d", y );
		sprintf( bfr3, "%02d", m );
		sprintf( bfr4, "%02d", d );
		str = hf_strreplace("%Y", bfr2, str.c_str() );
		str = hf_strreplace("%m", bfr3, str.c_str() );
		str = hf_strreplace("%d", bfr4, str.c_str() );
		return str;
	}else
		return "";
}
/// Returns global ticks in miliseconds.
/// Returnd value is 0-based at some random point in the past.
/// On Windows its a wrapper for GetTickCount().
uint32_t hf_getGlobalTicks()
{
#	ifdef _HF_TIMEOUT_SRV_WIN32
		return GetTickCount();
#	else
		// reference:
		// .../SDL/src/timer/unix/SDL_systimer.c
		// http://pubs.opengroup.org/onlinepubs/000095399/functions/gettimeofday.html
		struct timeval tvx;
		gettimeofday(&tvx, 0);
		uint32_t ticksx = (uint32_t)( (tvx.tv_sec * 1000) + (tvx.tv_usec / 1000) );
		return ticksx;
#	endif //_HF_TIMEOUT_SRV_WIN32
}
/**
	Returns UTC ticks in miliseconds.
	Equivalent of time(0) but with miliseconds resolution instead.
	\code
		// To convert to date string use retval divided by 1000.
		uint64_t tm4 = hf_getTimeTicksMs();
		time_t tm2 = (time_t)( tm4 / 1000 );
		const tm* tm3 = localtime( &tm2 );
		strftime( bfr, sizeof(bfr), "%Y-%m-%d/%H:%M:%S", tm3 );
	\endcode
*/
uint64_t hf_getTimeTicksMs()
{
#	ifdef _HF_TIMEOUT_SRV_WIN32
		hf_assert(!"hf_getTimeTicksMs() WIN32 not implemented.");
		return 0;
#	else
		//#include <sys/time.h>
		// A time value that is accurate to the nearest
		// microsecond but also has a range of years.
		//struct timeval:
		//	__time_t tv_sec;		// Seconds.
		//	__suseconds_t tv_usec;	// Microseconds.
		struct timeval tvx;
		gettimeofday(&tvx, 0);
		uint64_t tmticks = ( ((uint64_t)tvx.tv_sec) * 1000 + (tvx.tv_usec / 1000) );
		return tmticks;
#	endif //_HF_TIMEOUT_SRV_WIN32
}

/// \cond //DOXYGEN_SKIP //{
;
/// Uncompressed TGA Header.
/// (compressed has the byte set to 10 (instead of 2)).
uint8_t hf_UncmprTgaMagic[12] = {0,0,2,0,0,0,0,0,0,0,0,0};
;
/// \endcond //DOXYGEN_SKIP //}
;
/// Frees data loded by hf_LoadUncompressedTGA().
/// \sa GP_tga_routines
bool hf_FreeTGA( HfSTgaTexture* tex2 )
{
	if( tex2->imageData ){
		free( tex2->imageData );
		tex2->imageData = 0;
		return 1;
	}
	return 0;
}
/**
	Loads uncompressed TGA image (Truevision TGA, TARGA).

	\param tex3 - output structure.
	\param filename - TGA filename to load.
	\param flags2 - flags, fe. \ref HFE_TGA_SwapPerPixelBytes.
	\param err - optional, can be set to NULL. textural message on error.
	\return Returns true on success.

	TGA imagess come in two formats RGBA and RGB (32 and 24 bpp respectivelly).
	Pixel formt for returned pixels, for 32-bit mode images is 0xAARRGGBB.

	TGA files with alpha channel translucency (RGBA) can be created correctly
	by converting from PNG-s using mtPaint on Linux (tested v.3.40).

	\verbatim
		How to convert RGB to RGBA image using mtPaint on Linux
		---------------------------------------------------------------
		* Open TGA image
		* chose manu: Image - Convert to RGB (if its not alerady).
		* chose menu: Channels - Edit Alpha.
		* "Initial Channel State" select "Set".
		* optionally, change back to: Channels - Edit Image.
		* chose: File - Save As.
	\endverbatim
	Code based on the tutorial:
	\verbatim
		"NeHe Productions Loading Compressed And Uncompressed TGAs"
		http://nehe.gamedev.net/tutorial/loading_compressed_and_uncompressed_tga%27s/22001/
		Original note:
		// Load an uncompressed TGA (note, much of this code is based on NeHe's)
	\endverbatim
	\sa GP_tga_routines
*/
bool hf_LoadUncompressedTGA( HfSTgaTexture* tex3, const char* filename, uint32_t flags2, std::string* err )
{
	HfSTgaHeader   tgaheader;
	HfSTga         tga2;
	FILE*          hTGA;
	std::string    err2;
	err = ( err ? err : &err2 );

	hTGA = fopen( filename, "rb");
	if( !hTGA ){
		*err = "Could not open file.";
		return 0;
	}
	// Attempt to read 12 byte header from file
	if( !fread( &tgaheader, sizeof(HfSTgaHeader), 1, hTGA ) ){
		*err = "could not read file header.";
		if(hTGA != NULL)												// Check to seeiffile is still open
			fclose(hTGA);												// If it is, close it
		return 0;													// Exit function
	}
	// See if header matches the predefined header of an Uncompressed TGA image.
	if( memcmp( hf_UncmprTgaMagic, &tgaheader, sizeof(tgaheader) ) ){
		*err = "TGA file must be type 2 (uncompressed).";
		fclose(hTGA);
		return 0;
	}
	// TGA Loading code 'nehe.gamedev.net'.
	// Read TGA header.
	if( !fread(tga2.header, sizeof(tga2.header), 1, hTGA) ){ //HfSTga tga2;
		*err = "Could not read info header.";
		if( hTGA != NULL )
			fclose(hTGA);
		return 0;				// Return failure
	}
	tex3->width  = tga2.header[1] * 256 + tga2.header[0];	// Determine The TGA Width	(highbyte*256+lowbyte)
	tex3->height = tga2.header[3] * 256 + tga2.header[2];	// Determine The TGA Height	(highbyte*256+lowbyte)
	tex3->bpp	 = tga2.header[4];							// Determine the bits per pixel
	tga2.Width		= tex3->width;							// Copy width into local structure
	tga2.Height		= tex3->height;							// Copy height into local structure
	tga2.Bpp		= tex3->bpp;							// Copy BPP into local structure

	// Make sure all information is valid
	if( (tex3->width <= 0) || (tex3->height <= 0) || ((tex3->bpp != 24) && (tex3->bpp !=32)) ){
		*err = "Invalid texture information.";
		if( hTGA )
			fclose(hTGA);
		return 0;
	}
	if(tex3->bpp == 24){	// If the BPP of the image is 24...
		tex3->type	= HFE_TGA_RGB;		// Set Image type to HFE_TGA_RGB
	}else{	// Else if its 32 BPP
		tex3->type	= HFE_TGA_RGBA;		// Set image type to HFE_TGA_RGBA
	}
	tga2.bytesPerPixel	= (tga2.Bpp / 8);								// Compute the number of BYTES per pixel
	tga2.imageSize		= (tga2.bytesPerPixel * tga2.Width * tga2.Height);	// Compute the total amout of memory needed to store data.
	tex3->nBytes     = tga2.imageSize;
	tex3->imageData	= (uint8_t*)malloc( tga2.imageSize );				// Allocate that much memory
	if( !tex3->imageData ){				// If no space was allocated
		*err = "Could not allocate memory for image.";
		fclose(hTGA);
		return 0;
	}
	// Attempt to read image data
	if( fread( tex3->imageData, 1, tga2.imageSize, hTGA ) != tga2.imageSize ){
		*err = "Could not read image data.";
		if( tex3->imageData ) // If imagedata has data in it
			free(tex3->imageData); // Delete data from memory
		fclose(hTGA);
		return 0;
	}
	if( flags2 & HFE_TGA_SwapPerPixelBytes ){
		// Byte Swapping Optimized By Steve Thomas.
		uint32_t cswap;
		for( cswap = 0; cswap < (uint32_t)tga2.imageSize; cswap += tga2.bytesPerPixel ){
			uint8_t& a = tex3->imageData[cswap];
			uint8_t& b = tex3->imageData[cswap+2];
			a ^= b ^= a ^= b;
		}
	}
	fclose(hTGA);
	return 1; // Return success
}
/**
	Saves TGA file.
	\param filename  - image file name to be created.
	\param nWidth    - image width.
	\param nHeight   - image height.
	\param fmt2 - must specify either \ref HFE_TGA_RGB or \ref HFE_TGA_RGBA.
	              additionally other flags can be used.
	              if \ref HFE_TGA_SwapPerPixelBytes is specified, each pixel iteration
	              is performed and R and B components swapped before writing.
	\param data2 - byte array for each pixel.
				   Alpha component example: 0xFF - fully opaque pixel, 0x00 - fully translucent pixel.
	\param err - optional, error text on failure.

	\code
		// Example #1.
		// Creates a 4x5 pixel size file.
		// A 2x2 red dot, enclosed by gray pixels from left, top and right.
		// An area below the red dot is created fully translucent.
		uint8_t data2[] = {
			0x80,0x80,0x80,0xFF, 0x80,0x80,0x80,0xFF, 0x80,0x80,0x80,0xFF, 0x80,0x80,0x80,0xFF,
			0x80,0x80,0x80,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x80,0x80,0x80,0xFF,
			0x80,0x80,0x80,0xFF, 0x00,0x00,0xFF,0xFF, 0x00,0x00,0xFF,0xFF, 0x80,0x80,0x80,0xFF,
			0x80,0x80,0x80,0xFF, 0xFF,0xFF,0xFF,0x00, 0xFF,0xFF,0xFF,0x00, 0x80,0x80,0x80,0xFF,
			0x80,0x80,0x80,0xFF, 0xFF,0xFF,0xFF,0x00, 0xFF,0xFF,0xFF,0x00, 0x80,0x80,0x80,0xFF,
		};
		bool rs2;
		rs2 = hf_SaveUncompressedTGA( "./testoutp.tga", 4, 5, HFE_TGA_RGBA, data2, 0 );
		printf("rs2: %d\n", rs2 );
	\endcode
	\code
		// Example #2:
		// Same image as example #1 but using uint32_t array for pixels.
		// Assumes little-endian byte order.
		uint32_t data3[] = {
			0xFF808080,0xFF808080,0xFF808080,0xFF808080,
			0xFF808080,0xFFFF0000,0xFFFF0000,0xFF808080,
			0xFF808080,0xFFFF0000,0xFFFF0000,0xFF808080,
			0xFF808080,0x00FFFFFF,0x00FFFFFF,0xFF808080,
			0xFF808080,0x00FFFFFF,0x00FFFFFF,0xFF808080,
		};
		bool rs3;
		rs3 = hf_SaveUncompressedTGA( "./testoutp3.tga", 4, 5, HFE_TGA_RGBA, &data3[0], 0 );
		printf("rs3: %d\n", rs3 );
	\endcode

	\code
		// Example #3.
		// Same as example #1 but no alpha channel, 24-bit pixel format.
		uint8_t data4[] = {
			0x80,0x80,0x80, 0x80,0x80,0x80, 0x80,0x80,0x80, 0x80,0x80,0x80,
			0x80,0x80,0x80, 0x00,0x00,0xFF, 0x00,0x00,0xFF, 0x80,0x80,0x80,
			0x80,0x80,0x80, 0x00,0x00,0xFF, 0x00,0x00,0xFF, 0x80,0x80,0x80,
			0x80,0x80,0x80, 0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF, 0x80,0x80,0x80,
			0x80,0x80,0x80, 0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF, 0x80,0x80,0x80,
		};
		rs3 = hf_SaveUncompressedTGA( "./testoutp4.tga", 4, 5, HFE_TGA_RGB, data4, 0 );
		printf("rs3: %d\n", rs3 );
	\endcode
	\sa GP_tga_routines
*/
bool hf_SaveUncompressedTGA( const char* filename, uint16_t nWidth, uint16_t nHeight,
		int fmt2, const void* data2, std::string* err )
{
	const uint8_t bpp = ( fmt2 & HFE_TGA_RGBA ? 32 : 24 );
	uint32_t nBytes2 = nWidth * nHeight * (bpp/8);
	std::string sink0, *err3 = ( err ? err : &sink0 );
	//if( !( bpp == 24 || bpp == 32 ) )
	if( !( fmt2 & HFE_TGA_RGB || fmt2 & HFE_TGA_RGBA ) ){
		*err3 = "Only 24 and 32 bit formats supported.";
		return 0;
	}
	FILE* fph = fopen( filename, "w" );
	if( !fph ){
		*err3 = "Failed opening file for write.";
		return 0;
	}
	rewind( fph );
	if( !fwrite( hf_UncmprTgaMagic, sizeof(hf_UncmprTgaMagic), 1, fph ) ){
		*err3 = "Failed writing data (1).";
		fclose(fph);
		return 0;
	}
	uint16_t sizewh[] = { nWidth, nHeight, };
	if( !fwrite( sizewh, sizeof(sizewh), 1, fph ) ){
		*err3 = "Failed writing data (2).";
		fclose(fph);
		return 0;
	}
	// Image descriptor (1 byte): bits 3-0 give the alpha channel depth,
	// bits 5-4 give direction.
	// fe: 0x28 --> bpp: 8, direction: 2
	uint8_t nDrctn = ( bpp == 32 ? 0x28 : 0x20 );
	uint8_t bppEtc[] = { bpp, nDrctn, };
	if( !fwrite( bppEtc, sizeof(bppEtc), 1, fph ) ){
		*err3 = "Failed writing data (3).";
		fclose(fph);
		return 0;
	}
	if( !(fmt2 & HFE_TGA_SwapPerPixelBytes) ){
		// write bytes.
		if( !fwrite( data2, nBytes2, 1, fph ) ){
			*err3 = "Failed writing data (4).";
			fclose(fph);
			return 0;
		}
	}else{
		int r, i, nPxSize = ( fmt2 & HFE_TGA_RGBA ? 4 : 3 );
		const uint8_t* row2, *px2;
		uint8_t* bfr, *px3;
		bfr = (uint8_t*)malloc( nWidth * nPxSize );
		for( r=0; r<nHeight; r++ ){ // for each row.
			row2 = &((uint8_t*)data2)[ r * nWidth * nPxSize ];
			for( i=0; i<nWidth; i++ ){ // each row's pixel.
				px2 = &row2[ i * nPxSize ];
				px3 = &bfr[ i * nPxSize ];
				px3[0] = px2[2];
				px3[1] = px2[1];
				px3[2] = px2[0];
			}
			if( !fwrite( bfr, nWidth * nPxSize, 1, fph ) ){
				*err3 = "Failed writing data (5).";
				fclose(fph);
				free(bfr);
				return 0;
			}
		}
		free(bfr);
		bfr = 0;
	}
	fclose(fph);
	return 1;
}

/// \defgroup GP_strftime_etc Time string print functions.
/// ...
/// hf_strftime2() \n
/// hf_strftime3() \n
/// hf_convWinFiletime1601ToUnix1970ms() \n
/// hf_convUnix1970msToWinFiletime1601() \n

/**
	String prints local time.
	Based on C functions strftime(), gmtime() and localtime().
	\param fmt - format string, fe."%H:%M:%S".
	\param tmx - optional input timestamp. if 0, current time is used (calls time(0)).
	\param szTimezone - can be set to "GMT" to use GMT time (gmtime()), otherwise
	                    uses local time (localtime()).
	NOTE: This implementation has a 128 characters limit for output string.
	      Nontheless, using even up to 4 extra characters per each component should be ok.
	\sa GP_strftime_etc
*/
std::string hf_strftime2( const char* fmt, time_t tmx, const char* szTimezone )//bool bUseGMT
{
	struct tm tmxstruct;
	if(!tmx){
		tmx = time(0);
	}
	//if( bUseGMT )
	if( szTimezone && !hf_strcmp( szTimezone, "GMT" ) ){
		tmxstruct = *gmtime( &tmx );
	}else{
		tmxstruct = *localtime( &tmx );
	}
	return hf_strftime3( fmt, &tmxstruct );
	//return hf_strftime3( fmt, ( tmx ? localtime(&tmx) : 0 ), bUseGMT );
}
/**
	String prints time. Internally uses strftime().
	\sa GP_strftime_etc
*/
std::string hf_strftime3( const char* fmt, const struct tm* tmy )
{
	char bfr[128] = ""; struct tm tmy2;
	if(!tmy){
		time_t tmx3 = time(0);
		tmy2 = *localtime( &tmx3 );
		tmy = &tmy2;
	}
	strftime( bfr, sizeof(bfr), fmt, tmy );
	return bfr;
}

/**
	Outputs UTC timestamp in seconds given Y,M,D,H,mm and S.
	nYear   - year, fe: 1999.
	nMonth  - months since January – [1, 12].
	nDay    - day of the month – [1, 31].
	nHour   - hours since midnight – [0, 23].
	nMinute - minutes after the hour – [0, 59].
	nSecond - seconds after the minute - [0, 60].
	nDst    - Daylight Saving Time flag. The value is positive if DST is in effect, zero if not and negative if no information is available.
*/
time_t hf_mktime2( int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nDst )
{
	struct tm tm2;
	memset( &tm2, 0, sizeof(tm2) );
	tm2.tm_year  = nYear - 1900;
	tm2.tm_mon   = nMonth - 1;
	tm2.tm_mday  = nDay;
	tm2.tm_hour  = nHour;
	tm2.tm_min   = nMinute;
	tm2.tm_sec   = nSecond;
	tm2.tm_isdst = nDst;
	tm2.tm_wday  = 0;
	tm2.tm_yday  = 0;
	time_t tst2 = mktime( &tm2 );
	return tst2;
}



//typedef struct _FILETIME{ //prop.filetime
//  DWORD dwLowDateTime;
//  DWORD dwHighDateTime;
//}FILETIME;
// WINAPI CHM "ms-its://ms-its/topic_filetime.htm".
// WINAPI: The FILETIME structure is a 64-bit value representing the number of
//         100-nanosecond intervals since January 1, 1601.
// WCX int FileTime: 1980-Jan-1
//
// Epoch timestamp: -11644473600 (seconds)
// - Human time (GMT): Monday, January 1, 1601 12:00:00 AM
// Epoch timestamp: 0 (seconds)
// - Human time (GMT): Thursday, January 1, 1970 12:00:00 AM
// Epoch timestamp: 315532800 (seconds)
// - Human time (GMT): Tuesday, January 1, 1980 12:00:00 AM
// Epoch timestamp: 1514764800 (seconds)
// - Human time (GMT): Monday, January 1, 2018 12:00:00 AM
// ref: https://www.epochconverter.com/
//
// ll = Int32x32To64(t, 10000000) + 116444736000000000;
// -                                11644473600
// ref: https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
/**
	Returns unix timestamp in milliseconds (ms) (instead of seconds) (1s = 1000ms).
	Time since 1970-Jan-1. Conversion can yield values Less than zero.
	\param nano100thSecs - 64-bit value that usually is stored as two U32 valuse as
						   FILETIME structure.
	\code
		// std::string hf_strftime2( const char* fmt, const time_t tmx )
		const uint64_t val = 128166372000000000; // = [2007-02-22/18:00:00] //1172163600
		int tsx = hf_convWinFiletime1601ToUnix1970ms( val ) / 1000;
		str = hf_strftime2( "%Y:%m:%d-%H:%M:%S", static_cast<time_t>(tsx) );
	\endcode
	\sa GP_strftime_etc
*/
int64_t hf_convWinFiletime1601ToUnix1970ms( uint64_t nano100thSecs )
{
	// ref: https://stackoverflow.com/a/6161842/3239847
	const int nWindowsTick = 10000;
	const int64_t nSecToUnixEpoch1000 = 11644473600000LL;
	return (nano100thSecs / nWindowsTick - nSecToUnixEpoch1000);
}
/**
	Converts unix timestamp in milliseconds to Windows FILETIME.
	\code
		// use this method to conv 32 bit timestamp without overflow.
		uint64_t millisecs = uint64_t(tsxx) * 1000;
		uint64_t t = hf_convUnix1970msToWinFiletime1601( millisecs );
	\endcode
	\sa GP_strftime_etc
*/
uint64_t hf_convUnix1970msToWinFiletime1601( int64_t tmMsecs1970Based )
{
	// ref: https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
	//uint64_t retv = tm1970Based * 10000000 + 116444736000000000;
	uint64_t retv = tmMsecs1970Based * 10000 + 116444736000000000;
	return retv;
}

} // end namespace hef





