
#ifndef _P7U_DC_H_
#define _P7U_DC_H_

#include <string>
#if defined(WIN32) || defined(WIN64)
#	pragma warning (push)
#	pragma warning (disable:4163 4514)
#	include <windows.h>
#	pragma warning (pop)
#else
#	include <unistd.h>
#	include "win2nix_binds.h"
#endif
#include "wcxhead.h" // this header must have "win2nix_binds.h" included beforehand (on non-Windows).
#include "p7u_cmn.h"

// GCC type size compile time assertion.
// size of WCHAR must be 2 for DCMD WCX interface and 4 for P7ZIP library.
typedef char wcxi_assertion_on_type_size_PpoQu3sTj [ ( sizeof(WCHAR) == 2 ) *2 - 1 ];

namespace hef{}
using namespace hef;

class CDCProcRelay;

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
	void __stdcall ConfigurePacker( void* hwndParent, void* xDllInstance );
	// WCHAR related functions:
	HANDLE __stdcall OpenArchiveW( tOpenArchiveDataW* ArchiveData );
	int __stdcall ProcessFileW( HANDLE hArcData, int Operation, WCHAR* DestPath, WCHAR* DestName );
	int __stdcall ReadHeaderExW( HANDLE hArcData, tHeaderDataExW* HeaderData );
	BOOL __stdcall CanYouHandleThisFileW( WCHAR* FileName );
}

/// Class that binds common GUI actions into DCMD calls.
/// F.e. ask password, set unpacking progress percentage, etc.
class CDCProcRelay : public IProcRelay {
public:
	static const char* szInpBoxSmbl;
	// uFlags: 0x10 = error icon.
	static const char* szMsgBoxSmbl;
	//
	CDCProcRelay();
	virtual bool iprSetFilePercentage( const char* szFilename, float fPercentage );
	virtual bool iprSetTotalPercentage( const char* szFilename, float fPercentage2 );
	virtual bool iprTryGetOrAskArcFNPassword( const char* szArcFnm, std::string* outp );
	virtual bool iprRemoveArcFNPassword( const char* szArcFnm );
	virtual bool iprPromptBox( SPrompt& inp );
	virtual bool iprMessageBox( SMsgBox& inp );
private:
	bool getArcFNPassword( const char* szArcFnm, std::string* outp );
	void storeArcFNPassword( const char* szArcFnm, const char* szPass );
public:
	tProcessDataProc fnProcessDataProc;
private:
	std::vector<std::pair<std::string,std::string> > pwds2;
	int nMaxPasswords;
	DcPromptBoxProc_t fncDcPromptBoxProc;
	//DcMessageBoxProc_t fncDcMessageBoxProc;
};

//void wcxi_SafeInitDcInterface();

#endif //_P7U_DC_H_
