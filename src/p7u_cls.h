
#ifndef _P7U_CLS_H_
#define _P7U_CLS_H_
#include <string>
#include <vector>
#include <stdint.h>
#include "p7u_cmn.h"
//#include "7zip/IStream.h" //IInStream

namespace hef{}
using namespace hef;

/// structure for wcxi_OpenArchive() calls.
struct wcxi_OpenArchiveData{ //tOpenArchiveData
	const char* szArcName;		///< utf-8 archive file name.
	bool        bListingOnly;	///< corresponds to either PK_OM_LIST or PK_OM_EXTRACT.
	/// output value on error, ie. when wcxi_OpenArchive() returns 0, fe. E_EOPEN.
	int         eOpenResult;
};

/// Structure for readHeaderEx() calls, the enumeration of the files in the archive.
struct wcxi_HeaderDataEx{
	std::string strFileName;
	int         nFileAttr;			//0x10 - dir
	uint64_t    uPackSize;
	uint64_t    uUnpSize;
	/// \verbatim
	///     FileTime contains the date and the time of the file’s last update.
	///     Use the following algorithm to set the value:
	///     FileTime = (year - 1980) << 25 | month << 21 | day << 16 | hour << 11 | minute << 5 | second/2;
	///     Make sure that:
	///     year is in the four digit format between 1980 and 2100. month is a number between 1 and 12. hour is in the 24 hour format.
	/// \endverbatim
	int         nFileTime;
	int         eError;
	wcxi_HeaderDataEx() : nFileAttr(0), uPackSize(0), uUnpSize(0), nFileTime(0), eError(0) {}
};
enum{
	// these must have their WCX interface equivalents.
	WCXI_ENoMoreFiles = 10,     //E_END_ARCHIVE
	WCXI_EBadData = 12,	        //E_BAD_DATA
	WCXI_EUnknownFormat = 14,   //E_UNKNOWN_FORMAT
	WCXI_EOpen = 15,            //E_EOPEN
	WCXI_ENotSupported = 24,    //E_NOT_SUPPORTED=24
	WCXI_EWrite = 19,           //E_EWRITE
	WCXI_EUserCancel = 21,      //E_EABORTED
};
enum{
	// 0x10 equals to WIN32|WINAPI flag FILE_ATTRIBUTE_DIRECTORY (alias FA_DIRECTORY).
	WCXI_FADir = 0x10,
};
// CPP/7zip/UI/Common/LoadCodecs.cpp
struct SArcHandler{
	std::vector<uint8_t>     guid2;//, sig;
	std::string              name2;
	std::vector<std::string> exts, extsAdd;
	uint32_t                 flags2;
	//
	struct ByName{ const char* name3;
		ByName( const char* name_ ) : name3(name_) {}
		bool operator()( const SArcHandler& oth )const {return oth.name2 == name3;}
	};
};
struct SFi{
	std::string strDstFnmi, strSrcFnmi;
	uint32_t    uCFItem;
	uint64_t    uFsizei;
	uint64_t    uMTimeMs;  ///< file modification time. miliseconds since 1970-Jan-1. 0 = invalid time.
	SFi() : uCFItem(0), uFsizei(0), uMTimeMs(0) {}
	//
	struct CmpByItem{
		bool operator()( const SFi& a, const SFi& b )const {return a.uCFItem < b.uCFItem;}
	};
	struct ByItem{
		ByItem( uint32_t uCFItem2_ ) : uCFItem2(uCFItem2_) {}
		uint32_t uCFItem2;
		bool operator()( const SFi& otherr )const{
			return uCFItem2 == otherr.uCFItem;
		}
	};
};
struct wcxi_SOpenedArc{
	uint32_t           ident, verify2;
	std::string        strArcName;
	uint32_t           uNumItems;
	void*              archive3; //CMyComPtr<IInArchive>*
	uint32_t           uNextItem;
	std::string        strCurFile;
	uint32_t           uCurItem;
	const SArcHandler* hdlr3;
	std::vector<SFi>   lsExtr2;
	wcxi_SOpenedArc() : ident(0), verify2(0), uNumItems(0), archive3(0), uNextItem(0), uCurItem(0), hdlr3(0) {}
	bool isMailformed()const {return ident != ~verify2;}
};

//#ifndef P7ZUSR_INT_MAIN
	void fnInit_p7zUsr() __attribute__((constructor));
	void fnDeinit() __attribute__((destructor));
//#else
//	void fnInit_p7zUsr();
//	void fnDeinit();
//#endif

std::string wcxi_readlink( const char* szPath, bool bEmptyOnFail );
std::string wcxi_StripPostTarEtcExts( const char* inp, const SArcHandler* hdlr4 );

class WcxiPlugin
{
public:
	WcxiPlugin( const char* szIni );
	virtual ~WcxiPlugin() {}
	void* openArchive( wcxi_OpenArchiveData& acd );
	bool  closeArchive( void* hArcData, int* err );
	bool  canYouHandleThisFile( const char* FileName, const SArcHandler** ouHdlr, uint64_t uScanSize4, const char* szExt );
	bool  readHeaderEx( void* hArcData, wcxi_HeaderDataEx& shd );
	bool  extractFile2( wcxi_SOpenedArc* soa, uint32_t uCItm, const char* szDstFn );
//	const char* getDcMsgBoxSymbol()const {return strMsgBoxSmn.c_str();}
//	const char* getDcPromptBoxSymbol()const {return strPromptBoxSmn.c_str();}
	std::string getP7ZIPHeadersVersion()const;
	std::string getINIConfigFileName()const {return strIni2;}
	bool isDisableFileDatesSet()const {return bDisableFileDates;}
private:
	void updateArcHandlers();
	void clearArchiveStruct( wcxi_SOpenedArc& inp );
	std::vector<SArcHandler*> getHandlersForExt( const char* szExt );
	bool extractMultipleFiles( wcxi_SOpenedArc* soa, int* eError3, std::string* err2 );
	uint64_t getUpdateExtrFileSizesEtc( wcxi_SOpenedArc* soa, std::vector<uint32_t>* indexesOu )const;
private:
	struct SCyhtf{
		std::string strArcFn, strHnName;
		const SArcHandler* hdlr2;
		SCyhtf() : hdlr2(0) {}
		void clear2();
	};
	std::vector<SArcHandler> hndlrs2;
	SCyhtf                   sLastCyhtf;
	uint32_t                 uArcLastIdent;
	std::string              strCYHTFHandlersOff, strIni2, strArcOpenShellNotify;
	std::vector<std::string> lsCYHTFHandlersOff;
	uint64_t                 uScanSize, uCYHTFScanSize;
	bool                     bDoAccessViolationOnCAErr, bDisableFileDates;
//	std::string              strPromptBoxSmn, strMsgBoxSmn;
};
extern WcxiPlugin* cPlugin;
extern IProcRelay* cProcRelayIntrf;



#endif //_P7U_CLS_H_
