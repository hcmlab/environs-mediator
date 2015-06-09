/**
 * Environs.ios.h
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
#include "Environs.platforms.h"

#if ( defined(ENVIRONS_IOS) || defined(ENVIRONS_OSX) )

#ifdef ENVIRONS_IOS
    //******** iOS *************
    #import <UIKit/UIKit.h>
    #import <CoreMotion/CoreMotion.h>
    #import <Environs.ios.h>

#else
    //******** OSX *************
    #import <Environs.osx.h>
    #import <Cocoa/Cocoa.h>

    #define     UIView          NSView
    #define     UIAlertView     NSAlertView

#endif

#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>

#import "Environs.Listener.h"
#include "Portal.Info.h"


bool createAppID ( char * buffer, unsigned int bufSize );

@interface Environs :
#ifdef ENVIRONS_IOS
    NSObject<CLLocationManagerDelegate, UIAlertViewDelegate>
#else
    NSObject
#endif
{
}

+ (bool) opt:(NSString *) key;
+ (bool) verbose;

+ (void) showMessage:(int) deviceID Project:(const char *) projectName App:(const char *) appName Message:(const char *) message withLength:(int)msgLength;

/**
 * Initialize the environment. This must be called after the user interface has been loaded, rendered and shown.
 * Tasks:
 *  - Request display always on state, hence disable standby and power management functions.
 *  - Load sensor managers and sensor services
 *  - Initialize google cloud messaging
 *
 @returns success
 */
+ (bool) Init;
+ (void) resetIdentKeys;

/**
 * Get the native version of Environs.
 *
 * @return		version string
 */
+ (const char *) getVersionString;

/**
 * Get the native major version of Environs.
 *
 * @return		major version
 */
+ (int) getVersionMajor;

/**
 * Get the native minor version of Environs.
 *
 * @return		minor version
 */
+ (int) getVersionMinor;

/**
 * Get the native revision of Environs.
 *
 * @return		revision
 */
+ (int) getVersionRevision;


/**
 * Query whether the native layer was build for release (or debug).
 *
 * @return	true = Release build, false = Debug build.
 */
+ (bool) getIsReleaseBuild;


/**
* Set app status in order to enable or disable sleep mode, e.g. if the app runs in background.
* In APP_STATUS_SLEEPING, all network activities (except heartbeat packets) are disabled.
*
* @param 	status	One of the APP_STATUS_* values.
*/
+ (void) setAppStatus:(int) status;

/**
 Set the render surface of type UIView as the target to render the portal to. The width and height of the UIView is defined as the size of a fullscreen view.
 The portal is determined through the deviceID and portalID.
 @param deviceID
 @param portalID
 @param newSurface
 @returns success
 */
+ (bool) setRenderSurface:(int)deviceID withPortal:(int)portalID withSurface:(UIView *)newSurface;
+ (bool) setRenderSurface:(int)deviceID Project:(const char *) projectName App:(const char *) appName withPortal:(int)portalID withSurface:(UIView *)newSurface;

/**
 Set the render surface of type UIView as the target to render the portal to. The UIView has a defined width and size.
 The portal is determined through the deviceID and portalID.
 @param deviceID
 @param portalID
 @param newSurface
 @param width
 @param height
 @returns success
 */
+ (bool) setRenderSurface:(int)deviceID withPortal:(int)portalID withSurface:(UIView *)newSurface withWidth:(int) width withHeight:(int) height;

/**
 Set the render surface of type UIView as the target to render the portal to.
 The portal is determined through the deviceID, projectName, appName, and portalID.
 @param deviceID
 @param projectName
 @param appName
 @param portalID
 @param newSurface
 @param width
 @param height
 @returns success
 */
+ (bool) setRenderSurface:(int)deviceID Project:(const char *) projectName App:(const char *) appName withPortal:(int)portalID withSurface:(UIView *)newSurface withWidth:(int) width withHeight:(int) height;

/**
 Release the render surface determined through the deviceID and portalID.
 @param deviceID
 @param portalID
 @returns success
 */
+ (bool) releaseRenderSurface:(int)deviceID withPortal:(int)portalID;

/**
 Release the render surface determined through the deviceID, projectName, appName, and portalID.
 @param deviceID
 @param projectName
 @param appName
 @param portalID
 @returns success
 */
+ (bool) releaseRenderSurface:(int)deviceID Project:(const char *) projectName App:(const char *) appName withPortal:(int)portalID;

