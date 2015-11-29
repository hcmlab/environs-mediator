/**
 * Interface declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_INTERFACES_DISPOSE_H
#define	INCLUDE_HCM_ENVIRONS_INTERFACES_DISPOSE_H

#include "Environs.Platforms.h"
#include "Environs.Build.Opts.h"
#include "Interop/Threads.h"


namespace environs
{

#ifndef CLI_CPP

#define DERIVE_DISPOSEABLE	
#define DERIVE_TYPES		

	namespace lib
	{
		/**
		* IEnvironsDispose
		*
		*/
		PUBLIC_CLASS ENVIRONS_LIB_API IEnvironsDispose
		{
#ifndef CLI_CPP
			friend class PortalInstance;
			friend class DeviceList;
			friend class DeviceInstance;
			friend class MessageInstance;
			friend class FileInstance;
			friend class Environs;
#endif
		public:
			/** Constructor */
			IEnvironsDispose () { };

			virtual ~IEnvironsDispose () { };


			/**
			* Increase ownership by one in order to prevent releasing of the resources behind the interface.
			* Each increase MUST be balanced by a call to Release.
			* Otherwise the resources behind can not be released by Environs.
			*
			*/
			virtual bool Retain () = 0;


			/**
			* Release ownership on this interface and mark it disposeable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
			*/
			virtual void Release () = 0;

		protected:
			/**
			* Internal Release used by Environs to clean up internal resources while object is locked.
			*
			*/
			virtual void ReleaseLocked () { };
		};
	}

	void EnvironsDisposer ( lib::IEnvironsDispose * obj );
#else

	using namespace System;

#	define DERIVE_DISPOSEABLE		: IDisposable	
#	define DERIVE_TYPES				, public Types

#endif
}


#ifndef CLI_CPP

/*
 * myself is a pointer to the smart pointer instance within Environs.
 * myselfAtClients will hold a smart pointer to the instance within Environs as long as a client code acquired a reference to the object.
 *
 *
 */

#define ENVIRONS_OUTPUT_ALLOC_RESOURCE(type)	sp ( type ) myself; sp ( type ) myselfAtClients


#define ENVIRONS_OUTPUT_DE_ALLOC_DECL()         ENVIRONS_LIB_API bool	Retain (); \
                                                ENVIRONS_LIB_API void	Release (); \
                                                void * platformRef; \
                                            private: \
                                                LONGSYNC				refCountSP; \
                                                LONGSYNC                objID_;

/*
#define ENVIRONS_OUTPUT_DE_ALLOC_DECL()         ENVIRONS_LIB_API bool	Retain (); \
												ENVIRONS_LIB_API void	Release (); \
                                                void * platformRef; \
												private: \
                                                int						refCountSP; \
												pthread_mutex_t			refCountMutex; long objID_;
*/

#define ENVIRONS_OUTPUT_ALLOC_INIT()			myself = 0; refCountSP = 0; myselfAtClients = 0; platformRef = 0; objID_ = __sync_add_and_fetch ( &objectIdentifiers, 1 );

/*
#define ENVIRONS_OUTPUT_ALLOC_INIT()			myself = 0; refCountSP = 0; myselfAtClients = 0; MutexInit ( &refCountMutex ); platformRef = 0; objID_ = __sync_add_and_fetch ( &objectIdentifiers, 1 );
*/
 
#define ENVIRONS_OUTPUT_ALLOC(type)				extern LONGSYNC objectIdentifiers; \
bool type::Retain () { \
	CVerbVerbArg ( "Retain [%i]: [%i]", objID_, refCountSP );  \
	\
	LONGSYNC localRefCount = __sync_add_and_fetch ( &refCountSP, 1 ); \
	\
    if ( refCountSP == 1 && myself ) { \
        CVerbVerbArg ( "Retain [%i]: -> Allocating SP", objID_ ); \
        myselfAtClients = myself; \
    } \
	CVerbVerbArg ( "Retain [%i]: -> [%i] ", objID_, localRefCount );  \
	\
	return (localRefCount > 0); \
} 
/*
bool type::Retain () { \
    CVerbVerbArg ( "Retain [%i]: [%i]", objID_, refCountSP );  \
    \
    bool success = true; \
    \
    MutexLockV ( &refCountMutex, "Retain" ); \
    \
    if ( refCountSP < 0 )  {\
        success = false; \
        CErrArg ( "Retain [%i]: Negative reference count!!!", objID_ );  \
    } \
    else \
    { \
        if ( refCountSP == 0 && myself ) { \
            CVerbVerbArg ( "Retain [%i]: -> Allocating SP", objID_ ); \
            myselfAtClients = myself; \
        } \
        ++refCountSP; \
    } \
    \
    CVerbVerbArg ( "Retain [%i]: -> [%i] ", objID_, refCountSP );  \
    \
    MutexUnlockV ( &refCountMutex, "Retain" ); \
    return success; \
}
*/

 /**
 * Release ownership on this interface and mark it disposeable.
 * Release must be called once for each Interface that the Environs framework returns to client code.
 * Environs will dispose the underlying object if no more ownership is hold by anyone.
 *
 * setting localRefCount = -1 means prohibit further reference allocations and signal custom deletion of the container
 */
