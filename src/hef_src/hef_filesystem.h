
#ifndef _HEF_FILESYSTEM_H_
#define _HEF_FILESYSTEM_H_
#include "hef_str.h"
namespace hef{
;
enum {
	EHFDCF_NoFlags = 0,
	EHFDCF_IncludeDotedOnes = 0x1,
};

class HfDirContents {
public:
	HfDirContents( const char* szDir, size_t flags = EHFDCF_NoFlags );
	~HfDirContents();
	struct SFile {
		std::string fname;
		bool        bDir;
		SFile() : bDir(0) {}
	};
	bool getNextFile( SFile& out );
private:
	void cleanup();
	bool isFnameAcceptable( const std::string& fname2 );
private:
	std::string Dir;
	size_t Flags;
	void* WfdHandle;
};

} // end namespace hef

#endif //_HEF_FILESYSTEM_H_
