/**
******************************************************************************
* @file    timer_pool.hpp
* @brief   A class that initialize and manages a collection of rt timers
*
* @author  AA Dinema s.p.a.
* @version V1.0.0
* @date    05-Nov-2014
*          
* @verbatim
* @endverbatim
*
******************************************************************************
* @attention
*
******************************************************************************
* @note
*
*****************************************************************************/

/*Include only once */
#ifndef __TIMERPOOL_HPP_INCLUDED
#define __TIMERPOOL_HPP_INCLUDED

#ifndef __cplusplus
#error message_queue.hpp is C++ only.
#endif


//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////

#include <time.h>
#include <signal.h>
#include <pool_allocator.hpp>

#include <logging.hpp>
#include "typedumpers.hpp"

#ifdef LOG_SUBSYSTEM_ID
#undef LOG_SUBSYSTEM_ID
#endif
#define LOG_SUBSYSTEM_ID "timers"


typedef void * aptimer_t;

template <class T>
struct timer_block {
	timer_block() {} // Override default constructor
	~timer_block() { // Invalidate function pointer
		timer_func = 0;
	}

	void (T::*timer_func)();
	T * instance;
	bool active;
	bool one_shot;
	timer_t timer_handle;
};

template <class T>
class TimerPool : public PoolAllocator< timer_block<T> >
{
private:
	typedef struct timer_block<T> timer_block_t;
	typedef PoolAllocator< timer_block<T> > TimerPoolAllocator;
	typedef void (T::*CallbackMethod)();

public:
	TimerPool(size_t const slab_count):
		TimerPoolAllocator(slab_count),
		signal(-1)
	{}
	
	~TimerPool() {
		timer_block_t * p = this->get_first();
		while (p != 0) {
			if (p->timer_handle) {
				_VBL(2) << "destructor @" << this << " timer_delete " << p->timer_handle;
				timer_delete(p->timer_handle);
			}
			p = this->get_next(p);
		}
	}

	int initialize(int const sig, T * const target_instance) {
		timer_block_t * p = this->get_first();
		signal = sig;
		while (p != 0) {
			if (create_timer(&p->timer_handle, sig, p) != 0)
				return -1;
			p->instance = target_instance;
			p->active = false;
			p = this->get_next(p);
		}
		return 0;
	}
	int get_signal() const {
		return signal;
	}

	aptimer_t start_oneshot(timespec * const it, CallbackMethod const func) {
		return start(it, func, false);
	}
	aptimer_t start_oneshot(int const msec, CallbackMethod const func) {
		return start(msec, func, false);
	}
	aptimer_t start_periodic(timespec * const it, CallbackMethod const func) {
		return start(it, func, true);
	}
	aptimer_t start_periodic(int const msec, CallbackMethod const func) {
		return start(msec, func, true);
	}

	void stop(aptimer_t handle) {
		timer_block_t * tb = reinterpret_cast<timer_block_t*>(handle);
		timespec ts = {0, 0};
		_VBL(2) << "TimerPool @" << this << " stop " << tb << " (" << (tb ? tb->timer_handle : 0) << ")";
		if (set_time(tb, &ts, true) == 0)
			tb->active = false;
// 		if (set_time(tb, &ts, true) == 0)
// 			this->free_object(tb);
	}

	void release(aptimer_t &handle) {
		timer_block_t * tb = reinterpret_cast<timer_block_t*>(handle);
		_VBL(2) << "TimerPool @" << this << " release " << tb << " (" << (tb ? tb->timer_handle : 0) << ")";
		if (tb != 0) {
			this->free_object(tb);
			handle = 0;
		}
	}

	static void call_handler(aptimer_t const h) {
		timer_block_t *tb = reinterpret_cast<timer_block_t*>(h);
		_VBL(3) << "TimerPool call_handler @" << tb <<
		                         " handle @" << (tb ? tb->timer_handle : 0) <<
		                         " instance @" << (tb ? tb->instance : 0);
		if (tb != 0) {
			if (tb->active == false)
				_WARNING() << "Signal on inactive timer block " << tb;
			else
			if (tb->timer_func != 0)
				(*tb->instance.*tb->timer_func)();
		}
	}

	static T * get_target_instance(aptimer_t const h) {
		timer_block_t *tb = reinterpret_cast<timer_block_t*>(h);
		if (tb != 0)
			return tb->instance;
		return 0;
	}

	bool is_timer_active(aptimer_t handle) const {
		timer_block_t * const tb = reinterpret_cast<timer_block_t*>(handle);
		if (tb != 0)
			return tb->active;
		return false;
	}

protected:
	int create_timer(timer_t * handle, int const timers_signal, void * const uptr)
	{
		struct sigevent sev;
		sev.sigev_notify = SIGEV_SIGNAL;
		sev.sigev_signo = timers_signal;
		sev.sigev_value.sival_ptr = uptr;

		unsigned retry = 10;
		while (retry--) {
			int res = timer_create(CLOCK_MONOTONIC, &sev, handle);
			if (res == -1) {
				if (errno == EAGAIN) {
					struct timespec ts = {0, 50000000}; // 50ms
					nanosleep(&ts, 0);
				} else {
					_LSYSERROR_INST("timer_create failed");
					return -1;
				}
			} else {
				_VBL(2) << "TimerPool @" << this << " create_timer " << *handle;
				return 0;
			}
		}
		return -1;
	}

	int set_time(timer_block_t * h, timespec * const it, bool const periodic) {
		/* Tolerate null handler */
		if (h == 0)
			return -1;
		/* Setup itimerspec structure according to timer mode */
		struct itimerspec its;
		its.it_value = *it;
		if (periodic)
			its.it_interval = *it;
		else {
			its.it_interval.tv_sec = 0;
			its.it_interval.tv_nsec = 0;
		}
		/* Start the timer */
		if (timer_settime(h->timer_handle, 0, &its, NULL) == -1) {
			_LSYSERROR_INST("timer_settime failed");
			return -1;
		}
		_VBL(2) << "TimerPool @" << this << " set_time " << h << " " << its;
		return 0;
	}
	
	aptimer_t start(timespec * const it, CallbackMethod const func, bool const periodic) {
		timer_block_t * h = this->alloc_object();
		if (h != 0) {
			h->timer_func = func;
			h->one_shot = !periodic;
			if (set_time(h, it, periodic) == 0)
				h->active = true;
		}
		return h;
	}
	
	aptimer_t start(int const msec, CallbackMethod const func, bool const periodic) {
		timer_block_t * h = this->alloc_object();
		if (h != 0) {
			ldiv_t dt = ldiv(msec, 1000);
			timespec it = {dt.quot, dt.rem * 1000000};
			h->timer_func = func;
			h->one_shot = !periodic;
			if (set_time(h, &it, periodic) == 0)
				h->active = true;
		}
		return h;
	}

	static bool is_timer_elapsed(timer_t timer) {
		struct itimerspec itv;
		if (timer_gettime(timer, &itv) == -1) {
			_LSYSERROR("is_timer_elapsed timer_gettime failed");
			return true;
		} else {
			_VBL(2) << "is_timer_elapsed itv:" << itv;
			return ((itv.it_value.tv_sec == 0) || (itv.it_value.tv_nsec == 0));
		}
	}

private:
	int signal;
};


/****************************************************************************/
#undef LOG_SUBSYSTEM_ID

#endif /* __TIMERPOOL_HPP_INCLUDED */
/* EOF */

 
