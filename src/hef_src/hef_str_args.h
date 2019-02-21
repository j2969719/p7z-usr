
#ifndef _HEF_STR_ARGS_H_
#define _HEF_STR_ARGS_H_
#include "hef_str.h"
#include "hef_str_piece.h"
#include "hef_data.h"
#include <string>
namespace hef{
;
/// \cond DOXYGEN_SKIP //{
std::string _hf_sprintfFloat2( float fx, int nDec, int nFrac );
void str_args_general_tester();
/// \endcond //DOXYGEN_SKIP //}
;
/**
	Provides string printing using consecutive calls to overriden \p "arg()" methods.
	In general, HfArgs typedef is recomended.

	Example: \code
		HfTArgs<char>("dec value: %1, str value: %2\n").arg(21).arg("ab").c_str();
	\endcode

	General methods by argument type:
	<table>
		<tr><td> char*, wchar_t*, ...           </td><td>  \ref arg(const T*) "arg( const T* a ....)"  </td></tr>
		<tr><td> int, long, short, int64_t, ... </td><td>  \ref arg() "arg( int64_t a ....)"   </td></tr>
		<tr><td> uint32_t, uint64_t, size_t, ...</td><td>  \ref arg2() "arg2( uint64_t a ....)" </td></tr>
		<tr><td> float                          </td><td>  \ref arg3() "arg3( float a ....)"   </td></tr>
	</table>

	\sa HfArgs
	\sa HfArgsW
*/
template<class T>
class HfTArgs {
public:
	/// Constructor.
	/// \param fmt - text and formating. typically contains %<n> tags that gets replaced
	///              by consecutve calls to arg(), arg2(), etc. methods.
	HfTArgs( const T* fmt ) : Fmt(fmt), ArgCnt(1) {}
	/// Argument insert method for general signed integer types.
	/// Signed integer types: \code char, short, int, long, int32_t, int64_t, ... \endcode
	/// \sa GP_strprint_value
	HfTArgs& arg( const int64_t& a, int uBase=10, int uLeadingCharsCount=0,
				char cLeading='0', int uDigitGrouping=0, const char* glue2="," )
	{
		// Documenting note:
		//     allow this overriden method to appear before any other in code
		//     to cause doxygen to link to it by default. i.e. with no need to
		//     specify argument list.
		return arg_s64( a, uBase, uLeadingCharsCount, cLeading, uDigitGrouping, glue2 );
	}
	/// Argument insert method for general unsigned integer types.
	/// From "unsigned char" to "uint64_t".
	/// \sa GP_strprint_value
	HfTArgs& arg2( const uint64_t& a, int uBase=10, size_t uLeadingCharsCount=0, char cLeading='0' ){
		return arg_u64( a, uBase, uLeadingCharsCount, cLeading, 0, "" );
	}
	/// Argument insert for 'float' type value.
	/// \sa GP_strprint_value
	HfTArgs& arg3( float a ){ // arg()
		return arg_f32( a );
	}
	/// Argument C-string insert.
	/// \sa GP_strprint_value
	HfTArgs& arg( const T* a ){
		replacePercNum( a );
		return *this;
	}
	/// Inserts signed 64-bit integer value.
	/// Parameters "cLeading" and "glue2" must be set to char-strings,
	/// currently no template type argument implementation provided.
	/// \sa GP_strprint_value
	HfTArgs& arg_s64( const int64_t& a, size_t uBase, size_t uLeadingCharsCount,
			char cLeading, size_t uDigitGrouping, const char* glue2 )
	{
		std::string str = hf_StrPrintS64(
				a, uBase,
				uDigitGrouping, glue2,
				uLeadingCharsCount, cLeading ).c_str();
		replacePercNum( hf_anyStr<T>( str.c_str() ).c_str() );
		return *this;
	}
	/// Argument insert 64-bit.
	/// \sa GP_strprint_value
	HfTArgs& arg_u64( const uint64_t& a, size_t uBase, size_t uLeadingCharsCount,
						char cLeading, size_t uDigitGrouping, const char* glue2 )
	{
		std::string str = hf_StrPrintU64(
							a, uBase,
							uDigitGrouping, glue2,
							uLeadingCharsCount, cLeading ).c_str();
		replacePercNum( hf_anyStr<T>( str.c_str() ).c_str() );
		return *this;
	}
	HfTArgs& arg_f32( float a, int nDec = -1, int nFrac = -1 ){
		replacePercNum( hf_anyStr<T>( _hf_sprintfFloat2( a, nDec, nFrac ).c_str() ).c_str() );
		return *this;
	}
	/// Inserts argument from array-of-integers, given "size_" and "glue3" (separator).
	/// \sa GP_strprint_value
	HfTArgs& arg( const int32_t* int32_array, size_t size_, const T* glue3 = 0, const T* szDflt = 0, int nBase=10 ){
		std::basic_string<T> str; size_t i;
		if( !size_ && szDflt )
			str = szDflt;
		for(i=0; i<size_; i++ ){
			if( i && glue3 ) str += glue3;
			str += HfTArgs<T>("%1").arg_s64( (int)( int32_array[i] ), nBase, 0,0,0,"").c_str();
		}
		replacePercNum( str.c_str() );
		return *this;
	}
	/// Inserts arguments repeated 'num' times.
	HfTArgs& generatedArg( const T* sz, size_t num ){
		std::basic_string<T> str; size_t i;
		for( i=0; i<num; i++ )
			str += sz;
		replacePercNum( str.c_str() );
		return *this;
	}
	const T* c_str()const {return Fmt.c_str();}
	/// Returns wide-char string.
	/// Intended to use only when template parameter T is set to "wchar_t".
	std::basic_string<wchar_t> toStdWstr()const{
		return hf_anyStr<wchar_t>( Fmt.c_str() ).c_str();
	}
	/// Extract sub-string.
	/// \sa GP_str_substr
	std::basic_string<T> substr( int pos, int len = -1 ){
		std::basic_string<T> str = Fmt;
		if( pos < 0 )
			return str.substr( str.size() + pos, len>=0?len:std::basic_string<T>::npos );
		return str.substr( pos, len>=0?len:std::basic_string<T>::npos );
	}
	/// Replaces all occurences of szNeedle with szReplacement.
	/// \param bCS - if set, case sensitive.
	HfTArgs& replace( const T* szNeedle, const T* szReplacement, bool bCS=0, int limit=-1 ){
		HfSStrReplace ssr(limit,!bCS);
		Fmt = HfCStrPiece<T>( Fmt.c_str() ).replace( szNeedle, szReplacement, ssr );
		return *this;
	}
	/// Attempts to convert current text to 64-bit unsigned integer.
	/// \sa GP_strprint_value
	uint64_t toUint64( int nBase = 10 )const{
		uint64_t rs2 = hf_AsciiToU64( hf_AsciiStr<T>( Fmt.c_str() ).c_str(), nBase );
		return rs2;
	}
	/// Attempts to convert current text to 64-bit signed integer.
	/// \sa GP_strprint_value
	uint64_t toInt64( int nBase = 10 )const{
		uint64_t rs2 = hf_AsciiToS64( hf_AsciiStr<T>( Fmt.c_str() ).c_str(), nBase );
		return rs2;
	}
private:
	void replacePercNum( const T* szRplcmt ){
		std::basic_string<T> strPercNum = hf_anyStr<T>("%");	//string i.e. "%1", "%2", etc.
		strPercNum += hf_anyStr<T>( hf_StrPrintU64( ArgCnt++ ).c_str() );
		Fmt = HfCStrPiece<T>( Fmt.c_str() ).replace( strPercNum.c_str(), szRplcmt );
	}
private:
	std::basic_string<T> Fmt;
	std::basic_string<T> TmpStr;
	size_t ArgCnt;
};

typedef HfTArgs<char>    HfArgs;	///< Basic 'char' typedef of template class HfTArgs.
typedef HfTArgs<wchar_t> HfArgsW;	///< Wide-char typedef of template class HfTArgs.



} // end namespace hef

#endif //_HEF_STR_ARGS_H_
