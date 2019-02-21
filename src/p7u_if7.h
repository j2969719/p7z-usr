
#ifndef P7U_IF7_H_INCLUDED
#define P7U_IF7_H_INCLUDED

//#include <string>
//#include <vector>

#include "myWindows/StdAfx.h"
#include "7zip/Common/FileStreams.h"  // -> "7zip/IStream.h" //IInStream
#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"

#include "p7u_cmn.h"

extern IProcRelay* cProcRelayIntrf;

struct CWCXIInStream
	: public IInStream //, public IUnknown//, public CMyUnknownImp
	, public CMyUnknownImp
{
	MY_UNKNOWN_IMP1(IInStream)
	;            CWCXIInStream( const char* szFilename );
	virtual      ~CWCXIInStream();
	bool         isFPStreamOpened()const {return !!fp2;}
	virtual LONG Read( void* bfr, UInt32 sizeToRead2, UInt32* sizeReaded );
	virtual LONG Seek( Int64 offset2, UInt32 orig2, UInt64* newPosition );
protected:
	virtual LONG SeekLocal( Int64 offset2, UInt32 orig2, UInt64* newPosition, int flags4 );
private:
	FILE* fp2;
	int64_t nnFSize, nnPos;
};


class CArcOpenCalb
	: public IArchiveOpenCallback
	, public ICryptoGetTextPassword
	, public CMyUnknownImp
{
public:
	MY_UNKNOWN_IMP1(ICryptoGetTextPassword)
	;       CArcOpenCalb( const char* szArcFnm_ );
	virtual ~CArcOpenCalb();
	HRESULT SetTotal( const UInt64 *files, const UInt64 *bytes ) {return S_OK;}
	HRESULT SetCompleted( const UInt64 *files, const UInt64 *bytes ) {return S_OK;}
	HRESULT CryptoGetTextPassword( BSTR *aPassword );
public:
	bool bAskedPassword;
private:
	std::string strArcFnm;
};

#endif //P7U_IF7_H_INCLUDED
