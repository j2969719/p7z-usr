
#ifndef _HEF_REGEX_H_
#define _HEF_REGEX_H_
#include <vector>
#include <string>

// _cond DOXYGEN_SKIP //{
;
// Copy these definitions from pcre.h
// to avoid pulling in the entire header file
extern "C"
{
	struct real_pcre8_or_16;                 // declaration; the definition is private
	typedef struct real_pcre8_or_16 pcre;
	struct pcre_extra;
}
// _endcond //_DOXYGEN_SKIP //}
;

namespace hef {

/// A class for working with regular expressions.
/// Implemented using PCRE, the Perl Compatible
/// Regular Expressions library by Philip Hazel
/// (see http://www.pcre.org).
class HfRegEx {
public:
	struct SMatch; struct SExplode; struct SMatchXML;
	typedef std::vector<SMatch> MatchVec;
	typedef std::vector<HfRegEx::MatchVec> MatchAllVec;
	typedef std::vector<std::vector<std::pair<int,std::string> > > MatchAllVec2;
	typedef std::vector<std::string> VecStr;

	HfRegEx( const std::string& pattern, int options = 0, bool study = 1 );
	~HfRegEx();
	/// Returns true if constructor compilation succeeded.
	bool success2()const {return _bSuccess;}
	/// Returns message string, retval is valid only if success2() returns false.
	const char* message2()const {return _Message.c_str();}
	//
	bool               match2( const char* szSubj, int nSubjLen = -1, int offset = 0 )const;
	bool               match3( const char* szSubj, int nSubjLen, int offset, MatchVec& matchOu, int options = 0, std::string* err = 0 )const;
	int                matchAll( const char* szSubj, int nSubjLen, int offset2, MatchAllVec& matchesOu, int nLimit = -1, int options = 0, std::string* err = 0 )const;
	int                matchAll2( const char* szSubj, int nSubjLen, int offset2, MatchAllVec2& matchesOu, int nLimit = -1, int options = 0, std::string* err = 0 )const;
	int                matchDataAll( const void* dataSubj, int nDataLen, int offset2, MatchAllVec& mtchsOu, int nLimit = -1, int options = 0, std::string* err = 0 )const;
	static int         matchDataAll2( const char* szPtrn, const void* dataSubj, int nDataLen, int offset2, MatchAllVec& mtchsOu, int nLimit = -1, int options = 0, std::string* err = 0 );
	static std::string escape2( const char* inp, int len );
	static bool        isSlashPattern( const char* inp, int len, std::pair<int,int>* outSl = 0, std::string* ptrnOu = 0, std::string* modsOu = 0 );
	static std::string fromAsteriskQmPattern( const char* inp, int len = -1 );
	std::string        extract2( const char* subj, int nSubjLen = -1, int offset2=0, int nSubMatch=0, int options2=0, int* offsetOu=0 )const;
	VecStr             explode2( const char* subj )const;
	void               explode3( const char* subj, VecStr& outp2 )const;
	void               explode4( const char* subj, int nSubjLen, int offset2, VecStr* outp2 = 0, bool(*outp3)( const SExplode& inp ) = 0, void* user2 = 0, int nLimit = -1, int options2 = 0 )const;
	void               subst2( const char* subj, int nSubjLen, int offset2, const char* szReplacement, std::string& outp, int nLimit = -1, int options2 = 0 )const;
	static std::string getPcreVersion();
	static void        convMatchToStrArr( const char* szSubj, int nSubjLen, const MatchVec& inp, std::vector<std::pair<int,std::string> >& outp );
	static bool        matchXml2( const HfRegEx& reOp, const HfRegEx* reOpRcv, const HfRegEx& reCl, const char* szSubj, int nSubjLen, int offset2, SMatchXML& matchOu, int options2, std::string* err );

