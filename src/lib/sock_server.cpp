#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/signalfd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "epoll_fds_mgr.hpp"
#include "sock_server.hpp"

#include "typedumpers.hpp"
#include "logging.hpp"
#define LOG_SUBSYSTEM_ID "default"


/*************************************************************************//**
**
*/
std::ostream& operator<<(std::ostream& w, const struct signalfd_siginfo &fdsi)
{
    w << "(signalfd_siginfo " <<
		" ssi_signo="   << fdsi.ssi_signo   <<  // uint32_t
		" ssi_errno="   << fdsi.ssi_errno   <<  // int32_t 
		" ssi_code="    << fdsi.ssi_code    <<  // int32_t 
		" ssi_pid="     << fdsi.ssi_pid     <<  // uint32_t
		" ssi_uid="     << fdsi.ssi_uid     <<  // uint32_t
		" ssi_fd="      << fdsi.ssi_fd      <<  // int32_t 
		" ssi_tid="     << fdsi.ssi_tid     <<  // uint32_t
		" ssi_band="    << fdsi.ssi_band    <<  // uint32_t
		" ssi_overrun=" << fdsi.ssi_overrun <<  // uint32_t
		" ssi_trapno="  << fdsi.ssi_trapno  <<  // uint32_t
		" ssi_status="  << fdsi.ssi_status  <<  // int32_t 
		" si_int="      << fdsi.ssi_int     <<  // int32_t 
		" ssi_ptr="     << fdsi.ssi_ptr     <<  // uint64_t
		" ssi_utime="   << fdsi.ssi_utime   <<  // uint64_t
		" ssi_stime="   << fdsi.ssi_stime   <<  // uint64_t
		" ssi_addr="    << fdsi.ssi_addr    <<  // uint64_t
	")";
    return w;
}

/*************************************************************************//**
** Print list of signals within a signal set
*/
std::ostream& operator<<(std::ostream& w, const sigset_t &sigset)
{
    int sig, cnt;

	w << "(sigset_t";
	cnt = 0;
    for (sig = 1; sig < NSIG; sig++) {
        if (sigismember(&sigset, sig)) {
            cnt++;
			w << " " << strsignal(sig) << "(" << sig << ")";
        }
    }
    if (cnt == 0)
        w << " <empty>";
	w << ")";
	return w;
}


/*************************************************************************//**
** Get blocked signals
*/
static inline sigset_t _blocked_signals(void)
{
	sigset_t sigset;
	sigemptyset(&sigset);
	if (sigprocmask(SIG_SETMASK, NULL, &sigset) == -1)
		_LSYSERROR("get sigprocmask error");
	return sigset;
}


/*************************************************************************//**
**
*/
SocketServer::SocketServer()
{
	instance_thread = pthread_self();
	poll_timeout.tv_sec = 1;
	poll_timeout.tv_nsec = 0;
}


/*************************************************************************//**
**
*/
SocketServer::~SocketServer()
{
	ioev_manager.close_all();
}


pthread_t SocketServer::get_thread()
{
	return this->instance_thread;
}


/*************************************************************************//**
**
*/
char* SocketServer::_strerror(int const errnum)
{
		return strerror_r(errnum, strerror_buf, STRERR_BUF_SIZE);
// 		switch (res) {
// 			case EINVAL: return "(strerror) Not a valid error number";
// 			case ERANGE: return "(strerror) Insufficient storage";
// 		}
// 		return strerror_buf;
}


/*************************************************************************//**
**
*/
bool SocketServer::set_nonblocking(int const sockfd)
{
	int flags;

	if ((flags = fcntl(sockfd, F_GETFL, 0)) == -1) {
		_LSYSERROR("fcntl F_GETFL");
		return false;
	}

	flags |= O_NONBLOCK;

	if (fcntl(sockfd, F_SETFL, flags) < 0) {
		_LSYSERROR("fcntl F_SETFL");
		return false;
	}

	return true;
}


