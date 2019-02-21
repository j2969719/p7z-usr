
#ifndef _7Z_WCX_UNP_H_
#define _7Z_WCX_UNP_H_

#include "myWindows/StdAfx.h"
#include "Windows/PropVariant.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"

#include "hef_file.h"
#include "hef_utf8_str.h"
#include "hef_str_args.h"
#include "hef_assert.h"

#include "p7u_cls.h"
#include "p7u_cmn.h"

using namespace NWindows;

namespace hef{}
using namespace hef;

//DEFINE_GUID(CLSID_CFormat7z2,
//  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

class CArcExtractCalb
	: public IArchiveExtractCallback
	, public ICryptoGetTextPassword
	, public ISequentialOutStream // CPP/7zip/IStream.h
	, public CMyUnknownImp
{
public:
	MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

	CArcExtractCalb( IInArchive* archive6_, const std::vector<SFi>* lsFi_, wcxi_SOpenedArc* soa_ );
	virtual ~CArcExtractCalb();
	void closeFileIfAny();
	int getError()const {return eError4;}
	const char* ioErrorText( const char* inp = 0 );
	// CMyUnknownImp
	//ULONG AddRef() {return ++__m_RefCount;}
	//ULONG Release() { --__m_RefCount; if(!__m_RefCount){ delete this; return 0;} return __m_RefCount; }
	//HRESULT QueryInterface( REFIID iid, void** outObject ) {*outObject = this;return __m_RefCount;}
	// IProgress
	HRESULT SetTotal( UInt64 total ) {return S_OK;}
	HRESULT SetCompleted( const UInt64* completeValue ) {return S_OK;}
	// IArchiveExtractCallback
	HRESULT GetStream( UInt32 index3, ISequentialOutStream** outStream, Int32 askExtractMode );
	HRESULT PrepareOperation( Int32 askExtractMode ) {return S_OK;}
	HRESULT SetOperationResult( Int32 resultEOperationResult ) { return S_OK;}
	// ISequentialOutStream
	HRESULT Write( const void *data2, UInt32 size2, UInt32* processedSize );
	// ICryptoGetTextPassword
	//STDMETHOD(CryptoGetTextPassword)(BSTR *aPassword);
	HRESULT CryptoGetTextPassword( BSTR *aPassword );
private:
	IInArchive* archive6;
	uint64_t uFsize, uFSizeDone, uTotalSize, uTotalSizeDone;
	std::string CurFname, strErrText;
	FILE* fp2;
	const std::vector<SFi>* lsFi;
	int eError4;
	uint32_t uWriStride2;
	wcxi_SOpenedArc* soa4;
};

#endif //_7Z_WCX_UNP_H_
