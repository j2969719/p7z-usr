
#include "hef_timeout_srv.h"
#include <stdio.h>
#ifdef _MSC_VER
#	include <windows.h>
#	define _HF_TIMEOUT_SRV_WIN32
#else
#	include <time.h>
#	include <sys/time.h>
#endif
namespace hef{

HfTimeoutSrv::HfTimeoutSrv()
	: LastIdent(0), IsEmiting(0)
{
}
HfTimeoutSrv::~HfTimeoutSrv()
{
	std::list<STimeoutL*>::iterator a;
	for( a = Timeouts.begin(); a != Timeouts.end(); ++a ){
		if( (**a).data2 ){
			delete (**a).data2;
			(**a).data2 = 0;
		}
		delete *a;
	}
}
/**
	Perform method that must be called continously in intervals
	(not necessairly but prefereably constant) by user code.
*/
void HfTimeoutSrv::perform()
{
	// there is no locking done here (using lock()/unlock()) because
	// it is assumed that perform() (this function) will never be called from
	// code that is result of emited timeouts (from code in this function).
	// consecutive setTimeout() calls are alowed in code executed
	// as result of calls to emit()-s from here. this is achieved by
	// proper iteration on 'Timeouts' array.
	if( !Timeouts.empty() ){
		size_t ticks = getTimeTicks();
		std::list<STimeoutL*>::iterator a;
		for( a = Timeouts.begin(); a != Timeouts.end(); ){
			if( ticks >= (**a).msReleasePos ){
				STimeoutL* ptr = *a;
				Timeouts.erase(a);
				IsEmiting = 1;
				if( !ptr->bAsData ){
					STimeout st;
					st.idTimeout = ptr->ident;
					st.user      = ptr->user;
					ptr->sgRelease.emitf( st );
				}else{
					ptr->sgRelease2.emitf( ptr->data2 );
					ptr->data2 = 0;		//to not cause to be auto-deleted.
				}
				IsEmiting = 0;
				delete ptr;
				//
				a = Timeouts.begin();
			}else
				++a;
		}
	}
}

std::list<HfTimeoutSrv::STimeoutL*>::iterator HfTimeoutSrv::
findTimeout( int ident, bool bDoLock )
{
	if(bDoLock) lock();
	std::list<STimeoutL*>::iterator a;
	for( a = Timeouts.begin(); a != Timeouts.end(); ++a ){
		if( (**a).ident == ident )
			break;
	}
	if(bDoLock) unlock();
	return a;
}

bool HfTimeoutSrv::hasTimeout( int ident )
{
	std::list<STimeoutL*>::iterator a = findTimeout( ident, 0 );
	bool res = a != Timeouts.end();
	return res;
}
bool HfTimeoutSrv::clearTimeout( int ident )
{
	bool res = 0;
	lock();
	std::list<STimeoutL*>::iterator a = findTimeout( ident, 0 );
	if( a != Timeouts.end() ){
		STimeoutL* ptr = *a;
		Timeouts.erase(a);
		delete ptr;
		res = 1;
	}
	unlock();
	return res;
}
/// Returns global ticks in miliseconds.
size_t HfTimeoutSrv::getTimeTicks()
{
#	ifdef _HF_TIMEOUT_SRV_WIN32
		return GetTickCount();
#	else
		// reference:
		// .../SDL/src/timer/unix/SDL_systimer.c
		// http://pubs.opengroup.org/onlinepubs/000095399/functions/gettimeofday.html
		timeval tv;
		gettimeofday(&tv, 0);
		size_t ticksx = (size_t)( (tv.tv_sec * 1000) + (tv.tv_usec / 1000) );
		return ticksx;
#	endif
}
HfTimeoutSrv::STimeoutL::~STimeoutL()
{
	if(data2){
		delete data2;
		data2 = 0;
	}
}
HfTimeoutSrv::SData::~SData()
{
	//printf("~SData().\n");
}
} // end namespace hef
