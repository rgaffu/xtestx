/**
******************************************************************************
* @file    appl.cpp
*****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////
#include <logging.hpp>
#include "syssettings.hpp"

#include "version.h"
#include "appl2.hpp"
#include "fileutility.hpp"

#define LOG_SUBSYSTEM_ID "default"

//////////////////////////////////////////////////////////////////////////////
//                     C L A S S    M E T H O D S                           //
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
APPL2::APPL2()
{
	memset(this, 0, sizeof(APPL2));
    
    m_thread_created = false;
}

/////////////////////////////////////////////////////////////////////////////
APPL2::~APPL2()
{
	if (m_timers)
		delete m_timers;

	if (m_SockSrv)
		delete m_SockSrv;
	
    if (m_thread_created) {
        _VBL(1) << "Joining APPL2 Management thread";
        pthread_join(m_thread, 0);
    }
    
	running = false;
}

/////////////////////////////////////////////////////////////////////////////
void *appl2_thread_trampoline(APPL2 *obj)
{
    bool r;
    
    // Block all signals in this thread
    sigset_t sigset;
    sigfillset(&sigset);
    if (pthread_sigmask(SIG_SETMASK, &sigset, NULL) == -1) {
        _LSYSERROR("pthread_sigmask error");
        return 0;
    }

    // Startup synchronization
    //obj->m_state_mutex.lock();
    _VBL(1) << "MiWi main thread started ID:" << HEX(pthread_self(), sizeof(pthread_t)*2);
    //obj->m_state_mutex.unlock();

    do {
        r = obj->run_thread();
        //_VBL(4) << "MiWi state: " << obj->get_state();
    } while(r);
    
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
bool APPL2::init()
{
    int res;
    
    m_timers = new ApplTimerPool(4);
    if (m_timers->initialize(APPL_TIMERS_SIGNAL, this) == -1) {
        ERROR() << "Timers initialization error";
        return false;
    }
    
    // Init TCP socket protocol
    m_SockSrv = new SocketServer();
    m_SockSrv->set_poll_timeout_us(5000); // 200Hz
    
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, APPL_TIMERS_SIGNAL);
    m_sigh = new ApplSigHandler(m_SockSrv, this);
    if (m_SockSrv->add_signal_handler(m_sigh, &sigmask) == -1) {
        ERROR() << "Cannot install ApplSigHandler";
        return false;
    }

    m_timerLed = m_timers->start_periodic(5000, &APPL2::on_timer);
    

    //m_state_mutex.lock();
    res = pthread_create(&m_thread, NULL,
                         (void* (*)(void*))appl2_thread_trampoline, this);
    if (res != 0) {
        _ERROR() << "cannot create APPL2 thread";
        goto unlock_cleanup_error_exit;
    } else {
        _VBL(1) << "APPL2 thread created";
        m_thread_created = true;
    }
    //m_state_mutex.unlock();
    return true;

unlock_cleanup_error_exit:
    //m_state_mutex.unlock();
    return false;
}

#if 0
/////////////////////////////////////////////////////////////////////////////
bool APPL2::init()
{
	m_timers = new ApplTimerPool(4);
	if (m_timers->initialize(APPL_TIMERS_SIGNAL, this) == -1) {
		ERROR() << "Timers initialization error";
		return false;
	}
	
	// Init TCP socket protocol
	m_SockSrv = new SocketServer();
	m_SockSrv->set_poll_timeout_us(5000); // 200Hz
	
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, APPL_TIMERS_SIGNAL);
	m_sigh = new ApplSigHandler(m_SockSrv, this);
	if (m_SockSrv->add_signal_handler(m_sigh, &sigmask) == -1) {
		ERROR() << "Cannot install ApplSigHandler";
		return false;
	}

	m_timerLed = m_timers->start_periodic(5000, &APPL2::on_timer);
	
	return true;
}
#endif

/////////////////////////////////////////////////////////////////////////////
bool APPL2::run_thread()
{
	m_SockSrv->process_connections();
	return running;
}

/////////////////////////////////////////////////////////////////////////////
void APPL2::on_timer()
{
	static int m_cntLed = 0;
    
    INF() << __func__ << m_cntLed++;
    
    if ((m_cntLed % 50) == 0) {
    }
}

