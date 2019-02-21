
#include "hef_filesystem.h"

#ifdef WIN32
#	include <windows.h>
#endif
#include <stdio.h>
#include "hef_assert.h"

namespace hef{

HfDirContents::HfDirContents( const char* szDir, size_t flags )
	: Dir(szDir), Flags(flags), WfdHandle(0)
{
	Dir = hf_rtrim_stdstring( hf_rtrim_stdstring( Dir.c_str(), "*" ).c_str(), "/\\" );
}
HfDirContents::~HfDirContents()
{
	cleanup();
}
bool HfDirContents::getNextFile( SFile& out )
{
	hf_assert(0);
#	ifdef WIN32
		out = SFile();
		WIN32_FIND_DATA wf;
		memset( &wf, 0, sizeof(WIN32_FIND_DATA) );
		bool bOk = 0;
		if( !WfdHandle ){
			WfdHandle = FindFirstFile( std::string(Dir + "/*").c_str(), &wf );
			if( WfdHandle && WfdHandle != INVALID_HANDLE_VALUE ){
				bOk = 1;
			}else
				WfdHandle = 0;
		}else{
			bOk = !!FindNextFile( WfdHandle, &wf );
		}

		if( bOk ){
			out.fname  = wf.cFileName;
			out.bDir   = !!(wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			while( bOk && !isFnameAcceptable( out.fname ) ){
				bOk = getNextFile( out );
			}
		}
		if( bOk )
			return 1;
		cleanup();
#	else
		hf_assert(!"Not implemented.");
#	endif //WIN32
	return 0;
}
void HfDirContents::cleanup()
{
#	ifdef WIN32
	if( WfdHandle ){
		FindClose( WfdHandle );
		WfdHandle = 0;
	}
#	else
		hf_assert(!"Not implemented.");
#	endif //WIN32
}
bool HfDirContents::isFnameAcceptable( const std::string& fname2 )
{
	if( !(Flags & EHFDCF_IncludeDotedOnes) ){
		return fname2 != "." && fname2 != "..";
	}
	return 1;
}

} // end namespace hef
