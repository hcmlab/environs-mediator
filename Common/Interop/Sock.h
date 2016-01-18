/**
 *	Platform interop socket inclusion
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
#ifndef INCLUDE_HCM_ENVIRONS_INTEROP_SOCKETS_H
#define INCLUDE_HCM_ENVIRONS_INTEROP_SOCKETS_H
#include "Interop.h"
 
#if defined(_WIN32)

//#include <ws2ipdef.h>
#include <ws2tcpip.h>

#define LogSocketErrorF(f)					CWarnArg    ( "[%s]: SocketError [ %d ]",f, WSAGetLastError())
#define LogSocketErrorFID(f)				CWarnArgID  ( "[%s]: SocketError [ %d ]",f, WSAGetLastError())
#define VerbLogSocketErrorF(f)				CVerbArg    ( "[%s]: SocketError [ %d ]",f, WSAGetLastError())
#define LogSocketError1()					CWarnArg    ( "SocketError: [ %s ]",        strerror(errno))
#define VerbLogSocketError1()				CVerbArg    ( "SocketError: [ %s ]",        strerror(errno))
#define LogSocketError()					{ int err = WSAGetLastError(); CWarnArg    ( "SocketError: [ %d ]",        err); }
#define LogSocketErrorID()					{ int err = WSAGetLastError(); CWarnArgID  ( "SocketError: [ %d ]",        err); }
#define VerbLogSocketError()				{ int err = WSAGetLastError(); CVerbArg    ( "SocketError: [ %d ]",        err); }

#define SOCK_IN_PROGRESS					(WSAGetLastError() == WSAEWOULDBLOCK)
#define SOCKETRETRY()						{ int err1 = WSAGetLastError (); if ( err1 == WSAEWOULDBLOCK || err1 == WSATRY_AGAIN ) continue; }
#define SOCKETRETRYGOTO(label)				{ int err1 = WSAGetLastError (); if ( err1 == WSAEWOULDBLOCK || err1 == WSATRY_AGAIN ) goto label; }

#define DisableSIGPIPE(socki)				

#else

// -> POSIX includes
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>

#ifndef ANDROID
#include <ifaddrs.h>
#endif

#ifdef __APPLE__
// -> OSX/iOS includes
#define DisableSIGPIPE(socki)                {int value = 1; setsockopt(socki, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));}
#else
// -> Linux/Android includes
#define DisableSIGPIPE(socki)				signal(SIGPIPE, SIG_IGN);
#endif

#define LogSocketErrorF(f)					CWarnArg    ( "[%s]: SocketError [ %s ]",f, strerror(errno))
#define LogSocketErrorFID(f)				CWarnArgID  ( "[%s]: SocketError [ %s ]",f, strerror(errno))
#define VerbLogSocketErrorF(f)				CVerbArg    ( "[%s]: SocketError [ %s ]",f, strerror(errno))
#define LogSocketError()					CWarnArg    ( "SocketError: [ %s ]",        strerror(errno))
#define LogSocketError1()
#define VerbLogSocketError1()
#define LogSocketErrorID()					CWarnArgID  ( "SocketError: [ %s ]",        strerror(errno))
#define VerbLogSocketError()				CVerbArg    ( "SocketError: [ %s ]",        strerror(errno))

#define SOCK_IN_PROGRESS					(errno == EINPROGRESS || errno == EALREADY)
#define SOCK_CONNECTED						(errno == EISCONN)
#define SOCK_CON_REFUSED					(errno == ECONNREFUSED)

#define SOCKETRETRY()						{ int err1 = errno; if ( err1 == EWOULDBLOCK || err1 == EAGAIN ) continue; }
#define SOCKETRETRYGOTO(label)				{ int err1 = errno; if ( err1 == EWOULDBLOCK || err1 == EAGAIN ) goto label; }

#define closesocket(s) 						close(s)
#define WSACleanup()
#define WSAData								void *
#define WSAStartup(a,b)						((*b = 0) == (WSAData)1)
#endif


#ifndef DEBUGVERB
#undef VerbLogSocketErrorF
#define VerbLogSocketErrorF(f)

#undef VerbLogSocketError
#define VerbLogSocketError()

#undef VerbLogSocketError1
#define VerbLogSocketError1()

#endif


#ifdef __cplusplus
namespace environs
{
    
	extern void LimitLingerAndClose ( int &sock );

}
#endif


#endif // INCLUDE_HCM_ENVIRONS_INTEROP_SOCKETS_H
