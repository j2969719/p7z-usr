
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

/// Converts CRLF to LF (!r!n to !n).
/// Operaation is done in-place, no output buffer is needed.
void hf_ConvertCrLfToLf( std::vector<unsigned char>* data2 )
{
	int i, b, num = (int)data2->size(), decr = 0;
	for( i=0, b=0; i<num; ){
		if( (*data2)[i] == '\r' && i+1 < num && (*data2)[i+1] == '\n' ){
			i++; decr++;// abcxyzxyaaa
		}else{
			if( i > b )
				(*data2)[b] = (*data2)[i];
			b++; i++;
		}
	}
	if( decr )
		data2->resize( data2->size() - decr, 0 );
}
/// Gets time components from hour-minute-seconds-miliseconds string.
/// Format is: "<H:M:S.zzz>".
/// \param flags2 - flags fe. \ref HF_EGHMS2_MinutesRequired.
/// \param endp   - optional. NULL=ignore. returns stop position.
/// \sa hf_TimeMsecsDecomposeToHMSMs()
/// old: hf_GetHMSMsFromStr()
bool hf_GetTimeHMSMsFromStr( const char* inp, int* hr3, int* mn3, int* sc3, int* ms3, int flags2, const char** endp )
{
	int dm0, *hr2=(hr3?hr3:&dm0), *mn2=(mn3?mn3:&dm0), *sc2=(sc3?sc3:&dm0), *ms2=(ms3?ms3:&dm0);
	*hr2 = *mn2 = *sc2 = *ms2 = 0;
	std::vector<int> vals; bool bHasMsecs = 0; int val;
	const char* sz2=inp, *sz3, *sz4 = 0;
	for( sz3=inp; *sz2; sz3=sz2+1, sz4=0 ){
		val = (int)strtoul( sz3, (char**)&sz4, 10 );
		if( !sz4 || sz3 == sz4 ){ break; }
		vals.push_back(val);
		sz2 = sz4;
		if( !*sz2 ){ break; }
		sz3 = hf_strchr( sz2, ':' );
		sz4 = hf_strchr( sz2, '.' );
		sz3 = std::min( sz3?sz3:sz4, sz4?sz4:sz3 );
		if( !sz3 || sz3 > inp+64 ) break;
		sz2 = sz3;
		if( *sz2 == ':' ){
			continue;
		}else{
			sz4 = 0;
			val = (int)strtoul( sz2+1, (char**)&sz4, 10 );
			if( !sz4 || sz2+1 == sz4 ){ break; }
			sz2 = sz4;
			vals.push_back(val);
			bHasMsecs = 1;
			break;
		}
	}
	if(endp)
		*endp = sz2;
	if( !bHasMsecs && !vals.empty() )
		vals.push_back(0);
	if( !bHasMsecs && flags2 & HF_EGHMS2_MilisecsRequired )
		return 0;
	if( flags2 & HF_EGHMS2_SecondsRequired && vals.size() < 2 )
		return 0;
	if( flags2 & HF_EGHMS2_MinutesRequired && vals.size() < 3 )
		return 0;
	if( flags2 & HF_EGHMS2_HoursRequired && vals.size() < 4 )
		return 0;
	std::reverse( vals.begin(), vals.end() );
	if( !vals.empty() ){
		*ms2 = vals[0];
		if( vals.size() >= 2 ){
			*sc2 = vals[1];
			if( vals.size() >= 3 ){
				*mn2 = vals[2];
				if( vals.size() >= 4 )
					*hr2 = vals[3];
			}
		}
		if( !(flags2 & HF_EGHMS2_NoBoundaryCheck) ){
			if( vals[0] > 999 || vals[1] > 59 || vals[2] > 59 )
				return 0;
		}
		return 1;
	}
	return 0;
}
/**
	Converts input time in miliseconds into hour-minute-seconds-miliseconds components.
	Intended for use with the "<H:M:S.zzz>" format.
	\param inp - input time in miliseconds.
	\code
		// Example:
		int hr3=0,mn3=0,sc3=0,ms3=0;
		int milis = 23*60*60*1000 + 59*60*1000 + 59*1000 + 999; // = 23:59:59.999
		hf_TimeMsecsDecomposeToHMSMs( milis, &hr3, &mn3, &sc3, &ms3 );
		printf("result: %d:%02d:%02d.%03d\n", hr3, mn3, sc3, ms3 );
	\endcode
	Max input value, to not cause overflow in the hours
	component, is: 7730941132799999 = 2^31 * 3600000 - 1. Thus, max output that
	can be calculated corectly by this function is: "2147483647:59:59.999".
	\sa hf_GetTimeHMSMsFromStr()
*/
void hf_TimeMsecsDecomposeToHMSMs( uint64_t inp, int* hr3, int* mn3, int* sc3, int* ms3 )
{
	if(hr3){ *hr3 = ( inp / (60*60*1000) ); }
	if(mn3){ *mn3 = ( (inp / (60*1000)) % 60 ); }
	if(sc3){ *sc3 = ( (inp / 1000) % 60 ); }
	if(ms3){ *ms3 = ( (inp / 1) % 1000 ); }
}

} // end namespace hef
