/**
 * DeviceInstance iOSX
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_IOSX_H
#define INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_IOSX_H

#include "Device.Instance.h"

#include "Environs.iOSX.Imp.h"
#include "Device.Info.h"
#include "Device.Display.Decl.h"
#import "Portal.Instance.iOSX.h"
#import "Environs.Observer.iOSX.h"


/**
 *	DeviceInstance iOSX
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
@interface DeviceInstance : NSObject
{
}

/**
 * disposed is true if DeviceInstance is no longer valid. Nothing will be updated anymore.
 * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
 * */
@property (readonly, nonatomic) bool			disposed;

/** Perform the tasks asynchronously. If set to Environs.CALL_SYNC, the commands will block (if possible) until the task finishes. */
@property (nonatomic) int						async;

/** The device ID within the environment */
@property (readonly, nonatomic) int				deviceID;

/** The ID that is used by the native layer to identify this particular device within the environment.
 A value of 0 means that this device is not connected and therefore not actively managed. */
@property (readonly, nonatomic) int				nativeID;

/** IP from device. The IP address reported by the device which it has read from network configuration. */
@property (readonly, nonatomic) unsigned int 	ip;

/** IP external. The IP address which was recorded by external sources (such as the Mediator) during socket connections.
 * This address could be different from IP due to NAT, Router, Gateways behind the device.
 */
@property (readonly, nonatomic) unsigned int 	ipe; // The external IP or the IP resolved from the socket address

/** The tcp port on which the device listens for device connections. */
@property (readonly, nonatomic) unsigned short	tcpPort;

/** The udp port on which the device listens for device connections. */
@property (readonly, nonatomic) unsigned short	udpPort;

/** The number of alive updates noticed by the mediator layer since its appearance within the application environment. */
@property (readonly, nonatomic) unsigned int	updates;

/** A value that describes the device platform. */
@property (readonly, nonatomic) int				platform;

/** BroadcastFound is a value of DEVICEINFO_DEVICE_* and determines whether the device has been seen on the broadcast channel of the current network and/or from a Mediator service. */
@property (readonly, nonatomic) char			broadcastFound;
@property (readonly, nonatomic) bool            unavailable;

/** isConnected is true if the device is currently in the connected state. */
@property (readonly, nonatomic) bool			isConnected;
@property (readonly, nonatomic) char			internalUpdates;

/** Used internally by native layer. */
@property (readonly, nonatomic) char            internalType;

/** The device name. */
@property (readonly, nonatomic) const char *    deviceName;

/** The area name of the appliction environment. */
@property (readonly, nonatomic) const char *	areaName;

/** The application name of the appliction environment. */
@property (readonly, nonatomic) const char *	appName;


@property (readonly, nonatomic) short			connectProgress;

/** A DeviceDisplay structure that describes the device's display properties. */
@property (readonly, nonatomic) ENVIRONS_NAMESP DeviceDisplay	display;

@property (readonly, nonatomic) int				directStatus;

/** Application defined contexts for arbitrary use. */
@property (nonatomic) int						appContext0;
@property (nonatomic) id						appContext1;
@property (nonatomic) id						appContext2;
@property (nonatomic) id						appContext3;

/** An identifier that is unique for this object. */
@property (readonly, nonatomic) long			objID;

@property (readonly, nonatomic, copy) NSString * toString;

/**
* Add an observer (DeviceObserver) that notifies about device property changes.
*
* @param observer A DeviceObserver
*/
- (void) AddObserver:(id<DeviceObserver>) observer;

/**
* Remove an observer (DeviceObserver) that was added before.
*
* @param observer A DeviceObserver
*/
- (void) RemoveObserver:(id<DeviceObserver>) observer;

/**
* Add an observer (DataObserver) that notifies about data received or sent through the DeviceInstance.
*
* @param observer A DataObserver
*/
- (void) AddObserverForData:(id<DataObserver>) observer;

/**
* Remove an observer (SensorObserver) that was added before.
*
* @param observer A DataObserver
*/
- (void) RemoveObserverForData:(id<DataObserver>) observer;

/**
 * Add an observer (SensorObserver) that notifies about data received or sent through the DeviceInstance.
 *
 * @param observer A DataObserver
 */
- (void) AddObserverForSensors:(id<SensorObserver>) observer;

/**
 * Remove an observer (DataObserver) that was added before.
 *
 * @param observer A DataObserver
 */
- (void) RemoveObserverForSensors:(id<SensorObserver>) observer;

/**
* Add an observer (MessageObserver) that notifies about messages received or sent through the DeviceInstance.
*
* @param observer A MessageObserver
*/
- (void) AddObserverForMessages:(id<MessageObserver>) observer;

/**
* Remove an observer (MessageObserver) that was added before.
*
* @param observer A MessageObserver
*/
- (void) RemoveObserverForMessages:(id<MessageObserver>) observer;

/**
* Notify to all observers (DeviceObserver) that the appContext has changed.
*
* @param customFlags Either custom declared flags or 0. If 0 is provided, then the flag Environs.DEVICE_INFO_ATTR_APP_CONTEXT will be used.
*/
- (void) NotifyAppContextChanged:(int) customFlags;


- (NSString *) ips;
- (NSString *) ipes;

- (bool) EqualsAppEnv:(ENVIRONS_NAMESP DeviceInfo *) equalTo;
- (bool) EqualsAppEnv:(const char *)areaName App:(const char *)appName;

- (bool) LowerThanAppEnv:(ENVIRONS_NAMESP DeviceInfo *) compareTo;
- (bool) LowerThanAppEnv:(const char *)areaName App:(const char *)appName;

