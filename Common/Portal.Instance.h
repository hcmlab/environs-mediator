/**
 * PortalInstance CPP Object
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_H
#define INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_H

#include "Interop/Smart.Pointer.h"
#include "Interfaces/IPortal.Instance.h"
#include "Notify.Context.h"


/**
 *	PortalInstance CPP Object
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
/* Namespace: environs -> */
namespace environs 
{
	namespace lib 
	{
		class DeviceInstance;
		class PortalObserver;
		class IIPortalObserver;
        

		class PortalInstance : public IPortalInstance
		{
            friend class Environs;
			friend class DeviceList;
            friend class DeviceInstance;
            friend class PortalInstanceProxy;
            friend class DeviceInstanceProxy;
            
		public:
			ENVIRONS_LIB_API PortalInstance ();
            ENVIRONS_LIB_API ~PortalInstance ();
            
            
            /** An ID that identifies this portal across all available portals. */
            int                     portalID ();
            
            
            /** true = Object is disposed and not updated anymore. */
            bool                    disposed ();
            
            
            /**
             * Get an Interface to the DeviceInstance that this PortalInstance is attached to.
             * */
            IDeviceInstance *       deviceRetainedI ();
            
            
            environs::PortalInfo *  info ();
            
            
            int                     status ();
            bool                    disposeOngoing ();
            
            
            /** true = outgoing (Generator), false = incoming (Receiver). */
            bool                    outgoing ();
            
            /** true = outgoing (Generator), false = incoming (Receiver). */
            bool                    isOutgoing ();
            
            /** true = outgoing (Generator), false = incoming (Receiver). */
            bool                    isIncoming ();
            
            
			virtual environs::PortalType_t  portalType ();

            
            ENVIRONS_LIB_API void AddObserver ( PortalObserver * observer );
            ENVIRONS_LIB_API void RemoveObserver ( PortalObserver * observer );
            
            ENVIRONS_LIB_API int GetObserverCount ();
            ENVIRONS_LIB_API bool Establish ( bool askForType );
            
            ENVIRONS_LIB_API bool Start ();
            ENVIRONS_LIB_API bool Stop ();
            
            ENVIRONS_LIB_API bool HasPortalRenderer ();
            
            ENVIRONS_LIB_API bool SetRenderSurface ( void * surface_ );
            ENVIRONS_LIB_API bool SetRenderSurface ( void * surface_, int width, int height );
            
            ENVIRONS_LIB_API bool ReleaseRenderSurface ();
            ENVIRONS_LIB_API PortalInfoBase * GetIPortalInfo ( );
            
            static bool SetPortalInfo ( int hInst, PortalInfoBase * infoBase );
            ENVIRONS_LIB_API bool SetIPortalInfo ( PortalInfoBase * infoBase );
            

			/**
			* Release ownership on this interface and mark it disposeable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
             */
            ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

		private:
			ENVIRONS_OUTPUT_ALLOC_RESOURCE ( PortalInstance );
                        
            static bool             globalsInit;
            static bool				GlobalsInit ();
            static void				GlobalsDispose ();
            
			static smsp ( int, PortalInstance )	portals;
            
            int                     hEnvirons_;
            
            /** An ID that identifies this portal across all available portals. */
            int                     portalID_;
            
            /** true = Object is disposed and not updated anymore. */
            bool                    disposed_;
            
            void                    PlatformDispose ();
            
            environs::PortalInfo    info_;
            
            int                     status_;
            bool                    disposeOngoing_;
            
            /** true = outgoing (Generator), false = incoming (Receiver). */
            bool                    outgoing_;
            
			environs::PortalType_t  portalType_;
            
            static sp ( PortalInstance ) GetPortal ( int nativeID, int portalID );
            
            static sp ( PortalInfoBase ) GetPortalInfo ( int hInst, int portalID );
            
            /** A DeviceInstance object that this portal relates to. */
            sp ( DeviceInstance )   device_;

			std::vector < IIPortalObserver * >    observers;

			int                 dialogID;
			bool                networkOK;
			INTEROPTIMEVAL      createdTickCount;


			bool                establishCalled;
			bool                askForTypeValue;

			void			*    surface;
			bool                renderActivated;

			int					portalKey;

            static int GetKey ( int nativeID, int portalID_ );

			bool CreateInstance ( const sp ( DeviceInstance ) &device, int Environs_PORTAL_DIR_, environs::PortalType_t type, int slot );
			bool Create ( const sp ( DeviceInstance ) &device, int destID );
			bool Create ( const sp ( DeviceInstance ) &device, int Environs_PORTAL_DIR_, environs::PortalType_t type, int slot );

			static void * PortalPresenterThread ( void * pack );
			static void PresentPortalToObservers ( const sp ( PortalInstance ) &portal, int notification );

			static void HandleSuccessfulPortal ( const sp ( PortalInstance ) &portal, int notification );
            
			static bool UpdateWidthHeight ( const sp ( PortalInstance ) &portal, void * context );
            static void UpdateOptions ( NotifyContext * ctx );
            
            void Update ();
            static void Update ( int hInst, int nativeID, int notification, int portalID, void * context );
            
            bool CheckNetworkConnection ();
            void ShowDialogOutgoingPortalView ();
            void ShowDialogNoWiFiWarn ();
            void ShowDialogOutgoingPortal ();
            
			void NotifyObservers ( int notification);
			void Dispose ();
            static void Dispose ( int nativeID, int portalID );

            static void * Destroyer ( void * _targets );
			static void KillZombies ();
		};
	}
}

#endif	/// INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_H