	/// Some of the following options can only be passed to the constructor;
	/// some can be passed only to matching functions, and some can be used
	/// everywhere.
	///   * Options marked [ctor] can be passed to the \ref HfRegEx::HfRegEx "constructor".
	///   * Options marked [match3] can be passed to match3(), extract2(), explode2() and subst2().
	///   * Options marked [subst2] can be passed to subst2().
	///   * Options marked [matchXml2] can be passed to matchXml2().
	/// See the PCRE documentation for more information.
	enum{ // These must match the corresponsing options in pcre.h
		RE_Caseless        = 0x00000001, ///< case insensitive matching (/i) [ctor]
		RE_Multiline       = 0x00000002, ///< enable multi-line mode; affects ^ and $ (/m) [ctor]
		RE_DotAll          = 0x00000004, ///< dot matches all characters, including newline (/s) [ctor]
		RE_Extended        = 0x00000008, ///< totally ignore whitespace (/x) [ctor]
		RE_Anchored        = 0x00000010, ///< treat pattern as if it starts with a ^ [ctor, match3]
		RE_DollarEndonly   = 0x00000020, ///< dollar matches end-of-string only, not last newline in string [ctor]
		RE_Extra           = 0x00000040, ///< enable optional PCRE functionality [ctor]
		RE_NotBol          = 0x00000080, ///< circumflex does not match beginning of string [match3]
		RE_NotEol          = 0x00000100, ///< $ does not match end of string [match3]
		RE_Ungreedy        = 0x00000200, ///< make quantifiers ungreedy [ctor]
		RE_NotEmpty        = 0x00000400, ///< empty string never matches [match3]
		RE_UTF8            = 0x00000800, ///< assume pattern and subject is UTF-8 encoded [ctor]
		RE_NoAutoCapture   = 0x00001000, ///< disable numbered capturing parentheses [ctor, match3]
		RE_NoUTF8Check     = 0x00002000, ///< do not check validity of UTF-8 code sequences [match3]
		RE_FirstLine       = 0x00040000, ///< an  unanchored  pattern  is  required  to  match
		                                 ///< before  or  at  the  first  newline  in  the subject string,
		                                 ///< though the matched text may continue over the newline [ctor]
		RE_DupNames        = 0x00080000, ///< names used to identify capturing  subpatterns  need not be unique [ctor]
		RE_NewlineCR       = 0x00100000, ///< assume newline is CR ("\r"), the default [ctor]
		RE_NewlineLF       = 0x00200000, ///< assume newline is LF ("\n") [ctor]
		RE_NewlineCRLF     = 0x00300000, ///< assume newline is CRLF ("\r\n") [ctor]
		RE_NewlineAny      = 0x00400000, ///< assume newline is any valid Unicode newline character [ctor]
		RE_NewlineAnyCRLF  = 0x00500000, ///< assume newline is any of CR, LF, CRLF [ctor]

		// HEF library added flags.
		RE_NoAutoSlashPtrn = 0x01000000, ///< if set, pattern parsing in contructor does not considers slash pattern. f.e. "/abc/ix",
		                                 ///< slash patterns are usefull to specify common pattern flags, fe. "i" = case insensitive, "x" = ignore spaces (extended). [ctor]
		                                 ///< (this is first option being added thru HEF library).
		RE_SubstUseDollarN = 0x02000000, ///< use dollar character followed by a number in replacement string to insert match references ("$0" = whole match, "$1" = 1st subpatern, "$2" = second, etc.) [subst2]
		RE_XMLNoRcvCapt    = 0x04000000, ///< don't capture recursive subelements. on success, only 2 elements are returned [matchXml2].
		RE_XMLCaptByPos    = 0x08000000, ///< don't reorder open-close subelements,
		                                 ///< so that open and close tags are stored in order as they have been
		                                 ///< found in the subject. i.e. if this flag is NOT set, every open
		                                 ///< element is followed by it's corresponding close element (open tags have even index, close odd).
		                                 ///< if IS set, elements are returned in their offset order,
		                                 ///< disregarding open-close relationship.
		                                 ///< [matchXml2].
	};
	struct SMatch{
		int offset; ///< zero based offset.
		int length; ///< length of substring.
	};
	struct SExplode{
		const char* ptr;
		int         len;
		MatchVec    match;
		const char* szSubj;
		void*       user2;
	};
	/// Match result for matchXml2().
	struct SMatchXML{
		MatchAllVec2 mTags;
		int nLvlMax;
		bool bResUnordered;
		std::string getFullElem( const char* szOrigSubj, bool bOnlyContents )const;
		int getFullElem2( int* sizeOu, bool bOnlyContents )const;
	};
private:
	struct SEachRepl{
		const HfRegEx* thisp;
		const char* szRepl, *szSubj;
		std::string* strOutp;
		std::vector< std::pair<int,std::string> > arReplParsed;
		int options2;
	};
	struct SReplParsEach{
		const char* szSubj2;
		std::vector< std::pair<int,std::string> >* arReplParsed2;
	};
	/// Helper structure used in HfRegEx:matchXml2().
	struct MatchVec9 {
		int nLvl;
		MatchVec mtch;
		bool bClosing;
		MatchVec9( const MatchVec& inp, int nLvl_, bool bClosing_ ) : nLvl(nLvl_), mtch(inp), bClosing(bClosing_) {}
	};
	static bool fnEachSubstMatch( const SExplode& );
	static bool fnReplParseEach( const SExplode& );
	static int  convStrModsToFlags( const char* inp );
private:
	HfRegEx();
	HfRegEx( const HfRegEx& );
	HfRegEx& operator=( const HfRegEx& );
private:
	pcre*       _pcre;
	pcre_extra* _extra;
	bool        _bSuccess;
	std::string _Message;
	static const int OVEC_SIZE;
}; // HfRegEx class




} // end namespace hef

#endif // _HEF_REGEX_H_
