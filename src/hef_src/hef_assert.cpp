
#include "hef_assert.h"
#include <cstdlib>
#include "hef_str.h"

/// \cond DOXYGEN_SKIP
void _hf_assert_commit2_nons( const char* szAsrStr, const char* szFileName, int lineNr )
{
	hef::_hf_assert_commit( szAsrStr, szFileName, lineNr );
}
/// \endcond //DOXYGEN_SKIP

namespace hef{

/// \cond DOXYGEN_SKIP

HfSAssertSetup sHfAssertOpts;

// Internal function that prints message and calls exit().
void _hf_assert_commit( const char* szAsrStr, const char* szFileName, int lineNr )
{
	if( sHfAssertOpts.bOnlyInDebugMode ){
		bool bCancel = 0;
		#if defined(DEBUG) || defined(_DEBUG)
			bCancel = 0;
		#else
			bCancel = 1;
		#endif
		if(bCancel)
			return;
	}
	std::string strFn = szFileName?szFileName:"";
	if(sHfAssertOpts.bNoFullPath)
		strFn = hf_basename(strFn.c_str());

	printf("Assertion Failed: %s, file %s, line %d.\n",
		szAsrStr,
		!strFn.empty()?strFn.c_str():"<unknown-source-file>",
		lineNr );
	exit(sHfAssertOpts.nExitCode);
}
/// \endcond //DOXYGEN_SKIP

/// Sets global options for hf_assert() that persists through process instance.
void hf_assert_setup( const HfSAssertSetup& in )
{
	sHfAssertOpts = in;
}

} // end namespace hef
