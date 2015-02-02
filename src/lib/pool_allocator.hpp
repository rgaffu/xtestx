/**
******************************************************************************
* @file    pool_allocator.hpp
* @brief   A simple template class to implement pool memory allocators
*
* @author  
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
#ifndef __POOLALLOC_HPP_INCLUDED
#define __POOLALLOC_HPP_INCLUDED

#ifndef __cplusplus
#error message_queue.hpp is C++ only.
#endif

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <new>
#include <logging.hpp>


/*************************************************************************//**
**
*/
template <class T>
class PoolAllocator
{
public:
	PoolAllocator(size_t slab_count):
		supplied_buffer(false),
		slab_size(sizeof(T)),
		slab_count(slab_count),
		alloc_count(0),
		slabs(0),
		_head(0),
		_tail(0)
	{
		// It does not make sense to use a pool allocator for a
		// small type, but let's check for minimal size anyway...
		if (slab_size < sizeof(void *))
			slab_size = sizeof(void *);
		
		block_allocate();
		init_free_list();
	}
	
	PoolAllocator(void * storage, size_t slab_count):
		supplied_buffer(true),
		slab_size(sizeof(T)),
		slab_count(slab_count),
		alloc_count(0),
		slabs(storage),
		_head(0),
		_tail(0)
	{
		// Don't check for minimal size, since if you
		// use this constructor, you shall know what to do.
		init_free_list();
	}
	
	virtual ~PoolAllocator() {
		if ((!supplied_buffer) && (slabs != 0))
			free_mem(slabs);
	}
	
	void dump_free_list()
	{
		void ** p = (void **)_head;
		CVLOG(3, "memory") << "free slab list head:" << _head << " tail:" << _tail;
		while (p) {
			void ** next = (void **)(*p);
			CVLOG(3, "memory") << "\tobj @" << p << " -> " << next;
			p = next;
		}
	}
	
	T * alloc_object() {
		void * p = _head;
		if (p == 0) {
			grow_pool();
			p = _head;
		}
		if (p != 0) {
			if (_head == _tail)
				_tail = _head = *((void **)_head);
			else
				_head = *((void **)_head);
			CVLOG(2, "memory") << "allocated @" << p;
			alloc_count++;
			return new (p) T();
		}
		return 0;
	};

	void free_object(T * ptr) {
		if (ptr == 0)
			return;
		ptr->~T();
		alloc_count--;
#ifdef _STACK_FREE
		// Add released block on list head (stack behaviour)
		*((void **)ptr) = _head;
		_head = ptr;
#else /*_QUEUE_FREE */
		// Add released block on list tail (queue behaviour)
		*((void **)ptr) = 0;
		if (_tail == 0) {
			_head = _tail = ptr;
		} else {
			*((void **)_tail) = ptr;
			_tail = ptr;
		}
#endif
		CVLOG(2, "memory") << "released @" << ptr;
		trim_pool();
	};
	
	uint32_t get_pool_size() const {
		return slab_count;
	}
	uint32_t get_pool_usage() const {
		return alloc_count;
	}


protected:
	virtual void * alloc_mem(size_t const msize) {
		void * const ptr = calloc(1, msize);
		CVLOG(2, "memory") << "allocated heap memory @" << ptr << " size:" << msize;
		return ptr;
	}
	virtual void free_mem(void * const ptr) {
		CVLOG(2, "memory") << "releasing heap memory @" << ptr;
		free(ptr);
	}
	
	virtual void grow_pool() {
	}
	virtual void trim_pool() {
	}
	
	T * get_first() const {
		return reinterpret_cast<T*>(slabs);
	}
	T * get_next(T * const p) const {
		T * const limit = get_first() + slab_count;
		T * next = p + 1;
		if (next >= limit)
			return 0;
		return next;
	}


private:
	void block_allocate() {
		if (slabs == 0)
			slabs = alloc_mem(slab_count * slab_size);
	}

	void init_free_list() {
		void ** pred;
		alloc_count = 0;
		_head = slabs;
		pred = (void **)slabs;
		for (unsigned i=1; i < slab_count; i++) {
			void ** curr = (void**)((uint8_t*)slabs + (i * slab_size));
			*curr = 0;
			*pred = curr;
			_tail = pred;
			pred = curr;
		}
		dump_free_list();
	}

private:
	bool supplied_buffer;
	size_t slab_size;
	uint32_t slab_count;
	uint32_t alloc_count;
	void *slabs;
	void *_head;
	void *_tail;
};

/****************************************************************************/

#endif /* __POOLALLOC_HPP_INCLUDED */
/* EOF */

