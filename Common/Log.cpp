/**
* Log
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

#	include <stdio.h>
#ifndef CLI_CPP
#	include <stdarg.h>
#	include <stdlib.h>

#	ifdef _WIN32
#		include "windows.h"
#	endif

#	ifdef ANDROID
#   include <android/log.h>
#	endif

#	include "Environs.Obj.h"
#	include "Core/Callbacks.h"
#endif

#include "Environs.Utils.h"
#include "Environs.Native.h"
#include "Interop/Threads.h"
#include "Interop/Stat.h"

using namespace environs;

#define CLASS_NAME  "Log. . . . . . . . . . ."


#define ENVIRONS_LOGFILE_TEMP_MAXSIZE               200000
#define ENVIRONS_LOGBUFFER_MAXCHARS					ENVIRONS_LOGBUFFER_MAXSIZE - 2
#define ENVIRONS_LOGBUFFER_OUTPUT_MINSIZE			ENVIRONS_LOGBUFFER_MAXSIZE >> 2
#define ENVIRONS_LOGBUFFER_OUTPUT_AX_MS				500


namespace environs
{
	pthread_mutex_t     environsLogMutex;
	FILE            *   environsLogFileHandle		= 0;
	int                 environsLogFileErrCount     = 0;
	
#ifdef NDEBUG
#	define		LOGBUFFER
#	define		LOGPOS		
#	define		LOGPOSADD	
#	define		ENVIRONS_LOGBUFFER_MAXSIZE		1024
#	define		LOGBUFFEREMAIN					ENVIRONS_LOGBUFFER_MAXSIZE
#	define		LOG_OUT_BUFFER_NAME				msg
#	define		LOG_OUT_BUFFER_NAME_LENGTH		length
#	define		LOG_OUT_BUFFER_NAME_ARG_CALL	log.buffer
#else
#	define		LOGBUFFER						log.
#	define		LOGPOS							log.length
#	define		LOGPOSADD						+ 	LOGPOS
#	define		ENVIRONS_LOGBUFFER_MAXSIZE		256000
#	define		LOGBUFFEREMAIN					(ENVIRONS_LOGBUFFER_MAXCHARS - LOGPOS)
#	define		LOG_OUT_BUFFER_NAME				log.buffer
#	define		LOG_OUT_BUFFER_NAME_LENGTH		log.length
#	define		LOG_OUT_BUFFER_NAME_ARG_CALL	0
#endif

	void env_printf ( const char * msg )
	{
#ifndef NDEBUG

#	ifdef _WIN32
		OutputDebugStringA ( msg );
#	endif // -> end-_WIN32


#	ifdef ANDROID
		__android_log_print ( 0, ENVIRONS_LOG_TAG_ID, "%s", msg );
#	endif  // -> end-_ANDROID


#	if !defined(ANDROID) && !defined(_WIN32) // <-- ANY other platform, e.g. Linux

		printf ( "%s", msg );

#	endif  // -> end-_ANDROID
#endif

	}

	struct LogBuffer
	{
		bool			alive;
		bool			push;
		char			buffer [ ENVIRONS_LOGBUFFER_MAXSIZE ];

		int				length;

#ifndef NDEBUG
		INTEROPTIMEVAL	lastPrint;
#endif
		LogBuffer ()
		{
			env_printf ( CLASS_NAME ".LogBuffer: Construct\n" );

			if ( !MutexInit ( &environsLogMutex ) )
				return;

			alive = true; push = false;
			*buffer = 0;
#ifndef NDEBUG
			length = 0; lastPrint = 0;
#endif
		}

		~LogBuffer ()
		{
			env_printf ( CLASS_NAME ".LogBuffer: Destruct\n" );

			//native.useLogFile = false;

#ifndef NDEBUG
			if ( length > 0 ) {
				push = true;

#	ifdef ANDROID
				COutLog ( 0, buffer, length, false );
#	else
				COutLog ( buffer, length, false );
#	endif

			}
#endif
			//CloseLog ();

			alive = false;

			MutexDispose ( &environsLogMutex );

			env_printf ( CLASS_NAME ".LogBuffer: Destruct done.\n" );
		}
	};

	LogBuffer	log;


    bool OpenLog ()
    {
        CVerbN ( "OpenLog" );
        
        if ( !native.workDir || !*native.workDir )
            return false;
        
        char fileName [ 1024 ];
        
        sprintf ( fileName, "%s/environs.log", native.workDir );

		STAT_STRUCT ( st );

        if ( stat ( fileName, &st ) == 0 )
        {
            size_t fileSize = (size_t) st.st_size;
            if ( fileSize > 10000000 ) {
                if ( unlink ( fileName ) ) {
                    if ( environsLogFileErrCount < 10 ) {
                        printf ( "OpenLog: ERROR ---> Failed to remove old logfile." );
                        environsLogFileErrCount++;
                    }
                    return false;
                }
            }
        }
        
        
        FILE * fp = fopen ( fileName, "a" );
        if ( !fp ) {
            if ( environsLogFileErrCount < 10 ) {
                printf ( "OpenLog: ERROR ---> Failed to open/create logfile." );
                //CVerbArgN ( "OpenLog: Error %d", errno );
                environsLogFileErrCount++;
            }
            else
                native.useLogFile = false;
            
            return false;
        }
        
        environsLogFileHandle = fp;
        
        return true;
    }
    
    
    void CloseLog ()
    {
        CVerbN ( "CloseLog" );
        
		bool locked = false;

		if ( log.alive ) {
			if ( pthread_mutex_lock ( &environsLogMutex ) )
				printf ( "CloseLog: ERROR ---> Failed to lock mutex." );
			else
				locked = true;
        }
        
        if ( environsLogFileHandle ) {
            fclose ( environsLogFileHandle );
            environsLogFileHandle = 0;
        }
        
		if ( locked && pthread_mutex_unlock ( &environsLogMutex ) ) {
            printf ( "CloseLog: ERROR ---> Failed to unlock mutex." );
        }
        CVerbN ( "CloseLog done" );
    }


    void DisposeLog ()
    {
        CVerbN ( "DisposeLog" );

		bool locked = false;

		if ( log.alive ) {
			if ( pthread_mutex_lock ( &environsLogMutex ) )
				printf ( "DisposeLog: ERROR ---> Failed to lock mutex." );
			else
				locked = true;
		}
        
        CloseLog ();

		if ( locked && pthread_mutex_unlock ( &environsLogMutex ) ) {
			printf ( "DisposeLog: ERROR ---> Failed to unlock mutex." );
		}

        CVerbN ( "DisposeLog done" );
    }
    
    
#ifdef ANDROID
    void COutLog ( int tag, const char * msg, int length, bool useLock )
#else
	void COutLog ( const char * msg, int length, bool useLock )
#endif
	{
		if ( length <= 0 ) {
			length = ( int ) strlen ( msg );
			//printf ( "COutLog: Length [%i].", length );

			if ( length <= 0 ) {
				printf ( "COutLog: ERROR ---> Invalid length [%i].", length );
				return;
			}
		}

		bool locked = false;

		if ( useLock && log.alive ) {
			if ( pthread_mutex_lock ( &environsLogMutex ) ) {
				printf ( "COutLog: ERROR ---> Failed to lock mutex." );
				return;
			}
			else
				locked = true;
		}

#ifndef NDEBUG
		if ( msg ) {
			int len = sprintf_s ( log.buffer LOGPOSADD, LOGBUFFEREMAIN - 1, "%s", msg );

			LOGPOS += len;
		}

		INTEROPTIMEVAL now = 0;

		// Output if min output size has been exceeded or 500ms has passed since last output
		if ( log.length < ENVIRONS_LOGBUFFER_OUTPUT_MINSIZE && ( ( now = GetEnvironsTickCount () ) - log.lastPrint ) < ENVIRONS_LOGBUFFER_OUTPUT_AX_MS ) 
		{
			if ( locked && pthread_mutex_unlock ( &environsLogMutex ) ) {
				printf ( "COutLog: ERROR ---> Failed to unlock mutex." );
			}
			return;
		}

		if ( !now )
			now = GetEnvironsTickCount ();
#endif
        if ( native.useLogFile ) {
            if ( environsLogFileHandle || OpenLog () ) {
                fwrite ( LOG_OUT_BUFFER_NAME, 1, LOG_OUT_BUFFER_NAME_LENGTH, environsLogFileHandle );
            }
        }
        
#ifdef _WIN32
        OutputDebugStringA ( LOG_OUT_BUFFER_NAME );
#endif // -> end-_WIN32
        
        
#ifdef ANDROID
        __android_log_print ( tag,	ENVIRONS_LOG_TAG_ID, "%s", LOG_OUT_BUFFER_NAME );
#endif  // -> end-_ANDROID
        
        
#if !defined(ANDROID) && !defined(_WIN32) // <-- ANY other platform, e.g. Linux
        
        printf ( "%s", LOG_OUT_BUFFER_NAME );
        
#endif  // -> end-_ANDROID
        
        if ( native.useNotifyDebugMessage ) {
            for ( int i=1; i<ENVIRONS_MAX_ENVIRONS_INSTANCES; ++i )
            {
                Instance * env = instances[i];
                if ( !env )
                    break;
                
                if ( env->callbacks.doOnStatusMessage )
                    env->callbacks.OnStatusMessage ( i, LOG_OUT_BUFFER_NAME );
            }
        }

#ifndef NDEBUG
		log.lastPrint = now; log.length = 0;
#endif
		if ( locked && pthread_mutex_unlock ( &environsLogMutex ) ) {
			printf ( "COutLog: ERROR ---> Failed to unlock mutex." );
		}
    }
    
    
#ifdef ANDROID
    void COutArgLog ( int tag, const char * format, ... )
#else
    void COutArgLog ( const char * format, ... )
#endif
    {
        va_list marker;

		bool locked = false;

		if ( log.alive ) {
			if ( pthread_mutex_lock ( &environsLogMutex ) ) {
				printf ( "COutArgLog: ERROR ---> Failed to lock mutex." );
				return;
			}
			locked = true;
        }
        
        va_start ( marker, format );
        int length = (int) vsnprintf ( log.buffer LOGPOSADD, LOGBUFFEREMAIN - 1, format, marker );

#ifndef NDEBUG
		LOGPOS += length;
#endif

#ifdef ANDROID
		COutLog ( tag, LOG_OUT_BUFFER_NAME_ARG_CALL, LOG_OUT_BUFFER_NAME_LENGTH, false );
#else
		COutLog ( LOG_OUT_BUFFER_NAME_ARG_CALL, LOG_OUT_BUFFER_NAME_LENGTH, false );
#endif
        va_end ( marker );
        
        if ( locked && pthread_mutex_unlock ( &environsLogMutex ) ) {
            printf ( "COutArgLog: ERROR ---> Failed to unlock mutex." );
        }
    }
    
}





