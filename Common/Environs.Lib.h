/**
 * Environs Native Layer API exposed by the libraries
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
#ifndef INCLUDE_HCM_ENVIRONS_NATIVE_API_H
#define INCLUDE_HCM_ENVIRONS_NATIVE_API_H

#include "Environs.Lib.Inc.h"

#include "Environs.Lib.Display.h"
#include "Environs.Lib.Mobile.h"

namespace environs 
{

	bool LoadConfig ( Instance * env );
	bool SaveConfig ();

	namespace API 
	{
		void UpdateNetworkStatus ();
        void SetDeviceID ( int hInst, int deviceID );

		/**
		 *	API exports that are used for every platform
		 *
         * */
		ENVIRONSAPI LIBEXPORT void CallConv SetCallbacks ( int hInst, void * HumanInputCallback, void * SensorInputCallback, void * MessageCallback, void * MessageExtCallback, void * NotifyCallback, void * NotifyExtCallback, void * DataCallback, void * StatusMessageCallback );

		ENVIRONSAPI LIBEXPORT EBOOL CallConv GetPortalInfo1 ( int hInst, void * portalInfo );
		ENVIRONSAPI LIBEXPORT EBOOL CallConv SetPortalInfo1 ( int hInst, void * portalInfo );
		ENVIRONSAPI LIBEXPORT void CallConv SetDeviceID2 ( int hInst, int deviceID );

		/**
		* Query whether the native layer was build for release (or debug).
		*
		* @return	true = Release build, false = Debug build.
		*/
		ENVIRONSAPI LIBEXPORT EBOOL CallConv GetIsReleaseBuild ();
                
/**
*	API exports for all platforms but Android
*
* */
#if !defined(ANDROID)
#include "Interop/jni.h"

