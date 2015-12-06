/**
 * Common native declarations
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

/**
 * Build flags
 * ****************************************************************************************
 */
//#define DEBUGVERB
//#define DEBUGVERBVerb
//#define DEBUGVERBLocks
//#define DEBUGCIPHERS

#include "Environs.Platforms.h"
#include "Environs.Build.Opts.h"
#include "Environs.Build.Macros.h"

/**
 * Include native type and data structure declarations
 * ****************************************************************************************
 */
#include "Environs.Types.h"

/**
 * Log macros and forwards
 *
 **/
#include "Log.h"

#ifdef ANDROID
#	include <android/log.h>
#else
// Logging/Tracing/Debug defines for Windows/Linux/iOS devices
#	include <stdlib.h>
#	include <string.h>
#endif


#ifndef INCLUDE_HCM_ENVIRONS_NATIVE_COMMON_H
#define INCLUDE_HCM_ENVIRONS_NATIVE_COMMON_H


#ifdef DISPLAYDEVICE
#	define MAX_PORTAL_INSTANCES				40
#else
#	define MAX_PORTAL_INSTANCES				10
#endif

#define ENVIRONS_USER_PASSWORD_LENGTH		64
#define ENVIRONS_MAX_KEYSIZE				4096

#ifdef __APPLE__
#	define ENVIRONS_PRIVATE_KEYNAME			"env.4321a.bin"
#	define ENVIRONS_PUBLIC_CERTNAME			"env.1234a.bin"
#else
#	define ENVIRONS_PRIVATE_KEYNAME			"env.4321.bin"
#	define ENVIRONS_PUBLIC_CERTNAME			"env.1234.bin"
#endif

#define ENVIRONS_MEDIATOR_MAX_TRYS          10

#define ENVIRONS_TOUCH_RECOGNIZER_MAX       6
#define ENVIRONS_INPUT_RECOGNIZER_MAX       10

#define MAX_CONNECTED_DEVICES               120


//#define USE_MEDIATOR_OPT_KEY_MAPS_COMP

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
#	define ENVIRONS_DEVICE_KEY_EXT +4
#else
#	define ENVIRONS_DEVICE_KEY_EXT
#endif

//#define USE_NSLOG
//#define USE_PORTAL_THREADS_FOR_IOSX_CAM

#define ENABLE_IOS_NATIVE_H264_ONLY
//#define ENABLE_IOS_STATIC_TOUCH_LISTENER


//#define ENABLE_WIND3D_CAPTURE

#define DEBUG_PORTAL_HEXSTRING

//#define ENABLE_PORTAL_STALL_MECHS

//#define ENABLE_IMPROVED_PORTAL_GENERATOR

//#define	ENVIRONS_ENABLE_RECOGNIZER_MANAGER	1


#define USE_CALMED_BUFFER_ADAPT

/// Removed. Needs to be verified for windows and android platforms
#define USE_WORKER_THREADS
#define ENABLE_ENCRYPTION

#define USE_WORKER_THREADS_INITIAL_CONTEXT		1

#define ENABLE_WORKER_STAGES_LOCKS
//#define ENABLE_WORKER_STAGES_COMPARE

//#define INCREASE_TCP_SOCKET_BUFFERS

#define ENABLE_NATIVE_H264DECODER

#ifdef _WIN32
//#	define PERFORMANCE_MEASURE
#endif

// NAT behaviour compiler flags
/// This is problematic for connections through the internet
//#define ENABLE_TCL_NODELAY_FOR_NAT
#define DISABLE_BUFFER_TUNING_FOR_NAT

//#define	LIMIT_MAX_NATIVE_RESOLUTION

/// Search ports dynamically starting with the base port up to 100 above
#define ENVIRONS_DYNAMIC_PORTS_UPSTEPS				100

/// Move recongizer source code into an recongizers object
//#define ENABLE_RECOGNIZERS_OBJECT
//#define ENABLE_RECOGNIZERS_OBJECT_USAGE


#define PARTITION_MIN_BUFFER_REQUIREMENT	131072
#define PARTITION_SEND_BUFFER_SIZE			64000
#define PARTITION_CHUNK_SIZE				(PARTITION_SEND_BUFFER_SIZE - MSG_CHUNKED_HEADER_SIZE)


