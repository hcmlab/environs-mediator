/**
* Environs build options
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
#ifndef INCLUDE_HCM_ENVIRONS_BUILD_OPTIONS_COMMON_H
#define INCLUDE_HCM_ENVIRONS_BUILD_OPTIONS_COMMON_H


#define ENVIRONS_IOSX_USE_CPP_API
#define DISABLE_MEM_CHECKS

#ifndef NO_ML_TESTS
#	define NO_ML_TESTS
#endif

#ifndef NDEBUG
#ifndef MEDIATORDAEMON
#	define DEBUGSocketLog
#	define DEBUG_TRACK_SOCKET
#else
//#	define DEBUGSocketLog
//#	define DEBUG_TRACK_SOCKET
#endif
#endif

//#define DEBUGSocketCreateLog

#if defined(__APPLE__) && !defined(NDEBUG)
//#   define XCODE_ANALYZER_BUG
#endif

#define ENABLE_IOS_HEALTHKIT_SUPPORT

//#define ENABLE_MESSAGE_EXT_DISPATCH

#ifdef ENABLE_MESSAGE_EXT_DISPATCH
#   define ENABLE_MESSAGE_EXT_HANDLER
#endif

#ifdef DISPLAYDEVICE
#   define MAX_CONNECTED_DEVICES            120
#	define MAX_PORTAL_INSTANCES				40
#   define MEDIATOR_REC_BUFFER_SIZE_MAX     MEDIATOR_REC_BUFFER_SIZE_MAX_FULL

#   define ENVIRONS_MAX_ENVIRONS_INSTANCES  ENVIRONS_MAX_ENVIRONS_INSTANCES_FULL

#else
#   define MAX_CONNECTED_DEVICES            60
#	define MAX_PORTAL_INSTANCES				10
#   define MEDIATOR_REC_BUFFER_SIZE_MAX     MEDIATOR_REC_BUFFER_SIZE_MAX_MOBILE

#   define ENVIRONS_MAX_ENVIRONS_INSTANCES  ENVIRONS_MAX_ENVIRONS_INSTANCES_MOBILE
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

#define MAX_BULK_SEND_SIZE					2000000000

#define USE_ADDR_REUSE_ONLY_FOR_CONNECT

//#define USE_THREAD_NAME_ASSIGN_INSTRUCTION

#define USE_MEDIATOR_OPT_KEY_MAPS_COMP

#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
#	define ENVIRONS_DEVICE_KEY_EXT +4
#	define APPAREATYPE		AppAreaKey
#else
#	define ENVIRONS_DEVICE_KEY_EXT
#	define APPAREATYPE		const char
#endif

#if defined(_WIN32)
#   define USE_MEDIATOR_CLIENT_WINSOCK_SOCKETS
#endif

#define CONCURRENT_STUNT_LOGLEVEL   3

#if defined(_WIN32) && !defined(MEDIATORDAEMON)
#   define USE_MEDIATOR_NON_BLOCK_BROADCAST_WINSOCK
#endif

#define ENABLE_DEVICEBASE_WEAK_REFERENCE
#ifdef ENABLE_DEVICEBASE_WEAK_REFERENCE
#   define DEV_WP  .lock()
#else
#   define DEV_WP
#endif

#define ENABLE_INSTANCE_WEAK_REFERENCE
#ifdef ENABLE_INSTANCE_WEAK_REFERENCE
#   define MED_WP  .lock()
#else
#   define MED_WP
#endif

//#define USE_NSLOG
//#define USE_PORTAL_THREADS_FOR_IOSX_CAM

#define ENABLE_DEVICEBASE_WP_STUN
#define ENABLE_DEVICEBASE_WP_STUNT

#define ENABLE_IOS_NATIVE_H264_ONLY
//#define ENABLE_IOS_STATIC_TOUCH_LISTENER

//#define USE_OLD_STYLE_SENSORS

//#define ENABLE_WIND3D_CAPTURE

#define DEBUG_PORTAL_HEXSTRING

//#define ENABLE_PORTAL_STALL_MECHS

//#define ENABLE_IMPROVED_PORTAL_GENERATOR

//#define	ENVIRONS_ENABLE_RECOGNIZER_MANAGER	1

/// Removed. Needs to be verified for windows and android platforms
#define USE_WORKER_THREADS

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
#define ENVIRONS_DYNAMIC_PORTS_UPSTEPS				5000

/// Move recongizer source code into an recongizers object
//#define ENABLE_RECOGNIZERS_OBJECT
//#define ENABLE_RECOGNIZERS_OBJECT_USAGE

//#define ENABLE_EXT_BIND_IN_STUNT

#define PARTITION_MIN_BUFFER_REQUIREMENT	131072
#define PARTITION_SEND_BUFFER_SIZE			64000
#define PARTITION_PART_SIZE					(PARTITION_SEND_BUFFER_SIZE - MSG_PARTITIONS_HEADER_SIZE)

#if defined(MEDIATORDAEMON) && !defined(NDEBUG)
//#	define MEASURE_LOCK_ACQUIRE
#endif

#ifndef NDEBUG
//#   define TEST_DIFFERENT_CRT	"v120"
#endif

#ifdef __cplusplus
#   define ENVIRONS_NAMESP environs::
#else
#   define ENVIRONS_NAMESP
#endif

#ifndef DISABLE_MEM_CHECKS
#	if ( defined(_WIN32) && !defined(NDEBUG) )
#		ifdef MEDIATORDAEMON
#			define _USE_VLD
#		else
#			if (_MSC_VER >= 1800 || defined(NO_ML_TESTS))
#				define _USE_VLD
#			else
#				define USE_CRT_MLC
#			endif
#		endif

#		ifdef _USE_VLD
#			define _C_X64
#		endif

#		ifdef USE_CRT_MLC
#			define _CRTDBG_MAP_ALLOC
#			ifndef WINDOWS_PHONE
#				include <stdlib.h>
#			endif
#			include <crtdbg.h>
#		endif

//		The following header will be created by buildOpts.bat 
//		Its aim is to disable vld if it is not available on the system 
#		include "Environs.Build.Opts.VLD.h"

#		ifdef _USE_VLD
#			ifdef _C_X64
#				include "C:/Program Files (x86)/Visual Leak Detector/include/vld.h"
#			else
#				include "C:/Program Files/Visual Leak Detector/include/vld.h"
#			endif
#		endif
#	endif
#endif

#endif


