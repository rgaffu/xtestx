/*Include only once */
#ifndef __EPOLLFDSMGR_HPP_INCLUDED
#define __EPOLLFDSMGR_HPP_INCLUDED

#ifndef __cplusplus
#error message_queue.hpp is C++ only.
#endif

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <pool_allocator.hpp>

using namespace std;


/*************************************************************************//**
**
**
**
*****************************************************************************/

class EPollDescManager {
public:
	static const uint32_t FDIF_NONSOCKET  = (1L << 2);

private:
	static const uint32_t FDIF_VALID      = (1L << 0);
	static const uint32_t FDIF_LISTENING  = (1L << 1);

	struct fddesc_info {
		~fddesc_info() {
			flags = 0;
		}
		int fd;
		uint32_t flags;
		void * uptr;
	};
	
	typedef PoolAllocator<fddesc_info> FDDesc_PoolAllocator;
	class FDDescAllocator : public FDDesc_PoolAllocator {
		friend class EPollDescManager;
	public:
		FDDescAllocator(size_t _size):
			FDDesc_PoolAllocator(_size) {}

		fddesc_info * find_fd(int const fd) {
			fddesc_info * t = get_first();
			while (t) {
				if ((t->flags & FDIF_VALID) && (t->fd == fd))
					return t;
				t = get_next(t);
			}
			return 0;
		}
	};
	
	typedef FDDescAllocator fddesc_pool;

	struct _event : public epoll_event {
		bool is_signal() const {
			return (((struct fddesc_info *)data.ptr)->flags & FDIF_NONSOCKET) != 0;
		}
		bool is_error() const {
			return (events & EPOLLERR);
		}
		bool is_hangup() const {
			return (events & EPOLLHUP);
		}
		bool is_incoming_connection() const {
			return (events & (EPOLLIN|EPOLLPRI)) &&
				   (((struct fddesc_info *)data.ptr)->flags & FDIF_LISTENING);
		}
		bool is_incoming_data() const {
			return (events & (EPOLLIN|EPOLLPRI));
		}
		bool has_priority() const {
			return (events & EPOLLPRI);
		}
		int get_fd() const {
			return ((struct fddesc_info *)data.ptr)->fd;
		}
		void* get_udata() const {
			return ((struct fddesc_info *)data.ptr)->uptr;
		}
		int rem_socket(EPollDescManager * mgr) {
			return mgr->rem_socket(this);
		};
	};

public:
	enum event_flags {
		EV_IN  = EPOLLIN,  /* There is data to read.  */
		EV_PRI = EPOLLPRI, /* There is urgent data to read.  */
		EV_OUT = EPOLLOUT, /* Writing now will not block.  */
		EV_RDNORM = EPOLLRDNORM, /* Normal data may be read.  */
		EV_RDBAND = EPOLLRDBAND, /* Priority data may be read.  */
		EV_WRNORM = EPOLLWRNORM, /* Writing now will not block.  */
		EV_WRBAND = EPOLLWRBAND, /* Priority data may be written.  */
		EV_MSG   = EPOLLMSG,
		EV_ERR   = EPOLLERR,  /* Error condition.  */
		EV_HUP   = EPOLLHUP,  /* Hung up.  */
		EV_RDHUP = EPOLLRDHUP
	};
	static const uint32_t ERROR_EVENT = 0xFFFFFFFFU;
	typedef uint32_t event_iterator;
	typedef struct _event * event_descriptor;
	
public:
	EPollDescManager();
	virtual ~EPollDescManager();

	int add_fd(int fd, uint32_t events, void * udata, uint32_t flags);
	int rem_fd(int fd, void**);
	int rem_fd(struct epoll_event * ev);
	
	void* get_fd_udata(int const fd) {
		struct fddesc_info * fdd_info = find_fddinfo(fd);
		if (fdd_info == 0)
			return 0;
		return fdd_info->uptr;
	}
	
	int add_socket(int const fd, void * const udata) {
		return add_socket(fd, udata, false);
	}
	int add_server_socket(int const fd, void * const udata) {
		return add_socket(fd, udata, true);
	}
	int rem_socket(int const fd) {
		return rem_fd(fd, 0);
	}
	int rem_socket(struct epoll_event * const ev) {
		return rem_fd(ev);
	}
	
	event_iterator wait_for_events(struct timespec *);
	event_iterator wait_for_events(struct timespec *, sigset_t *);
	
	event_descriptor get_first_event();
	event_descriptor get_next_event();

	void close_all();

protected:
	int add_socket(int fd, void *udata, bool is_server);
	int rem_fd(struct fddesc_info * fddi);


private:
	void _dump_fds();
	void _dump_free_fdd();
	
	inline struct fddesc_info* alloc_fddinfo() {
		return fddescs->alloc_object();
	}
	inline void free_fddinfo(struct fddesc_info * fddi_ptr) {
		fddescs->free_object(fddi_ptr);
	}
	inline struct fddesc_info* find_fddinfo(int const fd) {
		return fddescs->find_fd(fd);
	}
	int grow_poll_buffer() {
		return -1;
	}
	int trim_poll_buffer() {
		return 0;
	}


private:
	unsigned fddesc_size;
	int epoll_handle;

	int ready_count;
	int event_index;
	struct epoll_event *epoll_fddesc;
	
	fddesc_pool * fddescs;
};

/****************************************************************************/

#endif /* __EPOLLFDSMGR_HPP_INCLUDED */
/* EOF */