#define ENVIRONS_OUTPUT_RELEASE()				 CVerbVerbArg ( "Release  [%i]: [%i]", objID_, refCountSP ); \
\
LONGSYNC localRefCount = __sync_sub_and_fetch ( &refCountSP, 1 ); \
\
if ( localRefCount == 0 )  { \
	ReleaseLocked (); \
} \
CVerbVerbArg ( "Release  [%i]: -> [%i]", objID_, localRefCount ); \
\
if ( localRefCount == 0 ) { \
	CVerbVerbArg ( "Release  [%i]: -> Disposing SP", objID_ );  \
	myselfAtClients = 0; \
}

/*
int localRefCount = 0; \
\
MutexLockV ( &refCountMutex, "Release" ); \
\
localRefCount = --refCountSP; \
\
if ( localRefCount == 0 )  { \
    ReleaseLocked (); \
} \
MutexUnlockV ( &refCountMutex, "Release" ); \
\
CVerbVerbArg ( "Release  [%i]: -> [%i]", objID_, localRefCount ); \
\
if ( localRefCount == 0 ) { \
    CVerbVerbArg ( "Release  [%i]: -> Disposing SP", objID_ );  \
    myselfAtClients = 0; \
}
*/

#if ((defined(ENVIRONS_IOS) || defined(ENVIRONS_OSX)))

#   define ENVIRONS_OUTPUT_PLATFORM_REF()          if ( platformRef && refCountSP == 1 ) { \
    platformRef = 0; \
    id transfered = (__bridge_transfer id) platformRef; \
    transfered = nil; \
}
#else
#   define ENVIRONS_OUTPUT_PLATFORM_REF()
#endif


#define ENVIRONS_I_SP1(type,call,arg1)             sp ( type ) sp1 ( arg1, ::environs::EnvironsDisposer ); arg1->Retain (); call ( sp1 );
#define ENVIRONS_I_SP2(type,call,arg1,arg2)        sp ( type ) sp1 ( arg1, ::environs::EnvironsDisposer ); arg1->Retain (); call ( sp1, arg2 );

#define ENVIRONS_IR_SP1_RETURN(type,arg1)           sp ( type ) sp1 ( (type *)arg1, ::environs::EnvironsDisposer ); return sp1

#define ENVIRONS_I_SP1_1(type,call,arg1,arg2)      sp ( type ) sp1 ( arg1, ::environs::EnvironsDisposer ); \
arg1->Retain (); call ( sp1, arg2 );

#define ENVIRONS_IR_SP2(type,call,arg1,arg2)        sp ( type ) sp1 ( (type *) arg1, ::environs::EnvironsDisposer ); \
	sp ( type ) sp2 ( (type *) arg2, ::environs::EnvironsDisposer ); \
	call ( environs_local_sp1, sp2 );

#define ENVIRONS_I_SP2_SEL(type,call,arg1,arg2)        sp ( type ) sp1; sp ( type ) sp2; \
	\
if ( arg1 ) { \
	arg1->Retain (); sp1 = sp ( type ) ( (type *) arg1, ::environs::EnvironsDisposer ); } \
if ( arg2 ) { \
	arg2->Retain (); sp2 = sp ( type ) ( (type *) arg2, ::environs::EnvironsDisposer ); } \
call ( sp1, sp2 );


#define ENVIRONS_I_BUILDSP_LIST(type,arg1)        sp ( type ) ent ( (type *)item ( arg1 ), ::environs::EnvironsDisposer ); if ( ent ) ent->Retain(); return ent;

#define ENVIRONS_I_BUILDSP_LISTITEM_ALLOC(type,arg1)  type * p1 = (type *)item ( arg1 ); \
	\
if ( p1 == 0 ) \
	return 0; \
((environs::lib::IEnvironsDispose *) p1)->Retain (); \
sp ( type ) ent ( p1, ::environs::EnvironsDisposer ); \
	return ent;

#else


#	define ENVIRONS_OUTPUT_ALLOC_RESOURCE(type)	
#	define ENVIRONS_OUTPUT_DE_ALLOC_DECL()				ENVIRONS_LIB_API void	Release (); \
														void * platformRef; \
													protected: \
														LONGSYNC                objID_; \
													private: 

#	define ENVIRONS_OUTPUT_ALLOC_INIT()					platformRef = 0; __int64 %tRef = objectIdentifiers; objID_ = __sync_add_and_fetch ( tRef, 1 );

#	define ENVIRONS_OUTPUT_ALLOC(type)					extern LONGSYNC objectIdentifiers;

#	define ENVIRONS_OUTPUT_RELEASE()						 

#	define ENVIRONS_I_SP1(type,call,arg1)				call ( arg1 );

#	define ENVIRONS_IR_SP1_RETURN(type,arg1)			return arg1

#endif


#endif	/// -> INCLUDE_HCM_ENVIRONS_INTERFACES_H









