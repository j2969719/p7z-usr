
#ifndef _C_STRING_FROM_DATA_CLTR_H_
#define _C_STRING_FROM_DATA_CLTR_H_
#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif
#include <string>
#include <vector>
namespace hef{
;
class HfIStringFromDataPred;
class _CDfltCharPred;

/// String From Data Collector class.
/// User calls collect() consecutivelly. Should continue calls whether retval 
/// remains 1 and 'bGotStr' remains 0.
/// Success should be assumed when returned retval==1 AND 'bGotStr' == 1.
/// Failure when returned retval==0.
/// See also: HfCStrFromDataCltr::collect()
class HfCStrFromDataCltr {
public:
	HfCStrFromDataCltr( size_t maxNonCharactersInARow = 0, HfIStringFromDataPred* pred = 0 );
	bool                collect( const void* data, size_t size, const char* szTerm, size_t* uConsumed=0, bool* bGotStr=0 );
	const char*         getConsumedString()const { return Partial.c_str(); }
	HfCStrFromDataCltr& clear();
private:
	size_t MaxNonAsciiInARow;
	std::string Partial;
	size_t MaxNonAsciiInARowStore;
	HfIStringFromDataPred* Pred;
	static _CDfltCharPred DfltCharPred;
};
/// Predicate class for valid characters.
class HfIStringFromDataPred {
public:
	virtual bool isCharacter( unsigned char c )const = 0;
	virtual char toCharacter( unsigned char c )const = 0;
};

} // end namespace hef

#endif //_C_STRING_FROM_DATA_CLTR_H_