/// Fake 42 test device entries for device information retrieval through the mediator
//#define FAKE42


#ifdef __cplusplus

namespace environs {
    extern void   *   pEnvirons;
}

#endif

/**
* Helper, Wrapper
* ****************************************************************************************
*/
#define		OBJ_RELEASE(obj)					if(obj) { obj->Release(); obj = 0; }

#define		Zero(mem)							memset(&mem,0,sizeof(mem))
#define		GetPortalIndex(p)					(p & 0xFF)
#define		RemovePortalIndex(p)				(p & 0xFFFFFF00)
#define		GetPortalDeviceID(p)				((p >> 24) & 0xFF)
#define		IsInvalidPortalDeviceID(p)			(p < 0 || p >= MAX_PORTAL_INSTANCES )
#define		IsInvalidPortalIndex(p)				(p < 0 || p >= MAX_PORTAL_STREAMS_A_DEVICE )
#define		IsValidPortalIndex(p)				(p >= 0 && p < MAX_PORTAL_STREAMS_A_DEVICE )
#define		PortalIndex()						(portalID & 0xFF)
#define		IsPortalGenerator()					(portalID & PORTAL_DIR_OUTGOING)
#define		IsPortalIDGenerator(p)				(p & PORTAL_DIR_OUTGOING)
#define		IsPortalReceiver()					(portalID & PORTAL_DIR_INCOMING)
#define		IsPortalIDReceiver()				(p & PORTAL_DIR_INCOMING)
#define		ClearPortalDir()					(portalID &= ~(PORTAL_DIR_INCOMING | PORTAL_DIR_OUTGOING))
#define     ReversePortalDirGet()				((portalID & ~PORTAL_DIR_MASK) | (~portalID & PORTAL_DIR_MASK))
#define     ReversePortalDir(portalID)          portalID = ((portalID & ~PORTAL_DIR_MASK) | (~portalID & PORTAL_DIR_MASK))
#define		ClearPortalDeviceID(p)				(p & 0xFFFFFF)

#define     IsStatus(n)                         (n < 0 && (n & 0xF))
#define     IsNotStatus(n)                      (n > 0 || (n & 0xF) == 0)
#define     GetStatusValue(n)                         (0 - n)

#define		ClearBit(val,mask)					(val &= ~mask)
#define		AddBit(val,mask)					(val |= mask)
#define		GetPad(val,align)					((val % align) ? (align - (val % align)) : 0)

#define		OBJECTSTATE_DELETEABLE_1			1
#define		OBJECTSTATE_DELETEABLE_2			2
#define		OBJECTSTATE_DELETED					0


// Defines
#define		TCP_RECV_CONNECT_BUFFER_SIZE_BEGIN	1024
#define		MSG_BUFFER_SEND_SIZE				1024
#define 	REC_BUF_SIZE						256000
#define 	REC_BUF_SIZE_MAX					2000000
#define		UDP_MAX_SIZE						66000
#define		DATA_BUFFER_COUNT					5
#define		DATA_BUFFER_SIZE					1000000
#define		STREAM_BUFFER_SIZE_MIN				100000
#define		TCP_MSG_PROTOCOL_VERSION			2
#define		UDP_MSG_PROTOCOL_VERSION			3
#define		UDP_MSG_EXT_PROTOCOL_VERSION		4

#define     NATIVE_DEFAULT_TCP_PORT				5901
#define     NATIVE_DEFAULT_UDP_PORT				5901

#define     ENVIRONS_DEBUG_TAGID				0xDD

#define		PORTAL_SOCKET_BUFFER_SIZE_NORMAL	30000
#define		PORTAL_SOCKET_BUFFER_SIZE_NATRES	90000


#ifndef PI
#	define PI 3.14159265
#endif

#ifdef DEBUGVERB
#   define ASSERT_ENV(exp,fn,msg)     if ( !(exp) ) CErr (fn ": exp(" ENVIRONS_TOSTRING(exp) ") " msg);
#else
#   define ASSERT_ENV(exp,fn,msg)
#endif

#ifdef ANDROID
#	define	ENVIRONS_LOG_NL
#else
#	define	ENVIRONS_LOG_NL								"\n"
#endif



