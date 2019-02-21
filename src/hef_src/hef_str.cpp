
#include "hef_str.h"
#include <stdio.h>
namespace hef{
;
/// \cond DOXYGEN_SKIP //{
;
#ifdef HEF_DEBUG //[HpthJFb85u4u]

void str_general_tester()
{
	printf("\n""str_general_tester()\n");
	{
		{
			printf("\n""TEST_01\n");
			const char* strs1[] = {"abc","def","-speed","200","aaa","-","zzz","-relevant",};
			printf("speed: %s\n",
				hf_getCommandLineParameter<char>( "speed", sizeof(strs1)/sizeof(strs1[0]),
					strs1, EGCLPF_NOFLAGS ).c_str() );

			printf("relevant: %s\n",
				hf_getCommandLineParameter<char>( "relevant", sizeof(strs1)/sizeof(strs1[0]),
					strs1, EGCLPF_NOVALUE ).c_str() );
		}
		{
			const char* argv2[] = {"ex.exe","=a","2","--noautodash","Yes","/misc","1",};

			printf("--noautodash: %s\n",
				hf_getCommandLineParameter<char>( "--noautodash", sizeof(argv2)/sizeof(argv2[0]),
					argv2, EGCLPF_NOAUTODASH ).c_str() );
			if(0){
				hf_getCommandLineParameter2(HfSCmdNames<char>("--noautodash"),
					sizeof(argv2)/sizeof(argv2[0]), argv2, EGCLPF_NOFLAGS );
				hf_getCommandLineParameter2(HfSCmdNames<char>( (const char*const*)0 ),
					sizeof(argv2)/sizeof(argv2[0]), argv2, EGCLPF_NOFLAGS );
				hf_getCommandLineParameter2(HfSCmdNames<char>( *(std::vector<const char*>*)0 ),
					sizeof(argv2)/sizeof(argv2[0]), argv2, EGCLPF_NOFLAGS );
				hf_getCommandLineParameter2(HfSCmdNames<char>(
					*(std::vector<std::basic_string<char> >*)0,(char*)""), 0, argv2, 0 );

			//	HfSCmdNames<char>* sx2 = 0;//();
			//	sx2->setArray<std::vector<std::string> >( 00 );
			//	hf_getCommandLineParameter2(
			//		HfSCmdNames<char>().setArray<std::vector<std::string> >( 0 ),
			//		0, argv2, 0 );
			}
		}
	}
	{
		//printf("\n""TEST_02 hf_isInRangeInclusive(3,2,4)\n");
		//printf("result: %d\n", (int)hf_isInRangeInclusive( 3, 2, 4 ) );
	}
	{
		printf("\n""TEST_03 some basic string functions()\n");
		char str[] = "abc";
		printf("reversed: %s\n", hf_strrev( str ) );
		const wchar_t* wstr = L"abc";
		printf("ascii str: %s\n", hf_AsciiStr<wchar_t>( wstr ).c_str() );
	}
	{
		printf("\n""TEST_04 dir and file.\n");
		printf("hf_dirname: %s\n", hf_dirname<char>("c:\\win\\sys.ini").c_str() );
		printf("hf_basename: %s\n", hf_basename<char>("c:\\programs\\somefile.dat") );
		printf("hf_basename2: %s, %s, %s\n",
			hf_basename2<char>("somefile.dat", EBNF_STRIP_VALID_EXT ).c_str(),
			hf_basename2<char>("c:\\programs\\somefile.12_4", EBNF_STRIP_VALID_EXT ).c_str(),
			hf_basename2<char>("c:\\programs\\somefile.1234", EBNF_STRIP_VALID_EXT ).c_str() );
	}

	hf_basename3("",0);
	hf_strstr("","");
	//hf_PathToPieces("",0);
	hf_swapCharacterCaseAsAscii("abc");
	hf_toLowerAsAscii("abc");
}

#endif // [HpthJFb85u4u]

/// \endcond //DOXYGEN_SKIP //}
;
bool hf_isAlpha( int in )
{
	static const char* lwrs = "abcdefghijklmnopqrstuvwxyz";
	static const char* uprs = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	return !!hf_strchr<char>( lwrs, (char)in ) ||
		!!hf_strchr<char>( uprs, (char)in );
}
bool hf_isAlphanum( int in )
{
	static const char* numbers = "0123456789";
	return !!hf_strchr<char>( numbers, (char)in ) || hf_isAlpha( in );
}
/*
/// Returns valid key value, otherwise 0 on failure.
std::string
hf_parseHotkeyTrigger( const char* inp, std::pair<std::string,std::string> modkeysOu, const char* szComma=",", const char* szPlus="+" )
{
	;
	return "";
}//*/



} // end namespace hef
