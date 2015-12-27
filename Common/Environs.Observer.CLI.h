/**
 * Environs Observer CLI
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

#include "Environs.Cli.Forwards.h"
#include "Environs.Platform.Decls.h"
#include "Interop/jni.h"

#ifndef INCLUDE_HCM_ENVIRONS_OBSERVERS_CLI_H
#define INCLUDE_HCM_ENVIRONS_OBSERVERS_CLI_H

#ifdef CLI_CPP

namespace environs
{
	typedef System::Collections::Specialized::NotifyCollectionChangedEventHandler	ListObserver;


	/**
	* OnPortalRequestOrProvided is called when a portal request from another devices came in, or when a portal has been provided by another device.
	*
	* @param portal 		The PortalInstance object.
	*/
	public delegate void PortalRequestOrProvided ( PortalInstance ^ portal );

	/**
	* DeviceObserver: Attachable to **DeviceInstance** objects in order to receive changes of a particular device.
	*/
	/**
	* DeviceObserver is called whenever the members of a DeviceInstance has changed.&nbsp;
	* The DEVICE_INFO_ATTR_changed parameter provides a bit set which indicates the member that has changed.
	*
	* @param device                    The DeviceInstance object that sends this notification.
	* @param DEVICE_INFO_ATTR_changed  The notification depends on the source object. If the sender is a DeviceItem, then the notification are flags.
	*/
	public delegate void DeviceObserver ( DeviceInstance ^device, int Environs_NOTIFY_ );


	/**
	* MessageObserver: Attachable to **DeviceInstance** objects in order to receive messages of a particular device.
	*/
	/**
	* OnMessage is called whenever a text message has been received from a device.
	*
	* @param msg                           The corresponding message object of type MessageInstance
	* @param MESSAGE_INFO_ATTR_changed     Flags that indicate the object change.
	*/
	public delegate void MessageObserver ( MessageInstance ^messageInst, int MESSAGE_INFO_ATTR_changed );


	/**
	* DataObserver: Attachable to **DeviceInstance** objects in order to receive data transmissions of a particular device.
	*/
	/**
	* OnData is called whenever new binary data (files, buffers) has been received.
	* Pass deviceID/fileID to Environs.GetFile() in order to retrieve a byte array with the content received.
	*
	* @param fileData                  The corresponding file object of type FileInstance
	* @param FILE_INFO_ATTR_changed    Flags that indicate the object change.
	*/
	public delegate void DataObserver ( FileInstance ^fileData, int FILE_INFO_ATTR_changed );


	/**
	* SensorObserver: Attachable to **DeviceInstance** objects in order to receive sensor data from a particular device.
	*/
	/**
	* OnSensorData is called whenever new sensor data has been received.
	*
	* @param pack     The corresponding SensorFrame of sensor data
	*/
	public delegate void SensorObserver ( environs::SensorFrame ^ pack );


	/**
	* OnStatus is called whenever the framework status changes.&nbsp;
	*
	* @param status      A status constant of type STATUS_*
	*/
	public delegate void StatusObserver ( int Environs_STATUS_ );


	/**
	* PortalObserver: Attachable to **PortalInstance** objects in order to receive changes of a particular interactive portal.
	*/
	/**
	* OnPortalChanged is called when the portal status has changed, e.g. stream has started, stopped, disposed, etc..
	*
	* @param portal                    The PortalInstance object.
	* @param Environs_NOTIFY_PORTAL_	The notification (Environs.NOTIFY_PORTAL_*) that indicates the change.
	*/
	public delegate void PortalObserver ( PortalInstance ^portal, int Environs_NOTIFY_ );



	/// <summary>
	/// The delegate type for signaling that environs has been initialized and is ready to be used by applications.
	/// </summary>
	public delegate void EnvironsInitializedHandler ();



	/// <summary>
	/// This handler type is called (by the native layer) when an rgba frame for an incoming portal is available.
	/// </summary>
	/// <param name="argumentType">A value of type RENDER_CALLBACK_TYPE_* that describes the callbackArgument. This type can be requested when calling SetRenderCallback()</param>
	/// <param name="surface"></param>
	/// <param name="callbackArgument"></param>
#if !WINDOWS_PHONE
	//[ SuppressUnmanagedCodeSecurityAttribute () ]
#endif
	[ UnmanagedFunctionPointer ( CallingConvention::Cdecl ) ]
	public delegate void PortalSinkSource ( int argumentType, IntPtr surface, IntPtr callbackArgument );
}


#endif

#endif

