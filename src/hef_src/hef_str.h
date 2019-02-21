
#ifndef _HEF_STR_H_
#define _HEF_STR_H_
#include <string>
#include <vector>
#include <algorithm>
#include <stdint.h> //uint64_t
namespace hef{
;
/// \cond DOXYGEN_SKIP //{
void str_general_tester();
/// \endcond //DOXYGEN_SKIP //}
;

// Function prototypes.
bool hf_isAlpha( int in );
bool hf_isAlphanum( int in );
void hf_ConvertCrLfToLf( std::vector<unsigned char>* data2 );
bool hf_GetTimeHMSMsFromStr( const char* inp, int* hr2, int* mn2, int* sc2, int* ms2, int flags2, const char** endp );
void hf_TimeMsecsDecomposeToHMSMs( uint64_t inp, int* hr3, int* mn3, int* sc3, int* ms3 );

/// Determines length of c-string (null-terminated string).
template<class T>
int hf_strlen( const T* in )
{
	const T* sz = in;
	for(; *sz; sz++ );
	return sz - in;
}
/// \sa GP_strchr_etc
template<class T>
const T* hf_strchr( const T* sz, const T& ch )
{
	T a;
	for(; (a = *sz); sz++ )
		if( a == ch )
			return sz;
	return 0;
}
/// \sa GP_strchr_etc
template<class T>
const T* hf_strrchr( const T* in, const T& ch )
{
	int len = hf_strlen( in );
	if(len){
		const T* sz = &in[len];
		for( sz--; sz >= in; sz-- ){
			if( *sz == ch )
				return sz;
		}
	}
	return 0;
}
/// \defgroup GP_strchr_etc String find by character or characters functions.
/// ...
/// hf_strchr() \n
/// hf_strrchr() \n
/// hf_strpbrk() \n
/// hf_strspn() \n

/// The function hf_strpbrk() returns a pointer to the first occurrence in
/// str1 of any character in str2, or NULL if no such characters are present.
/// \sa GP_strchr_etc
template<class T>
const T* hf_strpbrk( const T* str1, const T* str2 )
{
	const T* a, *b;
	for( a = str1; *a; a++ )
		for( b = str2; *b; b++ )
			if( *a == *b )
				return a;
	return 0;
}
/// The hf_strspn() function returns a pointer to the first character in str1
/// that doesn't match any character in str2 or NULL if not found.
/// Note: strspn() is different in that it returns length.
/// \sa GP_strchr_etc
template<class T>
const T* hf_strspn( const T* str1, const T* str2 )
{
	const T* a, *b;
	for( a = str1; *a; a++ )
		for( b = str2; *b; b++ )
			if( *a != *b )
				return a;
	return 0;
}

enum{
	/// Flags for hf_GetTimeHMSMsFromStr().
	HF_EGHMS2_MilisecsRequired = 0x1,
	HF_EGHMS2_SecondsRequired = 0x2,
	HF_EGHMS2_MinutesRequired = 0x4,
	HF_EGHMS2_HoursRequired = 0x8,
	HF_EGHMS2_NoBoundaryCheck = 0x10,
};

/**
	\defgroup GP_strtrims String trim functions
	...
	hf_rtrim() \n
	hf_trim() \n
	hf_ltrim() \n
	hf_ltrim_stdstring() \n
	hf_rtrim_stdstring() \n
	hf_trim_stdstring() \n
	HfCStrPiece::ltrim() \n
	HfCStrPiece::rtrim() \n
	HfCStrPiece::trim() \n
	HfCStrPiece::reduceLinesDepth() \n
*/
;
/// Returns new position in left trimmed string.
/// \sa GP_strtrims
template<class T>
const T* hf_ltrim( const T* in, const T* szCharset )
{
	int i;
	for( i=0; in[i] && hf_strchr( szCharset, in[i] ); i++  );
	return &in[i];
}
/// Returns new string length, after right trim operation.
/// \sa GP_strtrims
template<class T>
int hf_rtrim( const T* in, const T* szCharset )
{
	int len = hf_strlen(in);
	int c = 0;
	const T* b = &in[len];
	b--;
	for(; b>=in && hf_strchr( szCharset, *b ); b-- )
		c++;
	return len - c;
}
/// Trims string from both sides.
/// \sa GP_strtrims
template<class T>
const T* hf_trim( const T* in, const T* szCharset, int* length_out )
{
	const T* str = hf_ltrim( in, szCharset );
	*length_out = hf_rtrim( str, szCharset );
	return str;
}

/// Outputs left trimmed STD string.
/// \sa GP_strtrims
template<class T>
std::basic_string<T> hf_ltrim_stdstring( const T* in, const T* whitespacescharset )
{
	return hf_ltrim( in, whitespacescharset );
}
/// Outputs right trimmed STD string.
/// \sa GP_strtrims
template<class T>
std::basic_string<T> hf_rtrim_stdstring( const T* in, const T* whitespacescharset )
{
	std::basic_string<T> str = in;
/*	typename std::basic_string<T>::reverse_iterator a;
	//a = std::basic_string<T>::reverse_iterator( str.erase( (++a).base() ) );
	typename std::basic_string<T>::reverse_iterator tmp;
	for( a = str.rbegin(); a != str.rend(); ){
		if( hf_strchr( whitespacescharset, *a ) ){
			//a = std::basic_string<T>::reverse_iterator( str.erase( (++a).base() ) );
			tmp = str.erase( (++a).base() );
			a = tmp;
		}else{
			break;
		}
	}
	return str;//*/
	int endpos = (int)str.size();
	for( endpos--; endpos > -1; endpos-- ){
		if( hf_strchr( whitespacescharset, str[endpos] ) ){
			continue;
		}
		break;
	}
	return str.substr( 0, endpos+1 );
}
/// Outputs trimmed STD string.
/// \sa GP_strtrims
template<class T>
std::basic_string<T> hf_trim_stdstring( const T* in, const T* whitespacescharset )
{
	std::basic_string<T> str;
	int size2 = 0;
	const T* sz = hf_trim( in, whitespacescharset, &size2 );
	str.assign( sz, size2 );
	return str;
}

/**
	\defgroup GP_strcmp_funcs String compare functions
	...
	hf_strcmp() \n
	hf_strcasecmp() \n
	hf_stroptcmp() \n
	hf_StrStrPred() \n
	hf_strstr() \n
	hf_stristr() \n
	hf_strstrnn() \n
	hf_StrCmpPred() \n
	hf_strEndsWith() \n
	hf_strBeginsWith() \n
	HfCStrPiece::equalTo() \n
	HfCStrPiece::find() \n
	HfCStrPiece::strpos() \n
	HfCStrPiece::matchHead (2+ functions) \n
	HfCStrPiece::matchHead \n
	HfCStrPiece::matchTail() \n
	hf_strcmp_qm() \n
	hf_MatchStrSimple() \n
	HfRegEx::match2() \n
*/
;
/// \sa GP_strcmp_funcs
template<class T>
int hf_strcmp( const T* sz1, const T* sz2, int len = -1 )
{
	if(len){
		for(; *sz1 == *sz2 && *sz1 && --len; sz1++, sz2++ );
		if( *sz1 != *sz2 )
			return *sz1 < *sz2 ? -1: 1;
	}
	return 0;
}

/// Interface class for character predicate.
/// \sa hf_StrCmpPred()
template<class T>
class HfICharPredicate {
public:
	virtual bool lessThan( const T& a, const T& b )const = 0;
	virtual bool equal( const T& a, const T& b )const = 0;
};
/// \cond DOXYGEN_SKIP //{

/// Default character predicate implementation.
template<class T>
class HfCCharDfltPred : public HfICharPredicate<T> {
public:
	virtual bool lessThan( const T& a, const T& b )const{
		return a < b;
	}
	virtual bool equal( const T& a, const T& b )const{
		return a == b;
	}
};
/// \endcond //DOXYGEN_SKIP //}
;

/// String comparison with predicate class.
/// \sa GP_strcmp_funcs
template<class T>
int hf_StrCmpPred( const T* sz1, const T* sz2, int len,
				 const HfICharPredicate<T>& ipredicate )
{
	if(len){
		for(; ipredicate.equal( *sz1, *sz2 ) && *sz1 && --len; sz1++, sz2++ );
		if( !ipredicate.equal( *sz1, *sz2 ) )
			return ipredicate.lessThan( *sz1, *sz2 ) ? -1: 1;
	}
	return 0;
}
template<class T> T hf_GetLowerThroughASCII( const T& inp )
{
	static const char* lwrs = "abcdefghijklmnopqrstuvwxyz";
	static const char* uprs = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char a = (char)inp;
	if( a >= 'A' && a <= 'Z' ){
		const char* pos;
		if( (pos = hf_strchr<char>( uprs, a )) ){
			int idx = pos - uprs;
			return (T) ( lwrs[idx] );
		}
	}
	return inp;
}
/// \cond DOXYGEN_SKIP //{

/// Predicate class for case-insensitive comparision.
template<class T>
class HfCCharCaseInsensitivePred : public HfICharPredicate<T> {
public:
	virtual bool lessThan( const T& a, const T& b )const{
		return hf_GetLowerThroughASCII(a) < hf_GetLowerThroughASCII(b);
	}
	virtual bool equal( const T& a, const T& b )const{
		return hf_GetLowerThroughASCII(a) == hf_GetLowerThroughASCII(b);
	}
};
/// \endcond //DOXYGEN_SKIP //}
;
/// Case-insensitive string comparison.
/// \sa GP_strcmp_funcs
template<class T>
int hf_strcasecmp( const T* sz1, const T* sz2, int len = -1 )
{
	HfCCharCaseInsensitivePred<T> pred;
	return hf_StrCmpPred<T>( sz1, sz2, len, pred );
}

/// hf_strstr() but with options.
/// \sa GP_strcmp_funcs
template<class T>
int hf_stroptcmp( const T* sz1, const T* sz2, int len = -1, bool bCaseInsensitive=0 )
{
	if(bCaseInsensitive){
		HfCCharCaseInsensitivePred<T> pred;
		return hf_StrCmpPred<T>( sz1, sz2, len, pred );
	}else{
		HfCCharDfltPred<T> pred;
		return hf_StrCmpPred<T>( sz1, sz2, len, pred );
	}
}
/// Predicate version of hf_strstr().
/// \sa GP_strcmp_funcs
template<class T>
const T* hf_StrStrPred( const T* str1, const T* str2, const HfICharPredicate<T>& pred )
{
	size_t       len1 = hf_strlen( str1 );
	const size_t len2 = hf_strlen( str2 );
	for(; len1 && len1 >= len2; str1++, len1-- ){
		if( !hf_StrCmpPred<T>( str1, str2, std::min<size_t>(len1,len2), pred ) )
			return str1;
	}
	return 0;
}
/// Find first occurernce of str2 in str1.
/// \sa GP_strcmp_funcs
template<class T>
const T* hf_strstr( const T* str1, const T* str2 )
{
	HfCCharDfltPred<T> pred;
	return hf_StrStrPred<T>( str1, str2, pred );
}
// Case-insensitive hf_strstr().
/// \sa GP_strcmp_funcs
template<class T>
const T* hf_stristr( const T* str1, const T* str2 )
{
	HfCCharCaseInsensitivePred<T> pred;
	return hf_StrStrPred<T>( str1, str2, pred );
}

/// Finds first occurence of str2 inside str1, returns 0 on failure.
/// \sa GP_strcmp_funcs
template<class T>
const T* hf_strstrnn( const T* str1, size_t len1, const T* str2, size_t len2,
		   bool bCaseInsensitive )
{
	//tcsstrnn_i
	for(; len1 && len1>=len2; str1++, len1-- )
		if( !hf_stroptcmp( str1, str2, len2, bCaseInsensitive ) )
			return str1;
	return 0;
}

/// Checks whenever input c-string ends with specified c-string.
/// \sa GP_strcmp_funcs
template<class T>
bool hf_strEndsWith( const T* inp, const T* needle2, bool bCaseInsensitive=0,
					int maxlen=-1 )
{
	int ln2 = hf_strlen( needle2 );
	int ln3 = hf_strlen( inp );
	if( maxlen >=0 && ln2 >= maxlen )
		ln2 = maxlen;
	while( ln2-- ){
		if( !ln3-- )
			return 0;
		if( bCaseInsensitive ){
			if( hf_strcasecmp( &inp[ln3], &needle2[ln2], 1 ) )
				return 0;
		}else{
			if( inp[ln3] != needle2[ln2] )
				return 0;
		}
	}
	return 1;
}
/**
	Checks if input string begins with another string.
	\param inp - input string
	\param len2 - length or -1 for auto.
	\param needle2 - string to check.
	\param len3 - length or -1 for auto.
	\param bCaseInsensitive - set to 1 to use ANSI case insensitive mode.
*/
template<class T>
bool hf_strBeginsWith( const T* inp, int len2, const T* needle2, int len3 = -1,
						bool bCaseInsensitive=0 )
{
	len2 = ( len2 != -1 ? len2 : hf_strlen( inp ) );
	len3 = ( len3 != -1 ? len3 : hf_strlen( needle2 ) );
	if( len2 < len3 )
		return 0;
	int i;
	for( i=0; i<len3; i++ ){
		if( bCaseInsensitive ){
			if( hf_strcasecmp( &inp[i], &needle2[i], 1 ) )
				return 0;
		}else{
			if( inp[i] != needle2[i] )
				return 0;
		}
	}
	return 1;
}

/// Length adaptor class for speifying either valid length or infinite lenghtes.
class HfCLengthAdaptor
{
	bool bLengthOmited, bLenNeg; int nLength;
public:
	HfCLengthAdaptor() : bLengthOmited(1)
	{}
	HfCLengthAdaptor( int len ) : bLengthOmited(0), nLength(len){
		if(nLength < 0){
			nLength *= -1;
			bLenNeg = 1;
		}else{
			bLenNeg = 0;
		}
	}
	bool isLengthOmited()const { return bLengthOmited; }
	bool isLengthNegative()const { return bLenNeg; }
	int  length()const { return nLength; }
};

/**
	\defgroup GP_str_substr Sub-String functions
	...
	hf_substr() \n
	HfTArgs::substr() \n
	HfCStrPiece::getSubstrCount() \n
*/
;
/**
	Returns the portion of input string specified by the 'start' and
	'length' parameters.

	in - The input string

	start -
		If start is non-negative, the returned string will start at the start'th
		position in string, counting from zero. For instance, in the string 'abcdef',
		the character at position 0 is 'a', the character at position 2 is 'c', and so forth.

		If start is negative, the returned string will start at the start'th character from
		the end of string.

		If string is less than or equal to start characters long, empty string
		will be returned.

	length -
		If length is given and is positive, the string returned will contain at most
		length characters beginning from start (depending on the length of string).

		If length is given and is negative, then that many characters will be omitted
		from the end of string

	\sa GP_truncate_fncs
	\sa GP_str_substr
*/
template<class T>
std::basic_string<T> hf_substr( const T* in, int start,
							   const HfCLengthAdaptor& length = HfCLengthAdaptor() )
{
	if( !length.isLengthOmited() && !length.length() )
		return std::basic_string<T>();
	int len = hf_strlen(in);
	const T* sz2;
	if( start >= 0 ){
		if( start > len )
			return std::basic_string<T>();
		sz2 = &in[start];
	}else{
		start *= -1;		//abs.
		start = std::min<int>( start, len );
		sz2 = &in[ len - start ];
	}
	len = hf_strlen(sz2);
	//hf_ltruncate
	if( !length.isLengthOmited() ){
		if( !length.isLengthNegative() ){
			len = length.length();
		}else{
			len = hf_strlen(sz2);
			if( len < length.length() ){
				len = 0;
			}else{
				len = std::min<int>( len, len - length.length() );
			}
		}
	}
	return std::basic_string<T>( sz2, len );
}

/**
	\defgroup GP_dirname_and_fn Directory and file name
	...
	hf_dirname() \n
	hf_basename() \n
	hf_basename2() \n
	hf_basename3() \n
	HfCStrPiece::pathToPieces() \n
	HfCStrPiece::piecesToPath() \n
	hf_PathTruncateLeftSide() \n
	hf_ltruncate_path() \n
	hf_getcwd() \n
	hf_GetRelativePath() \n
	hf_getHomeDir() \n
*/
;
/// Converts multibyte c-string to any template specific c-string.
/// \sa hf_AsciiStr()
template<class T>
std::basic_string<T> hf_anyStr( const char* in, int size = -1 )
{
	std::basic_string<T> out;
	int i, size2 = size<0 ? (int)hf_strlen(in) : size;
	for( i=0; i<size2; i++ )
		out += (T)( in[i] );
	return out;
}

/**
	\defgroup GP_truncate_fncs Truncate String Functions
	...
	hf_ltruncate() \n
	hf_rtruncate() \n
	hf_ltruncate_path() \n
	HfCStrPiece::truncate() \n
	hf_DirSubstr() \n
	hf_substr() \n
	hf_PathTruncateLeftSide() \n
*/


/**
	Left truncate string.
	Prepends output with "..." if input string truncated.
	\param length2 - desired length.
	\param szDots  - optionl, NULL defaults to "...".

	\sa GP_truncate_fncs
*/
template<class T> std::basic_string<T>
hf_ltruncate( const T* inp, int nOutpLength, const T* szDots = 0 )
{
	int nNegStart = -1 * std::abs( nOutpLength );
	std::basic_string<T> str = hf_substr( inp, nNegStart );
	if( str.size() == (size_t)hf_strlen(inp) )
		return str;
	static std::basic_string<T> dots2 = ( szDots ? szDots : hf_anyStr<T>("...",-1) );
	if( dots2.size() >= str.size() ){
		return str;
	}else{
		// Eg: str = "..." + str.substr(3,-1)
		return ( dots2 + str.substr( dots2.size() ) );
	}
}
/**
	Right truncate string.
	\sa GP_truncate_fncs

	<img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAUA
	AAAFCAYAAACNbyblAAAAHElEQVQI12P4//8/w38GIAXDIBKE0DHxgljNBAAO
	9TXL0Y4OHwAAAABJRU5ErkJggg==" alt="Red dot" />
*/
template<class T> std::basic_string<T>
hf_rtruncate( const T* inp, int nOutpLength, const T* szDots = 0 )
{
	int length3 = std::abs( nOutpLength );
	std::basic_string<T> str = hf_substr( inp, 0, length3 );
	if( (int)str.size() == (int)hf_strlen(inp) )
		return str;
	static std::basic_string<T> dots2 = ( szDots ? szDots : hf_anyStr<T>("...",-1) );
	if( dots2.size() >= str.size() ){
		return str;
	}else{
		// Eg: str = str.substr(nn,mm) + "..."
		return ( str.substr( 0, str.size() - dots2.size() ) + dots2 );
	}
}
/**
	Truncates path string from left side to desired length.
	If truncation happens, output is prepended with "...".
	Additionallly, any incomplete dir names from left side are truncated.ĵ
	\param szDots - optional, NULL defaults to "...".

	\return Truncated path string of length equal or less to 'nOutpLength'.
			If leading dots were prepended, length is still guaranted
			equal or less to 'nOutpLength'.

	\sa GP_truncate_fncs
	\sa GP_dirname_and_fn
*/
template<class T> std::basic_string<T>
hf_ltruncate_path( const T* inp, int nOutpLength, const T* szDots = 0 )
{
	int nNegStart = -1 * std::abs( nOutpLength );
	std::basic_string<T> str = hf_substr( inp, nNegStart );
	if( str.size() == (size_t)hf_strlen(inp) )
		return str;
	std::basic_string<T> dots2 = ( szDots ? szDots : hf_anyStr<T>("...",-1) );
	if( dots2.size() >= str.size() ){
		return str;
	}else{
		// Note: Windows full path variation is handled properly since full input
		//       string is never passed till this point and truncation is from
		//       the left.
		static std::basic_string<T> slashes2 = hf_anyStr<T>("/\\",-1);

		// scan until first slash/backslash characcter.
		size_t i; bool bSlsh2 = 0;
		for( i=0; i<str.size(); i++ ){
			if( hf_strchr( slashes2.c_str(), str[i] ) ){ // if character is '/' or '\\'.
				str = str.substr( i );
				bSlsh2 = 1;
				break;
			}
		}
		if( bSlsh2 && dots2.size() <= i ){ // if removed enough chars to prepend dots to unchanged.
			return dots2 + str;
		}else{
			// remove space for leading dots.
			str = str.substr( dots2.size() );
			// Eg: str = "..." + str.substr(3,-1)
			return dots2 + str;
		}
	}
}

/// Returns directory part given path.
/// Returns "." if input string is zero length.
/// \sa GP_dirname_and_fn
template<class T>
std::basic_string<T> hf_dirname( const T* in )
{
	const T *sz;
	if( *in == (T) 0 )
		return hf_anyStr<T>(".");
	//sz = hf_max( hf_strrchr( in, (T)'\\' ), hf_strrchr( in, (T)'/' ) );
	sz = std::max( hf_strrchr( in, (T)'\\' ), hf_strrchr( in, (T)'/' ) );
	if( sz ){
		return std::basic_string<T>( in, sz - in );
	}else{
		return in;
	}
}
/// Returns base name part (filename + extension) given path.
/// \param inp   - input file name.
/// \param extOu - optional. if set, always gets value assigned. at char pos of ext or at null byte.
/// \sa GP_dirname_and_fn
template<class T>
const T* hf_basename( const T* inp, const T** extOu=0 )
{
	const T *sz2, *sz3;
	sz2 = std::max( hf_strrchr( inp, (T)'\\' ), hf_strrchr( inp, (T)'/' ) );
	if( sz2 ){
		if( extOu ){
			sz3 = hf_strrchr( inp, (T)'.' );
			*extOu = ( ( sz3 && sz3 > sz2 ) ? sz3 + 1 : inp + hf_strlen(inp) );
		}
		return sz2 + 1;
	}
	if(extOu){
		sz3 = hf_strrchr( inp, (T)'.' );
		*extOu = ( sz3 ? sz3 + 1 : inp + hf_strlen(inp) );
	}
	return inp;
}



/// Flags for hf_basename2().
enum {
	EBNF_NOFLAGS = 0,
	/// If set, valid file extensions is stripped from output string.
	/// Valid extension is maximum 4 character long and is build
	/// only from ASCII alpha-numeric characters.
	EBNF_STRIP_VALID_EXT = 0x1,
};
/// Extended version of hf_basename().
/// \param in     - input file name.
/// \param flags2 - fe. \ref EBNF_NOFLAGS.
/// \sa GP_dirname_and_fn
template<class T>
std::basic_string<T> hf_basename2( const T* in, size_t flags2 = EBNF_NOFLAGS )
{
	std::basic_string<T> z = hf_basename(in);
	if( flags2 & EBNF_STRIP_VALID_EXT ){
		//std::basic_string<T> dot = hf_anyStr<T>(".");
		typename std::basic_string<T>::reverse_iterator a;
		size_t i; bool bHasExt = 0;
		for( a = z.rbegin(), i=0; a != z.rend() && i<5; ++a, i++ ){
			if( *a == (T)'.' ){
				bHasExt = 1;
				i += 1;
				break;
			}
			if( !hf_isAlphanum( (int) *a ) ){
				break;
			}
		}
		if(bHasExt){
			z = z.substr( 0, z.size() - i );
		}
	}
	return z;
}
template<class T>
bool hf_isStrAlphanum( const T* in )
{
	for(; *in; in++ ){
		if( !hf_isAlphanum( (int)*in ) ){
			return 0;
		}
	}
	return 1;
}

/// Enum types for hf_basename3().
enum {
	EBN3F_NOFLAGS = 0,
	EBN3F_EXTANYCHARS = 0x1,
};
/// Returns base-name and file extension (suffix) as std::pair (as two strings).
/// \param in - input string.
/// \param flags2 - see EBN3F_NOFLAGS.
/// \param uExtMaxLen -
/// \sa GP_dirname_and_fn
template<class T>
std::pair<std::basic_string<T>,std::basic_string<T> >
hf_basename3( const T* in, size_t flags2 = EBN3F_NOFLAGS, size_t uExtMaxLen = 4 )
{
	std::basic_string<T> str = hf_basename(in);
	if( !str.empty() ){
		const T* sz1 = str.c_str();
		const T* end = sz1 + hf_strlen(sz1);
		const T* sz2 = hf_strrchr( sz1, (T)'.' );
		if(sz2){
			sz2++;
			size_t uExtLen = end - sz2;
			if(uExtLen){
				if( uExtLen <= uExtMaxLen ){
					std::basic_string<T> strBase = str.substr( 0, (str.size() - uExtLen)-1 );
					std::basic_string<T> strExt  = str.substr( (str.size() - uExtLen) );
					if( flags2 & EBN3F_EXTANYCHARS || hf_isStrAlphanum( strExt.c_str() ) ){
						return std::pair<std::basic_string<T>,std::basic_string<T> >(
							strBase, strExt );
					}
				}
			}
		}
	}
	return std::pair<std::basic_string<T>,std::basic_string<T> >(str,"");
}

/// Reverses a string inside buffer given number of starting characters to perform reverse on.
template<class T>
T* hf_strrevn( T* in, size_t num )
{
	if( num > 1 ){
		T* const end  = in + num;
		T* const bgn  = in;
		T* const rbgn = end - 1;
		const size_t half = num / 2;
		size_t i;
		T* head, *tail, tmp;
		for( i=0; i<half; i++ ){
			head = bgn  + i;
			tail = rbgn - i;
			//
			tmp = *head;
			*head = *tail;
			*tail = tmp;
		}
	}
	return in;
}
/// Reverses a string inside buffer.
template<class T>
T* hf_strrev( T* in )
{
	return hf_strrevn<T>( in, hf_strlen( in ) );
}
template<class T>
T* hf_strcat( T* str1, const T* str2 )
{
	T* dst = str1 + hf_strlen( str1 );
	for(; *str2; dst++, str2++ ){
		*dst = *str2;
	}
	*dst = 0;
	return str1;
}
/// String copy.
/// \sa hf_strncpy()
template<class T>
T* hf_strcpy( T* to2, const T* from, bool bDoNullTerminate = 0 )
{
	T* dst = to2;
	for(;; from++, dst++ ){
		*dst = *from;
		if( *from == 0 )
			break;
	}
	if(bDoNullTerminate)
		*dst = 0;
	return to2;
}
/// The hf_strncpy function copies at most 'count' characters of 'fromm' to the
/// string 'to2'. If 'fromm' has less than 'count' characters, the remainder
/// is padded with '\0' characters. The return value is the resulting string ('to').
/// Note, hf_strncpy() does not NULL terminate the resulting string automatically!
template<class T>
T* hf_strncpy( T* to2, const T* fromm, size_t count )
{
	T* dst = to2;
	bool bNullReached = 0;
	for(; count; dst++, count-- ){
		if( !bNullReached ){
			*dst = *fromm;
			if( *fromm == 0 )
				bNullReached = 1;
			fromm++;
		}else{
			*dst = 0;
		}
	}
	return to2;
}
/// Converts any type c-string to ASCII string (aka. multibyte string).
/// \sa hf_anyStr()
template<class T>
std::string hf_AsciiStr( const T* inp, int size2 = -1 )
{
//	int i, size2 = size<0 ? (int)hf_strlen(in) : size;
	int i;
	size2 = ( size2 < 0 ? hf_strlen(inp) : size2 );
	std::string out;
	for( i=0; i<size2; i++ ){
		char c = (char)inp[i];
		out += c;
	}
	return out;
}
/**
	\defgroup GP_escaped_substr Escaped sub-string functions
	...
	hf_getEscapedSubString() \n
	HfCStrPiece::getEscapedSubString() \n
	HfCStrPiece::getEscapedSubString2() \n
	HfCStrPiece::findMatchingBrace() \n
*/

/**
	Extracts string from input string until terminator character or '\\0'.
	takes escape character into account. extracts string until terminator not
	includng terminator itself.
	\param chEscape      - escape character, set to '\\0' to ignore.
	\param szIn          - input string, function starts at begin of it
	\param length        - length of string or -1 to indicate null terminated.
	\param szTerminators - specify terminators character set as c-string. set to NULL to ignore.
	\return len of string, return value 0 indicates that nothing has been extracted.
	\sa GP_escaped_substr
*/
template<class T>
size_t hf_getEscapedSubString( T chEscape, const T* szIn, int length = -1, const T* szTerminators = 0 )
{
	bool bEscaped = 0; size_t i, nProcessed = 1; const T* sz;
	if(length == -1)
		length = (int) hf_strlen( szIn );
	for( sz = szIn, i=0; i<(size_t)length; sz += nProcessed, i++ ){
		const T ch = *sz;
		if( ch == chEscape && !bEscaped ){
			nProcessed = 1;
			bEscaped = 1;
			continue;
		}
		// consider terminators
		if( szTerminators && !bEscaped )
			if( hf_strchr( szTerminators, ch ) )
				break;
		nProcessed = 1;
		bEscaped = 0;
	}
	return sz - szIn;
}

/// Flags for hf_getCommandLineParameter().
enum {
	EGCLPF_NOFLAGS = 0,
	/// if set, checks only if specified argument exists
	/// and returns "1" string if so.
	EGCLPF_NOVALUE = 0x1,
	/// Argument name is matched without adding leading dash ('-') or
	/// slash ('/') character.
	EGCLPF_NOAUTODASH = 0x2,
	/// Turns case sensitive string comparision.
	EGCLPF_CS = 0x4,
};

/**
	\defgroup GP_cmd_line_args Command line functions
	...
	hf_getCommandLineParameter() \n
	hf_getCommandLineParameter2() \n
	HfCStrPiece::toCommandLineArgv() \n
	HfSCmdNames \n
*/

/// Input parameter for hf_getCommandLineParameter2().
/// Adaptor from different string arrays/contaners.
template<class T> struct HfSCmdNames {
	std::vector<std::basic_string<T> > Names;
	/// Default constructor.
	HfSCmdNames() {}
	//
	HfSCmdNames( const T* name1, const T* n2=0, const T* n3=0, const T* n4=0, const T* n5=0, const T* n6=0 ) {
		Names.push_back( name1 );
		if(n2){
			Names.push_back( n2 );
			if(n3){
				Names.push_back( n3 );
				if(n4){
					Names.push_back( n4 );
					if(n5){
						Names.push_back( n5 );
						if(n6){
							Names.push_back( n6 );
						}
					}
				}
			}
		}
	}
	/// Constructs from array that can be the argv/argc one.
	HfSCmdNames( const T*const* lpArray, int size = -1 ){
		int i;
		for( i=0; i<size || size<0; i++ ){
			const T* sz = lpArray[i];
			if( !sz || !*sz)
				break;
			Names.push_back(sz);
		}
	}
	HfSCmdNames( const std::vector<const T*>& rArray ){
		typename std::vector<const T*>::const_iterator a;
		for( a = rArray.begin(); a != rArray.end(); ++a )
			Names.push_back(*a);
	}
	HfSCmdNames( const std::vector<std::basic_string<T> >& rArray, char* ){
		typename std::vector<std::basic_string<T> >::const_iterator a;
		for( a = rArray.begin(); a != rArray.end(); ++a )
			Names.push_back(a->c_str());
	}

//	template<class U> HfSCmdNames( const U& rArray, char*, char* ){
//		U::const_iterator a;
//		;
//	}
	template<typename U>
	HfSCmdNames<T>& setArray( U& rStrArray ){
		typename U::const_iterator a;
		for( a = rStrArray.begin(); a != rStrArray.end(); ++a ){
			std::basic_string<T> str2 = *a;
			Names.push_back( str2 );
		}
		return *this;
	}
};

/// String command names typedef for 8-bit character type.
typedef HfSCmdNames<char> HfScn;

/// Retrieves value of command line parameter given 'argc' and 'argv'.
/// Comparision is case insensitive.
/// Returns value of specified command line parameter or empty string
/// (or default if specified by 'szDflt').
/// \param flags2 - flags fe. \ref EGCLPF_NOFLAGS.
/// \sa EGCLPF_NOFLAGS
/// \sa GP_cmd_line_args
template<class T> std::basic_string<T>
hf_getCommandLineParameter( const T* name, int argc, const T*const* argv,
                        int flags2 = EGCLPF_NOFLAGS, const T* szDflt=0 )
{
	bool match2 = 0;
	int i = 0;
	for(; i<argc; i++ ){
		if( flags2 & EGCLPF_NOAUTODASH ){
			if( !hf_stroptcmp<T>( name, argv[i], -1, !(flags2 & EGCLPF_CS) ) ){
				match2 = 1;
			}
		}else{
			if(  argv[i][0] == (T)'-' || argv[i][0] == (T)'/' ){
				if( !hf_stroptcmp<T>( name, &argv[i][1], -1, !(flags2 & EGCLPF_CS) ) ){
					match2 = 1;
				}
			}
		}
		if(match2){
			if( flags2 & EGCLPF_NOVALUE )
				return hf_anyStr<T>("1");
			if( i + 1 < argc ){
				return argv[i+1];
			}else
				return hf_anyStr<T>("");
		}
	}
	return ( szDflt ? szDflt : hf_anyStr<T>("") );
}
/// Multiple parameter name version of hf_getCommandLineParameter().
/// \param flags2 - flags fe. \ref EGCLPF_NOFLAGS.
/// \sa HfSCmdNames<T>
/// \sa hf_getCommandLineParameter()
/// \sa GP_cmd_line_args
template<class T> std::basic_string<T>
hf_getCommandLineParameter2( const HfSCmdNames<T>& names, int argc, const T*const* argv,
                        int flags2 = EGCLPF_NOFLAGS )
{
	std::basic_string<T> str;
	typename std::vector<std::basic_string<T> >::const_iterator a;
	for( a = names.Names.begin(); a != names.Names.end(); ++a ){
		if(!(str = hf_getCommandLineParameter( a->c_str(),argc,argv,flags2)).empty()){
			return str;
		}
	}
	return std::basic_string<T>();
}

/**
	Extracts commandline argument given argument name(s).
	Returns empty string if not found.

	\code
		// Example:
		std::string str2 = hf_getCommandLineParameter3( arr, arr2, 0, "1" );
		// Example 2:
		str2 = hf_getCommandLineParameter3( HfScn("-b","--action").Names, arr2, 0, "1" );
	\endcode

	\param names2 - one or more names of the argument to extract.
	\param argv2  - fe. argv array that has been passed to the executable.
	\param bNoValue - if set indicates argument has no value, returned is 'szDfltTrue' string.
	\param szDfltTrue - typically set to "1".
*/
template<typename T, typename T2, typename T3> std::basic_string<T>
hf_getCommandLineParameter3( const T2& names2, const T3& argv2, bool bNoValue, const T* szDfltTrue )
{
	std::basic_string<T> str;
	typename T2::const_iterator a;
	typename T3::const_iterator b;
	for( a = names2.begin(); a != names2.end(); ++a ){
		for( b = argv2.begin(); b != argv2.end(); ++b ){
			if( *a == *b ){
				if( bNoValue )
					return szDfltTrue;
				++b;
				if( b != argv2.end() )
					return *b;
				break;
			}
		}
	}
	return std::basic_string<T>();
}


enum{
	EGCLFSF_NOFLAGS = 0,
	/// Setting this flag causes checking 1st argument too.
	/// By default, first argument (index 0) is skipped
	/// (index 0 is assumed to be application file name.)
	EGCLFSF_DONTSKIPFIRST = 0x1,
};
/// Returns file-item strings from command line.
/// File strings are items that arent prefixed with '-' or '/'.
/// \param flags2 - flags fe. \ref EGCLFSF_NOFLAGS.
/// \sa GP_cmd_line_args
/// \sa EGCLFSF_NOFLAGS
template<class T>
//std::basic_string<T>
std::vector<std::basic_string<T> >
hf_getCommandLineFileStrings( int argc, const T*const* argv, size_t flags2 = EGCLFSF_NOFLAGS )
{
	std::vector<std::basic_string<T> > out;
	int i = ( flags2 & EGCLFSF_DONTSKIPFIRST ? 0 : 1 );
	for(; i<argc; i++ ){
		if( argv[i][0] != (T)'-' && argv[i][0] != (T)'/' ){
			out.push_back( argv[i] );
		}
	}
	return out;
}

enum{
	/// Flags for hf_swapCharacterCaseAsAscii().
	ESCCAAF_NoFlags = 0,
	ESCCAAF_ToLower = 0x1,	///< Defaults if to-upper is not set.
	ESCCAAF_ToUpper = 0x2,
};
/// Outputs lowercase or uppercase STD string.
/// \param flags2 - flags fe. \ref ESCCAAF_NoFlags.
template<class T> std::basic_string<T>
hf_swapCharacterCaseAsAscii( const T* in, int size = -1, size_t flags2 = ESCCAAF_NoFlags )
{
	std::basic_string<T> str;
	int i, size2 = size<0 ? (int)hf_strlen(in) : size;
	const char* szL = "abcdefghijklmnopqrstuvwxyz";
	const char* szU = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const char* sz, *szSrc, *szDst; int pos;
	szSrc = flags2 & ESCCAAF_ToUpper ? szL : szU;	//auto ESCCAAF_ToLower is used.
	szDst = flags2 & ESCCAAF_ToUpper ? szU : szL;
	for( i=0; i<size2; i++ ){
		char ch = hf_AsciiStr( &in[i], 1 )[0];
		if( (sz = hf_strchr( szSrc, ch )) ){
			pos = sz - szSrc;
			//char ch2 = szDst[pos];
			str += hf_anyStr<T>( &szDst[pos], 1 );
		}else{
			str += hf_anyStr<T>( &ch, 1 );
		}
	}
	return str;
}
template<class T>
std::basic_string<T> hf_toLowerAsAscii( const T* in, int size = -1 )
{
	return hf_swapCharacterCaseAsAscii( in, size, ESCCAAF_ToLower );
}
template<class T>
std::basic_string<T> hf_toUpperAsAscii( const T* in, int size = -1 )
{
	return hf_swapCharacterCaseAsAscii( in, size, ESCCAAF_ToUpper );
}

template<class T>
std::basic_string<T> hf_generateStr( const T* in, int num )
{
	std::basic_string<T> z; int i;
	for( i=0; i<num; i++ )
		z += in;
	return z;
}

/// Truncates path from left side, either or both by number of characters or
/// number of path parts.
/// \param nCharsMax - optional, -1 to ignore, max_ number of characters.
///                    should be greater than length of 'szDots' parameter.
/// \param nPartsMax - optional, -1 to ignore, max_ number of path parts to
///                    keep at right side.
/// \param szDots    - usually "...", a string that is being prepended if truncation occurs.
/// \param bTruncated - optional, is set to true if actual truncation happened.
/// \param szPathMode - path separator, 1 or more characters, "/" on unix and "\\/" on Windows.
///
/// \sa GP_dirname_and_fn
template<class T> std::basic_string<T>
hf_PathTruncateLeftSide( const T* szPath, int nCharsMax, int nPartsMax,
						const T* szDots, bool* bTruncated, const T* szPathMode )
{
	std::basic_string<T> strPathMode = szPathMode;
	std::vector<std::basic_string<T> > parts2; bool bRootPath = 0;
	std::basic_string<T> strBn, strOu, slash2 = strPathMode.substr(0,1);
	bool bTruncated3, *bTruncated2 = ( bTruncated ? bTruncated : &bTruncated3 );
	*bTruncated2 = 0;
	if( *szPath && hf_strchr( strPathMode.c_str(), *szPath ) ){
		// if path begins with dir character, fe. absolute path on unix with "/" character.
		bRootPath = 1;
	}
	{
		std::basic_string<T> str2 = hf_rtrim_stdstring( szPath, strPathMode.c_str() );
		for(; !str2.empty(); ){
			strBn = hf_basename( str2.c_str() );
			parts2.push_back( strBn );
			str2 = hf_dirname( str2.c_str() );
		}
	}
	typename std::vector<std::basic_string<T> >::const_iterator a;
	int i;
	for( i=0, a = parts2.begin(); (i<nPartsMax||nPartsMax<0) && a != parts2.end(); i++, ++a ){
		strOu = *a + (strOu.empty()?"":slash2.c_str()) + strOu;
	}
	if( i < (int)parts2.size() ){ // if truncating path parts.
		*bTruncated2 = 1;
		std::basic_string<T> dots2 = ( szDots && *szDots ? (szDots+slash2).c_str() : "" );
		if( !dots2.empty() ){
			int nSiz = dots2.size() + strOu.size();
			if( nCharsMax > -1 && nSiz > nCharsMax ){
				int nRemove = nSiz - nCharsMax;
				if( nRemove < strOu.size() ){
					strOu = dots2.substr(0,dots2.size()-1) + strOu.substr( nRemove>0 ? nRemove-1 : 0 );
				}else{
					int pos = ( strOu.size() > nCharsMax ? strOu.size() - nCharsMax : 0 );
					strOu = strOu.substr( pos );
				}
			}else{
				strOu = dots2 + strOu;
			}
			return strOu;
		}
	}else if( bRootPath ){
		strOu = slash2 + strOu;
	}
	if( nCharsMax > -1 && strOu.size() > nCharsMax ){
		*bTruncated2 = 1;
		std::basic_string<T> dots3 = szDots;
		int nSiz2 = dots3.size() + strOu.size();
		if( nSiz2 > nCharsMax ){
			int nRemove = nSiz2 - nCharsMax;
			if( nRemove < strOu.size() ){
				strOu = dots3 + strOu.substr( nRemove );
			}else{
				strOu = strOu.substr( strOu.size() - nCharsMax );
			}
		}else{
			strOu = dots3 + strOu.substr( strOu.size() - nCharsMax );
		}
	}
	return strOu;
}

/// String implode with container argument type deduction.
/// \sa GP_str_split_funcs
template <class T, class U> std::basic_string<T>
hf_implode( const U& inp, const T* glue )
{
	std::basic_string<T> outp;
	typename U::const_iterator a;
	int i;
	for( i=0, a = inp.begin(); a != inp.end(); ++a, i++ ){
		if( glue && i )
			outp += glue;
		outp += *a;
	}
	return outp;
}
/// String explode with container argument type deduction.
/// \param outp - output containter, fe. 'std::vector<std::string>'.
/// \param szTrimRL - optional, characters to trim from left and right.
/// \param nLimit - optional, set to -1 to not use. max number of spilts to do.
/// \sa GP_str_split_funcs
template <class T, class U> void
hf_explode( const T* inp, const T* szGlue, U& outp, const T* szTrimRL=0, int nLimit = -1 )
{
	std::basic_string<T> str;
	int lenGlue = hf_strlen<T>(szGlue);
	const T* sz2 = inp, *sz3;
	outp.clear();
	if( lenGlue ){
		for(; nLimit && (sz3 = hf_strstr<T>( sz2, szGlue )); sz2 = sz3 + lenGlue, nLimit-- ){
			str.assign( sz2, sz3-sz2 );
			if(szTrimRL)
				str = hf_trim_stdstring( str.c_str(), szTrimRL );
			outp.push_back( str );
		}
	}
	str = sz2;
	if(szTrimRL)
		str = hf_trim_stdstring( str.c_str(), szTrimRL );
	outp.push_back( str );
}


/// Splits string into sub-strings given sub-string size.
/// If negative value given, splits starting from siring's end.
/// outp - output container, type fe. "std::vector<std::string>".
/// \sa GP_str_split_funcs
template <class T, class U> void
hf_split( const T* inp, int nSubstrLength, U& outp )
{
	if( !nSubstrLength ){
		outp.push_back( inp );
	}else if( nSubstrLength > 0 ){
		int siz = hf_strlen( inp );
		const T* pos = inp, *endMax = inp + siz, *end;
		for(; pos < endMax; ){
			end = pos + nSubstrLength;
			end = std::min( end, endMax );
			outp.push_back( std::basic_string<T>( pos, end - pos ) );
			pos = end;
		}
	}else{	//else, splitting starts from right.
		nSubstrLength = nSubstrLength * -1;
		const T* rpos = inp + hf_strlen(inp), *pos;
		for(; rpos > inp; ){
			pos = rpos - nSubstrLength;
			pos = std::max( pos, inp );
			outp.insert( outp.begin(), 1, std::basic_string<T>( pos, rpos - pos ) );
			rpos = pos;
		}
	}
}
/**
	\defgroup GP_str_srch_replace String Search-Replace functions.
	...
	hf_strreplace() \n
	HfRegEx::subst2() \n
	HfCStrPiece::replace() (3+ methods) \n
	HfCStrPiece::reduceLinesDepth() \n
	HfCStrPiece::replace_ch() \n
	hf_StrReplaceOpCloseTokensCalb() \n
*/

/// Replace all occurrences of the search string with the replacement string.
/// \param nLimit - limit of replacements, -1 sets no limit.
/// \param ouCount - optional, output number of occurences.
/// \sa GP_str_srch_replace
template <class T> std::basic_string<T>
hf_strreplace( const T* szSearch, const T* szRepl, const T* szSubject, int nLimit = -1, int* ouCount = 0 )
{
	// str_replace( mixed $search , mixed $replace , mixed $subject [,int &$count] )
	std::basic_string<T> outp;
	const T* sz2 = szSubject, *sz3;
	int lenSrch = hf_strlen<T>(szSearch);
	if( lenSrch ){
		for(; nLimit && (sz3 = hf_strstr<T>( sz2, szSearch )); sz2 = sz3 + lenSrch, --nLimit ){
			outp += std::basic_string<T>( sz2, sz3-sz2 );
			outp += szRepl;
			if(ouCount) ++(*ouCount);
		}
	}
	outp += sz2;
	return outp;
}

enum{
	/// Flags for hf_strcmp_qm().
	/// Makes comparision case insensitive  (ANSI only).
	HF_ECMQ_CaseInsensitive = 0x1,
	/// Disable question mark.
	HF_ECMQ_DisableQstnmrk = 0x2,
};
/**
	Alternate version of hf_strcmp(), with '?' special character matching.
	\param len   -  Length or -1. set to -1 to match C-string, untill '\0' character.
	\param qmark -  Question mark character. Note that setting it to '\0'
	                does not disable it correctly in case if string contains
	                actual '\0'-s.
	\param flags2 - Flags fe. \ref HF_ECMQ_CaseInsensitive.
	\return - Returns 0 on successful match (otherwise -1 or 1).

	\code
		// Example:
		int rs4 = hf_strcmp_qm( "abc", "ab?", -1, '?', 0 );
		printf("matched:%d (%d)\n", !rs4, rs4 );
	\endcode
	\sa GP_strcmp_funcs
*/
template<class T> int
hf_strcmp_qm( const T* sz1, const T* sz2, int len, const T& qmark, int flags2 )
{
	if( len == -1 ){
		int len2;
		len  = hf_strlen( sz1 );
		len2 = hf_strlen( sz2 );
		if( len != len2 )
			return ( len < len2 ? -1: 1 );
	}
	if(len){
		bool bQmrk = !( flags2 & HF_ECMQ_DisableQstnmrk );
		if( !(flags2 & HF_ECMQ_CaseInsensitive) ){
			for(;; sz1++, sz2++ ){
				if( ( *sz1 == *sz2 || (bQmrk && *sz2 == qmark) ) && --len )
					continue;
				break;
			}
		}else{
			T c1, c2;
			for(; len; sz1++, sz2++, --len ){
				c1 = hf_GetLowerThroughASCII( *sz1 );
				c2 = hf_GetLowerThroughASCII( *sz2 );
				if( c1 == c2 || (bQmrk && c2 == qmark) )
					continue;
				break;
			}
		}
		if( *sz1 != *sz2 && *sz2 != qmark )
			return *sz1 < *sz2 ? -1: 1;
	}
	return 0;
}

/**
	Simple pattern-string matching with wide characters.
	By default, wide-match characters are set to asterisk ('*')
	and question mark ('?').
	Asterisk stands for zero or more occurences of any character.
	Question-mark stands for any character one time.

	\param inp      - Input subject string to search in.
	\param ptrn_str - String to search for, a pattern possibly containing wide-match characters.
	\param astchars - Optional, definition of two possible wide-match characters.
					  If set to NULL, those characters are assumed '*' and '?'.
					  astchars[0] specifies replacement for '*',
					  astchars[1] specifies replacement for '?'.
					  If this parameter is specified, it must point to an array of
					  at least two valid characters.

	To disable one or both of the two wide characters, set it to 0 on it's corresponding
	array position. Fe. pass astchars[]="\0?" to disable asteerisk, results only
	in '?' recognized and '*' not used.

	Note: Regarding RE terminology, pattern is considered anchored, if it doesn't
	      begin with an asterisk character.

	\verbatim
		Pattern notes:
		* Empty pattern always fails.
		* Single asterisk pattern matches any string.
		* Pattern set to "*?" matches any string of length greater or equal 1.
	\endverbatim
	\code
		// Example:
		const char* sz2 = "abcd";
		const char* pt2 = "*bc?";
		bool rs2 = hf_MatchStrSimple( sz2, pt2, (const char*)0 );
		printf("rs2:%d\n", (int)rs2 );
		// Example-2:
		// use '#' and '%' as special characters.
		const wchar_t* sz3 = L"defg";
		const wchar_t* pt3 = L"#ef%";
		const wchar_t* ast3 = L"#%";
		bool rs3 = hf_MatchStrSimple( sz3, pt3, ast3 );
		printf("rs3:%d\n", (int)rs3 );
	\endcode
	\sa GP_strcmp_funcs
*/
template <class T> bool
hf_MatchStrSimple( const T* inp, const T* ptrn_str, const T* astchars )
{
	if( !*ptrn_str )
		return 0;
	static const T spcchrs2[] = { (T)'*', (T)'?', 0, };
	bool bLeadingAst, bLastPtrn;
	astchars = ( astchars ? astchars : spcchrs2 );
	if( *ptrn_str == *astchars && *astchars && !ptrn_str[1] ) // quick handling if pattern is set to "*".
		return 1;
	const T szAst[] = { astchars[0], 0, };
	const T szQmrk[] = { astchars[1], 0, };
	std::vector<std::basic_string<T> > arr;
	typename std::vector<std::basic_string<T> >::iterator a;
	if( *szAst )
		hf_explode<T>( ptrn_str, szAst, arr, 0 );
	else
		arr.push_back( ptrn_str );
	//printf("arr size: %d\n", (int)arr.size() );
	bool bFirstAst = ( (!arr.empty() && arr.begin()->empty()) ? 1 : 0 );
	bool bLastAst  = ( (!arr.empty() && arr.back().empty()) ? 1 : 0 );

	// see if pattern ends with "*?".
	bool bHasTrailingAstQmrks = 0;
	if( arr.size() >= 2 && !arr.back().empty() ){
		// if last pattern string has only question marks.
		if( !hf_strspn( arr.back().c_str(), szQmrk ) ){
			bHasTrailingAstQmrks = 1;
		}
	}
	// remove empty elements, if any.
	for( a = arr.begin(); a != arr.end(); ){
		if( a->empty() ){ a = arr.erase(a); }
		else{ ++a; }
	}
	int i, num = (int)arr.size();
	for( i=0, a = arr.begin(); a != arr.end(); ++a, i++ ){
		bLastPtrn   = ( i+1 == num );
		bLeadingAst = ( !!i || bFirstAst );
		if( bLeadingAst ){
			if( bLastPtrn && bHasTrailingAstQmrks ){
				return ( ( hf_strlen(inp) >= a->size() ) ? 1 : 0 );
			}
			bool bFnd2 = 0;
			for(; *inp; inp++ ){
				if( !hf_strcmp_qm( inp, a->c_str(), a->size(), astchars[1], 0 ) ){
					inp += a->size();
					bFnd2 = 1;
					break;
				}
			}
			if( !bFnd2 )
				return 0;
		}else{
			if( !hf_strcmp_qm( inp, a->c_str(), a->size(), astchars[1], 0 ) ){
				inp += a->size();
			}else
				return 0;
		}
		if( bLastAst && bLastPtrn )
			return 1;
	}
	return !*inp;
}

/// Flags for hf_StrPrintTable().
enum{
	/// Denotes that first row element is a row of column titles.
	/// Additionally, separator row is also inserted below.
	HF_ESPT_CaptsEl0 = 0x1,
};
/**
	Strin ptints table given 2-dimensional container.

	\param inp          - Input table. inner element must be of type: "std::basic_string<T>".
	\param flags2       - Flags, fe \ref HF_ESPT_CaptsEl0.
	\param arSpcDashBar - Optional, array of 3 characters: space, dash and bar ("\x20-|").
	                      If present, must point to array of at least 3 elements.
	                      Fe. to turn off bar character, pass c-string: "\0\0\0".
	\param szNl         - Optional, newline c-string. Default is "\n".
	\param calbStrlen   - Optional callback for string length, fe. for UTF-8 strings.
	\code
		// Example:
		std::vector<std::vector<std::string> > arr;
		str = hf_StrPrintTable<char>( arr, 0x0, "\0\0|", 0, 0 );
	\endcode
*/
template<class T, class U> std::basic_string<T>
hf_StrPrintTable( const U& inp, int flags2, const T arSpcDashBar[3], const T* szNl, int(*calbStrlen)(const T*), const T* szIndent )
{
	std::basic_string<T> outp, str;
	typename U::const_iterator a;
	typename U::value_type::const_iterator b;
	std::vector<int> maxColLengths;
	int i,k;
	const T chSpc  = ( arSpcDashBar && arSpcDashBar[0] ? arSpcDashBar[0] : (T)'\x20' );
	const T chDash = ( arSpcDashBar && arSpcDashBar[1] ? arSpcDashBar[1] : (T)'-' );
	const T chBar  = ( arSpcDashBar ? arSpcDashBar[2] : (T)'|' );
	static const T szNl2[] = { (T)'\n', 0, };
	szNl = ( szNl ? szNl : szNl2 );
	for( a = inp.begin(); a != inp.end(); ++a ){
		for( i=0, b = a->begin(); b != a->end(); ++b, i++ ){
			if( maxColLengths.size() <= i )
				maxColLengths.push_back(0);
			int len = ( calbStrlen ? calbStrlen( b->c_str() ) : b->size() );
			maxColLengths[i] = std::max<int>( len, maxColLengths[i] );
		}
	}
	for( k=0, a = inp.begin(); a != inp.end(); ++a, k++ ){
		outp += (szIndent?szIndent:"");
		for( i=0, b = a->begin(); b != a->end(); ++b, i++ ){
			int len = ( calbStrlen ? calbStrlen( b->c_str() ) : b->size() );
			int numPadd = maxColLengths[i] - len;
			outp += *b;
			while( numPadd-- )
				outp.push_back(chSpc);
			if( !( i+1 == (int)a->size() ) ){ // if not last.
				outp.push_back(chSpc);
				if(chBar){
					outp.push_back(chBar);
					outp.push_back(chSpc);
				}
			}
		}
		if( k==0 && flags2 & HF_ESPT_CaptsEl0 ){
			outp += szNl;
			outp += (szIndent?szIndent:"");
			std::vector<int>::const_iterator c;
			for( i=0, c = maxColLengths.begin(); c != maxColLengths.end(); ++c, i++ ){
				str.clear();
				str.resize( *c, chDash );
				outp += str;
				if( !(i+1 == (int)maxColLengths.size()) ){ // if not last.
					outp.push_back(chSpc);
					if(chBar){
						outp.push_back(chDash);
						outp.push_back(chSpc);
					}
				}
			}
		}
		outp += szNl;
	}
	return outp;
}

/// Flags for hf_GetRelativePath().
enum{
	HF_EGRP_DotMarkIfCwd = 0x1,
	HF_EGRP_WinMode = 0x2,
};
/**
	Returns relative path given two paths.
	Double-dot directory names will be used to create path that
	represent a path as if a person would
	have to traverse in file manager from dir specified as 'from3'
	to reach dir 'to3'.
	The two input paths should not contain any relative dir names
	like dot or double-dot.
	Returned path may contain multiple consecutive double-dot entries.
	\param flags2 - flags, fe. \ref HF_EGRP_WinMode.

	Example:
	\code
		struct{
			const char* a, *b;
		} ssx[] = {
			{"/tmp/abc",
			 "/tmp/abc/xxx/yyy",},
			 // --> "xxx/yyy"
			{"/tmp/abc",
			 "/tmp/abc",},
			 // --> ""
			{"/tmp/abc/def/ghi",
			 "/tmp/abc",},
			 // --> "../.."
			{"/tmp/abc/def/ghi",
			 "/tmp/abc/xxx/yyy",},
			 // --> "../../xxx/yyy"
			{"/bin",
			 "/tmp/abc/xxx/yyy",},
			 // --> "../tmp/abc/xxx/yyy"
			{"/",
			 "/tmp/abc/xxx/yyy",},
			 // --> "/tmp/abc/xxx/yyy"
			{"/abc/def/ghi",
			 "/xxx",},
			 // --> "../../../xxx"
			{"/x/y",
			 "/a/b/c/d",},
			 // --> ../../a/b/c/d
			{"c:/abc",
			 "c:/abc/def",},
			 // --> ../../a/b/c/d
			{0,0,},
		};
		std::string strRel;
		for( int i=0; ssx[i].a; i++ ){
			strRel = hf_GetRelativePath( ssx[i].a, ssx[i].b, 0 );
			printf("a: [%s]\n", ssx[i].a );
			printf("b: [%s]\n", ssx[i].b );
			printf("strRel: [%s]\n", strRel.c_str() );
		}
	\endcode
	\sa GP_dirname_and_fn
*/
template<class T>
std::basic_string<T> hf_GetRelativePath( const T* from3, const T* to3, int flags2 )
{
	// TODO: str literals to templ.
	std::vector<std::basic_string<T> > from2, to2, out2, ar2;
	bool bAbsPathMode;
	const T szSl[] = { (T)'/', 0, }, szBsl[] = { (T)'\\',0,};
	const T szDots[] = { (T)'.', (T)'.', 0, }, szDotSl[] = { (T)'.',(T)'/',0,};
	{
		std::basic_string<T> from4, to4;
		if( flags2 & HF_EGRP_WinMode ){
			from4 = hf_strreplace( szBsl, szSl, from4.c_str(), -1, 0 );
			to4   = hf_strreplace( szBsl, szSl, to4.c_str(), -1, 0 );
		}
		bAbsPathMode = ( from3[0] == (T)'/' || to3[0] == (T)'/' );
		from4 = hf_trim_stdstring( from3, szSl );
		to4   = hf_trim_stdstring( to3, szSl );
		// must endup with empty array if path is an empty string or is a root dir ("/").
		if( !from4.empty() )
			hf_explode<T>( from4.c_str(), szSl, from2 );
		if( !to4.empty() )
			hf_explode<T>( to4.c_str(), szSl, to2 );
	}
	int i;
	for( i=0; i < (int)from2.size(); i++ ){
		if( i == (int)to2.size() || from2[i] != to2[i] )
			break;
	}
	ar2.clear();
	ar2.resize( ((int)from2.size()) - i, szDots );
	out2.insert( out2.end(), ar2.begin(), ar2.end() );
	//
	typename std::vector<std::basic_string<T> >::iterator a = to2.begin();
	std::advance( a, i );
	out2.insert( out2.end(), a, to2.end() );
	//
	std::basic_string<T> outp;
	outp = hf_implode( out2, szSl );
	outp = hf_rtrim_stdstring( outp.c_str(), szSl );
	if( bAbsPathMode && !i && !outp.empty() && outp[0] != (T)'/' && outp[0] != (T)'.' ){
		outp.insert( outp.begin(), (T)'/' );
	}else if( flags2 & HF_EGRP_DotMarkIfCwd && !outp.empty() && outp[0] != (T)'/' && outp[0] != (T)'.' ){
		outp = szDotSl + outp;
	}
	return outp;
}

/**
	String search-replace occurences given open-tag, close-tag and
	string getter function.
	using this function it is possible to search-replace object-like
	properties that are prefixed and suffixed by tokens, eg.
	by curly brackets.

	\param inp -
				input string.
	\param fnStrGetter -
				calback function that is called for every key occurence.
				Required declaration:
				\code
					std::string (*)( const char* szKey, bool& bKeepOriginal );
				\endcode

	NOTE: about C++ lambdas, in member function scope, it is required to
			not use pointer or reference type for 'fnStrGetter' parameter.
			otherwise, lambda wont be able to capture local or class scope
			variables,

	\verbatim
		Example #1
		--------------
		Given input string as:
			"./script.sh -s {uri.scheme} -h {uri.host} -p {uri.port}"

		Return-value becomes:
			"./script.sh -s http -h example.com -p 9980"

		once the following input parameters has been provided:
		op:      "{uri."
		cl:      "}"
		search-replace:
			"scheme" -> "http",
			"host"   -> "example.com",
			"port"   -> "9980",
	\endverbatim
	\sa GP_str_srch_replace
*/
template<class T, class V> std::basic_string<T>
hf_StrReplaceOpCloseTokensCalb( const T* inp, const T* szOpenToken,
					const T* szCloseToken, V fnStrGetter )
{
	const std::basic_string<T> str = inp, op2 = szOpenToken, cl2 = szCloseToken;
	std::basic_string<T> outp;
	size_t ppos = 0;
	for( ;; ){
		size_t pos, endd, kpos;
		// search for a opening seqence, fe. "{rq.".   //}
		if( (pos = str.find( op2, ppos )) == std::string::npos )
			break;
		kpos = pos + op2.size();
		if( (endd = str.find( cl2, kpos )) == std::string::npos )
			break;
		outp += str.substr( ppos, pos-ppos );  //add orig stride.
		std::basic_string<T> knm = str.substr( kpos, endd-kpos );   //get key name.
		std::basic_string<T> val2;
		bool bKeepOriginal = 0;
		val2 = fnStrGetter( knm.c_str(), bKeepOriginal );
		if( !bKeepOriginal ){
			outp += val2;
		}else{
			outp += op2 + knm + cl2;
		}
		endd += cl2.size(); //advance past the closing sequence.
		ppos = endd;
	}
	outp += str.substr( ppos );
	return outp;
}

} // end namespace hef

#endif //_HEF_STR_H_
