
#ifndef _7Z_WCX_COMM_H_
#define _7Z_WCX_COMM_H_
#include <string>
#include <stdint.h>

namespace hef{}
using namespace hef;

extern bool             bShowDebug;
extern std::string*     DebugLogFile;
extern std::string*     SelfWcxFile;
extern std::string*     Str7zSoFile;
extern char             szP7zWcxFile[128];

void wcxi_DebugString( const char* );
void wcxi_ConvU64To2xU32( uint64_t inp, uint32_t* uLow, uint32_t* uHi );

#endif //_7Z_WCX_COMM_H_