#ifdef ANDROID
#	define ENVIRONS_LOG_NRCMD(tag,expression)					__android_log_print ( tag,	ENVIRONS_LOG_TAG_ID, "%s", expression )
#	define ENVIRONS_LOGARG_NRCMD(tag,expression,...)			__android_log_print ( expression, __VA_ARGS__ )
#else
#	ifdef _WIN32
#		define ENVIRONS_LOG_NRCMD(tag,expression)				OutputDebugStringA ( expression )
#		define ENVIRONS_LOGARG_NRCMD(tag,expression,...)		OutputDebugStringA ( expression ) //, __VA_ARGS__ )
#	else
#		if (defined(__APPLE__) && defined(USE_NSLOG))
#			define ENVIRONS_LOG_NRCMD(tag,expression)			NSLog ( @"%s", expression )
#			define ENVIRONS_LOGARG_NRCMD(tag,expression,...)	NSLog ( @expression, __VA_ARGS__ )
#		else
#			define ENVIRONS_LOG_NRCMD(tag,expression)			printf ( expression )
#			define ENVIRONS_LOGARG_NRCMD(tag,expression,...)	printf ( expression, __VA_ARGS__ )
#		endif
#	endif
#endif


#ifdef ENVIRONS_CORE_LIB


#ifdef ANDROID
#	define ENVIRONS_LOG_RCMD(tag,expression)                environs::COutLog ( tag, expression, 0, true )
#	define ENVIRONS_LOGARG_RCMD(tag,expression,...)         environs::COutArgLog ( tag, expression, __VA_ARGS__ )
#elif CLI_CPP
#	define ENVIRONS_LOG_RCMD(tag,expression)                environs::COutLog ( CLASS_NAME, expression )
#	define ENVIRONS_LOGARG_RCMD(tag,expression)             environs::COutLog ( CLASS_NAME, expression )
#else
#	define ENVIRONS_LOG_RCMD(tag,expression)                environs::COutLog ( expression, 0, true )
#	define ENVIRONS_LOGARG_RCMD(tag,expression,...)         environs::COutArgLog ( expression, __VA_ARGS__ )
#endif

#else // not environs core lib follows


#ifdef MEDIATORDAEMON
	extern void MLog ( const char * msg );
	extern void MLogArg ( const char * msg, ... );

#	define ENVIRONS_LOG_RCMD(tag,expression)                MLog ( expression )
#	define ENVIRONS_LOGARG_RCMD(tag,expression,...)         MLogArg ( expression, __VA_ARGS__ )
#else
#	ifdef ANDROID
#		define ENVIRONS_LOG_RCMD(tag,expression)            ((environs::Instance *) pEnvirons)->cOutLog ( tag, expression, 0, true )
#		define ENVIRONS_LOGARG_RCMD(tag,expression,...)     ((environs::Instance *) pEnvirons)->cOutArgLog ( tag, expression, __VA_ARGS__ )
#	else
#		define ENVIRONS_LOG_RCMD(tag,expression)            ((environs::Instance *) pEnvirons)->cOutLog ( expression, 0, true )
#		define ENVIRONS_LOGARG_RCMD(tag,expression,...)     ((environs::Instance *) pEnvirons)->cOutArgLog ( expression, __VA_ARGS__ )
#	endif
#endif

#endif // <-- ENVIRONS_CORE_LIB



#define ENVIRONS_LOG_TAG_ID							"Environs"
#define ENVIRONS_VERB_PREFIX						"[VERB]  "
#define ENVIRONS_LOG_PREFIX							"[TRACE] "
#define ENVIRONS_INFO_PREFIX						"[INFO]  "
#define ENVIRONS_WARN_PREFIX						"[WARN]  "
#define ENVIRONS_ERR_PREFIX							"[ERROR] -- E -->: "
#define ENVIRONS_LOCK_PREFIX						"[VERB] --> Lock   "
#define ENVIRONS_UNLOCK_PREFIX						"[VERB] <-- Unlock "

#define	ENVIRONS_LOGTAG_INFO						ANDROID_LOG_INFO
#define	ENVIRONS_LOGTAG_WARN						ANDROID_LOG_WARN
#define	ENVIRONS_LOGTAG_ERROR						ANDROID_LOG_ERROR
#define	ENVIRONS_LOGTAG_VERBOSE						ANDROID_LOG_VERBOSE

