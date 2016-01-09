/**
 * IEnvirons.h
 * Declares an Interface to access an Environs object
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
#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_INTERFACE_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_INTERFACE_H

#include "Environs.Platforms.h"
#include "Environs.Observer.h"
#include "Device.Display.Decl.h"

#include <string>


namespace environs 
{
	CLASS DeviceList;

	PUBLIC_CLASS Environs CLI_ABSTRACT : public lib::IEnvironsDispose
	{
    public:
        
        /** Perform calls to the Environs object asynchronously. If set to Environs.CALL_WAIT, then all commands will block until the call finishes.
         * If set to Environs.CALL_NOWAIT, then certain calls (which may take longer) will be performed asynchronously. */
        bool					async;
        
		/**
		* Construction of Environs objects have to be done using Environs.CreateInstance() or Environs.New()
		*/
		Environs () { };

        virtual ~Environs () { };
        
        
        /**
         * Release memory allocated by Environs to be temporarily used by client code.
         *
         * @param		mem
         */
        virtual void ReleaseEnvMemory ( void * mem ) = 0;


		/**
		* Load settings for the given application environment from settings storage,
		* if any have been saved previously.
		*
		* @param	hInst		The handle to a particular native Environs instance.
		* @param 	appName		The application name for the application environment.
		* @param  	areaName	The area name for the application environment.
		*
		* @return   success
		*/
		virtual bool LoadSettings ( const char * appName, const char * areaName ) = 0;


		/**
		* Load settings. Prior to this call, an application environment MUST be given
		* using SetApplicationName and SetAreaName.
		*
		* @return   success
		*/
		virtual bool LoadSettings () = 0;


		/**
		* Instruct Environs to output verbose debug logging.
		*
		* @param enable      true = enable, false = disable
		*/
        virtual void SetDebug ( int enable ) = 0;
        
        /**
         * Instruct Environs to output verbose debug logging.
         *
         * @return level      debug level 0 ... 16
         */
        virtual int GetDebug () = 0;
        
        
        /**
         * Enable or disable device list update notifications from Mediator layer.
         * In particular, mobile devices should disable notifications if the devicelist is not
         * visible to users or the app transitioned to background.
         * This helps recuding cpu load and network traffic when not necessary.
         *
         * @param enable      true = enable, false = disable
         */
        virtual void SetMediatorNotificationSubscription ( bool enable ) = 0;
        
        
        /**
         * Get subscription status of device list update notifications from Mediator layer.
         *
         * @return enable      true = enable, false = disable
         */
        virtual bool GetMediatorNotificationSubscription ( ) = 0;
        
        
        /**
         * Enable or disable short messages from Mediator layer.
         * In particular, mobile devices should disable short messages if the app transitioned to background or mobile network only.
         * This helps recuding cpu load and network traffic when not necessary.
         *
         * @param enable      true = enable, false = disable
         */
        virtual void SetMessagesSubscription ( bool enable ) = 0;
        
        
        /**
         * Get subscription status of short messages from Mediator layer.
         *
         * @return enable      true = enable, false = disable
         */
        virtual bool GetMessagesSubscription ( ) = 0;
        
        
        /**
         * Ignore autodetection of the actual runtime platform and enforce the given platform.
         *
         * @param		platform of type Environs.platform
         */
        virtual void SetPlatform ( Platforms_t platform ) = 0;


		/**
		* Instruct Environs to show log messages in the status log.
		*
		* @param enable      true = enable, false = disable
		*/
		virtual void SetUseNotifyDebugMessage ( int enable ) = 0;


		/**
		* Query Environs settings that show log messages in the status log.
		*
		* @return enable      true = enabled, false = disabled
		*/
		virtual bool GetUseNotifyDebugMessage () = 0;


		/**
		* Instruct Environs to create and write a log file in the working directory.
		*
		* @param enable      true = enable, false = disable
		*/
		virtual void SetUseLogFile ( bool enable ) = 0;


		/**
		* Query Environs settings that create and write a log file in the working directory.
		*
		* @return enable      true = enabled, false = disabled
		*/
		virtual bool GetUseLogFile () = 0;


		//virtual bool opt ( const char * key ) = 0;


		/**
		* Initialize the environment. This must be called after the user interface has been loaded, rendered and shown.
		* Tasks:
		*  - Request display always on state, hence disable standby and power management functions.
		*  - Load sensor managers and sensor services
		*
		* @return success    true = success, false = failed
		*/
		virtual bool Init () = 0;


		virtual void ResetIdentKeys () = 0;


		/**
		* Get the native version of Environs.
		*
		* @return		version string
		*/
		virtual const char * GetVersionString () = 0;


		/**
		* Get the native major version of Environs.
		*
		* @return		major version
		*/
		virtual int GetVersionMajor () = 0;

		/**
		* Get the native minor version of Environs.
		*
		* @return		minor version
		*/
		virtual int GetVersionMinor () = 0;

		/**
		* Get the native revision of Environs.
		*
		* @return		revision
		*/
		virtual int GetVersionRevision () = 0;


		/**
		* Query whether the native layer was build for release (or debug).
		*
		* @return	true = Release build, false = Debug build.
		*/
		virtual bool GetIsReleaseBuild () = 0;


		/**
		* Set the device type that the local instance of Environs shall use for identification within the environment.&nbsp;
		* Valid type are enumerated in Environs.DEVICE_TYPE_*
		* --> Deprecated <--
		*
		* @param	type	Environs.DEVICE_TYPE_*
		*/
		virtual void SetDeviceType ( char value ) = 0;


		/**
		* Get the device type that the local instance of Environs use for identification within the environment.&nbsp;
		* Valid type are enumerated in Types.DEVICE_TYPE_*
		* --> Deprecated <--
		*
		* @return	type	Environs.DEVICE_TYPE_*
		*/
		virtual char GetDeviceType () = 0;


		/**
		* Set the ports that the local instance of Environs shall use for listening on connections.
		*
		* @param	tcpPort communication channel
		* @param	udpPort data channel
		*
		* @return  success
		*/
		virtual bool SetPorts ( int tcpPort, int udpPort ) = 0;

		virtual unsigned int GetIPAddress () = 0;
		virtual unsigned int GetSubnetMask () = 0;

		virtual const char * GetSSID () = 0;
		virtual const char * GetSSIDDesc () = 0;


		/**
		* Set the device id that is assigned to the instance of Environs.
		*
		* @param   deviceID
		*/
		virtual void SetDeviceID ( int deviceID ) = 0;


		/**
		* Get the device id that the application has assigned to the instance of Environs.
		*
		* @return	deviceID
		*/
		virtual int GetDeviceID () = 0;


		/**
		* Request a device id from mediator server instances that have been provided before this call.
		* Prior to this call, the area and application name must be provided as well,
		* in order to get an available device id for the specified application environment.
		* If the application has already set a deviceID (using setDeviceID), this call returns the previously set value.
		*
		* @return	deviceID
		*/
		virtual int GetDeviceIDFromMediator () = 0;


		/**
		* Query whether the name of the current device has been set before.
		*
		* @return	has DeviceUID
		*/
		virtual bool HasDeviceUID () = 0;


		/**
		* Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
		*
		* @param 	name    A unique device identifier.
		*
		* @return	success
		*/
		virtual bool SetDeviceUID ( const char * name ) = 0;


		/**
		* Query ip of custom Mediator.
		*
		* @return ip
		*/
		virtual unsigned int GetMediatorIPValue () = 0;


		/**
		* Query ip of custom Mediator.
		*
		* @return ip
		*/
		virtual const char * GetMediatorIP () = 0;


		/**
		* Query port of custom Mediator.
		*
		* @return port
		*/
		virtual int GetMediatorPort () = 0;


		/**
		* Determines whether to use Crypto Layer Security for Mediator connections.
		* If a Mediator enforces CLS, then disabling this option will result in failure to connect to that Mediator.
		*
		* @param	enable
		*/
		virtual void SetUseCLSForMediator ( bool enable ) = 0;


		/**
		* Query whether to use Crypto Layer Security for Mediator connections.
		*
		* @return	enabled
		*/
		virtual bool GetUseCLSForMediator () = 0;


		/**
		* Determines whether to use Crypto Layer Security for device-to-device connections.
		*
		* @param	enable
		*/
		virtual void SetUseCLSForDevices ( bool enable ) = 0;


		/**
		* Query whether to use Crypto Layer Security for device-to-device connections.
		*
		* @return	enabled
		*/
		virtual bool GetUseCLSForDevices () = 0;


		/**
		* Determines whether to enforce Crypto Layer Security for device-to-device connections.
		*
		* @param	enable
		*/
		virtual void SetUseCLSForDevicesEnforce ( bool enable ) = 0;


		/**
		* Query whether to enforce Crypto Layer Security for device-to-device connections.
		*
		* @return	enabled
		*/
		virtual bool GetUseCLSForDevicesEnforce () = 0;


		/**
		* Enable Crypto Layer Security for all traffic (incl. those of interactive type) in device-to-device connections.
		*
		* @param	enable
		*/
		virtual void SetUseCLSForAllTraffic ( bool enable ) = 0;


		/**
		* Query whether all traffic (incl. those of interactive type) in device-to-device connections is encrypted.
		*
		* @return	enabled
		*/
		virtual bool GetUseCLSForAllTraffic () = 0;


		/**
		* Determines whether to use environs default Mediator predefined by framework developers or not.
		*
		* @param enable 	true = use the default Mediator
		*/
		virtual void SetUseDefaultMediator ( bool enable ) = 0;


		/**
		* Query whether to use given Mediator by setMediator()
		*
		* @return enabled
		*/
		virtual bool GetUseDefaultMediator () = 0;


		/**
		* Determines whether to use given Mediator by setMediator()
		*
		* @param enable 	true = enable, false = disable
		*/
		virtual void SetUseCustomMediator ( bool enable ) = 0;


		/**
		* Query whether to use given Mediator by setMediator()
		*
		* @return enabled
		*/
		virtual bool GetUseCustomMediator () = 0;


		/**
		* Set custom Mediator to use.
		*
		* @param ip
		* @param port
		*
		* @return	success
		*/
		virtual bool SetMediator ( const char * ip, unsigned short port ) = 0;


		/**
		* Set the user name for authentication with a Mediator service.&nbsp;Usually the user's email address is used as the user name.
		*
		* @param 	name
		*
		* @return	success
		*/
		virtual bool SetMediatorUserName ( const char * name ) = 0;
		virtual bool SetUserName ( const char * name ) = 0;


		/**
		* Query UserName used to authenticate with a Mediator.
		*
		* @return UserName
		*/
		virtual const char * GetMediatorUserName () = 0;


		/**
		* Enable or disable anonymous logon to the Mediator.
		*
		* @param 	enable A boolean that determines the target state.
		*/
		virtual void SetUseMediatorAnonymousLogon ( bool enable ) = 0;


		/**
		* Get setting of anonymous logon to the Mediator.
		*
		* @return 	enabled A boolean that determines the target state.
		*/
		virtual bool GetUseMediatorAnonymousLogon () = 0;


		/**
		* Set the user password for authentication with a Mediator service.&nbsp;The password is stored as a hashed token within Environs.
		*
		* @param 	password
		*
		* @return	success
		*/
		virtual bool SetMediatorPassword ( const char * password ) = 0;
		virtual bool SetUserPassword ( const char * password ) = 0;


		/**
		* Enable or disable authentication with the Mediator using username/password.
		*
		* @param 	enable
		*/
		virtual void SetUseAuthentication ( bool enable ) = 0;


		/**
		* Query the filter level for device management within Environs.
		*
		* return   level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_AREA, Environs.MEDIATOR_FILTER_AREA_AND_APP
		*/
		virtual int GetMediatorFilterLevel () = 0;


		/**
		* Set the filter level for device management within Environs.
		*
		* @param   level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_AREA, Environs.MEDIATOR_FILTER_AREA_AND_APP
		*/
		virtual void SetMediatorFilterLevel ( int level ) = 0;


		/**
		* Retrieve a boolean that determines whether Environs shows up a login dialog if a Mediator is used and no credentials are available.
		*
		* @return	true = yes, false = no
		*/
		virtual bool GetUseMediatorLoginDialog () = 0;


		/**
		* Instruct Environs to show up a login dialog if a Mediator is used and no credentials are available.
		*
		* @param   enable      true = enable, false = disable
		*/
		virtual void SetUseMediatorLoginDialog ( bool enable ) = 0;


		/**
		* Retrieve a boolean that determines whether Environs disable Mediator settings on dismiss of the login dialog.
		*
		* @return	true = yes, false = no
		*/
		virtual bool GetMediatorLoginDialogDismissDisable () = 0;


		/**
		* Instruct Environs disable Mediator settings on dismiss of the login dialog.
		*
		* @param   enable      true = enable, false = disable
		*/
		virtual void SetMediatorLoginDialogDismissDisable ( bool enable ) = 0;


		/**
		* Register at known Mediator server instances.
		*
		* @return	success
		*/
        virtual bool RegisterAtMediators () = 0;
        
        
        /**
         * Determines whether touch events should be translated to mouse events. This is performed by a touch recognizer module. Therefore the module must be in the lib-folder and loaded by Environs.
         *
         * @param   enable      true = enable, false = disable
         */
        virtual void SetUseMouseEmulation ( bool enable ) = 0;
        
        
        /**
         * Determines whether touch events should be visualized as rounded circles on the desktop Window. This is performed by a touch recognizer module. Therefore the module must be in the lib-folder and loaded by Environs.
         *
         * @param   enable      true = enable, false = disable
         */
        virtual void SetUseTouchVisualization ( bool enable ) = 0;

        
		/**
		* Start Environs.&nbsp;This is a non-blocking call and returns immediately.&nbsp;
		* 		Since starting Environs includes starting threads and activities that may take longer,&nbsp;
		* 		this call executes the start tasks within a thread.&nbsp;
		* 		In order to get the status, catch the onNotify handler of your EnvironsListener.
		*
		*/
		virtual void Start () = 0;


		/**
		* Query the status of Environs.&nsbp;Valid values are Types.NATIVE_STATUS_*
		*
		* @return  Environs.NATIVE_STATUS_*
		*/
		virtual int GetStatus () = 0;


		/**
		* Stop Environs and release all acquired resources.
		*/
		virtual void Stop () = 0;


		/**
		* Stop Environs and dispose all acquired resources.
		*/
		virtual void DisposeInstance () = 0;


		/**
		* Set the area name that the local instance of Environs shall use for identification within the environment.
		* It must be set before creating the Environs instance.
		*
		* @param	name
		*
		* @return	success
		*/
		virtual bool SetAreaName ( const char * name ) = 0;

		/**
		* Get the area name that the local instance of Environs use for identification within the environment.
		* It must be set before creating the Environs instance.
		*
		* @return	areaName
		*/
		virtual const char * GetAreaName () = 0;


		/**
		* Set the application name of that the local instance of Environs shall use for identification within the environment.
		* It must be set before creating the Environs instance.
		*
		* @param	name
		*
		* @return	success
		*/
		virtual void SetApplication ( const char * name ) = 0;
		virtual void SetApplicationName ( const char * name ) = 0;


		/**
		* Get the application name that the local instance of Environs use for identification within the environment.
		* It must be set before creating the Environs instance.
		*
		* @return	appName
		*/
		virtual const char * GetApplicationName () = 0;


		/**
		* Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
		*
		* @param 	deviceName
		*
		* @return	success
		*/
		virtual bool SetDeviceName ( const char * name ) = 0;


		/**
		* Use default encoder, decoder, capture, render modules.
		*
		* @return  success
		*/
		virtual bool SetUsePortalDefaultModules () = 0;


		virtual void SetUseH264 ( bool enable ) = 0;
		virtual bool GetUseH264 () = 0;


		/**
		* Determine whether to use  TCP for portal streaming (if not selectively set for a particular deviceID)
		*
		* @param   enable
		*/
		virtual void SetUsePortalTCP ( bool enable ) = 0;


		/**
		* Query whether to use TCP for portal streaming (UDP otherwise)
		*
		* @return enabled
		*/
		virtual bool GetUsePortalTCP () = 0;


		/**
		* Use encoder module with the name moduleName. (libEnv-Enc...).
		*
		* @param	name  The module name
		*
		* @return	success
		*/
		virtual bool SetUseEncoder ( const char * name ) = 0;


		/**
		* Use decoder module with the name moduleName. (libEnv-Dec...).
		*
		* @param	name  The module name
		*
		* @return	success
		*/
		virtual bool SetUseDecoder ( const char * name ) = 0;


		/**
		* Use capture module with the name moduleName. (libEnv-Cap...).
		*
		* @param	name	the name of the module
		*
		* @return	success
		*/
		virtual bool SetUseCapturer ( const char * name ) = 0;


		/**
		* Use render module with the name moduleName. (libEnv-Rend...).
		*
		* @param	name	the name of the module
		*
		* @return	success
		*/
		virtual bool SetUseRenderer ( const char * name ) = 0;

		/**
		* Enable or disable a touch recognizer module by name (libEnv-Rec...).
		*
		* @param	moduleName  The module name
		* @param	enable      Enable or disable
		*
		* @return  success
		*/
		virtual bool SetUseTouchRecognizer ( const char * name, bool enable ) = 0;


		virtual int SetUseTracker ( int async, const char * name ) = 0;

		virtual int GetUseTracker ( const char * name ) = 0;

		virtual bool DisposeTracker ( int async, const char * name ) = 0;

		virtual bool PushTrackerCommand ( int async, int moduleIndex, int command ) = 0;


		/**
		* Add an observer for communication with Environs and devices within the environment.
		*
		* @param   observer Your implementation of EnvironsObserver.
		*
		* @return	success
		*/
		virtual bool AddObserver ( EnvironsObserver * observer ) = 0;

		/**
		* Remove an observer for communication with Environs and devices within the environment.
		*
		* @param   observer Your implementation of EnvironsObserver.
		*
		* @return	success
		*/
		virtual bool RemoveObserver ( EnvironsObserver * observer ) = 0;

		/**
		* Add an observer for receiving messages.
		*
		* @param   observer Your implementation of EnvironsMessageObserver.
		*
		* @return	success
		*/
		virtual bool AddObserverForMessages ( EnvironsMessageObserver * observer ) = 0;

		/**
		* Remove an observer for receiving messages.
		*
		* @param   observer Your implementation of EnvironsMessageObserver.
		*
		* @return	success
		*/
		virtual bool RemoveObserverForMessages ( EnvironsMessageObserver * observer ) = 0;

		/**
		* Add an observer for receiving data buffers and files.
		*
		* @param   observer Your implementation of EnvironsDataObserver.
		*
		* @return	success
		*/
		virtual bool AddObserverForData ( EnvironsDataObserver * observer ) = 0;

		/**
		* Remove an observer for receiving data buffers and files.
		*
		* @param   observer Your implementation of EnvironsDataObserver.
		*
		* @return	success
		*/
		virtual bool RemoveObserverForData ( EnvironsDataObserver * observer ) = 0;

		/**
		* Add an observer for receiving sensor data of all devices.
		* Please note: This observer reports sensor data of all devices that are connected and send to us.
		* It's highly recommend to attach an SensorObserver to a DeviceInstance to process device filtered sensor data.
		*
		* @param   observer Your implementation of EnvironsDataObserver.
		*
		* @return	success
		*/
		virtual bool AddObserverForSensorData ( EnvironsSensorDataObserver * observer ) = 0;

		/**
		* Remove an observer for receiving data buffers and files.
		* Please note: This observer reports sensor data of all devices that are connected and send to us.
		* It's highly recommend to attach an SensorObserver to a DeviceInstance to process device filtered sensor data.
		*
		* @param   observer Your implementation of EnvironsDataObserver.
		*
		* @return	success
		*/
		virtual bool RemoveObserverForSensorData ( EnvironsSensorDataObserver * observer ) = 0;

		/**
		* Create a new collection that holds all devices of given list type. This list ist updated dynamically by Environs.
		* After client code is done with the list, the list->Release () method MUST be called by the client code,
		* in order to release the resource (give ownership) back to Environs.
		*
		* @return Collection of IDeviceInstance objects
		*/
        sp ( DeviceList ) CreateDeviceList ( int MEDIATOR_DEVICE_CLASS_ )
		{
			ENVIRONS_IR_SP1_RETURN ( DeviceList, CreateDeviceListRetained ( MEDIATOR_DEVICE_CLASS_ ) );
		}

		virtual DeviceList OBJ_ptr CreateDeviceListRetained ( int MEDIATOR_DEVICE_CLASS_ ) = 0;


		virtual bool GetPortalNativeResolution () = 0;
		virtual void SetPortalNativeResolution ( bool enable ) = 0;

		virtual bool GetPortalAutoStart () = 0;
		virtual void SetPortalAutoStart ( bool enable ) = 0;
        
        virtual int GetHandle () = 0;
        
        virtual DeviceDisplay * GetDeviceDisplayProps ( int nativeID ) = 0;
        
        /**
         * Connect to device with the given ID and a particular application environment.
         *
         * @param deviceID	Destination device ID
         * @param areaName Project name of the application environment
         * @param appName	Application name of the application environment
         * @param async	    Perform asynchronous. Non-async means that this call blocks until the call finished.
         * @return status	0: Connection can't be conducted (maybe environs is stopped or the device ID is invalid) &nbsp;
         * 					1: A connection to the device already exists or a connection task is already in progress) &nbsp;
         * 					2: A new connection has been triggered and is in progress
         */
        virtual int DeviceConnect ( int deviceID, const char * areaName, const char * appName, int async ) = 0;
        
        
        /**
         * Set render callback.
         *
         * @param async			Perform asynchronous. Non-async means that this call blocks until the call finished.
         * @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
         * @param callback		The pointer to the callback.
         * @param callbackType	A value of type RENDER_CALLBACK_TYPE_* that tells the portal receiver what we actually can render..
         * @return				true = success, false = failed.
         */
        virtual bool SetRenderCallback ( int async, int portalID, void * callback, int callbackType ) = 0;
        
        
        /**
         * Release render callback delegate or pointer
         *
         * @param async			Perform asynchronous. Non-async means that this call blocks until the call finished.
         * @param portalID		This is an ID that Environs use to manage multiple portals from the same source device. It is provided within the notification listener as sourceIdent. Applications should store them in order to address the correct portal within Environs.
         * @param callback		A delegate that manages the callback.
         * @return				true = success, false = failed.
         */
        virtual bool ReleaseRenderCallback ( int async, int portalID ) = 0;
        
        /**
         * Start streaming of portal to or from the portal identifier (received in notification).
         *
         * @param async      	Execute asynchronous. Non-async means that this call blocks until the command has finished.
         * @param portalID		An application specific id (e.g. used for distinguishing front facing or back facing camera)
         *
         * @return success
         */
        virtual bool StartPortalStream ( int async, int portalID ) = 0;
        
        
        /**
         * Stop streaming of portal to or from the portal identifier (received in notification).
         *
         * @param 	async      	Execute asynchronous. Non-async means that this call blocks until the command has finished.
         * @param 	nativeID    The native device id of the target device.
         * @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
         * 						It is provided within the notification listener as sourceIdent.&nbsp;
         * 					    Applications should store them in order to address the correct portal within Environs.
         * @return success
         */
        virtual bool StopPortalStream ( int async, int nativeID, int portalID ) = 0;
        
        
        /**
         * Get the status, whether the device (id) has established an active portal
         *
         * @param 	nativeID    The device id of the target device.
         * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
         * @return	success 	true = yes, false = no
         */
        virtual bool GetPortalEnabled ( int nativeID, int portalType ) = 0;
        
        
        /**
         * Get the number of devices that are currently connected to our device.
         *
         * @return	Count of connected devices
         */
        virtual int GetConnectedDevicesCount () = 0;
        
        
        /**
         * Get enabled status for stream encoding.
         *
         * @return	enabled
         */
        virtual bool GetUseStream () = 0;
        
        
        /**
         * Get platform support for OpenCL.
         *
         * @return	enabled
         */
        virtual bool GetUseOpenCL () = 0;
        
        
        /**
         * Switch platform support for OpenCL rendering.
         *
         * @param enable
         */
        virtual void SetUseOpenCL ( bool enable ) = 0;

        
        virtual const char * GetFilePathNative ( int nativeID, int fileID ) = 0;

        
        virtual char * GetFilePath ( int nativeID, int fileID ) = 0;
        
        
        /**
         * Load the file that is assigned to the fileID into a byte array.
         *
         * @param nativeIDOK		Indicates that the nativeOrDeviceID represents a nativeID.
         * @param nativeID		The native id of the device.
         * @param deviceID		The device id of the device.
         * @param areaName		Area name of the application environment.
         * @param appName		Application name of the application environment.
         * @param fileID			The id of the file to load (given in the onData receiver).
         * @param size			An int pointer, that receives the size of the returned buffer.
         * @return byte-array
         */
        virtual unsigned char * GetFile ( bool nativeIDOK, int nativeID, int deviceID, const char * areaName, const char * appName, int fileID, int & size ) = 0;
        

        
	};
}


#endif // INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT_INTERFACE_H









