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

#include "Environs.Observer.CLI.h"
#include "Interop/Threads.h"
#include "Interop/Smart.Pointer.h"
#include "Interfaces/IEnvirons.Dispose.h"
#include "Notify.Context.h"
#include "Environs.Msg.Types.h"

#ifdef CLI_CPP
#	include <cliext/queue>
	using namespace cliext;
#else
#	include "Interfaces/IDevice.List.h"
#endif

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

	namespace lib
	{
		CLASS Environs;
		CLASS DeviceInstance;
		CLASS DeviceCommandContext;


		PUBLIC_CLASS DeviceInstanceUpdateContext
		{
		public:
			sp ( EPSPACE DeviceInstance ) device;

			environs::DeviceInfo OBJ_ptr deviceInfo;
		};


		PUBLIC_CLASS DeviceListQueueItem
		{
		public:
			int     cmd;
			int     pos;

			sp ( EPSPACE DeviceInstance ) device;
		};

		PUBLIC_CLASS DeviceListUpdatePack
		{
		public:
            DeviceListUpdatePack () {
                api         = nill;
                devices     = nill;
                lock        = nill;
                updates     = nill;
            }
            
            pthread_mutex_t									OBJ_ptr		lock;
            
			devList ( EPSPACE DeviceInstance )							deviceList;
			NLayerVecTypeObj ( DeviceListQueueItem )                    items;

			NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr 	updates;

			NLayerVecType ( EPSPACE DeviceInstance )		vanished;
			NLayerVecType ( EPSPACE DeviceInstance )		appeared;

			Environs OBJ_ptr		api;
			int						listType;
			DeviceListItems			devices;
			int						devicesCount;
		};


		PUBLIC_CLASS DeviceList DERIVE_c_only ( environs::DeviceList ) DERIVE_DISPOSEABLE
		{
        
            MAKE_FRIEND_CLASS ( Environs );
            MAKE_FRIEND_CLASS ( DeviceListProxy );

        public:
			ENVIRONS_LIB_API DeviceList ();
			ENVIRONS_LIB_API ~DeviceList ();

			static bool				GlobalsInit ();
			static void				GlobalsDispose ();

			ENVIRONS_LIB_API bool	disposed ();

            ENVIRONS_LIB_API void	SetListType ( int MEDIATOR_DEVICE_CLASS_ );
        
            ENVIRONS_LIB_API void	SetIsUIAdapter ( bool enable );

			sp ( EPSPACE DeviceInstance )	GetItem ( int position );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetItemRetained ( int position );
#endif

			ENVIRONS_LIB_API int	GetCount ();

			ENVIRONS_LIB_API void	AddObserver ( environs::ListObserver OBJ_ptr observer );
			ENVIRONS_LIB_API void	RemoveObserver ( environs::ListObserver OBJ_ptr observer );

			sp ( EPSPACE DeviceInstance )   RefreshItem ( EPSPACE DeviceInstance OBJ_ptr source, DeviceObserver OBJ_ptr observer );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance OBJ_ptr RefreshItemRetained ( environs::DeviceInstance OBJ_ptr source, environs::DeviceObserver OBJ_ptr observer );
#endif
			/**
			* Get a collection that holds the devices according to the specified listtype. This list ist updated dynamically by Environs.
			*
			* @return Collection with DeviceInstance objects
			*/
			c_const devList ( EPSPACE DeviceInstance ) c_ref GetDevices ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::ArrayList * GetDevicesRetained ();
#endif


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
			sp ( EPSPACE DeviceInstance ) GetDevice ( int deviceID, CString_ptr areaName, CString_ptr appName, int * pos );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceRetained ( int deviceID, const char * areaName, const char * appName, int * pos );
#endif


			/**
			 * Query a DeviceInstance object from the devices according to the specified listtype devices within the environment.
			 *
			 * @param nativeID      The device id of the target device.
			 * @return DeviceInstance-object
			 */
			sp ( EPSPACE DeviceInstance ) GetDevice ( int nativeID );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceRetained ( int nativeID );
#endif


			/**
			* Query a DeviceInstance object from all available devices within the environment (including those of the Mediator).
			*
			* @param nativeID      The device id of the target device.
			* @return DeviceInstance-object
			*/
			sp ( EPSPACE DeviceInstance ) GetDeviceAll ( int nativeID );


			/**
			 * Query a DeviceInstance object that best match the deviceID only.
			 * Usually the one that is in the same app environment is picked up.
			 * If there is no matching in the app environment,
			 * then the areas are searched for a matchint deviceID.
			 *
			 * @param deviceID      The portalID that identifies an active portal.
			 * @return DeviceInstance-object
			 */
			sp ( EPSPACE DeviceInstance ) GetDeviceBestMatch ( int deviceID );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceBestMatchRetained ( int deviceID );
#endif


			/**
			 * Query a DeviceInstance object that best match the deviceID only from native layer.
			 * Usually the one that is in the same app environment is picked up.
			 * If there is no matching in the app environment,
			 * then the areas are searched for a matchint deviceID.
			 *
			 * @param deviceID      The portalID that identifies an active portal.
			 * @return DeviceInstance-object
			 */
			sp ( EPSPACE DeviceInstance ) GetDeviceBestMatchNative ( int deviceID );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceBestMatchNativeRetained ( int deviceID );
#endif


			/**
			 * Get a collection that holds the nearby devices. This list ist updated dynamically by Environs.
			 *
			 * @return ArrayList with DeviceInstance objects
			 */
			c_const devList ( EPSPACE DeviceInstance ) c_ref GetDevicesNearby ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::ArrayList * GetDevicesNearbyRetained ();
#endif


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
			sp ( EPSPACE DeviceInstance ) GetDeviceNearby ( int nativeID );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceNearbyRetained ( int nativeID );
#endif


			/**
			 * Release the ArrayList that holds the nearby devices.
			 */
			ENVIRONS_LIB_API void ReleaseDevicesNearby ();


			/**
			 * Get a collection that holds the Mediator server devices. This list ist updated dynamically by Environs.
			 *
			 * @return ArrayList with DeviceInstance objects
			 */
			c_const devList ( EPSPACE DeviceInstance ) c_ref GetDevicesFromMediator ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::ArrayList * GetDevicesFromMediatorRetained ();
#endif


			/**
			 * Query a DeviceInstance object of Mediator managed devices within the environment.
			 *
			 * @param nativeID      The device id of the target device.
			 * @return DeviceInstance-object
			 */
			sp ( EPSPACE DeviceInstance ) GetDeviceFromMediator ( int nativeID );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::DeviceInstance * GetDeviceFromMediatorRetained ( int nativeID );
#endif


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
			 * Reload all device lists. Applications may call this if they manually stopped and started Environs again.
			 * Environs does not automatically refresh the device lists so as to allow applications to add observers before refreshing of the lists.
			 */
			ENVIRONS_LIB_API void ReloadLists ();

			/**
			* Reload the device list.
			*/
			ENVIRONS_LIB_API void Reload ();


			/**
			* Release ownership on this interface and mark it disposeable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
			 */
			ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

		INTERNAL:

			ENVIRONS_OUTPUT_ALLOC_RESOURCE ( DeviceList );

			int                             hEnvirons;
            bool                            isUIAdapter;

#ifndef CLI_CPP
			Instance           *			env;
#endif
			environs::lib::Environs OBJ_ptr envObj;

			static bool                     globalsInit INIT_to_false_in_cli;
			bool                            disposed_;
			int                             listType;

			pthread_mutex_t     OBJ_ptr		listDevicesLock;

			devList ( EPSPACE DeviceInstance )      listDevices;

#ifndef CLI_CPP
			spv ( lib::IIListObserver * )   listDevicesObservers;
#endif            

			//void                            PlatformDispose ();

			c_const devList ( EPSPACE DeviceInstance ) c_ref GetDevices ( int type );

			static sp(EPSPACE DeviceInstance) GetDevice(c_const devList(EPSPACE DeviceInstance) c_ref deviceList, pthread_mutex_t OBJ_ptr lock, int deviceID, CString_ptr areaName, CString_ptr appName, int * pos);

			static sp(EPSPACE DeviceInstance) GetDevice(c_const devList(EPSPACE DeviceInstance) c_ref deviceList, pthread_mutex_t OBJ_ptr lock, int nativeID, int * pos);

			sp ( EPSPACE DeviceInstance ) GetDevice ( int nativeOrDeviceID, bool isNativeID );

			sp ( EPSPACE DeviceInstance ) GetDeviceAll ( int nativeOrDeviceID, bool isNativeID );

			static sp ( EPSPACE DeviceInstance ) GetDeviceSeeker ( c_const devList ( EPSPACE DeviceInstance ) c_ref list, pthread_mutex_t OBJ_ptr lock, int nativeOrDeviceID, bool isNativeID );


			/**
			 * Release the ArrayList that holds the available devices.
			 */
			void ReleaseDevices ();

			void DisposeLists ();

			static void DisposeList ( bool isUIAdapter, c_const devList ( EPSPACE DeviceInstance ) c_ref list, pthread_mutex_t OBJ_ptr lock );

			static void DisposeListDo ( c_const devList ( EPSPACE DeviceInstance ) c_ref list );

			static void DeviceListUpdater ( environs::lib::Environs OBJ_ptr api, int listType );

			static bool DeviceListUpdaterDo ( Environs OBJ_ptr api, int listType, devListRef ( EPSPACE DeviceInstance ) deviceList,
				DeviceListItems devices, int devicesCount,
				NLayerVecTypeObj ( EPSPACE DeviceInstance ) OBJ_ptr vanished, NLayerVecTypeObj ( EPSPACE DeviceInstance ) OBJ_ptr appeared,
				NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr updates );

			static bool DeviceListUpdateDispatchSync ( c_const sp ( DeviceListUpdatePack ) c_ref updatePacks );

			static bool DeviceListUpdateDataSourceSync ( c_const sp ( DeviceListUpdatePack ) c_ref updatePacks );

			static void TakeOverToOtherLists ( Environs OBJ_ptr api, int listType, NLayerVecTypeObj ( EPSPACE DeviceInstance ) OBJ_ptr vanished );

			static void TakeOverToList ( environs::lib::Environs OBJ_ptr api, c_const devList ( EPSPACE DeviceInstance ) c_ref list, bool getMediator,
				NLayerVecTypeObj ( EPSPACE DeviceInstance ) OBJ_ptr vanished );

			static void c_OBJ_ptr DeviceListCommandThread ( pthread_param_t pack );

			static bool DeviceInstanceUpdatesApply ( NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr updates );


			static void OnDeviceListNotification ( int hInst, environs::ObserverNotifyContext OBJ_ptr ctx );

			static sp ( EPSPACE DeviceInstance ) RemoveDevice ( c_const devList ( EPSPACE DeviceInstance ) c_ref list, pthread_mutex_t OBJ_ptr lock, DeviceCommandContext OBJ_ptr pack );

			static void RemoveDevice ( DeviceCommandContext OBJ_ptr pack );

            static void UpdateDevice ( DeviceCommandContext OBJ_ptr pack );
        
            static void InsertDevice ( int hInst, c_const devList ( EPSPACE DeviceInstance ) c_ref deviceList, pthread_mutex_t OBJ_ptr listLock,
                                  sp ( EPSPACE DeviceInstance ) c_ref deviceNew,
                                  c_const spv ( lib::IIListObserver OBJ_ptr ) c_ref observerList );
        
            static bool InsertDeviceDo ( c_const devList ( EPSPACE DeviceInstance ) c_ref deviceList, sp ( EPSPACE DeviceInstance ) c_ref deviceNew, 
				NLayerListTypeObj ( DeviceInstanceUpdateContext ) OBJ_ptr updates );

			static void EnqueueCommand ( DeviceCommandContext OBJ_ptr ctx );

			static void NotifyListObservers ( int hInst, c_const spv ( lib::IIListObserver OBJ_ptr ) c_ref observerList, NLayerVecType ( EPSPACE DeviceInstance ) vanished, NLayerVecType ( EPSPACE DeviceInstance ) appeared, bool enqueue );




			static void UpdateConnectProgress ( pthread_mutex_t OBJ_ptr lock, c_const devList ( EPSPACE DeviceInstance ) c_ref list, int nativeID, int progress );

			static void UpdateMessage ( pthread_mutex_t OBJ_ptr lock, c_const devList ( EPSPACE DeviceInstance ) c_ref list, environs::ObserverMessageContext OBJ_ptr ctx );

			static void UpdateData ( pthread_mutex_t OBJ_ptr lock, c_const devList ( EPSPACE DeviceInstance ) c_ref list, environs::ObserverDataContext OBJ_ptr ctx );

			static void UpdateSensorData ( pthread_mutex_t OBJ_ptr lock, c_const devList ( EPSPACE DeviceInstance ) c_ref list, int nativeID, environs::SensorFrame OBJ_ptr pack );

		};
	}
}


#endif	/// INCLUDE_HCM_ENVIRONS_DEVICELISTS_H


