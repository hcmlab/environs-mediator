/**
 *	Platform interop definitions
 * ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
 * @remarks
 *
 * This file is part of the Environs framework developed at the
 * Lab for Human Centered Multimedia of the University of Augsburg.
 * http://hcm-lab.de/environs
 *
 * Environ is free software; you can redistribute it and/or modify
 * it under the terms of the Eclipse Public License v1.0.
 * A copy of the license may be obtained at:
 * http://www.eclipse.org/org/documents/epl-v10.html
 * --------------------------------------------------------------------
 */
#pragma once
#ifndef INCLUDE_HCM_ENVIRONS_INTEROP_THREADS_H
#define INCLUDE_HCM_ENVIRONS_INTEROP_THREADS_H
#include "Interop.h"

/*
*	USE_PTHREADS_FOR_WINDOWS
*	Enable this flag to replace windows threads with posix threads
*/
//#define USE_PTHREADS_FOR_WINDOWS

/*
*	USE_PTHREADS_FOR_WINDOWS
*	Enable this flag to replace mutex objects through critical sections on windows
*/
#ifdef _WIN32
#define	USE_CRIT_SEC_MUTEX
#endif

/*
*	WAIT_TIME_FOR_THREAD_CLOSING
*	The time in miliseconds to wait for a thread to close by its own
*/
#define	WAIT_TIME_FOR_THREAD_CLOSING		5000

/*
*	WAIT_TIME_FOR_MUTEX_ACQUIRE
*	The timeout in miliseconds for a mutex to be acquired by the callee
*/
#define WAIT_TIME_FOR_MUTEX_ACQUIRE			INFINITE

/*
*	WAIT_TIME_FOR_RECEIVING_TCP_ACK
*	The timeout in miliseconds for receiving a response after sending a tcp message
*/
#define WAIT_TIME_FOR_RECEIVING_TCP_ACK		2500

/*
*	WAIT_TIME_FOR_CONNECTIONS
*	The timeout in miliseconds for a connect to succeed
*/
#define WAIT_TIME_FOR_CONNECTIONS			3500

/*
*	WAIT_TIME_FOR_RECEIVING_TCP_ACK
*	The timeout in miliseconds for a receiving during the handshake phase
*/
#define WAIT_TIME_FOR_RECEIVING_TCP_MSG		10


/*
*	//////// Declarations outside of namespace environs
*	- Include files
*/
#ifdef _WIN32
// Prevent windows.h to include winsock (winsock has to be included before windows.h)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef CLI_CPP
#include "windows.h"
#endif

#ifdef WINDOWS_PHONE
#include "winbase.h"

#define _CRT_USE_WINAPI_FAMILY_DESKTOP_APP
#include <process.h>
#undef _CRT_USE_WINAPI_FAMILY_DESKTOP_APP
#endif

#ifdef USE_PTHREADS_FOR_WINDOWS
#include <pthread.h>
#endif

#else	//---- NTDDI_WIN8
/// Include pthread for platforms other than windows
#include <pthread.h>
/// Semaphores are part of the POSIX standard
/// Still, we do need to consider them differently for platforms
/// i.e. Android does not implement sem_open (named sems), while iOS does not support sem_init (unnamed sems)
#include <semaphore.h>
#endif	//---- NTDDI_WIN8


/**
*	Declarations inside of namespace environs
*	- Include files
*/
/** Place declarations to global namespace for plain C */
#ifdef __cplusplus

namespace environs
{
#endif

    
	extern bool InitInteropThread ();
	extern void DisposeInteropThread ();

#ifdef _WIN32  // _WIN32
	/*
	* Windows specific definintions
	*/

#	ifdef USE_PTHREADS_FOR_WINDOWS

#		define pthread_reset(thread)			memset(&thread,0,sizeof(pthread_t))
#		define pthread_close(threadID)			
//#		define pthread_mutex_valid(m)			true	
#		define pthread_wait_fail(val)			(val != 0)

#	else
#		ifdef WINDOWS_PHONE
			/*
			* Sleep
			*/
			void Sleep ( DWORD millis );
#		endif

