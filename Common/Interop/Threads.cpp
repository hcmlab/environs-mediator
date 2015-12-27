/**
 *	Interop thread helper implementation
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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
//#   define DEBUGVERBLocks
#endif

#if ( !defined(NDEBUG) )
#	define	USE_LOCK_LOG
#endif

#include "./Threads.h"

#ifndef CLI_CPP
#	include <errno.h>
#	include <stdio.h>
#endif

#ifndef _WIN32
#   include <sys/time.h>
#endif

#include "Environs.Native.h"

#if (defined(ENVIRONS_CORE_LIB) || defined(ENVIRONS_NATIVE_MODULE))

#ifndef CLI_CPP
#   include "Environs.Obj.h"
#endif

#else
#	ifdef CErr
#		undef CErr
#	endif
#	define CErr(msg) printf(msg)

#	ifdef CVerb
#		undef CVerb
#	endif
#	define CVerb(msg) printf(msg)

#	ifdef CVerbArg
#		undef CVerbArg
#	endif
#	define CVerbArg(msg,...) printf(msg,__VA_ARGS__)

#	ifdef CLogArg
#		undef CLogArg
#	endif
#	define CLogArg(msg,...) printf(msg,__VA_ARGS__)

#	ifdef CErrArg
#		undef CErrArg
#	endif
#	define CErrArg(msg,...) printf(msg,__VA_ARGS__)

#	ifdef CWarnArg
#		undef CWarnArg
#	endif
#	define CWarnArg(msg,...) printf(msg,__VA_ARGS__)

#	ifdef DEBUGVERBLocks
#		ifdef CVerbLockArg
#			undef CVerbLockArg
#		endif
#		define CVerbLockArg(msg,...) printf(msg,__VA_ARGS__)
#	else
#		ifdef CVerbLockArg
#			undef CVerbLockArg
#		endif
#		define CVerbLockArg(msg,...)
#	endif
#endif

#ifndef DEBUGVERBLocks
#	ifdef CVerbLockArg
#		undef CVerbLockArg
#	endif
#	define CVerbLockArg(msg,...)
#endif


#ifdef USE_PTHREADS_FOR_WINDOWS
#   pragma comment ( lib, "pthreadVC2.lib" )
#endif

// The TAG for prepending to log messages
#define CLASS_NAME	"Threads. . . . . . . . ."

namespace environs
{
#ifdef ANDROID
	INCLINEFUNC long ___sync_val_compare_and_swap ( long * destination, unsigned int compare, unsigned int swap )
	{
		return __sync_val_compare_and_swap ( destination, compare, swap );
	}
#endif

#ifdef WINDOWS_PHONE

	static HANDLE threadSleepEvent = 0;

	INCLINEFUNC void Sleep ( DWORD millis )
	{
		WaitForSingleObjectEx ( threadSleepEvent, millis, false );
	}
#endif

	bool InitInteropThread ()
	{
#ifdef WINDOWS_PHONE
		if ( !threadSleepEvent ) {
			threadSleepEvent = CreateEventEx ( 0, 0, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS );
		}
		return (threadSleepEvent != 0);
#else
		return true;
#endif
	}

	void DisposeInteropThread ()
	{
#ifdef WINDOWS_PHONE
		if ( threadSleepEvent ) {
			CloseHandle ( threadSleepEvent );
			threadSleepEvent = 0;
		}
#endif
	}

#ifdef CLI_CPP

	int pthread_create_cli ( pthread_t % threadID, void *, System::Threading::ParameterizedThreadStart ^ startRoutine, pthread_param_t arg  )
	{
		threadID = gcnew Thread ( startRoutine );
		if ( threadID == nullptr )
			return 1;

		threadID->Start ( arg );
		return 0;
	}


	bool MutexLockBool ( pthread_mutex_t ^ mtx, CString_ptr mutexName, CString_ptr className, CString_ptr funcName )
	{
		if ( mtx == nill ) {
			//CErr ( "Lock: lock invalid " + className + "." + funcName + " " + mutexName );
			//MutexErrorLog ( "lock", mutexName, className, funcName );
			return false;
		}
		CVerb ( "Lock: " + className + "." + funcName + " " + mutexName );
		Monitor::Enter ( mtx );
		return true;
	}

	bool MutexUnlockBool ( pthread_mutex_t ^ mtx, CString_ptr mutexName, CString_ptr className, CString_ptr funcName )
	{
		if ( mtx == nill ) {
			//CErr ( "Unlock: lock invalid " + className + "." + funcName + " " + mutexName );
			//MutexErrorLog ( "unlock", mutexName, className, funcName );
			return false;
		}

		CVerb ( "Unlock: " + className + "." + funcName + " " + mutexName );
		Monitor::Exit ( mtx );
		return true;
	}


#else

	INCLINEFUNC bool pthread_valid ( pthread_t thread )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
#ifdef WINDOWS_PHONE
		if ( thread )
			return true;
#else
		DWORD exitCode = 0;

		if ( thread && GetExitCodeThread ( thread, &exitCode ) ) {
			if ( exitCode == STILL_ACTIVE )
				return true;
		}
#endif
#else
#ifdef _WIN32
		if ( thread.p )
			return true;
#else
		if ( thread )
			return true;
#endif
#endif
		return false;
	}


#ifdef USE_CRIT_SEC_MUTEX
	INCLINEFUNC bool pthread_mutex_init ( CRITICAL_SECTION  * critSEc, void * arg )
	{
		InitializeCriticalSection ( critSEc );
		return false;
	}

	INCLINEFUNC bool pthread_mutex_destroy ( CRITICAL_SECTION  * critSEc )
	{
		DeleteCriticalSection ( critSEc );
		return false;
	}
#endif


	INCLINEFUNC bool pthread_is_self_thread ( pthread_t thread )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		if ( !thread )
			return false;
#ifdef WINDOWS_PHONE
		DWORD id = (DWORD)thread;
#else
		DWORD id = GetThreadId ( thread );
#endif
		if ( !id ) // Failed
			return true;

		DWORD our_id = GetCurrentThreadId ();

		return (our_id == id);
#else
		pthread_t our_id = pthread_self ();

		return (memcmp ( &our_id, &thread, sizeof(pthread_t) ) == 0);
#endif
	}


	INCLINEFUNC bool pthread_wait_one ( pthread_cond_t &cond, pthread_mutex_t &lock )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS) 
#ifdef WINDOWS_PHONE
		return (WaitForSingleObjectEx ( cond, INFINITE, TRUE ) == WAIT_OBJECT_0 );
#else
		return (WaitForSingleObject ( cond, INFINITE ) == WAIT_OBJECT_0 );
#endif
#else
		bool ret = false;

		if ( pthread_mutex_lock ( &lock ) )
			return false;

		if ( !pthread_cond_wait ( &cond, &lock ) )
			ret = true;

		pthread_mutex_unlock ( &lock );
		return ret;
#endif
	}
    
    
	INCLINEFUNC int pthread_cond_timedwait_msec ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int timeout )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		return pthread_cond_timedwait ( cond, lock, &timeout );
#else
		return pthread_cond_timedwait_sec ( cond, lock, timeout / 1000 );
#endif
	}
    
    
	INCLINEFUNC int pthread_cond_timedwait_sec ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int timeout )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		unsigned int waitTime = timeout * 1000;
#else
		struct timeval	now;
		struct timespec waitTime;
        
		gettimeofday ( &now, NULL );
        
		waitTime.tv_sec = now.tv_sec + timeout;
		waitTime.tv_nsec = now.tv_usec * 1000;
#endif
		return pthread_cond_timedwait ( cond, lock, &waitTime );
	}


#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)

#ifdef USE_CRIT_SEC_MUTEX
	
#if _MSC_VER >= 1800
	_When_(return == 0, _Acquires_lock_(*lock) )
#endif
	INCLINEFUNC int pthread_mutex_lock ( pthread_mutex_t * lock )
	{
		EnterCriticalSection ( lock );
		return 0;
	}

	
#if _MSC_VER >= 1800
	_When_(return == 0, _Acquires_lock_(*lock))
#endif
	INCLINEFUNC int pthread_mutex_trylock ( pthread_mutex_t * lock )
	{
		return !TryEnterCriticalSection(lock);
	}

	
#if _MSC_VER >= 1800
	_When_(return == 0, _Releases_lock_(*lock))	
#endif
	INCLINEFUNC int pthread_mutex_unlock ( pthread_mutex_t * lock )
	{
		LeaveCriticalSection ( lock );
		return 0;
	}
#endif


	INCLINEFUNC int pthread_cond_timedwait ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int * timeout )
	{
		int ret = 0;
		
#ifdef USE_CRIT_SEC_MUTEX
		LeaveCriticalSection ( lock ); 

#ifdef WINDOWS_PHONE
		if ( WaitForSingleObjectEx ( *cond, *timeout, TRUE ) != WAIT_OBJECT_0 )
#else
		if ( WaitForSingleObject ( *cond, *timeout ) != WAIT_OBJECT_0 )
#endif
			ret = ETIMEDOUT;

		EnterCriticalSection ( lock );
#else
		ReleaseMutex ( *lock ); 

		if ( WaitForSingleObject ( *cond, *timeout ) != WAIT_OBJECT_0 )
			ret = ETIMEDOUT;

		WaitForSingleObject ( *lock, WAIT_TIME_FOR_MUTEX_ACQUIRE );
#endif
		return ret;
	}


#endif


#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4996 )
#endif

	bool env_sem_create ( sem_tp * sem, int iniVal, const char * name, unsigned int name1, int name2, int name3 )
	{
#ifdef _WIN32
#ifdef WINDOWS_PHONE
		static char buffer [48];

		int lenA = sprintf ( buffer, "sem-%s%u-%i-%u", name, name1, name2, name3 );
		if ( lenA <= 0 )
			return false;

		static TCHAR tbuffer [128];
		int lenW = ::MultiByteToWideChar ( CP_ACP, 0, buffer, lenA, tbuffer, lenA );
		if ( lenW <= 0 )
			return false;

		sem_tp tsem = CreateSemaphoreEx ( NULL, 1, 3, tbuffer, 0, SEMAPHORE_ALL_ACCESS );
#else
		sem_tp tsem = CreateSemaphore ( NULL, 0, 3, NULL );
#endif
		if ( !tsem )
			return false;
		*sem = tsem;
#endif
		///
#ifdef __APPLE__
		static char buffer [48];

		sprintf ( buffer, "sem-%s%u-%i-%u", name, name1, name2, name3 );
		sem_unlink ( buffer );

		sem_t * tsem = sem_open ( buffer, O_CREAT | O_EXCL, 0644, iniVal );
		if ( tsem == SEM_FAILED ) {
			return false;
		}

		*sem = tsem;
#endif

#ifdef ANDROID
		int ret = sem_init ( sem, 0, iniVal );
		if ( ret == -1 ){
			return false;
		}
#endif
		return true;
    }
    

	/**
	*	Dispose a thread and reset the threadID variable afterwards.
	*	Make sure that the thread has not bee detached before.
	*	Reset the thread variable stored in threadID on success.
	*
    void DisposeThread ( pthread_t &threadID, const char * threadName )
    {
        CVerb ( "DisposeThread" );
        
         pthread_t thrd = threadID;
         
         pthread_reset ( threadID );
        
        
        if ( !pthread_valid ( thrd ) ) {
            CVerb ( "DisposeThread: Thread is already closed." );
            return;
        }
        
        CLogArg ( "Waiting for %s ...", threadName );
        
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS) && !defined(WINDOWS_PHONE)
        
        //DWORD dw = WaitForSingleObject ( thrd, WAIT_TIME_FOR_THREAD_CLOSING );
        DWORD dw = WaitForSingleObject ( thrd, INFINITE );
        if ( dw == WAIT_OBJECT_0 || dw != WAIT_TIMEOUT ) {
            CloseHandle ( thrd );
        }
        else {
            CErrArg ( "DisposeThread: Waiting for %s thread TIMED OUT!", threadName );
            if ( TerminateThread ( thrd, 0 ) ) {
                CWarnArg ( "DisposeThread: Terminated %s thread.", threadName );
            }
            else {
                CErrArg ( "DisposeThread: Failed to terminate %s thread!", threadName );
            }
        }
#else
        pthread_join ( thrd, NULL );
        //pthread_kill ( thrd, SIGUSR1 );
        //pthread_detach_handle ( thrd );
#endif
     }
     */
    

	/**
	*	Detach a thread and reset the threadID variable afterwards.
	*	Make sure that the thread has not bee detached before.
	*	Reset the thread variable stored in threadID on success.
	*
	*/
    void DetachThread ( LONGSYNC * threadState, pthread_t &threadID, const char * threadName )
    {
        CVerb ( "DetachThread" );
        
        if ( threadState != nill && (___sync_val_compare_and_swap ( threadState, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE) )
            return;
        
        if ( !pthread_valid ( threadID ) ) {
            CVerb ( "DetachThread: Thread is already detached." );
            return;
        }
        
        pthread_detach_handle ( threadID );
        
        pthread_reset ( threadID );
    }


	/**
	*	Dispose a thread and reset the threadID variable afterwards.
	*	Make sure that the thread has not bee detached before.
	*	Reset the thread variable stored in threadID on success.
	*
	*/
	void DisposeThread ( LONGSYNC * threadState, pthread_t &threadID, const char * threadName )
	{
		CVerb ( "DisposeThread" );

        if ( ___sync_val_compare_and_swap ( threadState, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE )
            return;

		if ( !pthread_valid ( threadID ) ) {
			CVerb ( "DisposeThread: Thread is already closed." );
			return;
		}

		bool weAreTheThreadID = false;

#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)

		weAreTheThreadID = pthread_is_self_thread ( threadID );
#else
		pthread_t our_id = pthread_self ();

		weAreTheThreadID = (memcmp ( &our_id, &threadID, sizeof(pthread_t) ) == 0);
#endif
		if ( weAreTheThreadID ) {
			CLogArg ( "DisposeThread: Skip waiting for %s ...", threadName );
			return;
		}

		CLogArg ( "DisposeThread: Waiting for %s ...", threadName );
        
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS) && !defined(WINDOWS_PHONE)

		//DWORD dw = WaitForSingleObject ( thrd, WAIT_TIME_FOR_THREAD_CLOSING );
		DWORD dw = WaitForSingleObject ( threadID, INFINITE );
		if ( dw == WAIT_OBJECT_0 || dw != WAIT_TIMEOUT ) {
			CloseHandle ( threadID );
		}
		else {
			CErrArg ( "DisposeThread: Waiting for %s thread TIMED OUT!", threadName );
			if ( TerminateThread ( threadID, 0 ) ) {
				CWarnArg ( "DisposeThread: Terminated %s thread.", threadName );
			}
			else {
				CErrArg ( "DisposeThread: Failed to terminate %s thread!", threadName );
			}
		}
#else
        pthread_join ( threadID, NULL );
        //pthread_join ( thrd, NULL );
		//pthread_kill ( thrd, SIGUSR1 );
		//pthread_detach_handle ( thrd );
#endif
	}


	/**
	*	Dispose a thread and reset the threadID variable afterwards.
	*	Make sure that the thread has not bee detached before.
	*	Reset the thread variable stored in threadID on success.
	*
	*/
	void DisposeThread ( LONGSYNC * threadState, pthread_t &threadID, const char * threadName, pthread_cond_t &threadEvent )
	{
		if ( !pthread_valid ( threadID ) ) {
			CVerb ( "DisposeThread: Thread is already closed." );
			return;
		}

		if ( pthread_cond_valid ( threadEvent ) ) {
			CVerbArg ( "DisposeThread: Signaling %s thread...", threadName );

#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
			SetEvent ( threadEvent );
#else
			pthread_cond_signal ( &threadEvent );
#endif
		}
		DisposeThread ( threadState, threadID, threadName );
    }
    
	
	/**
	*	Dispose a thread and reset the threadID variable afterwards. 
	*	Make sure that the thread has not bee detached before.
	*	Reset the thread variable stored in threadID on success.
	*
    void DisposeThread ( pthread_t &threadID, const char * threadName, pthread_cond_t &threadEvent )
    {
        if ( !pthread_valid ( threadID ) ) {
            CVerb ( "DisposeThread: Thread is already closed." );
            return;
        }
        
        if ( pthread_cond_valid ( threadEvent ) ) {
            CVerbArg ( "DisposeThread: Signaling %s thread...", threadName );
            
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
            SetEvent ( threadEvent );
#else
            pthread_cond_signal ( &threadEvent );
#endif
        }
        DisposeThread ( threadID, threadName );
     }
     */

#ifdef _WIN32
#pragma warning( pop )
#endif


	/**
	*	Initialize a condition variable and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
    bool CondInitBool ( pthread_cond_t * cond, const char * name ) {
#else
	bool CondInitBool ( pthread_cond_t * cond )
	{
#endif
        if ( !cond ) {
#ifdef USE_LOCK_LOG
            CErrArg ( "CondInitBool: Invalid cond object for [ %s ]", name ? name : "Unknown" );
#else
#	ifndef CLI_CPP
			CErrN ( "CondInitBool: Invalid cond object" );
#	endif
#endif
            return false;
        }
        
        memset ( cond, 0, sizeof ( pthread_cond_t ) );
        
        if ( pthread_cond_init ( cond, NULL ) ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "CondInitBool: Failed to init [ %s ]", name ? name : "Unknown" );
#else
#	ifndef CLI_CPP
			CErrN ( "CondInitBool: Failed to init" );
#	endif
#endif
            return false;
        }
        return true;
    }


	/**
	*	Dispose a condition variable and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
    bool CondDisposeBool ( pthread_cond_t * cond, const char * name ) {
#else
	bool CondDisposeBool ( pthread_cond_t * cond )
	{
#endif
        if ( !cond || pthread_cond_destroy ( cond ) ) {
#ifdef USE_LOCK_LOG
            CErrArg ( "CondDisposeBool: Failed to destroy [ %s ]", name ? name : "Unknown" );
#else
#	ifndef CLI_CPP
			CErrN ( "CondDisposeBool: Failed to destroy" );
#	endif
#endif
            return false;
        }
        return true;
    }


	/**
	*	Initialize a mutex and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
	bool MutexInitBool ( pthread_mutex_t * mtx, const char * name )
	{
#else
	bool MutexInitBool ( pthread_mutex_t * mtx ) {
#endif
		if ( !mtx ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "MutexInit: Invalid mutex object for [ %s ]", name ? name : "Unknown" );
#else
#	ifndef CLI_CPP
			CErrN ( "MutexInit: Invalid mutex object" );
#	endif
#endif
			return false;
		}

		memset ( mtx, 0, sizeof ( pthread_mutex_t ) );

		if ( pthread_mutex_init ( mtx, NULL ) ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "MutexInit: Failed to init [ %s ]", name ? name : "Unknown" );
#else
#	ifndef CLI_CPP
			CErrN ( "MutexInit: Failed to init" );
#	endif
#endif
			return false;
		}
		return true;
	}


	/**
	*	Dispose a mutex and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
	bool MutexDisposeBool ( pthread_mutex_t * mtx, const char * name ) {
#else
	bool MutexDisposeBool ( pthread_mutex_t * mtx )
	{
#endif
		if ( !mtx || pthread_mutex_destroy ( mtx ) ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "MutexDispose: Failed to destroy [ %s ]", name ? name : "Unknown" );
#else
#	ifndef CLI_CPP
			CErrN ( "MutexDispose: Failed to destroy" );
#	endif
#endif
			return false;
		}
		return true;
	}


	/**
	*	Log lock error message.
	*
	*/
#ifdef USE_LOCK_LOG
	void MutexErrorLog ( const char * operation, const char * mutexName, const char * className, const char * funcName ) {
		CErrArg ( "%s.%s: Failed to %s [ %s ]", className ? className : "Unknown", funcName ? funcName : "Unknown", mutexName ? mutexName : "Unknown", operation );
	}
#else
	void MutexErrorLog ( const char * operation )
	{
#	ifndef CLI_CPP
		CErrArg ( "MutexErrorLog: Failed to %s ", operation  );
#	endif
	}
#endif

#undef CLASS_NAME
#define CLASS_NAME	""


	/**
	*	Release lock on mutex.
	*
	*/
#ifdef USE_LOCK_LOG
	void MutexLockVoid ( pthread_mutex_t * mtx, const char * mutexName, const char * className, const char * funcName ) 
	{
		CVerbLockArg ( "        -------> Lock   [ %-30s ]    | %16X | %s.%s", mutexName, GetCurrentThreadId (), className, funcName );
#else
	void MutexLockVoid ( pthread_mutex_t * mtx )
	{
#endif
		if ( !mtx || pthread_mutex_lock ( mtx ) ) {
#ifdef USE_LOCK_LOG
			MutexErrorLog ( "lock", className, funcName, mutexName );
            return;
#else
			MutexErrorLog ( "lock" );
#endif
        }
#ifdef USE_LOCK_LOG
        CVerbLockArg ( "        -------> Locked [ %-30s ]    | %16X | %s.%s", mutexName, GetCurrentThreadId (), className, funcName );
#endif
	}


	/**
	*	Acquire lock on mutex.
	*
	*/
#ifdef USE_LOCK_LOG
	void MutexUnlockVoid ( pthread_mutex_t * mtx, const char * mutexName, const char * className, const char * funcName ) 
    {
        CVerbLockArg ( "       <------   Unlock [ %-30s ]    | %16X | %s.%s", mutexName, GetCurrentThreadId (), className, funcName );
#else
	void MutexUnlockVoid ( pthread_mutex_t * mtx )
	{
#endif
		if ( !mtx || pthread_mutex_unlock ( mtx ) ) {
#ifdef USE_LOCK_LOG
			MutexErrorLog ( "unlock", className, funcName, mutexName );
#else
			MutexErrorLog ( "unlock" );
#endif
		}
	}


	/**
	*	Release lock on mutex and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
	bool MutexLockBool ( pthread_mutex_t * mtx, const char * mutexName, const char * className, const char * funcName ) 
	{
        CVerbLockArg ( "        -------> Lock   [ %-30s ]    | %16X | %s.%s", mutexName, GetCurrentThreadId (), className, funcName );
#else
	bool MutexLockBool ( pthread_mutex_t * mtx )
	{
#endif
		if ( !mtx || pthread_mutex_lock ( mtx ) ) {
#ifdef USE_LOCK_LOG
			MutexErrorLog ( "lock", className, funcName, mutexName );
#else
			MutexErrorLog ( "lock" );
#endif
			return false;
        }
#ifdef USE_LOCK_LOG
        CVerbLockArg ( "        -------> Locked [ %-30s ]    | %16X | %s.%s", mutexName, GetCurrentThreadId (), className, funcName );
#endif
		return true;
	}


	/**
	*	Acquire lock on mutex and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
	bool MutexUnlockBool ( pthread_mutex_t * mtx, const char * mutexName, const char * className, const char * funcName ) 
	{
        CVerbLockArg ( "       <------   Unlock [ %-30s ]    | %16X | %s.%s", mutexName, GetCurrentThreadId (), className, funcName );
#else
	bool MutexUnlockBool ( pthread_mutex_t * mtx )
	{
#endif
		if ( !mtx || pthread_mutex_unlock ( mtx ) ) {
#ifdef USE_LOCK_LOG
			MutexErrorLog ( "unlock", className, funcName, mutexName );
#else
			MutexErrorLog ( "unlock" );
#endif
			return false;
        }
		return true;
	}

#endif



#undef CLASS_NAME
#define CLASS_NAME	"ThreadSync . . . . . . ."

	bool ThreadSync::Init ()
	{
		if ( allocated )
			return true;

		if ( !MutexInitA ( lock ) )
			return false;
        
		if ( pthread_cond_manual_init ( c_Addr_of ( signal ), NULL ) ) {
			CErr ( "Init: Failed to init signal!" );
			return false;
		}

		allocated = true;
		return true;
	}


	ThreadSync::~ThreadSync ()
	{
		if ( allocated ) {
			CondDisposeA ( signal );

			MutexDisposeA ( lock );

			allocated = false;
		}
    }
        
        
    ThreadSync::ThreadSync ()
    {
        allocated = false;
        autoreset = true;
        
#ifdef CLI_CPP
		signal = nill;
		threadID = nill;
#else
        Zero ( signal );
        
        Zero ( threadID );
#endif
        state = ENVIRONS_THREAD_NO_THREAD;
    }
        
    
    bool ThreadSync::LockCond ( CString_ptr func )
    {
        if ( pthread_cond_mutex_lock ( c_Addr_of ( lock ) ) ) {
#ifndef CLI_CPP
            CErrArg ( "LockCond: Failed [%s]", func );
#endif
            return false;
        }
        return true;
    }
        
        
    bool ThreadSync::Lock ( CString_ptr func )
    {
        if ( pthread_mutex_lock ( Addr_of ( lock ) ) ) {
#ifndef CLI_CPP
            CErrArg ( "Lock: Failed [%s]", func );
#endif
            return false;
        }
        return true;
    }
    
    
    bool ThreadSync::ResetSync ( CString_ptr func, bool useLock )
    {
        if ( useLock && !LockCond ( func ) )
            return false;
            
        if ( pthread_cond_prepare ( c_Addr_of ( signal ) ) ) {
#ifndef CLI_CPP
            CErrArg ( "ResetSync: Failed [%s]", func );
#endif
            return false;
        }
        
        if ( useLock && !UnlockCond ( func ) )
            return false;
        
        return true;
    }


	void ThreadSync::ResetState ()
	{
		state = ENVIRONS_THREAD_NO_THREAD;
	}
        
        
    bool ThreadSync::WaitOne ( CString_ptr func, int ms, bool useLock, bool keepLocked )
    {
        if ( useLock && !LockCond ( func ) )
            return false;
        
        if ( WaitLocked ( func, ms ) ) {
            if ( keepLocked || UnlockCond ( func ) )
                return true;
        }
        else {
            if ( !keepLocked )
                UnlockCond ( func );
        }
        return false;
    }
        
        
    bool ThreadSync::WaitLocked ( CString_ptr func, int ms )
    {
        if ( ms > 0 ) {
            if (
#ifdef _WIN32
                !pthread_cond_wait_time ( c_Addr_of ( signal ), &lock, ms )
#else
                pthread_cond_wait_time ( &signal, &lock, ms ) == 0
#endif
                )
            {
                if ( autoreset ) {
                    pthread_cond_preparev ( c_Addr_of ( signal ) );
                }
                return true;
            }
        }
        else {
            if (
#ifdef _WIN32
                !pthread_cond_manual_wait ( c_Addr_of ( signal ), &lock )
#else
                pthread_cond_wait ( &signal, &lock ) == 0
#endif
                )
            {
                if ( autoreset ) {
                    pthread_cond_preparev ( c_Addr_of ( signal ) );
                }
                return true;
            }
        }
        
#ifndef CLI_CPP
        CErrArg ( "Wait: Failed [%s]", func );
#endif
        return false;
    }


	bool ThreadSync::Notify ( CString_ptr func, bool useLock )
	{
		if ( useLock && pthread_cond_mutex_lock ( c_Addr_of ( lock ) ) ) {
			CErr ( "Notify: Failed to acquire mutex" );
			return false;
		}

		if ( pthread_cond_broadcast ( c_Addr_of ( signal ) ) ) {
			CErr ( "Notify: Failed to signal thread_signal!" );
			return false;
		}
		CVerb ( "Notify: Thread signaled." );

		if ( useLock && pthread_cond_mutex_unlock ( c_Addr_of ( lock ) ) ) {
			CErr ( "Notify: Failed to release mutex" );
			return false;
		}
		return true;
	}


	bool ThreadSync::UnlockCond ( CString_ptr func )
	{
        if ( pthread_cond_mutex_unlock ( &lock ) ) {
#ifndef CLI_CPP
            CErrArg ( "UnlockCond: Failed [%s]", func );
#endif
            return false;
        }
        return true;
    }
        
        
    bool ThreadSync::Unlock ( CString_ptr func )
    {
        if ( pthread_mutex_unlock ( Addr_of ( lock ) ) ) {
#ifndef CLI_CPP
            CErrArg ( "Unlock: Failed [%s]", func );
#endif
            return false;
        }
        return true;
    }
        

	bool ThreadSync::Run ( pthread_start_routine_t startRoutine, pthread_param_t arg, CString_ptr func, bool waitForStart )
	{
		if ( ___sync_val_compare_and_swap ( c_ref state, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) != ENVIRONS_THREAD_NO_THREAD ) 
		{
#ifndef CLI_CPP
			CWarnArg ( "Run: [%s] Thread already running!", func );
#endif
			return true;
		}

		int ret;

		if ( waitForStart && !ResetSync ( func, true ) )
			goto Failed;

#ifndef CLI_CPP
		ret = pthread_create ( c_Addr_of ( threadID ), 0, startRoutine, arg );
#else
		ret = pthread_create_cli ( c_Addr_of ( threadID ), 0, startRoutine, arg );
#endif
		if ( ret ) {
#ifndef CLI_CPP
			CErrArg ( "Run: [%s] Failed to create thread!", func );
#endif
			
			if ( waitForStart ) 
				UnlockCond ( func );
		}
		else {
			if ( !waitForStart || WaitOne ( func, 2000, false, false ) )
				return true;

#ifndef CLI_CPP
			CErrArg ( "Run: [%s] Failed to wait for thread start!", func );
#endif
			Detach ( "Run" );
			return false;
		}

	Failed:
		state = ENVIRONS_THREAD_NO_THREAD;
		return false;
	}
       

	bool ThreadSync::isRunning ()
	{
		return ( state != ENVIRONS_THREAD_NO_THREAD );
	}


    void ThreadSync::Join ( CString_ptr func )
    {
#ifndef CLI_CPP
		DisposeThread ( &state, threadID, func );
#else
		if ( ___sync_val_compare_and_swap ( state, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE )
		{
			return;
		}

		if ( threadID == nill )
			return;

		if ( Thread::CurrentThread == threadID )
			return;

		threadID->Join ();
		threadID = nill;
#endif
    }
        
    
    void ThreadSync::Detach ( CString_ptr func )
    {
#ifndef CLI_CPP
        DetachThread ( &state, threadID, func );
#else
		threadID = nill;
		state = ENVIRONS_THREAD_NO_THREAD;
#endif
    }
    

	bool ThreadSync::areWeTheThread ()
	{
#ifndef CLI_CPP
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)

		return pthread_is_self_thread ( threadID );
#else
		pthread_t our_id = pthread_self ();

		return (memcmp ( &our_id, &threadID, sizeof(pthread_t) ) == 0);
#endif
#else
		return ( Thread::CurrentThread == threadID );
#endif
	}

}
