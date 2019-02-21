
#ifndef _C_HEADER_CLTR_H_
#define _C_HEADER_CLTR_H_
#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif
#include <string>
#include <vector>
#include <stdio.h>
#include "hef_str_from_data_cltr.h"
using namespace hef;

/**
	HTTP response header collector.
	Call moreData() whenever more data from server arrives to allow collector
	parse it. moreData() will return status about how much info has
	been collected already. Use EHHHCF_CollectClientRqInstead constructor flag to turn
	into server-response-mode, that is, handling headers sent by HTTP client.

	moreData() returns such info as:
	    * RETVAL 0 indicates error. on error, text message is also available.
	    * number of bytes consumed in current call, which may be part-of or
	      entire input data.
	    * whenever completed collecting header data ('bGotAll' set to 1 if so).
	    * what has been spotted in current moreData() call as 'uSpotedFlags',
	      i.e. protocol has been identified, Status Code recieved, etc.
	      so that user may want to analyse it prior alowing recieving
	      further data.

	\verbatim //{
	Example HTTP response from server:
		HTTP/1.1 206 Partial Content
		Content-Type: video/x-flv
		Cache-Control: private, max-age=22481
		Content-Range: bytes 4028456-32739076/32739077
		Accept-Ranges: bytes
		Content-Length: 28710621
		Connection: close

	Example HTTP response from server 2:
		HTTP/1.1 200 OK
		Date: Sun, 03 Mar 2013 11:13:51 GMT
		Server: Apache/2.2.12 (Win32) DAV/2 mod_ssl/2.2.12 OpenSSL/0.9.8k mod_autoindex_color PHP/5.3.0
		Last-Modified: Sun, 03 Mar 2013 11:13:23 GMT
		ETag: "90000000001f2-271-4d7035424eab3"
		Accept-Ranges: bytes
		Content-Length: 625
		Connection: close
		Content-Type: application/zip

	Example HTTP request from client 1:
		GET / HTTP/1.1
		Host: localhost
		User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:18.0) Gecko/20100101 Firefox/18.0
		Accept: text/html,application/xhtml+xml,application/xml;q=0.9,* / *;q=0.8
		Accept-Language: en-US,en;q=0.5
		Accept-Encoding: gzip, deflate
		Cookie: phpbb3_pgmlw_k=0aaddeaa2x6e; style_cookie=printonly
		Connection: keep-alive

	Example HTTP response from server 3:
		HTTP/1.1 200 OK
		Date: Tue, 19 Mar 2013 09:06:25 GMT
		Expires: Fri, 01 Jan 1990 00:00:00 GMT
		Cache-Control: no-cache, must-revalidate
		Pragma: no-cache
		X-Frame-Options: SAMEORIGIN
		Content-Type: text/html; charset=UTF-8
		Content-Language: en
		Server: HTTP server (unknown)
		X-XSS-Protection: 1; mode=block
		Connection: close

	Example Proxy HTTP request from client 2:
		POST http://localhost/phpbb3_forum/ucp.php?mode=register HTTP/1.1\r\n
		Host: localhost\r\n
		User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:18.0) Gecko/20100101 Firefox/18.0\r\n
		Accept: text/html,application/xhtml+xml,application/xml;q=0.9,* / *;q=0.8\r\n
		Accept-Language: en-US,en;q=0.5\r\n
		Accept-Encoding: gzip, deflate\r\n
		Referer: http://localhost/phpbb3_forum/ucp.php?mode=register\r\n
		Cookie: phpbb3_qtllb_k=; phpbb3_qtllb_u=1; PHPSESSID=qrt1mpghb16qt5m\r\n
		Connection: keep-alive\r\n
		Content-Type: application/x-www-form-urlencoded\r\n
		Content-Length: 119\r\n
		\r\n
		agreed=I+agree+to+these+terms&change_lang=&creation_time=1363735554&form_token=92f584174864bd48cab87d10702f0d15c32363e7

	Example Proxy HTTP request from client 3:
		GET http://localhost/phpbb3_forum/styles/prosilver/theme/large.css HTTP/1.1\r\n
		Host: localhost\r\n
		User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:18.0) Gecko/20100101 Firefox/18.0\r\n
		Accept: text/css,* / *;q=0.1\r\n
		Accept-Language: en-US,en;q=0.5\r\n
		Accept-Encoding: gzip, deflate\r\n
		DNT: 1\r\n
		Referer: http://localhost/phpbb3_forum/index.php?sid=73c0ab361418a14dd1d0006b186986e4\r\n
		Cookie: phpbb3_pgmlw_k=0a297eddeaae656e; style_cookie=printonly; phpbb3_pgmlw_u=2; phpbb3_pgmlw_sid=1083511652454df70865204940a007ee; phpbb3_qtllb_k=;phpbb3_qtllb_u=1; phpbb3_qtllb_sid=73c0ab361418a14dd1d0006b186986e4\r\n
		Connection: keep-alive\r\n
		If-Modified-Since: Mon, 20 Aug 2012 15:32:34 GMT\r\n
		If-None-Match: "b00000002b4f9-1a-4c7b437f9c880"\r\n
		Cache-Control: max-age=0\r\n

	\endverbatim //}

	References:
	* "rfc2616 - Hypertext Transfer Protocol -- HTTP-1.1.txt"

	\todo rename from CHeaderCltr2 to CHTTPHeaderCollector
	\todo rename from CHTTPHeaderCollector to HfHTTPHeaderCollector
*/
class HfHTTPHeaderCollector {
public:
	enum EHdrCltrMode {
		// Still tries to identify protocol.
		// Still checks if data begins with bytes:
		//     48545450         HTTP
		// or, longer variant:
		//     485454502F312E31 HTTP/1.1
		EHCM_ProtocolIdentify,
		// I.e. "HTTP/1.1 206 Partial Content",
		// rfc2616 HTTP - 6.1 Status-Line.
		EHCM_StatusLine,
		// Consecutive Headers, i.e. "Accept-Ranges: bytes", "Connection: close", etc.
		EHCM_ConsecutiveHeaders,
		//
		EHCM_Done,
		// When error happens mode is set to this value.
		EHCM_Error,
		// ---
		EHCM_ClientRqLine,	//valid in rq mode only.
	};
	enum ESpottedFlags {
		// Indicates first four bytes that arrived has been identified as "HTTP".
		ESF_ProtocolOK = 0x1,
		// Indicates status code has been readd.
		ESF_StatusCode = 0x2,
		// Indicates more consecutive headers has been read.
		ESF_MoreConsecutveHeaders = 0x4,
		//
		ESF_RequestLine = 0x8,	//valid in rq mode only.
	};
	enum EGHBKF{
		EGHBKF_CaseInsensitive = 0x1,
	};
	enum {
		EHHHCF_CollectClientRqInstead = 0x1,
	};
	// Constructor.
	HfHTTPHeaderCollector( size_t flagsx = 0 );
	bool               moreData( const void* data, size_t size, size_t* uConsumed, std::string& err, bool* bGotAll = 0, size_t* uSpotedFlags = 0 );
	EHdrCltrMode       mode()const { return Mode; }
	bool               isStatusCodeValid()const { return !!StatusCode; }
	size_t             getStatusCode()const;
	bool               hasAllConsecutiveHeaders()const { return GotAll; }
	const std::vector<std::string>&
	                   getConsecutiveHeaders()const { return HeaderFields; }
	const std::vector<std::pair<std::string,std::string> >&
	                   getConsecutiveHeaders2()const { return HeaderFieldsKv; }
	std::string        getHeaderByKey( const char* key, size_t flagsx )const;
	std::pair<std::string,std::string>&
	                   getHeaderByKey2( const char* key, size_t flagsx = 0 );
	// getRequestMethod() and getRequestText() functions are available only
	// in Client-request mode. See EHHHCF_CollectClientRqInstead.
	const char*        getRequestMethod()const {return !RequestLine2.empty() ? RequestLine2[0].c_str() : "";}
	const char*        getRequestText()const {return RequestLine2.size() >= 2 ? RequestLine2[1].c_str() : "";}
	std::string        getRequestLine()const;
	static std::string strPrintHeaders( const std::vector<std::pair<std::string,std::string> >& headers, const char* szEachLinePrefix="", const char* szNewlineMode="\n" );
	std::string        getStatusLine()const {return StatusLine;}
	std::string        toHTTPHeadersString( bool bNoExtraCrLf = 0 )const;
private:
	static size_t      getStatusCodeFromStatusLine( const char* staline );
	static std::string findHeaderByKey( const char* szKey, bool bCaseInsensitive, const std::vector<std::pair<std::string,std::string> >& keyvalarr );
	static std::pair<std::string,std::string>&
	                   findHeaderByKey2( const char* szKey, bool bCaseInsensitive, std::vector<std::pair<std::string,std::string> >& keyvalarr );
	static std::pair<std::string,std::string> oneConsecutiveHeaderToKeyVal( const char* in );
	static std::string strWithCrLfShown( const char* in, bool bInclLineBreaks=1 );
	static std::string dataToAsciiNlSpec( const void* data, size_t size );
	static size_t      collectDataUpToPartialSize( const void* data, size_t size, size_t untill, std::string& partial );
	static std::string str_ToLower( const char* in );
	static std::vector<std::string> decodeHTTPRequestLine( const char* in );
private:
	EHdrCltrMode             Mode;
	std::string              Partial;
	size_t                   PartialIterationCnt;
	HfCStrFromDataCltr       Collector;
	std::string              StatusLine;
	std::vector<std::string> HeaderFields;	//Response Header Fields.
	std::vector<std::pair<std::string,std::string> > HeaderFieldsKv;	//as key-value pairs.
	size_t                   StatusCode;
	bool                     GotAll;
	size_t                   Flags;
	std::vector<std::string> RequestLine2;
};

#endif //_C_HEADER_CLTR_H_