		/*
		* Threads
		*/
#		define pthread_reset(thread)				thread = NULL_ptr

#		ifdef CLI_CPP
#			define pthread_mvalid(m)				(m != NULL_ptr && m->ThreadState == ThreadState::Stopped)
#			define pthread_t						Thread ^
#			define pthread_param_t					System::Object ^
#			define pthread_close(threadID)			
#			define pthread_detach_handle(threadID)	
#		else
#			define pthread_mvalid(m)				(true)
#			define pthread_t						HANDLE
#			define pthread_param_t					void *
#			define pthread_close(threadID)			CloseHandle ( threadID )
#			define pthread_detach_handle(threadID)	CloseHandle ( threadID )
#		endif

#		ifdef WINDOWS_PHONE
#			define pthread_join(thread,retval)		WaitForSingleObjectEx ( thread, INFINITE, TRUE )
#			define pthread_cancel(threadID)			_endthread();
#		elif CLI_CPP
#			define pthread_join(thread,retval)		thread->Join ()
#			define pthread_cancel(thread)			thread->Abort ()
#		else
#			define pthread_join(thread,retval)		WaitForSingleObject ( thread, INFINITE )
#		endif

#		define pthread_wait_fail(val)			(val != WAIT_OBJECT_0)
#		define pthread_equal(val,arg)			(val == GetThreadId(arg))
#		define PTHREAD_THREAD_INITIALIZER		NULL_ptr
#		define pthread_setname_current(name)	
		//#define pthread_self()					GetCurrentThreadId()

#		ifdef WINDOWS_PHONE
#			define pthread_create(threadID,s0,startRoutine,arg) \
				((*threadID = (HANDLE)_beginthread ( (void (__cdecl *) (void *))startRoutine, 64000, arg )) == 0)
#		elif CLI_CPP
			using namespace System::Threading;

			int pthread_create_cli ( pthread_t % threadID, void *, System::Threading::ParameterizedThreadStart ^ startRoutine, pthread_param_t arg );

#			define pthread_create(threadID,s0,startRoutine,arg) \
				pthread_create_cli ( threadID, s0, gcnew ParameterizedThreadStart ( startRoutine ), arg )
#		else
#			define pthread_create(threadID,s0,startRoutine,arg) \
				((*threadID = CreateThread ( 0, 0, (LPTHREAD_START_ROUTINE) startRoutine, (LPVOID) arg, 0, 0 )) == 0)
#		endif

		/*
		* Events
		*/
#		ifdef CLI_CPP
#			define	pthread_cond_t					System::Threading::AutoResetEvent ^
#		else
#			define	pthread_cond_t					HANDLE	
#		endif
#		define	pthread_cond_mutex_t				void *

#		ifdef WINDOWS_PHONE
#			define PTHREAD_COND_INITIALIZER			CreateEventEx ( NULL, NULL, NULL, NULL )
#			define pthread_cond_init(e,d)			((*e = CreateEventEx ( NULL, NULL, NULL, NULL )) == NULL)
#			define pthread_cond_manual_init(e,d)	(*e = CreateEventEx ( NULL, NULL, CREATE_EVENT_MANUAL_RESET, NULL ))
#			define pthread_cond_manual_wait(e,m)	WaitForSingleObjectEx ( *e, INFINITE, TRUE )
#			define pthread_cond_wait_time(e,m,t)	(WaitForSingleObjectEx ( *e, t, TRUE ) != WAIT_OBJECT_0)
#		else
#			define PTHREAD_COND_INITIALIZER			CreateEvent ( NULL, FALSE, FALSE, NULL )
#			define pthread_cond_init(e,d)			((*e = CreateEvent ( NULL, FALSE, FALSE, NULL )) == NULL)
#			define pthread_cond_manual_init(e,d)	((*e = CreateEvent ( NULL, TRUE, FALSE, NULL )) == NULL)
#			define pthread_cond_manual_wait(e,m)	WaitForSingleObject ( *e, INFINITE )

#			ifdef CLI_CPP
#				define pthread_cond_wait_time(e,m,t)	!e->WaitOne (t)
#			else
#				define pthread_cond_wait_time(e,m,t)	(WaitForSingleObject ( *e, t ) != WAIT_OBJECT_0)
#			endif
#		endif

#		define pthread_cond_mutex_init(e,d)			false
		//#define pthread_cond_mutex_valid(m)		false	

#		define pthread_cond_mutex_destroy(m)		false

#		define pthread_cond_mutex_lock(m)			false
#		define pthread_cond_mutex_unlock(m)			false
#		define pthread_cond_prepare(e)				ResetEvent ( *e )
#		define pthread_cond_broadcast(e)			pthread_cond_signal(e)
#		define pthread_cond_signal_checked(e)		if (*e) SetEvent ( *e )
#		define pthread_cond_prepare_checked(e)		if (*e) ResetEvent ( *e )
#		define pthread_cond_destroy(e)				(!CloseHandle ( *e ) || ((*e = 0) != 0))
#		define pthread_cond_valid(e)				(e)    

#		ifdef CLI_CPP
#			define pthread_cond_signal(e)			!e->Set()
#		else
#			define pthread_cond_signal(e)			!SetEvent ( *e )
#		endif
		/*
		* Semaphore
		*/
#		define sem_tp								HANDLE
		//#define sem_fail(exp)						((exp) == 0)
#		define env_sem_post(sem)					ReleaseSemaphore ( sem, 1, 0 )
#		define env_sem_posts(sem)					env_sem_post ( sem )
#		ifdef WINDOWS_PHONE
#			define env_sem_wait(sem)				(WaitForSingleObjectEx ( sem, INFINITE, TRUE ) == WAIT_OBJECT_0)
#		else
#			define env_sem_wait(sem)				(WaitForSingleObject ( sem, INFINITE ) == WAIT_OBJECT_0)
#		endif
		//#define sem_trywait(sem)					(WaitForSingleObject ( sem, 0 ) == WAIT_OBJECT_0)

#		define env_sem_dispose(sem)					CloseHandle ( sem )

