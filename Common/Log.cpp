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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef _WIN32
#   include "windows.h"
#endif

#ifdef ANDROID
#   include <android/log.h>
#endif

#include "Environs.Native.h"
#include "Interop/Threads.h"
#include "Interop/Stat.h"
#include "Environs.Obj.h"
#include "Core/Callbacks.h"
using namespace environs;


#define CLASS_NAME   "Log"

#define ENVIRONS_LOGFILE_TEMP_MAXSIZE               200000

FILE            *   environsLogFileHandle           = 0;
int                 environsLogFileErrCount         = 0;


namespace environs
{
	pthread_mutex_t     environsLogMutex;


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
    
    
    void CloseLog ( bool useLock )
    {
        CVerbN ( "CloseLog" );
        
		if ( useLock && pthread_mutex_lock ( &environsLogMutex ) ) {
            printf ( "CloseLog: ERROR ---> Failed to lock mutex." );
        }
        
        if ( environsLogFileHandle ) {
            fclose ( environsLogFileHandle );
            environsLogFileHandle = 0;
        }
        
		if ( useLock && pthread_mutex_unlock ( &environsLogMutex ) ) {
            printf ( "CloseLog: ERROR ---> Failed to unlock mutex." );
        }
        CVerbN ( "CloseLog done" );
    }
    
    
    void DisposeLog ()
    {
        CVerbN ( "DisposeLog" );
        
		MutexLockV ( &environsLogMutex, "DisposeLog" );
        
        CloseLog ( false );
        
		MutexUnlockV ( &environsLogMutex, "DisposeLog" );

        CVerbN ( "DisposeLog done" );
    }
    
    
#ifdef ANDROID
    void COutLog ( int tag, const char * msg, int length, bool useLock )
#else
    void COutLog ( const char * msg, int length, bool useLock )
#endif
    {
        if ( length <= 0 ) {
            length = (int)strlen ( msg );
            //printf ( "COutLog: Length [%i].", length );
        }
        
        if ( length <= 0 ) {
            printf ( "COutLog: ERROR ---> Invalid length [%i].", length );
            return;
        }
        
        if ( useLock && pthread_mutex_lock ( &environsLogMutex ) ) {
            printf ( "COutLog: ERROR ---> Failed to lock mutex." );
            return;
        }
        
        if ( native.useLogFile ) {
            if ( environsLogFileHandle || OpenLog () ) {
                fwrite ( msg, 1, length, environsLogFileHandle );
            }
        }
        
#ifdef _WIN32
        OutputDebugStringA ( msg );
#endif // -> end-_WIN32
        
        
#ifdef ANDROID
        __android_log_print ( tag,	ENVIRONS_LOG_TAG_ID, "%s", msg );
#endif  // -> end-_ANDROID
        
        
#if !defined(ANDROID) && !defined(_WIN32) // <-- ANY other platform, e.g. Linux
        
        printf ( "%s", msg );
        
#endif  // -> end-_ANDROID
        
        if ( native.useNotifyDebugMessage ) {
            for ( int i=1; i<ENVIRONS_MAX_ENVIRONS_INSTANCES; ++i )
            {
                Instance * env = instances[i];
                if ( !env )
                    break;
                
                if ( env->callbacks.doOnStatusMessage )
                    env->callbacks.OnStatusMessage ( i, msg );
            }
        }
        
        if ( useLock ) {
            if ( pthread_mutex_unlock ( &environsLogMutex ) ) {
                printf ( "COutLog: ERROR ---> Failed to unlock mutex." );
            }
        }
    }
    
    
#ifdef ANDROID
    void COutArgLog ( int tag, const char * format, ... )
#else
    void COutArgLog ( const char * format, ... )
#endif
    {
        char buffer [1024];
        va_list marker;
        
        if ( pthread_mutex_lock ( &environsLogMutex ) ) {
            printf ( "COutArgLog: ERROR ---> Failed to lock mutex." );
            return;
        }
        
        va_start ( marker, format );
        int len = (int) vsnprintf ( buffer, 1024, format, marker );
        
#ifdef ANDROID
        COutLog ( tag, buffer, len, false );
#else
        COutLog ( buffer, len, false );
#endif
        va_end ( marker );
        
        if ( pthread_mutex_unlock ( &environsLogMutex ) ) {
            printf ( "COutArgLog: ERROR ---> Failed to unlock mutex." );
        }
    }
    
}





