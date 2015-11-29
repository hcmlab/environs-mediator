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

#include "Environs.Observer.CLI.h"
#include "Environs.Platforms.h"
#include "Interop/Smart.Pointer.h"
#include "Environs.Msg.Types.h"
#include "Interfaces/IEnvirons.Dispose.h"
#include "Device.Info.h"
#include "Device.Display.Decl.h"

#ifdef CLI_CPP
#else
#	include "Interfaces/IDevice.Instance.h"

#	include <queue>
#endif

#include <string>

/* Namespace: environs -> */
namespace environs
{
    class Instance;

	namespace lib
	{
        CLASS Environs;

#ifndef CLI_CPP
        class PortalInstance;
        class FileInstance;
        class MessageInstance;
#endif

		CLASS NotifierContext
		{
		public:
			int                     type;
			int                     flags;
			sp ( DeviceInstance )   device;
			sp ( PLATFORMSPACE MessageInstance )  message;
			sp ( PLATFORMSPACE FileInstance )     fileData;

#ifdef CLI_CPP
			String ^				propertyName;
#endif
		};


		PUBLIC_CLASS DeviceInstance CLI_ABSTRACT DERIVE_c_only ( environs::DeviceInstance ) DERIVE_DISPOSEABLE 
		{
            MAKE_FRIEND_CLASS ( Environs );
            MAKE_FRIEND_CLASS ( DeviceList );
            MAKE_FRIEND_CLASS ( PortalInstance );
            MAKE_FRIEND_CLASS ( MessageInstance );
            MAKE_FRIEND_CLASS ( FileInstance );
            MAKE_FRIEND_CLASS ( DeviceInstanceProxy );

		public:
			ENVIRONS_LIB_API    DeviceInstance ();
			ENVIRONS_LIB_API    ~DeviceInstance ();


			/**
			 * Add an observer (DeviceObserver) that notifies about device property changes.
			 *
			 * @param observer A DeviceObserver
			 */
			ENVIRONS_LIB_API void AddObserver ( environs::DeviceObserver OBJ_ptr observer );

			/**
			 * Remove an observer (DeviceObserver) that was added before.
			 *
			 * @param observer A DeviceObserver
			 */
			ENVIRONS_LIB_API void RemoveObserver ( environs::DeviceObserver OBJ_ptr observer );


			/**
			 * Add an observer (DataObserver) that notifies about data received or sent through the DeviceInstance.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void AddObserverForData ( environs::DataObserver OBJ_ptr observer );


			/**
			 * Remove an observer (SensorObserver) that was added before.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void RemoveObserverForData ( environs::DataObserver OBJ_ptr observer );


			/**
			 * Add an observer (SensorObserver) that notifies about data received or sent through the DeviceInstance.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void AddObserverForSensors ( environs::SensorObserver OBJ_ptr observer );


			/**
			 * Remove an observer (DataObserver) that was added before.
			 *
			 * @param observer A DataObserver
			 */
			ENVIRONS_LIB_API void RemoveObserverForSensors ( environs::SensorObserver OBJ_ptr observer );


			/**
			 * Add an observer (MessageObserver) that notifies about messages received or sent through the DeviceInstance.
			 *
			 * @param observer A MessageObserver
			 */
			ENVIRONS_LIB_API void AddObserverForMessages ( environs::MessageObserver OBJ_ptr observer );


			/**
			 * Remove an observer (MessageObserver) that was added before.
			 *
			 * @param observer A MessageObserver
			 */
			ENVIRONS_LIB_API void RemoveObserverForMessages ( environs::MessageObserver OBJ_ptr observer );


			/**
			 * Notify to all observers (DeviceObserver) that the appContext has changed.
			 *
			 * @param customFlags Either custom declared flags or 0. If 0 is provided, then the flag Environs.DEVICE_INFO_ATTR_APP_CONTEXT will be used.
			 */
			ENVIRONS_LIB_API void NotifyAppContextChanged ( int customFlags );


			/** The device properties structure into a DeviceInfo object. */
			environs::DeviceInfo OBJ_ptr info ();

			ENVIRONS_LIB_API CLI_VIRTUAL CString_ptr ToString () CLI_OVERRIDE;

			ENVIRONS_LIB_API CString_ptr ips ();

			ENVIRONS_LIB_API CString_ptr ipes ();

			ENVIRONS_LIB_API bool EqualsAppEnv ( environs::DeviceInfo OBJ_ptr equalTo );
			ENVIRONS_LIB_API bool EqualsAppEnv ( CString_ptr areaName, CString_ptr appName );

			ENVIRONS_LIB_API bool LowerThanAppEnv ( environs::DeviceInfo OBJ_ptr compareTo );
			ENVIRONS_LIB_API bool LowerThanAppEnv ( CString_ptr areaName, CString_ptr appName );

			ENVIRONS_LIB_API bool EqualsID ( environs::DeviceInstance OBJ_ptr equalTo );
			ENVIRONS_LIB_API bool EqualsID ( int deviceID, CString_ptr areaName, CString_ptr appName );


			ENVIRONS_LIB_API CString_ptr DeviceTypeString ( environs::DeviceInfo OBJ_ptr info );
			ENVIRONS_LIB_API CString_ptr DeviceTypeString ();

			ENVIRONS_LIB_API CString_ptr GetBroadcastString ( bool fullText );

			ENVIRONS_LIB_API bool isConnected ();
			ENVIRONS_LIB_API bool directStatus ();

#ifdef CLI_CPP
			/** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
			bool                  async;
#endif
			/**
			 * disposed is true if the object is no longer valid. Nothing will be updated anymore.
			 * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
			 * */
			ENVIRONS_LIB_API bool disposed ();

			/** An identifier that is unique for this object. */
			ENVIRONS_LIB_API long objID () { return ( long ) objID_; };

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
			ENVIRONS_LIB_API UCharArray_ptr GetFile ( int fileID, int OBJ_ref size );

			/**
			* Query the absolute path for the local filesystem that is assigned to the fileID received by deviceID.
			*
			* @param fileID        The id of the file to load (given in the onData receiver).
			* @return absolutePath
			*/
			ENVIRONS_LIB_API CString_ptr GetFilePath ( int fileID );


			/**
			* Creates a portal instance that requests a portal.
			*
			* @param 	portalType	        Project name of the application environment
			*
			* @return 	PortalInstance-object
			*/
			sp ( PLATFORMSPACE PortalInstance ) PortalRequest ( CPP_CLI ( PortalType_t, Environs::PortalType ) portalType );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::PortalInstance OBJ_ptr PortalRequestRetained ( environs::PortalType_t portalType );
#endif

			/**
			* Creates a portal instance that provides a portal.
			*
			* @param 	portalType	        Project name of the application environment
			*
			* @return 	PortalInstance-object
			*/
			sp ( PLATFORMSPACE PortalInstance ) PortalProvide ( CPP_CLI ( PortalType_t, Environs::PortalType ) portalType );

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::PortalInstance OBJ_ptr PortalProvideRetained ( environs::PortalType_t portalType );
#endif


			/**
			* Query the first PortalInstance that manages an outgoing portal.
			*
			* @return PortalInstance-object
			*/
			sp ( PLATFORMSPACE PortalInstance ) PortalGetOutgoing ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::PortalInstance OBJ_ptr PortalGetOutgoingRetained ();
#endif


			/**
			* Query the first PortalInstance that manages an incoming portal.
			*
			* @return PortalInstance-object
			*/
			sp ( PLATFORMSPACE PortalInstance ) PortalGetIncoming ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API environs::PortalInstance OBJ_ptr PortalGetIncomingRetained ();
#endif


			/**
			* Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
			*
			* @param fileID        A user-customizable id that identifies the file to be send.
			* @param fileDescriptor (e.g. filename)
			* @param filePath      The path to the file to be send.
			* @return success
			*/
			ENVIRONS_LIB_API bool SendFile ( int fileID, CString_ptr fileDescriptor, CString_ptr filePath );

			/**
			* Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
			*
			* @param fileID        A user-customizable id that identifies the file to be send.
			* @param fileDescriptor (e.g. filename)
			* @param buffer        A buffer to be send.
			* @param bytesToSend number of bytes in the buffer to send
			* @return success
			*/
			ENVIRONS_LIB_API bool SendBuffer ( int fileID, CString_ptr fileDescriptor, UCharArray_ptr buffer, int bytesToSend );

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
			ENVIRONS_LIB_API bool SendMessage ( CString_ptr msg, int length );

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
			ENVIRONS_LIB_API bool SendMessage ( CString_ptr msg );


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
			NLayerMapType ( int, PLATFORMSPACE FileInstance ) GetAllFiles ();

#ifndef CLI_CPP
			ENVIRONS_LIB_API envArrayList OBJ_ptr GetAllFilesRetained ();
#endif

			/**
			* Get a list with all messages that this device has received (and sent).
			*
			* @return Collection with objects of type MessageInstance
			*/
			NLayerVecType ( PLATFORMSPACE MessageInstance ) GetAllMessages ();
#ifndef CLI_CPP
			ENVIRONS_LIB_API envArrayList OBJ_ptr GetAllMessagesRetained ();
#endif

			/**
			* Query the absolute path for the local filesystem to the persistent storage for this DeviceInstance.
			*
			* @return absolutePath
			*/
			ENVIRONS_LIB_API CString_ptr GetStoragePath ();

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

		INTERNAL:

			ENVIRONS_OUTPUT_ALLOC_RESOURCE ( DeviceInstance );

			static bool                 globalsInit;
			static bool                 GlobalsInit ();
			static void                 GlobalsDispose ();

			int                         hEnvirons;

#ifndef CLI_CPP
			Instance                *   env;
#endif
			pthread_mutex_t				devicePortalsLock;

			/** A collection of PortalInstances that this device has established or is managing. */
			NLayerVecTypeObj ( PLATFORMSPACE PortalInstance )	devicePortals;

			/**
			 * disposed is true if DeviceInstance is no longer valid. Nothing will be updated anymore.
			 * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
			 * */
			bool						disposed_;

			void                        PlatformDispose ();

			/** The device properties structure into a DeviceInfo object. */
			environs::DeviceInfo cli_OBJ_ptr	info_;

			/** A collection of observers that observe this device instance for changes and events. */
			ENVOBSERVER ( lib::IIDeviceObserver, environs::DeviceObserver )	observers;

			/** A collection of observers that observe this device instance for changes and events. */
			ENVOBSERVER ( lib::IIMessageObserver, environs::MessageObserver )	observersForMessages;

			/** A collection of observers that observe this device instance for changes and events. */
			ENVOBSERVER ( lib::IIDataObserver, environs::DataObserver )		observersForData;

			/** A collection of observers that observe this device instance for changes and events. */
			ENVOBSERVER ( lib::IISensorObserver, environs::SensorObserver )	observersForSensorData;

			char					storageLoaded;

			bool                    directStatus_;

			/** Determines whether the device shall receive accelerometer events from us. */
			bool                    enableAccelSender;

			/** Determines whether the device shall receive magnetic field events from us. */
			bool                    enableMagneticSender;

			/** Determines whether the device shall receive gyroscope events from us. */
			bool                    enableGyroscopeSender;

			/** Determines whether the device shall receive orientation events from us. */
			bool                    enableOrientationSender;

			STRING_T	            toString_;
			STRING_T	            ips_;
			STRING_T	            ipes_;
			STRING_T	            filePath;

			STRING_T				storagePath;

			NLayerVecType ( PLATFORMSPACE MessageInstance ) messages;

			NLayerMapType ( int, PLATFORMSPACE FileInstance ) files;

			pthread_mutex_t			storageLock;

#ifdef CLI_CPP
			static CString_ptr DefAreaName = "Environs";
			static CString_ptr DefAppName = "HCMApp";

			virtual PLATFORMSPACE DeviceInstance ^ GetPlatformObj () = 0;
#endif
			pthread_cond_t          changeEvent;
			pthread_mutex_t			changeEventLock;

			bool                    changeEventPending;

			short                   connectProgress;

			/** A DeviceDisplay structure that describes the device's display properties. */
			environs::DeviceDisplay display;

			static stdQueue ( NotifierContext OBJ_ptr ) notifierQueue;
			static pthread_mutex_t						notifierMutex;

			static void     EnqueueNotification ( NotifierContext OBJ_ptr ctx );

			static void     NotifierThread ();
			static void c_OBJ_ptr   NotifierThreadStarter ( pthread_param_t );

			static sp ( PLATFORMSPACE DeviceInstance )	Create ( int hInst, environs::DeviceInfo OBJ_ptr device );
			bool							Init ( int hInst );

			bool			CopyInfo ( environs::DeviceInfo OBJ_ptr device );
			bool			Update ( environs::DeviceInfo OBJ_ptr device );

			void			SetProgress ( int progress );
			bool			SetDirectContact ( int status );
			bool			SetFileProgress ( int fileID, int progress, bool send );

			void			DisposeInstance ();
			void			NotifyObservers ( int flags, bool enqueue );

#ifdef CLI_CPP
			void			NotifyObservers ( String ^ prop, int flags, bool enqueue );

			virtual void	OnPropertyChanged ( String ^ name, int changed ) = 0;
#endif
			void			NotifyObserversForMessage ( c_const sp ( PLATFORMSPACE MessageInstance ) c_ref message, int flags, bool enqueue );
			void			NotifyObserversForData ( c_const sp ( PLATFORMSPACE FileInstance ) c_ref fileInst, int flags, bool enqueue );
			void			NotifySensorObservers ( environs::SensorFrame OBJ_ptr pack );

			void			ClearMessagesThread ();
			static void c_OBJ_ptr	ClearMessagesThreader ( pthread_param_t arg );

			void			ClearStorageThread ();
			static void c_OBJ_ptr	ClearStorageThreader ( pthread_param_t arg );

			void			VerifyStoragePath ();

			void			AddMessage ( CString_ptr message, int length, bool sent, char connection );
			void			AddFile ( int type, int fileID, CString_ptr fileDescriptor, CString_ptr filePath, int  bytesToSend, bool sent );

			static void c_OBJ_ptr	FileParseThread ( pthread_param_t pack );
			bool			ParseStoragePath ( bool wait );

			static void		ParseAllFiles ( c_const NLayerVecType ( PLATFORMSPACE DeviceInstance ) c_ref devices );

			/**
			* Creates a portal instance.
			*
			* @param request   The portal request.
			* @return PortalInstance-object
			*/
			sp ( PLATFORMSPACE PortalInstance ) PortalCreate ( int request );

			/**
			* Creates a portal instance.
			*
			* @param Environs_PORTAL_DIR   A value of PORTAL_DIR_* that determines whether an outgoing or incoming portal.
			* @param portalType
			* @param slot
			* @return PortalInstance-object
			*/
			sp ( PLATFORMSPACE PortalInstance ) PortalCreate ( int Environs_PORTAL_DIR, CPP_CLI ( PortalType_t, Environs::PortalType ) portalType, int slot );

			/**
			* Creates a portal instance with a given portalID.
			*
			* @param portalID   The portalID received from native layer.
			* @return PortalInstance-object
			*/
			sp ( PLATFORMSPACE PortalInstance ) PortalCreateID ( int portalID );


			/**
			* Query the first PortalInstance that manages a waiting/temporary incoming/outgoing portal.
			*
			* @return PortalInstance-object
			*/
			sp ( PLATFORMSPACE PortalInstance ) PortalGetWaiting ( bool outgoing );

			sp ( PLATFORMSPACE PortalInstance ) PortalGet ( bool outgoing );
		};

	}

} /* namepace Environs */

#endif

#endif // INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_H
