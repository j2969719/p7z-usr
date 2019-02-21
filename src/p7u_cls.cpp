
#include "p7u_cls.h"

//
// This cpp file put toogether based on example: CPP/7zip/UI/Client7z/Client7z.cpp
//

#include <stdio.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "myWindows/StdAfx.h"
#include "Common/MyInitGuid.h" // must be included once per link, otherwise repeated symbols...
//#include "Windows/DLL.h"   // NDLL::CLibrary
#include "Windows/PropVariant.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Archive/IArchive.h"
#include "../C/7zVersion.h"
#include "7zip/IPassword.h"

#include "hef_str.h"
#include "hef_file.h"
#include "hef_utf8_str.h"
#include "hef_str_args.h"
#include "hef_assert.h"

using namespace NWindows;

// GCC type size compile time assertion.
// size of WCHAR must be 2 for DCMD WCX interface and 4 for P7ZIP library.
typedef char wcxi_assertion_on_type_size_UsdQb00J [ ( sizeof(WCHAR) == 4 ) *2 - 1 ];


WcxiPlugin*               cPlugin = 0;	// as extern in "p7u_cls.h"
//static NDLL::CLibrary*  lib2 = 0;
static void*              lib3 = 0;
static Func_CreateObject  createObjectFunc2 = 0;
IProcRelay*               cProcRelayIntrf = 0; // initialized in "p7u_cls.cpp"
IProcRelay*               cProcRelayIntrf2 = 0;

/*
#include <cstdio>
#include <cstdlib>
// replacement of a minimal set of functions
void* operator new( std::size_t size2 )
{
	std::printf("global op new called, size = %zu\n",size2);
	return std::malloc(size2);
}//*/

class CWCXInFileStream : public CInFileStream
{
public:
	//static int nCntStrm = 0;
	CWCXInFileStream() {
		//wcxi_DebugString_( HfArgs("nCntStrm: %1").arg( ++nCntStrm ).c_str() );
		//int a = MY_VER_MAJOR;
	}
	~CWCXInFileStream() {
		//wcxi_DebugString_( HfArgs("~nCntStrm: %1").arg( --nCntStrm ).c_str() );
	}
};



