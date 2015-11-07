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
#ifndef INCLUDE_HCM_ENVIRONS_OBSERVERS_IOSX_H
#define INCLUDE_HCM_ENVIRONS_OBSERVERS_IOSX_H

#import <Foundation/Foundation.h>

/**
 * EnvironsObserver: Attachable to **Environs** objects in order to receive all notifications that the Environs instance processes or submits to the platform layer.
 */
@protocol EnvironsObserver <NSObject>
/**
 * OnStatus is called whenever the framework status changes.&nbsp;
 *
 * @param Environs_STATUS_      A status constant of type STATUS_*
 */
- (void) OnStatus:(int) Environs_STATUS_;


/**
 * OnNotify is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 * The notification parameter is an integer value which represents one of the values as listed in Types.*
 * The string representation can be retrieved through TypesResolver.get(notification).
 *
 * @param nativeID      The native identifier that targets the device.
 * @param notification  The notification
 * @param source   		A value of the enumeration Types.EnvironsSource
 * @param context       A value that provides additional context information (if available).
 */
- (void) OnNotify:(int) nativeID Notify:(int) notification Source:(int)source Context:(void *)context;


/**
 * OnNotifyExt is called whenever a notification is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 * The notification parameter is an integer value which represents one of the values as listed in Types.*
 * The string representation can be retrieved through TypesResolver.get(notification).
 *
 * @param deviceID      The device id of the sender device.
 * @param areaName      Area name of the application environment
 * @param appName		Application name of the application environment
 * @param notification  The notification
 * @param source   		A value of the enumeration Types.EnvironsSource
 * @param context       A value that provides additional context information (if available).
 */
- (void) OnNotifyExt:(int) deviceID Area:(const char *) areaName App:(const char *) appName Notify:(int) notification Source:(int)source Context:(void *)context;


/**
 * OnPortal is called when a portal request from another devices came in, or when a portal has been provided by another device.
 *
 * @param portal 		The PortalInstance object.
 */
- (void) OnPortalRequestOrProvided:(id) portal;

@end


/**
* ListObserver: Attachable to **DeviceList** objects in order to receive list changes of a particular IDeviceList.
*/
@protocol ListObserver <NSObject>

/**
* OnListChanged is called whenever the connected DeviceList has changed, e.g. new devices appeared or devices vanished from the list.
*
* @param vanished     A collection containing the devices vansihed and removed from the list. This argument can be null.
* @param appeared     A collection containing the devices appeared and added to the list. This argument can be null.
*/
- (void) OnListChanged:(NSArray *) vanished appeared:(NSArray *) appeared;

@end


/**
* DeviceObserver: Attachable to **DeviceInstance** objects in order to receive changes of a particular device.
*/
@protocol DeviceObserver <NSObject>

/**
* OnDeviceChanged is called whenever the members of a DeviceInstance has changed.&nbsp;
* The DEVICE_INFO_ATTR_changed parameter provides a bit set which indicates the member that has changed.
*
* @param device                    The DeviceInstance object that sends this notification.
* @param DEVICE_INFO_ATTR_changed  The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
*/
- (void) OnDeviceChanged:(id) device Flags:(int) DEVICE_INFO_ATTR_changed;

@end


/**
 * EnvironsMessageObserver: Attachable to **Environs** objects in order to receive all messages that the Environs instance received.
 */
@protocol EnvironsMessageObserver <NSObject>
/**
 * OnMessage is called whenever a text message is available either from a device (deviceID != 0) or Environs (deviceID == 0).&nbsp;
 *
 * @param nativeID      The native identifier that targets the device.
 * @param type	        Determines the source (either from a device, environs, or native layer)
 * @param message       The message as string text
 * @param length        The length of the message
 */
- (void) OnMessage:(int) nativeID Type:(int)type Msg:(const char *) message Length:(int)length;

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
- (void) OnMessageExt:(int) deviceID Area:(const char *) areaName App:(const char *) appName Type:(int)type Msg:(const char *) message Length:(int)length;

/**
 * OnStatusMessage is called when the native layer has broadcase a text message to inform about a status change.
 *
 * @param message      The status as a text message.
 */
- (void) OnStatusMessage:(const char *) message;

@end


/**
* MessageObserver: Attachable to **DeviceInstance** objects in order to receive messages of a particular device.
*/
@protocol MessageObserver <NSObject>
/**
 * OnMessage is called whenever a text message has been received from a device.
 *
 * @param msg                           The corresponding message object of type MessageInstance
 * @param MESSAGE_INFO_ATTR_changed     Flags that indicate the object change.
 */
- (void) OnMessage:(id) msg Flags:(int) MESSAGE_INFO_ATTR_changed;

@end


/**
 * EnvironsDataObserver: Attachable to **Environs** objects in order to receive all data transmissions that the Environs instance received.
 */
@protocol EnvironsDataObserver <NSObject>
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
- (void) OnData:(int) nativeID Type:(int) type FileID:(int) fileID Descriptor:(const char *) descriptor Size:(int) size;
@end


/**
* DataObserver: Attachable to **DeviceInstance** objects in order to receive data transmissions of a particular device.
*/
@protocol DataObserver <NSObject>
/**
 * OnData is called whenever new binary data (files, buffers) has been received.
 * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
 *
 * @param fileData                  The corresponding file object of type FileInstance
 * @param FILE_INFO_ATTR_changed    Flags that indicate the object change.
 */
- (void) OnData:(id) fileData Flags:(int) FILE_INFO_ATTR_changed;

@end


/**
 * EnvironsSensorDataObserver: Attachable to **Environs** objects in order to receive all sensor data that the Environs instance received.
 */
@protocol EnvironsSensorDataObserver <NSObject>
/**
 * OnSensorData is called whenever new binary data (files, buffers) has been received.
 * Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
 *
 * @param nativeID      The native identifier that targets the device.
 * @param pack          The frame containing the sensor data
 */
- (void) OnSensorData:(int) nativeID Frame:(void *) pack;
@end


/**
* SensorObserver: Attachable to **DeviceInstance** objects in order to receive sensor data from a particular device.
*/
@protocol SensorObserver <NSObject>
/**
 * OnSensorData is called whenever new sensor data has been received.
 *
 * @param pack     The corresponding SensorFrame of sensor data
 */
- (void) OnSensorData:(void *) pack;

@end


/**
 * PortalObserver: Attachable to **IPortalInstance** objects in order to receive changes of a particular interactive portal.
 */
@protocol PortalObserver <NSObject>
/**
 * OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
 *
 * @param portal                    The PortalInstance object.
 * @param Environs_NOTIFY_PORTAL_	The notification (Environs.NOTIFY_PORTAL_*) that indicates the change.
 */
- (void) OnPortalChanged:(id) portal Notify:(int)Environs_NOTIFY_PORTAL_;

@end

#endif
