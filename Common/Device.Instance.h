/**
 * Device Instance Objects
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_H
#define INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_H

#ifdef __cplusplus

#include "Interop/Smart.Pointer.h"
#include "Interfaces/IDevice.Instance.h"
#include "Environs.Msg.Types.h"
#include <string>
#include <queue>

/* Namespace: environs -> */
namespace environs
{
    class Instance;
    
	namespace lib
    {
        class Environs;
		class MessageInstance;
		class FileInstance;
		class PortalInstance;
        
        
        class NotifierContext
        {
        public:
            int                     type;
            int                     flags;
            sp ( DeviceInstance )   device;
            sp ( MessageInstance )  message;
            sp ( FileInstance )     fileData;
        };

        
		class DeviceInstance : public IDeviceInstance
		{
            friend class Environs;
			friend class DeviceList;
			friend class PortalInstance;
            friend class MessageInstance;
            friend class FileInstance;

            friend class DeviceInstanceProxy;

        public:
				ENVIRONS_LIB_API    DeviceInstance ();
				ENVIRONS_LIB_API    ~DeviceInstance ();
        
        
			/**
			 * Add an observer (DeviceObserver) that notifies about device property changes.
			 *
			 * @param observer A DeviceObserver
			 */
			ENVIRONS_LIB_API void AddObserver ( DeviceObserver * observer );
        
			/**
			 * Remove an observer (DeviceObserver) that was added before.
			 *
			 * @param observer A DeviceObserver
			 */
			ENVIRONS_LIB_API void RemoveObserver ( DeviceObserver * observer );
        
        
			/**
			 * Add an observer (DataObserver) that notifies about data received or sent through the DeviceInstance.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void AddObserverForData ( DataObserver * observer );
        
        
			/**
			 * Remove an observer (SensorObserver) that was added before.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void RemoveObserverForData ( DataObserver * observer );
        
        
			/**
			 * Add an observer (SensorObserver) that notifies about data received or sent through the DeviceInstance.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void AddObserverForSensors ( SensorObserver * observer );
        
        
			/**
			 * Remove an observer (DataObserver) that was added before.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void RemoveObserverForSensors ( SensorObserver * observer );
        
        
			/**
			 * Add an observer (MessageObserver) that notifies about messages received or sent through the DeviceInstance.
			 *
			 * @param observer A MessageObserver
			 */
			ENVIRONS_LIB_API void AddObserverForMessages ( MessageObserver * observer );
        
        
			/**
			 * Remove an observer (MessageObserver) that was added before.
			 *
			 * @param observer A MessageObserver
			 */
			ENVIRONS_LIB_API void RemoveObserverForMessages ( MessageObserver * observer );
        
        
			/**
			 * Notify to all observers (DeviceObserver) that the appContext has changed.
			 *
			 * @param customFlags Either custom declared flags or 0. If 0 is provided, then the flag Environs.DEVICE_INFO_ATTR_APP_CONTEXT will be used.
			 */
			ENVIRONS_LIB_API void NotifyAppContextChanged ( int customFlags );
            
            
            /** The device properties structure into a DeviceInfo object. */
            environs::DeviceInfo * info ();
            
			ENVIRONS_LIB_API const char * ToString ();
        
			ENVIRONS_LIB_API const char * ips ();
            
			ENVIRONS_LIB_API const char * ipes ();
        
			ENVIRONS_LIB_API bool EqualsAppEnv ( environs::DeviceInfo * equalTo );
			ENVIRONS_LIB_API bool EqualsAppEnv ( const char * areaName, const char * appName );
        
			ENVIRONS_LIB_API bool LowerThanAppEnv ( environs::DeviceInfo * compareTo );
			ENVIRONS_LIB_API bool LowerThanAppEnv ( const char * areaName, const char * appName );
        
			ENVIRONS_LIB_API bool EqualsID ( IDeviceInstance * equalTo );
			ENVIRONS_LIB_API bool EqualsID ( int deviceID, const char * areaName, const char *appName );
        
        
			ENVIRONS_LIB_API const char * DeviceTypeString ( environs::DeviceInfo * info );
			ENVIRONS_LIB_API const char * DeviceTypeString ();
        
			ENVIRONS_LIB_API const char * GetBroadcastString ( bool fullText );

