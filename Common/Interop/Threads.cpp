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
#   include "./Sock.h"
#endif

#include "Environs.Native.h"
#include "Threads.Int.h"

#ifdef USE_PTHREADS_FOR_WINDOWS
#   pragma comment ( lib, "pthreadVC2.lib" )
#endif

// The TAG for prepending to log messages
#define CLASS_NAME	"Threads. . . . . . . . ."

namespace environs
{
#if ( !defined(ENVIRONS_CORE_LIB) || defined(ENVIRONS_NATIVE_MODULE) )
	int g_Debug = 0;
#endif

#ifdef ANDROID
	INLINEFUNC long ___sync_val_compare_and_swap ( long * destination, unsigned int compare, unsigned int swap )
	{
		return __sync_val_compare_and_swap ( destination, compare, swap );
	}
#endif

#ifdef WINDOWS_PHONE

	static HANDLE threadSleepEvent = 0;

	INLINEFUNC void Sleep ( DWORD millis )
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
		return ( threadSleepEvent != 0 );
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

	int pthread_create_cli ( pthread_t % threadID, void *, System::Threading::ParameterizedThreadStart ^ startRoutine, pthread_param_t arg )
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

	INLINEFUNC bool pthread_valid ( pthread_t thread )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		if ( thread )
			return true;
#else
#	ifdef _WIN32
		if ( thread.p )
			return true;
#	else
		if ( thread )
			return true;
#	endif
#endif
		return false;
	}


#ifdef USE_CRIT_SEC_MUTEX
#	ifndef WINDOWS_PHONE
	INLINEFUNC bool pthread_mutex_init ( CRITICAL_SECTION  * critSEc, void * arg )
	{
		InitializeCriticalSection ( critSEc );
		return false;
	}

	INLINEFUNC bool pthread_mutex_destroy ( CRITICAL_SECTION  * critSEc )
	{
		DeleteCriticalSection ( critSEc );
		return false;
	}
#	endif
#endif


	INLINEFUNC pthread_t_id getSelfThreadID ()
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		return GetCurrentThreadId ();
#else
		return pthread_self ();
#endif
	}


	INLINEFUNC bool areWeTheThreadID ( pthread_t_id thread )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		if ( !thread )
			return false;

		DWORD id = thread;

		DWORD our_id = GetCurrentThreadId ();

		return ( our_id == id );
#else
		pthread_t our_id = pthread_self ();

		return ( memcmp ( &our_id, &thread, sizeof ( pthread_t ) ) == 0 );
#endif
	}


	INLINEFUNC bool pthread_is_self_thread ( pthread_t thread )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		if ( !thread )
			return false;
#ifdef WINDOWS_PHONE
		DWORD id = ( DWORD ) thread;
#else
		DWORD id = GetThreadId ( thread );
#endif
		if ( !id ) // Failed
			return true;

		DWORD our_id = GetCurrentThreadId ();

		return ( our_id == id );
#else
		pthread_t our_id = pthread_self ();

		return ( memcmp ( &our_id, &thread, sizeof ( pthread_t ) ) == 0 );
#endif
	}


	INLINEFUNC bool pthread_is_self_thread_id ( pthread_t_id handleID )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		if ( !handleID )
			return false;

		DWORD our_id = GetCurrentThreadId ();

		return ( our_id == handleID );
#else
		pthread_t our_id = pthread_self ();

		return ( memcmp ( &our_id, &handleID, sizeof ( pthread_t_id ) ) == 0 );
#endif
	}


	INLINEFUNC bool pthread_wait_one ( pthread_cond_t &cond, pthread_mutex_t &lock )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS) 
#ifdef WINDOWS_PHONE
		return ( WaitForSingleObjectEx ( cond, INFINITE, TRUE ) == WAIT_OBJECT_0 );
#else
		return ( WaitForSingleObject ( cond, INFINITE ) == WAIT_OBJECT_0 );
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


	INLINEFUNC int pthread_cond_timedwait_msec ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int timeout )
	{
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
		return pthread_cond_timedwait ( cond, lock, &timeout );
#else
		return pthread_cond_timedwait_sec ( cond, lock, timeout / 1000 );
#endif
	}


	INLINEFUNC int pthread_cond_timedwait_sec ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int timeout )
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
	_When_ ( return == 0, _Acquires_lock_ ( lock ) )