#ifdef CLI_CPP
#	define	ENVIRONS_MAKE_BODY(tag,msg)				tag, msg
#	define	ANDROID_LOG_INFO						1
#	define	ANDROID_LOG_WARN						1
#	define	ANDROID_LOG_ERROR						1
#	define	ANDROID_LOG_VERBOSE						1
#else
#	define	ENVIRONS_MAKE_BODY(tag,msg)				tag " " CLASS_NAME " " msg ENVIRONS_LOG_NL
#	define	ENVIRONS_MAKE_BODY_ID(tag,msg)			tag " " CLASS_NAME " [0x%X] " msg ENVIRONS_LOG_NL
#endif

#define ENVIRONS_VERB_NCMD(expression)				ENVIRONS_LOG_NRCMD ( ENVIRONS_LOGTAG_VERBOSE,	expression )
#define ENVIRONS_VERB_CMD(expression)				ENVIRONS_LOG_RCMD ( ENVIRONS_LOGTAG_VERBOSE,	expression )
#define ENVIRONS_LOG_CMD(expression)				ENVIRONS_LOG_RCMD ( ENVIRONS_LOGTAG_VERBOSE,	expression )
#define ENVIRONS_LOG_NCMD(expression)				ENVIRONS_LOG_NRCMD ( ENVIRONS_LOGTAG_VERBOSE,	expression )
#define ENVIRONS_INFO_CMD(expression)				ENVIRONS_LOG_RCMD ( ENVIRONS_LOGTAG_INFO,		expression )
#define ENVIRONS_WARN_CMD(expression)				ENVIRONS_LOG_RCMD ( ENVIRONS_LOGTAG_WARN,		expression )
#define ENVIRONS_ERR_CMD(expression)				ENVIRONS_LOG_RCMD ( ENVIRONS_LOGTAG_ERROR,		expression )
#define ENVIRONS_ERR_NCMD(expression)				ENVIRONS_LOG_NRCMD ( ENVIRONS_LOGTAG_ERROR,		expression )

#define ENVIRONS_VERBRG_CMD(expression,...)			ENVIRONS_LOGARG_RCMD ( ENVIRONS_LOGTAG_VERBOSE, expression, __VA_ARGS__ )
#define ENVIRONS_VERBRG_NCMD(expression,...)		ENVIRONS_LOGARG_NRCMD ( ENVIRONS_LOGTAG_VERBOSE, expression, __VA_ARGS__ )
#define ENVIRONS_LOGARG_CMD(expression,...)			ENVIRONS_LOGARG_RCMD ( ENVIRONS_LOGTAG_VERBOSE, expression, __VA_ARGS__ )
#define ENVIRONS_INFOARG_CMD(expression,...)		ENVIRONS_LOGARG_RCMD ( ENVIRONS_LOGTAG_INFO,	expression, __VA_ARGS__ )
#define ENVIRONS_WARNARG_CMD(expression,...)		ENVIRONS_LOGARG_RCMD ( ENVIRONS_LOGTAG_WARN,	expression, __VA_ARGS__ )
#define ENVIRONS_ERRARG_CMD(expression,...)			ENVIRONS_LOGARG_RCMD ( ENVIRONS_LOGTAG_ERROR,	expression, __VA_ARGS__ )

#define CVerb(msg)									ENVIRONS_VERB_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg ) )
#define CVerbN(msg)									ENVIRONS_VERB_NCMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg ) )
#define CVerbVerb(msg)								ENVIRONS_VERB_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg ) )
#define CVerbVerbN(msg)								ENVIRONS_VERB_NCMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg ) )
#define CLog(msg)									ENVIRONS_LOG_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg ) )
#define CListLog(msg)								ENVIRONS_LOG_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg ) )
#define CLogN(msg)									ENVIRONS_LOG_NCMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg ) )
#define CInfo(msg)									ENVIRONS_INFO_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_INFO_PREFIX,	msg ) )
#define CWarn(msg)									ENVIRONS_WARN_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_WARN_PREFIX,	msg ) )
#define CErr(msg)									ENVIRONS_ERR_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_ERR_PREFIX,	msg ) )
#define CErrN(msg)									ENVIRONS_ERR_NCMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_ERR_PREFIX,	msg ) )

