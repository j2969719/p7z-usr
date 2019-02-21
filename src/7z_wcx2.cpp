
#include "7z_wcx2.h"

//
// This cpp file put toogether based on example: CPP/7zip/UI/Client7z/Client7z.cpp
//

#include <stdio.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "myWindows/StdAfx.h"
#include "Common/MyInitGuid.h" // must be included once per link, otherwise repeated symbols...
#include "Windows/DLL.h"
#include "Windows/PropVariant.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Archive/IArchive.h"
#include "../C/7zVersion.h"

#include "hef_str.h"
#include "hef_file.h"
#include "hef_utf8_str.h"
#include "hef_str_args.h"
#include "hef_assert.h"
#include "7z_wcx_cmn.h"

using namespace NWindows;

WcxiPlugin*               cPlugin = 0;	// as extern in "7z_wcx2.h"
static NDLL::CLibrary*    lib2 = 0;
static Func_CreateObject  createObjectFunc2 = 0;
IProcRelay*               cProcRelayIntrf = 0; // initialized in "7z_wcx.cpp"


class CWCXInFileStream : public CInFileStream
{
public:
	//static int nCntStrm = 0;
	CWCXInFileStream() {
		//wcxi_DebugString( HfArgs("nCntStrm: %1").arg( ++nCntStrm ).c_str() );
		//int a = MY_VER_MAJOR;
	}
	~CWCXInFileStream() {
		//wcxi_DebugString( HfArgs("~nCntStrm: %1").arg( --nCntStrm ).c_str() );
	}
};