+ (void) setDebug:(int)value;

/**
 * Set the device type that the local instance of Environs shall use for identification within the environment.&nbsp;
 * Valid type are enumerated in Environs.DEVICE_TYPE_*
 *
 * @param	type	Environs.DEVICE_TYPE_*
 */
+ (void) setDeviceType:(char)value;

/**
 * Get the device type that the local instance of Environs use for identification within the environment.&nbsp;
 * Valid type are enumerated in Types.DEVICE_TYPE_*
 *
 * @return	type	Environs.DEVICE_TYPE_*
 */
+ (char) getDeviceType;

/**
 * Set the user name for authentication with a Mediator service.&nbsp;Usually the user's email address is used as the user name.
 *
 * @param 	username
 * @return	success
 */
+ (bool) setMediatorUserName: (NSString *) userName;
+ (bool) setUserName: (const char *) username;

/**
 * Query UserName used to authenticate with a Mediator.
 *
 * @return UserName
 */
+ (NSString *) getMediatorUserName;

/**
 * Set the user password for authentication with a Mediator service.&nbsp;The password is stored as a hashed token within Environs.
 *
 * @param 	password
 * @return	success
 */
+ (bool) setMediatorPassword: (NSString *) password;
+ (bool) setUserPassword: (const char *) password;

/**
 * Enable or disable authentication with the Mediator using username/password.
 *
 * @param 	enable
 */
+ (void) setUseAuthentication: (bool) enable;

/**
 * Query the filter level for device management within Environs.
 *
 * return level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_PROJECT, Environs.MEDIATOR_FILTER_PROJECT_AND_APP
 */
+ (int) getMediatorFilterLevel;

/**
 * Set the filter level for device management within Environs.
 *
 * @param level	can be one of the values Environs.MEDIATOR_FILTER_NONE, Environs.MEDIATOR_FILTER_PROJECT, Environs.MEDIATOR_FILTER_PROJECT_AND_APP
 */
+ (void) setMediatorFilterLevel:(int)level;


/**
 * Retrieve a boolean that determines whether Environs shows up a login dialog if a Mediator is used and no credentials are available.
 *
 * @return		true = yes, false = no
 */
+ (bool) getUseMediatorLoginDialog;

/**
 * Instruct Environs to show up a login dialog if a Mediator is used and no credentials are available.
 *
 * @param enable      true = enable, false = disable
 */
+ (void) setUseMediatorLoginDialog: (bool) enable;

/**
 * Retrieve a boolean that determines whether Environs disable Mediator settings on dismiss of the login dialog.
 *
 * @return		true = yes, false = no
 */
+ (bool) getMediatorLoginDialogDismissDisable;

/**
 * Instruct Environs disable Mediator settings on dismiss of the login dialog.
 *
 * @param enable      true = enable, false = disable
 */
+ (void) setMediatorLoginDialogDismissDisable: (bool) enable;


/**
 * Register at known Mediator server instances.
 *
 * @return success state
 */
+ (bool) registerAtMediators;


/**
 * Start Environs.&nbsp;This is a non-blocking call and returns immediately.&nbsp;
 * 		Since starting Environs includes starting threads and activities that may take longer,&nbsp;
 * 		this call executes the start tasks within a thread.&nbsp;
 * 		In order to get the status, catch the onNotify handler of your EnvironsListener.
 *
 */
+ (void) start;

/**
 * Query the status of Environs.&nsbp;Valid values are Types.NATIVE_STATUS_*
 *
 * @return Environs.NATIVE_STATUS_*
 */
+ (int)  getStatus;

/**
 * Stop Environs and release all acquired resources.
 */
+ (void) stop;

/**
 * Stop Environs and release all acquired resources.
 */
+ (void) destroy;

/**
 * Set the listener for communication with Environs and devices within the environment.
 *
 * @param environsListener Your implementation of EnvironsListener.
 */
+ (void) setListener:(id<EnvironsListener>) environsListener;

/**
 * Set the ports that the local instance of Environs shall use for listening on connections.
 *
 * @param	tcpPort
 * @param	udpPort
 * @return success
 */
+ (void) setPorts:(int)comPort withDataPort:(int)dataPort;


/**
 * Check whether the device with the given ID is connected to the local instance of Environs.
 *
 * @param deviceID	Destination device ID
 * @return success true: connected; false: not connected
 */
