/*Include only once */
#ifndef __SOCK_SERVER_HPP_INCLUDED
#define __SOCK_SERVER_HPP_INCLUDED

#ifndef __cplusplus
#error message_queue.hpp is C++ only.
#endif

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////

#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include "epoll_fds_mgr.hpp"

using namespace std;


/*************************************************************************//**
**
**
**
*****************************************************************************/

class ServiceInfo
{
public:
	ServiceInfo() {
		memset(&addr_info, 0, sizeof(struct addrinfo));
	}
	
	virtual ~ServiceInfo()
	{}
	
	struct addrinfo addr_info;
};


/*************************************************************************//**
**
*****************************************************************************/

class INETv4ServiceInfo : public ServiceInfo
{
	typedef struct sockaddr_in sockaddr_type;

public:
	INETv4ServiceInfo() {
		conn_backlog = 0;
		memset(&addr_in, 0, sizeof(struct sockaddr_in));
	}

	void set_inet_conn_data(uint32_t addr, uint16_t port, int bklg_size = 0) {
		addr_in.sin_family      = AF_INET;
		addr_in.sin_port        = htons(port);
		addr_in.sin_addr.s_addr = htonl(addr);
		conn_backlog = bklg_size;
	}
	
	void fill_addrinfo(int const sock_type, int const protocol)
	{
		addr_info.ai_family = addr_in.sin_family;
		addr_info.ai_socktype = sock_type;
		addr_info.ai_protocol = protocol;
		addr_info.ai_addrlen = sizeof(sockaddr_type);
		addr_info.ai_addr = reinterpret_cast<sockaddr *>(&addr_in);
		addr_info.ai_flags = 0;
	}
	
	void simple_tcp_service(in_port_t const port, int const bklg_size) {
		set_inet_conn_data(INADDR_ANY, port, bklg_size);
		fill_addrinfo(SOCK_STREAM, IPPROTO_TCP);
	}

	void simple_udp_service(in_port_t const port) {
		set_inet_conn_data(INADDR_ANY, port);
		fill_addrinfo(SOCK_DGRAM, IPPROTO_UDP);
	}

public:
	sockaddr_type addr_in;
	int conn_backlog;
};


/*************************************************************************//**
**
**
**
*****************************************************************************/

class ConnectionInfo
{
public:
	int listen_sock;
	int connect_sock;
	socklen_t peer_address_len;
	struct sockaddr *peer_address;
};


/*************************************************************************//**
**
**
**
*****************************************************************************/

class SocketServer;

class SocketHandler
{
public:
	SocketHandler(SocketServer * server):
		server(server)
	{}
	
	virtual ~SocketHandler()
	{}

	virtual ServiceInfo * get_service_info() {
		return 0;
	}
	
	virtual void on_service_acknowledge(int, ServiceInfo *) {
	}

	virtual SocketHandler* on_connect(ConnectionInfo *) {
		return 0;
	}
	
	virtual void on_disconnect(int) {
	}
	
	virtual int on_incoming_data(int) {
		return 0;
	};
	
	virtual int on_signal(int, uint32_t, void *) {
		return 0;
	}
	
	SocketServer * get_server() const {
		return server;
	};

private:
	SocketServer * server;
};


/*************************************************************************//**
**
**
**
*****************************************************************************/

class SocketServer
{
// TYPE DEFINITIONS //////////////////////////////////////////////////////////
private:
	typedef EPollDescManager IOEventManager;

// METHODS ///////////////////////////////////////////////////////////////////
public:
	SocketServer();
	virtual ~SocketServer();
	static void print_blocked_signals(const char *prefix);

	int add_handler(SocketHandler * handler);
	int add_signal_handler(SocketHandler * handler, const sigset_t *mask);
	int change_signal_handler_signals(SocketHandler * handler, const sigset_t *mask);

	int rem_fd(int fd, SocketHandler **);
	int rem_fd(int const fd) {
		return rem_fd(fd, 0);
	}
	
	int set_poll_timeout_us(uint32_t poll_timeout);
	int add_server_socket(int sock_type, int backlog, struct sockaddr *address, size_t addr_size, void *udata);
	int add_server_socket_ip_stream(void *udata, int backlog, uint16_t port, uint32_t addr = INADDR_ANY);
	int add_socket_ip_datagram(void *udata, uint16_t port, uint32_t addr = INADDR_ANY);
	int process_connections();
	
	pthread_t get_thread();

protected:
	virtual int add_accepted_socket(int conn_sock, SocketHandler *new_handler);
	virtual int remove_socket(int conn_sock, IOEventManager::event_descriptor event);
	int process_incoming_connection(int svr_sock, SocketHandler *h);
	int process_signal_handler(int sigfd, SocketHandler *h);
	int process_incoming_data(int cln_sock, bool priority, SocketHandler *h);

private:
	bool set_nonblocking(int sockfd);
	char* _strerror(int const errnum);


// MEMBER VARIABLES //////////////////////////////////////////////////////////
private:
	pthread_t instance_thread;
	int server_socket;
	struct timespec poll_timeout;
	IOEventManager ioev_manager;

	static const size_t STRERR_BUF_SIZE = 64;
	char strerror_buf[STRERR_BUF_SIZE];
};


/****************************************************************************/

#endif /* __SOCK_SERVER_HPP_INCLUDED */
/* EOF */
