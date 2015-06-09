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

#include "Environs.lib.inc.h"

#include "Environs.lib.display.h"
#include "Environs.lib.mobile.h"

namespace environs 
{
	namespace API 
	{
		extern void UpdateNetworkStatus ();
        extern void setDeviceID ( int deviceID );

		/**
		 *	API exports that are used for every platform
		 *
		 * */
		ENVIRONSAPI LIBEXPORT void CallConv setCallbacks ( long TouchCallback, long MessageCallback, long NotifyCallback, long DataCallback, long StatusMessageCallback );

		ENVIRONSAPI LIBEXPORT EBOOL CallConv getPortalInfo1 ( void * portalInfo, const char * projName, const char * appName );
		ENVIRONSAPI LIBEXPORT EBOOL CallConv setPortalInfo1 ( void * portalInfo, const char * projName, const char * appName );
		ENVIRONSAPI LIBEXPORT void CallConv setDeviceID2 ( int deviceID );

		/**
		* Query whether the native layer was build for release (or debug).
		*
		* @return	true = Release build, false = Debug build.
		*/
		ENVIRONSAPI LIBEXPORT EBOOL CallConv getIsReleaseBuild ();

/**
*	API exports for all platforms but Android
*
* */
#if !defined(ANDROID)

#ifdef __cplusplus
		extern "C"
		{
#endif
			LIBEXPORT const char * CallConv getVersionString ();
			LIBEXPORT int CallConv			getVersionMajor ();
			LIBEXPORT int CallConv			getVersionMinor ();
			LIBEXPORT int CallConv			getVersionRevision ();
			LIBEXPORT int CallConv			getRuntimePlatform ();

			LIBEXPORT void CallConv			registerMainThread ();
			LIBEXPORT void CallConv			setDebug ( int mode );
			LIBEXPORT void CallConv			setOSLevel ( int level );
			LIBEXPORT void CallConv			resetIdentKeys ();
			LIBEXPORT void CallConv			setDeviceID1 ( int myDeviceID );
			LIBEXPORT int CallConv			getDeviceID ();
			LIBEXPORT int CallConv			getDeviceIDFromMediator ();
            
            
			LIBEXPORT int CallConv			getMediatorFilterLevel ( );
			LIBEXPORT void CallConv			setMediatorFilterLevel ( int level );
			LIBEXPORT int CallConv			getIPAddress ( );
			LIBEXPORT int CallConv			getSubnetMask ( );
			LIBEXPORT void CallConv			setDeviceDims ( int width, int height, float width_mm, float height_mm, int leftpos, int toppos );

			LIBEXPORT EBOOL CallConv		setPorts1 ( int tcpPort, int udpPort );
			LIBEXPORT void CallConv			setDeviceType ( char type );
			LIBEXPORT char CallConv			getDeviceType ( );
            LIBEXPORT EBOOL CallConv		hasDeviceUID ( );
            LIBEXPORT EBOOL CallConv		setDeviceUID ( const char * name );
			LIBEXPORT EBOOL CallConv		setDeviceName ( const char * name );
            
			LIBEXPORT EBOOL CallConv		setUseTouchRecognizer1 ( const char * name, bool enable );
            
			LIBEXPORT EBOOL CallConv		setProjectName1 ( const char * name );
			LIBEXPORT const char * CallConv	getProjectName ( );
			LIBEXPORT EBOOL CallConv		setApplicationName1 ( const char * name );
			LIBEXPORT const char * CallConv	getApplicationName ( );
			LIBEXPORT EBOOL CallConv		setMediatorUserName1 ( const char * name );
			LIBEXPORT EBOOL CallConv		setMediatorPassword1 ( const char * name );
			LIBEXPORT void CallConv			setUseAuthentication ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		setGCMAPIKey ( const char * key );

			LIBEXPORT int CallConv			init1 ();
			LIBEXPORT void CallConv			setAppStatus ( int status );
			LIBEXPORT int CallConv			setMainAppWindow ( WNDHANDLE hWnd );
			LIBEXPORT EBOOL CallConv		setPortalSourceWindow ( WNDHANDLE hWnd, int deviceID, const char * projName, const char * appName );
			LIBEXPORT void CallConv			updateAppWindowSize ( );

			LIBEXPORT void CallConv			setUseTouchInjection ( EBOOL enabled );

			LIBEXPORT EBOOL CallConv		getUseOpenCL ( );
			LIBEXPORT void CallConv			setUseOpenCL ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		getUseStream ( );
			LIBEXPORT void CallConv			setUseStream ( EBOOL enable );

			LIBEXPORT void CallConv			setUseMouseEmulation ( EBOOL enable );
			LIBEXPORT EBOOL CallConv		getUseMouseEmulation ( );

			LIBEXPORT void CallConv			setUseTouchVisualization ( EBOOL enable );
			LIBEXPORT EBOOL CallConv		getUseTouchVisualization ( );

			LIBEXPORT int CallConv			getDeviceConnectStatus ( int deviceID, const char * projName, const char * appName );
			LIBEXPORT EBOOL CallConv		getDirectContactStatus ( int deviceID, const char * projName, const char * appName );
            
			LIBEXPORT EBOOL CallConv		getPortalEnabled ( int deviceID, const char * projName, const char * appName );
			LIBEXPORT int CallConv			deviceDetected ( int deviceID, const char * projName, const char * appName, int async, int x, int y, float angle );

			LIBEXPORT int CallConv			deviceConnect ( int deviceID, const char * projName, const char * appName, int async );
			LIBEXPORT EBOOL CallConv		deviceDisconnect1 ( int deviceID, const char * projName, const char * appName, int async );

			LIBEXPORT void CallConv			deviceUpdated ( int deviceID, const char * projName, const char * appName, int async, int x, int y, float angle );
			LIBEXPORT void CallConv			devicePositionUpdated ( int deviceID, const char * projName, const char * appName, int async, int x, int y );
			LIBEXPORT void CallConv			deviceAngleUpdated ( int deviceID, const char * projName, const char * appName, int async, float angle );
			LIBEXPORT void CallConv			deviceRemoved ( int deviceID, const char * projName, const char * appName, int async, int x, int y, float angle );
			LIBEXPORT void CallConv			deviceRemovedID ( int deviceID, const char * projName, const char * appName, int async );
				
			LIBEXPORT int CallConv			getStatus ();
			LIBEXPORT int CallConv			start1 ();
			LIBEXPORT int CallConv			stop1 ();
			LIBEXPORT void CallConv			release1 ();

			LIBEXPORT void CallConv			setStreamOverUdp ( );
			LIBEXPORT void CallConv			setStreamOverTcp ( );
			LIBEXPORT EBOOL CallConv		streamToggleQuality ( );
			LIBEXPORT void CallConv			setStreamJpegs ( );
			LIBEXPORT void CallConv			setStreamPngs ( );

			LIBEXPORT EBOOL CallConv		sendBuffer ( int deviceID, const char * projName, const char * appName, int async, int fileID, const char * fileDescriptor, char * buffer,  int size );
			LIBEXPORT EBOOL CallConv		sendFile ( int deviceID, const char * projName, const char * appName, int async, int fileID, const TCHAR * fileDescriptor, const TCHAR * file );
			LIBEXPORT EBOOL CallConv		sendMsg ( int deviceID, const char * projName, const char * appName, int async, const char * message, int length );

			LIBEXPORT EBOOL CallConv		sendPushNotification ( int deviceID, const char * projName, const char * appName, const char * message );

			LIBEXPORT EBOOL CallConv		registerAtMediators ();

			LIBEXPORT EBOOL CallConv		setMediator ( const char * ip, int port );
			LIBEXPORT const char * CallConv getMediatorIP ();
			LIBEXPORT const char * CallConv getMediatorUserName1 ();
			LIBEXPORT int CallConv			getMediatorPort ();

			LIBEXPORT void CallConv			setUseDefaultMediator ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		getUseDefaultMediator ( );
            
			LIBEXPORT void CallConv			setUseCustomMediator ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		getUseCustomMediator ( );
			
            LIBEXPORT void CallConv			setUseDefaultMediator ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		getUseDefaultMediator ( );

			LIBEXPORT void CallConv			setUseCLSForMediator ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		getUseCLSForMediator ( );

			LIBEXPORT void CallConv			setUseCLSForDevices ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		getUseCLSForDevices ( );

			LIBEXPORT void CallConv			setUseCLSForDevicesEnforce ( EBOOL usage );
			LIBEXPORT EBOOL CallConv		getUseCLSForDevicesEnforce ( );

			LIBEXPORT void CallConv			setUseCLSForAllTraffic ( EBOOL usage );
            LIBEXPORT EBOOL CallConv		getUseCLSForAllTraffic ( );
            
            LIBEXPORT void CallConv			setUseNotifyDebugMessage ( EBOOL usage );
            LIBEXPORT EBOOL CallConv		getUseNotifyDebugMessage ( );
            
            LIBEXPORT void CallConv			setUseLogFile ( EBOOL usage );
            LIBEXPORT EBOOL CallConv		getUseLogFile ( );
            
			LIBEXPORT void CallConv			setNetworkStatus ( int netStat );
			LIBEXPORT int CallConv			getNetworkStatus ( );
            
            LIBEXPORT EBOOL CallConv        applyModuleToSettings ( const char * moduleName, char ** target, bool testInstance = true );
            
			LIBEXPORT EBOOL CallConv		setUseEncoder ( const char * moduleName );
			LIBEXPORT EBOOL CallConv		setUseDecoder ( const char * moduleName );
			LIBEXPORT EBOOL CallConv		setUseRenderer ( const char * moduleName );
			LIBEXPORT EBOOL CallConv		setUseCapturer ( const char * moduleName );

			LIBEXPORT void * CallConv		getFile ( int deviceID, const char * projName, const char * appName, int fileID, void * buffer, int * capacity );
			LIBEXPORT BSTR CallConv			getFilePath ( int deviceID, const char * projName, const char * appName, int fileID );


			LIBEXPORT EBOOL CallConv		getPortalInfo ( void * buffer, const char * projName, const char * appName );
			LIBEXPORT EBOOL CallConv		setPortalInfo ( void * buffer, const char * projName, const char * appName );

			LIBEXPORT EBOOL CallConv		setPortalOverlayARGB ( int deviceID, const char * projName, const char * appName, int portalID, int layerID, int left, int top,
				 int width, int height, int stride, void * renderData, int alpha, bool positionDevice );

			LIBEXPORT void CallConv			freeNativeMemory ( void * obj );

			LIBEXPORT void * CallConv		getDevices ( int fromType );
			LIBEXPORT void * CallConv		getDevice ( int deviceID, const char * projName, const char * appName, int fromType );
			LIBEXPORT int CallConv			getDevicesCount ( int fromType );
			LIBEXPORT void * CallConv		getDeviceScreenProps ( int deviceID, const char * projName, const char * appName );
			LIBEXPORT void * CallConv		getDeviceAvailableCachedBestMatch1 ( int deviceID );

			LIBEXPORT int CallConv			getConnectedDevicesCount ( );

			LIBEXPORT int CallConv			getDeviceInstanceSize ();
			LIBEXPORT int CallConv			getDevicesHeaderSize ();

			//LIBEXPORT void CallConv			deallocNative ();
			LIBEXPORT void CallConv			initStorage ( const char * path );
			LIBEXPORT void CallConv			initWorkDir ( const char * path );
			LIBEXPORT void CallConv			setGCM ( const char * gcm );
			LIBEXPORT int CallConv			getDeviceWidth ( int deviceID, const char * projName, const char * appName );
			LIBEXPORT int CallConv			getDeviceHeight ( int deviceID, const char * projName, const char * appName );

			LIBEXPORT EBOOL CallConv		initRequestPortalStream ( int deviceID, const char * projName, const char * appName, int async, int typeID );
			LIBEXPORT EBOOL CallConv		providePortalStream1 ( int deviceID, const char * projName, const char * appName, int async, int portalID );
			LIBEXPORT EBOOL CallConv		startPortalStream1 ( int deviceID, const char * projName, const char * appName, int async, int portalID );
			LIBEXPORT EBOOL CallConv		pausePortalStream ( int deviceID, const char * projName, const char * appName, int async, int portalID );
			LIBEXPORT EBOOL CallConv		stopPortalStream ( int deviceID, const char * projName, const char * appName, int async, int portalID );
            
			LIBEXPORT EBOOL CallConv		setRenderSurface1 ( int deviceID, const char * projName, const char * appName, int portalID,
				void * renderSurface, int width, int height );
			LIBEXPORT EBOOL CallConv		releaseRenderSurface1 ( int deviceID, const char * projName, const char * appName, int async, int portalID );
			LIBEXPORT EBOOL CallConv		setRenderCallback ( int deviceID, const char * projName, const char * appName, int portalID, int async, long callback, int type );

#ifdef __cplusplus
		}
#endif

#endif // ANDROID - include only the jni-exports

	}
}

#endif	// INCLUDE_HCM_ENVIRONS_NATIVE_API_H