+ (bool) isDeviceConnected:(int)deviceID;

/**
 * Get status whether the device with the given ID is connected to the local instance of Environs.&nbsp;
 * The return value is of enumeration type Types.DeviceStatus.*&nbsp;
 * That is e.g. Types.DeviceStatus.ConnectInProgress
 *
 * @param deviceID	Destination device ID
 * @return connectStatus Status is the integer value of one of the items in the enumeration Types.DeviceStatus.*
 */
+ (int) getDeviceConnectStatus:(int)deviceID;

/**
 * Get status whether the device with the given ID is connected to the local instance of Environs.&nbsp;
 * The return value is of enumeration type Types.DeviceStatus.*&nbsp;
 * That is e.g. Types.DeviceStatus.ConnectInProgress
 *
 * @param deviceID	Destination device ID
 * @param 	projectName	Project name of the application environment
 * @param 	appName		Application name of the application environment
 * @return connectStatus Status is the integer value of one of the items in the enumeration Types.DeviceStatus.*
 */
+ (int) getDeviceConnectStatus:(int)deviceID Project:(const char *) projectName App:(const char *) appName;

/**
 * Query the number of devices that are connected to the local instance of Environs.
 *
 * @return numberOfConnectedDevices
 */
+ (int) getConnectedDevicesCount;


+ (int) deviceConnect:(int)deviceID doAsync:(int)async;
+ (int) deviceConnect:(int)deviceID Project:(const char *) projectName App:(const char *) appName  doAsync:(int)async;

+ (bool) deviceDisconnect:(int)deviceID doAsync:(int)async;
+ (bool) deviceDisconnect:(int)deviceID Project:(const char *) projectName App:(const char *) appName doAsync:(int)async;

/**
 * Set the project name that the local instance of Environs shall use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @param	name
 * @return	success
 */
+ (void) setProjectName: (const char *) name;

/**
 * Get the project name that the local instance of Environs use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @return	projectName
 */
+ (const char *) getProjectName;


/**
 * Set the application name of that the local instance of Environs shall use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @param	name
 * @return	success
 */
+ (void) setApplication: (const char *) name;
+ (void) setApplicationName: (const char *) name;

/**
 * Get the application name that the local instance of Environs use for identification within the environment.
 * It must be set before creating the Environs instance.
 *
 * @return	appName
 */
+ (const char *) getApplicationName;

/**
 * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
 *
 * @param 	deviceName
 * @return	success
 */
+ (void) setDeviceName: (const char *) name;


/**
 * Request a portal stream from the device with the given id.&nbsp;
 * The device must be in a connected state by means of prior call to deviceConnect ().
 *
 * @param 	deviceID
 * @param 	typeID		Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
 * @return 	success
 */
+ (bool) requestPortalStream:(int) deviceID doAsync:(int)async withType:(int) typeID;

/**
 * Request a portal stream from the device with the given id.&nbsp;
 * The device must be in a connected state by means of prior call to deviceConnect ().
 *
 * @param 	deviceID
 * @param 	projectName	Project name of the application environment
 * @param 	appName		Application name of the application environment
 * @param 	typeID		Values should be of type PortalType.&nbsp;This is an application specific type identifier (e.g. used for distinguishing front facing or back facing camera)
 * @return 	success
 */
+ (bool) requestPortalStream:(int) deviceID Project:(const char *) projectName App:(const char *) appName doAsync:(int)async withType:(int) typeID;

/**
 * Start streaming of portal to this device.
 *
 * @param 	deviceID
 * @param 	portalID	An application specific id (e.g. used for distinguishing front facing or back facing camera)
 * @return 	success
 */
+ (bool) startPortalStream:(int) deviceID doAsync:(int)async withPortal:(int) portalID;

/**
 * Start streaming of portal to this device.
 *
 * @param 	deviceID
 * @param 	projectName	Project name of the application environment
 * @param 	appName		Application name of the application environment
 * @param 	portalID	An application specific id (e.g. used for distinguishing front facing or back facing camera)
 * @return 	success
 */
+ (bool) startPortalStream:(int) deviceID Project:(const char *) projectName App:(const char *) appName doAsync:(int)async withPortal:(int) portalID;

/**
 * Stop streaming of portal from this device.
 *
 * @param deviceID
 * @param portalID		This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
 * 					It is provided within the notification listener as sourceIdent.&nbsp;
 * 					Applications should store them in order to address the correct portal within Environs.
 * @return success
 */
