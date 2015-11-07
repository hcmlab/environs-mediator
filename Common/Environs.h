/**
 * Environs.h
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

#ifndef INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT__IMPL_H
#define INCLUDE_HCM_ENVIRONS_LIBRARY_OBJECT__IMPL_H

#include "Environs.Platforms.h"
#include "Environs.Build.Opts.h"

#if ( defined(ENVIRONS_IOS) || defined(ENVIRONS_OSX) )
#   ifdef __OBJC__
#       import <Environs.iOSX.h>
#   endif
#endif

#ifdef __cplusplus

#include "Interfaces/IEnvirons.h"
#include "Environs.Loader.h"
#include "Notify.Context.h"
#include "Environs.Observer.h"
#include "Device.List.h"
#include "Device.Instance.h"
#include "File.Instance.h"
#include "Message.Instance.h"
#include "Portal.Instance.h"

#include <queue>

namespace environs 
{
    class Instance;
    
    
	namespace lib
	{
        class DeviceList;
        
        
        class DeviceCommandContext
        {
        public:
            int             hEnvirons;
            Environs *      envObj;
            int             deviceID;
            int             type;
            std::string     areaName;
            std::string     appName;
            
            spv ( IIListObserver * ) observerList;
            svsp ( DeviceInstance ) vanished;
            svsp ( DeviceInstance ) appeared;
            
            int     notification;
            
            sp ( DeviceInfo ) device;
        };
        
        
		class Environs : public IEnvirons
		{
            friend class DeviceList;
			friend class PortalInstance;
            friend class EnvironsProxy;
            friend class FactoryProxy;
            
        private:
            /**
             * Construction of Environs objects have to be done using Environs.CreateInstance() or Environs.New()
             */
            ENVIRONS_LIB_API Environs ();

		public:
			ENVIRONS_LIB_API ~Environs ();
            
        
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
            ENVIRONS_LIB_API bool LoadSettings ( const char * appName, const char * areaName );
            
            
            /**
             * Load settings. Prior to this call, an application environment MUST be given
             * using SetApplicationName and SetAreaName.
             *
             * @return   success
             */
            ENVIRONS_LIB_API bool LoadSettings ( );


			/**
			* Instruct Environs to output verbose debug logging.
			*
			* @param enable      true = enable, false = disable
			*/
			ENVIRONS_LIB_API void SetDebug ( int enable );
        
        
			/**
			 * Instruct Environs to show log messages in the status log.
			 *
			 * @param enable      true = enable, false = disable
			 */
			ENVIRONS_LIB_API void SetUseNotifyDebugMessage ( int enable );
        
        
			/**
			 * Query Environs settings that show log messages in the status log.
			 *
			 * @return enable      true = enabled, false = disabled
			 */
			ENVIRONS_LIB_API bool GetUseNotifyDebugMessage ();
        
        
			/**
			 * Instruct Environs to create and write a log file in the working directory.
			 *
			 * @param enable      true = enable, false = disable
			 */
			ENVIRONS_LIB_API void SetUseLogFile ( int enable );
        
        
			/**
			 * Query Environs settings that create and write a log file in the working directory.
			 *
			 * @return enable      true = enabled, false = disabled
			 */
			ENVIRONS_LIB_API bool GetUseLogFile ();
        
        
			//ENVIRONS_LIB_API bool opt ( const char * key );
        
        
			/**
			 * Initialize the environment. This must be called after the user interface has been loaded, rendered and shown.
			 * Tasks:
			 *  - Request display always on state, hence disable standby and power management functions.
			 *  - Load sensor managers and sensor services
			 *
			 * @return success    true = success, false = failed
			 */
			ENVIRONS_LIB_API bool Init ();
        

			ENVIRONS_LIB_API void ResetIdentKeys ();
        
        
			/**
			 * Get the native version of Environs.
			 *
			 * @return		version string
			 */
			ENVIRONS_LIB_API const char * GetVersionString ();
        
        
			/**
			 * Get the native major version of Environs.
			 *
			 * @return		major version
			 */
			ENVIRONS_LIB_API int GetVersionMajor ();
        
			/**
			 * Get the native minor version of Environs.
			 *
			 * @return		minor version
			 */
			ENVIRONS_LIB_API int GetVersionMinor ();
        
			/**
			 * Get the native revision of Environs.
			 *
			 * @return		revision
			 */
			ENVIRONS_LIB_API int GetVersionRevision ();
        
        
			/**
			 * Query whether the native layer was build for release (or debug).
			 *
			 * @return	true = Release build, false = Debug build.
			 */
			ENVIRONS_LIB_API bool GetIsReleaseBuild ();
        
        
			/**
			 * Set the device type that the local instance of Environs shall use for identification within the environment.&nbsp;
			 * Valid type are enumerated in Environs.DEVICE_TYPE_*
			 * --> Deprecated <--
			 *
			 * @param	type	Environs.DEVICE_TYPE_*
			 */
			ENVIRONS_LIB_API void SetDeviceType ( char value );
        
        
			/**
			 * Get the device type that the local instance of Environs use for identification within the environment.&nbsp;
			 * Valid type are enumerated in Types.DEVICE_TYPE_*
			 * --> Deprecated <--
			 *
			 * @return	type	Environs.DEVICE_TYPE_*
			 */
			ENVIRONS_LIB_API char GetDeviceType ();
        
        
			/**
			 * Set the ports that the local instance of Environs shall use for listening on connections.
			 *
			 * @param	tcpPort communication channel
			 * @param	udpPort data channel
			 *
			 * @return  success
			 */
			ENVIRONS_LIB_API bool SetPorts ( int tcpPort, int udpPort );
        
			ENVIRONS_LIB_API unsigned int GetIPAddress ();
			ENVIRONS_LIB_API unsigned int GetSubnetMask ();
        
			ENVIRONS_LIB_API const char * GetSSID ();
			ENVIRONS_LIB_API const char * GetSSIDDesc ();
        
        
			/**
			 * Set the device id that is assigned to the instance of Environs.
			 *
			 * @param   deviceID
			 */
			ENVIRONS_LIB_API void SetDeviceID ( int deviceID );
        
        
			/**
			 * Get the device id that the application has assigned to the instance of Environs.
			 *
			 * @return	deviceID
			 */
			ENVIRONS_LIB_API int GetDeviceID ();
        
        
			/**
			 * Request a device id from mediator server instances that have been provided before this call.
			 * Prior to this call, the area and application name must be provided as well,
			 * in order to get an available device id for the specified application environment.
			 * If the application has already set a deviceID (using setDeviceID), this call returns the previously set value.
			 *
			 * @return	deviceID
			 */
			ENVIRONS_LIB_API int GetDeviceIDFromMediator ();
        
        
			/**
			 * Query whether the name of the current device has been set before.
			 *
			 * @return	has DeviceUID
			 */
			ENVIRONS_LIB_API bool HasDeviceUID ();
        
        
			/**
			 * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
			 *
			 * @param 	name    A unique device identifier.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetDeviceUID ( const char * name );
        
        
			/**
			 * Query ip of custom Mediator.
			 *
			 * @return ip
			 */
			ENVIRONS_LIB_API unsigned int GetMediatorIPValue ();
        
        
			/**
			 * Query ip of custom Mediator.
			 *
			 * @return ip
			 */
			ENVIRONS_LIB_API const char * GetMediatorIP ();
        
        
			/**
			 * Query port of custom Mediator.
			 *
			 * @return port
			 */
			ENVIRONS_LIB_API int GetMediatorPort ();
        
        
			/**
			 * Determines whether to use Crypto Layer Security for Mediator connections.
			 * If a Mediator enforces CLS, then disabling this option will result in failure to connect to that Mediator.
			 *
			 * @param	enable
			 */
			ENVIRONS_LIB_API void SetUseCLSForMediator ( bool enable );
        
        
			/**
			 * Query whether to use Crypto Layer Security for Mediator connections.
			 *
			 * @return	enabled
			 */
			ENVIRONS_LIB_API bool GetUseCLSForMediator ();
        
        
			/**
			 * Determines whether to use Crypto Layer Security for device-to-device connections.
			 *
			 * @param	enable
			 */
			ENVIRONS_LIB_API void SetUseCLSForDevices ( bool enable );
        
        
			/**
			 * Query whether to use Crypto Layer Security for device-to-device connections.
			 *
			 * @return	enabled
			 */
			ENVIRONS_LIB_API bool GetUseCLSForDevices ();
        
        
			/**
			 * Determines whether to enforce Crypto Layer Security for device-to-device connections.
			 *
			 * @param	enable
			 */
			ENVIRONS_LIB_API void SetUseCLSForDevicesEnforce ( bool enable );
        
        
			/**
			 * Query whether to enforce Crypto Layer Security for device-to-device connections.
			 *
			 * @return	enabled
			 */
			ENVIRONS_LIB_API bool GetUseCLSForDevicesEnforce ();
        
        
			/**
			 * Enable Crypto Layer Security for all traffic (incl. those of interactive type) in device-to-device connections.
			 *
			 * @param	enable
			 */
			ENVIRONS_LIB_API void SetUseCLSForAllTraffic ( bool enable );
        
        
			/**
			 * Query whether all traffic (incl. those of interactive type) in device-to-device connections is encrypted.
			 *
			 * @return	enabled
			 */
			ENVIRONS_LIB_API bool GetUseCLSForAllTraffic ();
        
        
			/**
			 * Determines whether to use environs default Mediator predefined by framework developers or not.
			 *
			 * @param enable 	true = use the default Mediator
			 */
			ENVIRONS_LIB_API void SetUseDefaultMediator ( bool enable );
        
        
			/**
			 * Query whether to use given Mediator by setMediator()
			 *
			 * @return enabled
			 */
			ENVIRONS_LIB_API bool GetUseDefaultMediator ();
        
        
			/**
			 * Determines whether to use given Mediator by setMediator()
			 *
			 * @param enable 	true = enable, false = disable
			 */
			ENVIRONS_LIB_API void SetUseCustomMediator ( bool enable );
        
        
			/**
			 * Query whether to use given Mediator by setMediator()
			 *
			 * @return enabled
			 */
			ENVIRONS_LIB_API bool GetUseCustomMediator ();
        
        
			/**
			 * Set custom Mediator to use.
			 *
			 * @param ip
			 * @param port
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetMediator ( const char * ip, unsigned short port );

        
			/**
			 * Set the user name for authentication with a Mediator service.&nbsp;Usually the user's email address is used as the user name.
			 *
			 * @param 	name
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetMediatorUserName ( const char * name );
			ENVIRONS_LIB_API bool SetUserName ( const char * name );

        
			/**
			 * Query UserName used to authenticate with a Mediator.
			 *
			 * @return UserName
			 */
			ENVIRONS_LIB_API const char * GetMediatorUserName ();

        
			/**
			 * Enable or disable anonymous logon to the Mediator.
			 *
			 * @param 	enable A boolean that determines the target state.
			 */
            ENVIRONS_LIB_API void SetUseMediatorAnonymousLogon ( bool enable );
            
            
            /**
             * Get setting of anonymous logon to the Mediator.
             *
             * @return 	enabled A boolean that determines the target state.
             */
            ENVIRONS_LIB_API bool GetUseMediatorAnonymousLogon ();

        
			/**
			 * Set the user password for authentication with a Mediator service.&nbsp;The password is stored as a hashed token within Environs.
			 *
			 * @param 	password
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetMediatorPassword ( const char * password );
			ENVIRONS_LIB_API bool SetUserPassword ( const char * password );
        
        
			/**
			 * Enable or disable authentication with the Mediator using username/password.
			 *
			 * @param 	enable
			 */
			ENVIRONS_LIB_API void SetUseAuthentication ( bool enable );
        
        
			/**
			 * Query the filter level for device management within Environs.
			 *
			 * return   level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_AREA, Environs.MEDIATOR_FILTER_AREA_AND_APP
			 */
			ENVIRONS_LIB_API int GetMediatorFilterLevel ();

        
			/**
			 * Set the filter level for device management within Environs.
			 *
			 * @param   level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_AREA, Environs.MEDIATOR_FILTER_AREA_AND_APP
			 */
			ENVIRONS_LIB_API void SetMediatorFilterLevel ( int level );
        
        
			/**
			 * Retrieve a boolean that determines whether Environs shows up a login dialog if a Mediator is used and no credentials are available.
			 *
			 * @return	true = yes, false = no
			 */
			ENVIRONS_LIB_API bool GetUseMediatorLoginDialog ();
        
        
			/**
			 * Instruct Environs to show up a login dialog if a Mediator is used and no credentials are available.
			 *
			 * @param   enable      true = enable, false = disable
			 */
			ENVIRONS_LIB_API void SetUseMediatorLoginDialog ( bool enable );
        
        
			/**
			 * Retrieve a boolean that determines whether Environs disable Mediator settings on dismiss of the login dialog.
			 *
			 * @return	true = yes, false = no
			 */
			ENVIRONS_LIB_API bool GetMediatorLoginDialogDismissDisable ();


			/**
			* Instruct Environs disable Mediator settings on dismiss of the login dialog.
			*
			 * @param   enable      true = enable, false = disable
			*/
			ENVIRONS_LIB_API void SetMediatorLoginDialogDismissDisable ( bool enable );
        
        
			/**
			 * Register at known Mediator server instances.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool RegisterAtMediators ();
        

			/**
			 * Start Environs.&nbsp;This is a non-blocking call and returns immediately.&nbsp;
			 * 		Since starting Environs includes starting threads and activities that may take longer,&nbsp;
			 * 		this call executes the start tasks within a thread.&nbsp;
			 * 		In order to get the status, catch the onNotify handler of your EnvironsListener.
			 *
			 */
			ENVIRONS_LIB_API void Start ();
        
        
			/**
			 * Query the status of Environs.&nsbp;Valid values are Types.NATIVE_STATUS_*
			 *
			 * @return  Environs.NATIVE_STATUS_*
			 */
			ENVIRONS_LIB_API int GetStatus ();
        
        
			/**
			 * Stop Environs and release all acquired resources.
			 */
			ENVIRONS_LIB_API void Stop ();
        
        
			/**
			 * Stop Environs and release all acquired resources.
			 */
			ENVIRONS_LIB_API void Dispose ();
        
			/**
			 * Set the area name that the local instance of Environs shall use for identification within the environment.
			 * It must be set before creating the Environs instance.
			 *
			 * @param	name
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetAreaName ( const char * name );
        
			/**
			 * Get the area name that the local instance of Environs use for identification within the environment.
			 * It must be set before creating the Environs instance.
			 *
			 * @return	areaName
			 */
			ENVIRONS_LIB_API const char * GetAreaName ();
        
        
			/**
			 * Set the application name of that the local instance of Environs shall use for identification within the environment.
			 * It must be set before creating the Environs instance.
			 *
			 * @param	name
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API void SetApplication ( const char * name );
			ENVIRONS_LIB_API void SetApplicationName ( const char * name );
        
        
			/**
			 * Get the application name that the local instance of Environs use for identification within the environment.
			 * It must be set before creating the Environs instance.
			 *
			 * @return	appName
			 */
			ENVIRONS_LIB_API const char * GetApplicationName ();
        
        
			/**
			 * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
			 *
			 * @param 	deviceName
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetDeviceName ( const char * name );
        
        
			/**
			 * Use default encoder, decoder, capture, render modules.
			 *
			 * @return  success
			 */
			ENVIRONS_LIB_API bool SetUsePortalDefaultModules ();
        
        
			ENVIRONS_LIB_API void SetUseH264 ( bool enable );
			ENVIRONS_LIB_API bool GetUseH264 ();
        
        
			/**
			 * Determine whether to use  TCP for portal streaming (if not selectively set for a particular deviceID)
			 *
			 * @param   enable
			 */
			ENVIRONS_LIB_API void SetUsePortalTCP ( bool enable );
        
        
			/**
			 * Query whether to use TCP for portal streaming (UDP otherwise)
			 *
			 * @return enabled
			 */
			ENVIRONS_LIB_API bool GetUsePortalTCP ();
        
        
			/**
			 * Use encoder module with the name moduleName. (libEnv-Enc...).
			 *
			 * @param	name  The module name
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetUseEncoder ( const char * name );
        
        
			/**
			 * Use decoder module with the name moduleName. (libEnv-Dec...).
			 *
			 * @param	name  The module name
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetUseDecoder ( const char * name );
        
        
			/**
			 * Use capture module with the name moduleName. (libEnv-Cap...).
			 *
			 * @param	name	the name of the module
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetUseCapturer ( const char * name );
        
        
			/**
			 * Use render module with the name moduleName. (libEnv-Rend...).
			 *
			 * @param	name	the name of the module
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool SetUseRenderer ( const char * name );
        
			/**
			 * Enable or disable a touch recognizer module by name (libEnv-Rec...).
			 *
			 * @param	moduleName  The module name
			 * @param	enable      Enable or disable
			 *
			 * @return  success
			 */
			ENVIRONS_LIB_API bool SetUseTouchRecognizer ( const char * name, bool enable );
        
        
			ENVIRONS_LIB_API int SetUseTracker ( int async, const char * name );
        
			ENVIRONS_LIB_API int GetUseTracker ( const char * name );
        
			ENVIRONS_LIB_API bool DisposeTracker ( int async, const char * name );
        
			ENVIRONS_LIB_API bool PushTrackerCommand ( int async, int moduleIndex, int command );
        
        
			/**
			 * Add an observer for communication with Environs and devices within the environment.
			 *
			 * @param   observer Your implementation of EnvironsObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool AddObserver ( EnvironsObserver * observer );
        
			/**
			 * Remove an observer for communication with Environs and devices within the environment.
			 *
			 * @param   observer Your implementation of EnvironsObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool RemoveObserver ( EnvironsObserver * observer );
        
			/**
			 * Add an observer for receiving messages.
			 *
			 * @param   observer Your implementation of EnvironsMessageObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool AddObserverForMessages ( EnvironsMessageObserver * observer );
        
			/**
			 * Remove an observer for receiving messages.
			 *
			 * @param   observer Your implementation of EnvironsMessageObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool RemoveObserverForMessages ( EnvironsMessageObserver * observer );
        
			/**
			 * Add an observer for receiving data buffers and files.
			 *
			 * @param   observer Your implementation of EnvironsDataObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool AddObserverForData ( EnvironsDataObserver * observer );
        
			/**
			 * Remove an observer for receiving data buffers and files.
			 *
			 * @param   observer Your implementation of EnvironsDataObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool RemoveObserverForData ( EnvironsDataObserver * observer );
        
			/**
			 * Add an observer for receiving sensor data of all devices.
			 * Please note: This observer reports sensor data of all devices that are connected and send to us.
			 * It's highly recommend to attach an SensorObserver to a DeviceInstance to process device filtered sensor data.
			 *
			 * @param   observer Your implementation of EnvironsDataObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool AddObserverForSensorData ( EnvironsSensorDataObserver * observer );
        
			/**
			 * Remove an observer for receiving data buffers and files.
			 * Please note: This observer reports sensor data of all devices that are connected and send to us.
			 * It's highly recommend to attach an SensorObserver to a DeviceInstance to process device filtered sensor data.
			 *
			 * @param   observer Your implementation of EnvironsDataObserver.
			 *
			 * @return	success
			 */
			ENVIRONS_LIB_API bool RemoveObserverForSensorData ( EnvironsSensorDataObserver * observer );
            
            
            /**
             * Get a DeviceList object that manages all devices of given list type. This list ist updated dynamically by Environs.
             * After client code is done with the list, the list->Release () method MUST be called by the client code,
             * in order to release the resource (give ownership) back to Environs.
             *
             * @param MEDIATOR_DEVICE_CLASS_	A value of type MEDIATOR_DEVICE_CLASS_* that determines the list type
             
             * @return A DeviceList object
             */
			ENVIRONS_LIB_API DeviceList * GetDeviceListObj ( int MEDIATOR_DEVICE_CLASS_ );
            
            /**
             * Get a DeviceList object that manages all devices of given list type. This list ist updated dynamically by Environs.
             * After client code is done with the list, the list->Release () method MUST be called by the client code,
             * in order to release the resource (give ownership) back to Environs.
             *
             * @param MEDIATOR_DEVICE_CLASS_	A value of type MEDIATOR_DEVICE_CLASS_* that determines the list type
             
             * @return An interface of type IDeviceList
             */
			ENVIRONS_LIB_API IDeviceList * GetDeviceListRetainedI ( int MEDIATOR_DEVICE_CLASS_ );
            
            
            ENVIRONS_LIB_API bool GetPortalNativeResolution ();
            ENVIRONS_LIB_API void SetPortalNativeResolution ( bool enable );
            
            ENVIRONS_LIB_API bool GetPortalAutoStart ();
            ENVIRONS_LIB_API void SetPortalAutoStart ( bool enable );
            
            
            
            /**
             * Release ownership on this interface and mark it disposeable.
             * Release must be called once for each Interface that the Environs framework returns to client code.
             * Environs will dispose the underlying object if no more ownership is hold by anyone.
             *
             */
            ENVIRONS_OUTPUT_DE_ALLOC_DECL ();
            
        private:
            ENVIRONS_OUTPUT_ALLOC_RESOURCE ( Environs );
            
            bool                                            allocated;
            
            Instance                                    *   env;
            int                                             hEnvirons;
            
            void    SetInstance ( Instance * obj );
            
			spv ( IIEnvironsObserver * )             environsObservers;

            spv ( IIEnvironsMessageObserver * )      environsObserversForMessages;

            spv ( IIEnvironsDataObserver * )         environsObserversForData;

            spv ( IIEnvironsSensorDataObserver * )   environsObserversForSensorData;

            /**
             * BridgeForNotify static method to be called by Environs in order to notify about events,<br\>
             * 		such as when a connection has been established or closed.
             *
             * @param hInst			A handle to the Environs instance
             * @param nativeID      The native device id of the sender device.
             * @param notification  The received notification.
             * @param sourceIdent   A value of the enumeration type Types.EnvironsSource
             * @param context       A value that provides additional context information (if available).
             */
            static void BridgeForNotify ( int hInst, int nativeID, int notification, int sourceIdent, void * contextPtr );
            
            /**
             * BridgeForNotify static method to be called by Environs in order to notify about events,<br\>
             * 		such as when a connection has been established or closed.
             *
             * @param hInst			A handle to the Environs instance
             * @param deviceID      The device id of the sender device.
             * @param areaName		Area name of the application environment.
             * @param appName		Application name of the application environment.
             * @param notification  The received notification.
             * @param sourceIdent   A value of the enumeration type Types.EnvironsSource
             * @param context       A value that provides additional context information (if available).
             */
            static void BridgeForNotifyExt ( int hInst, int deviceID, const char * areaName, const char * appName, int notification, int sourceIdent, void * contextPtr );
            
            /**
             * BridgeForMessage static method to be called by native layer in order to notify about incoming messages.
             *
             * @param hInst			A handle to the Environs instance
             * @param deviceID      The device id of the sender device.
             * @param areaName		Area name of the application environment
             * @param appName		Application name of the application environment
             * @param type          The type of this message.
             * @param msg           The message.
             * @param length        The length of the message.
             */
            static void BridgeForMessageExt ( int hInst, int deviceID, const char * areaName, const char * appName, int type, const void * message, int length );
            
            /**
             * BridgeForMessage static method to be called by native layer in order to notify about incoming messages.
             *
             * @param hInst			A handle to the Environs instance
             * @param nativeID      The native device id of the sender device.
             * @param type          The type of this message.
             * @param msg           The message.
             * @param length        The length of the message.
             */
            static void BridgeForMessage ( int hInst, int nativeID, int type, const void * message, int length );
            
            /**
             * BridgeForStatusMessage static method to be called by native layer in order to drop a status messages.
             *
             * @param hInst			A handle to the Environs instance
             * @param msg           A status message of Environs.
             */
            static void BridgeForStatusMessage ( int hInst, const char * message );
            
            /**
             * BridgeForData static method to be called by native layer in order to notify about data received from a device.
             *
             * @param hInst			A handle to the Environs instance
             * @param nativeID      The native device id of the sender device.
             * @param type          The type of this message.
             * @param fileID        A fileID that was attached to the buffer.
             * @param descriptor    A descriptor that was attached to the buffer.
             * @param size          The size of the data buffer.
             */
            static void BridgeForData ( int hInst, int nativeID, int type, int fileID, const char * descriptor, int size );
            static void BridgeForSensorData ( int hInst, int nativeID, SensorFrame * pack );
            
            
            pthread_mutex_t             queryLock;
            pthread_mutex_t             listLock;
            pthread_mutex_t             threadDeviceCommandMutex;
            
            std::queue<DeviceCommandContext *> threadDeviceCommandQueue;
            
            long                        listAllUpdate;
            long                        listNearbyUpdate;
            long                        listMediatorUpdate;
            
            pthread_mutex_t             listAllLock;
            svsp ( DeviceInstance )     listAll;
            spv ( IIListObserver * )    listAllObservers;
            
            pthread_mutex_t             listNearbyLock;
            svsp ( DeviceInstance )     listNearby;
            spv ( IIListObserver * )    listNearbyObservers;
            
            pthread_mutex_t             listMediatorLock;
            svsp ( DeviceInstance )     listMediator;
            spv ( IIListObserver * )    listMediatorObservers;
            
            
            void DisposeLists ();
            
            void RefreshDeviceLists ();
            
            const svsp ( DeviceInstance ) & GetDevices ( int type );
            
            const svsp ( DeviceInstance ) & GetDevicesBest ();
            
            
            /**
             * Query a DeviceInstance object from all available devices within the environment (including those of the Mediator).
             *
             * @param nativeID      The device id of the target device.
             * @return DeviceInstance-object
             */
            sp ( DeviceInstance ) GetDeviceAllSP ( int nativeID );
            
            sp ( DeviceInstance ) GetDeviceAllSP ( int nativeOrDeviceID, bool isNativeID );
            
            
            /**
             * Query a DeviceInstance object of nearby (broadcast visible) devices within the environment.
             *
             * @param nativeID      The device id of the target device.
             * @return DeviceInstance-object
             */
            sp ( DeviceInstance ) GetDeviceNearbySP ( int nativeID );
            
            
            /**
             * Get a collection that holds the nearby devices. This list ist updated dynamically by Environs.
             *
             * @return ArrayList with DeviceInstance objects
             */
            svsp ( DeviceInstance ) GetDevicesNearbySP ();
            
            
            /**
             * Query a DeviceInstance object of Mediator managed devices within the environment.
             *
             * @param nativeID      The device id of the target device.
             * @return DeviceInstance-object
             */
            sp ( DeviceInstance ) GetDeviceFromMediatorSP ( int nativeID );
            
            
            /**
             * Get a collection that holds the Mediator server devices. This list ist updated dynamically by Environs.
             *
             * @return ArrayList with DeviceInstance objects
             */
            svsp ( DeviceInstance ) GetDevicesFromMediatorSP ();
            
            static void NotifyListObservers ( int hInst, const spv ( IIListObserver * ) &observerList, svsp ( DeviceInstance ) vanished, svsp ( DeviceInstance ) appeared, bool enqueue );
            
            
            void * DeviceListUpdateThread ( void * pack );
            
            static void * DeviceListUpdateThreadStarter ( void * pack );
            
            void OnDeviceListUpdate ();
            
            void OnDeviceListNotification ( NotifyContext * ctx );
            
            void OnDeviceListNotification1 ( const sp ( DeviceInstance ) &device, NotifyContext * ctx );
            
            void DeviceListUpdate ( int listType );
            
            
            void UpdateConnectProgress ( int nativeID, int progress );
            
            void UpdateMessage ( ObserverMessageContext * ctx );
            
            void UpdateData ( ObserverDataContext * ctx );
            
            void UpdateSensorData ( int nativeID, environs::SensorFrame * pack );
            
		};
	}
}
#endif


#endif






