#define CVerbArg(msg,...)							ENVIRONS_VERBRG_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg), __VA_ARGS__ )
#define CVerbArgN(msg,...)							ENVIRONS_VERBRG_NCMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg), __VA_ARGS__ )
#ifdef CLI_CPP
#	define CVerbArg1(msg,name1,type1,arg1)			ENVIRONS_VERB_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg + " " + name1 + ": [" + arg1 + "]" ) )
#else
#	define CVerbArg1(msg,name1,type1,arg1)			ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg " " name1 ": [%" type1 "]" ), arg1 )
#endif
#ifdef CLI_CPP
#	define CVerbArg2(msg,name1,type1,arg1,name2,type2,arg2)		\
	ENVIRONS_VERB_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg + " " + name1 + ": [" + arg1 + "] " + name2 + ": [" + arg2 + "]" ) )
#else
#	define CVerbArg2(msg,name1,type1,arg1,name2,type2,arg2)		\
	ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg " " name1 ": [%" type1 "] " name2 ": [%" type2 "] " ), arg1, arg2 )
#endif

#define CVerbVerbArg(msg,...)						ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg), __VA_ARGS__ )
#ifdef CLI_CPP
#	define CVerbVerbArg1(msg,name1,type1,arg1)		ENVIRONS_VERB_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg + " " + name1 + ": [" + arg1 + "]" ) )
#else
#	define CVerbVerbArg1(msg,name1,type1,arg1)		ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg " " name1 ": [%" type1 "]" ), arg1 )
#endif
#ifdef CLI_CPP
#	define CVerbVerbArg2(msg,name1,type1,arg1,name2,type2,arg2)		\
	ENVIRONS_VERB_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg + " " + name1 + ": [" + arg1 + "] " + name2 + ": [" + arg2 + "]" ) )
#else
#	define CVerbVerbArg2(msg,name1,type1,arg1,name2,type2,arg2)		\
	ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_VERB_PREFIX,	msg " " name1 ": [%" type1 "] " name2 ": [%" type2 "] " ), arg1, arg2 )
#endif

#define CLogArg(msg,...)							ENVIRONS_LOGARG_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg), __VA_ARGS__ )
#define CListLogArg(msg,...)						ENVIRONS_LOGARG_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg), __VA_ARGS__ )
#define CLogArgN(msg,...)							ENVIRONS_VERBRG_NCMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg), __VA_ARGS__ )

#ifdef CLI_CPP
#	define CLogArg1(msg,name1,type1,arg1)			ENVIRONS_LOG_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg + " " + name1 + ": [" + arg1 + "]" ) )
#else
#	define CLogArg1(msg,name1,type1,arg1)			ENVIRONS_LOGARG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_LOG_PREFIX,	msg " " name1 ": [%" type1 "]" ), arg1 )
#endif

#define CListLogArg1(msg,name1,type1,arg1)			CLogArg1 ( msg,name1,type1,arg1)

#define CInfoArg(msg,...)							ENVIRONS_INFOARG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_INFO_PREFIX,	msg), __VA_ARGS__ )
#define CWarnArg(msg,...)							ENVIRONS_WARNARG_CMD ( ENVIRONS_MAKE_BODY	( ENVIRONS_WARN_PREFIX,	msg), __VA_ARGS__ )
#define CErrArg(msg,...)							ENVIRONS_ERRARG_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_ERR_PREFIX,	msg), __VA_ARGS__ )

#ifdef CLI_CPP
#	define CErrArg1(msg,name1,type1,arg1)			ENVIRONS_ERR_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_ERR_PREFIX,	msg + " " + name1 + ": [" + arg1 + "]" ) )
#else
#	define CErrArg1(msg,name1,type1,arg1)			ENVIRONS_ERRARG_CMD  ( ENVIRONS_MAKE_BODY	( ENVIRONS_ERR_PREFIX,	msg " " name1 ": [%" type1 "]" ), arg1 )
#endif

#define CVerbID(msg)								ENVIRONS_VERBRG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg ), deviceID )
#define CVerbVerbID(msg)							ENVIRONS_VERBRG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg ), deviceID )
#define CLogID(msg)									ENVIRONS_LOGARG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_LOG_PREFIX,	msg ), deviceID )
#define CInfoID(msg)								ENVIRONS_INFOARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_INFO_PREFIX,	msg ), deviceID )
#define CWarnID(msg)								ENVIRONS_WARNARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_WARN_PREFIX,	msg ), deviceID )
#define CErrID(msg)									ENVIRONS_ERRARG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_ERR_PREFIX,	msg ), deviceID )

