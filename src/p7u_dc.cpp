
#include "p7u_dc.h"
#include <string.h>
#include <dlfcn.h>
#include "hef_utf8_str.h"
#include "hef_assert.h"
#include "hef_data.h"
#include "hef_str_args.h"
#include "p7u_cmn.h"
#include "p7u_cls.h"


DcMessageBoxProc_t fncDcMessageBoxProc;
DcPromptBoxProc_t fncDcPromptBoxProc;

// Symbol name for DCMD input dialog box function.
// Got it by examining 'doublecmd' executable with a command:
// "$> nm -D ./doublecmd"
//const char* CDCProcRelay::szInpBoxSmbl = "FDIALOGBOX_INPUTBOX$PCHAR$PCHAR$LONGBOOL$PCHAR$LONGINT$$LONGBOOL";
//const char* CDCProcRelay::szMsgBoxSmbl = "FDIALOGBOX_MESSAGEBOX$PCHAR$PCHAR$LONGINT$$LONGINT";


CDCProcRelay::CDCProcRelay()
	: fnProcessDataProc(0), nMaxPasswords(3)
{
	hf_assert( cPlugin );
	if( !fncDcPromptBoxProc ){
		//fncDcPromptBoxProc = (DcPromptBoxProc_t)dlsym( RTLD_DEFAULT, cPlugin->getDcPromptBoxSymbol() );
		fncDcPromptBoxProc = (DcPromptBoxProc_t)dlsym( RTLD_DEFAULT, "FDIALOGBOX_INPUTBOX$PCHAR$PCHAR$LONGBOOL$PCHAR$LONGINT$$LONGBOOL" );
	}
	//fncDcMessageBoxProc = (DcMessageBoxProc_t)dlsym( RTLD_DEFAULT, cPlugin->getDcMsgBoxSymbol() );
}

HANDLE __stdcall OpenArchive( tOpenArchiveData* ArchiveData )
{
	wcxi_DebugString("OpenArchive");
	//IsoImage* image = GetImage( ArchiveData->ArcName );
	//return image;
	return 0;
}

HANDLE __stdcall OpenArchiveW( tOpenArchiveDataW* arcd )
{
	if(!cProcRelayIntrf)
		cProcRelayIntrf = new CDCProcRelay;

	std::string strArcn;
	hf_Utf8Encode16( arcd->ArcName, -1, &strArcn, 0 );
	wcxi_DebugString( HfArgs("OpenArchiveW [%1]").arg(strArcn.c_str()).c_str() );

	wcxi_OpenArchiveData sdd;
	sdd.szArcName    = strArcn.c_str();
	sdd.bListingOnly = ( arcd->OpenMode == PK_OM_LIST ? 1 : 0 );
	sdd.eOpenResult  = 0;
	void* hptr = cPlugin->openArchive(sdd);
	arcd->OpenResult = sdd.eOpenResult;
	return hptr;
}
#if defined(WIN32) || defined(WIN64)    // [QrvVerHDPXmI]
BOOL APIENTRY DllMain( HANDLE hinstDLL, DWORD fdwReason, LPVOID )
{
	if( !hinstDLL )
		return 0;
	switch( fdwReason ){
	case DLL_PROCESS_ATTACH:
		wcxi_DebugString("Winapi process attach");
		fnInit();
		break;
	case DLL_PROCESS_DETACH:
		wcxi_DebugString("Winapi process detach");
		fnDeinit();
		break;
	}
	return 1;
}
#endif    // [QrvVerHDPXmI]

