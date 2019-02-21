
#include "hef_str_args.h"
#include <stdio.h>
namespace hef{
;
/// \cond DOXYGEN_SKIP //{

#ifdef HEF_DEBUG //[L53VwxZbr4nZ]

void str_args_general_tester()
{
	uint64_t uuVal = 5111222333; int iVal = -301;
	std::string str = HfTArgs<char>(
		"Str val: %1, Str val2: %2, iVal: %3, _U64: %4, _S64: %5, float: %6")
		.arg("abc")
		.arg("def")
		.arg( iVal )
		.arg_u64( uuVal, 10, 0,0,0,"" )
		.arg_s64( (int64_t)4445, 10, 0, '0', 0, "" )
		.arg_f32( 1.1f )
		.c_str();
	;
	printf("HfTArgs: [%s]\n", str.c_str() );
}

#endif // [L53VwxZbr4nZ]

/// \endcond //DOXYGEN_SKIP //}
;
/*std::string _hf_strprint_float( float a )
{
	char bfr[128];
	sprintf( bfr, "%f", a );
	return bfr;
}//*/


std::string _hf_sprintfFloat2( float fVal, int nDec, int nFrac )
{
	char bfrFrac[32]="", bfrFmt[52], bfr2[64];
	if( nFrac >= 0 )
		sprintf( bfrFrac, ".%d", nFrac );
	sprintf( bfrFmt, "%%%sf", bfrFrac );
	sprintf( bfr2, bfrFmt, fVal );
	if( nDec >= 0 ){
		int iDcmpt = atoi( bfr2 );
		const char* szFlt = hf_strchr(bfr2,'.');
		szFlt = szFlt ? szFlt+1 : "";
		sprintf( bfr2, "%s", ( HfArgs("%1.%2")
				.arg( HfArgs("%1").arg_s64((int64_t)iDcmpt,10,nDec,'0',0,"").c_str() )
				.arg(szFlt)
				.c_str() ) );
	}
	return bfr2;
}

} // end namespace hef