+ (bool) stopPortalStream:(int) deviceID doAsync:(int)async withPortal:(int) portalID;

/**
 * Stop streaming of portal from this device.
 *
 * @param   deviceID
 * @param 	projectName	Project name of the application environment
 * @param 	appName		Application name of the application environment
 * @param   portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
 *                      It is provided within the notification listener as sourceIdent.&nbsp;
 *                      Applications should store them in order to address the correct portal within Environs.
 * @return success
 */
+ (bool) stopPortalStream:(int) deviceID Project:(const char *) projectName App:(const char *) appName doAsync:(int)async withPortal:(int) portalID;

/**
 * Pause streaming of portal to this device.
 *
 * @param 	deviceID
 * @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
 * 						It is provided within the notification listener as sourceIdent.&nbsp;
 * 						Applications should store them in order to address the correct portal within Environs.
 * @return success
 */
+ (bool) pausePortalStream:(int) deviceID doAsync:(int)async withPortal:(int) portalID;

/**
 * Pause streaming of portal to this device.
 *
 * @param 	deviceID
 * @param 	projectName	Project name of the application environment
 * @param 	appName		Application name of the application environment
 * @param 	portalID	This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
 * 						It is provided within the notification listener as sourceIdent.&nbsp;
 * 						Applications should store them in order to address the correct portal within Environs.
 * @return success
 */
+ (bool) pausePortalStream:(int) deviceID Project:(const char *) projectName App:(const char *) appName doAsync:(int)async withPortal:(int) portalID;

+ (unsigned int) getIPAddress;
+ (unsigned int) getSubnetMask;
+ (NSString *) getSSID;
+ (NSString *) getSSIDDesc;

#ifdef ENVIRONS_IOS
+ (int) getActivePortalID;
+ (int) getRequestedPortalID;
#endif

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
 * a registered EnvironsListener instance.
 *
 * @param deviceID	The device id which the message should be send to.
 * @param message Message to send
 * @return success
 */
+ (bool) sendMessage: (int)deviceID doAsync:(int)async withMsg:(NSString *) msg;

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
 * a registered EnvironsListener instance.
 *
 * @param deviceID	The device id which the message should be send to.
 * @param 	projectName	Project name of the application environment
 * @param 	appName		Application name of the application environment
 * @param message Message to send
 * @return success
 */
+ (bool) sendMessage: (int)deviceID Project:(const char *) projectName App:(const char *) appName doAsync:(int)async withMsg:(NSString *) msg;

/**
 * Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
 *
 * @param deviceID
 * @param fileID
 * @param descriptor (e.g. filename)
 * @param path
 * @return success
 */
+ (bool) sendFile: (int)deviceID doAsync:(int)async withFileID:(int)fileID withDescriptor:(NSString *) descriptor withPath:(NSString *) path;

/**
 * Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
 *
 * @param deviceID
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @param fileID
 * @param descriptor (e.g. filename)
 * @param path
 * @return success
 */
+ (bool) sendFile: (int)deviceID Project:(const char *) projectName App:(const char *) appName doAsync:(int)async withFileID:(int)fileID withDescriptor:(NSString *) descriptor withPath:(NSString *) path;

/**
 * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
 *
 * @param deviceID
 * @param fileID
 * @param descriptor (e.g. filename)
 * @param buffer
 * @param length number of bytes in the buffer to send
 * @return success
 */
+ (bool) sendBuffer: (int)deviceID doAsync:(int)async withFileID:(int)fileID withDescriptor:(NSString *) descriptor withBuffer:(char *) buffer withLength:(int)length;

/**
 * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
 *
 * @param deviceID
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @param fileID
 * @param descriptor (e.g. filename)
 * @param buffer
 * @param length number of bytes in the buffer to send
 * @return success
 */
+ (bool) sendBuffer: (int)deviceID Project:(const char *) projectName App:(const char *) appName doAsync:(int)async withFileID:(int)fileID withDescriptor:(NSString *) descriptor withBuffer:(char *) buffer withLength:(int)length;

/**
 * Query the width in pixel of the deviceID's screen.&nbsp;The device must be connected before with deviceConnect ().
 *
 * @param deviceID
 * @return width in pixel
 */
+ (int) getDeviceWidth:(int)deviceID;

/**
 * Query the width in pixel of the deviceID's screen.&nbsp;The device must be connected before with deviceConnect ().
 *
 * @param deviceID
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @return width in pixel
 */