#ifdef CLI_CPP
#	define CVerbIDN(msg)							CVerb(msg)
#	define CVerbVerbIDN(msg)						CVerb(msg)
#	define CLogIDN(msg)								CLog(msg)
#	define CInfoIDN(msg)							CInfo(msg)
#	define CWarnIDN(msg)							CWarn(msg)
#	define CErrIDN(msg)								CErr(msg)
#else
#	define CVerbIDN(msg)							ENVIRONS_VERBRG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg ), nativeID )
#	define CVerbVerbIDN(msg)						ENVIRONS_VERBRG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg ), nativeID )
#	define CLogIDN(msg)								ENVIRONS_LOGARG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_LOG_PREFIX,	msg ), nativeID )
#	define CInfoIDN(msg)							ENVIRONS_INFOARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_INFO_PREFIX,	msg ), nativeID )
#	define CWarnIDN(msg)							ENVIRONS_WARNARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_WARN_PREFIX,	msg ), nativeID )
#	define CErrIDN(msg)								ENVIRONS_ERRARG_CMD	( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_ERR_PREFIX,	msg ), nativeID )
#endif

#define CVerbArgID(msg,...)							ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg), deviceID, __VA_ARGS__ )
#define CVerbVerbArgID(msg,...)						ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg), deviceID, __VA_ARGS__ )
#define CLogArgID(msg,...)							ENVIRONS_LOGARG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_LOG_PREFIX,	msg), deviceID, __VA_ARGS__ )
#define CInfoArgID(msg,...)							ENVIRONS_INFOARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_INFO_PREFIX,	msg), deviceID, __VA_ARGS__ )
#define CWarnArgID(msg,...)							ENVIRONS_WARNARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_WARN_PREFIX,	msg), deviceID, __VA_ARGS__ )
#define CErrArgID(msg,...)							ENVIRONS_ERRARG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_ERR_PREFIX,	msg), deviceID, __VA_ARGS__ )

#define CVerbArgIDN(msg,...)						ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg), nativeID, __VA_ARGS__ )
#define CVerbVerbArgIDN(msg,...)					ENVIRONS_VERBRG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_VERB_PREFIX,	msg), nativeID, __VA_ARGS__ )
#define CLogArgIDN(msg,...)							ENVIRONS_LOGARG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_LOG_PREFIX,	msg), nativeID, __VA_ARGS__ )
#define CInfoArgIDN(msg,...)						ENVIRONS_INFOARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_INFO_PREFIX,	msg), nativeID, __VA_ARGS__ )
#define CWarnArgIDN(msg,...)						ENVIRONS_WARNARG_CMD( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_WARN_PREFIX,	msg), nativeID, __VA_ARGS__ )
#define CErrArgIDN(msg,...)							ENVIRONS_ERRARG_CMD ( ENVIRONS_MAKE_BODY_ID	( ENVIRONS_ERR_PREFIX,	msg), nativeID, __VA_ARGS__ )

#define CVerbLock(msg)								ENVIRONS_INFO_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_LOCK_PREFIX,	msg ) )
#define CVerbUnLock(msg)							ENVIRONS_INFO_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_UNLOCK_PREFIX, msg ) )

#define CVerbLockPortalRecRes(msg)					ENVIRONS_INFO_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_LOCK_PREFIX,	msg ": resourcesMutex" ) )
#define CVerbUnLockPortalRecRes(msg)				ENVIRONS_INFO_CMD	( ENVIRONS_MAKE_BODY	( ENVIRONS_UNLOCK_PREFIX, msg ": resourcesMutex" ) )


#if (!defined(DEBUGVERBList) || defined(NDEBUG))
#	undef	CListLog
#	define CListLog(msg)
#	undef	CListLogArg
#	define CListLogArg(msg,...)
#	undef	CListLogArg1
#	define CListLogArg1(msg,...)
#endif

