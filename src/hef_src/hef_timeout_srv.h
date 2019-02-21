
#ifndef _HEF_TIMEOUT_SRV_H_
#define _HEF_TIMEOUT_SRV_H_
#include <list>
#include <stdio.h>
#ifdef _MSC_VER
#	pragma warning ( disable: 4786 )
#endif
//#include <sigslot_mod.h>
//#include "../../../libs/sigslot_cpp_library/inc/sigslot_mod.h"
#include "hef_sigslot_mod.h"
namespace hef{

template<class desttypeT> class HfDataRelay;

/**
	Timeout service class that relies on 'sigslot' library.
	To implement this class, perform() method must be called consecutivelly,
	prefereably in application message loop.
	Timeouts are then emited from thread that is calling perform() function.
	All global functions are thread safe, with added lock() and unlock() calls.
	Timeouts of objects, that need to be deleted before their timeout is
	emited, must be cleared calling clearTimeout() before potential next perform() call.

	IMPORTANT: never attempt to delete object that code is inside one of it's
	           functions. use another setTimeout() call, eg. with timeout in miliseconds
	           set to 0, then delete taht object there.

	Example 1:
	\code
		HfTimeoutSrv ts;
		...
		class CApple : public sigslot::has_slots<> {
		public:
			void taskCompleted( HfTimeoutSrv::STimeout in ){
				void* pUser = in.user;
				// ...
			}
		};
		...
		CApple apple;
		...
		ts.setTimeout( 2000, &apple, &CApple::taskCompleted, (void*)781 );
	\endcode

	Example 2:
	\code
		HfTimeoutSrv ts;
		...
		class CBanana : public sigslot::has_slots<> {
		public:
			void soCompleted( CRelay2* inp ){
				...
				delete inp;
			}
			CBanana( int a, int b );
			class CRelay2 : public HfDataRelay<CBanana> {
				...
			};
		};
		CBanana banana;
		ts.setTimeout2( 1000, &banana,
			reinterpret_cast<CBanana::CRelay2::pmemfunc_t>( &CBanana::soCompleted ),
				new CBanana::CRelay2( 11, 12 ) );
	\endcode
*/
class HfTimeoutSrv : public sigslot::has_slots<>
{
	struct STimeoutL;
public:
	struct STimeout {
		int       idTimeout;
		void*     user;
		virtual ~STimeout() {}
	};
	class SData {
	public:
		virtual ~SData();
		//virtual misc();
	};
	HfTimeoutSrv();
	~HfTimeoutSrv();
	void perform();
	// Generic set-timeout function.
	template<class desttype>
	int setTimeout( size_t miliseconds, desttype* pclass, void (desttype::*pmemfun2)(STimeout), void* user = 0 )
	{
		lock();
		//assert( pclass && pmemfun2 );
		size_t ticks = getTimeTicks();
		STimeoutL* st = new STimeoutL;
		st->ident        = ++LastIdent;
		st->msReleasePos = ticks + miliseconds;
		st->user         = user;
		st->sgRelease.connect( pclass, pmemfun2 );
		Timeouts.push_back(st);
		unlock();
		return st->ident;
	}
	/// Comprehensive set-timeout function with ability to pass objects durning timeout calls.
	/// Owneship of object pointed to by 'dataobject' ('data5') parameter is given to timeout
	/// service for the time timeout is not released. Once timeout is released,
	/// thus destination function (pmemfun3) called, owneship is passed back to
	/// destination function. Now deletion of class derived from
	/// HfDataRelay<desttype>* must be acomplished, at some point, in code
	/// executed by destination function call.
	/// This owneship mechanism is implemented so that memory is not lost in case
	/// if timeout is cancelled, before reaching its destination, by clearTimeout().
	template<class desttype>
	int setTimeout2( size_t miliseconds, desttype* pclass,
			void (desttype::*pmemfun3)(HfDataRelay<desttype>*),
			HfDataRelay<desttype>* dataobject )
	{
		//sigslot::lock_block<sigslot::multi_threaded_local> lockx(this);
		lock();
		size_t ticks = getTimeTicks();
		dataobject->setReciever( pclass, pmemfun3 );
		STimeoutL* st = new STimeoutL;
		st->ident        = ++LastIdent;
		st->msReleasePos = ticks + miliseconds;
		st->data2        = dataobject;
		st->bAsData      = 1;
		st->sgRelease2.connect( dataobject, &HfDataRelay<desttype>::soDone );
		Timeouts.push_back(st);
		unlock();
		return st->ident;
	}
	bool clearTimeout( int ident );
	bool hasTimeout( int ident );
	static size_t getTimeTicks();
	bool isEmiting()const {return IsEmiting;}
private:
	std::list<STimeoutL*>::iterator findTimeout( int ident, bool bDoLock = 0 );
	struct STimeoutL {
		sigslot::signal1<STimeout>  sgRelease;
		sigslot::signal1<SData*>    sgRelease2;
		int                         ident;
		size_t                      msReleasePos;
		SData*                      data2;
		bool                        bAsData;
		void*                       user;
		STimeoutL() : sgRelease(), ident(0), msReleasePos(0), data2(0), bAsData(0), user(0) {}
		~STimeoutL();
	};
	std::list<STimeoutL*> Timeouts;
	int LastIdent;
	bool IsEmiting;
};

/// Base class for timeouts custom parameter with HfTimeoutSrv::setTimeout2().
template<class desttypeT>
class HfDataRelay : public HfTimeoutSrv::SData, public sigslot::has_slots<> {
public:
	typedef void (desttypeT::*pmemfunc_t)(HfDataRelay<desttypeT>*);
	friend HfTimeoutSrv;
	HfDataRelay() : pclass3(0) {}
	virtual ~HfDataRelay(){
		//printf("~HfDataRelay().\n");
	}
private:
	HfDataRelay( desttypeT* pclass2, pmemfunc_t pmemfunc2 ) {
		pclass3 = pclass2;
		pmemfunc3 = pmemfunc2;
	}
	void setReciever( desttypeT* pclass2, pmemfunc_t pmemfunc2 ){
		pclass3 = pclass2;
		pmemfunc3 = pmemfunc2;
	}
	void soDone( HfTimeoutSrv::SData* data ){
		HfDataRelay<desttypeT>* datarelay = dynamic_cast<HfDataRelay<desttypeT>*>( data );
		// call member function.
		((*pclass3).*(pmemfunc3))( datarelay );
	}
private:
	desttypeT* pclass3;
	pmemfunc_t pmemfunc3;
};
} // end namespace hef
#endif //_HEF_TIMEOUT_SRV_H_