/*************************************************************************//**
**
*/
int SocketServer::add_server_socket(int sock_type, int backlog, struct sockaddr *address, size_t addr_size, void *udata)
{
	int sockfd;
	int res;
	int sock_domain;
	
	switch (address->sa_family) {
		case AF_INET: sock_domain = AF_INET; break;
		default:
			return -1;
	}
	
	sockfd = socket(sock_domain, sock_type, 0);
    if (sockfd == -1) {
		_LSYSERROR("cannot create socket");
		return -1;
	}
	
	const int yes = 1;
	res = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if (res < 0) {
		_LSYSERROR("setsockopt failed");
		goto close_and_exit_with_error;
	}

	if (set_nonblocking(sockfd) == false)
		goto close_and_exit_with_error;
	
	res = bind(sockfd, address, addr_size);
	if (res < 0) {
		_LSYSERROR("bind of address " << address << " failed");
		goto close_and_exit_with_error;
	}

	if (sock_type == SOCK_STREAM) {
		res = listen(sockfd, backlog);
		if (res < 0) {
			_LSYSERROR("listen error");
			goto close_and_exit_with_error;
		}
		res = ioev_manager.add_server_socket(sockfd, udata);
	} else
		res = ioev_manager.add_socket(sockfd, udata);

	if (res == 0)
		return sockfd;

close_and_exit_with_error:
	close(sockfd);
	return -1;
}


/*************************************************************************//**
**
*/
int SocketServer::add_server_socket_ip_stream(void *udata, int backlog, uint16_t port_num, uint32_t addr)
{
	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(addr);
    sock_addr.sin_port = htons(port_num);
	
	return add_server_socket(SOCK_STREAM, backlog,
							 (struct sockaddr *)&sock_addr, sizeof(struct sockaddr_in),
							 udata);
}


/*************************************************************************//**
**
*/
int SocketServer::add_socket_ip_datagram(void *udata, uint16_t port, uint32_t addr)
{
	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(addr);
    sock_addr.sin_port = htons(port);
	
	return add_server_socket(SOCK_DGRAM, 0,
							 (struct sockaddr *)&sock_addr, sizeof(struct sockaddr_in),
							 udata);
}


/*************************************************************************//**
**
*/
int SocketServer::add_handler(SocketHandler * handler)
{
	if (handler == 0)
		return -1;

	ServiceInfo * svi = handler->get_service_info();
	int res = -1;
	
	if (svi == 0)
		return -1;
	
	if (svi->addr_info.ai_socktype == SOCK_STREAM) {
		INETv4ServiceInfo * tsvi = dynamic_cast<INETv4ServiceInfo *>(svi);
		if (tsvi != 0)
			res = add_server_socket(SOCK_STREAM, tsvi->conn_backlog,
						svi->addr_info.ai_addr, svi->addr_info.ai_addrlen, handler);
	} else
	if 	(svi->addr_info.ai_socktype == SOCK_DGRAM) {
		res = add_server_socket(SOCK_DGRAM, 0, svi->addr_info.ai_addr,
						  svi->addr_info.ai_addrlen, handler);
	}
	
	handler->on_service_acknowledge(res, svi);
	
	return res;
}


/*************************************************************************//**
**
*/
int SocketServer::add_signal_handler(SocketHandler * handler, const sigset_t *mask)
{
	int sigfd;
	
	if (sigprocmask(SIG_BLOCK, mask, NULL) == -1) {
		_LSYSERROR("sigprocmask error");
		return -1;
	}
	
	sigfd = signalfd(-1, mask, O_CLOEXEC);
	if (sigfd == -1) {
		_LSYSERROR("signalfd error");
		return -1;
	}
	
	set_nonblocking(sigfd);
	
	IOEventManager::event_flags events = IOEventManager::EV_IN;
	int res = ioev_manager.add_fd(sigfd, events, handler, IOEventManager::FDIF_NONSOCKET);
	if (res != 0) {
		close(sigfd);
		return -1;
	}
	
	_VBL(2) << "created signalfd "<< sigfd << " with signals:" << *mask;
	
	return sigfd;
}


/*************************************************************************//**
**
*/
int SocketServer::change_signal_handler_signals(SocketHandler * handler, const sigset_t *mask)
{
	// (void)handler; (void)mask;
	_DBG() << "change_signal_handler_signals handler:"<< handler << " mask:" << *mask;
	return -1;
}


/*************************************************************************//**
**
*/
int SocketServer::add_accepted_socket(int conn_sock, SocketHandler *new_handler)
{
	return ioev_manager.add_socket(conn_sock, new_handler);
}


/*************************************************************************//**
**
*/
int SocketServer::remove_socket(int conn_sock, IOEventManager::event_descriptor event)
{
	event->rem_socket(&ioev_manager);
	return close(conn_sock);
}


/*************************************************************************//**
**
*/
int SocketServer::rem_fd(int fd, SocketHandler **sock_handler)
{
	return ioev_manager.rem_fd(fd, (void **)sock_handler);
}


