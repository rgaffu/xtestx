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
#include "appl.hpp"
#include "fileutility.hpp"

#define LOG_SUBSYSTEM_ID "default"

//////////////////////////////////////////////////////////////////////////////
//                     C L A S S    M E T H O D S                           //
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
APPL::APPL()
{
	memset(this, 0, sizeof(APPL));
	_VBL(1) << "APPL thread ID "<< HEX(pthread_self(), sizeof(pthread_t)*2);
	running = true;
    
    SysSettings ss;
    INF() << ss.systemUname_request();
}

/////////////////////////////////////////////////////////////////////////////
APPL::~APPL()
{
    if (pstat)
        delete pstat;
    
	if (m_timers)
		delete m_timers;

	if (m_SockSrv)
		delete m_SockSrv;
	
	running = false;
}

/////////////////////////////////////////////////////////////////////////////
bool APPL::init()
{
// 	m_pIO = new CIo;
// 	m_pIO->debug(false);
// 	if (m_pIO->init() == false) {
// 		ERROR() << "Cannot initialize hardware I/O";
// 		delete m_pIO;
// 		return false;
// 	}
/*******************************************************************************
** 
*******************************************************************************/
    static char version[40];
    snprintf(version, 40, "%d.%d", VERSION_MAJOR_APPLICATIVE, VERSION_MINOR_APPLICATIVE);
    pstat = new ApplConfigFile("appl.stat");
    pstat->put("ver_appl_appl", version);
	
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

	m_timerLed = m_timers->start_periodic(100, &APPL::on_timer_led);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool APPL::run()
{
    if (pstat && pstat->is_changed())
        pstat->write();
    
	m_SockSrv->process_connections();
	return running;
}

/////////////////////////////////////////////////////////////////////////////
void APPL::on_timer_led()
{
	static int m_cntLed = 0;
//	m_led_status = !m_led_status;
//		m_timers->stop(m_led_timer);
//		m_timers->release(m_led_timer);
	//INF() << __func__ << m_cntLed++;
    
    if ((m_cntLed % 50) == 0) {
        //FileUtility fu;
        //fu.domkdir("../prova");
        
        pstat->put("counter", m_cntLed);
    }
}
