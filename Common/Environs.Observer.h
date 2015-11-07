/**
 * EnvironsObserver
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
#ifndef INCLUDE_HCM_ENVIRONS_OBSERVERS_H
#define INCLUDE_HCM_ENVIRONS_OBSERVERS_H

#include "Environs.Observer.IAPI.h"


namespace environs
{
    namespace lib
    {
        /**
         * EnvironsObserver: Attachable to **IEnvirons** objects in order to receive all notifications that the Environs instance processes or submits to the platform layer.
         */
		class EnvironsObserver : IIEnvironsObserver
        {            
        public:
            /** Constructor */
            EnvironsObserver () {};            
            virtual ~EnvironsObserver ( ) {};
            
            /**
             * OnStatus is called whenever the framework status changes.&nbsp;
             *
             * @param Environs_STATUS_      A status constant of type STATUS_*
             */
            virtual void OnStatus ( int Environs_STATUS_ ) { OnEnvironsStatus_ = false; };
            
            
            /**
             * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
             * The notification parameter is an integer value which represents one of the values as listed in Types.*
             * The string representation can be retrieved through TypesResolver.get(notification).
             *
             * @param nativeID      The native identifier that targets the device.
             * @param notification  The notification
             * @param source	    A value of the enumeration Types.EnvironsSource
             * @param context       A value that provides additional context information (if available).
             */
            virtual void OnNotify ( int nativeID, int notification, int source, void * context ) { OnEnvironsNotify_ = false; };
            
            
            /**
             * OnNotifyExt is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
             * The notification parameter is an integer value which represents one of the values as listed in Types.*
             * The string representation can be retrieved through TypesResolver.get(notification).
             *
             * @param deviceID      The device id of the sender device.
             * @param areaName      Area name of the application environment
             * @param appName		Application name of the application environment
             * @param notification  The notification
             * @param source		A value of the enumeration Types.EnvironsSource
             * @param context       A value that provides additional context information (if available).
             */
            virtual void OnNotifyExt ( int deviceID, const char * areaName, const char * appName, int notification, int source, void * context ) { OnEnvironsNotifyExt_ = false; };
            
            
            /**
             * OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
             *
             * @param portal 		The PortalInstance object.
             */
            virtual void OnPortalRequestOrProvided ( sp ( IPortalInstance ) portal ) { OnEnvironsPortalRequestOrProvided_ = false; };
            
            
            /**
             * OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
             *
             * @param portal 		The PortalInstance object.
             */
            virtual void OnPortalRequestOrProvidedInterface ( IPortalInstance * portal ) { OnEnvironsPortalRequestOrProvidedInterface_ = false; };
		};


		/**
		* ListObserver: Attachable to **IDeviceList** objects in order to receive list changes of a particular IDeviceList.
		*/
		class ListObserver : IIListObserver
		{
		public:
			/** Constructor */
			ListObserver () {};
			virtual ~ListObserver () {};

			/**
			* OnListChanged is called whenever the connected DeviceList has changed, e.g. new devices appeared or devices vanished from the list.
			*
			* @param vanished     A collection containing the devices vansihed and removed from the list. This argument can be null.
			* @param appeared     A collection containing the devices appeared and added to the list. This argument can be null.
			*/
            virtual void OnListChanged ( sp ( IDeviceInstanceList ) vanished, sp ( IDeviceInstanceList ) appeared ) { OnListChanged_ = false; };
            
            /**
             * OnListChanged is called whenever the connected DeviceList has changed, e.g. new devices appeared or devices vanished from the list.
             *
             * @param vanished     A collection containing the devices vansihed and removed from the list. This argument can be null.
             * @param appeared     A collection containing the devices appeared and added to the list. This argument can be null.
             */
            virtual void OnListChangedInterface ( IDeviceInstanceList * vanished, IDeviceInstanceList * appeared ) { OnListChangedInterface_ = false; };
		};


		/**
		* DeviceObserver: Attachable to **IDeviceInstance** objects in order to receive changes of a particular device.
		*/
		class DeviceObserver : IIDeviceObserver
		{
		public:
			/** Constructor */
			DeviceObserver () {};
			virtual ~DeviceObserver () {};

			/**
			* OnDeviceChanged is called whenever the members of a DeviceInstance has changed.&nbsp;
			* The DEVICE_INFO_ATTR_changed parameter provides a bit set which indicates the member that has changed.
			*
			* @param device                    The DeviceInstance object that sends this notification.
			* @param DEVICE_INFO_ATTR_changed  The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
			*/
            virtual void OnDeviceChanged ( sp ( IDeviceInstance ) device, int DEVICE_INFO_ATTR_changed ) { OnDeviceChanged_ = false; };
            
            
            /**
             * OnDeviceChanged is called whenever the members of a DeviceInstance has changed.&nbsp;
             * The DEVICE_INFO_ATTR_changed parameter provides a bit set which indicates the member that has changed.
             *
             * @param device                    The DeviceInstance object that sends this notification.
             * @param DEVICE_INFO_ATTR_changed  The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
             */
            virtual void OnDeviceChangedInterface ( IDeviceInstance * device, int DEVICE_INFO_ATTR_changed ) { OnDeviceChangedInterface_ = false; };
		};
        
        
        /**
         * EnvironsMessageObserver: Attachable to **IEnvirons** objects in order to receive all messages that the Environs instance received.
         */
		class EnvironsMessageObserver : IIEnvironsMessageObserver
        {            
        public:
            /** Constructor */
            EnvironsMessageObserver () {};            
            virtual ~EnvironsMessageObserver ( ) {};
            
            /**
             * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
             *
             * @param nativeID      The native identifier that targets the device.
             * @param type	        Determines the source (either from a device, environs, or native layer)
             * @param message       The message as string text
             * @param length        The length of the message
             */
			virtual void OnMessage ( int nativeID, int type, const char * message, int length ) { OnEnvironsMessage_ = false; };
            
            
            /**
             * OnMessageExt is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
             *
             * @param deviceID      The device id of the sender device.
             * @param areaName      Area name of the application environment
             * @param appName		Application name of the application environment
             * @param type	        Determines the source (either from a device, environs, or native layer)
             * @param message       The message as string text
             * @param length        The length of the message
             */
			virtual void OnMessageExt ( int deviceID, const char * areaName, const char * appName, int type, const char * message, int length ) { OnEnvironsMessageExt_ = false; };
            
            
            /**
             * OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
             *
             * @param message      The status as a text message.
             */
            virtual void OnStatusMessage ( const char * message ) { OnEnvironsStatusMessage_ = false; };
		};


		/**
		* MessageObserver: Attachable to **IDeviceInstance** objects in order to receive messages of a particular device.
		*/
		class MessageObserver : IIMessageObserver
		{
		public:
			/** Constructor */
			MessageObserver () {};
			virtual ~MessageObserver () {};
            
            /**
             * OnMessage is called whenever a text message has been received from a device.
             *
             * @param msg                           The corresponding message object of type MessageInstance
             * @param MESSAGE_INFO_ATTR_changed     Flags that indicate the object change.
             */
            virtual void OnMessage ( sp ( IMessageInstance ) msg, int MESSAGE_INFO_ATTR_changed ) { OnMessage_ = false; };
            
            /**
             * OnMessage is called whenever a text message has been received from a device.
             *
             * @param msg                           The corresponding message object of type MessageInstance
             * @param MESSAGE_INFO_ATTR_changed     Flags that indicate the object change.
             */
            virtual void OnMessageInterface ( IMessageInstance * msg, int MESSAGE_INFO_ATTR_changed ) { OnMessageInterface_ = false; };
		};
        
        
        /**
         * EnvironsDataObserver: Attachable to **IEnvirons** objects in order to receive all data transmissions that the Environs instance received.
         */
		class EnvironsDataObserver : IIEnvironsDataObserver
        {
        public:
            /** Constructor */
            EnvironsDataObserver () {};            
            virtual ~EnvironsDataObserver ( ) {};
            
            /**
             * OnData is called whenever new binary data (files, buffers) has been received.
             * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
             *
             * @param nativeID      The native identifier that targets the device.
             * @param type          The type of the data
             * @param fileID        A user-customizable id that identifies the file
             * @param descriptor    A text that describes the file
             * @param size          The size in bytes
             */
            virtual void OnData ( int nativeID, int type, int fileID, const char * descriptor, int size ) { OnEnvironsData_ = false; };
		};


		/**
		* DataObserver: Attachable to **IDeviceInstance** objects in order to receive data transmissions of a particular device.
		*/
		class DataObserver : IIDataObserver
		{
		public:
			/** Constructor */
			DataObserver () {};
			virtual ~DataObserver () {};
            
            /**
             * OnData is called whenever new binary data (files, buffers) has been received.
             * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
             *
             * @param fileData                  The corresponding file object of type FileInstance
             * @param FILE_INFO_ATTR_changed    Flags that indicate the object change.
             */
            virtual void OnData ( sp ( IFileInstance ) fileData, int FILE_INFO_ATTR_changed ) { OnData_ = false; };
            
            /**
             * OnData is called whenever new binary data (files, buffers) has been received.
             * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
             *
             * @param fileData                  The corresponding file object of type FileInstance
             * @param FILE_INFO_ATTR_changed    Flags that indicate the object change.
             */
            virtual void OnDataInterface ( IFileInstance * fileData, int FILE_INFO_ATTR_changed ) { OnDataInterface_ = false; };
		};
        
        
        /**
         * EnvironsSensorDataObserver: Attachable to **IEnvirons** objects in order to receive all sensor data that the Environs instance received.
         */
		class EnvironsSensorDataObserver : IIEnvironsSensorDataObserver
        {
        public:
            /** Constructor */
            EnvironsSensorDataObserver () {};            
            virtual ~EnvironsSensorDataObserver ( ) {};
            
            /**
             * OnSensorData is called whenever new binary data (files, buffers) has been received.
             * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
             *
             * @param nativeID      The native identifier that targets the device.
             * @param pack          The frame containing the sensor data
             */
            virtual void OnSensorData ( int nativeID, environs::SensorFrame * pack ) { OnSensorData_ = false; };
		};


		/**
		* SensorObserver: Attachable to **IDeviceInstance** objects in order to receive sensor data from a particular device.
		*/
		class SensorObserver : IISensorObserver
		{
		public:
			/** Constructor */
			SensorObserver () {};
			virtual ~SensorObserver () {};

			/**
			* OnSensorData is called whenever new sensor data has been received.
			*
			* @param pack     The corresponding SensorFrame of sensor data
			*/
			virtual void OnSensorData ( environs::SensorFrame * pack ) { OnSensorData_ = false; };
		};
        
        
        /**
         * PortalObserver: Attachable to **IPortalInstance** objects in order to receive changes of a particular interactive portal.
         */
		class PortalObserver : IIPortalObserver
        {            
        public:
            /** Constructor */
            PortalObserver () {};            
            virtual ~PortalObserver ( ) {};
            
            /**
             * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
             *
             * @param portal                    The PortalInstance object.
             * @param Environs_NOTIFY_PORTAL_	The notification (Environs.NOTIFY_PORTAL_*) that indicates the change.
             */
            virtual void OnPortalChanged ( sp ( IPortalInstance ) portal, int Environs_NOTIFY_PORTAL_ ) { OnPortalChanged_ = false; };
            
            /**
             * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
             *
             * @param portal                    The PortalInstance object.
             * @param Environs_NOTIFY_PORTAL_	The notification (Environs.NOTIFY_PORTAL_*) that indicates the change.
             */
            virtual void OnPortalChangedInterface ( IPortalInstance * portal, int Environs_NOTIFY_PORTAL_ )  { OnPortalChangedInterface_ = false; };
        };
    }
}

#endif

