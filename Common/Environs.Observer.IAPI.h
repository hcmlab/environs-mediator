/**
 * EnvironsObserver Internal API
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
#ifndef INCLUDE_HCM_ENVIRONS_OBSERVERS_INTERNAL_API_H
#define INCLUDE_HCM_ENVIRONS_OBSERVERS_INTERNAL_API_H

#include "Environs.Loader.h"
#include "Environs.Types.h"
#include "Environs.Msg.Types.h"
#include "Notify.Context.h"

#include "Interfaces/IDevice.List.h"
#include "Interfaces/IDevice.Instance.h"
#include "Interfaces/IDevice.Instance.List.h"
#include "File.Instance.h"
#include "Message.Instance.h"
#include "Interfaces/IPortal.Instance.h"


namespace environs
{
    namespace lib
    {
		class Environs;
		class PortalInstance;
		class DeviceList;

        /**
         * IIEnvironsObserver: Attachable to **IEnvirons** objects in order to receive all notifications that the Environs instance processes or submits to the platform layer.
         */
		class IIEnvironsObserver
        {
            friend class Environs;
            friend class PortalInstance;
            
        public:
            /** Constructor */
			IIEnvironsObserver () : OnEnvironsStatus_ ( true ), OnEnvironsNotify_ ( true ), OnEnvironsNotifyExt_ ( true ), OnEnvironsPortalRequestOrProvided_ ( true ), OnEnvironsPortalRequestOrProvidedInterface_ ( true ) {};
            
			virtual ~IIEnvironsObserver () {};

			/**
			* OnStatus is called whenever the framework status changes.&nbsp;
			*
			* @param Environs_STATUS_      A status constant of type STATUS_*
			*/
			virtual void OnStatus ( int Environs_STATUS_ ) = 0;


			/**
			* OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
			* The notification parameter is an integer value which represents one of the values as listed in Types.*
			* The string representation can be retrieved through TypesResolver.get(notification).
             *
             * @param context		An object reference of type environs::ObserverNotifyContext.
			*/
			virtual void OnNotify ( environs::ObserverNotifyContext * context ) = 0;


			/**
			* OnNotifyExt is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
			* The notification parameter is an integer value which represents one of the values as listed in Types.*
			* The string representation can be retrieved through TypesResolver.get(notification).
             *
             * @param context		An object reference of type environs::ObserverNotifyContext.
			*/
			virtual void OnNotifyExt ( environs::ObserverNotifyContext * context ) = 0;
            
            
            /**
             * OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
             *
             * @param portal 		The PortalInstance object.
             */
            virtual void OnPortalRequestOrProvided ( sp ( environs::PortalInstance ) portal ) = 0;
            
            
            /**
             * OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
             *
             * @param portal 		The PortalInstance object.
             */
            virtual void OnPortalRequestOrProvidedInterface ( environs::PortalInstance * portal ) = 0;
            
            /**
             * OnPortalRequestOrProvidedBase is called when a portal request from another devices came in, or when a portal has been provided by another device.
             *
             * @param portal 		The PortalInstance object.
             */
            virtual void OnPortalRequestOrProvidedBase ( environs::PortalInstance * portal ) { ENVIRONS_I_SP1 ( environs::PortalInstance, OnPortalRequestOrProvided, portal ); };
            
            
        protected:
            bool OnEnvironsStatus_;
            bool OnEnvironsNotify_;
            bool OnEnvironsNotifyExt_;
            bool OnEnvironsPortalRequestOrProvided_;
            bool OnEnvironsPortalRequestOrProvidedInterface_;
		};


		/**
		* IIListObserver: Attachable to **IDeviceList** objects in order to receive list changes of a particular IDeviceList.
		*/
		class IIListObserver
		{
			friend class DeviceList;

		public:
			/** Constructor */
			IIListObserver () : OnListChanged_ ( true ), OnListChangedInterface_ ( true ), OnListChangedInternal_ ( false ) {};

			virtual ~IIListObserver () {};

			/**
			* OnListChanged is called whenever the connected DeviceList has changed, e.g. new devices appeared or devices vanished from the list.
			*
			* @param vanished     A collection containing the devices vansihed and removed from the list. This argument can be null.
			* @param appeared     A collection containing the devices appeared and added to the list. This argument can be null.
			*/
            virtual void OnListChanged ( sp ( DeviceInstanceList ) vanished, sp ( DeviceInstanceList ) appeared ) = 0;
            
            /**
             * OnListChanged is called whenever the connected DeviceList has changed, e.g. new devices appeared or devices vanished from the list.
             *
             * @param vanished     A collection containing the devices vansihed and removed from the list. This argument can be null.
             * @param appeared     A collection containing the devices appeared and added to the list. This argument can be null.
             */
            virtual void OnListChangedInterface ( DeviceInstanceList * vanished, DeviceInstanceList * appeared ) = 0;

			virtual void OnListChangedBase ( ArrayList * vanished, ArrayList * appeared ) { ENVIRONS_I_SP2_SEL ( DeviceInstanceList, OnListChanged, vanished, appeared ); }

			virtual void OnListChangedInternal ( svsp ( DeviceInstance ) vanished, svsp ( DeviceInstance ) appeared ) { OnListChangedInternal_ = false; };

		protected:
            bool OnListChanged_;
            bool OnListChangedInterface_;
			bool OnListChangedInternal_;
		};


		/**
		* IIDeviceObserver: Attachable to **IDeviceInstance** objects in order to receive changes of a particular device.
		*/
		class IIDeviceObserver
		{
			friend class DeviceInstance;

		public:
			/** Constructor */
			IIDeviceObserver () : OnDeviceChanged_ ( true ), OnDeviceChangedInterface_ ( true ), OnDeviceChangedInternal_ ( false ) {};

			virtual ~IIDeviceObserver () {};


			/**
			* OnDeviceChanged is called whenever the members of a DeviceInstance has changed.&nbsp;
			* The DEVICE_INFO_ATTR_changed parameter provides a bit set which indicates the member that has changed.
			*
			* @param device                    The DeviceInstance object that sends this notification.
			* @param DEVICE_INFO_ATTR_changed  The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
			*/
            virtual void OnDeviceChanged ( sp ( environs::DeviceInstance ) device, int DEVICE_INFO_ATTR_changed ) = 0;
            
            
            /**
             * OnDeviceChanged is called whenever the members of a DeviceInstance has changed.&nbsp;
             * The DEVICE_INFO_ATTR_changed parameter provides a bit set which indicates the member that has changed.
             *
             * @param device                    The DeviceInstance object that sends this notification.
             * @param DEVICE_INFO_ATTR_changed  The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
             */
            virtual void OnDeviceChangedInterface ( environs::DeviceInstance * device, int DEVICE_INFO_ATTR_changed ) = 0;
            
            virtual void OnDeviceChangedBase ( environs::DeviceInstance * device, int DEVICE_INFO_ATTR_changed ) { ENVIRONS_I_SP1_1 ( environs::DeviceInstance, OnDeviceChanged, device, DEVICE_INFO_ATTR_changed ); }

			virtual void OnDeviceChangedInternal ( int DEVICE_INFO_ATTR_changed ) { OnDeviceChangedInternal_ = false; };

		protected:
            bool OnDeviceChanged_;
            bool OnDeviceChangedInterface_;
			bool OnDeviceChangedInternal_;
		};
        
        
        /**
         * IIEnvironsMessageObserver: Attachable to **IEnvirons** objects in order to receive all messages that the Environs instance received.
         */
		class IIEnvironsMessageObserver
        {
            friend class Environs;
            
        public:
            /** Constructor */
			IIEnvironsMessageObserver () : OnEnvironsMessage_ ( true ), OnEnvironsMessageExt_ ( true ), OnEnvironsStatusMessage_ ( true ) {};
            
			virtual ~IIEnvironsMessageObserver () {};
            
            /**
             * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
             *
			 * @param context		An object reference of type environs::ObserverMessageContext.
             */
			virtual void OnMessage ( environs::ObserverMessageContext * context ) = 0;
            
            
            /**
             * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
             *
			 * @param context		An object reference of type environs::ObserverMessageContext.
             */
			virtual void OnMessageExt ( environs::ObserverMessageContext * context ) = 0;
            
            
            /**
             * OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
             *
             * @param message      The status as a text message.
             */
            virtual void OnStatusMessage ( const char * message ) = 0;
            
        protected:
            bool OnEnvironsMessage_;
            bool OnEnvironsMessageExt_;
            bool OnEnvironsStatusMessage_;
		};


		/**
		* IIMessageObserver: Attachable to **IDeviceInstance** objects in order to receive messages of a particular device.
		*/
		class IIMessageObserver
		{
			friend class DeviceInstance;

		public:
			/** Constructor */
			IIMessageObserver () : OnMessage_ ( true ), OnMessageInterface_ ( true ), OnMessageInternal_ ( false ) {};

			virtual ~IIMessageObserver () {};

			/**
			* OnMessage is called whenever a text message has been received from a device.
			*
			* @param msg   The corresponding message object of type MessageInstance
			*/
            virtual void OnMessage ( sp ( environs::MessageInstance ) msg, int MESSAGE_INFO_ATTR_changed ) = 0;
            
            /**
             * OnMessage is called whenever a text message has been received from a device.
             *
             * @param msg   The corresponding message object of type MessageInstance
             */
            virtual void OnMessageInterface ( environs::MessageInstance * msg, int MESSAGE_INFO_ATTR_changed ) = 0;

			/**
			* OnMessage is called whenever a text message has been received from a device.
			*
			* @param msg   The corresponding message object of type MessageInstance
			*/
			virtual void OnMessageBase ( environs::MessageInstance * msg, int MESSAGE_INFO_ATTR_changed ) { ENVIRONS_I_SP2 ( environs::MessageInstance, OnMessage, msg, MESSAGE_INFO_ATTR_changed ); };

			virtual void OnMessageInternal ( sp ( MessageInstance ) msg, int MESSAGE_INFO_ATTR_changed ) { OnMessageInternal_ = false; };

		protected:
			bool OnMessage_;
            bool OnMessageInterface_;
			bool OnMessageInternal_;
		};
        
        
        /**
         * IIEnvironsDataObserver: Attachable to **IEnvirons** objects in order to receive all data transmissions that the Environs instance received.
         */
		class IIEnvironsDataObserver
        {
        public:
            /** Constructor */
			IIEnvironsDataObserver () : OnEnvironsData_ ( true ) {};
            
			virtual ~IIEnvironsDataObserver () {};

			/**
			* OnData is called whenever new binary data (files, buffers) has been received.
			* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
			*
			* @param context		An object reference of type environs::ObserverDataContext.
			*/
			virtual void OnData ( environs::ObserverDataContext * context ) = 0;
            
        protected:
            bool OnEnvironsData_;
		};


		/**
		* IIDataObserver: Attachable to **IDeviceInstance** objects in order to receive data transmissions of a particular device.
		*/
		class IIDataObserver
		{
			friend class DeviceInstance;

		public:
			/** Constructor */
			IIDataObserver () : OnData_ ( true ), OnDataInterface_ ( true ), OnDataInternal_ ( false ) {};

			virtual ~IIDataObserver () {};
            
            /**
             * OnData is called whenever new binary data (files, buffers) has been received.
             * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
             *
             * @param fileData                  The corresponding file object of type FileInstance
             * @param FILE_INFO_ATTR_changed    Flags that indicate the object change.
             */
            virtual void OnData ( sp ( environs::FileInstance ) fileData, int FILE_INFO_ATTR_changed ) = 0;
            
            /**
             * OnData is called whenever new binary data (files, buffers) has been received.
             * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
             *
             * @param fileData                  The corresponding file object of type FileInstance
             * @param FILE_INFO_ATTR_changed    Flags that indicate the object change.
             */
            virtual void OnDataInterface ( environs::FileInstance * fileData, int FILE_INFO_ATTR_changed ) = 0;
            
            /**
             * OnDataBase is called whenever new binary data (files, buffers) has been received.
             * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
             *
             * @param fileData                  The corresponding file object of type FileInstance
             * @param FILE_INFO_ATTR_changed    Flags that indicate the object change.
             */
			virtual void OnDataBase ( environs::FileInstance * fileData, int FILE_INFO_ATTR_changed )  { ENVIRONS_I_SP2 ( environs::FileInstance, OnData, fileData, FILE_INFO_ATTR_changed ); };

			virtual void OnDataInternal ( sp ( FileInstance ) fileData, int FILE_INFO_ATTR_changed ) { OnDataInternal_ = false; };

		protected:
            bool OnData_;
            bool OnDataInterface_;
			bool OnDataInternal_;
		};
        
        
        /**
         * IIEnvironsSensorDataObserver: Attachable to **IEnvirons** objects in order to receive all sensor data that the Environs instance received.
         */
		class IIEnvironsSensorDataObserver
        {
        public:
            /** Constructor */
			IIEnvironsSensorDataObserver () : OnSensorData_ ( true ) {};
            
			virtual ~IIEnvironsSensorDataObserver () {};
            
            /**
             * OnSensorData is called whenever new binary data (files, buffers) has been received.
             * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
             *
             * @param nativeID      The native identifier that targets the device.
             * @param pack          The frame containing the sensor data
             */
            virtual void OnSensorData ( OBJIDType nativeID, environs::SensorFrame * pack ) = 0;
            
        protected:
            bool OnSensorData_;
		};


		/**
		* IISensorObserver: Attachable to **IDeviceInstance** objects in order to receive sensor data from a particular device.
		*/
		class IISensorObserver
		{
		public:
			/** Constructor */
			IISensorObserver () : OnSensorData_ ( true ) {};

			virtual ~IISensorObserver () {};

			/**
			* OnSensorData is called whenever new sensor data has been received.
			*
			* @param pack     The corresponding SensorFrame of sensor data
			*/
			virtual void OnSensorData ( environs::SensorFrame * pack ) = 0;

		protected:
			bool OnSensorData_;
		};
        
        
        /**
         * IIPortalObserver: Attachable to **PortalInstance** objects in order to receive changes of a particular interactive portal.
         */
		class IIPortalObserver
        {
            friend class PortalInstance;
            
        public:
            /** Constructor */
			IIPortalObserver () : OnPortalChanged_ ( true ), OnPortalChangedInterface_ ( true ) {};
            
			virtual ~IIPortalObserver () {};
            
            /**
             * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
             *
             * @param portal                    The PortalInstance object.
             * @param Environs_NOTIFY_PORTAL_	The notification (Environs.NOTIFY_PORTAL_*) that indicates the change.
             */
            virtual void OnPortalChanged ( sp ( environs::PortalInstance ) portal, int Environs_NOTIFY_PORTAL_ ) = 0;
            
            /**
             * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
             *
             * @param portal                    The PortalInstance object.
             * @param Environs_NOTIFY_PORTAL_	The notification (Environs.NOTIFY_PORTAL_*) that indicates the change.
             */
            virtual void OnPortalChangedInterface ( environs::PortalInstance * portal, int Environs_NOTIFY_PORTAL_ ) = 0;
            
            
            /**
             * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
             *
             * @param portal                    The PortalInstance object.
             * @param Environs_NOTIFY_PORTAL_	The notification (Environs.NOTIFY_PORTAL_*) that indicates the change.
             */
            virtual void OnPortalChangedBase ( environs::PortalInstance * portal, int Environs_NOTIFY_PORTAL_ ) { ENVIRONS_I_SP1_1 ( environs::PortalInstance, OnPortalChanged, portal, Environs_NOTIFY_PORTAL_ ); };
            
        protected:
            bool OnPortalChanged_;
            bool OnPortalChangedInterface_;
        };
    }
}

#endif

