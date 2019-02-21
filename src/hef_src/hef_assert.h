
#ifndef _HEF_ASSERT_H_
#define _HEF_ASSERT_H_
#include <stdio.h>

/// \cond DOXYGEN_SKIP
void _hf_assert_commit2_nons( const char* szAsrStr, const char* szFileName, int lineNr );
/// \endcond //DOXYGEN_SKIP

namespace hef{

/// \file hef_assert.h
///       Custom assertion macro and functions.
///       Parts of code taken from: ...\codeblocks\MinGW\include\assert.h.

struct HfSAssertSetup;

/// Custom assertion macro provided by this library.
/// Some features can be set-up using hf_assert_setup() function durning runtime.
/// Differences with standard assert() macro/function:
/// \li * present in no-debug mode by default.
/// \li * doesn't show full file path by default.
//#define hf_assert(e)       ((e) ? (void)0 : _hf_assert_commit(#e, __FILE__, __LINE__))
#define hf_assert(e)       ((e) ? (void)0 : _hf_assert_commit2_nons(#e, __FILE__, __LINE__))


/// \cond DOXYGEN_SKIP
void _hf_assert_commit( const char* szAsrStr, const char* szFileName, int lineNr );
/// \endcond //DOXYGEN_SKIP
void hf_assert_setup( const HfSAssertSetup& in );

/// Internal struct used when setting up hf_assert() with hf_assert_setup().
struct HfSAssertSetup{
	bool bNoFullPath;
	bool bOnlyInDebugMode;
	int  nExitCode;
	HfSAssertSetup()
		: bNoFullPath(1), bOnlyInDebugMode(0), nExitCode(1300)
	{}
};

} // end namespace hef

#endif //_HEF_ASSERT_H_