+ (int) getDeviceWidth:(int)deviceID Project:(const char *) projectName App:(const char *) appName;

/**
 * Query the height in pixel of the deviceID's screen. The device must be connected before with deviceConnect ()
 *
 * @param deviceID
 * @return height in pixel
 */
+ (int) getDeviceHeight:(int)deviceID;

/**
 * Query the height in pixel of the deviceID's screen. The device must be connected before with deviceConnect ()
 *
 * @param deviceID
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @return height in pixel
 */
+ (int) getDeviceHeight:(int)deviceID Project:(const char *) projectName App:(const char *) appName;

/**
 * Query a DeviceScreen object of the device with the deviceID.&nbsp;
 * Note: The device must be connected in order to query the information.
 *
 * @param deviceID
 * @return DeviceScreen
 */
+ (char *) getDeviceScreenSizes:(int)deviceID;

/**
 * Query a DeviceScreen object of the device with the deviceID.&nbsp;
 * Note: The device must be connected in order to query the information.
 *
 * @param deviceID
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @return DeviceScreen
 */
+ (char *) getDeviceScreenSizes:(int)deviceID Project:(const char *) projectName App:(const char *) appName;

/**
 * Query an array of DeviceInfo objects of available devices within the environment (including those of the Mediator)
 *
 * @return DeviceInfo-objects
 */
+ (char *) getDevicesAvailable;

/**
 * Query an array of DeviceInfo objects with one DeviceInfo of available devices within the environment (including those of the Mediator)
 *
 * @param deviceID
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @return DeviceInfo-objects
 */
+ (char *) getDeviceAvailable:(int)deviceID Project:(const char *) projectName App:(const char *) appName;

/**
 * Query the number of available devices within the environment (including those of the Mediator)
 *
 * @return numberOfDevices
 */
+ (int) getDevicesAvailableCount;

/**
 * Query an array of DeviceInfo objects of nearby (broadcast visible) devices within the environment.
 *
 * @return DeviceInfo-objects
 */
+ (char *) getDevicesNearby;

/**
 * Query an array of DeviceInfo objects of nearby (broadcast visible) devices within the environment.
 *
 * @param deviceID
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @return DeviceInfo-objects
 */
+ (char *) getDeviceNearby:(int)deviceID Project:(const char *) projectName App:(const char *) appName;

/**
 * Query the number of nearby (broadcast visible) devices within the environment.
 *
 * @return numberOfDevices
 */
+ (int) getDevicesNearbyCount;

/**
 * Query an array of DeviceInfo objects of Mediator managed devices within the environment.
 *
 * @return DeviceInfo-objects
 */
+ (char *) getDevicesFromMediator;

/**
 * Query an array of DeviceInfo objects with one DeviceInfo of Mediator managed devices within the environment.
 *
 * @param deviceID
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @return DeviceInfo-objects
 */
+ (char *) getDeviceFromMediator:(int)deviceID Project:(const char *) projectName App:(const char *) appName;

/**
 * Query the number of Mediator managed devices within the environment.
 *
 * @return numberOfDevices
 */
+ (int) getDevicesFromMediatorCount;

/**
 * Get the status, whether the device (id) has direct physical contact, such as lying on the surface
 *
 * @param		deviceID
 * @return		true = yes, false = no
 */
+ (bool) getDirectContactStatus:(int)deviceID;

/**
 * Get the status, whether the device (id) has direct physical contact, such as lying on the surface.
 *
 * @param deviceID		Destination device id
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @return		true = yes, false = no
 */
+ (bool) getDirectContactStatus:(int)deviceID Project:(const char *) projectName App:(const char *) appName;

/**
 * Get the status, whether the device (id) has established an active portal
 *
 * @param		deviceID
 * @return		true = yes, false = no
 */
+ (bool) getPortalEnabled:(int)deviceID;

/**
 * Get the status, whether the device (id) has established an active portal
 *
 * @param deviceID
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @return		true = yes, false = no
 */
+ (bool) getPortalEnabled:(int)deviceID Project:(const char *) projectName App:(const char *) appName;


/**
 * Get details about portal associated with the deviceID.
 *
 * @param deviceID  	The deviceID
 * @param portalID		This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
 * 						It is provided within the notification listener as sourceIdent.&nbsp;
 * 						Applications should store them in order to address the correct portal within Environs.
 * @return portalInfo A PortalInfo object containing the details about the portal. If the call fails, the value is null.
 */
