
#include "hef_file.h"
#include <stdio.h>
#ifdef _MSC_VER
#	include <io.h>
#endif
#ifdef __GNUC__
//#	define _FILE_OFFSET_BITS 64		//required by ftello(), makes off_t 64-bit size.
#	include <unistd.h>
#	include <sys/stat.h>
#	include <sys/types.h>
#	include <pwd.h>      //getpwuid()
#	include <dlfcn.h>    //dladdr()
#endif
#include <assert.h>
#include <cctype>
#include <cerrno> // ENOENT
#include <stdlib.h>
#include <cstring>
#include "hef_data.h"
#include "hef_assert.h"
namespace hef{
;
/// GCC compile time assertion that ensures  off_t is at least 64-bit long.
/// reference: "http://stackoverflow.com/questions/19401887/how-to-check-the-size-of-a-structure-at-compile-time"
typedef char hf_assertion_on_type_size_01[ ( sizeof(off_t) >= 8 ) *2 - 1 ];

/// \cond DOXYGEN_SKIP

;
#ifdef HEF_DEBUG //[2DxZZ7oO8rYU]

void file_functs_general_tester()
{
	printf("\n""file_functs_general_tester()\n");
	{
		printf("\n""TEST_01 64-bit functions seek/write sub 32-bit only\n");
		std::string fname = "./testfile_no1.bin";
		printf("Opening: [%s]\n", fname.c_str() );
		const char* dataarr1[] = {"abc","def",};
		size_t i, pos = 0;
		for( i=0; i<sizeof(dataarr1)/sizeof(dataarr1[0]); i++ ){
			const char* data = dataarr1[i];
			size_t len = hf_strlen( data );
			FILE* fp = 0;
			if(!i){
				fp = fopen( fname.c_str(), "wb+" );
			}else{
				fp = fopen( fname.c_str(), "rb+" );
			}
			assert(fp);

			hf_Seek64( fp, pos );
			fwrite( data, len, 1, fp );

			pos += len;
			fclose(fp);
		}
	}
	{
		printf("\n""TEST_02 hf_GetFileStrContents64(), sub 32-bit reads.\n");
		printf("result 1: %s\n", hf_GetFileStrContents64( "./testfile_fcont_no1.txt" ).c_str() );
		printf("result 2: %s\n", hf_GetFileStrContents64(
			"./testfile_fcont_no2.txt", 66, 9 ).c_str() );
		printf("result 3: %s\n", hf_GetFileStrContents64(
			"./testfile_fcont_no2.txt", 88, 11 ).c_str() );
	}
	{
		printf("\n""TEST_03 64-bit functions seek/write on existing 4GB+ file.\n");
		std::string testfn = "./testfile_no2_fpath.txt";
		if( !hf_FileExists(testfn.c_str()) ){
			printf("WARNING: skipping test, test-file with file-path\n"
			       "         to 4GB+ file not found.\n"
				   "         file: [%s]\n", testfn.c_str() );
		}else{
			std::string strfn = hf_GetFileStrContents64( testfn.c_str() );
			strfn = hf_trim_stdstring<char>( strfn.c_str(), " \r\n\t" );
			uint64_t addr = 5790728208;//5000000000; 4E6F6E6500
			uint32_t sizer = 8;
			//
			//sscanf: fn, addr_bfr, size
			{
				char bfrFn[128] = "", bfrAddr[31] = "", bfrExpectedBytesHex[128] = "";
				unsigned int sizer2;
				sscanf( strfn.c_str(), "%s %s %u %s", bfrFn, bfrAddr, &sizer2,
					bfrExpectedBytesHex );
				sizer = sizer2;
				strfn = bfrFn;
				addr  = hf_AsciiToU64(bfrAddr);
				printf("Read details: addr: %s, size: %u, fn: %s\n",
					hf_StrPrintU64(addr).c_str(), (unsigned int)sizer, strfn.c_str() );
				printf("Expected Bytes: %s\n", bfrExpectedBytesHex );
			}
			FILE* fp4 = fopen( strfn.c_str(), "r" );
			if(fp4){
				uint64_t fsize = hf_GetFileSize64( fp4 );
				if( fsize < addr ){
					printf("WARNING: skipping test, test-file [%s] not large enough.\n",
						strfn.c_str() );
				}else{
					printf("reading from 4GB+ file, addr: %s...\n",
						hf_StrPrintU64(addr).c_str() );
					bool rs2 = hf_Seek64( fp4, addr );
					assert(rs2);
					std::vector<uint8_t> data = hf_ReadFileBytes64( fp4, addr, sizer );
					printf("Readd %u bytes from file.\n", (unsigned int)data.size() );
				}
			}else{
				printf("WARNING: skipping test, test-file [%s] doesnt exists.\n",
					strfn.c_str() );
			}
		}
	}
	{
	/*	printf("\n""TEST_04 hf_StdCopyFile().\n");
		const char* src = "./testfile_no3_copysrc.bin";
		const char* dst = "./testfile_no3_copydst.bin";
		class HfCCpyNotify : public HfIStdCopyFile {
		public:
			virtual bool siscf_ProgressCalback( uint64_t pos, uint64_t full_size ){
				printf("pos: %u/%u\n", (unsigned int)pos, (unsigned int)full_size );
				return 1;
			}
		}cNotify;
		if( !hf_FileExists( src ) ){
			printf("WARNING: skipping test, test-file [%s] doesnt exists.\n", src );
		}else{
			if( hf_FileExists( dst ) )
				remove( dst );
			printf("copying [%s] to [%s]\n", src, dst );
			bool rs1 = hf_StdCopyFile( src, dst, &cNotify );
			printf("result: %d\n", (int)rs1 );
		}
		//*/
	}
}

#endif //HEF_DEBUG //[2DxZZ7oO8rYU]

/// \endcond //DOXYGEN_SKIP
;


/// Get file size 64-bit compatible given file name.
/// See also: hf_GetFileSize64().
uint64_t hf_GetFileSize64Fn( const char* fn )
{
	FILE* hf2 = fopen( fn, "r");
	if( hf2 ){
		uint64_t fsize = hf_GetFileSize64( hf2 );
		fclose(hf2);
		return fsize;
	}
	return 0;
}

/// Set file position.
/// \sa hf_assertion_on_type_size_01
bool hf_Seek64( FILE* hf, uint64_t position2 )
{
#	ifdef WIN32
		fseek( hf, 0, SEEK_SET );
		int fd = fileno( hf );
		if( fd == -1 )
			return 0;
		int64_t retv = _lseeki64( fd, (int64_t)position2, SEEK_SET );
		if( -1 == retv )
			return 0;
		if( position2 < 4000000000 ){
			uint32_t pos3 = ftell( hf );
			//assert( pos3 == position2 );
			if( pos3 != position2 )
				return 0;
		}
		return 1;
#	elif defined(__GNUC__)
		//
		// IMPORTANT:
		// Must use  fseeko() and ftello() as pairs and not other, fe. lseek64()
		// or fseek().
		// otherwise there will be inapropriate stream pos movement with fread()
		// fwrite().
		//
		hf_assert( sizeof(off_t) >= 8 );
		off_t pos3 = position2;
		off_t rs2 = fseeko( hf, pos3, SEEK_SET ); // --> ftello()
		return !rs2;
#	else
		//hf_assert(!"Unknown platform");
		#error "Unknown platform."
		return 0;
#	endif
}

/*uint64_t hf_Tell64( FILE* fph )
{
	// On many architectures both off_t and long are 32-bit types, but  compi‐
	// lation with
	// 	  #define _FILE_OFFSET_BITS 64
	// will turn off_t into a 64-bit type.
	;
	//hf_assert( sizeof(off_t) >= 8 );
	//off_t rs2 = fseeko( hf, pos3, SEEK_SET ); // --> ftello()
	//lseek64()
	;
	;
	return 0;
}//*/

uint64_t hf_GetFilePos64( FILE* hf )
{
#	ifdef WIN32
		hf_assert(!"Not implelmented.");
#	elif defined(__GNUC__)
		/*
		hf_assert( sizeof(off64_t) > 4 );
		int fds = fileno( hf );
		if( fds == -1 )
			return 0;
		off64_t pos2 = lseek64( fds, 0, SEEK_CUR );
		uint64_t retv = (uint64_t) pos2;//*/

		//#define _FILE_OFFSET_BITS 64
		//int valsiz = sizeof(off_t);
		//valsiz=valsiz;
		hf_assert( sizeof(off_t) >= 8 );
		uint64_t retv = 0;
	//	retv = lseek64( fileno( hf ), 0, SEEK_CUR );
	//	retv = fseek( hf, 0, SEEK_CUR );
	//	retv = ftell( hf );
		retv = ftello( hf ); // --> fseeko()
		return retv;
#	else
		hf_assert(!"Not implelmented.");
		return 0;
#	endif
	return 0;
}

/// Get file size 64-bit compatible.
/// \sa hf_GetFileSize64Fn().
uint64_t hf_GetFileSize64( FILE* hf )
{
	//_CRTIMP __int64 __cdecl _lseeki64(int, __int64, int);
	//_CRTIMP __int64 __cdecl _telli64(int);
#	ifdef WIN32
		fseek( hf, 0, SEEK_SET );
		int fd = fileno( hf );
		if( fd == -1 )
			return 0;

		if( -1 == _lseeki64( fd, 0, SEEK_END ) )
			return 0;
		int64_t retvSize = _telli64( fd );
		return ( retvSize == -1 ? 0 : retvSize );
#	elif defined(__GNUC__)
		//#define _FILE_OFFSET_BITS 64
		hf_assert( sizeof(off_t) >= 8 );
		fseeko( hf, 0, SEEK_END );
		off_t retvPos = ftello( hf );
		return retvPos;
#	else
		hf_assert(0);
		return 0;
#	endif
}

bool hf_FileExists( const char* fname )
{
//	FILE* fp = fopen( fname, "r" );
//	if(fp){
//		fclose(fp);
//		return 1;
//	}
	// src: stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform
	if( access( fname, F_OK ) != -1 ){ // F_OK = 0.
		return 1;
	}
	return 0;
}
/// Returns true if directory exists.
/// \param bNotDir - optional, set to true if path exists but is not a direcory,
///                  fe. is a regular file instead.
bool hf_DirExists( const char* szPath, bool* bNotDir )
{
	// ref: http://stackoverflow.com/questions/9314586/c-faster-way-to-check-if-a-directory-exists
	// >> If you ensure a trailing / in the directory name, this works perfectly.
	if(bNotDir) *bNotDir = 0;
	if( 0 != access( szPath, F_OK ) ){ // F_OK = 0.
		if( ENOENT == errno ) {
			// does not exist
		}
		if( ENOTDIR == errno ){ // if not a directory
			if(bNotDir) *bNotDir = 1;
		}
		return 0;
	}
	return 1;
}

bool hf_IsDirectory( const char* szPath )
{
	bool bNotDir = 0, res;
	res = hf_DirExists( szPath, &bNotDir );
	if( bNotDir )
		return 0;
	if( res )
		return 1;
	return 0;
}

bool hf_Mkdir( const char* szDirname )
{
	if( !mkdir( szDirname, 0777 ) ){
		return 1;
	}
	return 0;
}
bool hf_Unlink( const char* szFn )
{
	return !unlink( szFn );
}
bool hf_RmDir( const char* szFn )
{
	return !rmdir( szFn );
}
bool hf_RenameFileOrDir( const char* szOldPath, const char* szNewPath )
{
	if( hf_FileExists(szNewPath) ){
		return 0;
	}
	return !rename( szOldPath, szNewPath );
}
/// Reads file contents as multibyte string.
/// Optional addr, max-read, etc. may be specified.
/// \sa GP_rw_file_contents
/// flags - f.e. EGFC64_NOFLAGS.
std::string
hf_GetFileStrContents64( const char* fn, uint64_t addr, int max_read,
							size_t flags, std::string* outp2 )
{
	FILE* fp3 = fopen( fn, "rb" );
	std::string out4;
	std::string* out3 = outp2 ? outp2 : &out4;
	if(fp3){
		do{
			if( max_read == -1 ){
				uint64_t fsize = hf_GetFileSize64( fp3 );
				if( addr >= fsize )
					break;
				uint64_t maxr = fsize - addr;
				//max_read = (int) std::min<uint64_t>( fsize, 1*1000*1000*1000 );
				max_read = (int)maxr;
				if( maxr != (uint64_t)max_read )
					break;
			}
			if( !hf_Seek64( fp3, addr ) )
				break;
			char* bfr = (char*)malloc( max_read );
			bool rs1 = !!fread( bfr, max_read, 1, fp3 );
			char chr;
			if(rs1){
				for( int i=0; i<max_read; i++ ){
					chr = bfr[i];
					if( isprint( chr ) || chr=='\r'||chr=='\n'||chr=='\t' ){
						out3->append( (char*) &bfr[i], 1 );
				//	}else if( !chr ){
				//		break;
					}else if( flags & EGFC64_STOPONNONPRINTS ){
						break;
					}else{
						out3->append( "." );
					}
				}
			}
			free(bfr);
		}while(0);
		fclose(fp3);
	}
	return *out3;
}
/// Read bytes.
/// \sa GP_rw_file_contents
std::vector<uint8_t>
hf_ReadFileBytes64( FILE* fp, uint64_t addr, size_t size_read, std::vector<uint8_t>* outp )
{
	std::vector<uint8_t> z3, *out2;
	out2 = outp ? outp : &z3;
	do{
		if( !hf_Seek64( fp, addr ) )
			break;
		out2->resize( size_read, 0 );
		if( !fread( &(*out2)[0], size_read, 1, fp ) )
			out2->clear();
	}while(0);
	return *out2;
}

/// Copies single file.
/// Overwrites target file if exists, fails if target file already exists and has
/// read-only flag set.
bool hf_StdCopyFile( const char* szInput, const char* szOut, HfIStdCopyFile* intrf )
{
	const size_t BUFSIZ2 = 65536;
	size_t len = 0;
	char buffer2[BUFSIZ2] = { '\0', };
	bool success2 = 0;
	FILE* inpf = fopen( szInput, "rb" );
	FILE* outf = fopen( szOut, "wb" );
	if( inpf && outf ){
		uint64_t end, pos, fsize = hf_GetFileSize64( inpf );
		size_t nread;
		for( pos = 0; pos < fsize; pos += BUFSIZ2 ){
			end   = pos + BUFSIZ2;
			end   = std::min<uint64_t>( end, fsize );
			nread = (size_t)( (uint64_t) end - pos );
			if( !hf_Seek64( inpf, pos ) )
				break;
			len = (size_t)fread( buffer2, nread, 1, inpf );
			if(!len)
				break;
			if( !hf_Seek64( outf, pos ) )
				break;
			len = (size_t)fwrite( buffer2, nread, 1, outf );
			if(!len)
				break;
			if(intrf){
				if( !intrf->siscf_ProgressCalback( end, fsize ) )
					break;
			}
		}
		success2 = pos >= fsize;
	}
	if(inpf)
		fclose(inpf);
	if(outf)
		fclose(outf);
	return success2;
}

/// Constructor opener from file name as string.
/// \param flags2 - fe. \ref EHCFFOF_ConstrOpenRO.
HfCFileOpener::HfCFileOpener( const char* szFname, size_t flags2 )
	: Type(ET_FILENAMEMBCS), Fname(szFname), Stream(0), CntOpen(0), Flags2(flags2)
{
	if( Flags2 & EHCFFOF_ConstrOpenRO ){
		openIfNeeded("rb");
	}else if( Flags2 & EHCFFOF_ConstrOpenRW ){
		openIfNeeded("wb");
	}
}
HfCFileOpener::HfCFileOpener( FILE* fp )
	: Type(ET_STREAM), Stream(fp), CntOpen(0), Flags2(0)
{
}
HfCFileOpener::~HfCFileOpener()
{
	if( Flags2 & (EHCFFOF_ConstrOpenRO|EHCFFOF_ConstrOpenRW) ){
		closeIfNeeded();
	}
}
/// Opens file if needed.
/// Automatically counts references.
/// 'stream_mode' should be set to valid value fopen() uses,
/// f.e. "wb" for read-write (binary) and "rb" for read only (binary).
bool HfCFileOpener::openIfNeeded( const char* stream_mode )
{
	if( Type == ET_STREAM ){
		return 1;
	}else if( Type == ET_FILENAMEMBCS ){
		if(!CntOpen){
			assert( !Stream );
			Stream = fopen( Fname.c_str(), stream_mode );
			if(!Stream)
				return 0;
			CntOpen++;
		}else
			CntOpen++;
	}
	return 1;
}
void HfCFileOpener::closeIfNeeded()
{
	if( Type == ET_STREAM ){
		return;
	}else if( Type == ET_FILENAMEMBCS ){
		assert( CntOpen );
		--CntOpen;
		if( !CntOpen ){
			assert( Stream );
			fclose( Stream );
			Stream = 0;
		}
	}
}
FILE* HfCFileOpener::getStream()const
{
	return Stream;
}
bool HfCFileOpener::seek( const uint64_t& pos )const
{
	assert(Stream);
	return hf_Seek64( Stream, (int64_t) pos );
}

/// Reads file bytes at speciofied position (uses seek()).
/// flags - fe. ERBF_NOFLAGS.
std::vector<uint8_t> HfCFileOpener::readBytes( const uint64_t& pos, size_t sizetoread, size_t flags )const
{
	std::vector<uint8_t> out;
	size_t readd3 = 0;
	if( !readBytes2( pos, sizetoread, out, flags, &readd3 ) )
		return std::vector<uint8_t>();
	return out;
}
bool HfCFileOpener::
readBytes2( const uint64_t& pos, size_t sizetoread, std::vector<uint8_t>& out, size_t flags, size_t* readd2 )const
{
	if(readd2)
		*readd2 = 0;
	if( seek( pos ) ){
		assert(Stream);
		size_t pos2 = out.size();
		out.resize( pos2 + sizetoread, 0 );
		int readd = fread( &out[pos2], 1, sizetoread, Stream );
		if(readd2)
			*readd2 = readd;
		if( !(flags & ERBF_NOSIZECONSTRAINT) && ( !readd || readd != (int)sizetoread ) ){	//if error_.
			return 0;
		}
	}
	return 1;
}
// file name only available if constructed from file name, not from stream (FILE*).
const char* HfCFileOpener::getFileName()const
{
	if( Type == ET_STREAM )
		return "<file-name-not-available>";
	return Fname.c_str();
}
uint64_t HfCFileOpener::getFileSize()const
{
	if( Stream )
		return hf_GetFileSize64( Stream );
	return 0;
}
/// \cond DOXYGEN_SKIP
void _hf_SwapIOB( FILE *A, FILE *B )
{
#	ifdef WIN32
		FILE temp2;
		// make a copy of IOB A (usually this is "stdout")
		memcpy(&temp2, A, sizeof(struct _iobuf));
		// copy IOB B to A's location, now any output
		// sent to A is redirected thru B's IOB.
		memcpy(A, B, sizeof(struct _iobuf));
		// copy A into B, the swap is complete
		memcpy(B, &temp2, sizeof(struct _iobuf));
#	else
		hf_assert(0);
#	endif
}
static FILE* hf_StdoutRedirFpGlob_ = 0;
/// \endcond //DOXYGEN_SKIP

/// Begins STDOUT redirect.
/// \sa hf_StdoutEndRedirect().
/// To supress STDOUT, on windows 'szFname' can be set to "./nul"
/// and on Linux to "/dev/nul".
/// flags - eg. ESBRF_NoFlags.
void hf_StdoutBeginRedirect( const char* szFname, size_t flags )
{
	if( flags & ESBRF_AutoSuppress ){
		#ifdef WIN32
			szFname = "./nul";
		#else
			szFname = "/dev/nul";
		#endif
	}
	#ifdef _MSC_VER
		// src: hxxp://support.microsoft.com/kb/58667
		hf_assert( !hf_StdoutRedirFpGlob_ );
		hf_StdoutRedirFpGlob_ = freopen( szFname, "w", stdout );
		hf_assert( hf_StdoutRedirFpGlob_ );
	#else
		hf_assert( !hf_StdoutRedirFpGlob_ );
		hf_StdoutRedirFpGlob_ = fopen( szFname, "w" );
		hf_assert( hf_StdoutRedirFpGlob_ );
		_hf_SwapIOB( stdout, hf_StdoutRedirFpGlob_ );
	#endif
}
/// Ends STDOUT redirection.
/// \sa hf_StdoutBeginRedirect()
void hf_StdoutEndRedirect()
{
	#ifdef _MSC_VER
		hf_assert( hf_StdoutRedirFpGlob_ );
		hf_StdoutRedirFpGlob_ = freopen( "CON", "w", stdout );
		hf_StdoutRedirFpGlob_ = 0;
	#else
		hf_assert( hf_StdoutRedirFpGlob_ );
		_hf_SwapIOB( stdout, hf_StdoutRedirFpGlob_ );
		fclose( hf_StdoutRedirFpGlob_ );
		hf_StdoutRedirFpGlob_ = 0;
	#endif
}
/// Reads disk file with DSV contents with ability to specify delimiter string
/// and trimmable characters.
/// DSV: Delimiter-separated values (eg. CSV is a subset of DSV).
/// http://en.wikipedia.org/wiki/Delimiter-separated_values
void hf_GetFileDSVContents( const char* fn, const char* delim, std::vector<std::string>& outDSV, const char* szTrimcharsLR )
{
	std::string strfc;
	hf_GetFileStrContents64( fn, 0, -1, 0x0, &strfc );
	//int tmp_1 = strfc.size();
	HfCStrPiece<char>(strfc.c_str())
			.explode4( delim, outDSV, szTrimcharsLR?szTrimcharsLR:0 );
	//int tmp_2 = outDSV.size();
	//tmp_2 = tmp_2;
}

/*
///		Simple INI file parse without sections.
///		Sections instead are returned as items with key set to section name and empty value.
///		Output is a list of string key-value pairs.
bool hf_ParseSimpleIniFile_1( const char* szIniFn, std::vector<std::pair<std::string,std::string> >& outp )
{
	std::string str, str2, strKey, strVal;
	hf_GetFileStrContents64( szIniFn, 0, -1, 0x0, &str );
	// convert to array of lines, split by LF ("\n").
	HfCStrPiece<char>::LsStrPiece::iterator a;
	HfCStrPiece<char>::LsStrPiece lines2;
	HfCStrPiece<char>( str.c_str() ).explode("\n", lines2 );
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
	//printf("Num lines: %d\n", (int)lines2.size() );
	HfCStrPiece<char>::LsStrPiece::const_iterator b;
	for( b = lines2.begin(); b != lines2.end(); ++b ){
		if( !hf_strcasecmp( b->c_str(), "[", 1 ) ){	//], //if INI section name, fe. "[OPTION]"
			// TODO: sections,
			//       this requires recursive output list, WIP.
			strKey = hf_trim_stdstring( b->c_str(), "\r\n\t\x20" );
			outp.push_back( std::make_pair( strKey, "" ) );
		}else{
			int pos = HfCStrPiece<char>( b->c_str() ).strpos("=");
			if( pos != -1 && pos ){
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
	}
	return 1;
}//*/




//template<typename T>
//bool hf_ParseSimpleIniFile( const char* szIniFn, T& outp )

/**
	Gets command line non-standard way.
	Deditated for use in shared libraries (SO) or DLLs,
	argc|argv isn't normally available there.

	\param uPid - optional, zero retrieves commandline of current process.

	\verbatim
		Linux: Shared lib or DLL.
		>> On Linux the pseudo-file /proc/self/cmdline holds the command line
		>> for the process. Each argument is terminated with a 0 byte, and the
		>> final argument is followed by an additional 0 byte.
		ref: http://stackoverflow.com/questions/160292/how-can-i-access-argc-and-argv-in-c-from-a-library-function

		WIN32 WINAPI: GetCommandLine()
		>> then use CommandLineToArgvW() to convert that
		>> pointer to argv[] format. There is only a wide (Unicode) version
		>> available, though.
	\endverbatim
*/
bool hf_GetCommandLineSOSpec( std::vector<std::string>& outp, uint32_t uPid )
{
#	ifdef WIN32
		hf_assert(!"No implemented.");
#	else
		char bfrFn[128] = "/proc/self/cmdline";
		if( uPid )
			sprintf( bfrFn, "/proc/%u/cmdline", uPid );
		FILE* fp2 = fopen( bfrFn, "rb" );
		if(fp2){
			rewind(fp2);
			char bfr[128], bfr2[2] = {0,0,};
			size_t num, cnt = 0;
			outp.push_back("");
			for(;;){
				num = fread( bfr, 1, sizeof(bfr), fp2 );
				for( size_t i=0; i<num; i++ ){
					if( bfr[i] ){
						bfr2[0] = (char) bfr[i];
						outp.back().append( bfr2 );
					}else{
						if( !outp.back().empty() )
							outp.push_back("");
					}
				}
				cnt += num;
				if( feof(fp2) || ferror(fp2) )
					break;
				if( cnt >= 65536 ) // sanity check.
					break;
			}
			if( !outp.empty() && outp.back().empty() ) // if last is empty, remove it.
				outp.resize( outp.size() - 1 );
			fclose(fp2);
			return !outp.empty();
		}
#	endif
	return 0;
}

/// Gets current working directory. Uses getcwd() from the 'libc'.
/// \sa GP_dirname_and_fn
std::string hf_getcwd()
{
	#ifdef __GNUC__ //{
		int i, num = 256; std::string str;
		for( i=0; i<4; i++, num *= 2 ){ // 4 -> num 4096
			char* bfr3 = (char*)malloc( num );
			if(!bfr3) break;
			if( !getcwd( bfr3, num ) ){
				if( errno == ERANGE ){ // if 'errno' is set to 'ERANGE'
					free(bfr3);
					continue;
				}
			}else
				str = bfr3;
			free(bfr3);
			break;
		}
		return str;
	#else
		// use GetCurrentDirectory() on Windows.
		#error "WIP."
		return "";
	#endif //}
}

/// Gets the home directory.
/// \sa GP_dirname_and_fn
std::string hf_getHomeDir()
{
	#ifdef __GNUC__ //{
		struct passwd* ptr;
		const char* homedir = getenv("HOME");
		if( homedir && *homedir )
			return homedir;
		if( (ptr = getpwuid( getuid() )) ){
			homedir = ptr->pw_dir;
			if( homedir && *homedir )
				return homedir;
		}
		return "";
	#else
		//#error "Unsupported platform."
		hf_assert(!"Unsupported platform.");
	#endif //}
	return "";
}
/// Removes lib path from LD_PRELOAD env var given pointer to one of it's symbols.
/// Using pointer symbol, after dyn lib file name is retrirved,
/// env var is reassigned to new patched value.
bool hf_ClearLibFromLdPreloadEnv( void* pAnySymbolInDynLib )
{
	#ifdef __GNUC__ //{
		void* pSymbol = (pAnySymbolInDynLib ? pAnySymbolInDynLib : (void*)hf_ClearLibFromLdPreloadEnv);
		const char* szLdp = getenv("LD_PRELOAD");
		if( szLdp ){
			Dl_info dii; std::string strLdp;
			memset( &dii, 0, sizeof(dii) );
			dladdr( pSymbol, &dii );
			int num = 0;
			strLdp = hf_strreplace( dii.dli_fname, "", szLdp, -1, &num );
			if( num ){
				strLdp = "LD_PRELOAD=" + strLdp;
				putenv( (char*)strLdp.c_str() );
				//if( (szLdp = getenv("LD_PRELOAD")) ){
				//	if( !strstr(szLdp, dii.dli_fname ) )
				//		return 1;
				//}
				return 1;
			}
		}
	#else
		hf_assert(!"Unsupported platform.");
	#endif //}
	return 0;
}

} // end namespace hef