/*************************************************************************//**
**
*/
int SocketServer::process_signal_handler(int const sigfd, SocketHandler * const handler)
{
	int res;
	struct signalfd_siginfo fdsi;
	
	res = read(sigfd, &fdsi, sizeof(struct signalfd_siginfo));
	if (res != sizeof(struct signalfd_siginfo)) {
		_LSYSERROR("read error");
		return -1;
	}
	
	_VBL(4) << fdsi;
	return handler->on_signal(sigfd, fdsi.ssi_signo, reinterpret_cast<void *>(fdsi.ssi_ptr));
}


/*************************************************************************//**
**
*/
int SocketServer::process_incoming_connection(int const svr_sock, SocketHandler * const handler)
{
	struct sockaddr_in acpt_addr;
	socklen_t addrlen;
	int conn_sock = 0;
	ConnectionInfo ci;
	SocketHandler * new_handler;

	for(;;) {
		addrlen = sizeof(struct sockaddr_in);
		conn_sock = accept(svr_sock, (struct sockaddr*)&acpt_addr, &addrlen);
		if (conn_sock == -1) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
				return 0; // All backlogged connections have been accepted
			// Error accepting new connection
			_LSYSERROR("accept error");
			return -1;
		}

		ci.listen_sock = svr_sock;
		ci.connect_sock = conn_sock;
		ci.peer_address_len = addrlen;
		ci.peer_address = (struct sockaddr*)&acpt_addr;
		
		new_handler = handler->on_connect(&ci);
		if (new_handler == 0) {
			_VBL(2) << "on_connect returned null pointer";
			close(conn_sock);
			return -1;
		}
		
		//_dump_accepted(svr_sock, conn_sock, &acpt_addr);
		add_accepted_socket(conn_sock, new_handler);
	}
	
	return -1;
}


/*************************************************************************//**
**
*/
int SocketServer::process_incoming_data(int const cln_sock, bool const priority, SocketHandler * const handler)
{
	int res, dummy;
	(void)priority;
	
	for(;;) {
		res = recvfrom(cln_sock, &dummy, 1, MSG_PEEK, 0, 0);
		if (res > 0) {
			return handler->on_incoming_data(cln_sock);

		} else
		if (res < 0) {
			if (errno != EINTR) {
				_LSYSERROR("recvfrom error");
				break;
			}
		} else
			break;
	}
	return res;
}


/*************************************************************************//**
**
*/
int SocketServer::set_poll_timeout_us(uint32_t const timeout_us)
{
	struct timespec ts;
	ldiv_t dt = ldiv(timeout_us, 1000000);
	ts.tv_sec  = dt.quot;
	ts.tv_nsec = dt.rem * 1000;
	
	poll_timeout = ts;
	return 0;
}


/*************************************************************************//**
**
*/
int SocketServer::process_connections()
{
	IOEventManager::event_iterator events;
	IOEventManager::event_descriptor event;
	
	events = ioev_manager.wait_for_events(&poll_timeout);
	if (events == 0) {
		_VBL(5) << "wait_for_events zero events";
		return 0;
	}
	_VBL(4) << "process_connections got " << events << " events";
	
	// wait_for_events is guaranteed to return system error in errno
	if (events == IOEventManager::ERROR_EVENT) {
		if (errno != EINTR)
			_LSYSERROR("wait_for_events error");
		return 0;
	}
	
	event = ioev_manager.get_first_event();
	while (event) {
		int fd = event->get_fd();
		SocketHandler * handler = static_cast<SocketHandler *>(event->get_udata());
		
		if (event->is_error() || event->is_hangup()) {
			_VBL(1) << "POLL ERR " << fd << " " << HEX1(event->events);
			handler->on_disconnect(fd);
			remove_socket(fd, event);
		
		} else
		if (event->is_signal()) {
			_VBL(4) << "wait_for_events signalfd event";
			process_signal_handler(fd, handler);
		
		} else
		if (event->is_incoming_connection()) {
			_VBL(4) << "wait_for_events connection event";
			process_incoming_connection(fd, handler);
			
		} else
		if (event->is_incoming_data()) {
			_VBL(4) << "wait_for_events data event";
			int res = process_incoming_data(fd, event->has_priority(), handler);
			if (res <= 0) {
				_VBL(4) << "process_incoming_data returned 0: disconnection";
				handler->on_disconnect(fd);
				remove_socket(fd, event);
			}
		}
		_VBL(5) << "get_next_event";
		event = ioev_manager.get_next_event();
	}
	return 0;
}