int __stdcall ReadHeader( HANDLE hArcData, tHeaderData* HeaderData )
{
	wcxi_DebugString("ReadHeader");
	;
	return E_NOT_SUPPORTED;
}
int __stdcall ReadHeaderEx( HANDLE hArcData, tHeaderDataEx* HeaderData )
{
	wcxi_DebugString("ReadHeaderEx");
	//cPlugin->readHeaderEx()
	return E_NOT_SUPPORTED;
}
int __stdcall ReadHeaderExW( HANDLE hArcData, tHeaderDataExW* swhd )
{
	//wcxi_DebugString("ReadHeaderExW");
	memset( swhd, 0, sizeof(*swhd) );
	wcxi_HeaderDataEx shd;
	if( cPlugin->readHeaderEx( hArcData, shd ) ){
		//wcxi_DebugString(HfArgs( "cPlugin->readHeaderEx() ok %1 [%2]")
		//		.arg( shd.nFileAttr & WCXI_FADir ? "d":"f" )
		//		.arg( shd.strFileName.c_str() ).c_str() );
		std::basic_string<WCHAR> strFn;
		hf_Utf8DecodeToAny( shd.strFileName.c_str(), -1, strFn, 0 );

		// WCXI_FADir <=> WIN32 FA_DIRECTORY.
		swhd->FileAttr |= ( shd.nFileAttr & WCXI_FADir ? WCXI_FADir : 0 );

		int num = ( sizeof(swhd->FileName) / sizeof(swhd->FileName[0]) ) - 1;
		num = std::min( (int)strFn.size(), num );
		hf_strncpy( swhd->FileName, strFn.c_str(), num );
		swhd->FileName[num] = 0;
		wcxi_ConvU64To2xU32( shd.uUnpSize, &swhd->UnpSize, &swhd->UnpSizeHigh );
		wcxi_ConvU64To2xU32( shd.uPackSize, &swhd->PackSize, &swhd->PackSizeHigh );
		swhd->FileTime = shd.nFileTime;
		return 0; // 0=ok
	}else{
		if( shd.eError == WCXI_ENoMoreFiles ){
			wcxi_DebugString("ReadHeaderExW done, WCXI_ENoMoreFiles.");
			return WCXI_ENoMoreFiles; // E_END_ARCHIVE;
		}
		return shd.eError; // return E_BAD_DATA;
	}
}
int __stdcall ProcessFile( HANDLE hArcData, int eOperation, char* szDestPath, char* szDestName )
{
	if( eOperation == PK_EXTRACT ){
		wcxi_SOpenedArc* arcd2 = reinterpret_cast<wcxi_SOpenedArc*>(hArcData);
		if( arcd2->isMailformed() )
			return E_BAD_DATA;
		SFi sfi;
		sfi.strSrcFnmi = arcd2->strCurFile;
		sfi.strDstFnmi = HfArgs("%1%2%3")
				.arg( szDestPath ? szDestPath : "" )
				.arg( ( szDestPath && *szDestPath && szDestName && *szDestName ) ? "/" : "" )
				.arg( szDestName ? szDestName : "" ).c_str();
		sfi.uCFItem      = arcd2->uCurItem;
		sfi.uFsizei      = 0;
		arcd2->lsExtr2.push_back( sfi );
		return 0; //0=ok
	}else if( eOperation == PK_SKIP ){
		//wcxi_DebugString(HfArgs("ProcessFile(), PK_SKIP").arg("").c_str() );
		return 0;
	} // else PK_TEST
	return E_NOT_SUPPORTED;
}
int __stdcall ProcessFileW( HANDLE hArcData, int eOperation, WCHAR* szDestPath, WCHAR* szDestName )
{
	//wcxi_DebugString("ProcessFileW");
	std::string str2, str3;
	if( eOperation == PK_EXTRACT ){
		static const WCHAR szEmptyW[] = {0,}; //static WCHAR szNullW[] = { 'n','u','l','l',0, };
		hf_Utf8EncodeFromAny( ( szDestPath ? szDestPath : szEmptyW ), -1, &str2, 0 );
		hf_Utf8EncodeFromAny( ( szDestName ? szDestName : szEmptyW ), -1, &str3, 0 );
	}
	return ProcessFile( hArcData, eOperation, (char*)str2.c_str(), (char*)str3.c_str() );
}
int __stdcall CloseArchive( HANDLE hArcData )
{
	wcxi_DebugString("CloseArchive()");
	int err = 0;
	bool rs2 = cPlugin->closeArchive( hArcData, &err );

	int retv2 = ( rs2 ? err : ( err ? err : E_BAD_DATA ) );
	wcxi_DebugString( HfArgs("CloseArchive() done (%1)")
		.arg( retv2 )
		.c_str() );
	return retv2;
}


