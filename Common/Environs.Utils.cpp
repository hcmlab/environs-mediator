/**
 * Utils and common stuff
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
#include <stdint.h>

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "Environs.Native.h"
#include "Interop/Stat.h"
#include "Interop.h"

#if defined(_WIN32)
#   include "windows.h"
#else // <- _WIN32 ->

#   if defined(__APPLE__) || defined(ANDROID)
#       include <time.h>
#   else  // <- __APPLE__ | ANDROID ->
#       include <time.h>
#       include <sys/times.h>
#   endif

#   include <errno.h>
#endif // <-- end of _WIN32

#ifdef __APPLE__
#   include <mach/mach_time.h>
#endif

#define CLASS_NAME	"Environs.Utils . . . . ."


namespace environs
{
	void refactorBuffer ( char * &curStart, char * bufferStart, unsigned int bytesInBuffer, char * &curEnd )
	{
		CVerbVerbArg ( "refactorBuffer: Refactoring memory usage, bytesInBuffer = %i", bytesInBuffer );

		// Refactor memory usage
		int chunkSize = (int) (curStart - bufferStart);
		if ( chunkSize <= 0 ) {
			CVerbVerb ( "refactorBuffer: Refactoring not neccessary since buffer is already pointing to the start." );
			return;
		}

		if ( bytesInBuffer < (unsigned int) chunkSize ) { // No overlap
			memcpy ( bufferStart, curStart, bytesInBuffer );
		}
		else { // Overlap, so alloc heap space
            void * tmp = malloc ( bytesInBuffer );
            if ( !tmp ) {
                CErrArg ( "refactorBuffer: Allocation of memory failed [ %i ]", bytesInBuffer );
                return;
            }
            
            memcpy ( tmp, curStart, bytesInBuffer );
            
            memcpy ( bufferStart, tmp, bytesInBuffer );
            
            free ( tmp );
        }
        /*else { // Overlap, so copy multipass
            char * dest = bufferStart;
            char * src = curStart;
            
            int remain = bytesInBuffer - chunkSize;
            while ( remain > 0 ) {
                if ( remain < chunkSize )
                    chunkSize = remain;
                memcpy ( dest, src, chunkSize );
                
                dest += chunkSize;
                src += chunkSize;
                remain -= chunkSize;
                chunkSize += chunkSize;
            }
        }*/
		curStart = bufferStart;
		curEnd = curStart + bytesInBuffer;
	}