			ENVIRONS_LIB_API bool isConnected ();
            
            /**
             * disposed is true if the object is no longer valid. Nothing will be updated anymore.
             * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
             * */
			ENVIRONS_LIB_API bool disposed ();
            
            /** An identifier that is unique for this object. */
            ENVIRONS_LIB_API long objID () { return objID_; };

			/**
			* Connect to this device asynchronously.
			*
			* @param Environs_CALL_   A value of Environs_CALL_* that determines whether (only this call) is performed synchronous or asynchronous.
			*
			* @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
			* 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
			* 					true: A new connection has been triggered and is in progress
			*/
			ENVIRONS_LIB_API bool Connect ();

			/**
			* Connect to this device using the given mode.
			*
			* @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
			* 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
			* 					true: A new connection has been triggered and is in progress
			*/
			ENVIRONS_LIB_API bool Connect ( int Environs_CALL_ );

			/**
			* Disconnect the device with the given id and a particular application environment.
			*
			* @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
			*/
			ENVIRONS_LIB_API bool Disconnect ();

			/**
			* Disconnect the device using the given mode with the given id and a particular application environment.
			*
			* @param Environs_CALL_   A value of Environs_CALL_* that determines whether (only this call) is performed synchronous or asynchronous.
			*
			* @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
			*/
			ENVIRONS_LIB_API bool Disconnect ( int Environs_CALL_ );

			/**
			* Retrieve display properties and dimensions of this device. The device must be connected before this object is available.
			*
			* @return PortalInstance-object
			*/
			ENVIRONS_LIB_API environs::DeviceDisplay GetDisplayProps ();

			/**
			* Load the file that is assigned to the fileID received by deviceID into an byte array.
			*
			* @param fileID        The id of the file to load (given in the onData receiver).
			* @param size        An int pointer, that receives the size of the returned buffer.
			* @return byte-array
			*/
			ENVIRONS_LIB_API char * GetFile ( int fileID, int * size );

			/**
			* Query the absolute path for the local filesystem that is assigned to the fileID received by deviceID.
			*
			* @param fileID        The id of the file to load (given in the onData receiver).
			* @return absolutePath
			*/
			ENVIRONS_LIB_API const char * GetFilePath ( int fileID );


			/**
			* Creates a portal instance that requests a portal.
			*
			* @param 	portalType	        Project name of the application environment
			*
			* @return 	PortalInstance-object
			*/
			sp ( PortalInstance ) PortalRequestSP ( environs::PortalType_t portalType );
        
			ENVIRONS_LIB_API IPortalInstance * PortalRequestRetainedI ( environs::PortalType_t portalType );
        

			/**
			* Creates a portal instance that provides a portal.
			*
			* @param 	portalType	        Project name of the application environment
			*
			* @return 	PortalInstance-object
			*/
			sp ( PortalInstance ) PortalProvideSP ( environs::PortalType_t portalType );
        
			ENVIRONS_LIB_API IPortalInstance * PortalProvideRetainedI ( environs::PortalType_t portalType );


			/**
			* Query the first PortalInstance that manages an outgoing portal.
			*
			* @return PortalInstance-object
			*/
			sp ( PortalInstance ) PortalGetOutgoingSP ();
        
			ENVIRONS_LIB_API IPortalInstance * PortalGetOutgoingRetainedI ();
        

			/**
			* Query the first PortalInstance that manages an incoming portal.
			*
			* @return PortalInstance-object
			*/
			sp ( PortalInstance ) PortalGetIncomingSP ();
        
			ENVIRONS_LIB_API IPortalInstance * PortalGetIncomingRetainedI ();
        

			/**
			* Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
			*
			* @param fileID        A user-customizable id that identifies the file to be send.
			* @param fileDescriptor (e.g. filename)
			* @param filePath      The path to the file to be send.
			* @return success
			*/
			ENVIRONS_LIB_API bool SendFile ( int fileID, const TCHAR * fileDescriptor, const TCHAR * filePath );

			/**
			* Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
			*
			* @param fileID        A user-customizable id that identifies the file to be send.
			* @param fileDescriptor (e.g. filename)
			* @param buffer        A buffer to be send.
			* @param bytesToSend number of bytes in the buffer to send
			* @return success
			*/
			ENVIRONS_LIB_API bool SendBuffer ( int fileID, const char * fileDescriptor, char * buffer, int bytesToSend );
        
