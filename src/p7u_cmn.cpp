
#include "p7u_cmn.h"
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
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


