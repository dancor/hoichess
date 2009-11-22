/* $Id: thread.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/thread.h
 *
 * Copyright (C) 2005 Holger Ruckdeschel <holger@hoicher.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */
#ifndef THREAD_H
#define THREAD_H

#include "common.h"

#if defined(HAVE_PTHREAD)
# include <pthread.h>
#elif defined(WIN32)
# include <windows.h>
#else
# error no thread support is available
#endif

/*****************************************************************************
 *
 * Mutex Class
 *
 *****************************************************************************/

class Mutex {
      private:
#if defined(HAVE_PTHREAD)
	pthread_mutex_t mtx;
#elif defined(WIN32)
	HANDLE hndl; 
#endif

      public:
	inline Mutex();
	inline ~Mutex();

      public:
	inline void lock();
	inline void unlock();
};


inline Mutex::Mutex()
{
#if defined(HAVE_PTHREAD)
	pthread_mutex_init(&mtx, NULL);
#elif defined(WIN32)
	hndl = CreateMutex(NULL, FALSE, NULL);
#endif
}

inline Mutex::~Mutex()
{
#if defined(HAVE_PTHREAD)
	pthread_mutex_destroy(&mtx);
#elif defined(WIN32)
#endif
}

inline void Mutex::lock()
{
#if defined(HAVE_PTHREAD)
	pthread_mutex_lock(&mtx);
#elif defined(WIN32)
	DWORD res = WaitForSingleObject(hndl, INFINITE);
	ASSERT(res == WAIT_OBJECT_0);
#endif
}

inline void Mutex::unlock()
{
#if defined(HAVE_PTHREAD)
	pthread_mutex_unlock(&mtx);
#elif defined(WIN32)
	bool res = ReleaseMutex(hndl);
	ASSERT(res);
#endif
}


/*****************************************************************************
 *
 * Thread Class
 *
 *****************************************************************************/

class Thread {
      private:
#if defined(HAVE_PTHREAD)
	pthread_t threadid;
#elif defined(WIN32)
	HANDLE hndl;
#endif

	void * (*startfunc)(void *);
	
      public:
	inline Thread(void * (*startfunc)(void *));
	inline ~Thread();

      public:
	inline void start(void * arg);
	inline void wait();

      private:
#if defined(WIN32)
	struct win32_startfunc_wrapper_arg {
		Thread * thread;
		void * real_arg;
	};
	static inline DWORD WINAPI win32_startfunc_wrapper(LPVOID arg);
#endif
};
	

inline Thread::Thread(void * (*_startfunc)(void *))
{
	startfunc = _startfunc;

#if defined(HAVE_PTHREAD)
#elif defined(WIN32)
#endif
}

inline Thread::~Thread()
{
#if defined(HAVE_PTHREAD)
#elif defined(WIN32)
#endif
}

inline void Thread::start(void * arg)
{
#if defined(HAVE_PTHREAD)
	pthread_create(&threadid, NULL, startfunc, arg);
#elif defined(WIN32)
	struct win32_startfunc_wrapper_arg * a =
		new struct win32_startfunc_wrapper_arg;
	a->thread = this;
	a->real_arg = arg;
	DWORD dwThreadId;
	hndl = CreateThread(0, 0, win32_startfunc_wrapper, (LPVOID) a, 0,
			&dwThreadId);
#endif
}

inline void Thread::wait()
{
#if defined(HAVE_PTHREAD)
	pthread_join(threadid, NULL);
	threadid = 0;
#elif defined(WIN32)
	WaitForSingleObject(hndl, INFINITE);
	CloseHandle(hndl);
	hndl = INVALID_HANDLE_VALUE;
#endif
}

#if defined(WIN32)
inline DWORD WINAPI Thread::win32_startfunc_wrapper(LPVOID arg)
{
	struct win32_startfunc_wrapper_arg * a =
		(struct win32_startfunc_wrapper_arg *) arg;
	(*(a->thread->startfunc))(a->real_arg);
	delete a;
	return 0;
}
#endif

#endif // THREAD_H