#ifdef _WIN32
#	pragma warning( push )
#	pragma warning( disable: 4996 )
#endif
    
    size_t GetSizeOfFile ( const char * filePath )
	{
#ifdef _WIN32
		HANDLE handle = CreateFileA ( filePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if ( handle == INVALID_HANDLE_VALUE)
			return 0;

		size_t fileSize = 0;

		LARGE_INTEGER size;

		if ( GetFileSizeEx( handle, &size ) )
		{
			fileSize = size.QuadPart;
		}

		CloseHandle ( handle );
		return fileSize;
#else
		STAT_STRUCT ( st );

        if ( stat ( filePath, &st ) != 0 )
            return 0;
        return (size_t) st.st_size;
#endif
    }
    
    
	char * LoadBinary ( const char * filePath, int * size )
	{
		// Get filesize if it exists 
		size_t fileSize = 0;
		int bytesRead = 0;
		bool ret = false;

        fileSize = GetSizeOfFile ( filePath );
        CVerbVerbArg ( "LoadBinary: Filesize [%d]", fileSize );
        
        if ( fileSize < 1 )
            return 0;

		char * binary = (char *) malloc ( fileSize + 2 );
        if ( !binary ) {
            CVerbVerb ( "LoadBinary: malloc failed" );
			return 0;
		}

        FILE * fp = fopen ( filePath, "rb" );
        CVerbVerbArg ( "LoadBinary: Opening [%s]", filePath );
        
		if ( !fp )
            goto Finish;
        CVerbVerbArg ( "LoadBinary: Opened [%s]", filePath );

		//rewind ( fp );
		bytesRead = (unsigned int) fread ( binary, 1, fileSize, fp );
		if ( bytesRead != (int) fileSize ){
			CErrArg ( "LoadBinary: Read file [%s] failed read [%i] != expected [%i].", filePath, bytesRead, (int) fileSize );
			goto Finish;
        }
        CVerbVerb ( "LoadBinary: Read" );

		binary [fileSize] = 0;
		if ( size )
			*size = (int) fileSize;
		ret = true;

	Finish:
		if ( !ret ) {
			free ( binary );
			binary = 0;
		}
		if ( fp )
			fclose ( fp );
        
		return binary;
	}


	char * LoadPrivateBinary ( const char * fileName, int * size )
	{
		return LoadBinary ( fileName, size );
	}


	bool SavePrivateBinary ( const char * fileName, const char * buffer, int size )
	{
		if ( !fileName || !buffer ) {
			CErr ( "SavePrivateBinary: Invalid NULL parameters." );
			return 0;
		}

		int bytesWritten = 0;
		bool ret = false;
		
		FILE * fp = fopen ( fileName, "wb" );
		if ( !fp ) {
			CVerbArg ( "SavePrivateBinary: Cannot create file [%s].", fileName );
			return false;
		}

		bytesWritten = (int)fwrite ( buffer, 1, size, fp );
		if ( bytesWritten != size ) {
			CVerbArg ( "SavePrivateBinary: Write file [%s] failed. Written [%i] != expected [%i].", fileName, bytesWritten, size );
			goto Finish;
		}
        fflush ( fp );

		ret = true;

	Finish:
        fclose ( fp );
		return ret;
	}

#ifdef _WIN32
#   pragma warning( pop )
#endif

    
#ifdef __APPLE__
    mach_timebase_info_data_t environs_time_base_info;
#endif


#if ( !defined(__APPLE__) && (defined(_WIN32) || defined(LINUX)) )
	size_t strlcpy ( char *d, char const *s, size_t n )
	{
		if ( !d || !s || n <= 0 )
			return 0;

		size_t c = 0;

		for ( --n; n > 0 && *s ; --n, ++d, ++s, ++c )
			*d = *s;

		*d = 0;

		return c;
	}

	size_t strlcat ( char *d, char const *s, size_t n )
	{
		if ( !d || !s || n <= 0 )
			return 0;

		for ( --n; n > 0 && *d ; --n, ++d );

		if ( n <= 0 )
			return 0;

		size_t c = 0;

		for ( ; n > 0 && *s ; --n, ++d, ++s, ++c )
			*d = *s;

		*d = 0;

		return c;
	}
#endif
    
//#ifndef _WIN32    
//#   ifndef ANDROID
//#       ifndef __APPLE__
//    size_t strlcpy ( char *d, char const *s, size_t n )
//    {
//        return snprintf ( d, n, "%s", s );
//    }
//#       endif
//    // <- Linux/iOS includes
//#   endif
//    // <- POSIX includes
//#endif
    
#ifdef _WIN32
	// Number of milliseconds since system has started
	//
    INTEROPTIMEVAL GetEnvironsTickCount ()
    {
        return ( INTEROPTIMEVAL ) GetTickCount64 ();
    }
    
#pragma warning(push)
#pragma warning(disable:4310)
    
    int gettimeofday ( struct timeval *tv, void * )
    {
        if ( !tv )
            return 0;
        
        static const LONGLONG        UNIX_DELTA_IN_MICSECS = 116444736000000000;
        
        ULARGE_INTEGER  ul, bunix;
        FILETIME        ft;
        LONGLONG        usecs;
        
        GetSystemTimeAsFileTime ( &ft );
        ul.LowPart  = ft.dwLowDateTime;
        ul.HighPart = ft.dwHighDateTime;
        
        bunix.LowPart = (DWORD)UNIX_DELTA_IN_MICSECS;
        bunix.HighPart = UNIX_DELTA_IN_MICSECS >> 32;
        
        usecs = (LONGLONG)((ul.QuadPart - bunix.QuadPart) / 10);
        
        tv->tv_sec = (long)(usecs /   1000000);
        tv->tv_usec = (long)(usecs % 1000000);
        
        return 0;
    }
    
#pragma warning(pop) 
    
#endif
    
    
#ifdef __APPLE__
    // return milliseconds
    INTEROPTIMEVAL GetEnvironsTickCount ()
    {
        if ( !environs_time_base_info.denom )
            mach_timebase_info ( &environs_time_base_info );
        
        return ((mach_absolute_time() / 1000000) * environs_time_base_info.numer) / environs_time_base_info.denom;
    }
#endif
    
#if !defined(_WIN32) && !defined(__APPLE__)
    // return milliseconds
    INTEROPTIMEVAL GetEnvironsTickCount ()
    {
        struct timespec	ts;
        
        if ( clock_gettime ( CLOCK_REALTIME, &ts ) != 0 ) {
            CErrArg ( "GetEnvironsTickCount: Failed to fetch time [%s]", strerror ( errno ) );
            return 0;
        }
        
		return ( ( ts.tv_sec * 1000 ) + ( ts.tv_nsec  > 0 ? ( ts.tv_nsec / 1000000 ) : 0 ) );
    }
#endif
    
    
    unsigned long long   GetUnixEpoch ()
    {
        unsigned long long unixEpoch = 0;
        
#ifdef _WIN32
        SYSTEMTIME st;
        GetSystemTime( &st );
        
        FILETIME ft;
		if ( !SystemTimeToFileTime ( &st, &ft ) )
			return 0;
        
        unsigned long long ticks = (((unsigned long long) ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
        
		unixEpoch = ticks / 10000000 - 11644473600LL;
#else
        unixEpoch = time ( NULL );
#endif
        return unixEpoch;
    }

    
	unsigned int getRandomValue ( void * value )
	{
		unsigned int now = (unsigned int) GetEnvironsTickCount ();
		return now + (unsigned int) (uintptr_t) value;
	}

}
