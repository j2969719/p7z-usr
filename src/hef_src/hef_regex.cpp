
#include <stdio.h>
#include <cstring>
#include "../libraries/pcre_minimal_rev2/pcre_config.h"
#include "../libraries/pcre_minimal_rev2/pcre.h"
#include "hef_regex.h"

namespace hef {

const int HfRegEx::OVEC_SIZE = 64;

/// Creates a regular expression and parses the given pattern.
/// If study is true, the pattern is analyzed and optimized. This
/// is mainly useful if the pattern is used more than once.
/// For a description of the options, please see the PCRE documentation.
HfRegEx::
HfRegEx( const std::string& pattern2, int options2, bool study )
	: _pcre(0), _extra(0), _bSuccess(0)
{
	int options3 = options2 & ( ~(RE_NoAutoSlashPtrn|RE_SubstUseDollarN) );
	std::string pattern3 = pattern2;
	if( !(options2 & RE_NoAutoSlashPtrn) ){
		std::string ptrn, mods;
		if( isSlashPattern( pattern3.c_str(), -1, 0, &ptrn, &mods ) ){
			pattern3 = ptrn;
			options3 |= convStrModsToFlags( mods.c_str() );
		}
	}
	//
	const char* error;
	int offs;
	_pcre = pcre_compile( pattern3.c_str(), options3, &error, &offs, 0 );
	if( !_pcre ){
		char bfr[128];
		sprintf( bfr, " (at offset %d)", offs );
		_Message = "";
		_Message += error;
		_Message += bfr;
		return;
	}
	if (study)
		_extra = pcre_study(_pcre, 0, &error);
	_bSuccess = 1;
}

HfRegEx::~HfRegEx()
{
	if (_pcre)  pcre_free(_pcre);
	if (_extra) pcre_free(_extra);
}

bool HfRegEx::
match3( const char* szSubj, int nSubjLen, int offset, MatchVec& matchOu, int options2,
			std::string* err )const
{
	matchOu.clear();
	std::string err3, *err2 = ( err ? err : &err3 );

	int nSubjLen2 = ( nSubjLen == -1 ? strlen(szSubj) : nSubjLen );
	int ovec[OVEC_SIZE];

	int res = pcre_exec( _pcre, _extra, szSubj, nSubjLen2, offset,
							options2 & 0xFFFF, ovec, OVEC_SIZE );
	if( res == PCRE_ERROR_NOMATCH ){
		return 0;
	}else if( res == PCRE_ERROR_BADOPTION ){
		*err2 = "bad option";
		return 0;
	}else if( res == 0 ){
		*err2 = "too many captured substrings";
		return 0;
	}else if( res < 0 ){
		char bfr[128];
		sprintf( bfr, "PCRE error %d", res );
		*err2 = bfr;
		return 0;
	}
	matchOu.reserve(res);
	int i;
	for( i=0; i < res; i++ ){
		SMatch m;
		m.offset = ovec[i*2] < 0 ? std::string::npos : ovec[i*2];
		m.length = ovec[i*2 + 1] - m.offset;
		matchOu.push_back(m);
	}
	return !!res;
}

int HfRegEx::
matchAll( const char* szSubj, int nSubjLen, int offset2, MatchAllVec& matchesOu,
			int nLimit, int options, std::string* err )const
{
	std::string err3, *err2 = ( err ? err : &err3 );
	int i, nSubjLen2 = ( nSubjLen == -1 ? strlen(szSubj) : nSubjLen );
	for( i=0; i < nLimit || nLimit<0; i++ ){
		HfRegEx::MatchVec mt2;
		if( match3( szSubj, nSubjLen2, offset2, mt2, options, err2 ) ){
			matchesOu.push_back( mt2 );
			offset2 = mt2[0].offset + mt2[0].length;
		}else
			break;
	}
	return i;
}
int HfRegEx::
matchAll2( const char* szSubj, int nSubjLen, int offset2, MatchAllVec2& matchesOu,
				int nLimit, int options, std::string* err )const
{
	MatchAllVec mt3;
	if( matchAll( szSubj, nSubjLen, offset2, mt3, nLimit, options, err ) ){
		MatchAllVec::const_iterator a;
		MatchVec::const_iterator b;
		for( a = mt3.begin(); a != mt3.end(); ++a ){
			matchesOu.push_back( std::vector<std::pair<int,std::string> >() );
			for( b = a->begin(); b != a->end(); ++b ){
				std::pair<int,std::string> mtc;
				mtc.first  = b->offset;
				mtc.second.assign( szSubj + b->offset, b->length );
				matchesOu.back().push_back( mtc );
			}
		}
	}
	return (int)mt3.size();
}

int HfRegEx::
matchDataAll( const void* dataSubj, int nDataLen, int offset2, MatchAllVec& mtchsOu, int nLimit, int options, std::string* err )const
{
	options |= RE_NoUTF8Check;
	return matchAll( (const char*)dataSubj, nDataLen, offset2, mtchsOu, nLimit, options, err );
}

/// Optimized matching against stream of data potentially containing
/// non-ansi characters.
/// Shouldn't be used in matching regular text strings, as UTF-8 support gets disabled.
/// Pattern may contain hexadecimal character sepcifiers using slash-x notation,
/// fe. "\x20", "\x0D\x0B\xFF+", etc.
int HfRegEx::
matchDataAll2( const char* szPtrn, const void* dataSubj, int nDataLen, int offset2,
					MatchAllVec& mtchsOu, int nLimit, int options2, std::string* err )
{
	int options3 = options2;
	options3 |= RE_NoUTF8Check;
	options3 &= ~RE_UTF8;
	HfRegEx re2( szPtrn, options3 );
	int res = re2.matchAll( (const char*)dataSubj, nDataLen, offset2, mtchsOu,
							nLimit, options3, err );
	return res;
}

/// Escapes all reg-ex special characters by prepending them with back-slash.
/// Usefull for inserting strings into pattern for literal matching.
/// Notes:
/// * slash character ("/") is not considered a special character.
/// * dash is always considered as special character, even outside of
///   character class (square brackets "[]").
std::string HfRegEx::escape2( const char* in, int len )
{
	static const char* szReSpecialchars = "\\^$.[]|()?*+{}-";
	std::string z; int i;
	for( i=0; (len<0 || i<len) && in[i]; i++ ){
		if( strchr( szReSpecialchars, in[i] ) )
			z += "\\";
		z += in[i];
	}
	return z;
}

/// Returns true if reg-ex pattern is slash type.
/// Examples of slash type patterns: "/abc/i", "/def[xz]+/ix", " /xyz/".
/// Optional output std::pair parameter returns numeric positions of first and last slash.
/// Optional output parameters ptrnOu and modsOu can be used to retrieve embeded pattern and modifiers.
bool HfRegEx::
isSlashPattern( const char* inp, int len, std::pair<int,int>* outSl, std::string* ptrnOu, std::string* modsOu )
{
	int i;
	len = len < 0 ? (int)strlen(inp) : len;
	// skip any leading whitespaces.
	for( i=0; i<len && strchr( "\x20\r\n\t", inp[i] ); i++ );
	if( inp[i] == '/' ){
		const char* opSl = &inp[i], *endp = 0;
		const char* sz2 = inp + (len-1);
		for(; sz2 > opSl; sz2-- ){
			if( *sz2 == '/' ){
				endp = sz2;
				break;
			}
			if( !isalnum( *sz2 ) )
				break;
		}
		if( endp ){
			if(outSl){
				outSl->first = opSl - inp;
				outSl->second = endp - inp;
			}
			if(ptrnOu){
				const char* bgnPtrn2 = opSl + 1;
				ptrnOu->assign( bgnPtrn2, endp - bgnPtrn2 );
			}
			if( modsOu ){
				modsOu->assign( endp + 1 );
			}
			return 1;
		}
	}
	return 0;
}
int HfRegEx::convStrModsToFlags( const char* inp )
{
	int optsOu = 0;
	if( strchr( inp, 'i' ) )
		optsOu |= RE_Caseless;
	if( strchr( inp, 'm' ) )
		optsOu |= RE_Multiline;
	if( strchr( inp, 's' ) )
		optsOu |= RE_DotAll;
	if( strchr( inp, 'x' ) )
		optsOu |= RE_Extended;
	//if( strchr( inp, 'u' ) )
	//	optsOu |= RE_Ungreedy;
	return optsOu;
}
/// Converts string pattern that potentially has asterisiks and question-marks
/// characters in it into reg-ex pattern. Special characters get back-slash escaped.
/// This kind of simple pattern is used in file name matching, fe. Winapi
/// FindFirstFile() uses it.
std::string HfRegEx::
fromAsteriskQmPattern( const char* inp, int len )
{
	len = ( len == -1 ? strlen(inp) : len );
	const char* sz2, *sz3, *sz4, *ends = inp + len;
	std::string z, str;
	for( sz2 = inp; ; sz2++ ){
		sz3 = ( ( sz3 = strchr( sz2, '?' ) ) ? sz3 : ends );
		sz4 = ( ( sz4 = strchr( sz2, '*' ) ) ? sz4 : ends );
		if( sz3 == sz4 )
			break;
		sz3 = std::min( sz3, sz4 );
		str.assign( sz2, sz3 - sz2 );
		z += escape2( str.c_str(), str.size() );
		if( *sz3 == '?' ){
			z += ".";
		}else if( *sz3 == '*' ){
			z += ".*";
		}
		sz2 = sz3;
	}
	z += escape2( sz2, -1 );
	return z;
}


/// Performs RE matching and returns match already as string.
/// Returns empty string if there was no match or match yielded empty result.
/// nSubMatch may specify which sub-pattern to return, denoted as parentheses in
/// original RE pattern. value 0 (default) causes entire match to be returned, 1: first
/// sub-pattern, 2: second, etc.
std::string HfRegEx::
extract2( const char* szSubj, int nSubjLen, int offset2, int nSubMatch, int options2,
			int* offsetOu )const
{
	MatchVec mtch;
	if( match3( szSubj, nSubjLen, offset2, mtch, options2, 0 ) ){
		if( nSubMatch < (int)mtch.size() && mtch[nSubMatch].length ){
			std::string str;
			str.assign(
					szSubj + mtch[nSubMatch].offset, mtch[nSubMatch].length );
			if( offsetOu )
				*offsetOu = (int)mtch[nSubMatch].offset;
			return str;
		}
	}
	return "";
}

/// Splits subject string by current RE and stores and/or passes results to
/// output array and/or callback.
/// Result strings consist of substrings inbetween matches.
/// If there is no match, result is single string, the original input subject.
void HfRegEx::
explode4( const char* szSubj, int nSubjLen, int offset2,
		VecStr* outp2, bool(*outp3)( const SExplode& ), void* user2,
		int nLimit, int options2 )const
{
	int num; std::string str; SMatch seg; MatchVec mtch;
	SExplode spl;
	spl.szSubj = szSubj;
	spl.user2  = user2;
	for( num=0; match3( szSubj, nSubjLen, offset2, mtch, options2, 0 ); ){
		if( nLimit != -1 && num >= nLimit ) break;
		num++;
		seg.offset = offset2;
		seg.length = mtch.begin()->offset - offset2;
		if( outp2 ){
			str.assign( szSubj + seg.offset, seg.length );
			outp2->push_back( str );
		}
		offset2 = mtch.begin()->offset + mtch.begin()->length;
		if( outp3 ){
			spl.ptr   = szSubj + seg.offset;
			spl.len   = seg.length;
			spl.match = mtch;
			if( !outp3( spl ) )
				break;
		}
	}
	seg.offset = offset2;
	seg.length = strlen( szSubj + offset2 );
	if( outp2 ){
		str.assign( szSubj + seg.offset, seg.length );
		outp2->push_back( str );
	}
	if( outp3 ){
		mtch.clear();
		spl.ptr   = szSubj + seg.offset;
		spl.len   = seg.length;
		spl.match = mtch;
		outp3( spl );
	}
}

bool HfRegEx::fnReplParseEach( const SExplode& inp )
{
	SReplParsEach& sre( *((SReplParsEach*)inp.user2) );
	// first append string that preceds matched dollar var, param 'sz3'.
	sre.arReplParsed2->push_back(
			std::pair<int,std::string>( -1, std::string( inp.ptr, inp.len ) ) );
	if( inp.match.size() < 3 )
		return 1;
	char su2 = sre.szSubj2[ inp.match[1].offset ];
	if( su2 != '$' ){ // if not a dollar var.
		sre.arReplParsed2->push_back(
				std::pair<int,std::string>( -1, std::string(
						sre.szSubj2 + inp.match[0].offset,
						inp.match[0].length ) ) );
		return 1;
	}
	const char* szNum = sre.szSubj2 + inp.match[2].offset;
	int num = atoi( szNum );
	sre.arReplParsed2->push_back( std::pair<int,std::string>(num,"") );
	return 1;
}

bool HfRegEx::fnEachSubstMatch( const SExplode& inp )
{
	SEachRepl& srp( *((SEachRepl*)inp.user2) );
	srp.strOutp->append( inp.ptr, inp.len );
	// if not last split segment, ie. split segment is folloed by re-match string.
	if( !inp.match.empty() ){
		if( srp.options2 & RE_SubstUseDollarN ){
			if( srp.arReplParsed.empty() ){
				SReplParsEach sre;
				sre.arReplParsed2 = &srp.arReplParsed;
				sre.szSubj2       = srp.szRepl;
				HfRegEx("/ (\\$) (\\d+) /ix", 0 )
					.explode4( srp.szRepl, -1,0,0, fnReplParseEach, (void*)&sre, -1,0 );
				if( srp.arReplParsed.empty() )
					srp.arReplParsed.push_back( std::pair<int,std::string>(-1,"") );
			}
			std::vector<std::pair<int,std::string> >::const_iterator a;
			for( a = srp.arReplParsed.begin(); a != srp.arReplParsed.end(); ++a ){
				if( a->first == -1 ){
					srp.strOutp->append( a->second );
				}else if( a->first < (int)inp.match.size() ){
					int ofs2 = inp.match[a->first].offset;
					int len2 = inp.match[a->first].length;
					srp.strOutp->append( srp.szSubj + ofs2, len2 );
				}
			}
		}else{
			srp.strOutp->append( srp.szRepl );
		}
	}
	return 1;
}
/// Substitute string occurences with provided replacement, alias string-replace.
/// If RE_SubstUseDollarN is specified, occurences of $<n>
/// (for example, $0, $1, $2, ... $9) in replacement are replaced
/// with the corresponding captured (sub-)string.
/// $0 is the captured string. $1 - $n are the substrings maching the subpatterns.
/// The subpatterns are parts of the original pattern denoted by parentheses
/// (round braces ("()")).
/// \param options2 - fe. \ref RE_SubstUseDollarN.
/// \sa GP_str_srch_replace
void HfRegEx::
subst2( const char* subj, int nSubjLen, int offset2, const char* szReplacement,
			std::string& outp, int nLimit, int options2 )const
{
	SEachRepl srp;
	srp.thisp = this;
	srp.szRepl = szReplacement;
	srp.strOutp = &outp;
	srp.options2 = options2;
	srp.szSubj = subj;
	explode4( subj, nSubjLen, offset2, 0, fnEachSubstMatch, (void*)&srp, nLimit, options2 );
}

std::string HfRegEx::getPcreVersion()
{
	const char* sz2 = pcre_version();
	if( sz2 ){
		return sz2;
	}
	return "";
}
void HfRegEx::explode3( const char* subj, VecStr& outp2 )const
{
	explode4( subj, -1, 0, &outp2, 0,0, -1, 0 );
}
/// RE str explode into array.
/// \sa GP_str_split_funcs
HfRegEx::VecStr HfRegEx::explode2( const char* subj )const
{
	VecStr outp4;
	explode4( subj, -1, 0, &outp4, 0,0, -1, 0 );
	return outp4;
}
/// RE matching.
/// \sa GP_strcmp_funcs
bool HfRegEx::match2( const char* szSubj, int nSubjLen, int offset )const
{
	MatchVec msink;
	return match3( szSubj, nSubjLen, offset, msink, 0, 0 );
}

void HfRegEx::
convMatchToStrArr( const char* szSubj, int nSubjLen, const MatchVec& inp, std::vector<std::pair<int,std::string> >& outp )
{
	typename MatchVec::const_iterator a;
	for( a = inp.begin(); a != inp.end(); ++a ){
		outp.push_back( std::make_pair( a->offset,
					std::string( szSubj + a->offset, a->length ) ) );
	}
}
/**
	RE match for recursively occuring elements, fe XML element.
	\param reOp     - RE pattern to match XML open tags (fe. "<div>").
	\param reOpRcv  - optional, RE to match consecutively recursive XML open tags.
					  if set to NULL, 'reOp' is used in it's place.
					  intended to be set to widely matching pattern than 'reOp' is.
					  ex: if 'reOp' set to "<div.*?name=abc.*?>", set 'reOpRcv' to
					  "<div.*?>"
	\param reCl     - RE to search for close tags (fe. "<\/div>").
	\param matchOu  - result \ref HfRegEx::SMatchXML.
	\param options2 - options, [match3] and [matchXml2] specific, fe. \ref HfRegEx::RE_XMLNoRcvCapt.
	\param err2     - optional, on error, stored brief message.
*/
bool HfRegEx::matchXml2( const HfRegEx& reOp, const HfRegEx* reOpRcv, const HfRegEx& reCl,
					const char* szSubj, int nSubjLen, int offset2,
					SMatchXML& matchOu, int options2, std::string* err2 )
{
	nSubjLen = ( nSubjLen < 0 ? strlen(szSubj) : nSubjLen );
	reOpRcv  = ( reOpRcv ? reOpRcv : &reOp );
	std::string err3, *err = ( err2 ? err2 : &err3 );
	MatchVec mt2, mt3; int rs2, rs3;
	matchOu.bResUnordered = !!( options2 & RE_XMLCaptByPos );
	std::vector<MatchVec9> mt2b;
	if( reOp.match3( szSubj, nSubjLen, offset2, mt2, options2, err ) ){
		int lvl = 1;
		mt2b.push_back( MatchVec9( mt2, lvl, 0 ) );
		offset2 = mt2[0].offset + mt2[0].length;
		while( lvl ){
			matchOu.nLvlMax = ( matchOu.nLvlMax < lvl ? lvl : matchOu.nLvlMax );
			mt2.clear();
			mt3.clear();
			rs2 = reOpRcv->match3( szSubj, nSubjLen, offset2, mt2, options2, err );
			rs3 = reCl.    match3( szSubj, nSubjLen, offset2, mt3, options2, err );
			rs2 = ( rs2 ? mt2[0].offset : nSubjLen );
			rs3 = ( rs3 ? mt3[0].offset : nSubjLen );
			if( rs2 < rs3 ){
				++lvl;
				offset2 = rs2 + mt2[0].length;
				mt2b.push_back( MatchVec9( mt2, lvl+0, 0 ) );
			}else if( rs3 < rs2 ){
				--lvl;
				offset2 = rs3 + mt3[0].length;
				mt2b.push_back( MatchVec9( mt3, lvl+1, 1 ) );
			}else{
				if( rs3 == nSubjLen ){ // also == rs2, if not found either tag.
					*err = "couldn't find recursive close tag.";
					return 0;
				}else{ // else: open and close patterns, both match at same pos.
					*err = "open and close, match at same pos.";
					return 0;
				}
			}
		}
		if( lvl ){
			*err = "unknown error."; // code should never reach this place.
			return 0;
		}
		if( !(options2 & RE_XMLNoRcvCapt) ){
			std::vector<MatchVec9>::const_iterator b,c;
			if( !(options2 & RE_XMLCaptByPos) ){
				bool bHasClose = 0;
				for( b = mt2b.begin(); b != mt2b.end(); ++b ){
					if( !b->bClosing ){
						matchOu.mTags.resize( matchOu.mTags.size() + 1 );
						convMatchToStrArr( szSubj, nSubjLen, b->mtch, matchOu.mTags.back() );
						c = b;
						bHasClose = 0;
						for( ++c; c != mt2b.end(); ++c ){
							if( c->bClosing && c->nLvl == b->nLvl ){
								matchOu.mTags.resize( matchOu.mTags.size() + 1 );
								convMatchToStrArr( szSubj, nSubjLen, c->mtch, matchOu.mTags.back() );
								bHasClose = 1;
								break;
							}
						}
						if( !bHasClose ){
							// place should never be reached. add empty match anyway.
							matchOu.mTags.resize( matchOu.mTags.size() + 1 );
							matchOu.mTags.back().resize( 1 );
							matchOu.mTags.back().back().first = 0;
						}
					}
				}
			}else{
				for( b = mt2b.begin(); b != mt2b.end(); ++b ){
					matchOu.mTags.resize( matchOu.mTags.size() + 1 );
					convMatchToStrArr( szSubj, nSubjLen, b->mtch, matchOu.mTags.back() );
				}
			}
		}else{ // else, store only first and last captured, as open and close tags.
			matchOu.mTags.resize( 2 );
			if( !mt2b.empty() ){
				convMatchToStrArr( szSubj, nSubjLen, mt2b[0].mtch, matchOu.mTags[0] );
			}else{
				matchOu.mTags[0].resize( 1 );
				matchOu.mTags[0].back().first = 0;
			}
			if( mt2b.size() >= 2 ){
				convMatchToStrArr( szSubj, nSubjLen, mt2b.back().mtch, matchOu.mTags[1] );
			}else{
				matchOu.mTags[1].resize( 1 );
				matchOu.mTags[1].back().first = 0;
			}
		}
		return 1;
	}
	return 0;
}

/// Returns start offset and size of full match in original subject string.
/// \return start offset.
int HfRegEx::SMatchXML::getFullElem2( int* sizeOu, bool bOnlyContents )const
{
	*sizeOu = 0;
	if( mTags.size() < 2 || mTags[0].empty() )
		return 0;
	int nOffset = mTags[0][0].first + ( bOnlyContents ? mTags[0][0].second.size() : 0 );
	int nEnd = 0;
	if( bResUnordered ){
		if( mTags.back().empty() )
			return 0;
		nEnd =  mTags.back()[0].first;
		nEnd += ( bOnlyContents ? 0 : mTags.back()[0].second.size() );
	}else{
		nEnd =  mTags[1][0].first;
		nEnd += ( bOnlyContents ? 0 : mTags[1][0].second.size() );
	}
	if( nEnd <= nOffset )
		return 0;
	*sizeOu = nEnd - nOffset;
	return nOffset;
}

std::string HfRegEx::SMatchXML::
getFullElem( const char* szOrigSubj, bool bOnlyContents )const
{
	int nLen = 0;
	int nOffset = getFullElem2( &nLen, bOnlyContents );
	return std::string( szOrigSubj + nOffset, nLen );
}



} // end namespace hef