#ifdef __cplusplus
		extern "C"
		{
#endif
			LIBEXPORT const char * CallConv GetVersionString ();
			LIBEXPORT int CallConv			GetVersionMajor ();
			LIBEXPORT int CallConv			GetVersionMinor ();
			LIBEXPORT int CallConv			GetVersionRevision ();

			LIBEXPORT int CallConv			GetPlatform1 ();
			LIBEXPORT void CallConv			SetPlatform1 ( int platform );
            
            
            /**
             * Create a native Environs object and return a handle to the object.
             * A return value of 0 means Error
             *
             * @return   An instance handle that refers to the created Environs object
             */
            LIBEXPORT int CallConv			CreateEnvirons1 ();
            
            
            /**
             * Load settings for the given application environment
             *
             * @param	hInst		The handle to a particular native Environs instance.
             * @param 	appName		The application name for the application environment.
             * @param  	areaName	The area name for the application environment.
             *
             * @return   success
             */
            LIBEXPORT EBOOL CallConv        LoadSettings1 ( int hInst, const char * app, const char * area );
            

			LIBEXPORT void CallConv			RegisterMainThread ( int hInst );
			LIBEXPORT void CallConv			SetDebug ( int mode );
			LIBEXPORT void CallConv			SetOSLevel ( int level );
            LIBEXPORT void CallConv			ResetIdentKeys ();
            
            LIBEXPORT int CallConv			GetAppAreaID1 ( int hInst );
            
			LIBEXPORT void CallConv			SetDeviceID1 ( int hInst, int myDeviceID );
			LIBEXPORT int CallConv			GetDeviceID ( int hInst );
            
			LIBEXPORT int CallConv			GetDeviceIDFromMediator1 ( int hInst );
            
            
			LIBEXPORT int CallConv			GetMediatorFilterLevel1 ( int hInst );
			LIBEXPORT void CallConv			SetMediatorFilterLevel1 ( int hInst, int level );
			LIBEXPORT int CallConv			GetIPAddress ( );
			LIBEXPORT int CallConv			GetSubnetMask ( );
			LIBEXPORT void CallConv			SetDeviceDims1 ( int width, int height, int width_mm, int height_mm, int leftpos, int toppos );

			LIBEXPORT EBOOL CallConv		SetPorts1 ( int hInst, int tcpPort, int udpPort );
			LIBEXPORT void CallConv			SetDeviceType ( char type );
			LIBEXPORT char CallConv			GetDeviceType ( );

			/**
			* Query whether the name of the current device has been set before.
			*
			* @return	has DeviceUID
			*/
            LIBEXPORT EBOOL CallConv		HasDeviceUID ( );

			/**
			* Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
			*
			* @param 	deviceUID A unique identifier to identify this device.
			* @return	success
			*/
            LIBEXPORT EBOOL CallConv		SetDeviceUID ( const char * name );

			/**
			* Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
			*
			* @param 	deviceName  The device name.
			* @return	success
			*/
			LIBEXPORT EBOOL CallConv		SetDeviceName1 ( const char * name );
            
			LIBEXPORT EBOOL CallConv		SetUseTouchRecognizer1 ( int hInst, const char * name, bool enable );
            
			LIBEXPORT EBOOL CallConv		SetAreaName1 ( int hInst, const char * name );
			LIBEXPORT const char * CallConv	GetAreaName1 ( int hInst );
            
			LIBEXPORT EBOOL CallConv		SetApplicationName1 ( int hInst, const char * name );
			LIBEXPORT const char * CallConv	GetApplicationName1 ( int hInst );
            
			LIBEXPORT EBOOL CallConv		SetGCMAPIKey ( const char * key );

			LIBEXPORT int CallConv			Init1 ( int hInst );
			LIBEXPORT void CallConv			SetAppStatus1 ( int hInst, int status );
			LIBEXPORT int CallConv			SetMainAppWindow1 ( int hInst, WNDHANDLE hWnd );
            
            LIBEXPORT EBOOL CallConv		SetPortalSourceWindow1 ( WNDHANDLE hWnd, int nativeID );
			LIBEXPORT void CallConv			UpdateAppWindowSize1 ( int hInst );
			/**
			* Instruct Environs native layer to prepare required portal resources to base on generation within the platform layer.
			*
			* @param enable      true = enable, false = disable
			*/
			LIBEXPORT void CallConv			SetUsePlatformPortalGenerator1 ( int hInst, EBOOL enabled );

			LIBEXPORT EBOOL CallConv		GetUseOpenCL1 ( int hInst );
			LIBEXPORT void CallConv			SetUseOpenCL1 ( int hInst, EBOOL enable );

			LIBEXPORT EBOOL CallConv		GetUseStream1 ( int hInst );
			LIBEXPORT void CallConv			SetUseStream1 ( int hInst, EBOOL enable );

			LIBEXPORT void CallConv			SetUseMouseEmulation1 ( int hInst, EBOOL enable );
			LIBEXPORT EBOOL CallConv		GetUseMouseEmulation1 ( int hInst );

			LIBEXPORT void CallConv			SetUseTouchVisualization1 ( int hInst, EBOOL enable );
			LIBEXPORT EBOOL CallConv		GetUseTouchVisualization1 ( int hInst );
            
            LIBEXPORT int CallConv			GetDeviceConnectStatus1 ( int hInst, int nativeID );
            LIBEXPORT EBOOL CallConv		GetDirectContactStatus1 ( int hInst, int nativeID );
            
            /**
             * Get the status, whether the device (id) has established an active portal
             *
             * @param 	deviceID    The device id of the target device.
             * @param 	areaName	Area name of the application environment
             * @param 	appName		Application name of the application environment
             * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
             * @return	success 	true = yes, false = no
             */
            LIBEXPORT EBOOL CallConv		GetPortalEnabled1 ( int hInst, int nativeID, int portalType );

            /**
             * Get the portalID of the first active portal
             *
             * @param 	deviceID    The device id of the target device.
             * @param 	areaName	Area name of the application environment
             * @param 	appName		Application name of the application environment
             * @param	portalType  0 = Any type, or PORTAL_DIR_INCOMING, PORTAL_DIR_OUTGOING
             * @return	portalID 	The portal ID.
             */
            LIBEXPORT int CallConv          GetPortalId1 ( int hInst, int nativeID, int portalType );
            
            LIBEXPORT int CallConv			DeviceDetected1 ( int hInst, int deviceID, const char * areaName, const char * appName, int Environs_CALL_, int x, int y, float angle );
            
            LIBEXPORT int CallConv			DeviceConnect1 ( int hInst, int deviceID, const char * areaName, const char * appName, int async );
            LIBEXPORT EBOOL CallConv		DeviceDisconnect1 ( int hInst, int nativeID, int async );
            
            LIBEXPORT void CallConv			DeviceUpdated1 ( int hInst, int nativeID, int async, int x, int y, float angle );
            LIBEXPORT void CallConv			DevicePositionUpdated1 ( int hInst, int nativeID, int async, int x, int y );
            LIBEXPORT void CallConv			DeviceAngleUpdated1 ( int hInst, int nativeID, int async, float angle );
            LIBEXPORT void CallConv			DeviceRemoved1 ( int hInst, int nativeID, int async, int x, int y, float angle );
            LIBEXPORT void CallConv			DeviceRemovedID1 ( int hInst, int nativeID, int async );
				
			LIBEXPORT int CallConv			GetStatus1 ( int hInst );
			LIBEXPORT int CallConv			Start1 ( int hInst );
			LIBEXPORT int CallConv			Stop1 ( int hInst );

			/**
			* Dispose/Release a native Environs object with the given object handle.
			*/
			LIBEXPORT void CallConv			Dispose1 ( int hInst );

			LIBEXPORT void CallConv			SetStreamOverUdp1 ( int hInst );
			LIBEXPORT void CallConv			SetStreamOverTcp1 ( int hInst );
			LIBEXPORT EBOOL CallConv		StreamToggleQuality1 ( int hInst );
			LIBEXPORT void CallConv			SetStreamJpegs1 ( int hInst );
			LIBEXPORT void CallConv			SetStreamPngs1 ( int hInst );
            
            LIBEXPORT EBOOL CallConv		SendBuffer1 ( int hInst, int nativeID, int async, int fileID, const char * fileDescriptor, char * buffer,  int size );
            LIBEXPORT EBOOL CallConv		SendFile1 ( int hInst, int nativeID, int async, int fileID, const TCHAR * fileDescriptor, const TCHAR * file );

            LIBEXPORT EBOOL CallConv		SendMessage1 ( int hInst, int deviceID, const char * areaName, const char * appName, int async, const char * message, int length );
            
			LIBEXPORT EBOOL CallConv		SendPushNotification1 ( int hInst, int deviceID, const char * areaName, const char * appName, const char * message );

			LIBEXPORT EBOOL CallConv		RegisterAtMediators1 ( int hInst );

			LIBEXPORT EBOOL CallConv		SetMediator1 ( int hInst, const char * ip, int port );
            LIBEXPORT const char * CallConv GetMediatorIP1 ( int hInst );
            LIBEXPORT int CallConv			GetMediatorPort1 ( int hInst );
            
            /**
             * Enable or disable anonymous logon to the Mediator.
             *
             * @param 	enable A boolean that determines the target state.
             */
            LIBEXPORT void CallConv			SetUseMediatorAnonymousLogon1 ( int hInst, EBOOL usage );
            
            /**
             * Get setting of anonymous logon to the Mediator.
             *
             * @param 	enable A boolean that determines the target state.
             */
            LIBEXPORT EBOOL CallConv		GetUseMediatorAnonymousLogon1 ( int hInst );
            
			/**
			* Set the user name for authentication with a mediator service.&nbsp;Usually the user's email address is used as the user name.
			*
			* @param 	username    The user name for authentication at the Mediator.
			* @return	success
			*/
			LIBEXPORT EBOOL CallConv		SetMediatorUserName1 ( int hInst, const char * name );

			/**
			* Set the user password for authentication with a Mediator service.&nbsp;The password is stored as a hashed token within Environs.
			*
			* @param 	password    The mediator password.
			* @return	success
			*/
			LIBEXPORT EBOOL CallConv		SetMediatorPassword1 ( int hInst, const char * name );

			/**
			* Enable or disable authentication with the Mediator using username/password.
			*
			* @param 	enable A boolean that determines the target state.
			*/
			LIBEXPORT void CallConv			SetUseAuthentication1 ( int hInst, EBOOL usage );

			/**
			* Query UserName used to authenticate with a Mediator.
			*
			* @return UserName The user name for authentication at the Mediator.
			*/
			LIBEXPORT const char * CallConv GetMediatorUserName1 ( int hInst );

			LIBEXPORT void CallConv			SetUseDefaultMediator1 ( int hInst, EBOOL usage );
			LIBEXPORT EBOOL CallConv		GetUseDefaultMediator1 ( int hInst );
            
			LIBEXPORT void CallConv			SetUseCustomMediator1 ( int hInst, EBOOL usage );
			LIBEXPORT EBOOL CallConv		GetUseCustomMediator1 ( int hInst );
            
            LIBEXPORT void CallConv			SetUseMediatorLoginDialog1 ( int hInst, EBOOL usage );
            LIBEXPORT EBOOL CallConv		GetUseMediatorLoginDialog1 ( int hInst );
            
            LIBEXPORT void CallConv			SetMediatorLoginDialogDismissDisable1 ( int hInst, EBOOL usage );
            LIBEXPORT EBOOL CallConv		GetMediatorLoginDialogDismissDisable1 ( int hInst );

			LIBEXPORT void CallConv			SetUseCLSForMediator1 ( int hInst, EBOOL usage );
			LIBEXPORT EBOOL CallConv		GetUseCLSForMediator1 ( int hInst );

			LIBEXPORT void CallConv			SetUseCLSForDevices1 ( int hInst, EBOOL usage );
			LIBEXPORT EBOOL CallConv		GetUseCLSForDevices1 ( int hInst );

			LIBEXPORT void CallConv			SetUseCLSForDevicesEnforce1 ( int hInst, EBOOL usage );
			LIBEXPORT EBOOL CallConv		GetUseCLSForDevicesEnforce1 ( int hInst );

			LIBEXPORT void CallConv			SetUseCLSForAllTraffic1 ( int hInst, EBOOL usage );
            LIBEXPORT EBOOL CallConv		GetUseCLSForAllTraffic1 ( int hInst );
            
            LIBEXPORT void CallConv			SetUseNotifyDebugMessage ( EBOOL usage );
            LIBEXPORT EBOOL CallConv		GetUseNotifyDebugMessage ( );
            
            LIBEXPORT void CallConv			SetUseLogFile1 ( EBOOL usage );
            LIBEXPORT EBOOL CallConv		GetUseLogFile1 ( );
            
			LIBEXPORT void CallConv			SetNetworkStatus ( int netStat );
			LIBEXPORT int CallConv			GetNetworkStatus ( );
            
            LIBEXPORT EBOOL CallConv        ApplyModuleToSettings ( int hInst, const char * moduleName, char ** target, int environs_InterfaceType, bool testInstance = true );

			/**
			* Use default encoder, decoder, capture, render modules.
			*
			* @return  success
			*/
			LIBEXPORT EBOOL CallConv		SetUsePortalDefaultModules1 ( int hInst );

			/**
			* Use encoder module with the name moduleName. (libEnv-Enc...).
			*
			* @param	moduleName	the name of the module
			* @return  success
			*/
			LIBEXPORT EBOOL CallConv		SetUseEncoder1 ( int hInst, const char * moduleName );

			/**
			* Use decoder module with the name moduleName. (libEnv-Dec...).
			*
			* @param	moduleName	the name of the module
			* @return  success
			*/
			LIBEXPORT EBOOL CallConv		SetUseDecoder1 ( int hInst, const char * moduleName );

			/**
			* Use render module with the name moduleName. (libEnv-Rend...).
			*
			* @param	moduleName	the name of the module
			* @return  success
			*/
			LIBEXPORT EBOOL CallConv		SetUseRenderer1 ( int hInst, const char * moduleName );

			/**
			* Use capture module with the name moduleName. (libEnv-Cap...).
			*
			* @param	moduleName	the name of the module
			* @return  success
			*/
			LIBEXPORT EBOOL CallConv		SetUseCapturer1 ( int hInst, const char * moduleName );

            LIBEXPORT void * CallConv		GetFileNative ( int hInst, int nativeID, int fileID, void * buffer, int * capacity );
            LIBEXPORT void * CallConv		GetFile ( int hInst, int deviceID, const char * areaName, const char * appName, int fileID, void * buffer, int * capacity );
            
            /**
             * Query the absolute path name that contains the file belonging to the fileID and received from the deviceID.
             *  The resulting memory is managed by Environs.
             *
             * @param    nativeID
             * @param    fileID
             *
             * @return
             */
            LIBEXPORT jstring CallConv     GetFilePathNative ( int hInst, int nativeID, int fileID );
            
            /**
             * Query the absolute path name that contains the file belonging to the fileID and received from the deviceID.
             *  The resulting memory is managed by Environs.
             *
             * @param   deviceID
             * @param   areaName
             * @param   appName
             * @param   fileID
             *
             * @return
             */
			LIBEXPORT jstring CallConv     GetFilePath ( int hInst, int deviceID, const char * areaName, const char * appName, int fileID );

            /**
             * Query the absolute path name to the storage of a given device identity.
             *  The memory MUST BE FREED by the callee.
             *
             * @param    deviceID
             * @param    areaName
             * @param    appName
             *
             * @return
             */
			LIBEXPORT char * CallConv		GetFilePathForStorage1 ( int hInst, int deviceID, const char * areaName, const char * appName );
            
			LIBEXPORT void CallConv			ReleaseString ( void * ptr );


			LIBEXPORT EBOOL CallConv		GetPortalInfo ( int hInst, void * buffer );
			LIBEXPORT EBOOL CallConv		SetPortalInfo ( int hInst, void * buffer );
            
            LIBEXPORT EBOOL CallConv		SetPortalOverlayARGB ( int nativeID, int portalID, int layerID, int left, int top,
                                                                  int width, int height, int stride, void * renderData, int alpha, bool positionDevice );
			LIBEXPORT void CallConv			FreeNativeMemory ( void * obj );

			LIBEXPORT void * CallConv		GetDevices1 ( int hInst, int listType );
            LIBEXPORT void * CallConv		GetDevice1 ( int hInst, int deviceID, const char * areaName, const char * appName, int fromType );
            LIBEXPORT void * CallConv		GetDeviceForPortal1 ( int hInst, int portalID );
            
            LIBEXPORT int CallConv			GetDevicesCount1 ( int hInst, int fromType );

            LIBEXPORT void * CallConv		GetDeviceDisplayProps1 ( int hInst, int nativeID );

			LIBEXPORT void * CallConv		GetDeviceBestMatch1 ( int hInst, int deviceID );

			LIBEXPORT int CallConv			GetConnectedDevicesCount1 ( int hInst );

			LIBEXPORT int CallConv			GetDeviceInstanceSize ();
			LIBEXPORT int CallConv			GetDevicesHeaderSize ();

			//LIBEXPORT void CallConv			deallocNative ();
			LIBEXPORT void CallConv			InitStorage ( const char * path );
            LIBEXPORT void CallConv			InitLibDir ( const char * path );
            LIBEXPORT void CallConv			InitWorkDir ( const char * path );
            LIBEXPORT void CallConv			SetGCM ( const char * gcm );

            LIBEXPORT int CallConv			GetDeviceWidth ( int nativeID );
            LIBEXPORT int CallConv			GetDeviceHeight ( int nativeID );


			/**
			* Request a portal stream from the device with the given id.&nbsp;The device must be connected before with deviceConnect ().
			*
			* @param 	nativeID		The native id of the target device.
			* @param 	async			Execute asynchronous. Non-async means that this call blocks until the command has finished.
			* @param 	portalDetails	An application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
			* @param 	width       	The width of the portal that we request. 0 = portalsource determined. It is not guaranteed, that the requested resolution will be provided.
			*                       	e.g. if the portalsource is a camera, then only some predefined resolutions can be provided.
			* @param 	height      	The height of the portal that we request. 0 = portalsource determined. It is not guaranteed, that the requested resolution will be provided.
			*                       	e.g. if the portalsource is a camera, then only some predefined resolutions can be provided.
			*
			* @return 	success
			*/
			LIBEXPORT EBOOL CallConv		RequestPortalStream1 ( int hInst, int nativeID, int async, int portalDetails, int width, int height );
			

			/**
			* Provide a portal stream to the device with the given id.&nbsp;
			* The device must be in a connected state by means of prior call to deviceConnect ().
			*
			* @param 	nativeID		The native id of the target device.
			* @param 	async			Execute asynchronous. Non-async means that this call blocks until the command has finished.
			* @param 	portalDetails	Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)	 * @return 	success
			*
			* @return success
			*/
            LIBEXPORT EBOOL CallConv		ProvidePortalStream1 ( int hInst, int nativeID, int async, int portalDetails );
            
            
            /**
             * Provide a portal stream to be requested by the device with the given id.&nbsp;
             * The device must be in a connected state by means of prior call to deviceConnect ().
             *
             * @param 	nativeID		The native id of the target device.
             * @param 	async			Execute asynchronous. Non-async means that this call blocks until the command has finished.
             * @param 	portalDetails	Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)	 * @return 	success
             *
             * @return success
             */
            LIBEXPORT EBOOL CallConv		ProvideRequestPortalStream1 ( int hInst, int nativeID, int async, int portalDetails );


			/**
			* Find a free portalID slot for the direction encoded into the given portalDetails.
			*
			* @param 	nativeID    	The native device id of the target device.
			* @param	portalDetails	Required PORTAL_DIR_INCOMING or PORTAL_DIR_OUTGOING
			* @return	portalID 		The portal ID with the free id slot encoded in bits 0xFF.
			*/
			LIBEXPORT int CallConv			GetPortalIDFreeSlot1 ( int hInst, int nativeID, int portalDetails );

            /**
             * Start streaming of portal to or from the portal identifier (received in notification).
             *
             * @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
             * @param 	portalID	An application specific id (e.g. used for distinguishing front facing or back facing camera)
             * @return 	success
             */
			LIBEXPORT EBOOL CallConv		StartPortalStream1 ( int hInst, int async, int portalID );
            
            /**
             * Pause streaming of portal to or from the portal identifier (received in notification).
             *
             * @param 	async       Execute asynchronous. Non-async means that this call blocks until the command has finished.
             * @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
             * 						It is provided within the notification listener as sourceIdent.&nbsp;
             * 						Applications should store them in order to address the correct portal within Environs.
             * @return success
             */
			LIBEXPORT EBOOL CallConv		PausePortalStream1 ( int hInst, int async, int portalID );
            
            /**
             * Stop streaming of portal to or from the portal identifier (received in notification).
             *
             * @param async      Execute asynchronous. Non-async means that this call blocks until the command has finished.
             * @param nativeID   The native device id of the target device.
             * @param portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
             * 					It is provided within the notification listener as sourceIdent.&nbsp;
             * 					Applications should store them in order to address the correct portal within Environs.
             * @return success
             */
			LIBEXPORT EBOOL CallConv		StopPortalStream1 ( int hInst, int async, int nativeID, int portalID );
            
            /**
             * Acquire a native layer send identifier (and increase instanceLock on device) to be used in sendTcpPortal. This resource must be released on disposal of the portal generator.
             *
             * @param deviceID      The device id of the target device.
             * @param areaName		Area name of the application environment
             * @param appName		Application name of the application environment
             * @param portalUnitType	e.g. MSG_TYPE_STREAM
             * @param portalID    Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
             * @return sendID
             */
            LIBEXPORT int CallConv          AcquirePortalSendID ( int portalUnitType, int portalID );
            
            /**
             * Release a native layer send identifier that was acquired by a call to acquireTcpPortalSendID.
             *
             * @param sendID      The portal send id resource to be released.
             */
            LIBEXPORT void CallConv         ReleasePortalSendID ( int sendID );
            
            /**
             * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
             *
             * @param sendID      		The device id of the target device.
             * @param portalUnitFlags	Flags that will be or-ed with the portalUnitType
             * @param prefixBuffer    	A prefix that prepend the actual buffer.
             * @param prefixSize   		The size of the content within the prefixbuffer.
             * @param buffer        	A buffer to be send.
             * @param offset        	An offset into the buffer.
             * @param contentSize   	The size of the content within the buffer.
             * @return success
             */
            LIBEXPORT EBOOL CallConv		SendTcpPortal ( int sendID, int portalUnitFlags, jobject prefixBuffer, int prefixSize, jobject byteBuffer, int offset, int contentSize );
            
			LIBEXPORT EBOOL CallConv		SetRenderSurface1 ( int hInst, int portalID, void * renderSurface, int width, int height );
			LIBEXPORT EBOOL CallConv		ReleaseRenderSurface1 ( int hInst, int async, int portalID );
			LIBEXPORT EBOOL CallConv		SetRenderCallback ( int hInst, int async, int portalID, void * callback, int type );
			
			/**
			* Determine whether Environs shall automatically adapt the layout dimensions of
			* the View provided for the portal with the deviceID.
			* The layout dimensions are in particular important for proper mapping of TouchDispatch contact points
			* on the remote portal.
			* If enable is set to false, then custom applications must adapt the layout parameters
			* by means of calling SetPortalViewDims1().
			*
			* @param enable    A boolean that determines the target state.
			*/
			LIBEXPORT void CallConv			SetPortalViewDimsAuto1 ( int hInst, EBOOL enable );

			/**
			* Query the option whether Environs adapts the portal according to the size/location
			* of its view within the layout.
			*
			* @return enabled
			*/
			LIBEXPORT EBOOL CallConv		GetPortalViewDimsAuto1 ( int hInst );

			/**
			* Set the location (and its size) of the portal that belongs to the nativeID.
			* Such values are usually provided within the onLayoutChangeListener of a View.
			*
			* @param portalID      The portal device id of the target device.
			* @param left          The left coordinate
			* @param top           The top coordinate
			* @param right         The right coordinate
			* @param bottom        The bottom coordinate
			*
			* @return success		This call will fail, if the touchsource (and portal resources) have not been initialized.
			*/
			LIBEXPORT EBOOL CallConv		SetPortalViewDims1 ( int hInst, int portalID, int left, int top, int right, int bottom );
            
            LIBEXPORT int CallConv			SetUseTracker ( int hInst, int async, const char * moduleName );
            LIBEXPORT int CallConv			GetUseTracker ( int hInst, const char * moduleName );
            LIBEXPORT EBOOL CallConv		DisposeTracker ( int hInst, int async, const char * moduleName );
            
            LIBEXPORT EBOOL CallConv		GetTrackerEnabled ( int hInst, int index );
            LIBEXPORT EBOOL CallConv		SetTrackerParams ( int hInst, int index, int channels, int width, int height, int stride );
            LIBEXPORT EBOOL CallConv		SetTrackerImage ( int hInst, int index, void * rawImage, int size );
            LIBEXPORT EBOOL CallConv		PushTrackerCommand ( int hInst, int async, int index, int command );

            
            
            LIBEXPORT EBOOL CallConv		GetPortalNativeResolution1 ( int hInst );
            LIBEXPORT void CallConv			SetPortalNativeResolution1 ( int hInst, EBOOL enable );
            
            LIBEXPORT EBOOL CallConv		GetPortalAutoStart1 ( int hInst );
            LIBEXPORT void CallConv			SetPortalAutoStart1 ( int hInst, EBOOL enable );
#ifdef __cplusplus
		}
#endif

#endif // ANDROID - include only the jni-exports

	}
}

#endif	// INCLUDE_HCM_ENVIRONS_NATIVE_API_H