		/*
		* Mutex
		*/
#		ifdef USE_CRIT_SEC_MUTEX	 // USE_CRIT_SEC_MUTEX
#			ifdef CLI_CPP
#				define pthread_mutex_t				System::Object
#			else
			/*
			* Use critical section objects for windows mutex
			*/

#			define pthread_mutex_t					CRITICAL_SECTION
			//#define pthread_mutex_valid(m)		true	

#			ifdef WINDOWS_PHONE
#				define pthread_mutex_init(m,d)			
#			else
				extern INCLINEFUNC bool pthread_mutex_init ( CRITICAL_SECTION  * critSEc, void * arg );
				//#define pthread_mutex_init(m,d)	InitializeCriticalSection(m)
#			endif

			extern INCLINEFUNC bool pthread_mutex_destroy ( CRITICAL_SECTION  * critSEc );
			//#define pthread_mutex_destroy(m)		DeleteCriticalSection(m))

#			ifdef WINDOWS_PHONE
#					define pthread_cond_wait(e,m)	LeaveCriticalSection(m); WaitForSingleObjectEx ( *e, INFINITE, TRUE ); EnterCriticalSection (m);
#			else
#				define pthread_cond_wait(e,m)		LeaveCriticalSection(m); WaitForSingleObject ( *e, INFINITE ); EnterCriticalSection (m);
#			endif

#if _MSC_VER >= 1800
			_When_(return == 0, _Acquires_lock_(*lock))
#endif
			extern INCLINEFUNC int pthread_mutex_lock (		pthread_mutex_t OBJ_ptr lock );
			
#if _MSC_VER >= 1800
			_When_(return == 0, _Acquires_lock_(*lock))
#endif
			extern INCLINEFUNC int pthread_mutex_trylock (	pthread_mutex_t OBJ_ptr lock );
			
#if _MSC_VER >= 1800
			_When_(return == 0, _Releases_lock_(*lock))
#endif
			extern INCLINEFUNC int pthread_mutex_unlock (	pthread_mutex_t OBJ_ptr lock );

#		endif

#	else // USE_CRIT_SEC_MUTEX
		/*
		* Use mutex objects for windows mutex
		*/
#		define pthread_mutex_t					HANDLE
#		define pthread_mutex_valid(m)			m
#		define PTHREAD_MUTEX_INITIALIZER		CreateMutex ( NULL, FALSE, NULL )
#		define pthread_mutex_init(m,d)			( (*m = CreateMutex ( NULL, FALSE, NULL )) == 0 )
#		define pthread_mutex_destroy(m)		CloseHandle ( *m )
#		define pthread_mutex_lock(m)			(WaitForSingleObject ( *m, WAIT_TIME_FOR_MUTEX_ACQUIRE ) != WAIT_OBJECT_0)
#		define pthread_mutex_unlock(m)			(!ReleaseMutex ( *m ))

#		define pthread_cond_wait(e,m)			ReleaseMutex(*m); WaitForSingleObject ( *e, INFINITE ); WaitForSingleObject ( *m, WAIT_TIME_FOR_MUTEX_ACQUIRE );

#	endif // USE_CRIT_SEC_MUTEX


#	ifdef CLI_CPP

#	else
		extern INCLINEFUNC int pthread_cond_timedwait (		pthread_cond_t	* cond, pthread_mutex_t OBJ_ptr lock, unsigned int * timeout );
		extern INCLINEFUNC int pthread_cond_timedwait_sec ( pthread_cond_t	* cond, pthread_mutex_t OBJ_ptr lock, unsigned int timeout );
		extern INCLINEFUNC int pthread_cond_timedwait_msec ( pthread_cond_t * cond, pthread_mutex_t OBJ_ptr lock, unsigned int timeout );
#	endif

#	define pthread_csec_t                  CRITICAL_SECTION
#	define pthread_csec_init(m,d)			InitializeCriticalSection(m)
#	define pthread_csec_destroy(m)         DeleteCriticalSection(m)
#	define pthread_csec_lock(m)            EnterCriticalSection (m)
#	define pthread_csec_trylock(m)         TryEnterCriticalSection (m)
#	define pthread_csec_unlock(m)          LeaveCriticalSection(m)
		
#	endif // USE_PTHREADS_FOR_WINDOWS

#else 	 // _WIN32 - Section for __APPLE__, ANDROID, _GNUC_
	extern INCLINEFUNC int pthread_cond_timedwait_sec ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int timeout );
	extern INCLINEFUNC int pthread_cond_timedwait_msec ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int timeout );

	/*
	* Android/iOS/MacOS/Linux specific definintions
	*/