			/**
			 * Send a string message to a device through one of the following ways.&nbsp;
			 * If a connection with the destination device has been established, then use that connection.
			 * If the destination device is not already connected, then distinguish the following cases:
			 * (1) If the destination is within the same network, then try establishing a direct connection.
			 * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
			 * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
			 * 		a STUNT connection with the latest connection details that are available.
			 *
			 * On successful transmission, Environs returns true if the devices already had an active connection,
			 * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
			 * a registered EnvironsObserver instance.
			 *
			 * @param message       A message to send.
			 * @param length       Length of the message to send.
			 * @return success
			 */
			ENVIRONS_LIB_API bool SendMessage ( const char * msg, int length );
        
			/**
			 * Send a string message to a device through one of the following ways.&nbsp;
			 * If a connection with the destination device has been established, then use that connection.
			 * If the destination device is not already connected, then distinguish the following cases:
			 * (1) If the destination is within the same network, then try establishing a direct connection.
			 * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
			 * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
			 * 		a STUNT connection with the latest connection details that are available.
			 *
			 * On successful transmission, Environs returns true if the devices already had an active connection,
			 * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
			 * a registered EnvironsObserver instance.
			 *
			 * @param message       A message to be send.
			 * @return success
			 */
			ENVIRONS_LIB_API bool SendMessage ( const char * msg );


			/**
			* Clear (Delete permanently) all messages for this DeviceInstance in the persistent storage.
			*
			*/
			ENVIRONS_LIB_API void ClearMessages ();

			/**
			* Clear (Delete permanently) all files for this DeviceInstance in the persistent storage.
			*
			*/
			ENVIRONS_LIB_API void ClearStorage ();


			/**
			* Get a dictionary with all files that this device instance has received.
			*
			* @return Collection with objects of type FileInstance with the fileID as the key.
			*/
			smsp ( int, FileInstance ) GetAllFilesSP ();
        
			ENVIRONS_LIB_API IArrayList * GetAllIFiles ();


			/**
			* Get a list with all messages that this device has received (and sent).
			*
			* @return Collection with objects of type MessageInstance
			*/
			svsp ( MessageInstance ) GetAllMessagesSP ();
        
			ENVIRONS_LIB_API IArrayList * GetAllIMessages ();

        
			/**
			* Query the absolute path for the local filesystem to the persistent storage for this DeviceInstance.
			*
			* @return absolutePath
			*/
			ENVIRONS_LIB_API const char * GetStoragePath ();
        
			/**
			 * Enable sending of magnetic field events to this DeviceInstance.
			 * Events are send if the device is connected and stopped if the device is disconnected.
			 *
			 * @param ENVIRONS_SENSOR_TYPE_ A value of type ENVIRONS_SENSOR_TYPE_*.
			 * @param enable true = enable, false = disable.
			 *
			 * @return success true = enabled, false = failed.
			 */
			ENVIRONS_LIB_API bool SetSensorEventSending ( int ENVIRONS_SENSOR_TYPE_, bool enable );
            
            
			/**
			* Release ownership on this interface and mark it disposeable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
             */
            ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

		private:
			ENVIRONS_OUTPUT_ALLOC_RESOURCE ( DeviceInstance );
            
            static bool                 globalsInit;
            static bool                 GlobalsInit ();
            static void                 GlobalsDispose ();
        
            int                         hEnvirons;
            Instance                *   env;
            
			pthread_mutex_t				devicePortalsLock;
        
			/** A collection of PortalInstances that this device has established or is managing. */
			vsp ( PortalInstance )      devicePortals;
        
			/**
			 * disposed is true if DeviceInstance is no longer valid. Nothing will be updated anymore.
			 * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
			 * */
            bool						disposed_;
            
            void                        PlatformDispose ();
            
            /** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
            bool                        async;
            
            /** The device properties structure into a DeviceInfo object. */
            environs::DeviceInfo		info_;
        
			/** A collection of observers that observe this device instance for changes and events. */
			vct ( IIDeviceObserver * )  observers;
        
