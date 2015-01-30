/*************************************************************************//**
**
**
**
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "epoll_fds_mgr.hpp"


#include "logging.hpp"
#define LOG_SUBSYSTEM_ID "default"


/*************************************************************************//**
**
*/
EPollDescManager::EPollDescManager()
{
 	fddesc_size = 16;

	epoll_handle = epoll_create1(0);
	if (epoll_handle < 0)
		_LSYSERROR("epoll_create1 error");

	epoll_fddesc = (struct epoll_event *)calloc(fddesc_size,
												sizeof(struct epoll_event));
	if (epoll_fddesc == 0)
		_ERROR() << "memory allocation error";

	fddescs = new fddesc_pool(fddesc_size);
}


/*************************************************************************//**
**
*/
EPollDescManager::~EPollDescManager()
{
	if (epoll_fddesc != 0)
		free(epoll_fddesc);
	
	delete fddescs;
}


/*************************************************************************//**
**
*/
void EPollDescManager::_dump_fds()
{
	int fddesc_count = fddescs->get_pool_usage();
	_DBG() << "--- fds dump (" << fddesc_count << ")";
	for (int i=0; i < fddesc_count; i++)
		_DBG() << "--- " << i <<
			" fd:" <<  epoll_fddesc[i].data.fd <<
			" events:" << HEX(epoll_fddesc[i].events, 4);
}


/*************************************************************************//**
**
*/
void EPollDescManager::close_all()
{
	fddesc_info * t = fddescs->get_first();
	while (t) {
		if (t->flags & FDIF_VALID) {
			int fd = t->fd;
			rem_fd(t);
			_VBL(2) << "close_all closing fd " << fd;
			close(fd);
		}
		t = fddescs->get_next(t);
	}
}


/*************************************************************************//**
**
*/
int EPollDescManager::add_fd(int const fd, uint32_t const events, void * const udata, uint32_t const flags)
{
	struct epoll_event event;
	struct fddesc_info * fdd_info;

	fdd_info = alloc_fddinfo();
	if (fdd_info == 0)
		return -1;
	
	if (fddescs->get_pool_size() > fddesc_size)
		grow_poll_buffer();

	fdd_info->fd = fd;
	fdd_info->flags = FDIF_VALID | flags;
	fdd_info->uptr = udata;

	event.events = events;
	event.data.ptr = fdd_info;
	
	int rc = epoll_ctl(epoll_handle, EPOLL_CTL_ADD, fd, &event);
	if (rc < 0) {
		_LSYSERROR("EPOLL_CTL_ADD error");
		free_fddinfo(fdd_info);
		return -1;
	}

	return 0;
}


/*************************************************************************//**
**
*/
int EPollDescManager::rem_fd(struct fddesc_info * const fddi)
{
	int rc;
	
	if (fddi == 0)
		return -1;

	rc = epoll_ctl(epoll_handle, EPOLL_CTL_DEL, fddi->fd, 0);
	if (rc < 0) {
		_LSYSERROR("EPOLL_CTL_DEL error");
		return -1;
	}

	free_fddinfo(fddi);
	if (fddescs->get_pool_size() < fddesc_size)
		trim_poll_buffer();
	
	return 0;
}


/*************************************************************************//**
**
*/
int EPollDescManager::rem_fd(struct epoll_event * const ev) {
	if (ev == 0)
		return -1;
	return rem_fd((struct fddesc_info *)ev->data.ptr);
}


/*************************************************************************//**
**
*/
int EPollDescManager::rem_fd(int const fd, void **uptr)
{
	struct fddesc_info * fdd_info = find_fddinfo(fd);
	
	if (uptr != 0)
		*uptr = 0;
	
	if (fdd_info != 0) {
		if (uptr != 0)
			*uptr = fdd_info->uptr;
		
		return rem_fd(fdd_info);
	}
	_VBL(4) << "rem_fd no descriptor for fd " << fd;
	
	return -1;
}


/*************************************************************************//**
**
*/
int EPollDescManager::add_socket(int fd, void *udata, bool is_server)
{
	uint32_t events = EPOLLIN | EPOLLPRI; // EPOLLET; // Make fd events edge triggered
	uint32_t flags = FDIF_VALID | (is_server ? FDIF_LISTENING : 0);
	
	_VBL(3) <<  "fd:" << fd << " udata:" << udata << " svr:" << (is_server ? 'Y' : 'N');
	return add_fd(fd, events, udata, flags);
}


/*************************************************************************//**
**
*/
EPollDescManager::event_iterator
EPollDescManager::wait_for_events(struct timespec *tout, sigset_t *blksig)
{
	int timeout = tout->tv_sec * 1000 + tout->tv_nsec / 1000000;
	int maxevents = fddescs->get_pool_size();
	
	if (maxevents <= 0) {
		nanosleep(tout, 0);
		return 0;
	}
	
	ready_count = epoll_pwait(epoll_handle, epoll_fddesc, maxevents, timeout, blksig);
	if (ready_count == -1) {
		_LSYSERROR("epoll_pwait error");
		ready_count = 0;
	}
	return ready_count;
}


EPollDescManager::event_iterator
EPollDescManager::wait_for_events(struct timespec *tout)
{
	int timeout = tout->tv_sec * 1000 + tout->tv_nsec / 1000000;
	int maxevents = fddescs->get_pool_size();
	
	if (maxevents <= 0) {
		nanosleep(tout, 0);
		return 0;
	}
	
	ready_count = epoll_wait(epoll_handle, epoll_fddesc, maxevents, timeout);
	if (ready_count == -1) {
		_LSYSERROR("epoll_wait error");
		ready_count = 0;
	}
	return ready_count;
}

/*************************************************************************//**
**
*/
EPollDescManager::event_descriptor
EPollDescManager::get_first_event()
{
	event_index = 0;
	return static_cast<event_descriptor>(epoll_fddesc);
}


EPollDescManager::event_descriptor
EPollDescManager::get_next_event()
{
	event_index++;
	if (event_index >= ready_count)
		return 0;
	
	return static_cast<event_descriptor>(epoll_fddesc + event_index);
}
