/* DO NOT EDIT THIS FILE - it is machine generated by j2c.jar */
/* Type to Name implementation for environs TypesSource */
#include "stdafx.h"
#include "Environs.Types.h"


/**
 * ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
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


#ifdef __cplusplus
namespace environs
{
#endif

	/// <summary>
	/// Events, Types, Messages for environs TypesSource 
	/// This class defines integer values which identifies status values, events, message types and so on delivered by Environs.
	/// </summary>


#ifndef CLI_CPP



#ifndef NDEBUG
	const char * MSG_PORTAL_Descriptions [ ] = { 
				"Portal requested",
				"Portal provided",
				"Stream portal provided",
				"Image portal provided",
				"Portal request failed",
				"Portal Stop",
				"Portal Stop ack",
				"Portal Stop failed",
				"Portal Start",
				"Portal Start ack",
				"Portal Start failed",
				"Portal pause",
				"Portal pause ack",
				"Portal pause failed",
				"Portal buffer full",
				"Portal buffer available again",
				"Portal i-frame requested",
	}; 
#endif

#endif

	int sensorFlags [ ] = { 
				ENVIRONS_SENSOR_FLAG_ACCELEROMETER,
				ENVIRONS_SENSOR_FLAG_MAGNETICFIELD,
				ENVIRONS_SENSOR_FLAG_GYROSCOPE,
				ENVIRONS_SENSOR_FLAG_ORIENTATION,
				ENVIRONS_SENSOR_FLAG_LIGHT,
				ENVIRONS_SENSOR_FLAG_LOCATION,
				ENVIRONS_SENSOR_FLAG_HEADING,
				ENVIRONS_SENSOR_FLAG_ALTIMETER,
				ENVIRONS_SENSOR_FLAG_MOTION_ATTITUTDE_ROTATION,
				ENVIRONS_SENSOR_FLAG_MOTION_GRAVITY_ACCELERATION,
				ENVIRONS_SENSOR_FLAG_MOTION_MAGNETICFIELD,
				ENVIRONS_SENSOR_FLAG_HEARTRATE,
				MAX_ENVIRONS_SENSOR_TYPE_VALUE,
	}; 

#ifndef CLI_CPP




	const char * resolveName ( int constToResolve )
	{
		switch ( constToResolve ) {
			case ( -1 ):
				return "STATUS_DISPOSED";
			case ( 0 ):
				return "STATUS_UNINITIALIZED";
			case ( 1 ):
				return "STATUS_DISPOSING";
			case ( 2 ):
				return "STATUS_INITIALIZING";
			case ( 3 ):
				return "STATUS_INITIALIZED";
			case ( 4 ):
				return "STATUS_STOPPED";
			case ( 5 ):
				return "STATUS_STOP_IN_PROGRESS";
			case ( 6 ):
				return "STATUS_STOPPING";
			case ( 7 ):
				return "STATUS_STARTING";
			case ( 8 ):
				return "STATUS_STARTED";
			case ( 9 ):
				return "STATUS_CONNECTED";
		} /// -> switch
		if ( constToResolve == ( 10 ) )
			return "ENVIRONS_MAX_ENVIRONS_INSTANCES_FULL";
		if ( constToResolve == ( -2 ) )
			return "ENVIRONS_OBJECT_DISPOSED_PLATFORM";


		switch ( constToResolve ) {
			case ( 0xFF00 ):
				return "MSG_NOTIFY_ID";
			case ( 0xFF0000 ):
				return "MSG_NOTIFY_CLASS";
		// Handshake states
			case ( 0x100 ):
				return "MSG_HANDSHAKE";
		// Former 'D'
			case ( MSG_HANDSHAKE | 0x10 ):
				return "MSG_HANDSHAKE_MAIN";
		// Former 'D'
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_MAIN | 1 ):
				return "MSG_HANDSHAKE_MAIN_REQ";
		// Former 'D'
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_MAIN | 2 ):
				return "MSG_HANDSHAKE_MAIN_ACK";
		// Former 'D'
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_MAIN | 3 ):
				return "MSG_HANDSHAKE_MAIN_FAIL";
		// Former 'D'
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_MAIN | 4 ):
				return "MSG_HANDSHAKE_MAIN_CLOSED";
			case ( MSG_HANDSHAKE | 0x20 ):
				return "MSG_HANDSHAKE_COMDAT";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_COMDAT | 1 ):
				return "MSG_HANDSHAKE_COMDAT_REQ";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_COMDAT | 2 ):
				return "MSG_HANDSHAKE_COMDAT_ACK";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_COMDAT | 3 ):
				return "MSG_HANDSHAKE_COMDAT_FAILED";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_COMDAT | 4 ):
				return "MSG_HANDSHAKE_COMDAT_CLOSED";
			case ( MSG_HANDSHAKE | 0x40 ):
				return "MSG_HANDSHAKE_PROC";
		// Former 'P'
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 1 ):
				return "MSG_HANDSHAKE_PORTS";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 2 ):
				return "MSG_HANDSHAKE_PORTS_ACK";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 3 ):
				return "MSG_HANDSHAKE_CONIG_REQ";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 4 ):
				return "MSG_HANDSHAKE_CONIG_RESP";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 5 ):
				return "MSG_HANDSHAKE_CONIG_RESP_ACK";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 0xA ):
				return "MSG_HANDSHAKE_CONNECTED";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 0xB ):
				return "MSG_HANDSHAKE_DISCONNECTED";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 0xC ):
				return "MSG_HANDSHAKE_PING";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 6 ):
				return "MSG_HANDSHAKE_UDP";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 7 ):
				return "MSG_HANDSHAKE_UDP_ACK";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 0xE ):
				return "MSG_HANDSHAKE_SUCCESS";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_PROC | 0xF ):
				return "MSG_HANDSHAKE_SUCCESS_ACK";
			case ( MSG_HANDSHAKE | 0x81 ):
				return "MSG_HANDSHAKE_SHORT_MESSAGE";
			case ( MSG_HANDSHAKE | 0x82 ):
				return "MSG_HANDSHAKE_SHORT_MESSAGE_ACK";
		} /// -> switch


		switch ( constToResolve ) {
			case ( (MSG_TYPE_HELO << 16) ):
				return "NOTIFY_TYPE_CONNECTION";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_MAIN_REQ ):
				return "NOTIFY_CONNECTION_MAIN_NEW";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_MAIN_ACK ):
				return "NOTIFY_CONNECTION_MAIN_ACK";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_MAIN_FAIL ):
				return "NOTIFY_CONNECTION_MAIN_FAILED";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_MAIN_CLOSED ):
				return "NOTIFY_CONNECTION_MAIN_CLOSED";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_COMDAT_REQ ):
				return "NOTIFY_CONNECTION_COMDAT_NEW";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_COMDAT_ACK ):
				return "NOTIFY_CONNECTION_COMDAT_ACK";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_COMDAT_FAILED ):
				return "NOTIFY_CONNECTION_COMDAT_FAILED";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_COMDAT_CLOSED ):
				return "NOTIFY_CONNECTION_COMDAT_CLOSED";
			case ( NOTIFY_TYPE_CONNECTION | 0xA ):
				return "NOTIFY_CONNECTION_DATA_ACK";
			case ( NOTIFY_TYPE_CONNECTION | 0xC ):
				return "NOTIFY_CONNECTION_DATA_CLOSED";
			case ( NOTIFY_TYPE_CONNECTION | 0xD ):
				return "NOTIFY_CONNECTION_PROGRESS";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_SUCCESS ):
				return "NOTIFY_CONNECTION_ESTABLISHED";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_SUCCESS_ACK ):
				return "NOTIFY_CONNECTION_ESTABLISHED_ACK";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_DISCONNECTED ):
				return "NOTIFY_CONNECTION_CLOSED";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_SHORT_MESSAGE ):
				return "NOTIFY_SHORT_MESSAGE";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_SHORT_MESSAGE_ACK ):
				return "NOTIFY_SHORT_MESSAGE_ACK";
		// Stream types
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0x200 ):
				return "DATA_STREAM";
			case ( 0x400 ):
				return "DATA_STREAM_IFRAME";
		} /// -> switch
		// 0x10


		switch ( constToResolve ) {
			case ( MSG_TYPE_IMAGE << 4 ):
				return "DATA_STREAM_IMAGE";
			case ( DATA_STREAM | DATA_STREAM_IMAGE | DATA_STREAM_INIT ):
				return "DATA_STREAM_IMAGE_INIT";
			case ( DATA_STREAM | DATA_STREAM_IMAGE | 8 ):
				return "DATA_STREAM_IMAGE_DATA";
			case ( DATA_STREAM_IMAGE_DATA | 2 ):
				return "DATA_STREAM_IMAGE_JPEG";
			case ( DATA_STREAM_IMAGE_DATA | 4 ):
				return "DATA_STREAM_IMAGE_PNG";
		} /// -> switch
		// 0x20


		switch ( constToResolve ) {
			case ( MSG_TYPE_STREAM << 4 ):
				return "DATA_STREAM_H264";
		// Initialization protocol version 1 packet with width and height
			case ( DATA_STREAM | DATA_STREAM_H264 | DATA_STREAM_INIT ):
				return "DATA_STREAM_H264_INIT";
		// Header packets of h264
			case ( DATA_STREAM | DATA_STREAM_H264 | 2 ):
				return "DATA_STREAM_H264_HDR";
			case ( DATA_STREAM | DATA_STREAM_H264 | 4 ):
				return "DATA_STREAM_H264_NAL";
			case ( DATA_STREAM | DATA_STREAM_H264 | 8 ):
				return "DATA_STREAM_H264_NALUS";
		} /// -> switch
		if ( constToResolve == ( 30000 ) )
			return "MAX_PORTAL_REQUEST_WAIT_TIME_MS";


		switch ( constToResolve ) {
			case ( 0x1000 ):
				return "PORTAL_TYPE_SCREEN";
			case ( 0x2000 ):
				return "PORTAL_TYPE_BACK_CAM";
			case ( 0x4000 ):
				return "PORTAL_TYPE_FRONT_CAM";
			case ( 0xF000 ):
				return "PORTAL_TYPE_MASK";
		} /// -> switch

		if ( constToResolve == ( ENVIRONS_OBJECT_DISPOSED ) )
			return "PORTAL_STATUS_DISPOSED";


		switch ( constToResolve ) {
			case ( 0x10 ):
				return "STREAMTYPE_IMAGES";
			case ( STREAMTYPE_IMAGES | 0x1 ):
				return "STREAMTYPE_IMAGES_JPEG";
			case ( STREAMTYPE_IMAGES | 0x2 ):
				return "STREAMTYPE_IMAGES_PNG";
			case ( 0x20 ):
				return "STREAMTYPE_VIDEO";
			case ( STREAMTYPE_VIDEO | 0x1 ):
				return "STREAMTYPE_VIDEO_H264";
		} /// -> switch

		if ( constToResolve == ( 0x1 ) )
			return "PORTAL_INFO_FLAG_LOCATION";


		switch ( constToResolve ) {
			case ( 0x2 ):
				return "PORTAL_INFO_FLAG_ANGLE";
			case ( 0x4 ):
				return "PORTAL_INFO_FLAG_SIZE";
		} /// -> switch
		if ( constToResolve == ( 0x300 ) )
			return "PORTAL_DIR_MASK";


		switch ( constToResolve ) {
			case ( 0x800 ):
				return "NOTIFY_PORTAL";
			case ( 0x100800 ):
				return "NOTIFY_PORTAL_INSTANCE";
		} /// -> switch
		if ( constToResolve == ( NOTIFY_PORTAL_INSTANCE | 0x1 ) )
			return "PORTAL_INSTANCE_FLAG_SURFACE_CHANGED";


		switch ( constToResolve ) {
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_REQUEST_ID ):
				return "MSG_PORTAL_REQUEST";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_ASK_FOR_REQUEST_ID ):
				return "MSG_PORTAL_ASK_FOR_REQUEST";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_PROVIDE_STREAM_ID ):
				return "MSG_PORTAL_PROVIDE_STREAM";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_PROVIDE_IMAGES_ID ):
				return "MSG_PORTAL_PROVIDE_IMAGES";
			case ( MSG_PORTAL_ERROR 	| MSG_PORTAL_REQUEST_FAIL_ID ):
				return "MSG_PORTAL_REQUEST_FAIL";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_STOP_ID ):
				return "MSG_PORTAL_STOP";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_STOP_ACK_ID ):
				return "MSG_PORTAL_STOP_ACK";
			case ( MSG_PORTAL_ERROR 	| MSG_PORTAL_STOP_FAIL_ID ):
				return "MSG_PORTAL_STOP_FAIL";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_START_ID ):
				return "MSG_PORTAL_START";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_START_ACK_ID ):
				return "MSG_PORTAL_START_ACK";
			case ( MSG_PORTAL_ERROR 	| MSG_PORTAL_START_FAIL_ID ):
				return "MSG_PORTAL_START_FAIL";
			case ( 11 ):
				return "MSG_PORTAL_PAUSE_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_PAUSE_ID ):
				return "MSG_PORTAL_PAUSE";
			case ( 12 ):
				return "MSG_PORTAL_PAUSE_ACK_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_PAUSE_ACK_ID ):
				return "MSG_PORTAL_PAUSE_ACK";
			case ( 13 ):
				return "MSG_PORTAL_PAUSE_FAIL_ID";
			case ( MSG_PORTAL_ERROR 	| MSG_PORTAL_PAUSE_FAIL_ID ):
				return "MSG_PORTAL_PAUSE_FAIL";
			case ( 14 ):
				return "MSG_PORTAL_BUFFER_FULL_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_BUFFER_FULL_ID ):
				return "MSG_PORTAL_BUFFER_FULL";
			case ( 15 ):
				return "MSG_PORTAL_BUFFER_AVAIL_AGAIN_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_BUFFER_AVAIL_AGAIN_ID ):
				return "MSG_PORTAL_BUFFER_AVAIL_AGAIN";
			case ( 16 ):
				return "MSG_PORTAL_IFRAME_REQUEST_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_IFRAME_REQUEST_ID ):
				return "MSG_PORTAL_IFRAME_REQUEST";
			case ( 16 + 1 ):
				return "MSG_PORTAL_MAX_COUNT";
		} /// -> switch


		switch ( constToResolve ) {
			case ( (MSG_TYPE_PORTAL << 16) ):
				return "NOTIFY_TYPE_PORTAL";
			case ( 0x80 ):
				return "NOTIFY_PORTAL_ESTABLISHED";
			case ( NOTIFY_TYPE_PORTAL | 0x81 ):
				return "NOTIFY_PORTAL_ESTABLISHED_RESOLUTION";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_REQUEST ):
				return "NOTIFY_PORTAL_REQUEST";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_ASK_FOR_REQUEST ):
				return "NOTIFY_PORTAL_ASK_REQUEST";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_STREAM | PORTAL_DIR_INCOMING ):
				return "NOTIFY_PORTAL_STREAM_INCOMING";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_IMAGES | PORTAL_DIR_INCOMING ):
				return "NOTIFY_PORTAL_IMAGES_INCOMING";
			case ( NOTIFY_PORTAL_STREAM_INCOMING | NOTIFY_PORTAL_IMAGES_INCOMING | NOTIFY_PORTAL_ESTABLISHED ):
				return "NOTIFY_PORTAL_INCOMING_ESTABLISHED";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_STREAM | PORTAL_DIR_OUTGOING ):
				return "NOTIFY_PORTAL_PROVIDE_STREAM_ACK";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_IMAGES | PORTAL_DIR_OUTGOING ):
				return "NOTIFY_PORTAL_PROVIDE_IMAGES_ACK";
			case ( NOTIFY_PORTAL_PROVIDE_STREAM_ACK | NOTIFY_PORTAL_PROVIDE_IMAGES_ACK | NOTIFY_PORTAL_ESTABLISHED ):
				return "NOTIFY_PORTAL_PROVIDER_READY";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_REQUEST_FAIL | PORTAL_DIR_INCOMING ):
				return "NOTIFY_PORTAL_REQUEST_FAIL";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_REQUEST_FAIL | PORTAL_DIR_OUTGOING ):
				return "NOTIFY_PORTAL_PROVIDE_FAIL";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_START_ACK ):
				return "NOTIFY_PORTAL_STREAM_STARTED";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_PAUSE_ACK ):
				return "NOTIFY_PORTAL_STREAM_PAUSED";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_STOP_ACK ):
				return "NOTIFY_PORTAL_STREAM_STOPPED";
			case ( NOTIFY_PORTAL_REQUEST | PORTAL_DIR_INCOMING | 0xFF ):
				return "NOTIFY_PORTAL_STREAM_RECEIVER_STARTED";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0xF00 ):
				return "MSG_OPTION_TYPE";
		// Transport options
			case ( MSG_OPTION_TRANSPORT | 0x1 ):
				return "MSG_OPT_TRANSP_TCP_PORTAL";
			case ( MSG_OPTION_SET | MSG_OPT_TRANSP_TCP_PORTAL ):
				return "MSG_OPT_TRANSP_TCP_PORTAL_SET";
			case ( MSG_OPTION_GET | MSG_OPT_TRANSP_TCP_PORTAL ):
				return "MSG_OPT_TRANSP_TCP_PORTAL_GET";
		// Portal options
			case ( MSG_OPTION_PORTAL | 0x1 ):
				return "MSG_OPT_PORTAL_CENTER";
			case ( MSG_OPTION_SET | MSG_OPT_PORTAL_CENTER ):
				return "MSG_OPT_PORTAL_CENTER_SET";
			case ( MSG_OPTION_GET | MSG_OPT_PORTAL_CENTER ):
				return "MSG_OPT_PORTAL_CENTER_GET";
			case ( MSG_OPTION_PORTAL | 0x2 ):
				return "MSG_OPT_PORTAL_WH";
			case ( MSG_OPTION_SET | MSG_OPT_PORTAL_WH ):
				return "MSG_OPT_PORTAL_WH_SET";
			case ( MSG_OPTION_PORTAL | 0x4 ):
				return "MSG_OPT_PORTAL_INFO";
			case ( MSG_OPTION_SET | MSG_OPT_PORTAL_INFO ):
				return "MSG_OPT_PORTAL_INFO_SET";
			case ( MSG_OPTION_GET | MSG_OPT_PORTAL_INFO ):
				return "MSG_OPT_PORTAL_INFO_GET";
		// Physical contact options
			case ( 0x40 ):
				return "MSG_OPTION_CONTACT";
			case ( MSG_OPTION_CONTACT | 0x1 ):
				return "MSG_OPT_CONTACT_DIRECT";
			case ( MSG_OPTION_SET | MSG_OPT_CONTACT_DIRECT ):
				return "MSG_OPT_CONTACT_DIRECT_SET";
			case ( MSG_OPTION_GET | MSG_OPT_CONTACT_DIRECT ):
				return "MSG_OPT_CONTACT_DIRECT_GET";
		} /// -> switch


		switch ( constToResolve ) {
			case ( (MSG_TYPE_OPTIONS << 16) ):
				return "NOTIFY_TYPE_OPTIONS";
			case ( NOTIFY_TYPE_OPTIONS | MSG_OPT_PORTAL_CENTER_SET ):
				return "NOTIFY_PORTAL_LOCATION_CHANGED";
			case ( NOTIFY_TYPE_OPTIONS | MSG_OPT_PORTAL_WH_SET ):
				return "NOTIFY_PORTAL_SIZE_CHANGED";
			case ( NOTIFY_TYPE_OPTIONS | MSG_OPT_CONTACT_DIRECT_SET ):
				return "NOTIFY_CONTACT_DIRECT_CHANGED";
		} /// -> switch
		// File types


		switch ( constToResolve ) {
			case ( NATIVE_FILE_TYPE ):
				return "NATIVE_FILE_TYPE_APP_DEFINED";
			case ( NATIVE_FILE_TYPE | 1 ):
				return "NATIVE_FILE_TYPE_EXT_DEFINED";
			case ( NATIVE_FILE_TYPE | 6 ):
				return "NATIVE_FILE_TYPE_PARTS";
			case ( NATIVE_FILE_TYPE | 0xF ):
				return "NATIVE_FILE_TYPE_ACK";
		} /// -> switch
		if ( constToResolve == ( MESSAGE_FROM_APP | 1 ) )
			return "MESSAGE_APP_STRING";


		switch ( constToResolve ) {
			case ( (MSG_TYPE_FILE << 16) ):
				return "NOTIFY_TYPE_FILE";
			case ( NOTIFY_TYPE_FILE | 0x20 ):
				return "NOTIFY_TYPE_FILE_PROGRESS";
			case ( NOTIFY_TYPE_FILE_PROGRESS | 1 ):
				return "NOTIFY_FILE_SEND_PROGRESS";
			case ( NOTIFY_TYPE_FILE_PROGRESS | 2 ):
				return "NOTIFY_FILE_RECEIVE_PROGRESS";
			case ( NOTIFY_TOUCHSOURCE | 2 ):
				return "NOTIFY_TOUCHSOURCE_STARTED";
			case ( NOTIFY_TOUCHSOURCE | 4 ):
				return "NOTIFY_TOUCHSOURCE_STOPPED";
			case ( NOTIFY_TOUCHSOURCE | 8 ):
				return "NOTIFY_TOUCHSOURCE_NOTAVAIL";
			case ( NOTIFY_TOUCHSOURCE | 9 ):
				return "NOTIFY_TOUCHSOURCE_FAILED";
		} /// -> switch


		switch ( constToResolve ) {
			case ( NATIVE_EVENT_ERROR_MISC | 3 ):
				return "NATIVE_EVENT_DATA_CON_FAILED";
			case ( NATIVE_EVENT_ERROR_MISC | 4 ):
				return "NATIVE_EVENT_TOUCH_SOURCE_FAILED";
		} /// -> switch


		switch ( constToResolve ) {
			case ( (MSG_TYPE_ENVIRONS << 16) ):
				return "NOTIFY_TYPE_ENVIRONS";
			case ( NOTIFY_TYPE_ENVIRONS | 0x100 ):
				return "NOTIFY_START";
			case ( NOTIFY_START | 1 ):
				return "NOTIFY_START_IN_PROGRESS";
			case ( NOTIFY_START | 2 ):
				return "NOTIFY_START_ENABLING_WIFI";
			case ( NOTIFY_START | 3 ):
				return "NOTIFY_START_STREAM_DECODER";
			case ( NOTIFY_START | 4 ):
				return "NOTIFY_START_INIT";
			case ( NOTIFY_START | 7 ):
				return "NOTIFY_START_INIT_FAILED";
			case ( NOTIFY_START | 8 ):
				return "NOTIFY_START_METHOD_FAILED";
			case ( NOTIFY_START | 9 ):
				return "NOTIFY_START_DECODER_FAILED";
			case ( NOTIFY_START | 10 ):
				return "NOTIFY_START_WIFI_FAILED";
			case ( NOTIFY_START | 11 ):
				return "NOTIFY_START_FAILED";
			case ( NOTIFY_START | 12 ):
				return "NOTIFY_START_INIT_SUCCESS";
			case ( NOTIFY_START | 13 ):
				return "NOTIFY_START_SUCCESS";
			case ( NOTIFY_START | 14 ):
				return "NOTIFY_START_LISTEN_SUCCESS";
			case ( NOTIFY_START | 15 ):
				return "NOTIFY_START_LISTENDA_SUCCESS";
			case ( NOTIFY_TYPE_ENVIRONS | 0x200 ):
				return "NOTIFY_STOP";
			case ( NOTIFY_STOP | 1 ):
				return "NOTIFY_STOP_BEGIN";
			case ( NOTIFY_STOP | 2 ):
				return "NOTIFY_STOP_IN_PROGRESS";
			case ( NOTIFY_STOP | 10 ):
				return "NOTIFY_STOP_FAILED";
			case ( NOTIFY_STOP | 11 ):
				return "NOTIFY_STOP_SUCCESS";
			case ( NOTIFY_STOP | 12 ):
				return "NOTIFY_STOP_RELEASED";
			case ( NOTIFY_TYPE_ENVIRONS | 0x400 ):
				return "NOTIFY_SOCKET";
			case ( NOTIFY_SOCKET | 7 ):
				return "NOTIFY_SOCKET_BIND_FAILED";
			case ( NOTIFY_SOCKET | 8 ):
				return "NOTIFY_SOCKET_LISTEN_FAILED";
			case ( NOTIFY_SOCKET | 9 ):
				return "NOTIFY_SOCKET_FAILED";
			case ( NOTIFY_TYPE_ENVIRONS | 0x480 ):
				return "NOTIFY_SETTINGS";
			case ( NOTIFY_SETTINGS | 0x1 ):
				return "NOTIFY_SETTINGS_CHANGED";
			case ( NOTIFY_TYPE_ENVIRONS | 0x800 ):
				return "NOTIFY_PAIRING";
			case ( NOTIFY_PAIRING | 1 ):
				return "NOTIFY_DEVICE_ON_SURFACE";
			case ( NOTIFY_PAIRING | 2 ):
				return "NOTIFY_DEVICE_NOT_ON_SURFACE";
			case ( NOTIFY_PAIRING | 8 ):
				return "NOTIFY_DEVICE_FLAGS_UPDATE";
		} /// -> switch
		if ( constToResolve == ( MSG_TYPE_SENSOR + 1 ) )
			return "MSG_TYPE_MAX_COUNT";
		/// Disable all devicelist notifications


		switch ( constToResolve ) {
			case ( 20 ):
				return "MEDIATOR_BROADCAST_PORTS_START";
			case ( 24 ):
				return "MEDIATOR_BROADCAST_PORTS_ABS_START";
			case ( 28 ):
				return "MEDIATOR_BROADCAST_PLATFORM_ABS_START";
			case ( 32 ):
				return "MEDIATOR_BROADCAST_DESC_ABS_START";
			case ( 40 ):
				return "MEDIATOR_BROADCAST_STATUS_DESC_ABS_START";
		} /// -> switch
		if ( constToResolve == ( 30 ) )
			return "MAX_NAMEPROPERTY";


		switch ( constToResolve ) {
			case ( DEVICEINFO_NATIVE_ID_START + 4 ):
				return "DEVICEINFO_IP_START";
			case ( DEVICEINFO_IP_START + 4 ):
				return "DEVICEINFO_IPe_START";
			case ( DEVICEINFO_IPe_START + 4 ):
				return "DEVICEINFO_TCP_PORT_START";
			case ( DEVICEINFO_TCP_PORT_START + 2 ):
				return "DEVICEINFO_UDP_PORT_START";
			case ( DEVICEINFO_UDP_PORT_START + 2 ):
				return "DEVICEINFO_UPDATES_START";
			case ( DEVICEINFO_UPDATES_START + 4 ):
				return "DEVICEINFO_PLATFORM_START";
			case ( DEVICEINFO_PLATFORM_START + 4 ):
				return "DEVICEINFO_BROADCAST_START";
			case ( DEVICEINFO_BROADCAST_START + 1 ):
				return "DEVICEINFO_UNAVAILABLE_START";
			case ( DEVICEINFO_UNAVAILABLE_START + 1 ):
				return "DEVICEINFO_ISCONNECTED_START";
			case ( DEVICEINFO_ISCONNECTED_START + 2 ):
				return "DEVICEINFO_HASAPPAREA_START";
			case ( DEVICEINFO_HASAPPAREA_START + 1 ):
				return "DEVICEINFO_DEVICENAME_START";
			case ( DEVICEINFO_DEVICENAME_START + (MAX_NAMEPROPERTY + 1) ):
				return "DEVICEINFO_AREANAME_START";
			case ( DEVICEINFO_AREANAME_START + (MAX_NAMEPROPERTY + 1) ):
				return "DEVICEINFO_APPNAME_START";
			case ( DEVICEINFO_APPNAME_START + (MAX_NAMEPROPERTY + 1) ):
				return "DEVICEINFO_FLAGS_START";
			case ( DEVICEINFO_FLAGS_START + 2 ):
				return "DEVICEINFO_OBJID_START";
		} /// -> switch


		switch ( constToResolve ) {
			case ( NOTIFY_TYPE_ENVIRONS | 0x1000 ):
				return "NOTIFY_MEDIATOR";
			case ( NOTIFY_MEDIATOR | 0x100 ):
				return "NOTIFY_MEDIATOR_SERVER";
			case ( NOTIFY_MEDIATOR | 1 ):
				return "NOTIFY_MEDIATOR_DEVICE_CHANGED";
			case ( NOTIFY_MEDIATOR | 2 ):
				return "NOTIFY_MEDIATOR_DEVICE_ADDED";
			case ( NOTIFY_MEDIATOR | 4 ):
				return "NOTIFY_MEDIATOR_DEVICE_REMOVED";
			case ( NOTIFY_MEDIATOR | 20 ):
				return "NOTIFY_MEDIATOR_SERVER_CONNECTED";
			case ( NOTIFY_MEDIATOR | 21 ):
				return "NOTIFY_MEDIATOR_SERVER_DISCONNECTED";
			case ( NOTIFY_MEDIATOR | 51 ):
				return "NOTIFY_MEDIATOR_DEVICELISTS_UPDATE_AVAILABLE";
			case ( NOTIFY_MEDIATOR | 52 ):
				return "NOTIFY_MEDIATOR_DEVICELISTS_CHANGED";
			case ( NOTIFY_MEDIATOR | 11 ):
				return "NOTIFY_MEDIATOR_MED_CHANGED";
			case ( NOTIFY_MEDIATOR_DEVICE_CHANGED | NOTIFY_MEDIATOR_SERVER ):
				return "NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED";
			case ( NOTIFY_MEDIATOR_DEVICE_ADDED | NOTIFY_MEDIATOR_SERVER ):
				return "NOTIFY_MEDIATOR_SRV_DEVICE_ADDED";
			case ( NOTIFY_MEDIATOR_DEVICE_REMOVED | NOTIFY_MEDIATOR_SERVER ):
				return "NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED";
			case ( NOTIFY_MEDIATOR | 22 | NOTIFY_MEDIATOR_SERVER ):
				return "NOTIFY_MEDIATOR_SRV_STUNT_REG_REQ";
			case ( NOTIFY_MEDIATOR | 41 ):
				return "NOTIFY_MEDIATOR_SERVER_PASSWORD_FAIL";
			case ( NOTIFY_MEDIATOR | 42 ):
				return "NOTIFY_MEDIATOR_SERVER_PASSWORD_MISSING";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0x8 ):
				return "DEVICEFLAGS_INTERNAL_MESSAGE_READY";
			case ( 0xFF ):
				return "DEVICEFLAGS_INTERNAL_NOTIFY_MASK";
			case ( 0x0100 ):
				return "DEVICEFLAGS_INTERNAL_CP_NATIVE_READY";
			case ( 0x0200 ):
				return "DEVICEFLAGS_INTERNAL_CP_PLATFORM_READY";
			case ( 0x0400 ):
				return "DEVICEFLAGS_INTERNAL_CP_OBSERVER_READY";
			case ( 0x0800 ):
				return "DEVICEFLAGS_INTERNAL_CP_MESSAGE_READY";
		} /// -> switch


		switch ( constToResolve ) {
			case ( NOTIFY_TYPE_ENVIRONS | 0x2000 ):
				return "NOTIFY_NETWORK";
			case ( NOTIFY_NETWORK | 0x1 ):
				return "NOTIFY_NETWORK_CHANGED";
		} /// -> switch


		switch ( constToResolve ) {
			case ( NOTIFY_TYPE_ENVIRONS | 0x4000 ):
				return "NOTIFY_TRACKER";
			case ( NOTIFY_TRACKER | 0x1 ):
				return "NOTIFY_TRACKER_ENABLED";
			case ( NOTIFY_TRACKER | 0x2 ):
				return "NOTIFY_TRACKER_CHANGED";
			case ( NOTIFY_TRACKER | 0x4 ):
				return "NOTIFY_TRACKER_DISABLED";
			case ( NOTIFY_TRACKER | NOTIFY_TRACKER_FAILED_FLAG ):
				return "NOTIFY_TRACKER_ENABLE_FAILED";
			case ( NOTIFY_TRACKER | 0x10 ):
				return "NOTIFY_TRACKER_STATE_INIT_SENSOR";
			case ( NOTIFY_TRACKER_STATE_INIT_SENSOR | NOTIFY_TRACKER_FAILED_FLAG ):
				return "NOTIFY_TRACKER_STATE_INIT_SENSOR_FAILED";
			case ( NOTIFY_TRACKER | 0x20 ):
				return "NOTIFY_TRACKER_STATE_START";
			case ( NOTIFY_TRACKER | NOTIFY_TRACKER_STATE_START | NOTIFY_TRACKER_FAILED_FLAG ):
				return "NOTIFY_TRACKER_STATE_START_FAILED";
			case ( NOTIFY_TRACKER | 0x40 ):
				return "NOTIFY_TRACKER_STATE_STOP";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 'D' ):
				return "DEVICE_TYPE_DISPLAY";
			case ( 'M' ):
				return "DEVICE_TYPE_MULTITACTION";
			case ( 'R' ):
				return "DEVICE_TYPE_SURFACE1";
			case ( 'S' ):
				return "DEVICE_TYPE_SURFACE2";
			case ( 'T' ):
				return "DEVICE_TYPE_TABLET";
			case ( 'U' ):
				return "DEVICE_TYPE_UNKNOWN";
			case ( 'P' ):
				return "DEVICE_TYPE_SMARTPHONE";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0x8000 ):
				return "DEVICE_ACTIVITY_LISTENER_CLOSED";
			case ( 0x10000 ):
				return "DEVICE_ACTIVITY_MAIN_INIT";
			case ( 0x20000 ):
				return "DEVICE_ACTIVITY_COMDAT_INIT";
			case ( 0x40000 ):
				return "DEVICE_ACTIVITY_UDP_INIT";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 65535 ):
				return "MEDIATOR_BUFFER_SIZE_MAX";
			case ( (350 * 1024) ):
				return "MEDIATOR_REC_BUFFER_SIZE_MAX_MOBILE";
			case ( (650 * 1024) ):
				return "MEDIATOR_REC_BUFFER_SIZE_MAX_FULL";
		} /// -> switch
		if ( constToResolve == ( (40 * 1024 * 1024) ) )
			return "ENVIRONS_SEND_SIZE_MAX";


		switch ( constToResolve ) {
			case ( 0x3 ):
				return "PORTAL_BUFFERTYPE_BGRA";
			case ( 0x12 ):
				return "PORTAL_BUFFERTYPE_YV12";
			case ( 0x14 ):
				return "PORTAL_BUFFERTYPE_YUV2";
			case ( 0x3000 ):
				return "PORTAL_BUFFERTYPE_CVPIXELBUFFER_IOSX";
		} /// -> switch
		if ( constToResolve == ( -3 ) )
			return "NETWORK_CONNECTION_TRIGGER_UPDATE";
		if ( constToResolve == ( 0x41 ) )
			return "DEVICE_INFO_ATTR_DEVICE_TYPE";
		if ( constToResolve == ( 0x80000 ) )
			return "FILE_INFO_ATTR_RECEIVE_PROGRESS";
		if ( constToResolve == ( 0x200000 ) )
			return "MESSAGE_INFO_ATTR_CREATED";
		if ( constToResolve == ( 60 ) )
			return "ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT";
		if ( constToResolve == ( 1200000 ) )
			return "MAX_TCP_SEND_PACKET_SIZE";


		switch ( constToResolve ) {
			case ( 0x1001 ):
				return "ENVIRONS_PLATFORMS_MSSUR01";
			case ( 0x1002 ):
				return "ENVIRONS_PLATFORMS_SAMSUR40";
			case ( 0x2011 ):
				return "ENVIRONS_PLATFORMS_IPAD1";
			case ( 0x2021 ):
				return "ENVIRONS_PLATFORMS_IPAD2";
			case ( 0x2022 ):
				return "ENVIRONS_PLATFORMS_IPAD2MINI";
			case ( 0x2031 ):
				return "ENVIRONS_PLATFORMS_IPAD3";
			case ( 0x2041 ):
				return "ENVIRONS_PLATFORMS_IPAD4";
			case ( 0x2042 ):
				return "ENVIRONS_PLATFORMS_IPAD4AIR";
			case ( 0x2043 ):
				return "ENVIRONS_PLATFORMS_IPAD4MINI";
			case ( 0x2044 ):
				return "ENVIRONS_PLATFORMS_IPAD4MINI3";
			case ( 0x2051 ):
				return "ENVIRONS_PLATFORMS_IPAD5AIR2";
			case ( 0x4041 ):
				return "ENVIRONS_PLATFORMS_IPHONE4";
			case ( 0x4051 ):
				return "ENVIRONS_PLATFORMS_IPHONE5";
			case ( 0x4061 ):
				return "ENVIRONS_PLATFORMS_IPHONE6";
			case ( 0x4062 ):
				return "ENVIRONS_PLATFORMS_IPHONE6P";
			case ( 0x8055 ):
				return "ENVIRONS_PLATFORMS_MULTITACTION55";
			case ( 0x10010 ):
				return "ENVIRONS_PLATFORMS_MACBOOK_FLAG";
			case ( 0x10020 ):
				return "ENVIRONS_PLATFORMS_MACMINI_FLAG";
			case ( 0x40100 ):
				return "ENVIRONS_PLATFORMS_RASPBERRY";
			case ( 0x20050 ):
				return "ENVIRONS_PLATFORMS_WINDOWSVISTA";
			case ( 0x20060 ):
				return "ENVIRONS_PLATFORMS_WINDOWSXP";
			case ( 0x20070 ):
				return "ENVIRONS_PLATFORMS_WINDOWS7";
			case ( 0x20080 ):
				return "ENVIRONS_PLATFORMS_WINDOWS8";
			case ( 0x20100 ):
				return "ENVIRONS_PLATFORMS_WINDOWS10";
			case ( 0x100000 ):
				return "ENVIRONS_PLATFORMS_TABLET_FLAG";
			case ( 0x400000 ):
				return "ENVIRONS_PLATFORMS_TABLETOP_FLAG";
			case ( 0x800000 ):
				return "ENVIRONS_PLATFORMS_DISPLAY_FLAG";
			case ( 0x1000000 ):
				return "ENVIRONS_PLATFORMS_LOCATIONNODE_FLAG";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 2048 ):
				return "ENVIRONS_DEVICES_KEYSIZE";
			case ( (1 << 24) ):
				return "ENVIRONS_CRYPT_PAD_OAEP";
			case ( (2 << 24) ):
				return "ENVIRONS_CRYPT_PAD_PKCS1";
			case ( (4 << 24) ):
				return "ENVIRONS_CRYPT_PAD_PKCS1SHA1";
			case ( (8 << 24) ):
				return "ENVIRONS_CRYPT_PAD_PKCS1SHA256";
		} /// -> switch
		if ( constToResolve == ( 0x1FFFF ) )
			return "MEDIATOR_CLIENT_MAX_BUFFER_SIZE";
		if ( constToResolve == ( MEDIATOR_CLIENT_MAX_BUFFER_SIZE ) )
			return "DEVICE_HANDSHAKE_BUFFER_MAX_SIZE";
		if ( constToResolve == ( RENDER_CALLBACK_TYPE_AVCONTEXT ) )
			return "DECODER_AVCONTEXT_TYPE_AVCONTEXT";
		if ( constToResolve == ( ENVIRONS_SENSOR_FLAG_HEARTRATE ) )
			return "MAX_ENVIRONS_SENSOR_TYPE_VALUE";



		return "Unknown";
	} /// -> get()



#endif



#ifdef __cplusplus
} /// -> namespace environs
#endif