void __stdcall SetChangeVolProc( HANDLE, tChangeVolProc pChangeVolProc )
{
	if(!cProcRelayIntrf)
		cProcRelayIntrf = new CDCProcRelay;
	//fnChangeVolProc = pChangeVolProc;
}
void __stdcall SetProcessDataProc( HANDLE, tProcessDataProc pProcessDataProc )
{
	//wcxi_DebugString("SetProcessDataProc()");
	if(!cProcRelayIntrf)
		cProcRelayIntrf = new CDCProcRelay;

	//cProcRelayIntrf2_->fnProcessDataProc = pProcessDataProc;
	((CDCProcRelay*)cProcRelayIntrf)->fnProcessDataProc = pProcessDataProc;//cProcRelayIntrf2
}

BOOL __stdcall CanYouHandleThisFile( char* szFileName )
{
	if(!cProcRelayIntrf)
		cProcRelayIntrf = new CDCProcRelay;
	bool rs2 = cPlugin->canYouHandleThisFile( szFileName, 0, 0, 0 );
	wcxi_DebugString( HfArgs("CanYouHandleThisFile() done, %1").arg((int)rs2).c_str());
	return rs2;
}
BOOL __stdcall CanYouHandleThisFileW( WCHAR* szFileName )
{
	std::string str;
	hf_Utf8EncodeFromAny( szFileName, -1, &str, 0 );
	return CanYouHandleThisFile( (char*)str.c_str() );
}
int __stdcall GetPackerCaps()
{

	return PK_CAPS_BY_CONTENT | PK_CAPS_SEARCHTEXT | PK_CAPS_OPTIONS;
}
void __stdcall ConfigurePacker( void* hwndParent, void* xDllInstance )
{
	if(!cProcRelayIntrf)
		cProcRelayIntrf = new CDCProcRelay;
	hf_assert(cPlugin);
	std::string msg2 = HfArgs(
				"P7Z Usr Plugin\n"
				"\n"
				"P7ZIP headers version: [%1]\n"
				"(Note: this is compile time value, shared library version may differ)\n"
				"\n"
				"Configuration file: [%2]\n"
				"\n"
				"Shared lib file: [%3]\n"
				"\n")
			.arg( cPlugin->getP7ZIPHeadersVersion().c_str() )
			.arg( cPlugin->getINIConfigFileName().c_str() )
			.arg( Str7zSoFile ? Str7zSoFile->c_str() : "" )
			.c_str();
	IProcRelay::SMsgBox smb;
	smb.capt = szP7zWcxFile;
	smb.msg = msg2.c_str();
	cProcRelayIntrf->iprMessageBox( smb );//cProcRelayIntrf2
}
bool CDCProcRelay::iprPromptBox( SPrompt& inp )
{
	if( fncDcPromptBoxProc ){
		char bfrVal[128] = "";
		hf_strncpy( bfrVal, inp.strIoVal.c_str(), sizeof(bfrVal)-1 );
		bfrVal[sizeof(bfrVal)-1] = 0;
		bool rs2;
		inp.bOkAccepted = 0;
		rs2 = !!fncDcPromptBoxProc( inp.capt, inp.msg, 0, bfrVal, sizeof(bfrVal) );
		if(rs2){
			inp.strIoVal = bfrVal;
			inp.bOkAccepted = 1;
		}
		return 1;
	}
	return 0;
}
bool CDCProcRelay::iprMessageBox( SMsgBox& inp )
{
	if( fncDcMessageBoxProc ){
		inp.bOkAccepted = !!fncDcMessageBoxProc( inp.msg, inp.capt, inp.flags3, 0 );
		return 1;
	}
	return 0;
}
bool CDCProcRelay::iprTryGetOrAskArcFNPassword( const char* szArcFnm, std::string* outp )
{
	std::string pw2;
	if( !getArcFNPassword( szArcFnm, &pw2 ) ){
		std::string capt2 = HfArgs("%1 - %2")
				.arg( szP7zWcxFile )
				.arg( hf_basename2(szArcFnm).c_str() ).c_str();
		IProcRelay::SPrompt spx;
		spx.capt = capt2.c_str();
		spx.msg  = "Enter password:";
		if( iprPromptBox( spx ) ){
			if( spx.bOkAccepted ){
				*outp = spx.strIoVal;
				storeArcFNPassword( szArcFnm, spx.strIoVal.c_str() );
				return 1;
			}
		}
	}else{
		*outp = pw2;
		return 1;
	}
	return 0;
}