			/** A collection of observers that observe this device instance for changes and events. */
			vct ( IIMessageObserver * ) observersForMessages;
        
			/** A collection of observers that observe this device instance for changes and events. */
			vct ( IIDataObserver * )	observersForData;
        
			/** A collection of observers that observe this device instance for changes and events. */
			vct ( IISensorObserver * )  observersForSensorData;
            
            char					storageLoaded;
            
            bool                    directStatus;
            
			/** Determines whether the device shall receive accelerometer events from us. */
			bool                    enableAccelSender;
        
			/** Determines whether the device shall receive magnetic field events from us. */
			bool                    enableMagneticSender;
        
			/** Determines whether the device shall receive gyroscope events from us. */
			bool                    enableGyroscopeSender;
        
			/** Determines whether the device shall receive orientation events from us. */
            bool                    enableOrientationSender;
            
            std::string             toString_;
            std::string             ips_;
            std::string             ipes_;
            std::string             filePath;
            
			std::string				storagePath;
        
			svsp ( MessageInstance ) messages;
        
			smsp ( int, FileInstance ) files;
        
			pthread_mutex_t			storageLock;
        
			pthread_cond_t          changeEvent;
			pthread_mutex_t			changeEventLock;
            
			bool                    changeEventPending;
        
			short                   connectProgress;
        
			/** A DeviceDisplay structure that describes the device's display properties. */
			environs::DeviceDisplay display;
            
            static std::queue < NotifierContext * >    notifierQueue;
            static pthread_mutex_t                     notifierMutex;
            
            static void     EnqueueNotification ( NotifierContext * ctx );
            
            static void     NotifierThread ();
            static void *   NotifierThreadStarter ( void * );
            
			static sp ( DeviceInstance ) Create ( int hInst, DeviceInfo * device );
			bool	Init ( int hInst );

			bool	CopyInfo ( DeviceInfo * device );
			bool	Update ( DeviceInfo * device );

			void	SetProgress ( int progress );
			bool	SetDirectContact ( int status );
			bool	SetFileProgress ( int fileID, int progress, bool send );

			void	Dispose ();
			void	NotifyObservers ( int flags, bool enqueue = true );
        
			void	NotifyObserversForMessage ( const sp ( MessageInstance ) &message, int flags, bool enqueue = true );
			void	NotifyObserversForData ( const sp ( FileInstance ) &fileInst, int flags, bool enqueue = true );
			void	NotifySensorObservers ( environs::SensorFrame * pack );

			void	ClearMessagesThread ();
			static void * ClearMessagesThreader ( void * arg );

			void	ClearStorageThread ();
			static void * ClearStorageThreader ( void * arg );

			void	VerifyStoragePath ();

			void	AddMessage ( const char * message, int length, bool sent, char connection );
			void	AddFile ( int type, int fileID, const char * fileDescriptor, const char * filePath, int  bytesToSend, bool sent );

			static void * FileParseThread ( void * pack );
			bool	ParseStoragePath ( bool wait );

			static void ParseAllFiles ( const svsp ( DeviceInstance ) &devices );

			/**
			* Creates a portal instance.
			*
			* @param request   The portal request.
			* @return PortalInstance-object
			*/
			sp ( PortalInstance ) PortalCreate ( int request );

			/**
			* Creates a portal instance.
			*
			* @param Environs_PORTAL_DIR   A value of PORTAL_DIR_* that determines whether an outgoing or incoming portal.
			* @param portalType
			* @param slot
			* @return PortalInstance-object
			*/
			sp ( PortalInstance ) PortalCreate ( int Environs_PORTAL_DIR, environs::PortalType_t portalType, int slot );

			/**
			* Creates a portal instance with a given portalID.
			*
			* @param portalID   The portalID received from native layer.
			* @return PortalInstance-object
			*/
			sp ( PortalInstance ) PortalCreateID ( int portalID );


			/**
			* Query the first PortalInstance that manages a waiting/temporary incoming/outgoing portal.
			*
			* @return PortalInstance-object
			*/
			sp ( PortalInstance ) PortalGetWaiting ( bool outgoing );

			sp ( PortalInstance ) PortalGet ( bool outgoing );
		};

	}
} /* namepace Environs */

#endif

#endif // INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_H
