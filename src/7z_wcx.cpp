
#include "7z_wcx.h"
#include <string.h>
#include "hef_utf8_str.h"
#include "hef_assert.h"
#include "hef_data.h"
#include "hef_str_args.h"
#include "7z_wcx_cmn.h"
#include "7z_wcx2.h"

//static tChangeVolProc   fnChangeVolProc = 0;
//static tProcessDataProc fnProcessDataProc_ = 0;

static CProcRelay* cProcRelayIntrf2 = 0; //CProcRelay* cProcRelayIntrf;

HANDLE __stdcall OpenArchive( tOpenArchiveData* ArchiveData )
{
	wcxi_DebugString("OpenArchive");
	//IsoImage* image = GetImage( ArchiveData->ArcName );
	//return image;
	return 0;
}

HANDLE __stdcall OpenArchiveW( tOpenArchiveDataW* arcd )
{
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
	;
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
		return 0; // 0=ok
	}else{
		//wcxi_DebugString("ReadHeaderExW done (2).");
		if( shd.eError == WCXI_ENoMoreFiles )
			return E_END_ARCHIVE;
		return shd.eError; // return E_BAD_DATA;
	}
}
int __stdcall ProcessFile( HANDLE hArcData, int eOperation, char* szDestPath, char* szDestName )
{
	if( eOperation == PK_EXTRACT ){
		wcxi_SProcFile spf;
		spf.arcd2    = (wcxi_SOpenedArc*)hArcData;
		spf.strDstFn = HfArgs("%1%2%3")
				.arg( szDestPath ? szDestPath : "" )
				.arg( ( szDestPath && *szDestPath && szDestName && *szDestName ) ? "/" : "" )
				.arg( szDestName ? szDestName : "" ).c_str();

		if( spf.arcd2->isMailformed() )
			return E_BAD_DATA;
		SFi sfi;
		sfi.strDstFnmi = spf.strDstFn;
		sfi.uCFItem    = spf.arcd2->uCurItem;
		sfi.uFsizei    = 0;
		sfi.strSrcFnmi = spf.arcd2->strCurFile;
		spf.arcd2->lsExtr2.push_back( sfi );
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
class CProcRelay : public IProcRelay {
public:
	CProcRelay() : fnProcessDataProc(0) {}
	virtual bool iprSetFilePercentage( const char* szFilename, float fPercentage ){
		if(fnProcessDataProc){
			// // If the user has clicked on Cancel, fnProcessDataProc returns zero.
			// // ... values -1..-100 to directly set first percent bar.
			// // ... values -1000..-1100 for second percent bar (-1000=0%).
			int nSizParam;
			nSizParam = -( (int)( (fPercentage * 99.f) ) + 1 );
			return !!fnProcessDataProc( (char*)szFilename, nSizParam );
		}
		return 1;
	}
	virtual bool iprSetTotalPercentage( const char* szFilename, float fPercentage2 ){
		if(fnProcessDataProc){
			int nSizParam2;
			nSizParam2 = -( (int)( (fPercentage2 * 100.f) ) + 1000 );
			return !!fnProcessDataProc( (char*)szFilename, nSizParam2 );
		}
		return 1;
	}
	tProcessDataProc fnProcessDataProc;
};
void __stdcall SetChangeVolProc( HANDLE, tChangeVolProc pChangeVolProc )
{
	if(!cProcRelayIntrf)
		cProcRelayIntrf = cProcRelayIntrf2 = new CProcRelay;
	//fnChangeVolProc = pChangeVolProc;
}
void __stdcall SetProcessDataProc( HANDLE, tProcessDataProc pProcessDataProc )
{
	//wcxi_DebugString("SetProcessDataProc()");
	if(!cProcRelayIntrf)
		cProcRelayIntrf = cProcRelayIntrf2 = new CProcRelay;
	cProcRelayIntrf2->fnProcessDataProc = pProcessDataProc;
}

BOOL __stdcall CanYouHandleThisFile( char* szFileName )
{
	//wcxi_DebugString("CanYouHandleThisFile()");
	return cPlugin->canYouHandleThisFile( szFileName, 0, 0, 0 );
}
BOOL __stdcall CanYouHandleThisFileW( WCHAR* szFileName )
{
	std::string str;
	hf_Utf8EncodeFromAny( szFileName, -1, &str, 0 );
	return CanYouHandleThisFile( (char*)str.c_str() );
}
int __stdcall GetPackerCaps()
{
	return PK_CAPS_BY_CONTENT | PK_CAPS_SEARCHTEXT;
}

