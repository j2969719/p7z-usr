
#include "7z_wcx3.h"

CArcExtractCalb::CArcExtractCalb( IInArchive* archive6_, const std::vector<SFi>* lsFi_ )
	: archive6(archive6_), uFsize(0), uFSizeDone(0), uTotalSize(0), uTotalSizeDone(0)
	, fp2(0), lsFi(lsFi_), eError4(0), uWriStride2(0)
{
	std::vector<SFi>::const_iterator a;
	for( a = lsFi->begin(); a != lsFi->end(); ++a ){
		uTotalSize += a->uFsizei;
	}
}
CArcExtractCalb::~CArcExtractCalb()
{
	closeFileIfAny();
}
void CArcExtractCalb::closeFileIfAny()
{
	if(fp2){
		fclose( fp2 );
		fp2 = 0;
	}
}
uint64_t WcxiPlugin::getUpdateExtrFileSizes( wcxi_SOpenedArc* soa, std::vector<uint32_t>* indexesOu )const
{
	CMyComPtr<IInArchive>* archive2 = (CMyComPtr<IInArchive>*)soa->archive3;
	hf_assert( archive2 );
	std::vector<SFi>::iterator a;
	NCOM::CPropVariant prop; uint64_t uSizeSum = 0;
	std::sort( soa->lsExtr2.begin(), soa->lsExtr2.end(), SFi::CmpByItem() );
	for( a = soa->lsExtr2.begin(); a != soa->lsExtr2.end(); ++a ){
		prop.Clear();
		(**archive2).GetProperty( a->uCFItem, kpidSize, &prop );
		a->uFsizei = ( prop.vt == VT_UI8 ? prop.uhVal.QuadPart : 0 );
		uSizeSum += a->uFsizei;
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
		return E_FAIL;
	}
	hf_assert( !fp2 );
	fp2 = fopen( a->strDstFnmi.c_str(), "a+b" );//wb,"a+b"
	if(!fp2){
		eError4 = WCXI_EOpen;
		return E_FAIL;
	}
	uFsize = a->uFsizei;
	uFSizeDone = 0;
	CurFname = a->strSrcFnmi;
	*outStream = this;
	return S_OK;
}
HRESULT CArcExtractCalb::Write( const void *data2, UInt32 size2, UInt32* processedSize )
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
			return E_FAIL;
		}
		uFSizeDone += nWri;
		float fPerc = ( uFsize ? (float)( (double)uFSizeDone / uFsize ) : 0.33f );
		if( !cProcRelayIntrf->iprSetFilePercentage( CurFname.c_str(), fPerc ) ){
			eError4 = WCXI_EUserCancel; // E_EABORTED
			return E_FAIL;
		}
		uTotalSizeDone += nWri;
		fPerc = ( uTotalSize ? (float)( (double)uTotalSizeDone / uTotalSize ) : 0.22f );
		cProcRelayIntrf->iprSetTotalPercentage( CurFname.c_str(), fPerc );
	}
	*processedSize = size2;
	return S_OK;
}
bool WcxiPlugin::extractMultipleFiles( wcxi_SOpenedArc* soa, int* eError3 )
{
	wcxi_DebugString(HfArgs("extractMultipleFiles(), num:%1")
		.arg( (int)soa->lsExtr2.size() )
		.c_str() );
	std::vector<UInt32> idxsOrd;
	getUpdateExtrFileSizes( soa, &idxsOrd );
	// x::Extract(const UInt32* indices, UInt32 numItems,
	//     Int32 testMode, IArchiveExtractCallback *extractCallback)
	CMyComPtr<IInArchive>* archive5 = (CMyComPtr<IInArchive>*)soa->archive3;
	hf_assert( archive5 );
	CArcExtractCalb cAec( *archive5, &soa->lsExtr2 );
	HRESULT rs2;
	rs2 = (**archive5).Extract( &idxsOrd[0], (UInt32)idxsOrd.size(), 0, &cAec );
	if( rs2 != S_OK ){
		// WCXI_EBadData = E_BAD_DATA // WCXI_EUserCancel = E_EABORTED
		*eError3 = cAec.getError();
		*eError3 = ( !(*eError3) ? WCXI_EWrite : *eError3 );
		return 0;
	}
	return 1;
}