#endif
	INLINEFUNC int pthread_mutex_lock ( pthread_mutex_t * lock )
	{
		EnterCriticalSection ( lock );
		return 0;
	}


#if _MSC_VER >= 1800
	_When_ ( return == 0, _Acquires_lock_ ( *lock ) )
#endif
	INLINEFUNC int pthread_mutex_trylock ( pthread_mutex_t * lock )
	{
		return !TryEnterCriticalSection ( lock );
	}


#if _MSC_VER >= 1800
	_When_ ( return == 0, _Releases_lock_ ( *lock ) )
#endif
	INLINEFUNC int pthread_mutex_unlock ( pthread_mutex_t * lock )
	{
		LeaveCriticalSection ( lock );
		return 0;
	}
#endif


	INLINEFUNC int pthread_cond_timedwait ( pthread_cond_t * cond, pthread_mutex_t * lock, unsigned int * timeout )
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
		static char buffer [ 48 ];

		int lenA = sprintf ( buffer, "sem-%s%u-%i-%u", name, name1, name2, name3 );
		if ( lenA <= 0 )
			return false;

		static TCHAR tbuffer [ 128 ];
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
		static char buffer [ 48 ];

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
		if ( ret == -1 ) {
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

		if ( threadState != nill && ( ___sync_val_compare_and_swap ( threadState, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE ) )
			return;

		if ( !pthread_valid ( threadID ) ) {
			CVerb ( "DetachThread: Thread is already detached." );
			return;
		}

		/*pthread_t thread = threadID;

		pthread_reset ( threadID );

        pthread_detach_handle ( thread );
         */        
        
        pthread_detach_handle ( threadID );
        
        pthread_reset ( threadID );
	}


	/**
	*	Dispose a thread and reset the threadID variable afterwards.
	*	Make sure that the thread has not bee detached before.
	*	Reset the thread variable stored in threadID on success.
	*
	*/
	void DisposeThread ( LONGSYNC * threadState, pthread_t &threadID, pthread_t_id handleID, const char * threadName )
	{
		CVerb ( "DisposeThread" );

		pthread_t thread = threadID;

		if ( !pthread_valid ( thread ) ) {
			if ( threadState && ___sync_val_compare_and_swap ( threadState, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE ) {
				CVerbVerb ( "DisposeThread: Thread has not been started." );
			}

			CVerb ( "DisposeThread: Thread is already closed." );
			return;
		}

		bool threadMatchesCaller	= false;

#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)

		if ( handleID )
			threadMatchesCaller	= pthread_is_self_thread_id ( handleID );
		else
			threadMatchesCaller	= pthread_is_self_thread ( threadID );
#else
		pthread_t our_id = pthread_self ();

		threadMatchesCaller		= ( memcmp ( &our_id, &thread, sizeof ( pthread_t ) ) == 0 );
#endif
		if ( threadMatchesCaller ) {
#ifdef MEDIATORDAEMON
			CVerbArg ( "DisposeThread: Skip waiting for [ %s ] ...", threadName );
#else
			CVerbArg ( "DisposeThread: Skip waiting for [ %s ] ...", threadName );
#endif
			return;
		}

		if ( threadState != nill )
		{
			if ( ___sync_val_compare_and_swap ( threadState, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE )
				return;

			if ( !pthread_valid ( threadID ) ) {
				CVerb ( "DisposeThread: Thread is already closed." );
				return;
			}
		}

		pthread_reset ( threadID );

		CLogsArg ( 2, "DisposeThread: Waiting for %s ...", threadName );

#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS) && !defined(WINDOWS_PHONE)

		DWORD dw = WAIT_OBJECT_0;

		if ( !GetExitCodeThread ( thread, &dw ) )
			dw = WAIT_OBJECT_0;

		if ( dw == STILL_ACTIVE )
			dw = WaitForSingleObject ( thread, INFINITE );

		if ( dw == WAIT_OBJECT_0 ) {
			CloseHandle ( thread );
		}

		/*if ( dw == WAIT_OBJECT_0 || dw != WAIT_TIMEOUT ) {
			CloseHandle ( thread );
		}
		else {
			CErrArg ( "DisposeThread: Waiting for %s thread TIMED OUT!", threadName );
			if ( TerminateThread ( thread, 0 ) ) {
				CWarnArg ( "DisposeThread: Terminated %s thread.", threadName );
			}
			else {
				CErrArg ( "DisposeThread: Failed to terminate %s thread!", threadName );
			}
		}*/
#else
		pthread_join ( thread, NULL );
#endif

		CLogsArg ( 3, "DisposeThread: Waiting for %s done.", threadName );
	}


	/**
	*	Dispose a thread and reset the threadID variable afterwards.
	*	Make sure that the thread has not bee detached before.
	*	Reset the thread variable stored in threadID on success.
	*
	*/
	void DisposeThread ( LONGSYNC * threadState, pthread_t &threadID, pthread_t_id handleID, const char * threadName, pthread_cond_t &threadEvent )
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
		DisposeThread ( threadState, threadID, handleID, threadName );
	}


#ifdef _WIN32
#pragma warning( pop )
#endif


	/**
	*	Initialize a condition variable and return success as bool.
	*
	*	@return success
	*/
#ifdef USE_LOCK_LOG
	bool CondInitBool ( pthread_cond_t * cond, const char * name )
	{
#else
	bool CondInitBool ( pthread_cond_t * cond )
	{
#endif
		if ( !cond ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "CondInitBool: Invalid cond object for [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "CondInitBool: Invalid cond object" );
#endif
			return false;
		}

		memset ( cond, 0, sizeof ( pthread_cond_t ) );

		if ( pthread_cond_init ( cond, NULL ) ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "CondInitBool: Failed to init [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "CondInitBool: Failed to init" );
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
	bool CondDisposeBool ( pthread_cond_t * cond, const char * name )
	{
#else
	bool CondDisposeBool ( pthread_cond_t * cond )
	{
#endif
		if ( !cond || pthread_cond_destroy ( cond ) ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "CondDisposeBool: Failed to destroy [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "CondDisposeBool: Failed to destroy" );
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
	bool MutexInitBool ( pthread_mutex_t * mtx )
	{
#endif
		if ( !mtx ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "MutexInit: Invalid mutex object for [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "MutexInit: Invalid mutex object" );
#endif
			return false;
		}

		memset ( mtx, 0, sizeof ( pthread_mutex_t ) );

		if ( pthread_mutex_init ( mtx, NULL ) ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "MutexInit: Failed to init [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "MutexInit: Failed to init" );
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
	bool MutexDisposeBool ( pthread_mutex_t * mtx, const char * name )
	{
#else
	bool MutexDisposeBool ( pthread_mutex_t * mtx )
	{
#endif
		if ( !mtx || pthread_mutex_destroy ( mtx ) ) {
#ifdef USE_LOCK_LOG
			CErrArg ( "MutexDispose: Failed to destroy [ %s ]", name ? name : "Unknown" );
#else
			CErrN ( "MutexDispose: Failed to destroy" );
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
	void MutexErrorLog ( const char * operation, const char * mutexName, const char * className, const char * funcName )
	{
		CErrArg ( "%s.%s: Failed to %s [ %s ]", className ? className : "Unknown", funcName ? funcName : "Unknown", mutexName ? mutexName : "Unknown", operation );
	}
#else
	void MutexErrorLog ( const char * operation )
	{
		CErrArg ( "MutexErrorLog: Failed to %s ", operation );
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
		CVerbsLockArg ( 12, "        -------> Lock   [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
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
		CVerbsLockArg ( 12, "        -------> Locked [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
#endif
	}


	/**
	*	Acquire lock on mutex.
	*
	*/
#ifdef USE_LOCK_LOG
	void MutexUnlockVoid ( pthread_mutex_t * mtx, const char * mutexName, const char * className, const char * funcName )
	{
		CVerbsLockArg ( 12, "       <------   Unlock [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
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
		CVerbsLockArg ( 12, "        -------> Lock   [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
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
		CVerbsLockArg ( 12, "        -------> Locked [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
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
		CVerbsLockArg ( 12, "       <------   Unlock [ %-30s ]    | %16llX | %s.%s", mutexName, ( long long ) GetCurrentThreadId (), className, funcName );
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
#define CLASS_NAME	"EnvThread. . . . . . . ."

	EnvThread::EnvThread ()
	{
#ifdef CLI_CPP
		threadID = nill;
#else
		Zero ( threadID );
#endif
		Win32_Only ( handleID = 0; )

		state = ENVIRONS_THREAD_NO_THREAD;
    }
        
    
    EnvThread::~EnvThread ()
    {
#ifdef USE_THREADSYNC_OWNER_NAME
        CVerbVerbArg ( "Destruct: [ %s ]", owner );
#endif
        Detach ( "ThreadBase" );
    }

/*
	void EnvThread::ResetState ()
	{
		state = ENVIRONS_THREAD_NO_THREAD;
	}
        */


	/*
	* Run	Create a thread with the given thread routine
	* @return	1	success, thread is running or was already runing. (if wait is requested, then wait was successful)
	*			0	failed
	*			-1	failed, thread was started and is probably running (soon). However, wait for thread start failed.
	*/
	int EnvThread::Run ( pthread_start_routine_t startRoutine, pthread_param_t arg, CString_ptr func )
	{
		if ( ___sync_val_compare_and_swap ( c_ref state, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) != ENVIRONS_THREAD_NO_THREAD )
		{
			CWarnsArg ( 6, "Run: [%s] Thread already running!", func );
			return 1;
		}

		int retCode = pthread_create_cli ( c_Addr_of ( threadID ), 0, startRoutine, arg );
		if ( retCode ) {
			CErrArg ( "Run: [%s] Failed to create thread!", func );
		}
		else {
			// It is possible, that we may (continue here and) find an empty threadID at this point.
			// We're just test the threadID and go on with an invalid handle if all goes wrong
			Win32_Only ( HANDLE h = threadID; if ( h ) handleID = GetThreadId ( threadID ); )
			return 1;
		}

		state = ENVIRONS_THREAD_NO_THREAD;
		return 0;
	}


	bool EnvThread::isRunning ()
	{
		return ( state != ENVIRONS_THREAD_NO_THREAD );
	}


	void EnvThread::Join ( CString_ptr func )
	{
#ifndef CLI_CPP
#	ifdef _WIN32
		DisposeThread ( &state, threadID, handleID, func );
#	else
		DisposeThread ( &state, threadID, threadID, func );
#	endif
#else
		if ( threadID == nill )
			return;

		if ( Thread::CurrentThread == threadID )
			return;

		if ( ___sync_val_compare_and_swap ( state, ENVIRONS_THREAD_DETACHEABLE, ENVIRONS_THREAD_NO_THREAD ) != ENVIRONS_THREAD_DETACHEABLE ) {
			return;
		}

		if ( threadID == nill )
			return;

		threadID->Join ();
		threadID = nill;
#endif
	}


	void EnvThread::Detach ( CString_ptr func )
	{
#ifndef CLI_CPP
		DetachThread ( &state, threadID, func );
#else
		threadID = nill;
		state = ENVIRONS_THREAD_NO_THREAD;
#endif
	}


	bool EnvThread::areWeTheThread ()
	{
#ifndef CLI_CPP
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)

		return pthread_is_self_thread ( threadID );
#else
		pthread_t our_id = pthread_self ();

		return ( memcmp ( &our_id, &threadID, sizeof ( pthread_t ) ) == 0 );
#endif
#else
		return ( Thread::CurrentThread == threadID );
#endif
	}



#undef CLASS_NAME
#define CLASS_NAME	"EnvLock. . . . . . . . ."
        
    
    EnvLock::EnvLock ()
    {
        allocated = false;
        autoreset = true;
        
#ifndef _WIN32
        signalState = false;
#endif
        
#ifdef USE_THREADSYNC_OWNER_NAME
        owner       = "Unknown";
#endif
        
#ifdef CLI_CPP
        signal = nill;
#else
        Zero ( signal );
#endif
    }
        

	bool EnvLock::Init ()
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


	EnvLock::~EnvLock ()
	{
#ifdef USE_THREADSYNC_OWNER_NAME
		CLogArg ( "Destruct: [ %s ]", owner );
#endif
		if ( allocated ) {
			CondDisposeA ( signal );

			MutexDisposeA ( lock );

			allocated = false;
		}
	}


	void EnvLock::DisposeInstance ()
	{
#ifdef USE_THREADSYNC_OWNER_NAME
		CLogArg ( "ThreadSync.DisposeInstance: [ %s ]", owner );
#endif
		if ( allocated ) {
			CondDisposeA ( signal );

			MutexDisposeA ( lock );

			allocated = false;
		}
	}


	bool EnvLock::LockCond ( CString_ptr func )
	{
#ifdef USE_THREADSYNC_OWNER_NAME
		CVerbVerbArg ( "LockCond: [ %s ]", owner );
#endif
		if ( pthread_cond_mutex_lock ( c_Addr_of ( lock ) ) ) {
			CErrArg ( "LockCond: Failed [ %s ]", func );
			return false;
		}
		return true;
	}


	bool EnvLock::Lock ( CString_ptr func )
	{
#ifdef USE_THREADSYNC_OWNER_NAME
		CVerbVerbArg ( "Lock: [ %s ]", owner );
#endif
		if ( pthread_mutex_lock ( Addr_of ( lock ) ) ) {
			CErrArg ( "Lock: Failed [ %s ]", func );
			return false;
		}
		return true;
	}


	bool EnvLock::ResetSync ( CString_ptr func, bool useLock, bool keepLocked )
	{
		if ( useLock && !LockCond ( func ) )
			return false;
        
#ifndef _WIN32
        signalState = false;
#else
        if ( pthread_cond_prepare ( c_Addr_of ( signal ) ) ) {
            CErrArg ( "ResetSync: Failed [ %s ]", func );
            if ( useLock )
                UnlockCond ( func );
            
            return false;
        }
#endif
		if ( useLock && !keepLocked && !UnlockCond ( func ) )
			return false;

		return true;
	}


	int EnvLock::WaitOne ( CString_ptr func, int ms, bool useLock, bool keepLocked )
	{
		if ( useLock && !LockCond ( func ) )
			return 0;

		int waitRes = WaitLocked ( func, ms );
		if ( waitRes )
		{
			if ( !useLock || keepLocked || UnlockCond ( func ) )
				return waitRes;
		}
		else {
			if ( !keepLocked )
				UnlockCond ( func );
		}
		return 0;
	}


	int EnvLock::WaitLocked ( CString_ptr func, int ms )
	{
#ifdef _WIN32
#	ifdef CLI_CPP
		bool waitRes;

		if ( ms == ENV_INFINITE_MS )
			waitRes = signal->WaitOne ();
		else
			waitRes = signal->WaitOne ( ms );
#	else
		DWORD waitRes;

		if ( ms == ENV_INFINITE_MS )
#ifdef WINDOWS_PHONE
			waitRes = WaitForSingleObjectEx ( signal, INFINITE, TRUE );
#else
			waitRes = WaitForSingleObject ( signal, INFINITE );
#endif
		else
#ifdef WINDOWS_PHONE
			waitRes = WaitForSingleObjectEx ( signal, ms, TRUE );
#else
			waitRes = WaitForSingleObject ( signal, ms );
#endif
#	endif
#else
		int waitRes;
        
        if ( autoreset || !signalState )
        {
            if ( ms == ENV_INFINITE_MS )
                waitRes = pthread_cond_wait ( &signal, &lock );
            else
                waitRes = pthread_cond_timedwait_msec ( &signal, &lock, ms );
        }
        else
            waitRes = 0;
#endif
		if ( autoreset ) {
			pthread_cond_preparev ( c_Addr_of ( signal ) );
		}

#ifdef _WIN32
#	ifdef CLI_CPP
		if ( ms != ENV_INFINITE_MS )
			return -1; // No signal means timeout
		if ( waitRes )
			return 1;
#	else
		if ( waitRes == WAIT_OBJECT_0 )
			return 1;
		else if ( waitRes == WAIT_TIMEOUT ) {
			CVerbsArg ( 3, "WaitLocked: Time out [ %s ]", func );
			return -1;
		}
#	endif
#else
		if ( waitRes == 0 )
			return 1;
		else if ( waitRes == ETIMEDOUT ) {
			CVerbsArg ( 3, "WaitLocked: Time out [ %s ]", func );
			return -1;
		}
#endif

#if ( defined(ENVIRONS_CORE_LIB) )
		CErrArg ( "WaitLocked: Failed [ %s ]", func );
#endif
		return 0;
	}


	bool EnvLock::Notify ( CString_ptr func, bool useLock )
	{
#ifdef USE_THREADSYNC_OWNER_NAME
		CVerbVerbArg ( "Notify.Lock: [ %s ]", owner );
#endif
        if ( useLock && pthread_cond_mutex_lock ( c_Addr_of ( lock ) ) ) {
            CErrArg ( "Notify.Lock: Failed [ %s ]", func );
			return false;
		}
        
#ifndef _WIN32
        signalState = true;
#endif
		if ( pthread_cond_broadcast ( c_Addr_of ( signal ) ) ) {
			CErr ( "Notify: Failed to signal thread_signal!" );
			return false;
		}
		CVerb ( "Notify: Thread signaled." );

#ifdef USE_THREADSYNC_OWNER_NAME
		CVerbVerbArg ( "Notify.Unlock: [ %s ]", owner );
#endif
		if ( useLock && pthread_cond_mutex_unlock ( c_Addr_of ( lock ) ) ) {
			CErr ( "Notify: Failed to release mutex" );
			return false;
		}
		return true;
	}


	bool EnvLock::UnlockCond ( CString_ptr func )
	{
#ifdef USE_THREADSYNC_OWNER_NAME
		CVerbVerbArg ( "UnlockCond: [ %s ]", owner );
#endif
		if ( pthread_cond_mutex_unlock ( &lock ) ) {
			CErrArg ( "UnlockCond: Failed [%s]", func );
			return false;
		}
		return true;
	}


	bool EnvLock::Unlock ( CString_ptr func )
	{
#ifdef USE_THREADSYNC_OWNER_NAME
		CVerbVerbArg ( "Unlock: [ %s ]", owner );
#endif
		if ( pthread_mutex_unlock ( Addr_of ( lock ) ) ) {
			CErrArg ( "Unlock: Failed [%s]", func );
			return false;
		}
		return true;
    }
        
        
        
#undef CLASS_NAME
#define CLASS_NAME	"ThreadSync . . . . . . ."
            
    /*
    void ThreadSync::Reset ()
    {
        state = ENVIRONS_THREAD_NO_THREAD;
        
        pthread_reset ( threadID );
    }*/

	/*
	* Run	Create a thread with the given thread routine
	* @return	1	success, thread is running or was already runing. (if wait is requested, then wait was successful)
	*			0	failed
	*			-1	failed, thread was started and is probably running (soon). However, wait for thread start failed.
	*/
	int ThreadSync::Run ( pthread_start_routine_t startRoutine, pthread_param_t arg, CString_ptr func, bool waitForStart )
	{
		if ( ___sync_val_compare_and_swap ( c_ref state, ENVIRONS_THREAD_NO_THREAD, ENVIRONS_THREAD_DETACHEABLE ) != ENVIRONS_THREAD_NO_THREAD )
		{
			CWarnsArg ( 6, "Run: [%s] Thread already running!", func );
			return 1;
		}

		int success;

		if ( waitForStart && !ResetSync ( func, true, true ) )
			goto Failed;

		success = pthread_create_cli ( c_Addr_of ( threadID ), 0, startRoutine, arg );
		if ( success ) {
			CErrArg ( "Run: [%s] Failed to create thread!", func );

			if ( waitForStart )
				UnlockCond ( func );
		}
		else {
			// It is possible, that we may (continue here and) find an empty threadID at this point.
			// We're just test the threadID and go on with an invalid handle if all goes wrong
			Win32_Only ( HANDLE h = threadID; if ( h ) handleID = GetThreadId ( threadID ); )

			if ( !waitForStart )
				return 1;

			int waitRes = WaitOne ( func, 4000, false, false );

			UnlockCond ( func );

			if ( waitRes <= 0 ) {
				CErrArg ( "Run: [%s] Failed to wait for thread start!", func );
				//Detach ( "Run" );
			}

			if ( waitRes != 0 )
				return waitRes;
		}

	Failed:
		state = ENVIRONS_THREAD_NO_THREAD;
		return 0;
	}

}








