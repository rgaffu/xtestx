/**
******************************************************************************
* @file    appl2.hpp
* @brief   Class for application (thread)
*
* @author  GR
* @version V1.0.0
* @date    10-Jan-2015
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
#ifndef __APPL2_HPP_INCLUDED
#define __APPL2_HPP_INCLUDED

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////
#include <timer_pool.hpp>
#include <sock_server.hpp>
#include "applConfigFile.hpp"

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
#define APPL_TIMERS_SIGNAL   ((unsigned)(SIGRTMIN+3))

/////////////////////////////////////////////////////////////////////////////
/// Classe: gestione applicativo.
///
/// \dot	digraph {
///		}
///	\enddot
class APPL2
{
    friend void *appl2_thread_trampoline(APPL2 *);
    
	// CLASS TYPES ///////////////////////////////////////////////////////////////
	typedef TimerPool<APPL2> ApplTimerPool;

//--- Funzioni ---
public:
	APPL2();
	~APPL2();

	/// Init
	///
	/// \param[in]	ipAddr= ip address per coldfire
	///
	/// \return		true= avvio corretto \n
	///				false= avvio errato
	bool init();
	
	/// Loop di main dell'applicativo
	///
	/// \return		true= run corretto \n
	///				false= run errato
	bool run_thread();

protected:
	class ApplSigHandler : public SocketHandler {
	public:
		ApplSigHandler(SocketServer * const server, APPL2 * const app):
			SocketHandler(server),
			m_appl(app)
		{}
	
		int on_signal(int fd, uint32_t sig, void *uptr) {
			(void)fd;
			if (sig == APPL_TIMERS_SIGNAL)
				m_appl->handle_timers_signal(reinterpret_cast<aptimer_t>(uptr));
			return 0;
		}
	
	private:
		APPL2 *m_appl;
	};
	
	void handle_timers_signal(aptimer_t elapsed_timer) {
		ApplTimerPool::call_handler(elapsed_timer);
	}
	
private:
	void on_timer();
	
//--- Variabili ---
protected:
	bool running;
	
	SocketServer        *m_SockSrv;
	ApplSigHandler      *m_sigh;
	
private:
    pthread_t m_thread;
    bool      m_thread_created;
    
	ApplTimerPool       *m_timers;
	aptimer_t 		    m_timerLed;
};

#endif /* __APPL2_HPP_INCLUDED */
/* EOF */