bool CDCProcRelay::iprRemoveArcFNPassword( const char* szArcFnm )
{
	std::vector<std::pair<std::string,std::string> >::iterator a;
	a = std::find_if( pwds2.begin(), pwds2.end(), HfPredTTPair<std::string,std::string>(szArcFnm) );
	if( a != pwds2.end() ){
		pwds2.erase( a );
		return 1;
	}
	return 0;
}

void CDCProcRelay::storeArcFNPassword( const char* szArcFnm, const char* szPass )
{
	std::vector<std::pair<std::string,std::string> >::const_iterator a;
	a = std::find_if( pwds2.begin(), pwds2.end(), HfPredTTPair<std::string,std::string>(szArcFnm) );
	if( a == pwds2.end() ){
		pwds2.push_back( std::make_pair( szArcFnm, szPass ) );
		if( pwds2.size() > (size_t)nMaxPasswords ){
			pwds2.erase( pwds2.begin() );
		}
	}
}
bool CDCProcRelay::getArcFNPassword( const char* szArcFnm, std::string* outp )
{
	std::vector<std::pair<std::string,std::string> >::const_iterator a;
	a = std::find_if( pwds2.begin(), pwds2.end(), HfPredTTPair<std::string,std::string>(szArcFnm) );
	if( a != pwds2.end() ){
		*outp = a->second;
		return 1;
	}
	return 0;
}
bool CDCProcRelay::iprSetFilePercentage( const char* szFilename, float fPercentage )
{
	if(fnProcessDataProc){
		// // If the user has clicked on Cancel, fnProcessDataProc returns zero.
		// // ... values -1..-100 to directly set first percent bar.
		// // ... values -1000..-1100 for second percent bar (-1000=0%).
		int nSizParam = -( (int)( (fPercentage * 100.f) ) + 1000 );
		return !!fnProcessDataProc( (char*)szFilename, nSizParam );
	}
	return 1;
}
bool CDCProcRelay::iprSetTotalPercentage( const char* szFilename, float fPercentage2 )
{
	if(fnProcessDataProc){
		int nSizParam2 = -( (int)( (fPercentage2 * 99.f) ) + 1 );
		return !!fnProcessDataProc( (char*)szFilename, nSizParam2 );
	}
	return 1;
}

/**
	Function called by DCMD.
	Using hardcoded offsets. Ideally it would be to use
	'tExtensionStartupInfo' type from 'extension.h' from DCMD SDK.
	Not using SDK headers atm. (as of v0.5).
*/
void ExtensionInitialize( void* ptrExtensionStartupInfo )
{
	wcxi_DebugString( HfArgs("ExtensionInitialize(), pid:%1").arg( (int)getpid() ).c_str());
	// EXT_MAX_PATH = 16384
	const int offs2 = sizeof(uint32_t) + ((sizeof(char) * 16384) * 2);
	const int offs3 = offs2 + sizeof(void*);
	//int offs4 = sizeof(uint32_t);
	//const char* szDir = ((char*)ptrExtensionStartupInfo) + offs4;
	//wcxi_DebugString(szDir);

	void** ppPromptBoxProc = (void**) (((char*)ptrExtensionStartupInfo) + offs2);
	fncDcPromptBoxProc = (DcPromptBoxProc_t) *ppPromptBoxProc;

	void** ppMsgBoxProc = (void**) (((char*)ptrExtensionStartupInfo) + offs3);
	fncDcMessageBoxProc = (DcMessageBoxProc_t) *ppMsgBoxProc;
}
