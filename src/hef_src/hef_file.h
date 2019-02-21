
#ifndef _HEF_FILE_H_
#define _HEF_FILE_H_
#ifdef __GNUC__
	// required by ftello(), makes 'off_t' type 64-bit size.
	// required on 32-bit platforms, 64-bit platforms seems to be ok without it.
	// if problems from here, otherwise this should be defined from compiler
	// command-line (makefile or workspaces).
#	define _FILE_OFFSET_BITS 64
#endif
#include <vector>
#include <stdint.h>
#include "hef_str.h"
namespace hef{

;
class HfIStdCopyFile;

/// \cond DOXYGEN_SKIP
;
void file_functs_general_tester();
/// \endcond //DOXYGEN_SKIP
;

uint64_t         hf_GetFileSize64( FILE* fph );
uint64_t         hf_GetFileSize64Fn( const char* fnm );
bool             hf_Seek64( FILE* fph, uint64_t position );
//uint64_t       hf_Tell64( FILE* fph );
uint64_t         hf_GetFilePos64( FILE* hf );
enum { EGFC64_NOFLAGS = 0, EGFC64_STOPONNONPRINTS = 0x1, };
std::string      hf_GetFileStrContents64( const char* fn, uint64_t addr = 0, int max_read = -1, size_t flags = 0, std::string* outp2 = 0 );
void             hf_GetFileDSVContents( const char* fn, const char* delim, std::vector<std::string>& outDSV, const char* szTrimcharsLR = 0 );
std::vector<uint8_t> hf_ReadFileBytes64( FILE* fp, uint64_t addr, size_t size_read, std::vector<uint8_t>* outp = 0 );
bool             hf_FileExists( const char* fname );
bool             hf_DirExists( const char* szPath, bool* bNotDir=0 );
bool             hf_IsDirectory( const char* szPath );
bool             hf_Mkdir( const char* szFn );
bool             hf_Unlink( const char* szFn );
bool             hf_RmDir( const char* szFn );
bool             hf_RenameFileOrDir( const char* szOldFn, const char* szNewFn );
bool             hf_StdCopyFile( const char* src, const char* dst, HfIStdCopyFile* intrf = 0 );
void             hf_StdoutBeginRedirect( const char* szFname, size_t flags = 0 );
void             hf_StdoutEndRedirect();
//bool           hf_ParseSimpleIniFile_1( const char* szIniFn, std::vector<std::pair<std::string,std::string> >& outp );
bool             hf_GetCommandLineSOSpec( std::vector<std::string>& outp, uint32_t uPid );
std::string      hf_getcwd();
std::string      hf_getHomeDir();
bool             hf_ClearLibFromLdPreloadEnv( void* pAnySymbol );

//template<typename T>
//bool hf_ParseSimpleIniFile( const char* szIniFn, T& outp );
//typedef std::vector<std::pair<std::string,std::string> > lsxt;
//typedef hf_ParseSimpleIniFile< lsxt > hf_ParseSimpleIniFile3;
//template<typename T> void fnTs2( int a, T& b );
//template bool hf_ParseSimpleIniFile2<lsxt>( const char* szIniFn, lsxt& outp );

/**
	Simple INI file parse without sections.
	Sections instead are returned as items with key set to section name and empty value.
	Output is a list of string key-value pairs.
*/
template<typename T>
bool hf_ParseSimpleIniFile( const char* szIniFn, T& outp )
{
	std::string str, str2, strKey, strVal;
	hf_GetFileStrContents64( szIniFn, 0, -1, 0x0, &str );
	// convert to array of lines, split by LF ("\n").
	std::vector<std::string>::iterator a;
	std::vector<std::string> lines2;
	hf_explode<char>( str.c_str(), "\n", lines2, 0 );
	for( a = lines2.begin(); a != lines2.end(); ){
		int len;
		const char* sz = hf_trim<char>( a->c_str(), "", &len );
		str2.assign( sz, len );
		if( str2.empty() ){
			a = lines2.erase(a);
		}else{
			++a;
		}
	}
	std::vector<std::string>::const_iterator b;
	for( b = lines2.begin(); b != lines2.end(); ++b ){
		//outp.push_back( std::make_pair( *b, "" ) );
		//continue;

		if( !hf_strcasecmp( b->c_str(), "[", 1 ) ){	//], //if INI section name, fe. "[OPTION]"
			strKey = hf_trim_stdstring( b->c_str(), "\r\n\t\x20" );
			outp.push_back( std::make_pair( strKey, "" ) );
		}else{
			const char* sz2 = b->c_str();
			const char* sz3 = hf_strchr( sz2, '=' );
			if( sz3 ){
				int pos = sz3 - sz2;
				strKey.assign( b->c_str(), pos );
				strVal.assign( b->c_str() + pos + 1 );
				strKey = hf_trim_stdstring( strKey.c_str(), "\r\n\t\x20" );
				strVal = hf_trim_stdstring( strVal.c_str(), "\r\n\t\x20" );
				size_t num = strVal.size();
				if( num >= 2 ){
					int i;
					for( i=0; i<2; i++ ){
						char quot = ( !i ? '\"' : '\'');
						if( strVal[0] == quot ){
							if( strVal[ num-1 ] == quot ){
								strVal.assign( std::string(strVal).c_str(), 1, num-2 );
								break;
							}
						}
					}
				}
				outp.push_back( std::make_pair( strKey, strVal ) );
			}
		}
	}//*/
	return 1;
}


class HfIStdCopyFile {
public:
	// renamed, old name: siscfProgressCalback();
	virtual bool siscf_ProgressCalback( uint64_t pos, uint64_t full_size ) = 0;
};

enum{
	EHCFFOF_NoFlags = 0,
	/// Auto opens file in constructor and auto closes in destructor.
	/// read-only mode.
	EHCFFOF_ConstrOpenRO = 0x1,
	/// Same as above but read-write mode.
	EHCFFOF_ConstrOpenRW = 0x2,
};

/// Object that can simulate opening of same file multiple times by internal
/// reference count.
/// File can be opened from either file name or FILE* descriptor.
class HfCFileOpener {
public:
	HfCFileOpener( const char* szFname, size_t flags = EHCFFOF_NoFlags );
	HfCFileOpener( FILE* fp );
	~HfCFileOpener();
	bool openIfNeeded( const char* stream_mode );
	void closeIfNeeded();
	FILE* getStream()const;
	bool seek( const uint64_t& pos )const;
	enum { ERBF_NOFLAGS = 0, ERBF_NOSIZECONSTRAINT = 0x1, };
	std::vector<uint8_t> readBytes( const uint64_t& pos, size_t sizetoread, size_t flags=ERBF_NOFLAGS )const;
	bool readBytes2( const uint64_t& pos, size_t sizetoread, std::vector<uint8_t>& out, size_t flags=ERBF_NOFLAGS, size_t* readd2 = 0 )const;
	const char* getFileName()const;
	void setFileName( const char* in ) {Fname = in;}
	uint64_t getFileSize()const;
private:
	enum ET{ ET_FILENAMEMBCS, ET_STREAM, };
	ET          Type;
	std::string Fname;
	FILE*       Stream;
	int         CntOpen;
	size_t      Flags2;
};

/// Flags for hf_StdoutBeginRedirect().
enum {
	ESBRF_NoFlags = 0,
	ESBRF_AutoSuppress = 0x1,
};

} // end namespace hef
#endif //_HEF_FILE_H_