- (bool) EqualsID:(DeviceInstance *) equalTo;
- (bool) EqualsID:(int) deviceID Area:(const char *)areaName App:(const char *)appName;


+ (NSString *) DeviceTypeString:(ENVIRONS_NAMESP DeviceInfo *) info;
- (NSString *) DeviceTypeString;

- (const char *) GetBroadcastString:(bool) fullText;

/**
 * Connect to this device asynchronously.
 *
 * @param Environs_CALL_   A value of Environs_CALL_* that determines whether (only this call) is performed synchronous or asynchronous.
 *
 * @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
 * 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
 * 					true: A new connection has been triggered and is in progress
 */
- (bool) Connect;

/**
 * Connect to this device using the given mode.
 *
 * @return status	fase: Connection can't be conducted (maybe environs is stopped or the device id is invalid) &nbsp;
 * 					true: A connection to the device already exists or a connection task is already in progress) &nbsp;
 * 					true: A new connection has been triggered and is in progress
 */
- (bool) Connect:(int) Environs_CALL_;

/**
 * Disconnect the device with the given id and a particular application environment.
 *
 * @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
 */
- (bool) Disconnect;

/**
 * Disconnect the device using the given mode with the given id and a particular application environment.
 *
 * @param Environs_CALL_   A value of Environs_CALL_* that determines whether (only this call) is performed synchronous or asynchronous.
 *
 * @return	success		true: Connection has been shut down; false: Device with deviceID is not connected.
 */
- (bool) Disconnect:(int) Environs_CALL_;

/**
 * Retrieve display properties and dimensions of this device. The device must be connected before this object is available.
 *
 * @return PortalInstance-object
 */
- (ENVIRONS_NAMESP DeviceDisplay) GetDisplayProps;

/**
 * Load the file that is assigned to the fileID received by deviceID into an byte array.
 *
 * @param fileID        The id of the file to load (given in the onData receiver).
 * @param size        An int pointer, that receives the size of the returned buffer.
 * @return byte-array
 */
- (NSData *) GetFile:(int) fileID Size:(int *)size;

/**
 * Query the absolute path for the local filesystem that is assigned to the fileID received by deviceID.
 *
 * @param fileID        The id of the file to load (given in the onData receiver).
 * @return absolutePath
 */
- (const char *) GetFilePath:(int) fileID;


/**
 * Creates a portal instance that requests a portal.
 *
 * @param 	portalType	        Project name of the application environment
 *
 * @return 	PortalInstance-object
 */
- (PortalInstance *) PortalRequest:(ENVIRONS_NAMESP PortalType_t) portalType;

/**
 * Creates a portal instance that provides a portal.
 *
 * @param 	portalType	        Project name of the application environment
 *
 * @return 	PortalInstance-object
 */
- (PortalInstance *) PortalProvide:(ENVIRONS_NAMESP PortalType_t) portalType;


/**
 * Query the first PortalInstance that manages an outgoing portal.
 *
 * @return PortalInstance-object
 */
- (PortalInstance *) PortalGetOutgoing;

/**
 * Query the first PortalInstance that manages an incoming portal.
 *
 * @return PortalInstance-object
 */
- (PortalInstance *) PortalGetIncoming;

/**
 * Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
 *
 * @param fileID        A user-customizable id that identifies the file to be send.
 * @param fileDescriptor (e.g. filename)
 * @param filePath      The path to the file to be send.
 * @return success
 */
- (bool) SendFile:(int)fileID Desc:(const char *)fileDescriptor Path:(const char *)filePath;

/**
 * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
 *
 * @param fileID        A user-customizable id that identifies the file to be send.
 * @param fileDescriptor (e.g. filename)
 * @param buffer        A buffer to be send.
 * @param bytesToSend number of bytes in the buffer to send
 * @return success
 */
- (bool) SendBuffer:(int)fileID Desc:(const char *)fileDescriptor Data:(unsigned char *)buffer Size:(int)bytesToSend;

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
 * @param async			(Environs.CALL_NOWAIT) Perform asynchronous. (Environs.CALL_WAIT) Non-async means that this call blocks until the call finished.
 * @param message       A message to send.
 * @param length       Length of the message to send.
 * @return success
 */
- (bool) SendMessage:(int) async  message:(const char *)message length:(int)length;

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
- (bool) SendMessage:(NSString *)message;

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
 * @param async			(Environs.CALL_NOWAIT) Perform asynchronous. (Environs.CALL_WAIT) Non-async means that this call blocks until the call finished.
 * @param message       A message to be send.
 * @return success
 */
- (bool) SendMessage:(int) async message:(NSString *)message;


/**
* Query the absolute path for the local filesystem to the persistent storage for this DeviceInstance.
*
* @return absolutePath
*/
- (NSString *) GetStoragePath;

/**
* Clear (Delete permanently) all messages for this DeviceInstance in the persistent storage.
*
*/
- (void) ClearMessages;

/**
 * Clear cached MessageInstance and FileInstance objects for this DeviceInstance.
 *
 */
- (void) DisposeStorageCache;

/**
* Clear (Delete permanently) all files for this DeviceInstance in the persistent storage.
*
*/
- (void) ClearStorage;

/**
 * Get a dictionary with all files that this device instance has received.
 *
 * @return Collection with objects of type FileInstance with the fileID as the key.
 */
- (NSMutableArray *) GetAllFiles;


/**
 * Get a list with all messages that this device has received (and sent).
 *
 * @return Collection with objects of type MessageInstance
 */
- (NSMutableArray *) GetAllMessages;

@end

#endif	/// INCLUDE_HCM_ENVIRONS_DEVICEINSTANCE_IOSX_H