void fnInit_p7zUsr()
{
	std::string str;
	const char* envv = getenv("P7ZUSRWCX_DEBUG"); //fe. "export P7ZUSRWCX_DEBUG=1"
	bShowDebug =  ( (envv && *envv) ? !!atoi(envv) : 0 );
	bShowDebugIsEnv = ( envv && *envv );
	wcxi_InitMsgBoxFunction();
	//
	DebugLogFile = new std::string;
	envv = getenv("P7ZUSRWCX_LOGFILE"); //fe. "export P7ZUSRWCX_LOGFILE=../ab/wcx.log"
	*DebugLogFile = ( envv && *envv ? envv : "" );
	//
	wcxi_DebugString( HfArgs("P7z Usr initializing, pid:%1").arg((int)getpid()).c_str(),
			WCXI_DSShowAlways );

	wcxi_DebugString( HfArgs("P7Zip C headers, version: [%1]")
			.arg( (const char*)P7ZIP_VERSION )
			.c_str() );
	{
		// get pathh of own module file.
		SelfWcxFile = new std::string;
		Dl_info sdi;
		memset( &sdi, 0, sizeof(sdi) );
		dladdr( (void*)fnInit_p7zUsr, &sdi );
		*SelfWcxFile = sdi.dli_fname;
		wcxi_DebugString( HfArgs("Module file name [%1]").arg(sdi.dli_fname).c_str() );
		if( !SelfWcxFile->empty() )
			hf_strcpy( szP7zWcxFile, hf_basename( SelfWcxFile->c_str() ), 1 );
	}
	hf_assert( !Str7zSoFile );
	Str7zSoFile = new std::string;
	envv = getenv("P7ZUSRWCX_7Z_SO_FILE"); //fe. "export P7ZUSRWCX_7Z_SO_FILE=../abc/7z.so"
	if( envv && *envv ){
		*Str7zSoFile = envv;
		wcxi_DebugString( HfArgs("P7ZUSRWCX_7Z_SO_FILE: [%1]").arg(Str7zSoFile->c_str()).c_str() );
	}
	if( Str7zSoFile->empty() ){
		if( !SelfWcxFile->empty() ){
			*Str7zSoFile = HfArgs("%1/7z.so").arg( hf_dirname(SelfWcxFile->c_str()).c_str()).c_str();
		}else{
			*Str7zSoFile = "./7z.so";
		}
	}
	std::string strIni;
	envv = getenv("P7ZUSRWCX_INI"); //fe. "export P7ZUSRWCX_INI=../abc/p7z_usr.ini"
	if( envv && *envv ){
		strIni = envv;
	}else{
		strIni = HfArgs("%1/p7z_usr.ini").arg( hf_dirname(SelfWcxFile->c_str()).c_str()).c_str();
	}
	if( !hf_FileExists( Str7zSoFile->c_str() ) ){
		wcxi_DebugString( HfArgs("ERROR: 7-zip library file not found. [%1]").arg(Str7zSoFile->c_str()).c_str(),
				WCXI_DSShowAlways|WCXI_DSMsgBox|WCXI_DSAddExitNowMBText );
		hf_assert(0);
	}
	hf_assert( !lib3 );
	if(!(lib3 = dlopen( Str7zSoFile->c_str(), RTLD_LAZY ))){
		wcxi_DebugString( HfArgs("ERROR: Can not load 7-zip library. [%1]").arg(Str7zSoFile->c_str()).c_str(),
				WCXI_DSShowAlways|WCXI_DSMsgBox|WCXI_DSAddExitNowMBText );
		hf_assert(0);
	}
	hf_assert( !createObjectFunc2 );
	if( !( createObjectFunc2 = (Func_CreateObject) dlsym(lib3,"CreateObject") ) ){
		wcxi_DebugString("ERROR: Can not get 'CreateObject' function [eq0B4ig]",
				WCXI_DSShowAlways|WCXI_DSMsgBox|WCXI_DSAddExitNowMBText );
		hf_assert(0);
	}
	hf_assert( !cPlugin );
	cPlugin = new WcxiPlugin( strIni.c_str() );
	//
	wcxi_DebugString("fnInit_p7zUsr done.");
}
WcxiPlugin::WcxiPlugin( const char* szIni )
	: uArcLastIdent(1000), strCYHTFHandlersOff("Cab,\0""xxxxxxxxxxxxxxxxxxxxx")
	, uScanSize(1<<23), uCYHTFScanSize(1<<23), bDoAccessViolationOnCAErr(1)
	, strPromptBoxSmn("FDIALOGBOX_INPUTBOX$PCHAR$PCHAR$LONGBOOL$PCHAR$LONGINT$$LONGBOOL\0;xxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
	, strMsgBoxSmn("FDIALOGBOX_MESSAGEBOX$PCHAR$PCHAR$LONGINT$$LONGINT\0;xxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
{
	std::string str;
	strIni2 = wcxi_readlink(szIni,0);
	wcxi_DebugString( HfArgs("INI: [%1], exists: %2")
			.arg(strIni2.c_str())
			.arg( (int)hf_FileExists(szIni) )
			.c_str());

	std::vector<std::pair<std::string,std::string> >::iterator a;
	std::vector<std::pair<std::string,std::string> > vars;

	hf_ParseSimpleIniFile( strIni2.c_str(), vars );
	wcxi_DebugString( HfArgs("INI parse, n-vars: %1").arg((int)vars.size()).c_str() );
	if( !bShowDebugIsEnv ){
		a = std::find_if( vars.begin(), vars.end(), HfPredTTPair<std::string,std::string>("bShowDebug") );
		bShowDebug = ( a != vars.end() ? !!atoi(a->second.c_str()) : bShowDebug );
	}
	{
		a = std::find_if( vars.begin(), vars.end(), HfPredTTPair<std::string,std::string>("szCYHTFHandlersOff") );
		if( a != vars.end() )
			strCYHTFHandlersOff = a->second.c_str();
		hf_explode( strCYHTFHandlersOff.c_str(), ",", lsCYHTFHandlersOff, "\r\n\t\x20" );
		wcxi_DebugString( HfArgs("szCYHTFHandlersOff: [%1]").arg(strCYHTFHandlersOff.c_str()).c_str() );
	}
	updateArcHandlers();
	{	// "szCYHTFHandlersPriority"
		a = std::find_if( vars.begin(), vars.end(), HfPredTTPair<std::string,std::string>("szCYHTFHandlersPriority") );
		if( a != vars.end() ){
			wcxi_DebugString( HfArgs("szCYHTFHandlersPriority: [%1]").arg( a->second.c_str() ).c_str() );
			std::vector<std::string>::const_reverse_iterator b;
			std::vector<SArcHandler>::iterator c;
			std::vector<std::string> lss;
			hf_explode( a->second.c_str(), ",", lss, "\r\n\t\x20" );
			std::remove( lss.begin(), lss.end(), std::string("") );
			for( b = lss.rbegin(); b != lss.rend(); ++b ){
				c = std::find_if( hndlrs2.begin(), hndlrs2.end(), SArcHandler::ByName(b->c_str()) );
				if( c != hndlrs2.end() ){
					SArcHandler itrm = *c;
					hndlrs2.erase( c );
					hndlrs2.insert( hndlrs2.begin(), itrm );
				}
			}
		}
	}
	{
		std::vector<SArcHandler>::const_iterator a; int nMaxLen2 = 0;
		wcxi_DebugString( HfArgs("Number of handlers: %1").arg((int)hndlrs2.size()).c_str() );
		for( a = hndlrs2.begin(); a != hndlrs2.end(); ++a ){
			nMaxLen2 = std::max( nMaxLen2, (int)a->exts.size() );
		}
		for( a = hndlrs2.begin(); a != hndlrs2.end(); ++a ){
			str = a->name2.c_str();
			str.resize( nMaxLen2, '\x20' );
			wcxi_DebugString(HfArgs("%1: [%2]")
					.arg( str.c_str() )
					.arg( hf_implode( a->exts, "\x20" ).c_str() ).c_str() );
		}
	}
	{
		//uScanSize,uCYHTFScanSize
		a = std::find_if( vars.begin(), vars.end(), HfPredTTPair<std::string,std::string>("uScanSize") );
		uScanSize = ( a != vars.end() ? HfArgs("%1").arg(a->second.c_str()).toUint64() : uScanSize );
		wcxi_DebugString( HfArgs("uScanSize: [%1]").arg(uScanSize).c_str() );

		a = std::find_if( vars.begin(), vars.end(), HfPredTTPair<std::string,std::string>("uCYHTFScanSize") );
		uCYHTFScanSize = ( a != vars.end() ? HfArgs("%1").arg(a->second.c_str()).toUint64() : uCYHTFScanSize );
		wcxi_DebugString( HfArgs("uCYHTFScanSize: [%1]").arg(uCYHTFScanSize).c_str() );
	}
	{
		a = std::find_if( vars.begin(), vars.end(), HfPredTTPair<std::string,std::string>("bDoAccessViolationOnCAErr") );
		bDoAccessViolationOnCAErr = ( a != vars.end() ? !!HfArgs("%1").arg(a->second.c_str()).toUint64() : bDoAccessViolationOnCAErr );
		wcxi_DebugString( HfArgs("bDoAccessViolationOnCAErr: %1").arg((int)bDoAccessViolationOnCAErr).c_str() );
	}
	{
		// Symbol name for DCMD input dialog box function.
		// Got them by examining 'doublecmd' executable with a command: "$> nm -D ./doublecmd"
		// szInputBoxSymbol and szMessageBoxSymbol.
		a = std::find_if( vars.begin(), vars.end(), HfPredTTPair<std::string,std::string>("szInputBoxSymbol") );
		strPromptBoxSmn = ( a != vars.end() ? a->second : strPromptBoxSmn );
		a = std::find_if( vars.begin(), vars.end(), HfPredTTPair<std::string,std::string>("szMessageBoxSymbol") );
		strMsgBoxSmn = ( a != vars.end() ? a->second : strMsgBoxSmn );
	}
	{
		// szArcOpenShellNotify
		a = std::find_if( vars.begin(), vars.end(), HfPredTTPair<std::string,std::string>("szArcOpenShellNotify") );
		strArcOpenShellNotify = ( a != vars.end() ? a->second : strArcOpenShellNotify );
	}
}
std::vector<SArcHandler*> WcxiPlugin::getHandlersForExt( const char* szExt )
{
	std::vector<SArcHandler*> lsx;
	std::string ext2 = szExt;
	std::vector<SArcHandler>::iterator a;
	for( a = hndlrs2.begin(); a != hndlrs2.end(); ++a ){
		if( ext2 == "*" ){
			lsx.push_back(&*a);
		}else{
			std::vector<std::string>::const_iterator b = a->exts.begin();
			for(; b != a->exts.end(); ++b ){
				if( !hf_strcasecmp( ext2.c_str(), b->c_str(), ext2.size() ) ){
					lsx.push_back(&*a);
				}
			}
		}
	}
	return lsx;
}
void fnDeinit()
{
	wcxi_DebugString(HfArgs("fnDeinit, pid:%1, uGlobArcExcCalbRefc: %2")
			.arg( (int)getpid() )
			.arg( uGlobArcExcCalbRefc ).c_str() );
	if( DebugLogFile ){
		delete DebugLogFile;
		DebugLogFile = 0;
	}
	if( SelfWcxFile ){
		delete SelfWcxFile;
		SelfWcxFile = 0;
	}
	if( Str7zSoFile ){
		delete Str7zSoFile;
		Str7zSoFile = 0;
	}
//	if( lib2 ){
//		delete lib2;
//		lib2 = 0;
//	}
	if( lib3 ){
		dlclose(lib3);
		lib3 = 0;
	}
	if( cPlugin ){
		delete cPlugin;
		cPlugin = 0;
	}
	//wcxi_DebugString( HfArgs("exit_ nCntStrm: %1").arg( nCntStrm ).c_str() );
	//wcxi_DebugString(HfArgs("deinit uGlobArcExcCalbRefc: %1").arg( uGlobArcExcCalbRefc ).c_str() );
}

//static int uAocRefcnt = 0;
class CArcOpenCalb
	: public IArchiveOpenCallback
	, public ICryptoGetTextPassword
	, public CMyUnknownImp
{
public:
	CArcOpenCalb( const char* szArcFnm_ ) : bAskedPassword(0), strArcFnm(szArcFnm_) {
		//wcxi_DebugString_( HfArgs("CArcOpenCalb() n:%1").arg( ++uAocRefcnt ).c_str() );
	}
	virtual ~CArcOpenCalb(){
		//wcxi_DebugString_( HfArgs("~CArcOpenCalb() n:%1").arg( --uAocRefcnt ).c_str() );
	}
	MY_UNKNOWN_IMP1(ICryptoGetTextPassword)
	//
	HRESULT SetTotal( const UInt64 *files, const UInt64 *bytes ) {return S_OK;}
	HRESULT SetCompleted( const UInt64 *files, const UInt64 *bytes ) {return S_OK;}
	// ICryptoGetTextPassword
	//STDMETHOD (CryptoGetTextPassword)( BSTR *aPassword )
	HRESULT CryptoGetTextPassword( BSTR *aPassword ){
		wcxi_DebugString( HfArgs("CArcOpenCalb::CryptoGetTextPassword().").c_str() );
		bAskedPassword = 1;

		hf_assert( cProcRelayIntrf );
		std::string pw2; std::basic_string<wchar_t> pw3; // wchar_t <=> WCHAR
		if( cProcRelayIntrf->iprTryGetOrAskArcFNPassword( strArcFnm.c_str(), &pw2 ) )
			hf_Utf8DecodeToAny( pw2.c_str(), -1, pw3, 0 );
		HRESULT res = StringToBstr( UString(pw3.c_str()), aPassword );
		{
			std::basic_string<wchar_t>::iterator a;
			for( a = pw3.begin(); a != pw3.end(); ++a )
				*a = 'x';
			pw3.clear();
		}
		return res;
	}
	bool bAskedPassword;
private:
	std::string strArcFnm;
};

void WcxiPlugin::clearArchiveStruct( wcxi_SOpenedArc& inp )
{
	if( inp.archive3 ){
		CMyComPtr<IInArchive>* archive4 = (CMyComPtr<IInArchive>*)inp.archive3;
		(*archive4)->Close();
		//(*archive4)->Release();
		delete archive4;
		inp.archive3 = archive4 = 0;
	}
	inp.ident = inp.verify2 = 0;
}
bool WcxiPlugin::closeArchive( void* hArcData, int* err )
{
	wcxi_DebugString("x::closeArchive().");
	wcxi_SOpenedArc* soa2 = (wcxi_SOpenedArc*)hArcData;
	bool retv = 1, bPreventAccVio = 0;
	if( soa2->isMailformed() ){
		wcxi_DebugString("ERROR: Mailformed archive data struct on close-archive [IWJvzRyw].");
		retv = 0;
		*err = WCXI_EBadData; // E_BAD_DATA
	}
	if( !soa2->lsExtr2.empty() ){
		std::string err2;
		if( !(retv = extractMultipleFiles( soa2, err, &err2 ) ) ){
			std::string str3 = ( !err2.empty() ? err2.c_str() : "unknown error" );
			IProcRelay::SMsgBox smb;
			smb.capt   = szP7zWcxFile;
			smb.msg    = str3.c_str();
			smb.flags3 = 0x10;
			if( cProcRelayIntrf->iprMessageBox(smb) ){
				bPreventAccVio = 1;
			}
		}
	}
	clearArchiveStruct( *soa2 );
	delete soa2;
	if( bDoAccessViolationOnCAErr && !retv && !bPreventAccVio ){
		// This is a workaround method to indicate that error happened durning archive
		// extract process.
		// currently DCMD doesnt report any errors returned by the CloseArchive().
		// generate Access Violation error by an arbitrary attempt of writing
		// integer value at address 0.
		wcxi_DebugString( HfArgs(
				"ERROR: [%1]. NOTE: No way to report error durning CloseArchive() call. "
				"See INI.bDoAccessViolationOnCAErr. "
				"Arbitrary Access Violation follows..."
				).arg( *err ).c_str() );
		*((int*)0) = 1;
	}
	return retv;
}
void* WcxiPlugin::openArchive( wcxi_OpenArchiveData& acd )
{
	hf_assert( createObjectFunc2 );
	std::string str, strFnmRl2;
	wcxi_SOpenedArc soa;
	soa.ident      = ++uArcLastIdent;
	soa.verify2    = ~soa.ident;
	soa.strArcName = acd.szArcName;

	if( !(str = wcxi_readlink( acd.szArcName, 0 )).empty() ){
		strFnmRl2 = str;
	}else{
		strFnmRl2 = acd.szArcName;
	}
	UInt64 uScanSize3 = uScanSize;
	const SArcHandler* hdlr = 0;
	if( soa.strArcName == sLastCyhtf.strArcFn ){
		hf_assert( sLastCyhtf.hdlr2 );
		hdlr = sLastCyhtf.hdlr2;
		uScanSize3 = uCYHTFScanSize;
		//sLastCyhtf.clear2();
	}else{
		sLastCyhtf.clear2();
		uScanSize3 = uScanSize;
		std::string ext4 = hf_basename3( acd.szArcName ).second;
		if( !canYouHandleThisFile( soa.strArcName.c_str(), &hdlr, uScanSize3, ext4.c_str() ) ){
			wcxi_DebugString( HfArgs("NOTE: No handler for archive: [.../%1]. [aY0lZRW]")
					.arg( hf_basename(acd.szArcName) ).c_str() );
			clearArchiveStruct( soa );
			acd.eOpenResult = WCXI_EUnknownFormat;
			return 0;
		}
	}
	if(!hdlr){
		wcxi_DebugString( HfArgs("ERROR: No handler for archive: [.../%1]. [WmqBeAy]")
				.arg( hf_basename(acd.szArcName) ).c_str() );
		clearArchiveStruct( soa );
		acd.eOpenResult = WCXI_EUnknownFormat;
		return 0;
	}
	soa.hdlr3 = hdlr;
	CWCXInFileStream* fileSpec = new CWCXInFileStream;
	std::basic_string<wchar_t> strUc;
	hf_Utf8DecodeToAny( strFnmRl2.c_str(), -1, strUc, 0 );

	if( !fileSpec->Open( FString(strUc.c_str()) ) ){
		wcxi_DebugString( HfArgs("ERROR: Can not open archive file: [%1] [UNkxd1]").arg(acd.szArcName).c_str() );
		clearArchiveStruct( soa );
		delete fileSpec;
		acd.eOpenResult = WCXI_EOpen; //E_EOPEN
		return 0;
	}
	CMyComPtr<IInArchive>* archive2 = new CMyComPtr<IInArchive>;
	soa.archive3 = archive2;//&CLSID_Format
	if( createObjectFunc2( (GUID*)(&hdlr->guid2[0]), &IID_IInArchive, (void**)&(*archive2) ) != S_OK ){
		wcxi_DebugString("ERROR: Can not get class object. [gKIBoR1f]");
		clearArchiveStruct( soa );
		delete fileSpec;
		acd.eOpenResult = WCXI_EUnknownFormat;
		return 0;
	}
	CMyComPtr<IInStream> file2( fileSpec );
	CArcOpenCalb* cAoc = new CArcOpenCalb( soa.strArcName.c_str() );
	//
	CMyComPtr<IArchiveOpenCallback> cAocCom2( cAoc );
	if( (**archive2).Open( file2, &uScanSize3, cAocCom2 ) != S_OK ){
		wcxi_DebugString( HfArgs("ERROR: Can not open file as archive: [%1] [MZDBcQG]").arg(acd.szArcName).c_str() );
		clearArchiveStruct( soa );
		acd.eOpenResult = WCXI_EUnknownFormat;
		if( cAoc->bAskedPassword )
			cProcRelayIntrf->iprRemoveArcFNPassword( soa.strArcName.c_str() );
		return 0;
	}
	UInt32 uNumItems2 = 0;
	(*archive2)->GetNumberOfItems( &uNumItems2 );
	wcxi_DebugString( HfArgs("[%1] num items: %2")
			.arg( hf_basename( acd.szArcName ) )
			.arg((uint32_t)uNumItems2).c_str() );

	soa.uNumItems = uNumItems2;
	wcxi_DebugString("x::openArchive() done OK.");


	if( !strArcOpenShellNotify.empty() ){
		wcxi_DebugString( HfArgs("shell-command: [%1]")
				.arg( strArcOpenShellNotify.c_str() ).c_str() );
		std::string strSelfWcxDir = hf_dirname( SelfWcxFile->c_str() );
		//wcxi_DebugString( HfArgs("handler_name : [%1]\n"
		//				"arc_name     : [%2]\n"
		//				"wcx_dir      : [%3]")
		//		.arg( hdlr->name2.c_str() )
		//		.arg( soa.strArcName.c_str() )
		//		.arg( strSelfWcxDir.c_str() ).c_str() );
		std::string cmd2 = strArcOpenShellNotify;
		cmd2 = hf_strreplace( "{*handler_name*}", hdlr->name2.c_str(), cmd2.c_str() );
		cmd2 = hf_strreplace( "{*arc_name*}", soa.strArcName.c_str(), cmd2.c_str() );
		cmd2 = hf_strreplace( "{*wcx_dir*}", strSelfWcxDir.c_str(), cmd2.c_str() );
		if( system( cmd2.c_str() ) ){
		}
	}
	return new wcxi_SOpenedArc( soa );
}
bool WcxiPlugin::readHeaderEx( void* hArcData, wcxi_HeaderDataEx& shd )
{
	wcxi_SOpenedArc* soa3 = (wcxi_SOpenedArc*)hArcData;
	if( soa3->isMailformed() ){
		wcxi_DebugString("WARNING: Mailformed archive data struct on read-header-ex call [ynxubrS].");
		return WCXI_EBadData;//E_BAD_DATA
	}
	//wcxi_DebugString( HfArgs("readHeaderEx, uNextItem: %1")
	//		.arg( (int)soa3->uNextItem ).c_str() );
	if( soa3->uNextItem >= soa3->uNumItems ){
		shd.eError = WCXI_ENoMoreFiles;
		return 0;
	}
	CMyComPtr<IInArchive>* archive2 = (CMyComPtr<IInArchive>*)soa3->archive3;
	//hf_assert_( archive2 );
	if(!archive2){
		shd.eError = WCXI_EBadData;
		return 0;
	}
	NCOM::CPropVariant propFn, propIsDir, prop;
	std::string str; UString str2; std::basic_string<wchar_t> strFnWBfr;

	uint32_t uCitm = soa3->uNextItem++;
	(*archive2)->GetProperty( uCitm, kpidPath, &propFn );
	// "CPP/Common/MyWindows.h"
	// VT_BSTR --> bstrVal, VT_BOOL --> boolVal
	if( propFn.vt == VT_EMPTY && soa3->uNumItems == 1 ){ // if fe. ".tar.gz"
		std::string fnm2;
		fnm2 = wcxi_StripPostTarEtcExts( hf_basename(soa3->strArcName.c_str()), soa3->hdlr3 );
		hf_Utf8DecodeToAny( fnm2.c_str(), -1, strFnWBfr, 0 );
		propFn = strFnWBfr.c_str();
	}
	if( propFn.vt == VT_BSTR ){
		(*archive2)->GetProperty( uCitm, kpidIsDir, &propIsDir );
		shd.nFileAttr |= ( propIsDir.bVal ? WCXI_FADir : 0 );
		str2 = propFn.bstrVal;
		str.clear();
		hf_Utf8EncodeFromAny( str2.Ptr(), str2.Len(), &str, 0 );
		shd.strFileName = ( propIsDir.bVal ? "/" : "" ) + str;
		soa3->strCurFile = shd.strFileName;
		soa3->uCurItem = uCitm;

		prop.Clear();
		(*archive2)->GetProperty( uCitm, kpidSize, &prop );
		shd.uUnpSize = ( prop.vt == VT_UI8 ? (uint64_t)prop.uhVal.QuadPart : 0 );
		prop.Clear();
		(*archive2)->GetProperty( uCitm, kpidPackSize, &prop );
		shd.uPackSize = ( prop.vt == VT_UI8 ? (uint64_t)prop.uhVal.QuadPart : 0 );

		// NOTE: must prepend dirs with slash ('/') character,
		//       otherwise DCMD boguoussly shows directory name 2nd
		//       time as a regular file.
	}else{
		wcxi_DebugString( HfArgs("ERROR: readHeaderEx, item %1 file name not a string.")
				.arg(uCitm).c_str() );
		shd.eError = WCXI_EBadData;//E_BAD_DATA
		return 0;
	}
	return 1;
}
void WcxiPlugin::updateArcHandlers()
{
	hf_assert(lib3);
	Func_GetNumberOfFormats fnGnof = (Func_GetNumberOfFormats) dlsym( lib3, "GetNumberOfFormats" );
	Func_GetHandlerProperty2 fnGhp2 = (Func_GetHandlerProperty2) dlsym( lib3, "GetHandlerProperty2" );
	UInt32 nNumFmts = 0;
	fnGnof( &nNumFmts );

	std::string str;
	UString strUc; NCOM::CPropVariant prop;
	std::basic_string<wchar_t> dmy0;

	for( uint32_t i = 0; i < nNumFmts; i++ ){
		SArcHandler sah;
		prop.Clear();
		fnGhp2( i, NArchive::NHandlerPropID::kName, &prop );
		//hf_assert(prop.bstrVal);
		strUc = ( prop.bstrVal ? prop.bstrVal : dmy0.c_str() );
		hf_Utf8EncodeFromAny( strUc.Ptr(), strUc.Len(), &sah.name2, 0 );

		prop.Clear();
		fnGhp2( i, NArchive::NHandlerPropID::kExtension, &prop );
		if( prop.bstrVal ){
			strUc = prop.bstrVal;
			str.clear();
			hf_Utf8EncodeFromAny( strUc.Ptr(), strUc.Len(), &str, 0 );
			hf_explode( str.c_str(), "\x20", sah.exts, "\x20\r\n" );
		}
		prop.Clear();
		fnGhp2( i, NArchive::NHandlerPropID::kAddExtension, &prop );
		if( prop.bstrVal ){
			strUc = prop.bstrVal;
			str.clear();
			hf_Utf8EncodeFromAny( strUc.Ptr(), strUc.Len(), &str, 0 );
			hf_explode( str.c_str(), "\x20", sah.extsAdd, "\x20\r\n" );
		}
		prop.Clear();
		fnGhp2( i, NArchive::NHandlerPropID::kClassID, &prop );
		//hf_assert(prop.bstrVal); // GUID: "CPP/Common/MyGuidDef.h"
		const GUID* guid3 = (const GUID*)( prop.bstrVal ? prop.bstrVal : dmy0.c_str() );
		sah.guid2.resize( sizeof(*guid3), 0 );
		memcpy( &sah.guid2[0], guid3, sizeof(*guid3) );

		prop.Clear();
		fnGhp2( i, NArchive::NHandlerPropID::kFlags, &prop );
		sah.flags2 = prop.ulVal;

		hndlrs2.push_back(sah);
	}
}
void WcxiPlugin::SCyhtf::clear2()
{
	strArcFn = strHnName = "";
	hdlr2 = 0;
}
std::string wcxi_readlink( const char* szPath, bool bEmptyOnFail )
{
	std::string str = ( bEmptyOnFail ? "" : szPath );
	struct stat sbx;
	memset( &sbx, 0, sizeof(sbx) );
	if( -1 != lstat( szPath, &sbx ) ){
		if( S_ISLNK(sbx.st_mode) ){ // if is symbolic link.
			if( sbx.st_size < 65536 ){ // sanity check on sym-link size.
				std::vector<char> bfr;
				int num = sbx.st_size + 1;
				bfr.resize( num, 0 );
				ssize_t numr = readlink( szPath, &bfr[0], bfr.size() );
				if( numr != -1 ){
					bfr[numr] = 0;
					str.assign( &bfr[0] );
				}
			}
		}
	}
	return str;
}
std::string wcxi_StripPostTarEtcExts( const char* inp, const SArcHandler* hdlr4 )
{
	// bzip2: [bz2 bzip2 tbz2 tbz; * * .tar .tar]
	// gzip:  [gz gzip tgz tpz; * * .tar .tar]
	// xz:    [xz txz; * .tar]
	// Z:     [z taz; * .tar]
	std::string ext3; std::vector<std::string>::const_iterator a;
	std::pair<std::string,std::string> bn2 = hf_basename3( inp, EBN3F_EXTANYCHARS, 5 );
	std::pair<std::string,std::string> bn3 = hf_basename3( bn2.first.c_str(), EBN3F_EXTANYCHARS, 5 );
	if( !bn3.second.empty() ){
		a = std::find( hdlr4->extsAdd.begin(), hdlr4->extsAdd.end(), bn3.second );
		if( a != hdlr4->extsAdd.end() )
			return bn2.first;
	}
	if( !bn2.second.empty() ){
		a = std::find( hdlr4->exts.begin(), hdlr4->exts.end(), bn2.second );
		if( a != hdlr4->exts.end() ){
            int dist = std::distance( hdlr4->exts.begin(), a );
            if( dist < (int)hdlr4->extsAdd.size() ){
				ext3 = hdlr4->extsAdd[dist]; // fe. ".tar"
				if( ext3 != "*" )
					return bn2.first + ext3;
            }
		}
		return bn2.first;
	}
	return inp;
}
bool WcxiPlugin::
canYouHandleThisFile( const char* szFileName, const SArcHandler** ouHdlr,
						uint64_t uScanSize4, const char* szExt )
{
	//wcxi_DebugString( HfArgs("x::canYouHandleThisFile()").arg("").c_str());
	std::string str, strFnmRl;
	uScanSize4 = ( uScanSize4 ? uScanSize4 : uCYHTFScanSize );
	hf_assert(cPlugin);
	hf_assert(lib3);
	sLastCyhtf.clear2();
	if( !szFileName || !*szFileName )
		return 0;
	if( !(str = wcxi_readlink( szFileName, 0 )).empty() ){
		strFnmRl = str;
	}else{
		strFnmRl = szFileName;
	}
	std::basic_string<wchar_t> strFnmUc;
	hf_Utf8DecodeToAny( strFnmRl.c_str(), -1, strFnmUc, 0 );
	wcxi_DebugString( HfArgs("strFnmUc.size: %1").arg( strFnmUc.size() ).c_str());

	std::vector<SArcHandler*> hdlrs3;
	std::vector<SArcHandler*>::const_iterator c;
	if( szExt && *szExt ){
		hdlrs3 = getHandlersForExt( szExt );
	}else{
		hdlrs3 = getHandlersForExt( "*" );
	}
	wcxi_DebugString( HfArgs("Trying archive handlers (num:%1)").arg((int)hdlrs3.size()).c_str() );

	CWCXInFileStream* fileSpec2 = new CWCXInFileStream;//CInFileStream
	if( !fileSpec2->Open( FString(strFnmUc.c_str()) ) ){
		wcxi_DebugString( HfArgs("ERROR: Can not open archive file: [%1] [Q7JgqyS]").arg(szFileName).c_str() );
		delete fileSpec2;
		return 0;
	}
	fileSpec2->AddRef();
	// NOTE: Somehow Cab handler freezes somwhere in the stream-open,
	//       on all files tested...
	int i; bool bCanOpen = 0; NCOM::CPropVariant prop;
	for( c = hdlrs3.begin(), i=0; c != hdlrs3.end(); ++c, i++ ){
		std::vector<std::string>::const_iterator b;
		b = std::find( lsCYHTFHandlersOff.begin(), lsCYHTFHandlersOff.end(), (**c).name2 );
		if( b != lsCYHTFHandlersOff.end() )
			continue;

		wcxi_DebugString( HfArgs("Trying handler %1/%2 '%3'")
				.arg(i+1)
				.arg((int)hdlrs3.size())
				.arg((**c).name2.c_str()).c_str() );

		CMyComPtr<IInArchive> archive4;
		if( createObjectFunc2( (GUID*)(&(**c).guid2[0]), &IID_IInArchive, (void**)&archive4 ) != S_OK ){
			wcxi_DebugString(HfArgs("ERROR: Can not get class object: '%1'. [LkB9ktH]").arg((**c).name2.c_str()).c_str() );
			continue;
		}
		fileSpec2->Seek( 0, STREAM_SEEK_SET, 0 );//[offset,origin,*offset]
		CMyComPtr<IInStream> file3( fileSpec2 );
		const UInt64 uScanSize2 = uScanSize4;//uCYHTFScanSize;
		// STDMETHOD(Open_)(IInStream *stream, const UInt64 *maxCheckStartPosition,
		//     IArchiveOpenCallback *openCallback) MY_NO_THROW_DECL_ONLY x; ...
		CArcOpenCalb* cAoc = new CArcOpenCalb( szFileName );
		CMyComPtr<IArchiveOpenCallback> cAocCom( cAoc );
		if( archive4->Open( file3, &uScanSize2, cAocCom ) == S_OK ){
			wcxi_DebugString(HfArgs("NOTE: open ok as '%1'").arg((**c).name2.c_str()).c_str() );
			bCanOpen = 1;
			//
			//prop.Clear();
			//archive4->GetArchiveProperty( kpidPhySize, &prop );
			//uint64_t uPhySize = ( prop.vt == VT_UI8 ? (uint64_t)prop.uhVal.QuadPart : 0 );
			//wcxi_DebugString(HfArgs("uPhySize: %1").arg2( uPhySize ).c_str() );
			//prop.Clear();
			//archive4->GetArchiveProperty( kpidOffset, &prop );
			//uint64_t uArcOffset = (uint64_t)prop.uhVal.QuadPart;
			//wcxi_DebugString(HfArgs("uArcOffset: %1").arg2( uArcOffset ).c_str() );
			//
			sLastCyhtf.strArcFn  = szFileName;
			sLastCyhtf.strHnName = (**c).name2.c_str();
			sLastCyhtf.hdlr2     = &(**c);
			if(ouHdlr)
				*ouHdlr = &(**c);
			break;
		}else{
			if( cAoc->bAskedPassword )
				cProcRelayIntrf->iprRemoveArcFNPassword( szFileName );
		}
	}
	fileSpec2->Release();
	return bCanOpen;
}
std::string WcxiPlugin::getP7ZIPHeadersVersion()const
{
	std::string str = P7ZIP_VERSION;
	return str;
}
