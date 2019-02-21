
#include "hef_http_header_collector.h"
#include <cctype>
#include <cstring>
#include <assert.h>
#include "hef_data.h"
#include "hef_str_args.h"

/// Constructor.
/// flagsx - i.e. EHHHCF_CollectClientRqInstead.
HfHTTPHeaderCollector::HfHTTPHeaderCollector( size_t flagsx )
	: Mode(EHCM_ProtocolIdentify), PartialIterationCnt(0)
	, Collector(0), StatusCode(0), GotAll(0), Flags(flagsx)
{
	if( Flags & EHHHCF_CollectClientRqInstead )
		Mode = EHCM_ClientRqLine;
}
std::string HfHTTPHeaderCollector::dataToAsciiNlSpec( const void* data_, size_t size )
{
	const unsigned char* data = (const unsigned char*)data_;
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
// returns number of consumed bytes.
size_t HfHTTPHeaderCollector::
collectDataUpToPartialSize( const void* data, size_t size, size_t uPartialSizeTarget,
             std::string& partial )
{
	size_t consumed = 0;
	size_t need = partial.size() < uPartialSizeTarget ?
		uPartialSizeTarget - partial.size() : 0;
	if( need ){
		size_t needMaxPsbl = std::min<size_t>( need, size );
		partial += dataToAsciiNlSpec( data, needMaxPsbl );
		consumed = needMaxPsbl;
	}
	return consumed;
}
std::string HfHTTPHeaderCollector::strWithCrLfShown( const char* in, bool bInclLineBreaks )
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
size_t HfHTTPHeaderCollector::getStatusCodeFromStatusLine( const char* staline )
{
	const char* pos;
	pos = strchr( staline, 0x20 );
	if( pos ){
		pos++;
		static const char* digits = "0123456789";
		for(; *pos && !strchr(digits,*pos); pos++ );
		if(*pos){
			size_t uStatus = (size_t)atoi( pos );
			return uStatus;
		}
	}
	return 0;
}
// valid only if isStatusCodeValid() returns 1.
size_t HfHTTPHeaderCollector::getStatusCode()const
{
	return StatusCode;
}
std::pair<std::string,std::string> HfHTTPHeaderCollector::
oneConsecutiveHeaderToKeyVal( const char* in )
{
	const char* ptr = in;
	std::pair<std::string,std::string> z;
	for(; *ptr; ptr++ ){
		if(*ptr == ':'){
			z.first.assign( in, ptr-in );
			break;
		}
	}
	if(*ptr == ':'){
		ptr++;
		for(; *ptr && *ptr == 0x20; ptr++ );
		z.second = ptr;
	}else{
		z.second = in;
	}
	return z;
}
std::string HfHTTPHeaderCollector::str_ToLower( const char* in )
{
	std::string z;
	for(; *in; in++ ){
		z += (char) tolower(*in);
	}
	return z;
}
std::string HfHTTPHeaderCollector::
findHeaderByKey( const char* szKey, bool bCaseInsensitive,
				const std::vector<std::pair<std::string,std::string> >& arr )
{
	std::vector<std::pair<std::string,std::string> >::const_iterator a;
	size_t uKeyLen = strlen(szKey);
	std::string strKeyLwr, strCurLwr;
	if(bCaseInsensitive)
		strKeyLwr = str_ToLower( szKey );
	for( a = arr.begin(); a != arr.end(); ++a ){
		if( bCaseInsensitive ){
			strCurLwr = str_ToLower( a->first.c_str() );
			if( uKeyLen == strCurLwr.size() && !strcmp( strCurLwr.c_str(), strKeyLwr.c_str() ) )
				return a->second;
		}else{
			if( uKeyLen == a->first.size() && !strcmp( a->first.c_str(), szKey ) )
				return a->second;
		}
	}
	return "";
}
std::pair<std::string,std::string>& HfHTTPHeaderCollector::
findHeaderByKey2( const char* szKey, bool bCaseInsensitive, std::vector<std::pair<std::string,std::string> >& arr )
{
	static std::pair<std::string,std::string> dfltEmpty;
	std::vector<std::pair<std::string,std::string> >::iterator a;
	size_t uKeyLen = strlen(szKey);
	std::string strKeyLwr, strCurLwr;
	if(bCaseInsensitive)
		strKeyLwr = str_ToLower( szKey );
	for( a = arr.begin(); a != arr.end(); ++a ){
		if( bCaseInsensitive ){
			strCurLwr = str_ToLower( a->first.c_str() );
			if( uKeyLen == strCurLwr.size() && !strcmp( strCurLwr.c_str(), strKeyLwr.c_str() ) ){
				return *a;
			}
		}else{
			if( uKeyLen == a->first.size() && !strcmp( a->first.c_str(), szKey ) ){
				return *a;
			}
		}
	}
	return dfltEmpty;
}
std::pair<std::string,std::string>& HfHTTPHeaderCollector::
getHeaderByKey2( const char* key, size_t flagsx )
{
	return findHeaderByKey2( key, !!(flagsx & EGHBKF_CaseInsensitive), HeaderFieldsKv );
}
// Caled by user to provide more data to collect.
// uSpotedFlags - i.e. ESF_ProtocolOK
bool HfHTTPHeaderCollector::
moreData( const void* data_, size_t size, size_t* uConsumed, std::string& err,
         bool* bGotAll, size_t* uSpotedFlags )
{
	const unsigned char* data = (unsigned char*)data_;
	size_t uSpotedFlagsDmy = 0, *uSpotedFlags3;
	bool bGotAll2, *bGotAll3 = ( bGotAll ? bGotAll : &bGotAll2 );
	if(uSpotedFlags)
		uSpotedFlags3 = uSpotedFlags;
	else
		uSpotedFlags3 = &uSpotedFlagsDmy;
	*uConsumed = 0;
	*bGotAll3 = 0;
	if( Mode == EHCM_ProtocolIdentify ){
		PartialIterationCnt++;
		while( Partial.size() < 4 && size ){
			size_t cnsmd = collectDataUpToPartialSize( data, size, 4, Partial );
			size -= cnsmd;
			data += cnsmd;
			*uConsumed += cnsmd;
		}
		assert( Partial.size() <= 4 );
		if( Partial.size() == 4 ){
			if( !strcmp( "HTTP", Partial.c_str() ) ){
				Partial = "";
				PartialIterationCnt = 0;
				Collector.clear();
				Mode = EHCM_StatusLine;
				//printf("Got 4 'HTTP' bytes.\n");
				*uSpotedFlags3 |= ESF_ProtocolOK;
			}else{
				Mode = EHCM_Error;
				err = "Not recieved 'HTTP' as first four bytes.";
				return 0;
			}
		}else{
			if( PartialIterationCnt > 4 ){
				Mode = EHCM_Error;
				err = "Iteration count for protocol identify exceeded 4.";
				return 0;
			}
		}
	}
	if( Mode == EHCM_StatusLine && size ){
		// status line, past "HTTP" bytes.
		size_t uCnsmd = 0; bool bGotNl = 0, rs=0;
		rs = Collector.collect( data, size, "\r\n", &uCnsmd, &bGotNl );
		if(!rs){
			Mode = EHCM_Error;
			err = "Failed collecting full HTTP Status Line.";
			return 0;
		}
		*uConsumed += uCnsmd;
		data       += uCnsmd;
		size       -= uCnsmd;
		if(bGotNl){
			StatusLine = std::string("HTTP") + Collector.getConsumedString();
			StatusLine = StatusLine.substr( 0, StatusLine.size() - 2 );
			//printf("Got HTTP Status line: [%s].\n",
			//	strWithCrLfShown(StatusLine.c_str(),0).c_str() );
			StatusCode = getStatusCodeFromStatusLine( StatusLine.c_str() );
			if(!StatusCode){
				Mode = EHCM_Error;
				err = "No Status Code in Status Line.";
				return 0;
			}
			*uSpotedFlags3 |= ESF_StatusCode;
			Mode = EHCM_ConsecutiveHeaders;
			Collector.clear();
		}
	}
	if( Mode == EHCM_ClientRqLine && size ){
		size_t uCnsmd = 0; bool bGotNl = 0, rs=0;
		rs = Collector.collect( data, size, "\r\n", &uCnsmd, &bGotNl );
		if(!rs){
			Mode = EHCM_Error;
			err = "Failed collecting HTTP request line from client.";
			return 0;
		}
		*uConsumed += uCnsmd;
		data       += uCnsmd;
		size       -= uCnsmd;
		if(bGotNl){
			// request line i.e. "GET / HTTP/1.1"
			std::string str = Collector.getConsumedString();
			str = str.substr( 0, str.size() - 2 );
			RequestLine2 = decodeHTTPRequestLine( str.c_str() );
			if( RequestLine2.size() != 3 ){
				Mode = EHCM_Error;
				err = "Request Line has invalid format.";
				return 0;
			}
			*uSpotedFlags3 |= ESF_RequestLine;
			Mode = EHCM_ConsecutiveHeaders;
			Collector.clear();
		}
	}
	if( Mode == EHCM_ConsecutiveHeaders && size ){
		size_t uCnsmd2 = 0; bool bGotNl2 = 0, rs=0; std::pair<std::string,std::string> kv;
		for(; size; ){
			rs = Collector.collect( data, size, "\r\n", &uCnsmd2, &bGotNl2 );
			if(!rs){
				Mode = EHCM_Error;
				err = "Failed parsing Consecutive HTTP Headers.";
				//printf("ERROR: %s\n", err.c_str() );
				return 0;
			}
			*uConsumed += uCnsmd2;
			data       += uCnsmd2;
			size       -= uCnsmd2;
			if(bGotNl2){
				std::string strHdrLn1 = Collector.getConsumedString();
				Collector.clear();
				strHdrLn1 = strHdrLn1.substr( 0, strHdrLn1.size() - 2 );
				if( strHdrLn1.empty() ){	//if reached empty line terminated by CrLf.
					Mode = EHCM_Done;
					*bGotAll3 = 1;
					GotAll = 1;
					return 1;
				}
				*uSpotedFlags3 |= ESF_MoreConsecutveHeaders;
				//printf("Got one header: [%s]\n", strHdrLn1.c_str() );
				HeaderFields.push_back( strHdrLn1 );
				kv = oneConsecutiveHeaderToKeyVal( strHdrLn1.c_str() );
				HeaderFieldsKv.push_back(kv);
			}
		}
	}
	return 1;
}
// flagsx - i.e. EGHBKF_CaseInsensitive.
std::string HfHTTPHeaderCollector::getHeaderByKey( const char* key, size_t flagsx )const
{
	//return findHeaderByKey( key, !!(flagsx & EGHBKF_CaseInsensitive), HeaderFieldsKv );
	std::vector<std::pair<std::string,std::string> >*
		ptr = (std::vector<std::pair<std::string,std::string> >*) &HeaderFieldsKv;
	return ((HfHTTPHeaderCollector*)this)->findHeaderByKey2( key,
		!!(flagsx & EGHBKF_CaseInsensitive), *ptr ).second.c_str();
}
std::string HfHTTPHeaderCollector::
strPrintHeaders( const std::vector<std::pair<std::string,std::string> >& headers,
				const char* szEachLinePrefix, const char* szNewlineMode )
{
	std::string z;
	std::vector<std::pair<std::string,std::string> >::const_iterator a = headers.begin();
	for(; a != headers.end(); ++a ){
		z += szEachLinePrefix + a->first + ": " + a->second + szNewlineMode;
	}
	return z;
}
/// Returns 3 elements in array on success. Method, request string and protocol version.
/// Input status line string must be w/o any trailing Cr/Lf.
std::vector<std::string> HfHTTPHeaderCollector::decodeHTTPRequestLine( const char* in )
{
	std::vector<std::string> z; const char* sz = in, *sz2;
	z.resize(3,"");
	if( ( sz2 = hf_strchr( sz, '\x20' ) ) ){
		z[0].assign( sz, sz2-sz );	//copy method. "GET", "POST", etc.
		sz = sz2 + 1;
		if( ( sz2 = hf_strchr( sz, '\x20' ) ) ){
			z[1].assign( sz, sz2-sz );	//copy request str. "/", "index.htm", etc.
			z[2].assign( sz2 + 1 );		//copy HTTP version. i.e. "HTTP/1.1".
			return z;
		}
	}
	// error.
	z.clear();
	return z;
}
// Returns recreated request line.
std::string HfHTTPHeaderCollector::getRequestLine()const
{
	std::string z;
	std::vector<std::string>::const_iterator a;
	for( a = RequestLine2.begin(); a != RequestLine2.end(); ++a ){
		if(!z.empty())
			z += "\x20";
		z += a->c_str();
	}
	return z;
}
/// Converts all headers, including status/request line, to string
/// as if it was original HTTP request (or response).
std::string HfHTTPHeaderCollector::toHTTPHeadersString( bool bNoExtraCrLf )const
{
	std::string z;
	if( Flags & EHHHCF_CollectClientRqInstead ){
		z += getRequestLine() + "\r\n";
	}else{
		z += getStatusLine() + "\r\n";
	}
	// std::vector<std::pair<std::string,std::string> > HeaderFieldsKv
	std::vector<std::pair<std::string,std::string> >::const_iterator a;
	for( a = HeaderFieldsKv.begin(); a != HeaderFieldsKv.end(); ++a ){
		z += HfArgs("%1: %2\r\n").arg(a->first.c_str()).arg(a->second.c_str()).c_str();
	}
	if( !bNoExtraCrLf )
		z += "\r\n";
	return z;
}