void fnInit_p7zUsr()
{
	std::string str;
	const char* envv = getenv("P7ZUSRWCX_DEBUG"); //fe. "export P7ZUSRWCX_DEBUG=1"
	bShowDebug = ( envv && !!atoi(envv) );
	//
	DebugLogFile = new std::string;
	envv = getenv("P7ZUSRWCX_LOGFILE"); //fe. "export P7ZUSRWCX_LOGFILE=../ab/wcx.log"
	*DebugLogFile = ( envv && *envv ? envv : "" );
	//
	wcxi_DebugString( HfArgs("fnInit_p7zUsr, pid:%1").arg((int)getpid()).c_str() );

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
		wcxi_DebugString( HfArgs("ERROR: 7-zip library file not found. [%1]").arg(Str7zSoFile->c_str()).c_str() );
		hf_assert(0);
	}
	hf_assert( !lib2 );
	lib2 = new NDLL::CLibrary;
	std::basic_string<wchar_t> strSoFile2;
	hf_Utf8DecodeToAny( Str7zSoFile->c_str(), -1, strSoFile2, 0 );
	if( !lib2->Load( strSoFile2.c_str() ) ){
		wcxi_DebugString( HfArgs("ERROR: Can not load 7-zip library. [%1]").arg(Str7zSoFile->c_str()).c_str() );
		hf_assert(0);
	}
	hf_assert( !createObjectFunc2 );
	if( !( createObjectFunc2 = (Func_CreateObject)lib2->GetProc("CreateObject") ) ){
		wcxi_DebugString("ERROR: Can not get 'CreateObject' function [cIlwn5z]");
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
{
	std::string str, strIni = wcxi_readlink(szIni,0);
	wcxi_DebugString( HfArgs("INI: [%1], exists: %2")
			.arg(strIni.c_str())
			.arg( (int)hf_FileExists(szIni) )
			.c_str());

	std::vector<std::pair<std::string,std::string> >::iterator a;
	std::vector<std::pair<std::string,std::string> > vars;

	hf_ParseSimpleIniFile( strIni.c_str(), vars );
	wcxi_DebugString( HfArgs("INI parse, n-vars: %1").arg((int)vars.size()).c_str() );
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
		std::string str2;
		std::vector<SArcHandler>::const_iterator a;
		wcxi_DebugString( HfArgs("num-hndlrs: %1").arg((int)hndlrs2.size()).c_str() );
		for( a = hndlrs2.begin(); a != hndlrs2.end(); ++a ){
			str2 += a->name2 + ",";
			wcxi_DebugString(HfArgs("%1: n-exts:%2")
					.arg(a->name2.c_str())
					.arg((int)a->exts.size()).c_str() );
		}
		wcxi_DebugString(HfArgs("handlers: [%1]").arg(str2.c_str() ).c_str() );
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
}
std::vector<SArcHandler*> WcxiPlugin::getHandlersForExt( const char* szExt )
{
	std::vector<SArcHandler*> lsx;
	std::string ext2 = szExt;//hf_basename3( szFnm, 0, 99 ).second;
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
	char bfr[128];
	sprintf( bfr, "fnDeinit, pid:%d", (int)getpid() ); //nCntStrm
	wcxi_DebugString( bfr );
	if(DebugLogFile){
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
	if( lib2 ){
		delete lib2;
		lib2 = 0;
	}
	if(cPlugin){
		hf_assert(cPlugin);
		delete cPlugin;
		cPlugin = 0;
	}
	//wcxi_DebugString( HfArgs("exit_ nCntStrm: %1").arg( nCntStrm ).c_str() );
}
void WcxiPlugin::clearArchiveStruct( wcxi_SOpenedArc& inp )
{
	if( inp.archive3 ){
		CMyComPtr<IInArchive>* archive4 = (CMyComPtr<IInArchive>*)inp.archive3;
		(*archive4)->Close();
		delete archive4;
		inp.archive3 = archive4 = 0;
	}
	inp.ident = inp.verify2 = 0;
}
bool WcxiPlugin::closeArchive( void* hArcData, int* err )
{
	wcxi_DebugString("x::closeArchive().");
	wcxi_SOpenedArc* soa2 = (wcxi_SOpenedArc*)hArcData;
	bool retv = 1;
	if( soa2->isMailformed() ){
		wcxi_DebugString("ERROR: Mailformed archive data struct on close-archive [IWJvzRyw].");
		retv = 0;
		*err = WCXI_EBadData; // E_BAD_DATA
	}
	if( !soa2->lsExtr2.empty() ){
		retv = extractMultipleFiles( soa2, err );
	}
	clearArchiveStruct( *soa2 );
	delete soa2;
	if( bDoAccessViolationOnCAErr && !retv ){
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
	if( (*archive2)->Open( file2, &uScanSize3, 0 ) != S_OK ){
		wcxi_DebugString( HfArgs("ERROR: Can not open file as archive: [%1] [MZDBcQG]").arg(acd.szArcName).c_str() );
		clearArchiveStruct( soa );
		acd.eOpenResult = WCXI_EUnknownFormat;
		return 0;
	}
	UInt32 uNumItems2 = 0;
	(*archive2)->GetNumberOfItems( &uNumItems2 );
	wcxi_DebugString( HfArgs("[%1] num items: %2")
			.arg( hf_basename( acd.szArcName ) )
			.arg((uint32_t)uNumItems2).c_str() );

	soa.uNumItems = uNumItems2;
	wcxi_DebugString("wcxi_OpenArchive() done OK.");
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
	hf_assert( archive2 );

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
	hf_assert(lib2);
	Func_GetNumberOfFormats fnGnof = (Func_GetNumberOfFormats)lib2->GetProc("GetNumberOfFormats");
	Func_GetHandlerProperty2 fnGhp2 = (Func_GetHandlerProperty2)lib2->GetProc("GetHandlerProperty2");
	UInt32 nNumFmts = 0;
	fnGnof( &nNumFmts );

	std::string str;
	UString strUc; NCOM::CPropVariant prop;
	for( uint32_t i = 0; i < nNumFmts; i++ ){
		SArcHandler sah;
		prop.Clear();
		fnGhp2( i, NArchive::NHandlerPropID::kName, &prop );
		hf_assert(prop.bstrVal);
		strUc = prop.bstrVal;
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
		hf_assert(prop.bstrVal);
		const GUID* guid3 = (const GUID*)prop.bstrVal;  // GUID: "CPP/Common/MyGuidDef.h"
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
	if( -1 != lstat( szPath, &sbx ) ){
		std::vector<char> bfr;
		int num = sbx.st_size + 1;
		bfr.resize( num, 0 );
		ssize_t numr = readlink( szPath, &bfr[0], bfr.size() );
		if( numr != -1 ){
			bfr[numr] = 0;
			str.assign( &bfr[0] );
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
	std::string str, strFnmRl;
	uScanSize4 = ( uScanSize4 ? uScanSize4 : uCYHTFScanSize );
	hf_assert(cPlugin);
	hf_assert(lib2);
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
	int i;
	bool bCanOpen = 0;
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
		fileSpec2->Seek( 0, STREAM_SEEK_SET, 0 );//[offset,origin, *Offset]
		CMyComPtr<IInStream> file3( fileSpec2 );
		const UInt64 uScanSize2 = uScanSize4;//uCYHTFScanSize;
		if( (*archive4).Open( file3, &uScanSize2, 0 ) == S_OK ){
			wcxi_DebugString(HfArgs("NOTE: open ok as '%1'").arg((**c).name2.c_str()).c_str() );
			bCanOpen = 1;
			//
			sLastCyhtf.strArcFn  = szFileName;
			sLastCyhtf.strHnName = (**c).name2.c_str();
			sLastCyhtf.hdlr2     = &(**c);
			if(ouHdlr)
				*ouHdlr = &(**c);
			break;
		}
		//(*archive4).Close();
	}
	fileSpec2->Release();
	return bCanOpen;
}