+ (environs::PortalInfo *) getPortalInfo:(int)deviceID withID:(int)portalID;

/**
 * Get details about portal associated with the deviceID.
 *
 * @param deviceID  	The deviceID
 * @param projectName	Project name of the application environment
 * @param appName		Application name of the application environment
 * @param portalID		This is an id that Environs use to manage multiple portals from the same source device.&nbsp;
 * 						It is provided within the notification listener as sourceIdent.&nbsp;
 * 						Applications should store them in order to address the correct portal within Environs.
 * @return portalInfo A PortalInfo object containing the details about the portal. If the call fails, the value is null.
 */
+ (environs::PortalInfo *) getPortalInfo:(int)deviceID Project:(const char *) projectName App:(const char *) appName withID:(int)portalID;

/**
 * Set details for the portal associated with the deviceID.
 *
 * @param   A PortalInfo object (that may have been queried by a former call to GetPortalInfo()).&nbsp;
 * 			The deviceID and portalID members of the PortalInfo object must have valid values.
 * @return success
 */
+ (bool) setPortalInfo:(environs::PortalInfo *)info;

/**
 * Set details for the portal associated with the deviceID.
 *
 * @param   A PortalInfo object (that may have been queried by a former call to GetPortalInfo()).&nbsp;
 * 			The deviceID and portalID members of the PortalInfo object must have valid values.
 * @param 	projectName	Project name of the application environment
 * @param 	appName		Application name of the application environment
 * @return success
 */
+ (bool) setPortalInfo:(environs::PortalInfo *)info Project:(const char *) projectName App:(const char *) appName;

/**
 * Set the device id that is assigned to the instance of Environs.
 *
 * @param		deviceID
 */
+ (void) setDeviceID:(int)deviceID;

/**
 * Get the device id that the application has assigned to the instance of Environs.
 *
 * @return		deviceID
 */
+ (int) getDeviceID;

/**
 * Request a device id from mediator server instances that have been provided before this call.
 * Prior to this call, the project and application name must be provided as well,
 * in order to get an available device id for the specified application environment.
 * If the application has already set a deviceID (using setDeviceID), this call returns the previously set value.
 *
 * @return		deviceID
 */
+ (int) getDeviceIDFromMediator;

/**
 * Query whether the name of the current device has been set before.
 *
 * @return	has DeviceUID
 */
+ (bool) hasDeviceUID;

/**
 * Set the name of the current device.&nbsp;This name is used to communicate with other devices within the environment.
 *
 * @param 	uid
 * @return	success
 */
+ (bool) setDeviceUID:(const char *)name;


/**
 * Determines whether to use environs default Mediator predefined by framework developers or not.
 *
 * @param enable 	true = use the default Mediator
 */
+ (void) setUseDefaultMediator:(bool)usage;

/**
 * Query whether to use given Mediator by setMediator()
 *
 * @return enabled
 */
+ (bool) getUseDefaultMediator;

/**
 * Determines whether to use given Mediator by setMediator()
 *
 * @param enable 	true = enable, false = disable
 */
+ (void) setUseCustomMediator:(bool)usage;

/**
 * Query whether to use given Mediator by setMediator()
 *
 * @return enabled
 */
+ (bool) getUseCustomMediator;

/**
 * Query ip of custom Mediator.
 *
 * @return ip
 */
+ (NSString *) getMediatorIP;
//+ (void) setMediatorIP:(NSString *)value;

/**
 * Query port of custom Mediator.
 *
 * @return Port
 */
+ (int) getMediatorPort;

//+ (void) setMediatorPort:(unsigned short) value;

/**
 * Set custom Mediator to use.
 *
 * @param ip
 * @param port
 */
+ (void) setMediator:(NSString *)ip withPort:(unsigned short) port;

/**
 * Determines whether to use Crypto Layer Security for Mediator connections.
 * If a Mediator enforces CLS, then disabling this option will result in failure to connect to that Mediator.
 *
 * @param	enable
 */
+ (void) setUseCLSForMediator:(bool)usage;

/**
 * Query whether to use Crypto Layer Security for Mediator connections.
 *
 * @return	enabled
 */
+ (bool) getUseCLSForMediator;

/**
 * Determines whether to use Crypto Layer Security for device-to-device connections.
 *
 * @param	enable
 */
+ (void) setUseCLSForDevices:(bool)usage;