#	define pthread_param_t						void *

#	ifdef __APPLE__
#		define pthread_setname_current(name)	pthread_setname_np ( name );
#	else
#		define pthread_setname_current(name)	pthread_setname_np ( pthread_self (), name );
#	endif

#	ifdef _OPEN_THREADS
#		define pthread_detach_handle(threadID)	pthread_detach ( &threadID )
#	else
#		define pthread_detach_handle(threadID)	pthread_detach ( threadID )
	#endif

#	define pthread_reset(thread)				memset(&thread,0,sizeof(pthread_t))

#	ifdef ANDROID
		extern long ___sync_val_compare_and_swap ( long * destination, unsigned int compare, unsigned int swap );

#		define pthread_mutex_valid(m)			(m.value != 0)
#		define pthread_cond_valid(e)			(true)
#	else
#		define pthread_mutex_valid(m)			(true)
#		define pthread_cond_valid(e)			(true)
#	endif

#define	pthread_cond_mutex_t			pthread_mutex_t
#define pthread_cond_mutex_lock(m)		pthread_mutex_lock(m)
#define pthread_cond_mutex_unlock(m)	pthread_mutex_unlock(m)
#define pthread_cond_prepare(e)			
#define pthread_cond_manual_init(e,d)	pthread_cond_init(e,d)
#define pthread_cond_manual_wait(e,m)	pthread_cond_wait(e,m)
#define pthread_cond_wait_time(e,m,t)	pthread_cond_wait (e,m)
#define pthread_cond_mutex_init(m,d)	pthread_mutex_init(m,d)
#define pthread_cond_mutex_valid(m)		pthread_mutex_valid(m)	
#define pthread_cond_mutex_destroy(m)	pthread_mutex_destroy(m)
#define pthread_cond_signal_checked(e)	if (pthread_cond_valid(*e)) pthread_cond_signal(e)
#define pthread_cond_prepare_checked(e)

#define pthread_close(threadID)
#define pthread_wait_fail(val)			(val != 0)

#define pthread_csec_t                  pthread_mutex_t
#define pthread_csec_init(m,d)			pthread_mutex_init(m,d)
#define pthread_csec_destroy(m)         pthread_mutex_destroy(m)
#define pthread_csec_lock(m)            pthread_mutex_lock (m)
#define pthread_csec_trylock(m)         !pthread_mutex_trylock (m)
#define pthread_csec_unlock(m)          pthread_mutex_unlock(m)

	/*
	* Semaphore
	*/

	//#define sem_fail(exp)					((exp) == -1)

#ifdef __APPLE__
#define sem_tp							sem_t *
#define env_sem_post(sem)				(sem_post ( sem ) != -1)
#define env_sem_posts(sem)				sem_post ( sem )
#define env_sem_wait(sem)				(sem_wait ( sem ) != -1)
#define env_sem_dispose(sem)			(sem_close ( sem ) != -1)

#else
#define sem_tp							sem_t
#define env_sem_post(sem)				(sem_post ( &(sem) ) != -1)
#define env_sem_posts(sem)				sem_post ( &(sem) )
#define env_sem_wait(sem)				(sem_wait ( &(sem) ) != -1)
#define env_sem_dispose(sem)			(sem_destroy ( &(sem) ) != -1)
#endif


#endif	 // _WIN32

#ifdef __cplusplus
	/*
	* pthread Extensions for Environs.
	*/
