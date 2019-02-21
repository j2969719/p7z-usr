
#include "hef_str_piece.h"
#include <stdio.h>
#include <stdint.h>
namespace hef{
;
/// \cond DOXYGEN_SKIP
;
#ifdef HEF_DEBUG //[Ev0KeAeXoWZD]

void str_piece_general_tester()
{
	printf("\n""str_piece_general_tester()\n");
	{
		printf("\n""TEST_01 HfCStrPiece<T>::trim()\n");
		printf("result: %s\n", HfCStrPiece<char>("_abc_").trim("_").c_str() );
	}
	{
		printf("\n""TEST_02 HfCStrPiece<T>::getEscapedSubString() and getEscapedSubString2()\n");
		HfSGetEscapedSubString<char> opts;
		const char* str1 = "[abc^]def]ghi";
		uint32_t length = HfCStrPiece<char>(str1+1)
			.getEscapedSubString("^","]","",opts);
		printf("result #1 length: %u\n", (unsigned int)length );
		printf("result #2 sub-str: %s\n", HfCStrPiece<char>(str1+1)
			.getEscapedSubString2("^","]","",HfCStrPiece<char>::EGES2F_RETESCAPED)
			.c_str() );
	}
	{
		printf("\n""TEST_03 HfCStrPiece<T>::explode2()\n");
		printf("result: %s\n", HfCStrPiece<char>::implode(
			HfCStrPiece<char>("ab__cd__ef").explode2("__"), "," ).c_str() );
	}
	{
		printf("\n""TEST_04 HfCStrPiece<T>::replace()\n");
		const char* srchs[] = {"__","--",0,};
		const char* replcs[] = {"==","++",0,};
		printf("result: %s\n",
			HfCStrPiece<char>("ab__cd--ef__gh").replace( srchs, replcs ).c_str() );
	}
	{
		printf("\n""TEST_05 HfCStrPiece<T>::split()\n");
		HfCStrPiece<char>::LsStrPiece lsSp = HfCStrPiece<char>("abcdef12345").split(3);
		printf("res1 num sub strings: %u\n", (unsigned int)lsSp.size() );
		std::string str = HfCStrPiece<char>::implode( lsSp, "+" );
		printf("res1 imploded: %s\n", str.c_str() );

		lsSp = HfCStrPiece<char>("abcdef12345").split(-3);
		printf("res2 num sub strings: %u\n", (unsigned int)lsSp.size() );
		str = HfCStrPiece<char>::implode( lsSp, "-" );
		printf("res2 imploded: %s\n", str.c_str() );
	}
	{
		printf("\n""TEST_06 HfCStrPiece<T>::toCommandLineArgv()\n");

		HfCStrPiece<char> sp1("ex.exe -a \"ab cd\" --pro");
		std::vector<HfCStrPiece<char> > ls1 = sp1.toCommandLineArgv();
		std::string str = HfCStrPiece<char>::implode( ls1, "==" );
		printf("rs: %s\n", str.c_str() );

		std::vector<std::basic_string<char> > strarr;
		HfCStrPiece<char>::convSpArrayToStrArray( ls1, strarr );
	}
	{
		// misc tests.
		HfCStrPiece<char> sp;
		sp.pathToPieces( 0 );
		sp.explode3( HfCStrPiece<char>::LsStrPiece() );
	}
}
#endif // HEF_DEBUG //[Ev0KeAeXoWZD]

/// \endcond //DOXYGEN_SKIP
;

} // end namespace hef
