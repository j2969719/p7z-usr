
#include "p7u_if7.h"
#include "hef_str_args.h"
#include "hef_assert.h"
#include "hef_utf8_str.h"
#include "hef_file.h"

static bool CWCXIInStream_bCountObj = 0;
static int  CWCXIInStream_nCountObj = 0;
CWCXIInStream::CWCXIInStream( const char* szFilename )
	: fp2(0), nnFSize(0), nnPos(0)
{
	if( CWCXIInStream_bCountObj ){
		wcxi_DebugString( HfArgs("CWCXIInStream() %1").arg(++CWCXIInStream_nCountObj).c_str() );
	}
	fp2 = fopen( szFilename, "rb" );
	if(fp2){
		//fseek( fp2, 0, SEEK_END );
		//nnFSize = ftell( fp2 );
		//fseek( fp2, 0, SEEK_SET );
		uint64_t uuSize = hf_GetFileSize64( fp2 );
		nnFSize = int64_t( std::min<uint64_t>( uuSize, 0x7FFFFFFFFFFFFFFF ) );
		hf_Seek64( fp2, 0 );
	}
}
CWCXIInStream::~CWCXIInStream()
{
	if(fp2){
		fclose(fp2);
		fp2 = 0;
	}
	if( CWCXIInStream_bCountObj ){
		wcxi_DebugString( HfArgs("~CWCXIInStream() %1").arg(--CWCXIInStream_nCountObj).c_str() );
	}
}
// STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) PURE;
// STREAM_SEEK_SET = 0, STREAM_SEEK_CUR = 1, STREAM_SEEK_END = 2
// fseek(): SEEK_SET, SEEK_CUR, SEEK_END
LONG CWCXIInStream::Seek( Int64 offset2, UInt32 orig2, UInt64* newPosition )
{
	return SeekLocal( offset2, orig2, newPosition, 0x1 );
}
LONG CWCXIInStream::
SeekLocal( Int64 offset2, UInt32 orig2, UInt64* newPosition, int flags4 )
{
	//static int nCntSeek = 0;
	//nCntSeek++;
	Int64 offset3 = offset2;
	UInt64 dmy2, *newPosition2 = ( newPosition ? newPosition : &dmy2 );
	*newPosition2 = nnPos;
	//if( flags4 & 0x1 )
	//	printf("x::Seek/%d( %lld, %s, ) cur:%ld\n", nCntSeek, offset2, szOrg, nnPos );//*/
	if(!fp2){
		return E_FAIL;
	}
	if( orig2 == STREAM_SEEK_CUR ){
		offset3 += nnPos;
	}else if( orig2 == STREAM_SEEK_END ){
		offset3 += nnFSize;
	}
	if( offset3 < 0 ){
		return E_FAIL;
	}
	bool bOk = hf_Seek64( fp2, uint64_t(offset3) );  // !fseek( ,, SEEK_SET );
	if( bOk ){
		*newPosition2 = offset3;
		nnPos = offset3;
	}else{
		// nnPos = ftell(fp2);
		// *newPosition2 = nnPos;
	}
	//if( flags4 & 0x1 )
	//	printf("x::Seek/%d( %lld )->%d\n", nCntSeek, offset3, bOk );//*/
	return ( bOk ? S_OK : E_FAIL );
}
// STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize) PURE;
LONG CWCXIInStream::Read( void* bfr, UInt32 sizeToRead2, UInt32* processedSize2 )
{
	//static int nCntRead = 0;
	//nCntRead++;
	//printf("x::Read/%d( %u, ) cur:%ld\n", nCntRead, sizeToRead2, nnPos );
	UInt32 dmy0, *processedSize3 = ( processedSize2 ? processedSize2 : &dmy0 ), sizeToRead3 = sizeToRead2;
	*processedSize3 = 0;
	if(!fp2)
		return E_FAIL;
	if( !sizeToRead3 || nnPos > nnFSize ){
		return S_OK;
	}
	if( sizeToRead3 + nnPos > nnFSize )
		sizeToRead3 = ( std::max<int64_t>( 0, nnFSize - nnPos ) );
	if( !sizeToRead3 ){
		return S_OK;
	}
	bool bOk = 0;
	int64_t numreadd = fread( bfr, 1, sizeToRead3, fp2 );
	if( numreadd ){
		bOk = 1;
		*processedSize3 = numreadd; //sizeToRead3;
		SeekLocal( numreadd, STREAM_SEEK_CUR, 0, 0x0 );
	}
	//printf("x::Read/%d( nr:%ld, ps:%u, )->%d\n", nCntRead, numreadd, *processedSize3, bOk );
	return ( bOk ? S_OK : E_FAIL );
}

static int CArcOpenCalb_bRefCnt = 0;
static int CArcOpenCalb_nRefCnt = 0;
CArcOpenCalb::CArcOpenCalb( const char* szArcFnm_ )
	: bAskedPassword(0)
	, strArcFnm(szArcFnm_)
{
	if( CArcOpenCalb_bRefCnt )
		wcxi_DebugString( HfArgs("CArcOpenCalb() %1").arg( ++CArcOpenCalb_nRefCnt ).c_str() );
}
CArcOpenCalb::~CArcOpenCalb()
{
	if( CArcOpenCalb_bRefCnt )
		wcxi_DebugString( HfArgs("~CArcOpenCalb() %1").arg( --CArcOpenCalb_nRefCnt ).c_str() );
}

// ICryptoGetTextPassword
//STDMETHOD (CryptoGetTextPassword)( BSTR *aPassword )
HRESULT CArcOpenCalb::CryptoGetTextPassword( BSTR *aPassword )
{
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
