
#ifndef _7Z_WCX3_H_
#define _7Z_WCX3_H_

#include "myWindows/StdAfx.h"
#include "Windows/PropVariant.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Archive/IArchive.h"

#include "hef_file.h"
#include "hef_utf8_str.h"
#include "hef_str_args.h"
#include "hef_assert.h"

#include "7z_wcx2.h"
#include "7z_wcx_cmn.h"

using namespace NWindows;

namespace hef{}
using namespace hef;

class CArcExtractCalb
	: public IArchiveExtractCallback
	, public CMyUnknownImp
	, public ISequentialOutStream // CPP/7zip/IStream.h
{
public:
	CArcExtractCalb( IInArchive* archive6_, const std::vector<SFi>* lsFi_ );
	virtual ~CArcExtractCalb();
	void closeFileIfAny();
	int getError()const {return eError4;}
	// unknown-intrf.
	ULONG AddRef() {return 1;}
	ULONG Release() {return 1;}
	HRESULT QueryInterface( REFIID iid, void** outObject ) {return S_OK;}
	// IProgress
	HRESULT SetTotal( UInt64 total ) {return S_OK;}
	HRESULT SetCompleted( const UInt64* completeValue ) {return S_OK;}
	// IArchiveExtractCallback
	HRESULT GetStream( UInt32 index3, ISequentialOutStream** outStream, Int32 askExtractMode );
	HRESULT PrepareOperation( Int32 askExtractMode ) {return S_OK;}
	HRESULT SetOperationResult( Int32 resultEOperationResult ) {return S_OK;}
	// ISequentialOutStream
	HRESULT Write( const void *data2, UInt32 size2, UInt32* processedSize );
	// ICryptoGetTextPassword
	//STDMETHOD(CryptoGetTextPassword)(BSTR *aPassword);
private:
	IInArchive* archive6;
	uint64_t uFsize, uFSizeDone, uTotalSize, uTotalSizeDone;
	std::string CurFname;
	FILE* fp2;
	const std::vector<SFi>* lsFi;
	int eError4;
	uint32_t uWriStride2;
};

#endif //_7Z_WCX3_H_
