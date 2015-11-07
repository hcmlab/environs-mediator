/**
 * DeviceLists
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICELISTS_H
#define INCLUDE_HCM_ENVIRONS_DEVICELISTS_H

#include "Interop/Threads.h"
#include "Interop/Smart.Pointer.h"
#include "Interfaces/IDevice.List.h"
#include "Notify.Context.h"
#include "Environs.Msg.Types.h"

/**
 *	DeviceLists
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
namespace environs
{
    class Instance;
    
    
    namespace lib
    {
        class Environs;
		class DeviceInstance;
        class DeviceCommandContext;
        

		class DeviceList : public IDeviceList
        {
            friend class Environs;
            
            friend class DeviceListProxy;
            
        public:
			ENVIRONS_LIB_API DeviceList ();
			ENVIRONS_LIB_API ~DeviceList ();
            
            static bool				GlobalsInit ();
            static void				GlobalsDispose ();
            
			ENVIRONS_LIB_API bool	disposed ();
            
			ENVIRONS_LIB_API void	SetListType ( int MEDIATOR_DEVICE_CLASS_ );
            
            sp ( DeviceInstance )	GetItemSP ( int position );
            
			ENVIRONS_LIB_API IDeviceInstance * GetItemRetainedI ( int position );
            
			ENVIRONS_LIB_API int	GetCount ();
            
			ENVIRONS_LIB_API void	AddObserver ( ListObserver * observer );
			ENVIRONS_LIB_API void	RemoveObserver ( ListObserver * observer );
            
            sp ( DeviceInstance )   RefreshItemSP ( DeviceInstance * source, DeviceObserver * observer );
            
			ENVIRONS_LIB_API IDeviceInstance *       RefreshItemRetainedI ( IDeviceInstance * source, DeviceObserver * observer );

			/**
			* Get a collection that holds the devices according to the specified listtype. This list ist updated dynamically by Environs.
			*
			* @return Collection with DeviceInstance objects
			*/
			const svsp ( DeviceInstance ) & GetDevicesSP ();

			ENVIRONS_LIB_API IArrayList * GetDevicesRetainedI ();
            
            
            /**
             * Query the number of the devices according to the specified listtype within the environment.
             *
             * @return numberOfDevices
             */
			ENVIRONS_LIB_API int GetDevicesCount ();


			/**
			* Query a DeviceInstance object from the devices according to the specified listtype within the environment.
			*
			* @param deviceID      The device id of the target device.
			* @param areaName      Area name of the application environment
			* @param appName		Application name of the application environment
			* @return DeviceInstance-object
			*/
			sp ( DeviceInstance ) GetDeviceSP ( int deviceID, const char * areaName, const char * appName, int * pos );

			ENVIRONS_LIB_API IDeviceInstance * GetDeviceRetainedI ( int deviceID, const char * areaName, const char * appName, int * pos );
            
            
            /**
             * Query a DeviceInstance object from the devices according to the specified listtype devices within the environment.
             *
             * @param nativeID      The device id of the target device.
             * @return DeviceInstance-object
             */
            sp ( DeviceInstance ) GetDeviceSP ( int nativeID );
            
			ENVIRONS_LIB_API IDeviceInstance * GetDeviceRetainedI ( int nativeID );


			/**
			* Query a DeviceInstance object from all available devices within the environment (including those of the Mediator).
			*
			* @param nativeID      The device id of the target device.
			* @return DeviceInstance-object
			*/
            sp ( DeviceInstance ) GetDeviceAllSP ( int nativeID );
            
            
            /**
             * Query a DeviceInstance object that best match the deviceID only.
             * Usually the one that is in the same app environment is picked up.
             * If there is no matching in the app environment,
             * then the areas are searched for a matchint deviceID.
             *
             * @param deviceID      The portalID that identifies an active portal.
             * @return DeviceInstance-object
             */
            sp ( DeviceInstance ) GetDeviceBestMatchSP ( int deviceID );
            
			ENVIRONS_LIB_API IDeviceInstance * GetDeviceBestMatchRetainedI ( int deviceID );
            
            
            /**
             * Query a DeviceInstance object that best match the deviceID only from native layer.
             * Usually the one that is in the same app environment is picked up.
             * If there is no matching in the app environment,
             * then the areas are searched for a matchint deviceID.
             *
             * @param deviceID      The portalID that identifies an active portal.
             * @return DeviceInstance-object
             */
            sp ( DeviceInstance ) GetDeviceBestMatchNativeSP ( int deviceID );
            
			ENVIRONS_LIB_API IDeviceInstance * GetDeviceBestMatchNativeRetainedI ( int deviceID );
            
            
            /**
             * Get a collection that holds the nearby devices. This list ist updated dynamically by Environs.
             *
             * @return ArrayList with DeviceInstance objects
             */
            svsp ( DeviceInstance ) GetDevicesNearbySP ();
            
			ENVIRONS_LIB_API IArrayList * GetDevicesNearbyRetainedI ();
            
            
            /**
             * Query the number of nearby (broadcast visible) devices within the environment.
             *
             * @return numberOfDevices
             */
			ENVIRONS_LIB_API int GetDevicesNearbyCount ();
            
            
            /**
             * Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
             *
             * @param nativeID      The device id of the target device.
             * @return DeviceInstance-object
             */
            sp ( DeviceInstance ) GetDeviceNearbySP ( int nativeID );
            
			ENVIRONS_LIB_API IDeviceInstance * GetDeviceNearbyRetainedI ( int nativeID );
            
            
            /**
             * Release the ArrayList that holds the nearby devices.
             */
			ENVIRONS_LIB_API void ReleaseDevicesNearby ();
            
            
            /**
             * Get a collection that holds the Mediator server devices. This list ist updated dynamically by Environs.
             *
             * @return ArrayList with DeviceInstance objects
             */
            svsp ( DeviceInstance ) GetDevicesFromMediatorSP ();
            
			ENVIRONS_LIB_API IArrayList * GetDevicesFromMediatorRetainedI ();
            
            
            /**
             * Query a DeviceInstance object of Mediator managed devices within the environment.
             *
             * @param nativeID      The device id of the target device.
             * @return DeviceInstance-object
             */
            sp ( DeviceInstance ) GetDeviceFromMediatorSP ( int nativeID );
            
			ENVIRONS_LIB_API IDeviceInstance * GetDeviceFromMediatorRetainedI ( int nativeID );
            
            
            /**
             * Query the number of Mediator managed devices within the environment.
             *
             * @return numberOfDevices
             */
			ENVIRONS_LIB_API int GetDevicesFromMediatorCount ();
            
            /**
             * Release the ArrayList that holds the Mediator server devices.
             */
			ENVIRONS_LIB_API void ReleaseDevicesMediator ();
            
            /**
             * Refresh all device lists. Applications may call this if they manually stopped and started Environs again.
             * Environs does not automatically refresh the device lists so as to allow applications to add observers before refreshing of the lists.
             */
			ENVIRONS_LIB_API void RefreshDeviceLists ();            


			/**
			* Release ownership on this interface and mark it disposeable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
             */
            ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

		private:
			ENVIRONS_OUTPUT_ALLOC_RESOURCE ( DeviceList );
			
            int                             hEnvirons;
            Instance                    *   env;
            Environs                    *   envObj;

            static bool                     globalsInit;
            bool                            disposed_;
            int                             listType;
            
            pthread_mutex_t     *           listDevicesLock;
            svsp ( DeviceInstance )         listDevices;
			spv ( IIListObserver * )        listDevicesObservers;
            
            
            void                            PlatformDispose ();
            
            const svsp ( DeviceInstance ) & GetDevices ( int type );
            
            static sp ( DeviceInstance ) GetDeviceSP ( const svsp ( DeviceInstance ) &deviceList, int deviceID, const char * areaName, const char * appName, int * pos );
            
            static sp ( DeviceInstance ) GetDeviceSP ( const svsp ( DeviceInstance ) &deviceList, int nativeID, int * pos );
            
            sp ( DeviceInstance ) GetDeviceSP ( int nativeOrDeviceID, bool isNativeID );

            sp ( DeviceInstance ) GetDeviceAllSP ( int nativeOrDeviceID, bool isNativeID );

            static sp ( DeviceInstance ) GetDeviceSeekerSP ( const svsp ( DeviceInstance ) &list, pthread_mutex_t * lock, int nativeOrDeviceID, bool isNativeID );
            
            
            /**
             * Release the ArrayList that holds the available devices.
             */
            void ReleaseDevices ();
            
            void DisposeLists ();
            
            static void DisposeList ( const svsp ( DeviceInstance ) &list );
            
            static void DeviceListUpdater ( Environs * api, int listType );
            
            
            static void * DeviceListCommandThread ( void * pack );
            
            
            static void OnDeviceListNotification ( int hInst, int deviceID, const char * areaName, const char * appName, int notification, void * context );
            
            static sp ( DeviceInstance ) RemoveDevice ( const svsp ( DeviceInstance ) &list, pthread_mutex_t &lock, void * pack );
            
            static void RemoveDevice ( void * pack );
            
            static void UpdateDevice ( void * pack );
            
            static void InsertDevice ( int hInst, const svsp ( DeviceInstance ) &deviceList, pthread_mutex_t &listLock, const sp ( DeviceInstance ) &deviceNew,
				const spv ( IIListObserver * ) &observerList );
            
            static void EnqueueCommand ( DeviceCommandContext * ctx );
            
			static void NotifyListObservers ( int hInst, const spv ( IIListObserver * ) &observerList, svsp ( DeviceInstance ) vanished, svsp ( DeviceInstance ) appeared, bool enqueue );
            
            
            
            static void TakeOverToList ( Environs * api, const svsp ( DeviceInstance ) &list, bool getMediator );
            
            
            static void UpdateConnectProgress ( pthread_mutex_t * lock, const svsp ( DeviceInstance ) &list, int nativeID, int progress );
            
            static void UpdateMessage ( pthread_mutex_t * lock, const svsp ( DeviceInstance ) &list, ObserverMessageContext * ctx );
            
            static void UpdateData ( pthread_mutex_t * lock, const svsp ( DeviceInstance ) &list, ObserverDataContext * ctx );
            
            static void UpdateSensorData ( pthread_mutex_t * lock, const svsp ( DeviceInstance ) &list, int nativeID, environs::SensorFrame * pack );
            
        };
    }
}


#endif	/// INCLUDE_HCM_ENVIRONS_DEVICELISTS_H