/**
 * Query whether to use Crypto Layer Security for device-to-device connections.
 *
 * @return	enabled
 */
+ (bool) getUseCLSForDevices;

/**
 * Determines whether to enforce Crypto Layer Security for device-to-device connections.
 *
 * @param	enable
 */
+ (void) setUseCLSForDevicesEnforce:(bool)usage;

/**
 * Query whether to enforce Crypto Layer Security for device-to-device connections.
 *
 * @return	enabled
 */
+ (bool) getUseCLSForDevicesEnforce;

/**
 * Enable Crypto Layer Security for all traffic (incl. those of interactive type) in device-to-device connections.
 *
 * @param	enable
 */
+ (void) setUseCLSForAllTraffic:(bool)usage;

/**
 * Query whether all traffic (incl. those of interactive type) in device-to-device connections is encrypted.
 *
 * @return	enabled
 */
+ (bool) getUseCLSForAllTraffic;

+ (void) setUseH264:(bool)usage;
+ (bool) getUseH264;

/**
 * Instruct Environs to show log messages in the status log.
 *
 * @param enable      true = enable, false = disable
 */
+ (void) setUseNotifyDebugMessage:(bool)usage;

/**
 * Query Environs settings that show log messages in the status log.
 *
 * @return enable      true = enable, false = disable
 */
+ (bool) getUseNotifyDebugMessage;

/**
 * Instruct Environs to create and write a log file in the working directory.
 *
 * @param enable      true = enable, false = disable
 */
+ (void) setUseLogFile:(bool)usage;

/**
* Query Environs settings that create and write a log file in the working directory.
*
* @return enable      true = enable, false = disable
*/
+ (bool) getUseLogFile;


/**
 * Enable or disable a touch recognizer module by name (libEnv-Rec...).
 *
 * @param	moduleName  The module name
 * @param	enable      Enable or disable
 * @return  success
 */
+ (bool) setUseTouchRecognizer: (NSString *) moduleName withStatus:(bool)enable;


/**
 * Enable or disable a portal encoder module by name (libEnv-Enc...).
 *
 * @param	moduleName  The module name
 * @param	enable      Enable or disable
 * @return  success
 */
+ (bool) setUseEncoder: (NSString *) moduleName;


/**
 * Enable or disable a portal decoder module by name (libEnv-Dec...).
 *
 * @param	moduleName  The module name
 * @param	enable      Enable or disable
 * @return  success
 */
+ (bool) setUseDecoder: (NSString *) moduleName;


#ifdef ENVIRONS_IOS
/**
 * Determines whether to use native C++ for portal stream encoding/decoding or not.
 *
 * @param	enable
 */
+ (void) setUseNativeDecoder:(bool)usage;

/**
 * Query whether to use native C++ for portal stream encoding/decoding or not.
 *
 * @return	enabled
 */
+ (bool) getUseNativeDecoder;

/**
 * Determines whether to use device sensor data (compass, acc, gyro, etc.) for interaction or not.
 *
 * @param	enable
 */
+ (void) setUseSensors:(bool)usage;

/**
 * Query whether to use device sensor data (compass, acc, gyro, etc.) for interaction or not.
 *
 * @return	enabled
 */
+ (bool) getUseSensors;

/**
 * Query whether to use native display resolution for the portal stream.
 *
 * @return enabled
 */
+ (bool) getPortalNativeResolution;

/**
 * Determines whether to use native display resolution for the portal stream.
 *
 * @param enable 	true = enable, false = disable
 */
+ (void) setPortalNativeResolution:(bool)usage;

/**
 * Query whether to automatically start a portal stream on successful device connection or not.
 *
 * @return enabled 	true = enable, false = disable
 */
+ (bool) getPortalAutoStart;

/**
 * Determine whether to automatically start a portal stream on successful device connection or not.
 *
 * @param enable 	true = enable, false = disable
 */
+ (void) setPortalAutoStart:(bool)usage;

/**
 * Determine whether to use  TCP for portal streaming (if not selectively set for a particular deviceID)
 *
 * @param enable
 */
+ (void) setPortalTCP:(bool)usage;

/**
 * Query whether to use TCP for portal streaming (UDP otherwise)
 *
 * @return enabled
 */
+ (bool) getPortalTCP;

+ (void) setUsePortalTCP:(bool)usage;
+ (bool) getUsePortalTCP;
#endif

@end

#endif
