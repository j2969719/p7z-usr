
#include "p7u_cmn.h"

#include <stdio.h>
#ifdef __GNUC__
	#include <dlfcn.h>
	#include <unistd.h>
	//#include <sys/types.h>
	//#include <utime.h> //utime()
	#include <sys/time.h> //utimes()
#endif //__GNUC__
#include <stdlib.h>
#include "hef_str.h"
#include "hef_data.h"
#include "hef_assert.h"


char             szP7zWcxFile[128] = "p7z_usr.wcx";
bool             bShowDebug = 1;
bool             bShowDebugIsEnv = 0;
//char           szDirCharABfr[2] = { '\\', 0, }; // '\\' => 0x5C
std::string*     DebugLogFile = 0;
std::string*     SelfWcxFile = 0;
std::string*     Str7zSoFile = 0;
int              uGlobArcExcCalbRefc = 0;

/// Debug message.
/// inp    - input message.
/// flags2 - flags, fe. WCXI_DSShowAlways.
void wcxi_DebugString( const char* inp, int flags2 )
{
	if( bShowDebug || (flags2 & WCXI_DSShowAlways) ){
		printf("%s: %s\n", szP7zWcxFile, inp );
		if( DebugLogFile && !DebugLogFile->empty() ){
			std::string str;
			str = hf_strftime2( "%H:%M:%S ", 0 );
			str += inp;
			str += "\n";
			hf_PutFileBytes( DebugLogFile->c_str(), str.c_str(), -1, HF_EPFBF_APPEND );
		}
		if( flags2 & WCXI_DSMsgBox ){
			int fl2 = 0;
			if( hf_strBeginsWith( inp, -1, "ERROR", -1, 0 ) )
				fl2 |= 0x10;
			std::string msg = inp;
			if( flags2 & WCXI_DSAddExitNowMBText ){
				msg += "\n\n""Application will now terminate.";
			}
			wcxi_MessageBox3( szP7zWcxFile, msg.c_str(), fl2 );
		}
	}
}
void wcxi_ConvU64To2xU32( uint64_t inp, uint32_t* uLow, uint32_t* uHi )
{
	*uLow = (uint32_t)( inp & 0xFFFFFFFF );
	*uHi = (uint32_t)( (inp >> 32) & 0xFFFFFFFF );
}
uint64_t wcxi_Conv2xU32ToU64( uint32_t uLow, uint32_t uHi )
{
	uint64_t outp = 0;
	outp |= ((uint64_t)uHi) << 32;
	outp |= uLow;
	return outp;
}
void wcxi_InitMsgBoxFunctionOnSOLoad()
{
	//hf_assert( !fncDcMessageBoxProc );
	if( !fncDcMessageBoxProc )
		fncDcMessageBoxProc = (DcMessageBoxProc_t)dlsym( RTLD_DEFAULT, "FDIALOGBOX_MESSAGEBOX$PCHAR$PCHAR$LONGINT$$LONGINT" );

	//cPlugin->getDcMsgBoxSymbol();
}
bool wcxi_MessageBox3( const char* capt, const char* msg, int flags2 )
{
	if( fncDcMessageBoxProc );
		return !!fncDcMessageBoxProc( msg, capt, flags2, 0 );
}
/**
	Converts unix timestamp to TCMD WCX 'tHeaderData::Filetime'.
	Output is meant to be compatible with member 'Filetime' of
	the 'tHeaderData' structure of the TCMD WCX interface.

	NOTE: On DCMD on Linux, 'Filetime' integer is an actual unix timestamp,
	      and not 1980-based integer bitmask as the original doc says.
	      Thus, input value to this function is returned unconverted.
*/
int wcxi_convUnixTime1970ToTCMDTime1980( time_t inp )
{
	#ifdef __GNUC__
		// On DCMD on Linux, tHeaderData::Filetime integer is actual unix timestamp.
		return inp;
	#else
		#error "Non-Linux platforms are not implemented."
		hf_assert(!"Not implemented.");
		return 0;
	#endif
}
int wcxi_convWinFiletime1601ToTCMDTime1980( uint64_t uuLowWordOrBoth, uint32_t uHiWordIfAny )
{
	uuLowWordOrBoth |= ( uint64_t(uHiWordIfAny) << 32 );
	uint64_t nano100thSecs = uuLowWordOrBoth;
	int64_t tm2 = hf_convWinFiletime1601ToUnix1970ms( nano100thSecs );
	int64_t tm3 = std::min<int64_t>( 0x7FFFFFFF, std::max<int64_t>( tm2 / 1000, 1 ) );
	time_t tm4 = static_cast<time_t>(tm3);
	int retv = wcxi_convUnixTime1970ToTCMDTime1980( tm4 );
	return retv;
}

/// Sets "modtime" and "actime' of file specified as path by 'szFilename' parameter.
/// Input 'ftimeMs' is expected to be unix timestamp in milliseconds since 1970-Jan-1.
bool wcxi_SetFileMTimeFromUnixTime1970Ms( const char* szFilename, uint64_t ftimeMs )
{
	// utime(), utimes();
	// ref: * https://stackoverflow.com/questions/651652/how-do-i-write-file-modification-dates-programmatically-in-posix
	//      * man utime
	if( ftimeMs > 0x7FFFFFFFFFFFFFFF )
		return 0;
	struct timeval tvx[2];
	tvx[0].tv_sec  = ftimeMs / 1000;
	tvx[0].tv_usec = (ftimeMs % 1000) * 1000;
	tvx[1] = tvx[0];
	bool rs2 = !utimes( szFilename, tvx );
	return rs2;
}