#	ifdef CLI_CPP

#	else
	extern INCLINEFUNC bool pthread_wait_one ( pthread_cond_t &cond, pthread_mutex_t OBJ_ref lock );
	extern INCLINEFUNC bool pthread_is_self_thread ( pthread_t thread );
	extern INCLINEFUNC bool pthread_valid ( pthread_t thread );

	/*
	* POSIX Semaphore extensions for Environs.
	*/
	extern bool env_sem_create ( sem_tp * sem, int iniVal, const char * name, unsigned int name1, int name2, int name3 );


	extern void DisposeThread ( pthread_t &threadID, const char * threadName );
#	endif

#endif

#ifdef CLI_CPP

	using namespace System::Threading;

#	define MutexInitA(m)			true
#	define MutexDisposeA(m)			true

#	define MutexLockV(m,f)			Monitor::Enter(m)
#	define MutexLockVA(m,f)			Monitor::Enter(%m)

#	define MutexLock(m,f)			MutexLockBool(m,#m,CLASS_NAME,f)
#	define MutexLockA(m,f)			MutexLockBool(%m,#m,CLASS_NAME,f)
	extern bool MutexLockBool ( pthread_mutex_t OBJ_ptr mtx, CString_ptr mutexName, CString_ptr className, CString_ptr funcName );

#	define MutexUnlockV(m,f)		Monitor::Exit(m)
#	define MutexUnlockVA(m,f)		Monitor::Exit(%m)

#	define MutexUnlock(m,f)			MutexUnlockBool(m,#m,CLASS_NAME,f)
#	define MutexUnlockA(m,f)		MutexUnlockBool(%m,#m,CLASS_NAME,f)
	extern bool MutexUnlockBool ( pthread_mutex_t OBJ_ptr mtx, CString_ptr mutexName, CString_ptr className, CString_ptr funcName );

#	define CondInitA(m)				((m = gcnew System::Threading::AutoResetEvent(false)) != NULL_ptr)
#	define CondDisposeA(m)			(m = NULL_ptr)
#else

#	define MutexInit(m)			MutexInitBool(m,#m)
#	define MutexInitA(m)		MutexInitBool(&m,#m)
	extern bool MutexInitBool ( pthread_mutex_t OBJ_ptr mtx, const char * name );

#	define MutexDispose(m)		MutexDisposeBool(m,#m)
#	define MutexDisposeA(m)		MutexDisposeBool(&m,#m)
	extern bool MutexDisposeBool ( pthread_mutex_t OBJ_ptr mtx, const char * name );

#	define MutexLockV(m,f)		MutexLockVoid(m,#m,CLASS_NAME,f)
#	define MutexLockVA(m,f)		MutexLockVoid(&m,#m,CLASS_NAME,f)
	extern void MutexLockVoid ( pthread_mutex_t OBJ_ptr mtx, const char * mutexName, const char * className, const char * funcName );

#	define MutexUnlockV(m,f)	MutexUnlockVoid(m,#m,CLASS_NAME,f)
#	define MutexUnlockVA(m,f)	MutexUnlockVoid(&m,#m,CLASS_NAME,f)
	extern void MutexUnlockVoid ( pthread_mutex_t OBJ_ptr mtx, const char * mutexName, const char * className, const char * funcName );

#	define MutexLock(m,f)		MutexLockBool(m,#m,CLASS_NAME,f)
#	define MutexLockA(m,f)		MutexLockBool(&m,#m,CLASS_NAME,f)
	extern bool MutexLockBool ( pthread_mutex_t OBJ_ptr mtx, const char * mutexName, const char * className, const char * funcName );

#	define MutexUnlock(m,f)		MutexUnlockBool(m,#m,CLASS_NAME,f)
#	define MutexUnlockA(m,f)	MutexUnlockBool(&m,#m,CLASS_NAME,f)
	extern bool MutexUnlockBool ( pthread_mutex_t OBJ_ptr mtx, const char * mutexName, const char * className, const char * funcName );


#	define CondInit(m)			CondInitBool(m,#m)
#	define CondInitA(m)			CondInitBool(&m,#m)
	extern bool CondInitBool ( pthread_cond_t * mtx, const char * name );

#	define CondDispose(m)		CondDisposeBool(m,#m)
#	define CondDisposeA(m)		CondDisposeBool(&m,#m)
	extern bool CondDisposeBool ( pthread_cond_t * mtx, const char * name );
#endif

    
#ifdef __cplusplus
}
#endif
#endif // INCLUDE_HCM_ENVIRONS_INTEROP_H