#if (!defined(DEBUGVERB) || defined(NDEBUG))
#	undef	CVerbN
#	define CVerbN(msg)
#	undef	CVerb
#	define CVerb(msg)
#	undef	CVerbArg
#	define CVerbArg(msg,...)
#	undef	CVerbArg1
#	define CVerbArg1(msg,...)
#	undef	CVerbArg2
#	define CVerbArg2(msg,...)
#	undef	CVerbArgN
#	define CVerbArgN(msg,...)
#	undef	CVerbID
#	define CVerbID(msg)
#	undef	CVerbIDN
#	define CVerbIDN(msg)
#	undef	CVerbArgID
#	define CVerbArgID(msg,...)
#	undef	CVerbArgIDN
#	define CVerbArgIDN(msg,...)
#endif

#if (!defined(DEBUGVERBVerb) || defined(NDEBUG))
#	undef	CVerbVerb
#	define CVerbVerb(msg)
#	undef	CVerbVerbN
#	define CVerbVerbN(msg)
#	undef	CVerbVerbArg
#	define CVerbVerbArg(msg,...)
#	undef	CVerbVerbArg1
#	define CVerbVerbArg1(msg,...)
#	undef	CVerbVerbArg2
#	define CVerbVerbArg2(msg,...)
#	undef	CVerbVerbID
#	define CVerbVerbID(msg)
#	undef	CVerbVerbIDN
#	define CVerbVerbIDN(msg)
#	undef	CVerbVerbArgID
#	define CVerbVerbArgID(msg,...)
#	undef	CVerbVerbArgIDN
#	define CVerbVerbArgIDN(msg,...)
#endif

#if (!defined(DEBUGVERBLocks) || defined(NDEBUG))
#	undef	CVerbLock
#	define CVerbLock(msg)
#	undef	CVerbUnLock
#	define CVerbUnLock(msg)
#	undef	CVerbLockPortalRecRes
#	define CVerbLockPortalRecRes(msg)
#	undef	CVerbUnLockPortalRecRes
#	define CVerbUnLockPortalRecRes(msg)
#endif

#if (defined(CLI_CPP1) || (defined(NDEBUG) && defined(RELEASELIB)))
#	undef	CLog
#	define CLog(msg)
#	undef	CLogArg
#	define CLogArg(msg,...)
#	undef	CLogArg1
#	define CLogArg1(msg,...)
#	undef	CLogID
#	define CLogID(msg)
#	undef	CLogArgID
#	define CLogArgID(msg,...)
#	undef	CLogN
#	define CLogN(msg)
#	undef	CLogArgN
#	define CLogArNg(msg,...)
#	undef	CLogIDN
#	define CLogIDN(msg)
#	undef	CLogArgIDN
#	define CLogArgIDN(msg,...)
#	undef	CInfo
#	define CInfo(msg)
#	undef	CInfoID
#	define CInfoID(msg)
#	undef	CInfoArg
#	define CInfoArg(msg,...)
#	undef	CInfoArgID
#	define CInfoArgID(msg,...)
#	undef	CInfoN
#	define CInfoN(msg)
#	undef	CInfoIDN
#	define CInfoIDN(msg)
#	undef	CInfoArgN
#	define CInfoArgN(msg,...)
#	undef	CInfoArgIDN
#	define CInfoArgIDN(msg,...)
#	undef	CWarn
#	define CWarn(msg)
#	undef	CWarnID
#	define CWarnID(msg)
#	undef	CWarnArg
#	define CWarnArg(msg,...)
#	undef	CWarnArgID
#	define CWarnArgID(msg,...)
#	undef	CWarnN
#	define CWarnN(msg)
#	undef	CWarnIDN
#	define CWarnIDN(msg)
#	undef	CWarnArgN
#	define CWarnArgN(msg,...)
#	undef	CWarnArgIDN
#	define CWarnArgIDN(msg,...)
#	undef	CErr
#	define CErr(msg)
#	undef	CErrID
#	define CErrID(msg)
#	undef	CErrArg
#	define CErrArg(msg,...)
#	undef	CErrArgID
#	define CErrArgID(msg,...)
#	undef	CListLog
#	define CListLog(msg,...)
#	undef	CErrN
#	define CErrN(msg)
#	undef	CErrIDN
#	define CErrIDN(msg)
#	undef	CErrArgN
#	define CErrArgN(msg,...)
#	undef	CErrArgIDN
#	define CErrArgIDN(msg,...)
#	undef	CListLogN
#	define CListLogN(msg,...)
#endif

#endif  /// end-INCLUDE_HCM_ENVIRONS_NATIVE_COMMON_H



