
#include "hef_str_from_data_cltr.h"
#include <cctype>
#include <cstring>
namespace hef{
;
/// \cond DOXYGEN_SKIP
;
class _CDfltCharPred : public HfIStringFromDataPred {
public:
	virtual bool isCharacter( unsigned char c3 )const{
		char c = (char)c3;
		return isprint( c ) || c=='\r' || c=='\n' || c=='\t' || c==0x20;
	}
	virtual char toCharacter( unsigned char c3 )const{
		char c = (char)c3;
		if( isCharacter(c) )
			return c;
		return '.';
	}
};
/// \endcond //DOXYGEN_SKIP
;

// static member instantation.
_CDfltCharPred HfCStrFromDataCltr::DfltCharPred;

/// Constructor.
/// 'pred' - if 0 then default is used.
HfCStrFromDataCltr::
HfCStrFromDataCltr( size_t maxNonCharactersInARow, HfIStringFromDataPred* pred )
	: MaxNonAsciiInARow(maxNonCharactersInARow), Pred(pred)
{
	if(!Pred){
		Pred = &HfCStrFromDataCltr::DfltCharPred;
	}
	clear();
}
HfCStrFromDataCltr& HfCStrFromDataCltr::clear()
{
	MaxNonAsciiInARowStore = 0;
	Partial = "";
	return *this;
}

/// Returns error if max number of non-characters has been ecountered.
/// bGotStr is set to 1 if reached terminating string, specified as 'szTerm'.
/// terminating string is being consumed.
bool HfCStrFromDataCltr::
collect( const void* data_, size_t size, const char* szTerm, size_t* uConsumed,
		bool* bGotStr )
{
	const unsigned char* data = (const unsigned char*)data_;
	unsigned char c2;
	size_t i, uTermSize = strlen(szTerm); //char c;
	*bGotStr = 0; *uConsumed = 0;
	for( i=0; i<size; i++ ){
		c2 = data[i];
		if( Pred->isCharacter( c2 ) ){
			MaxNonAsciiInARowStore = 0;
			Partial += Pred->toCharacter( c2 );
		}else{
			MaxNonAsciiInARowStore++;
			if( MaxNonAsciiInARowStore > MaxNonAsciiInARow )
				return 0;
			Partial += Pred->toCharacter( c2 );
		}
		(*uConsumed)++;
		if( Partial.size() >= uTermSize ){
			std::string str = Partial.substr( Partial.size() - uTermSize );
			if( !strcmp( str.c_str(), szTerm ) ){
				*bGotStr = 1;
				return 1;
			}
		}
	}
	return 1;
}

} // end namespace hef
