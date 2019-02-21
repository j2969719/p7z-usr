
#if defined(WIN32) || defined(WIN64)
#	pragma warning (push)
#	pragma warning (disable:4163 4514)
#	include <windows.h>
#	pragma warning (pop)
#else
#	include <unistd.h>
#	include "win2nix_binds.h"
#endif
#include "wcxhead.h" // this header must have "win2nix_binds.h" included beforehand (on unix).

namespace hef{}
using namespace hef;

class CProcRelay;

extern "C"{
	int __stdcall CloseArchive( HANDLE hArcData );
	HANDLE __stdcall OpenArchive( tOpenArchiveData* ArchiveData );
	int __stdcall ProcessFile( HANDLE hArcData, int Operation, char* DestPath, char* DestName );
	int __stdcall ReadHeader( HANDLE hArcData, tHeaderData* HeaderData );
	int __stdcall ReadHeaderEx( HANDLE hArcData, tHeaderDataEx* HeaderData );
	void __stdcall SetChangeVolProc( HANDLE, tChangeVolProc pChangeVolProc );
	void __stdcall SetProcessDataProc( HANDLE, tProcessDataProc pProcessDataProc );
	BOOL __stdcall CanYouHandleThisFile( char* FileName );
	int __stdcall GetPackerCaps();
	// WCHAR related functions:
	HANDLE __stdcall OpenArchiveW( tOpenArchiveDataW* ArchiveData );
	int __stdcall ProcessFileW( HANDLE hArcData, int Operation, WCHAR* DestPath, WCHAR* DestName );
	int __stdcall ReadHeaderExW( HANDLE hArcData, tHeaderDataExW* HeaderData );
	BOOL __stdcall CanYouHandleThisFileW( WCHAR* FileName );
}
