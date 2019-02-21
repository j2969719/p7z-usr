
#ifndef _HEF_DATA_H_
#define _HEF_DATA_H_
#include "hef_str_piece.h"
#include <algorithm>
#include <inttypes.h>

/**
	\mainpage

	\section intro_sec Introduction

	A library that provides basic routines for working with strings, text,
	binary files, etc.\n
	All code is in the \ref hef "hef namespace".\n

	\section instructions_sec Instructions

	Compile all source files with your program.
*/
/**
	\namespace hef
	Main namespace.
*/
namespace hef{
;

/// Flags for hf_PutFileBytes().
/// old: EPFBF_NOFLAGS
enum { HF_EPFBF_NOFLAGS=0, HF_EPFBF_APPEND=0x1, };

// library functions #1.
std::string      hf_dataToHexViewAsLines( const void* data, size_t size, size_t uLineLen=8, const char* szNewline="\n", const char* szEachLinePrefix="", const char szFillerHex[3]="..", const char szFillerChar[2]=" " );
std::string      hf_dataToAsciiNlSpec( const uint8_t* data, size_t size );
std::string      hf_strWithCrLfShown( const char* in, bool bInclLineBreaks=1 );
size_t           hf_ConvertBytesArrayToU32LE( void* addrArray4 );
size_t           hf_ReadFileU32LE( FILE* fp, size_t addrInFile );
std::vector<uint8_t> hf_ReadFileBytes( FILE* fp, size_t addrInFile, size_t numBytes );
bool             hf_WriteFileBytes( FILE* fp, size_t addrInFile, const void* bytes, size_t size );
bool             hf_PutFileBytes( const char* fn, const void* bytes, int size = -1, size_t flags = HF_EPFBF_NOFLAGS );
std::string      hf_hexEncode( const void* data, uint32_t size );
std::vector<uint8_t> hf_hexDecode( const char* str, int len = -1 );
std::string      hf_PercentEncode( const char* inp, int num = -1, const char* szAllowed="" );
std::string      hf_convCompilerDateYMD( const char* szUudateuu, const char* fmt, int* y=0, int* m=0, int* d=0 );
uint32_t         hf_getGlobalTicks();
uint64_t         hf_getTimeTicksMs();

std::string hf_strftime2( const char* fmt, time_t tmx, const char* szTimezone = 0 );
std::string hf_strftime3( const char* fmt, const struct tm* tmy );
time_t      hf_mktime2( int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nDst = -1 );
int64_t     hf_convWinFiletime1601ToUnix1970ms( uint64_t nano100thSecs );
uint64_t    hf_convUnix1970msToWinFiletime1601( int64_t tmMsecs1970Based );

/// \cond DOXYGEN_SKIP
// internal struct for hf_arrayUnionViaVarname() use.
struct SLsPairFindByFirst {
	bool operator()( const std::pair<std::string,std::string>& in )const {return !bBySecond ? in.first == *szVarname : in.second == *szVarname;}
	SLsPairFindByFirst( const std::string* szVarname_, bool bBySecond_=0 ) : szVarname(szVarname_), bBySecond(bBySecond_) {}
private:
	SLsPairFindByFirst();	//hidding constructor.
	const std::string* szVarname;
	bool bBySecond;
};
/// \endcond //DOXYGEN_SKIP

/**
	\defgroup GP_ArrayStrStr Array as std::string Pairs
	...
	hf_arrayUnionViaVarname() \n
	hf_arraySetPropertyViaVarname() \n
	hf_arrayGetPropertyViaVarname() \n
	hf_arrayFindPropertyViaValue() \n
*/

/**
	Union of 'array_a' and array #2 (second array is specified by [begin,end] range).
	Template parameter 'Tx' is expected to becontainer type of std::pair elements.
	template parameters for std::pair element both must be std::string.
	Examples: \code
		std::vector<std::pair<std::string,std::string> > vec1;
		std::list<std::pair<std::string,std::string> > list2;
	\endcode
	Returns reference to, possibly modified, 'array_a' parameter.

	Example 2:
	\code
		std::vector<std::pair<std::string,std::string> >::const_iterator a;
		std::vector<std::pair<std::string,std::string> > ls1, ls2;
		ls1.push_back( std::make_pair("szVarA","100") );
		//
		ls2.push_back( std::make_pair("szVarA","200") );
		ls2.push_back( std::make_pair("szVarB","201") );
		hf_arrayUnionViaVarname( ls1, ls2 );
		for( a = ls1.begin(); a != ls1.end(); ++a ){
			printf("%s: [%s]\n", a->first.c_str(), a->second.c_str() );
		}
		// results in ls1:
		// ["szVarA","100"]
		// ["szVarB","201"]
	\endcode
	\sa GP_ArrayStrStr
*/
template<class Tx> Tx&
hf_arrayUnionViaVarname( Tx& array_a, const typename Tx::const_iterator& bgn,
						 const typename Tx::const_iterator& end )
{
	std::vector<const std::pair<std::string,std::string>*> tmpAddList;
	std::vector<const std::pair<std::string,std::string>*>::const_iterator b;
	typename Tx::const_iterator a;
	for( a = bgn; a != end; ++a ){
		if( std::find_if( array_a.begin(), array_a.end(), SLsPairFindByFirst(&a->first,0) ) == array_a.end() )
			tmpAddList.push_back( &(*a) );
	}
	for( b = tmpAddList.begin(); b != tmpAddList.end(); ++b ){
		array_a.push_back( **b );
	}
	return array_a;
}
/// Retrieves propety given 'szVarname'.
/// \sa GP_ArrayStrStr
template<class Tx> const char*
hf_arrayGetPropertyViaVarname( Tx& arr, const char* szVarname, const char* szDefault = "" )
{
	typename Tx::const_iterator a;
	std::string str(szVarname);
	if( (a=std::find_if( arr.begin(), arr.end(), SLsPairFindByFirst( &str, 0 ) )) != arr.end() ){
		return a->second.c_str();
	}
	return szDefault;
}
template<class Tx> const std::pair<std::string,std::string>&
hf_arrayFindPropertyViaValue( Tx& arr, const char* szValue, const std::pair<std::string,std::string>* dflt = 0 )
{
	typename Tx::const_iterator a;
	std::string str(szValue);
	if( (a=std::find_if( arr.begin(), arr.end(), SLsPairFindByFirst(&str,1) )) != arr.end() ){
		return *a;
	}
	if(dflt)
		return *dflt;
	static const std::pair<std::string,std::string> emptyDflt;
	return emptyDflt;
}
/// Sets property to 'szValue'.
/// \sa GP_ArrayStrStr
template<class Tx> Tx&
hf_arraySetPropertyViaVarname( Tx& arr, const char* szVarname, const char* szValue )
{
	typename Tx::iterator a;
	std::string str(szValue);
	if( (a=std::find_if( arr.begin(), arr.end(), SLsPairFindByFirst( &str,0 ) )) != arr.end() ){
		a->second = szValue;
	}else{
		arr.push_back( std::pair<std::string,std::string>(szVarname,szValue) );
	}
	return arr;
}

/// \cond DOXYGEN_SKIP
// internal routines.
void _hf_SprintThousand( unsigned int in, char* Buffer, int base, const char* indiced_characters );
void data_general_tester();
extern const char* _hf_indiced_characters;// = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
/// \endcond //DOXYGEN_SKIP

// library functions #2.
std::string           hf_StrPrintU64( uint64_t in, uint32_t base = 10, uint32_t uDigitGrouping=0, const char* glue=",", size_t uLeadingCharsCount=0, char leading='0' );
std::string           hf_StrPrintS64( int64_t in, uint32_t base = 10, uint32_t uDigitGrouping=0, const char* glue=",", size_t uLeadingCharsCount=0, char leading='0' );
uint64_t              hf_AsciiToU64( const char* in, int base = 10, bool* u64overflow = 0, const char* indiced_characters = _hf_indiced_characters );
int32_t               hf_AsciiToS64( const char* in, int base = 10, bool* u64overflow = 0, const char* indiced_characters = _hf_indiced_characters );
uint64_t              hf_AsciiToU64With0x( const char* inp, bool* u64overflow = 0, const char* indiced_characters = _hf_indiced_characters );

/// \cond DOXYGEN_SKIP

/// Comprehensive string print of unsigned integer of any type (uses template parameter).
/// May be used to print any type of value, eg. 64-bit, bool, etc. Outputs
/// string in any desired base (note: currrently max safe base is 32).
/// For example, set 'base' to 10 for decimal output, to 2 for binary and to 16 for
/// hexadecimal. 'base' must not be lower than 2.
/// Intended for internal use, in application use hf_StrPrintS64() and
/// hf_StrPrintU64() instead.
template<class T>
char* _hf_sprintUT( T in, char* Buffer, const char* comma = "", int base = 10,
                  int digit_grouping = 3, int desired_min_length = 0,
                  char prepend_with = 0,
                  const char* indiced_characters = _hf_indiced_characters )
{
	*Buffer = 0;
	char bfrThousand[128], bfrTmp[128];
	int i;
	T a = in, reminder, thousand = 1;
	for( i=0; i<digit_grouping; i++, thousand *= base );
	for(;;){
		reminder = a % thousand;
		a /= thousand;
		if(a){
			_hf_sprintUT( reminder, bfrThousand, 0, base, digit_grouping + 1, digit_grouping,
				indiced_characters[0], indiced_characters );
		}else{
			_hf_SprintThousand( (unsigned int)reminder, bfrThousand, base, indiced_characters );
		}
		hf_strcpy( bfrTmp, Buffer );
		hf_strcpy( Buffer, bfrThousand );
		if( comma && *bfrTmp ){
			hf_strcat( Buffer, comma );
		}
		hf_strcat( Buffer, bfrTmp );
		if(!a)
			break;
	}
	if( desired_min_length && prepend_with ){
		int ln = (int) hf_strlen( Buffer );
		if( ln < desired_min_length ){
			int num = desired_min_length - ln;
			char bfr[] = { prepend_with, 0, };
			hf_strcpy( bfrTmp, Buffer );
			*Buffer = 0;
			for(; num--; ){
				hf_strcat( Buffer, bfr );
			}
			hf_strcat( Buffer, bfrTmp );
		}
	}
	return Buffer;
}
/// Similar to _hf_sprintUT() but for signed integers.
template<class T>
char* _hf_sprintST( T a, char* Buffer, const char* comma="", int base=10,
				  int digit_grouping=3, int desired_min_length = 0,
                  char prepend_with = 0,
				  const char* indiced_characters = _hf_indiced_characters )
{
	*Buffer = 0;
	char* bfr = Buffer;
	T val = a;
	if( val < 0 ){
		val *= -1;
		Buffer[0] = '-';
		Buffer[1] = 0;
		bfr = &Buffer[1];
	}
	_hf_sprintUT<T>( val, bfr, comma, base, digit_grouping, desired_min_length,
		prepend_with, indiced_characters );
	return Buffer;
}
/// \endcond //DOXYGEN_SKIP

/**
	Comprehensive string-print for any type of integer.
	Replacement for: \code
		sprint64(), sprintS64(), hf_StrPrintU64(),
		hf_StrPrintS64(), _hf_sprintST(), _hf_sprintUT(), etc.
	\endcode
	Examples: \code
		hf_StrPrintAny<uint64_t>( 5000000001, 10 );	//output: "5000000001"
		hf_StrPrintAny<uint64_t>( 5000000301, 16 );	//output: "12A05F32D"
		hf_StrPrintAny<uint32_t>( 0xFFFFFFFF, 32 );	//output: "3VVVVVV" //NOTE: base set to 32.
	\endcode
	\sa GP_strprint_value
*/
template<class T>
std::string
hf_StrPrintAny( T in, int base = 10, int digit_grouping = 0,
               const char* comma = ",", int desired_min_length = 0,
               char prepend_with = '0',
               const char* indiced_characters = _hf_indiced_characters )
{
	// determine number of characters in max value.
	size_t uMaxValueChars = sizeof(T) * 8 * 2 / ( std::min( base/2, 8 ) );
	//printf("-- uMaxValueChars: %u\n", uMaxValueChars );
	// determine max possible buffer size, incl. digits, minus sign, comma characters, etc.
	size_t nMaxBfrSize = ( std::max<uint32_t>( uMaxValueChars, desired_min_length ) + 3) * 2;
	char bfrL[28];	//64,28
	char* pBfr = bfrL;
	bool bOwnBfr = 0;
	if( nMaxBfrSize > sizeof(bfrL) ){
		bOwnBfr = 1;
		pBfr = (char*) malloc( nMaxBfrSize );
	}
	const char* comma2 = comma;
	if(!digit_grouping){
		digit_grouping = 3;
		comma2 = "";
	}
	std::string z;
	// test if template integer is signed type.
	if( ((T)-1) < 0 ){	//if signed type
		//printf("-- signed int type\n");
		_hf_sprintST<T>( in, pBfr, comma2, base, digit_grouping, desired_min_length,
			prepend_with, indiced_characters );
		z = pBfr;
	}else{
		//printf("-- unsigned int type\n");
		_hf_sprintUT<T>( in, pBfr, comma2, base, digit_grouping, desired_min_length,
			prepend_with, indiced_characters );
		z = pBfr;
	}
	if(bOwnBfr)
		free( pBfr );
	return z;
}
/// Generates unique 64-bit data and provides verification methods.
/// May be used for debug checks if there exists possibility data is being overriden.
class HfOwnDataVerifier {
public:
	HfOwnDataVerifier();
	~HfOwnDataVerifier();
	bool verify()const;
private:
	unsigned long VarA, VarB;
	static uint32_t CurULongVar;
};

/// Extends std::vector<T> adding various initialization constructors and
/// reference returning push-back methods.
template<class T>
class HfVec : public std::vector<T> {
public:
	HfVec() {}
//	HfVec( const T& a1 ) {push2(a1);}
//	HfVec( const T& a1, const T& a2 ) { push2(a1,a2); }
//	HfVec( const T& a1, const T& a2, const T& a3 ) {push2(a1,a2,a3);}
	HfVec<T>& push2( const T& in ){
		this->push_back(in);
		return *this;
	}
	HfVec<T>& push2( const T& in1, const T& in2 ) {return push2(in1).push2(in2);}
	HfVec<T>& push2( const T& in1, const T& in2, const T& in3 ) {return push2(in1,in2).push2(in3);}
	HfVec<T>& push2( const HfVec<T>& other ){
		typename HfVec<T>::const_iterator a;
		for( a = other.begin(); a != other.end(); ++a )
			push2(*a);
		return *this;
	}
	HfVec<T>& push_vec( const std::vector<T>& other ){
		typename std::vector<T>::const_iterator a;
		for( a = other.begin(); a != other.end(); ++a )
			push2(*a);
		return *this;
	}
};

template<class T>
T hf_arrayGetAverage_1( const std::vector<T>& arr )
{
	T sum = 0;
	typename std::vector<T>::const_iterator a;
	for( a = arr.begin(); a != arr.end(); ++a ){
		sum += *a;
	}
	T avrg = sum / arr.size();
	return avrg;
}
/**
	Returns average value of all elements provided through range from 'begin_' to 'end_'.
	Uses addition and division operators.
	NOTE: Returns 0 if input range was empty.
	Example:
	\code
		std::vector<float> arrf( 2, 2.1f );
		arrf.push_back( 4.1f );
		float avg = hf_arrayGetAverage( arrf, arrf.begin(), arrf.end() );
	\endcode
*/
template<class T, class TArr>
T hf_arrayGetAverage( const TArr&, const typename TArr::const_iterator& begin_,
					  const typename TArr::const_iterator& end_, T value_type_setter )
{
	typename TArr::value_type sum(0);//, avrg(0);
	T avrgOut(0);
	typename TArr::const_iterator a; size_t count = 0;
	for( a = begin_; a != end_; ++a, count++ ){
		sum += *a;
	}
	if(count)
		avrgOut = sum / count;
	return avrgOut;
}



/**
	Predicate template class for std::pair<> with std::find_if() and std::find().

	* For searching by std::pair::first use:
	  1st constructor

	* For searching by std::pair::second use:
	  2nd constructor

	\verbatim
		Example:
		a = std::find_if( lsWhSkidIdxs.begin(), lsWhSkidIdxs.end(), HfPredTTPair<size_t,int>( inp.uWheelIndex ) );
	\endverbatim
*/
template<class TY1, class TY2=int>
struct HfPredTTPair{
	const TY1 xFirst;
	HfPredTTPair( const TY1 xFirst_ ) : xFirst(xFirst_) {}
	bool operator()( const std::pair<TY1,TY2>& other )const{
		return xFirst == other.first;
	}
	struct S2{
		const TY2 xSecond;
		S2( const TY2 xSecond_ ) : xSecond(xSecond_) {}
		bool operator()( const std::pair<TY1,TY2>& other )const{
			return xSecond == other.second;
		}
	};
};
typedef HfPredTTPair<std::string,std::string> HfPredStrPair;

struct HfSProgressMessage{
	std::string message;
	HfSProgressMessage( const char* msg ) : message(msg) {}
};
class HfIProgressMessage {
public:
	virtual void hf_calbProgressMessage( const HfSProgressMessage& inp )const = 0;
};


class HfIntDataNotify{
public:
	virtual int hf_IntDataNotifyCalb( void* userdata ) = 0;
};



/// \defgroup GP_tga_routines TGA Image Processing Routines.
/// ...
/// hf_LoadUncompressedTGA() \n
/// hf_SaveUncompressedTGA() \n
/// hf_FreeTGA() \n
/// HfSTgaTexture \n

enum{
	/// TGA types for 'HfSTgaTexture' 'type' member.
	/// 24 bits per pixel.
	HFE_TGA_RGB = 0x1,		///< 24 bits per pixel
	HFE_TGA_RGBA = 0x2,		///< 32 bits per pixel
	/// Flags for hf_LoadUncompressedTGA().
	/// Swaps per-pixel, red with blue components.
	HFE_TGA_SwapPerPixelBytes = 0x4,
};

/// TGA image loaded from hf_LoadUncompressedTGA().
/// \sa GP_tga_routines
typedef struct {
	/// Image Data (Up To 32 Bits).
	/// size: width*height*(bpp/8).
	/// Pixel format: 0xAARRGGBB, fe. alpha mask is 0xFF000000.
	uint8_t*  imageData;
	uint32_t  bpp;			///< Image Color Depth In Bits Per Pixel.
	uint32_t  width;		///< Image Width.
	uint32_t  height;		///< Image Height.
	uint32_t  texID;		///< Texture ID Used To Select A Texture.
	uint32_t  type;			///< Image Type, fe HFE_TGA_RGBA.
	uint32_t  nBytes;
} HfSTgaTexture;

/// \cond DOXYGEN_SKIP //{

/// TGA header.
typedef struct {
	uint8_t  header[6];		///< First 6 Useful Bytes From The Header
	uint32_t bytesPerPixel;	///< Holds Number Of Bytes Per Pixel Used In The TGA File
	uint32_t imageSize;		///< Used To Store The Image Size When Setting Aside Ram
	uint32_t temp;			///< Temporary Variable
	uint32_t type;
	uint32_t Height;		///< Height of Image
	uint32_t Width;			///< Width ofImage
	uint32_t Bpp;			///< Bits Per Pixel
} HfSTga;

/// TGA File Header.
typedef struct
{
	uint8_t Header[12];
} HfSTgaHeader;

// instaniated in cpp
extern uint8_t hf_UncmprTgaMagic[12]; //= {0,0,2,0,0,0,0,0,0,0,0,0};

/// \endcond //DOXYGEN_SKIP //}

bool hf_LoadUncompressedTGA( HfSTgaTexture* texture, const char* filename, uint32_t flags2, std::string* err );
bool hf_FreeTGA( HfSTgaTexture* tex );
bool hf_SaveUncompressedTGA( const char* filename, uint16_t w, uint16_t h, int fmt2, const void* data3, std::string* err = 0 );

} // end namespace hef

#endif //_HEF_DATA_H_
