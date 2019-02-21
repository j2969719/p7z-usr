
#ifndef _HEF_STR_PIECE_H_
#define _HEF_STR_PIECE_H_
#include "hef_str.h"
//#include "hef_primitive_numbers.h"
#include <vector>
namespace hef{
;

// incomplete declarations.
template<class T> class HfCStrPiece;
struct HfSSPExplode;
template<class T> class HfCCharset;
template<class T> struct HfSChTDepth;
struct HfSStrReplace;
template<class T> struct HfSStrReplaceExt;
template<class T> class HfIStrReplCalb;
template<class T> class HfIStrReplCalbOutString;
template<class T> class HfIStrReplCalbStrPceList;
template<class T> struct HfSReduceLnDpth;
template<class T> struct HfSGetEscapedSubString;

/// \cond DOXYGEN_SKIP //{
void str_piece_general_tester();
/// \endcond //DOXYGEN_SKIP //}
;

struct HfSSPExplode {
	bool           bInclEmpty;			///< Include empty strings in output.
	/// If limit is set and >=0, the returned array will contain a
	/// maximum of limit elements with the last element containing the rest of string.
	/// -1 = no limit.
	int            nLimit;
	bool           bCaseInsensitive;	///< Case insensitive string comparision.
	bool           bInclSplitter;		///< If true 'splitter' is included at the end of each sub-string.
	HfSSPExplode( bool bInclEmpty_=0, int limit_=-1, bool bCaseInsensitive_=0,
			bool bInclSplitter_=0 )
		: bInclEmpty(bInclEmpty_)
		, nLimit(limit_)
		, bCaseInsensitive(bCaseInsensitive_)
		, bInclSplitter(bInclSplitter_)
	{}
	HfSSPExplode& inclSplitter(bool in){ bInclSplitter = in; return *this; }
	HfSSPExplode& limit(int in) { nLimit = in; return *this; }
	HfSSPExplode& caseInsensitive(bool in) { bCaseInsensitive = in; return *this; }
	HfSSPExplode& inclEmpty(bool in) { bInclEmpty = in; return *this; }
};

/// String piece template class.
/// Provides routines to work with -pieces that are defined by start position (pointer)
/// and size (number of characters). Alternative to C-strings (NULL-terminated strings).
template<class T> class HfCStrPiece
{
public:
	// typedefs
	typedef std::vector<HfCStrPiece<T> > LsStrPiece;	//old LsStringPiece

	// constructors
	HfCStrPiece(){
		assign( 0 );
	}
	HfCStrPiece( const T* sz, int size_ = -1 ){
		assign( sz, size_ );
	}
	HfCStrPiece( const T* begin, const T* end_, int ){
		assign( begin, end_, -1 );
	}
	HfCStrPiece( const std::basic_string<T>& in ){
		assign( in );
	}
	HfCStrPiece( const HfCStrPiece<T>& other, int size_ = -1 ){
		assign( other, size_ );
	}
	// assign-s
	HfCStrPiece<T>& assign( const T* in, int size_ = -1 ){
		if( in == 0 ){
			sz  = 0; siz = 0;
		}else{
			sz = in;
			if( size_ < 0 ){
				siz = hf_strlen( in );
			}else{
				siz = size_;
			}
		}
		return *this;
	}
	HfCStrPiece<T>& assign( const T* begin, const T* end_, int ){
		return assign( begin, end_ - begin );
	}
	HfCStrPiece<T>& assign( const HfCStrPiece<T>& other, int size_ = -1 ){
		sz = other.sz;
		siz = other.siz;
		if( size_ != -1 )
			siz = size_;
		return *this;
	}
	HfCStrPiece<T>& assign( const std::basic_string<T>& in ){
		sz = in.c_str();
		siz = in.size();
		return *this;
	}
	void operator=( const T* other ){
		assign( other );
	}
	void operator=( const HfCStrPiece<T>& other ){
		assign( other );
	}
	const T* c_str()const {
		strTmp.assign( sz, siz );
		return strTmp.c_str();
	}
	operator std::basic_string<T>()const{
		return std::basic_string<T>( sz, siz );
	}
	const T* pointer()const { return sz; }
	int      size()const { return siz; }
	bool     empty()const { return !sz || !*sz || !siz; }
	void     clear() { assign( 0 ); }
	const T* end()const { return sz + siz; }
	std::basic_string<T> toStd()const { return std::basic_string<T>( sz, siz ); }

	/// Left side move.
	/// \sa rightSideMove()
	HfCStrPiece<T>& leftSideMove( int amount ){
		sz += amount;
		siz -= amount;
		return *this;
	}
	/// Right side move.
	/// \sa leftSideMove()
	HfCStrPiece<T>& rightSideMove( int amount ){
		siz += amount;
		return *this;
	}
	/// \sa GP_strcmp_funcs
	bool equalTo( const T* str, int len=-1, bool bCaseInsensitive=0 )const{
		if( len == -1 )
			len = hf_strlen( str );
		if( siz == len )
			if( !hf_stroptcmp( str, sz, siz, bCaseInsensitive ) )
				return 1;
		return 0;
	}
	/// \sa GP_strcmp_funcs
	bool matchHead( const T* str, int num=-1, bool bCaseInsensitive=0 )const {
		if( num < 0 )
			num = hf_strlen( str );
		if( num <= siz ){
			return !hf_stroptcmp( sz, str, num, bCaseInsensitive );
		}
		return 0;
	}
	bool matchTail( const T* str, int num=-1, bool bCaseInsensitive=0 )const {
		if( num < 0 )
			num = hf_strlen( str );
		if( num <= siz ){
			const T* pos = (sz + siz) - num;
			return !hf_stroptcmp( pos, str, num, bCaseInsensitive );
		}
		return 0;
	}
	/// Matches head given array of c-strings.
	/// \param in - array of c-strings, terminated by null-pointer.
	/// \sa GP_strcmp_funcs
	const T* matchHead( const T*const* in, bool bCaseInsensitive=0 )const{
		int i;
		for( i=0; in[i]; i++ )
			if( matchHead( in[i], -1, bCaseInsensitive ) )
				return in[i];
		return 0;
	}

	const T& charAt( int idx )const {
		if( idx < siz )
			return sz[idx];
		//strTmp.assign( (T*)0, (int)0 );
		static const T chrTmp = 0;
		return chrTmp;
	}
	const T& operator[]( int idx )const{
		return charAt( idx );
	}
	// Merges string pieces. Works only if they refer to same string continuous in memory.
	HfCStrPiece<T>& merge( const HfCStrPiece<T>& other ){
		sz = hf_min( sz, other.sz );
		const T* end1 = sz + siz;
		const T* end2 = other.sz + other.siz;
		end1 = hf_max( end1, end2 );
		siz = end1 - sz;
		return *this;
	}
	/// \sa HfCStrPiece::implode() HfCStrPiece::explode2()
	void explode( const HfCStrPiece<T>& separator, LsStrPiece& out,
		const HfSSPExplode& opt = HfSSPExplode() )const
	{
		// call private-static func.
		HfCStrPiece<T>::explode_a( *this, separator, out, opt );
	}
	/// \sa HfCStrPiece::implode()
	LsStrPiece explode2( const HfCStrPiece<T>& separator, const HfSSPExplode& opt = HfSSPExplode() )const
	{
		LsStrPiece out;
		explode( separator, out, opt );
		return out;
	}
	/**
		\defgroup GP_str_split_funcs String splitting functions
		...
		HfCStrPiece::explode() \n
		HfCStrPiece::explode2() \n
		HfCStrPiece::explode3() \n
		HfCStrPiece::explode4() \n
		HfCStrPiece::split() \n
		HfCStrPiece::implode() \n
		HfCStrPiece::pathToPieces() \n
		HfCStrPiece::piecesToPath() \n
		hf_explode() \n
		hf_implode() \n
		hf_split() \n
		HfRegEx::explode2() \n
	*/
	;
	/// Explode string function that can take as input more than one separator string.
	/// \sa GP_str_split_funcs
	LsStrPiece explode3( const LsStrPiece& separs, const HfSSPExplode& opt = HfSSPExplode() )const{
		LsStrPiece pieces0;
		typename LsStrPiece::const_iterator a, b = separs.end();
		HfCStrPiece<T> sp( *this ), sp2;
		int pos = -1, pos2 = -1;
		int nLimit = opt.nLimit;
		for(; nLimit; pos = -1, nLimit-- ){
			// get minimal pos of all separators.
			for( a = separs.begin(); a != separs.end(); ++a ){
				if( !a->empty() && -1 != (pos2 = sp.strpos( a->c_str(), a->size(), opt.bCaseInsensitive )) ){
					if( pos == -1 || pos2 < pos ){
						pos = pos2;
						b = a;
					}
				}
			}
			if( pos != -1 ){
				sp2.assign( sp, (int)pos + ( opt.bInclSplitter ? b->size() : 0 ) );
				if( !sp2.empty() || opt.bInclEmpty )
					pieces0.push_back( sp2 );
				sp.assign( sp.pointer() + pos + b->size(), (int)-1 );
			}else{
				break;
			}
		}
		if( !sp.empty() || opt.bInclEmpty )
			pieces0.push_back( sp );
		return pieces0;
	}
	/// \sa GP_str_split_funcs
	void explode4( const HfCStrPiece<T>& separator, std::vector<std::basic_string<T> >& out, const T* szTrimChars=0, const HfSSPExplode& opt = HfSSPExplode() )const{
		LsStrPiece sps;
		explode( separator, sps, opt );
		typename LsStrPiece::iterator a;
		out.clear();
		for( a = sps.begin(); a != sps.end(); ++a ){
			if(szTrimChars)
				*a = HfCStrPiece<T>(a->c_str()).trim( szTrimChars );
			out.push_back( a->c_str() );
		}
	}
	/// Splits str-piece into sub-strings given sub-string size.
	/// If negative value given, splits starting from str-piece's end.
	LsStrPiece split( int nSubstrLength )const
	{
		LsStrPiece out;
		if( !nSubstrLength ){
			out.push_back( *this );
		}else if( nSubstrLength > 0 ){
			const T* pos = sz, *endMax = sz + siz, *end;
			for(; pos < endMax; ){
				end = pos + nSubstrLength;
				end = std::min( end, endMax );
				out.push_back( HfCStrPiece<T>( pos, end - pos ) );
				pos = end;
			}
		}else{	//else, splitting starts from right.
			nSubstrLength = nSubstrLength * -1;
			const T* rpos = sz + siz, *pos;
			for(; rpos > sz; ){
				pos = rpos - nSubstrLength;
				pos = std::max( pos, sz );
				out.insert( out.begin(), 1, HfCStrPiece<T>( pos, rpos - pos ) );
				rpos = pos;
			}
		}
		return out;
	}
	/**
		\defgroup GP_strarray_conv String array conversions
		...
		HfCStrPiece::implode() \n
		HfCStrPiece::explode2() \n
		HfCStrPiece::convSpArrayToStrArray() \n
		HfCStrPiece::convSpArrayToStrArray2() \n
		HfCStrPiece::convStrArrayToSpArray() \n
		HfCStrPiece::convStrArrayToSpArray2() \n
		HfCStrPiece::pathToPieces() \n
		HfCStrPiece::piecesToPath() \n
	*/
	;
	/// \sa GP_strarray_conv
	/// \sa HfCStrPiece::explode() HfCStrPiece::explode2()
	static std::basic_string<T> implode( const LsStrPiece& in, const T* glue = 0 ){
		std::basic_string<T> out;
		typename LsStrPiece::const_iterator a; int i;
		for( i=0, a = in.begin(); a != in.end(); ++a, i++ ){
			if( glue && i )
				out += glue;
			out += a->toStd();
		}
		return out;
	}
	/// \sa GP_strarray_conv
	static void convSpArrayToStrArray( const LsStrPiece& in, std::vector<std::basic_string<T> >& out ){
		typename LsStrPiece::const_iterator a;
		for( a = in.begin(); a != in.end(); ++a ){
			out.push_back( a->toStd() );
		}
	}
	/// \sa GP_strarray_conv
	static std::vector<std::basic_string<T> > convSpArrayToStrArray2( const LsStrPiece& in ){
		std::vector<std::basic_string<T> > out;
		convSpArrayToStrArray( in, out );
		return out;
	}

	/// Converts string-array to string-piece-array (actual container is std::vector).
	/// Warning: all strings gets referenced using just c_str() method of each input
	/// string.
	/// \sa GP_strarray_conv
	static void convStrArrayToSpArray(
			const std::vector<std::basic_string<T> >& in, LsStrPiece& out )
	{
		typename std::vector<std::basic_string<T> >::const_iterator a;
		for( a = in.begin(); a != in.end(); ++a )
			out.push_back( HfCStrPiece<T>( a->c_str() ) );
	}
	/// This method is a ret-val version of convStrArrayToSpArray().
	/// \sa GP_strarray_conv
	static LsStrPiece convStrArrayToSpArray2( const std::vector<std::basic_string<T> >& in )
	{
		LsStrPiece out;
		convStrArrayToSpArray( in, out );
		return out;
	}
	/**
		Left trims current string piece.
		First parameter 'charset' is a class that will adopt string pointer through
		constructor parameter. \n
		Example: \code
			wprintf( L"spw.trim(), %s\n",
				HfCStrPiece<wchar_t>(L"abcdefw").trim(L"awf").c_str() );
		\endcode
		\sa GP_strtrims
	*/
	HfCStrPiece<T>& ltrim( const HfCCharset<T>& charset=HfCCharset<T>(" \r\n\t",-9), int limit=-1 ){
		for(; siz && hf_strchr( charset.charset(), *sz ) && limit; siz--, sz++, limit-- );
		return *this;
	}
	/// \sa GP_strtrims
	HfCStrPiece<T>& rtrim( const HfCCharset<T>& charset=HfCCharset<T>(" \r\n\t",-9), int limit=-1 ){
		const T* bk = end();
		bk--;
		for(; bk >= sz && hf_strchr( charset.charset(), *bk ) && limit; siz--, bk--, limit-- );
		return *this;
	}
	/// \sa GP_strtrims
	HfCStrPiece<T>& trim( const HfCCharset<T>& charset=HfCCharset<T>(" \r\n\t",-9), int limit=-1 ){
		return ltrim(charset.charset(),limit).rtrim(charset.charset(),limit);
	}
	// Counts starting characters that are in set specified by 'charset' parameter.
	// Stops on first character that isn't in set.
	int countHeadCharacters( const T* charset )const{
		int i;
		for( i=0; i<(int)siz && hf_strchr( charset, (*this)[i] ); i++ );
		return i;
	}
	int countTailCharacters( const T* charset )const{
		if( empty() )
			return 0;
		int i;
		const T* back2 = sz + siz - 1;
		for( i=0; back2 >= sz && hf_strchr( charset, *back2 ); back2--, i++ );
		return i;
	}

	/**
		Returns line depth and line depth in characters given num spaces per tab character
		and optionally, maximum depth in tab characters.
		\param nSpcsPerTab - Number of spaces per tab character. Ie. Windows' notepad
		        as well Windows' console have it set to 8, most IDEs' uses value 4.
		\param chrDepthOut - Output character depth.
		\param depthMax    - Maximum tab depth to stop counting characters at.
			    Set to -1 to ignore.
		\param bTrailingSpcsAsTab - If true, trailing space characters with number less than
			   nSpcsPerTab will account into single tab.
		\param sctd        - Optional structure that may specify different tab and
		       space characters.
		\return Line depth in tab characters. If depthMax specified, less or equal depthMax.

		This method may be used to obtain number of characters at specified tab depth.
		This can be done by comparing return value against input depthMax value.
		If they're equal, chrDepthOut contains
		desired nunmber of characters. If return value is less, it means line isn't "deep"
		enough.
	*/
	int calcLineDepthCharacters( int nSpcsPerTab, int& chrDepthOut, int depthMax=-1,
		bool bTrailingSpcsAsTab=0, const HfSChTDepth<T>& sctd = HfSChTDepth<T>() )const
	{
		chrDepthOut = 0;
		HfCStrPiece<T> sp = *this;
		int trailingSpcs = 0, spcs, depthOut = 0;
		T ch;
		for(; ( ( !sp.empty() && depthOut < depthMax ) || depthMax==-1 ); sp.leftSideMove(1) ){
			ch = sp.charAt(0);
			if(0){
			}else if( ch == sctd.spaceCharacter ){
				spcs         = trailingSpcs + 1;
				depthOut    += spcs / nSpcsPerTab;
				trailingSpcs = spcs % nSpcsPerTab;
			}else if( ch == sctd.tabCharacter ){
				depthOut    += 1;
				trailingSpcs = 0;
			}else
				break;
		}
		chrDepthOut = sp.pointer() - this->pointer();
		if(bTrailingSpcsAsTab){
			depthOut += !!trailingSpcs;
		}else{
			chrDepthOut -= trailingSpcs;
		}
		return depthOut;
	}

	/// Generic search-replace method.
	/// Note that because of specific HfCStrPiece class constructor,
	/// regular strings may be used as corresponding parameters.
	std::basic_string<T> replace( const HfCStrPiece<T>& search,
		const HfCStrPiece<T>& replacement = HfCStrPiece<T>(0),
		const HfSStrReplace& cfg = HfSStrReplace() )const
	{
		std::basic_string<T> out;
		HfIStrReplCalbOutString<T> cb;
		cb.strOut = &out;
		HfSStrReplaceExt<T> sr;
		*static_cast<HfSStrReplace*>(&sr) = cfg;
		sr.eachString = &cb;
		HfCStrPiece<T> arSearch[] = {
			search,
		};
		HfCStrPiece<T> arReplacement[] = {
			replacement,
		};
		replace_priv2( arSearch, sizeof(arSearch)/sizeof(arSearch[0]),
			arReplacement, sizeof(arReplacement)/sizeof(arReplacement[0]), sr );
		return out;
	}
	void replace( const HfCStrPiece<T>& search, const HfCStrPiece<T>& replacement,
		LsStrPiece& out, const HfSStrReplace& cfg = HfSStrReplace() )const
	{
		HfIStrReplCalbStrPceList<T> ir;
		ir.Out = &out;
		HfSStrReplaceExt<T> sr;
		*static_cast<HfSStrReplace*>(&sr) = cfg;
		sr.eachString = &ir;

		HfCStrPiece<T> arSearch[] = {
			search,
		};
		HfCStrPiece<T> arReplacement[] = {
			replacement,
		};
		replace_priv2( arSearch, sizeof(arSearch)/sizeof(arSearch[0]),
			arReplacement, sizeof(arReplacement)/sizeof(arReplacement[0]), sr );

	}

	/// Search-replace using array of string pointers.
	/// \param search - Array of strings to search for, must be terminated by
	///         null pointer element.
	/// \param replace - Array of strings for replacement, must be terminated
	///         by null pointer element.
	/// \param cfg - configuration structure.
	///
	/// If index of search string doesnt exist in replacement, search is replaced by last
	/// replacement element in array.
	/// If replacement array is empty (size = 0) each element gets replaced
	/// by empty string, aka. all found elements are removed.
	std::basic_string<T> replace( const T* const* search, const T* const* replace,
		const HfSStrReplace& cfg = HfSStrReplace() )const
	{
		std::basic_string<T> out;
		HfIStrReplCalbOutString<T> cb;	//.
		cb.strOut = &out;
		HfSStrReplaceExt<T> sr;
		*static_cast<HfSStrReplace*>(&sr) = cfg;
		sr.eachString = &cb;
		LsStrPiece lsSearch, lsRepl;
		int i;
		for( i=0; search[i]; i++ )
			lsSearch.push_back( search[i] );
		for( i=0; replace && replace[i]; i++ )
			lsRepl.push_back( replace[i] );
		replace_priv2( &lsSearch[0], lsSearch.size(), &lsRepl[0], lsRepl.size(), sr );
		return out;

	}
	std::basic_string<T> replace( const HfCStrPiece<T> search2[], int len_s,
		const HfCStrPiece<T> replacement2[], int len_r,
		const HfSStrReplace& cfg = HfSStrReplace() )const
	{
		;
		std::basic_string<T> out;
		HfIStrReplCalbOutString<T> cb;
		cb.strOut = &out;
		HfSStrReplaceExt<T> sr;
		*static_cast<HfSStrReplace*>(&sr) = cfg;
		sr.eachString = &cb;
		//
		replace_priv2( search2, len_s, replacement2, len_r, sr );
		return out;

	}
	std::basic_string<T> replace_ch( const T* charsetSrch, const T* charsetRepl,
		const HfSStrReplace& cfg = HfSStrReplace() )const
	{
		LsStrPiece lsSearch, lsRepl;
		for(; *charsetSrch; charsetSrch++ ){
			T ch = *charsetSrch;
			lsSearch.push_back( HfCStrPiece<T>( charsetSrch, 1 ) );
		}
		for(; charsetRepl && *charsetRepl; charsetRepl++ ){
			lsRepl.push_back( HfCStrPiece<T>( charsetRepl, 1 ) );
		}
		return replace( &lsSearch[0], lsSearch.size(), &lsRepl[0], lsRepl.size(), cfg );
	}
	/// \sa GP_strcmp_funcs
	const T* find( const T* search, int size_=-1, bool bCaseInsensitive=0 )const
	{
		if( size_ == -1 )
			size_ = hf_strlen( search );
		return hf_strstrnn( sz, siz, search, size_, bCaseInsensitive );
	}
	/// \sa GP_strcmp_funcs
	int strpos( const T* search, int size_=-1, bool bCaseInsensitive=0 )const{
		const T* szPos = find( search, size_, bCaseInsensitive );
		if( szPos )
			return szPos - sz;
		return -1;
	}
	/**
		Right truncate this string piece.
		Since it operates on C-string, doesn't appends any dots.
		\param truncated - optional, gets set to number of characters truncated.
		\sa GP_truncate_fncs
	*/
	HfCStrPiece<T>& truncate( int max_size, int* truncated=0 ){
		if(truncated)*truncated = 0;
		if( siz > max_size ){
			if(truncated)
				*truncated = siz - max_size;
			siz = max_size;
		}
		return *this;
	}
	/// \sa GP_strtrims
	void reduceLinesDepth( LsStrPiece& out,
		const HfSReduceLnDpth<T>& cfg = HfSReduceLnDpth<T>() )const
	{
		HfCStrPiece<T> sp = *this;

		HfSSPExplode sspl;
		sspl.nLimit      = cfg.nFirstLins;
		sspl.bInclEmpty = 1;
		sspl.bCaseInsensitive = 0;
		sspl.bInclSplitter = 1;
		sp.explode( cfg.strNlSplitter.c_str(), out, sspl );

		// calculate min line depth.
		int minLnDepth = cfg.maxReduce; //-1;
		{
			int dpt, notused;
			typename LsStrPiece::iterator a;
			for( a = out.begin(); a != out.end(); ){
				if( !HfCStrPiece<T>(*a).trim( cfg.strEmptyLnChars.c_str() ).empty() ){
					dpt = a->calcLineDepthCharacters( cfg.nSpcsPerTab, notused, -1, cfg.bTrailingSpcsAsTab, cfg );
					if( minLnDepth != -1 )
						minLnDepth = std::min( dpt, minLnDepth );
					else
						minLnDepth = dpt;
					++a;
				}else if( !cfg.bInclEmptyLines ){
					a = out.erase( a );
				}else
					++a;
			}
		}
		// left trim line indents by minimal depth value.
		if( minLnDepth != -1 ){
			typename LsStrPiece::iterator a;
			for( a = out.begin(); a != out.end(); ++a ){
				int chrDpth = 0;
				if( minLnDepth == a->calcLineDepthCharacters( cfg.nSpcsPerTab,
						chrDpth, minLnDepth, cfg.bTrailingSpcsAsTab, cfg ) )
				{
					a->leftSideMove( chrDpth );
				}
			}
		}

	}
	std::basic_string<T> reduceLinesDepth( const HfSReduceLnDpth<T>& cfg = HfSReduceLnDpth<T>() )const
	{
		LsStrPiece lines;
		reduceLinesDepth( lines, cfg );
		return implode( lines );
	}
	/**
		Extracts string until terminator character or end of string.
		Takes escape characters into account.
		\param escapes - Escape characters as null-terminated array.
		\param terms - Terminator characters...
		\param terms_ne - Terminator characters that cannot be escaped by escape character.
		\param opt - Additional options structure, struct HfSGetEscapedSubString.
		\return - Returns position where extracting stopped. Either before or after terminator, see options structure.
		\sa struct HfSGetEscapedSubString.
		\sa GP_escaped_substr
	*/
	int getEscapedSubString( const HfCStrPiece<T>& escapes,
		const HfCStrPiece<T>& terms, const HfCStrPiece<T>& terms_ne,
		const HfSGetEscapedSubString<T>& opt = HfSGetEscapedSubString<T>() )const
	{
		HfCStrPiece<T> sp = *this;
		T ch;
		bool bEscaped = 0;
		std::basic_string<T>* out = opt.escaped;
		const T* ptr;

		for(; !sp.empty(); ){
			ch = sp[0];
			if( !bEscaped && !!escapes.find( &ch,1,opt.bCaseInsensitive ) ){
				sp.leftSideMove(1);
				bEscaped = 1;
				continue;
			}
			if( !bEscaped && !!(ptr = terms.find( &ch,1,opt.bCaseInsensitive )) ){
				if( opt.bInclTerminator ){
					sp.leftSideMove(1);
					if(out)
						*out += ch;
				}
				if(opt.terminatedby)
					*opt.terminatedby = ptr;
				break;
			}
			if( !!(ptr = terms_ne.find( &ch,1,opt.bCaseInsensitive )) ){
				if( opt.bInclNeTerminator ){
					sp.leftSideMove(1);
					if(out)
						*out += ch;
				}
				if(opt.terminatedby)
					*opt.terminatedby = ptr;
				break;
			}
			if(out)
				*out += ch;
			bEscaped = 0;
			sp.leftSideMove(1);
		}
		return sp.pointer() - this->pointer();
	}
	// Flags for HfCStrPiece<>::getEscapedSubString2().
	enum {
		EGES2F_NOFLAGS=0,
		EGES2F_CI=0x1,			///< Case insensitive comparision.
		EGES2F_RETESCAPED=0x2,	///< Return already escaped string.
	};
	/**
		Alternate to getEscapedSubString() method.\n
		Example: \n
		\code
			printf("sub-str: %s\n", HfCStrPiece<char>( "abc^]def]ghi" )
				.getEscapedSubString2("^","]","",HfCStrPiece<char>::EGES2F_RETESCAPED)
				.c_str() );
			// returns: "abc]def" string.
		\endcode
		\sa GP_escaped_substr
		\sa struct HfSGetEscapedSubString
	*/
	std::basic_string<T>
	getEscapedSubString2( const HfCStrPiece<T>& escapes,
		const HfCStrPiece<T>& terms, const HfCStrPiece<T>& terms_ne,
		int flags = EGES2F_NOFLAGS, int* lengthOut = 0 )const
	{
		HfSGetEscapedSubString<T> opt;
		opt.bCaseInsensitive = !!(flags & EGES2F_CI);
		std::basic_string<T> out1;
		if( flags & EGES2F_RETESCAPED ){
			opt.escaped = &out1;
		}
		int length = getEscapedSubString(escapes,terms,terms_ne,opt);
		if( !(flags & EGES2F_RETESCAPED) ){
			out1.assign( sz, length );
		}
		if(lengthOut)
			*lengthOut = length;
		return out1;
	}
	/**
		Converts command line string-piece to array.
		Respects quoted arguments and extracts them correctly.\n
		Example:\n
		\code
			HfCStrPiece<char> sp1("ex.exe -a \"ab cd\" --pro");
			std::vector<HfCStrPiece<char> > ls1 = sp1.toCommandLineArgv();
			// result:
			//     ex.exe
			//     -a
			//     ab cd
			//     --pro
		\endcode
		\sa GP_cmd_line_args
	*/
	std::vector<HfCStrPiece<T> > toCommandLineArgv()const
	{
		std::vector<HfCStrPiece<T> > out;
		int i = 0; HfCStrPiece<T> last;
		for(; i<siz; ){
			if( sz[i] == (T)'"' || sz[i] == (T)'\'' ){
				if( !last.empty() ){
					out.push_back(last);
					last.clear();
				}
				T trm = sz[i];
				i++;
				if ( siz < i ) break;
				T empty[] = { 0, };
				HfSGetEscapedSubString<T> sc;
				sc.bInclNeTerminator = 1;
				int len = HfCStrPiece<T>( &sz[i], this->end(), 77 )
					.getEscapedSubString( empty, HfCStrPiece<T>(&trm,1), empty, sc );
				out.push_back( HfCStrPiece<T>( &sz[i], len ) );
				i += len + 1;
			}else{
				if( sz[i] == (T)' ' ){
					if( !last.empty() ){
						out.push_back(last);
						last.clear();
					}
				}else{
					last.empty() ? last.assign(&sz[i],1) : last.rightSideMove(1);
				}
				i++;
			}
		}
		if( !last.empty() ){
			out.push_back(last);
		}
		return out;
	}

	/// Indents all lines with given string.
	std::basic_string<T> indent( const T* indentWith )const
	{
		HfCStrPiece<T> sp( *this );
		return std::basic_string<T>(indentWith) +
			sp.replace( hf_anyStr<T>("\n").c_str(),
				std::basic_string<T>( hf_anyStr<T>("\n") + indentWith ).c_str() );
	}
	std::basic_string<T> padd( int padd_size, const T* paddWith = hf_anyStr<T>(" ").c_str() )const
	{
		std::basic_string<T> str = *this;
		int len = hf_strlen(paddWith), padd_now;
		for(; padd_size; ){
			if( padd_size > len ){
				padd_now = len;
				padd_size -= len;
			}else{
				padd_now = padd_size;
				padd_size = 0;
			}
			str.append( paddWith, padd_now );
		}
		return str;
	}
	/// Flags for HfCStrPiece<T>::pathToPieces().
	enum { ESPPTPF_NOFLAGS=0, ESPPTPF_EMPTYSTRATLEAST=0x1, };
	/// \sa GP_str_split_funcs
	/// \sa GP_dirname_and_fn
	std::vector<HfCStrPiece<T> > pathToPieces( size_t flags = ESPPTPF_NOFLAGS )
	{
		LsStrPiece pieces, separs;
		separs.push_back("/");
		separs.push_back("\\");
		pieces = explode3( separs, HfSSPExplode() );
		if( pieces.empty() && flags & ESPPTPF_EMPTYSTRATLEAST )
			pieces.push_back( HfCStrPiece<T>() );
		return pieces;
	}
	/// Flags for HfCStrPiece::piecesToPath().
	enum { ESPPTP2F_NOFLAGS=0, ESPPTP2F_BACKSLMODE=0x10, };
	/// Returns path as string.
	/// Returns path always without adding leading slash character.
	/// \sa HfCStrPiece::pathToPieces().
	/// \sa GP_str_split_funcs
	/// \sa GP_dirname_and_fn
	static std::basic_string<T> piecesToPath(
			const std::vector<HfCStrPiece<T> >& in, size_t flags = ESPPTP2F_NOFLAGS )
	{
		std::basic_string<T> slashMode = hf_anyStr<T>( flags & ESPPTP2F_BACKSLMODE ? "\\":"/" );
		std::basic_string<T> out;
		typename std::vector<HfCStrPiece<T> >::const_iterator a; size_t i;
		for( i=0, a = in.begin(); a != in.end(); ++a, i++ ){
			if( !out.empty() )
				out += slashMode;
			out.append( a->pointer(), a->size() );	//out += a->c_str();
		}
		return out;
	}
	/**
		Finds matching brace given opening and closing character.
		For round brackets, set 'op' to "(" and 'close' to ")".
		Additionally 'escSeq' may specify backslash to turn on escaped braces skipping.
	*/
	const T* findMatchingBrace( const T& op, const T& close, size_t uDepthNow = 0, bool bUseEscChar = 0, const T& escChar = 0 )
	{
		int i, nDepth2 = (int)uDepthNow; T ch; bool bEscaped = 0;
		for( i=0; i<siz; i++ ){
			ch = sz[i];
			if( ch == op && !bEscaped ){
				nDepth2++;
			}else if( ch == close && !bEscaped ){
				nDepth2--;
				if(!nDepth2)
					return &sz[i];
				if( nDepth2 < 0 )
					return 0;
			}else if( bUseEscChar && ch == escChar && !bEscaped ){
				bEscaped = 1;
				continue;
			}
			bEscaped = 0;
		}
		return 0;
	}
	/// Counts the number of substring occurrences.
	/// Returns the number of times the 'szSubstr' substring occurs in the
	/// string-piece. Case sensitive.
	/// \sa GP_str_substr
	size_t getSubstrCount( const T* szSubstr )const{
		size_t len = hf_strlen(szSubstr);
		size_t cnt = 0, i,k, num = siz;
		for( i=0; num; num--, i++ ){
			for( k=0; i+k < siz && k<len && sz[i+k] == szSubstr[k] ; k++ ){
				if( k+1 == len )
					cnt++;
			}
		}
		return cnt;
	}

private:
	static void explode_a( const HfCStrPiece<T>& subject, const HfCStrPiece<T>& separ,
		LsStrPiece& out, const HfSSPExplode& opt = HfSSPExplode() )
	{
		//LsStrPiece::const_iterator a;
		int limit = opt.nLimit; bool bMatch;
		int length = subject.size();
		HfCStrPiece<T> sp;
		const T* sbj = subject.pointer();
		const T* bgn = sbj;
		while( length && length >= (int)separ.size() ){
			bMatch = !hf_stroptcmp( sbj, separ.pointer(), separ.size(), opt.bCaseInsensitive );
			if( bMatch ){
				if( !limit )
					break;
				sp.assign( bgn, sbj, (int)0 );
				if(opt.bInclSplitter)
					sp.rightSideMove( separ.size() );
				if( (opt.bInclEmpty || !sp.empty()) ){
					limit--;
					out.push_back( sp );
				}
				sbj    += separ.size();
				length -= separ.size();
				bgn     = sbj;
				continue;
			}
			sbj++;
			length--;
		}
		if( limit ){
			sp.assign( bgn, subject.end(), (int)0 );
			if( (opt.bInclEmpty || !sp.empty())/*&&(!opt.fnEachPiece || opt.fnEachPiece( sp ))*/ )
				out.push_back( sp );
		}
	}
	void replace_priv2( const HfCStrPiece<T> search2[], int len_s,
		const HfCStrPiece<T> replacement2[], int len_r,
		const HfSStrReplaceExt<T>& cfg = HfSStrReplaceExt<T>() )const
	{
		const T emptystr[] = { 0, };
		int limit = cfg.limit, i, cFnd;
		HfCStrPiece<T> sp = *this;
		const T* bgn = sp.pointer();
		for(; !sp.empty() && limit; ){
			bool found = 0;
			int cTestL = 0;
			for( i=0, cFnd=0; i<len_s; i++ ){	//for each search_ elem.
				const HfCStrPiece<T>& sr = search2[i];
				if( sp.size() >= sr.size() ){
					cTestL++;
					if( !hf_stroptcmp( sp.pointer(), sr.pointer(), sr.size(), cfg.bCaseInsensitive ) ){
						cFnd++;
						(*cfg.eachString)( bgn, sp.pointer() - bgn );
						int r;
						if( i<len_r ){
							r = i;
						}else{
							r = len_r - 1;
						}
						const T* ptr = 0; int len = 0;
						if( r >= 0 ){
							ptr = replacement2[r].pointer();
							len = replacement2[r].size();
						}else{
							ptr = emptystr;
							len = 0;
						}
						(*cfg.eachString)( ptr, len );

						sp.leftSideMove( sr.size() );
						bgn = sp.pointer();
						limit--;
						found = 1;
						break;
					}
				}
				if( found )
					break;
			}
			if( !cTestL )			//if nothing tested due to size.
				break;
			if( !found )
				sp.leftSideMove(1);
		}
		(*cfg.eachString)( bgn, sp.end() - bgn );
	}
private:
	const T*  sz;
	int       siz;
	mutable std::basic_string<T> strTmp;
};



template<class T>
class HfCCharset
{
	std::basic_string<T> chrs;
public:
	HfCCharset( const char* dflt, int ){
		for(; *dflt; dflt++ )
			chrs += (T)*dflt;
	}
	HfCCharset( const T* in ) : chrs(in) {
	}
	const T* charset()const { return chrs.c_str(); }
};
/// Specifies some options for calcLineDepthCharacters() method.
template<class T>
struct HfSChTDepth {
	T tabCharacter;		///< Tab character, the \\t.
	T spaceCharacter;	///< Space character, the ' '.
	HfSChTDepth( T tabCharacter_= (T)'\t', T spaceCharacter_ = (T)' ' )
		: tabCharacter(tabCharacter_)
		, spaceCharacter(spaceCharacter_)
	{}
};
/// Some options for HfCStrPiece::replace() method.
struct HfSStrReplace {
	int            limit;				///< -1 = no limit.
	bool           bCaseInsensitive;	///< Case insensitive string comparision.
	HfSStrReplace( int limit_=-1, bool bCaseInsensitive_=0 )
		: limit(limit_)
		, bCaseInsensitive(bCaseInsensitive_)
	{}
};
/// \cond DOXYGEN_SKIP //{
;
template<class T>
struct HfSStrReplaceExt : public HfSStrReplace {
	HfIStrReplCalb<T>*  eachString;
	HfSStrReplaceExt()
		: eachString(&isrStatic)
	{}
private:
	HfIStrReplCalb<T> isrStatic;//static
};
/// Interface for string replace_ callbacks.
template<class T>
class HfIStrReplCalb {
public:
	virtual void operator() ( const T* szz, size_t sizee ) {}
};
template<class T>
class HfIStrReplCalbOutString : public HfIStrReplCalb<T>
{
public:
	std::basic_string<T>* strOut;
	virtual void operator() ( const T* szz, size_t sizee ){
		strOut->append( szz, sizee );
	}
};
template<class T>
class HfIStrReplCalbStrPceList : public HfIStrReplCalb<T>
{
public:
	std::vector<HfCStrPiece<T> >* Out;
	virtual void operator() ( const T* szz, size_t sizee )
	{
		//strOut->append( szz, sizee );
		Out->push_back( HfCStrPiece<T>(szz,sizee) );
	}
};
/// \endcond //DOXYGEN_SKIP //}
;

/// Specifies options for lines depth reduction.
template<class T>
struct HfSReduceLnDpth : public HfSChTDepth<T>
{
	int       nSpcsPerTab;		///< Number of spaces per tab character.
	int       nFirstLins;		///< Limit number of lines from start, -1 is ignore, 0 is only first line.
	int       maxReduce;		///< Maximum indent depth to reduce, -1 is ignore.
	bool      bTrailingSpcsAsTab;	///< If set, spaces that follow last tab indent are treated as single tab indent.
	bool      bInclEmptyLines;
	std::basic_string<T> strNlSplitter;	///< New line splitter. Sequence that splits string into lines.
	std::basic_string<T> strEmptyLnChars;	///< Trimable characters when testing if line is empty.
	//
	HfSReduceLnDpth( int nSpcsPerTab_=4, int maxReduce_=-1, bool bTrailingSpcsAsTab_=0 )
		: nSpcsPerTab(nSpcsPerTab_)
		, nFirstLins(-1)
		, maxReduce(maxReduce_)
		, bTrailingSpcsAsTab(bTrailingSpcsAsTab_)
		, bInclEmptyLines(1)
	{
		const char* a = "\r\n\t ";
		for(; *a; ++a )
			strEmptyLnChars += (T)*a;
		const char* b = "\n";
		for(; *b; ++b )
			strNlSplitter += (T)*b;
	}
	HfSReduceLnDpth<T>& indents( T tabCharacter_, T spaceCharacter_ )
	{
		HfSChTDepth<T>::tabCharacter = tabCharacter_;
		HfSChTDepth<T>::spaceCharacter = spaceCharacter_;
		return *this;
	}
	HfSReduceLnDpth<T>& nlSplitter( const T* in ) { strNlSplitter = in; return *this; }
	HfSReduceLnDpth<T>& lines( int in ) { nFirstLins = in; return *this; }
	HfSReduceLnDpth<T>& inclEmpty( bool empty_ ) { bInclEmptyLines = empty_; return *this; }
	HfSReduceLnDpth<T>& skipEmpty() { bInclEmptyLines = 0; return *this; }
};
template<class T>
struct HfSGetEscapedSubString {
	bool bCaseInsensitive;
	bool bInclTerminator;		///< set to true to include terminator in result.
	bool bInclNeTerminator;		///< set to true to include no-escape terminator in result.
	std::basic_string<T>* escaped;			///< Optional. Output, escaped string will be stored here.
	/// Optional. If terminated by terminator (not by end of string piece) pointer
	/// to terminator character is stored here. Stored value is a pointer to character
	/// inside one of terminators string.
	const T** terminatedby;
	HfSGetEscapedSubString( bool bCaseInsensitive_=0, bool bInclTerminator_=0,
		bool bInclNeTerminator_=0, std::basic_string<T>* escaped_=0, const T** terminatedby_=0 )
		: bCaseInsensitive(bCaseInsensitive_)
		, bInclTerminator(bInclTerminator_)
		, bInclNeTerminator(bInclNeTerminator_)
		, escaped(escaped_)
		, terminatedby(terminatedby_)
	{}
};
/// \cond DOXYGEN_SKIP //{
;
// internal, static declarations.
static const HfSSPExplode HfSpExplodeDflt;
/// \endcond //DOXYGEN_SKIP //}
;

/// String piece typedef for 8-bit character type.
typedef HfCStrPiece<char> HfSp;


} // end namespace hef

#endif //_HEF_STR_PIECE_H_
