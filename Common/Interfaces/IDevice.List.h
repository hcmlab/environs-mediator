/**
 * DeviceLists Interface
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICELISTS_INTERFACE_H
#define INCLUDE_HCM_ENVIRONS_DEVICELISTS_INTERFACE_H

#include "Interfaces/IArray.List.h"
#include "Interfaces/IDevice.Instance.List.h"


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
        //class IDeviceInstance;
        //class IDeviceInstanceList;

		class IArrayList;
		class ListObserver;
		class IIListObserver;
        class IIDeviceObserver;
        
        class IDeviceList : public IEnvironsDispose
        {
        public:
            IDeviceList () {};
            virtual ~IDeviceList () {};
            
            
            /**
             * Release ownership on this interface and mark it disposeable.
             * Release must be called once for each Interface that the Environs framework returns to client code.
             * Environs will dispose the underlying object if no more ownership is hold by anyone.
             *
             */
            virtual void                    Release () = 0;
            
            virtual bool					disposed () = 0;
            
            virtual void					SetListType ( int MEDIATOR_DEVICE_CLASS_ ) = 0;
            
            virtual void					AddObserver ( ListObserver * observer ) = 0;
            virtual void					RemoveObserver ( ListObserver * observer ) = 0;
            
            
            sp ( IDeviceInstance )  GetItem ( int pos ) {
                ENVIRONS_IR_SP1_RETURN ( IDeviceInstance, GetItemRetainedI ( pos ) );
            }
            
            
            virtual int						GetCount () = 0;
            
            virtual IDeviceInstance *       GetItemRetainedI ( int pos ) = 0;
            
            virtual IDeviceInstance *       RefreshItemRetainedI ( IDeviceInstance * source, DeviceObserver * observer ) = 0;
            
            /**
             * Get a collection that holds all available devices. This list ist updated dynamically by Environs.
             *
             * @return ArrayList with DeviceInstance objects
             */
            sp ( IDeviceInstanceList )  GetDevices () {
                ENVIRONS_IR_SP1_RETURN ( IDeviceInstanceList, GetDevicesRetainedI () );
            }
            
			virtual IArrayList * GetDevicesRetainedI () = 0;
            
            
            /**
             * Query the number of all available devices within the environment (including those of the Mediator)
             *
             * @return numberOfDevices
             */
            virtual int GetDevicesCount () = 0;
            
            
            /**
             * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
             *
             * @param deviceID      The device id of the target device.
             * @param areaName      Area name of the application environment
             * @param appName		Application name of the application environment
             * @return DeviceInstance-object
             */
            sp ( IDeviceInstance )  GetDevice ( int deviceID, const char * areaName, const char * appName, int * pos ) {
                ENVIRONS_IR_SP1_RETURN ( IDeviceInstance, GetDeviceRetainedI ( deviceID, areaName, appName, pos ) );
            }
            
            virtual IDeviceInstance * GetDeviceRetainedI ( int deviceID, const char * areaName, const char * appName, int * pos ) = 0;
            
            
            /**
             * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator)
             *
             * @param nativeID      The device id of the target device.
             * @return DeviceInstance-object
             */
            sp ( IDeviceInstance )  GetDevice ( int nativeID ) {
                ENVIRONS_IR_SP1_RETURN ( IDeviceInstance, GetDeviceRetainedI ( nativeID ) );
            }
            
            virtual IDeviceInstance * GetDeviceRetainedI ( int nativeID ) = 0;
            
            /**
             * Query a DeviceInstance object that best match the deviceID only.
             * Usually the one that is in the same app environment is picked up.
             * If there is no matching in the app environment,
             * then the areas are searched for a matchint deviceID.
             *
             * @param deviceID      The portalID that identifies an active portal.
             * @return DeviceInstance-object
             */
            sp ( IDeviceInstance )  GetDeviceBestMatch ( int deviceID ) {
                ENVIRONS_IR_SP1_RETURN ( IDeviceInstance, GetDeviceBestMatchRetainedI ( deviceID ) );
            }
            
            virtual IDeviceInstance * GetDeviceBestMatchRetainedI ( int deviceID ) = 0;
            
            /**
             * Query a DeviceInstance object that best match the deviceID only from native layer.
             * Usually the one that is in the same app environment is picked up.
             * If there is no matching in the app environment,
             * then the areas are searched for a matchint deviceID.
             *
             * @param deviceID      The portalID that identifies an active portal.
             * @return DeviceInstance-object
             */
            sp ( IDeviceInstance )  GetDeviceBestMatchNative ( int deviceID ) {
                ENVIRONS_IR_SP1_RETURN ( IDeviceInstance, GetDeviceBestMatchNativeRetainedI ( deviceID ) );
            }
            
            virtual IDeviceInstance * GetDeviceBestMatchNativeRetainedI ( int deviceID ) = 0;
            
            
            /**
             * Get a collection that holds the nearby devices. This list ist updated dynamically by Environs.
             *
             * @return ArrayList with DeviceInstance objects
             */
            sp ( IDeviceInstanceList )  GetDevicesNearby () {
                ENVIRONS_IR_SP1_RETURN ( IDeviceInstanceList, GetDevicesNearbyRetainedI () );
            }
            
			virtual IArrayList * GetDevicesNearbyRetainedI () = 0;
            
            
            /**
             * Query the number of nearby (broadcast visible) devices within the environment.
             *
             * @return numberOfDevices
             */
            virtual int GetDevicesNearbyCount () = 0;
            
            /**
             * Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
             *
             * @param nativeID      The device id of the target device.
             * @return DeviceInstance-object
             */
            sp ( IDeviceInstance )  GetDeviceNearby ( int nativeID ) {
                ENVIRONS_IR_SP1_RETURN ( IDeviceInstance, GetDeviceNearbyRetainedI ( nativeID ) );
            }
            
            virtual IDeviceInstance * GetDeviceNearbyRetainedI ( int nativeID ) = 0;
            
            /**
             * Release the ArrayList that holds the nearby devices.
             */
            virtual void ReleaseDevicesNearby () = 0;
            
            /**
             * Get a collection that holds the Mediator server devices. This list ist updated dynamically by Environs.
             *
             * @return ArrayList with DeviceInstance objects
             */
            sp ( IDeviceInstanceList )  GetDevicesFromMediator () {
                ENVIRONS_IR_SP1_RETURN ( IDeviceInstanceList, GetDevicesFromMediatorRetainedI () );
            }
            
			virtual IArrayList * GetDevicesFromMediatorRetainedI () = 0;
            
            
            /**
             * Query a DeviceInstance object of Mediator managed devices within the environment.
             *
             * @param nativeID      The device id of the target device.
             * @return DeviceInstance-object
             */
            sp ( IDeviceInstance )  GetDeviceFromMediator ( int nativeID ) {
                ENVIRONS_IR_SP1_RETURN ( IDeviceInstance, GetDeviceFromMediatorRetainedI ( nativeID ) );
            }
            
            virtual IDeviceInstance * GetDeviceFromMediatorRetainedI ( int nativeID ) = 0;
            
            /**
             * Query the number of Mediator managed devices within the environment.
             *
             * @return numberOfDevices
             */
            virtual int GetDevicesFromMediatorCount () = 0;
            
            /**
             * Release the ArrayList that holds the Mediator server devices.
             */
            virtual void ReleaseDevicesMediator () = 0;
            
            /**
             * Refresh all device lists. Applications may call this if they manually stopped and started Environs again.
             * Environs does not automatically refresh the device lists so as to allow applications to add observers before refreshing of the lists.
             */
            virtual void RefreshDeviceLists () = 0;
        };
    }
}


#endif	/// INCLUDE_HCM_ENVIRONS_DEVICELISTS_INTERFACE_H

