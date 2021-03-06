/**
******************************************************************************
* @file    appl.hpp
* @brief   Class for application
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
#ifndef __APPL_HPP_INCLUDED
#define __APPL_HPP_INCLUDED

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////
#include <timer_pool.hpp>
#include <sock_server.hpp>
#include "applConfigFile.hpp"
#include "appl2.hpp"

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
class APPL
{
	// CLASS TYPES ///////////////////////////////////////////////////////////////
	typedef TimerPool<APPL> ApplTimerPool;

//--- Funzioni ---
public:
	APPL();
	~APPL();

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
	bool run();

protected:
	class ApplSigHandler : public SocketHandler {
	public:
		ApplSigHandler(SocketServer * const server, APPL * const app):
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
		APPL *m_appl;
	};
	
	void handle_timers_signal(aptimer_t elapsed_timer) {
		ApplTimerPool::call_handler(elapsed_timer);
	}
	
private:
	void on_timer_led();
	
//--- Variabili ---
protected:
	bool running;
	
	SocketServer        *m_SockSrv;
	ApplSigHandler      *m_sigh;
	
private:
	ApplTimerPool       *m_timers;
	aptimer_t 		    m_timerLed;
    
    ApplConfigFile      *m_pstat;
    
    APPL2               *m_pAppl2;
};

#endif /* __APPL_HPP_INCLUDED */
/* EOF */

