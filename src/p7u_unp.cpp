
#include "p7u_unp.h"

CArcExtractCalb::
CArcExtractCalb( IInArchive* archive6_, const std::vector<SFi>* lsFi_, wcxi_SOpenedArc* soa_ )
	: archive6(archive6_), uFsize(0), uFSizeDone(0), uTotalSize(0), uTotalSizeDone(0)
	, fp2(0), lsFi(lsFi_), eError4(0), nNumFtimeFails(0), uWriStride2(0), soa4(soa_)
	, CurFi(0)
{
	++uGlobArcExcCalbRefc;
	wcxi_DebugString(HfArgs("uGlobArcExcCalbRefc: %1").arg( uGlobArcExcCalbRefc ).c_str() );
	std::vector<SFi>::const_iterator a;
	for( a = lsFi->begin(); a != lsFi->end(); ++a ){
		uTotalSize += a->uFsizei;
	}
}
CArcExtractCalb::~CArcExtractCalb()
{
	closeFileIfAny();
	--uGlobArcExcCalbRefc;
	wcxi_DebugString(HfArgs("~uGlobArcExcCalbRefc: %1").arg( uGlobArcExcCalbRefc ).c_str() );
}
void CArcExtractCalb::closeFileIfAny()
{
	if(fp2){
		fclose( fp2 );
		fp2 = 0;
		if( CurFi ){
			if( !cPlugin->isDisableFileDatesSet() ){ // INI.bDisableFiletime
				if( CurFi->uMTimeMs && !nNumFtimeFails ){
					if( !wcxi_SetFileMTimeFromUnixTime1970Ms( CurFi->strDstFnmi.c_str(), CurFi->uMTimeMs ) )
						nNumFtimeFails++;
				}
			}
			CurFi = 0;
		}
	}
}
uint64_t WcxiPlugin::
getUpdateExtrFileSizesEtc( wcxi_SOpenedArc* soa, std::vector<uint32_t>* indexesOu )const
{
	CMyComPtr<IInArchive>* archive2 = reinterpret_cast<CMyComPtr<IInArchive>*>(soa->archive3);
	//hf_assert( archive2 );
	if(!archive2){
		return 0;
	}
	std::vector<SFi>::iterator a;
	NCOM::CPropVariant prop; uint64_t uSizeSum = 0;
	std::sort( soa->lsExtr2.begin(), soa->lsExtr2.end(), SFi::CmpByItem() );
	for( a = soa->lsExtr2.begin(); a != soa->lsExtr2.end(); ++a ){
		prop.Clear();
		(**archive2).GetProperty( a->uCFItem, kpidSize, &prop );
		a->uFsizei = ( prop.vt == VT_UI8 ? prop.uhVal.QuadPart : 0 );
		uSizeSum += a->uFsizei;
		if( !cPlugin->isDisableFileDatesSet() ){
			prop.Clear();
			(**archive2).GetProperty( a->uCFItem, kpidMTime, &prop );
			if( prop.vt == VT_FILETIME ){
				uint64_t nano100thSecs = wcxi_Conv2xU32ToU64( prop.filetime.dwLowDateTime, prop.filetime.dwHighDateTime );
				int64_t x = hf_convWinFiletime1601ToUnix1970ms( nano100thSecs );
				a->uMTimeMs = std::max<int64_t>( x, 0 );
			}
		}
		indexesOu->push_back( a->uCFItem );
	}
	return uSizeSum;
}
HRESULT CArcExtractCalb::
GetStream( UInt32 index3, ISequentialOutStream** outStream, Int32 askExtractMode )
{
	hf_assert(archive6);
	if( askExtractMode != NArchive::NExtract::NAskMode::kExtract )
		return S_OK;
	closeFileIfAny();
	std::vector<SFi>::const_iterator a;
	a = std::find_if( lsFi->begin(), lsFi->end(), SFi::ByItem(index3) );
	hf_assert( a != lsFi->end() );
	if( hf_FileExists( a->strDstFnmi.c_str() ) ){
		eError4 = WCXI_EOpen;
		strErrText = HfArgs("File already exists [%1].").arg( hf_basename(a->strDstFnmi.c_str()) ).c_str();
		return E_FAIL;
	}
	hf_assert( !fp2 );
	fp2 = fopen( a->strDstFnmi.c_str(), "a+b" );//wb,"a+b"
	if(!fp2){
		eError4 = WCXI_EOpen;
		strErrText = HfArgs("File open failed [%1].").arg( hf_basename(a->strDstFnmi.c_str()) ).c_str();
		return E_FAIL;
	}
	uFsize = a->uFsizei;
	uFSizeDone = 0;
	CurFname = a->strSrcFnmi;
	CurFi = &*a;  //SFi*
	*outStream = this;
	return S_OK;
}
HRESULT CArcExtractCalb::
Write( const void *data2, UInt32 size2, UInt32* processedSize )
{
	hf_assert(fp2);
	const UInt32 nStride = ( !uWriStride2 ? size2 : uWriStride2 ); //65536
	UInt32 nRem = size2, nWri, nWri2, uOfs = 0;
	for( ; nRem; nRem -= nWri, uOfs += nWri ){
		const uint8_t* data3 = ( (uint8_t*)data2 ) + uOfs;
		nWri = std::min( nStride, nRem );
		//fseek( fp2, 0, SEEK_END );
		nWri2 = fwrite( data3, 1, nWri, fp2 );
		if( nWri != nWri2 ){
			eError4 = WCXI_EWrite; // E_EWRITE
			strErrText = HfArgs("File write failed [%1].").arg( hf_basename(CurFname.c_str()) ).c_str();
			return E_FAIL;
		}
		uFSizeDone += nWri;
		float fPerc = ( uFsize ? (float)( (double)uFSizeDone / uFsize ) : 0.33f );
		if( !cProcRelayIntrf->iprSetFilePercentage( CurFname.c_str(), fPerc ) ){
			eError4 = WCXI_EUserCancel; // E_EABORTED
			strErrText = "User cancel.";//HfArgs("User cancel.").arg( hf_basename(CurFname.c_str()) ).c_str();
			return E_FAIL;
		}
		uTotalSizeDone += nWri;
		fPerc = ( uTotalSize ? (float)( (double)uTotalSizeDone / uTotalSize ) : 0.22f );
		cProcRelayIntrf->iprSetTotalPercentage( CurFname.c_str(), fPerc );
	}
	*processedSize = size2;
	return S_OK;
}
HRESULT CArcExtractCalb::CryptoGetTextPassword( BSTR *aPassword )
{
	wcxi_DebugString( HfArgs("CArcExtractCalb::CryptoGetTextPassword().").c_str() );

	hf_assert( cProcRelayIntrf );
	std::string pw2; std::basic_string<wchar_t> pw3; // wchar_t <=> WCHAR
	if( cProcRelayIntrf->iprTryGetOrAskArcFNPassword( soa4->strArcName.c_str(), &pw2 ) )
		hf_Utf8DecodeToAny( pw2.c_str(), -1, pw3, 0 );

	//IProcRelay::SMsgBox smb;
	//smb.capt = "abc";
	//pw2 = std::string("------ password: ") + pw2.c_str();
	//smb.msg = pw2.c_str();
	//cProcRelayIntrf->iprMessageBox(smb);

	HRESULT res = StringToBstr( UString(pw3.c_str()), aPassword );
	{
		std::basic_string<wchar_t>::iterator a;
		for( a = pw3.begin(); a != pw3.end(); ++a )
			*a = 'x';
		pw3.clear();
	}
	return res;
}
bool WcxiPlugin::extractMultipleFiles( wcxi_SOpenedArc* soa, int* eError3, std::string* err2 )
{
	wcxi_DebugString(HfArgs("extractMultipleFiles(), num:%1")
		.arg( (int)soa->lsExtr2.size() )
		.c_str() );
	std::vector<UInt32> idxsOrd;
	getUpdateExtrFileSizesEtc( soa, &idxsOrd );
	// x::Extract(const UInt32* indices, UInt32 numItems,
	//     Int32 testMode, IArchiveExtractCallback *extractCallback)
	CMyComPtr<IInArchive>* archive5 = (CMyComPtr<IInArchive>*)soa->archive3;
	//hf_assert( archive5 );
	if(!archive5){
		*eError3 = WCXI_EBadData;
		*err2    = "Bad internal archive structure.";
		return 0;
	}
	CArcExtractCalb* cAec = new CArcExtractCalb( *archive5, &soa->lsExtr2, soa );
	// fix for the unknown-interface, cryptic... no way to deal with it...
	for( size_t i=0; i < idxsOrd.size(); i++ )
		cAec->AddRef();
	cAec->AddRef();
	CMyComPtr<IArchiveExtractCallback> cAecCom( cAec );
	HRESULT rs2; bool bSucc = 1;
	rs2 = (**archive5).Extract( &idxsOrd[0], (UInt32)idxsOrd.size(), 0, cAecCom );
	if( rs2 != S_OK ){
		// WCXI_EBadData = E_BAD_DATA // WCXI_EUserCancel = E_EABORTED
		*eError3 = cAec->getError();
		*eError3 = ( !(*eError3) ? WCXI_EWrite : *eError3 );
		*err2 = cAec->ioErrorText();
		bSucc = 0;
	}
	ULONG num = cAec->Release();
	for(; num > 1; num-- )
		cAec->Release();
	//delete cAec;
	return bSucc;
}
const char* CArcExtractCalb::ioErrorText( const char* inp )
{
	if(inp)
		strErrText = inp;
	return strErrText.c_str();
}







