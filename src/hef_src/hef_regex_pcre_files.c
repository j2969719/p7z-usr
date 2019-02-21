
/*
	Pupose of this file is to have all PCRE C-sources compiled
	in one file.
*/


//#define HAVE_CONFIG_H 1
#define PCRE_STATIC
#define SUPPORT_UTF8
// // disable debug for PCRE. If DEBUG define is present, PCRE prints many texts to STDOUT.
#ifdef DEBUG
#undef DEBUG
#endif

#include "../libraries/pcre_minimal_rev2/pcre_chartables.c"
#include "../libraries/pcre_minimal_rev2/pcre_compile.c"
#include "../libraries/pcre_minimal_rev2/pcre_exec.c"
#include "../libraries/pcre_minimal_rev2/pcre_fullinfo.c"
#include "../libraries/pcre_minimal_rev2/pcre_globals.c"
#include "../libraries/pcre_minimal_rev2/pcre_newline.c"
#include "../libraries/pcre_minimal_rev2/pcre_ord2utf8.c"
#include "../libraries/pcre_minimal_rev2/pcre_study.c"
#include "../libraries/pcre_minimal_rev2/pcre_tables.c"
#include "../libraries/pcre_minimal_rev2/pcre_ucd.c"
#include "../libraries/pcre_minimal_rev2/pcre_valid_utf8.c"
#include "../libraries/pcre_minimal_rev2/pcre_xclass.c"
#include "../libraries/pcre_minimal_rev2/pcre_version.c"
//#include "../libraries/pcre_minimal_rev2/pcre_byte_order.c"
//#include "../libraries/pcre_minimal_rev2/pcre_config.c"
//#include "../libraries/pcre_minimal_rev2/pcre_dfa_exec.c"
//#include "../libraries/pcre_minimal_rev2/pcre_get.c"
//#include "../libraries/pcre_minimal_rev2/pcre_jit_compile.c"
//#include "../libraries/pcre_minimal_rev2/pcre_maketables.c"
//#include "../libraries/pcre_minimal_rev2/pcre_refcount.c"
//#include "../libraries/pcre_minimal_rev2/pcre_string_utils.c"
