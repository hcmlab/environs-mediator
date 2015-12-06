/* DO NOT EDIT THIS FILE - it is machine generated by j2c.jar (see Environs.framework) */
/* Type to Name implementation for environs Types */
#include "stdafx.h"
#include "Environs.Types.h"


/**
 * Types - This class defines integer values which are used as constant
 * for status values, events, message types and so on
 * delivered by the environment.
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
/** 
 * Types - This class defines integer values which identifies status values, events, message types and so on delivered by the environment.
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 * 
 */


#ifdef __cplusplus
namespace environs
{
#endif

	/// <summary>
	/// Events, Types, Messages for environs Types 
	/// This class defines integer values which identifies status values, events, message types and so on delivered by Environs.
	/// </summary>


	const char * resolveName ( int constToResolve )
	{
		switch ( constToResolve ) {
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
				return "STATUS_STOPPING";
			case ( 6 ):
				return "STATUS_STARTING";
			case ( 7 ):
				return "STATUS_STARTED";
			case ( 8 ):
				return "STATUS_CONNECTED";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 10 ):
				return "ENVIRONS_MAX_ENVIRONS_INSTANCES";
			case ( -1 ):
				return "ENVIRONS_DISPLAY_UNINITIALIZED_VALUE";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "DEVICE_STATUS_DELETEABLE";
			case ( 2 ):
				return "DEVICE_STATUS_CREATED";
			case ( 3 ):
				return "DEVICE_STATUS_CONNECT_IN_PROGRESS";
			case ( 4 ):
				return "DEVICE_STATUS_CONNECTED";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "SOURCE_NATIVE";
			case ( 1 ):
				return "SOURCE_PLATFORM";
			case ( 2 ):
				return "SOURCE_DEVICE";
			case ( 3 ):
				return "SOURCE_APPLICATION";
		} /// -> switch


		switch ( constToResolve ) {
			case ( -1 ):
				return "ENVIRONS_OBJECT_DISPOSED";
			case ( -2 ):
				return "ENVIRONS_OBJECT_DISPOSED_PLATFORM";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0xFF00 ):
				return "MSG_NOTIFY_ID";
			case ( 0xFF0000 ):
				return "MSG_NOTIFY_CLASS";
		} /// -> switch

		if ( constToResolve == ( 0 ) )
			return "MSG_TYPE_HELO";


		switch ( constToResolve ) {
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
				return "MSG_HANDSHAKE_BULK";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_BULK | 1 ):
				return "MSG_HANDSHAKE_BULK_REQ";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_BULK | 2 ):
				return "MSG_HANDSHAKE_BULK_ACK";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_BULK | 3 ):
				return "MSG_HANDSHAKE_BULK_FAILED";
			case ( MSG_HANDSHAKE | MSG_HANDSHAKE_BULK | 4 ):
				return "MSG_HANDSHAKE_BULK_CLOSED";
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
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_BULK_REQ ):
				return "NOTIFY_CONNECTION_BULK_NEW";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_BULK_ACK ):
				return "NOTIFY_CONNECTION_BULK_ACK";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_BULK_FAILED ):
				return "NOTIFY_CONNECTION_BULK_FAILED";
			case ( NOTIFY_TYPE_CONNECTION | MSG_HANDSHAKE_BULK_CLOSED ):
				return "NOTIFY_CONNECTION_BULK_CLOSED";
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
			case ( 1 ):
				return "DATA_STREAM_INIT";
			case ( 0x400 ):
				return "DATA_STREAM_IFRAME";
		} /// -> switch

		if ( constToResolve == ( 1 ) )
			return "MSG_TYPE_IMAGE";
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

		if ( constToResolve == ( 2 ) )
			return "MSG_TYPE_STREAM";
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

		if ( constToResolve == ( 6 ) )
			return "MAX_PORTAL_STREAMS_A_DEVICE";


		switch ( constToResolve ) {
			case ( 2 ):
				return "MAX_PORTAL_CONTEXT_WORKERS";
			case ( 6 ):
				return "MAX_PORTAL_OVERLAYS";
			case ( 5 ):
				return "MAX_PORTAL_GENERATOR_SLOTS";
			case ( 30000 ):
				return "MAX_PORTAL_REQUEST_WAIT_TIME_MS";
		} /// -> switch

		if ( constToResolve == ( 0 ) )
			return "PORTAL_TYPE_ANY";


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
			case ( 0 ):
				return "PORTAL_STATUS_CREATED";
			case ( 1 ):
				return "PORTAL_STATUS_CREATED_FROM_REQUEST";
			case ( 2 ):
				return "PORTAL_STATUS_CREATED_ASK_REQUEST";
			case ( 4 ):
				return "PORTAL_STATUS_ESTABLISHED";
			case ( 6 ):
				return "PORTAL_STATUS_STARTED";
		} /// -> switch

		if ( constToResolve == ( 0 ) )
			return "STREAMTYPE_UNKNOWN";


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

		if ( constToResolve == ( 5 ) )
			return "MSG_TYPE_PORTAL";
		if ( constToResolve == ( 0x400 ) )
			return "MSG_PORTAL_ERROR";


		switch ( constToResolve ) {
			case ( 0x200 ):
				return "PORTAL_DIR_INCOMING";
			case ( 0x100 ):
				return "PORTAL_DIR_OUTGOING";
			case ( 0x300 ):
				return "PORTAL_DIR_MASK";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0x800 ):
				return "NOTIFY_PORTAL";
			case ( 0x100800 ):
				return "NOTIFY_PORTAL_INSTANCE";
		} /// -> switch
		if ( constToResolve == ( NOTIFY_PORTAL_INSTANCE | 0x1 ) )
			return "PORTAL_INSTANCE_FLAG_SURFACE_CHANGED";


		switch ( constToResolve ) {
			case ( 0 ):
				return "MSG_PORTAL_REQUEST_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_REQUEST_ID ):
				return "MSG_PORTAL_REQUEST";
			case ( 1 ):
				return "MSG_PORTAL_ASK_FOR_REQUEST_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_ASK_FOR_REQUEST_ID ):
				return "MSG_PORTAL_ASK_FOR_REQUEST";
			case ( 2 ):
				return "MSG_PORTAL_PROVIDE_STREAM_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_PROVIDE_STREAM_ID ):
				return "MSG_PORTAL_PROVIDE_STREAM";
			case ( 3 ):
				return "MSG_PORTAL_PROVIDE_IMAGES_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_PROVIDE_IMAGES_ID ):
				return "MSG_PORTAL_PROVIDE_IMAGES";
			case ( 4 ):
				return "MSG_PORTAL_REQUEST_FAIL_ID";
			case ( MSG_PORTAL_ERROR 	| MSG_PORTAL_REQUEST_FAIL_ID ):
				return "MSG_PORTAL_REQUEST_FAIL";
			case ( 5 ):
				return "MSG_PORTAL_STOP_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_STOP_ID ):
				return "MSG_PORTAL_STOP";
			case ( 6 ):
				return "MSG_PORTAL_STOP_ACK_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_STOP_ACK_ID ):
				return "MSG_PORTAL_STOP_ACK";
			case ( 7 ):
				return "MSG_PORTAL_STOP_FAIL_ID";
			case ( MSG_PORTAL_ERROR 	| MSG_PORTAL_STOP_FAIL_ID ):
				return "MSG_PORTAL_STOP_FAIL";
			case ( 8 ):
				return "MSG_PORTAL_START_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_START_ID ):
				return "MSG_PORTAL_START";
			case ( 9 ):
				return "MSG_PORTAL_START_ACK_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_START_ACK_ID ):
				return "MSG_PORTAL_START_ACK";
			case ( 10 ):
				return "MSG_PORTAL_START_FAIL_ID";
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

		if ( constToResolve == ( 6 ) )
			return "MSG_TYPE_OPTIONS";


		switch ( constToResolve ) {
			case ( 7 ):
				return "MSG_TYPE_OPTIONS_RESPONSE";
			case ( 0xF00 ):
				return "MSG_OPTION_TYPE";
			case ( 0x100 ):
				return "MSG_OPTION_SET";
			case ( 0x200 ):
				return "MSG_OPTION_GET";
		// Transport options
			case ( 0x10 ):
				return "MSG_OPTION_TRANSPORT";
			case ( MSG_OPTION_TRANSPORT | 0x1 ):
				return "MSG_OPT_TRANSP_TCP_PORTAL";
			case ( MSG_OPTION_SET | MSG_OPT_TRANSP_TCP_PORTAL ):
				return "MSG_OPT_TRANSP_TCP_PORTAL_SET";
			case ( MSG_OPTION_GET | MSG_OPT_TRANSP_TCP_PORTAL ):
				return "MSG_OPT_TRANSP_TCP_PORTAL_GET";
		// Portal options
			case ( 0x20 ):
				return "MSG_OPTION_PORTAL";
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
		if ( constToResolve == ( 3 ) )
			return "MSG_TYPE_FILE";
		// File types

		if ( constToResolve == ( 0x400 ) )
			return "NATIVE_FILE_TYPE";


		switch ( constToResolve ) {
			case ( NATIVE_FILE_TYPE ):
				return "NATIVE_FILE_TYPE_APP_DEFINED";
			case ( NATIVE_FILE_TYPE | 1 ):
				return "NATIVE_FILE_TYPE_EXT_DEFINED";
			case ( NATIVE_FILE_TYPE | 6 ):
				return "NATIVE_FILE_TYPE_CHUNKED";
			case ( NATIVE_FILE_TYPE | 0xF ):
				return "NATIVE_FILE_TYPE_ACK";
		} /// -> switch
		if ( constToResolve == ( 4 ) )
			return "MSG_TYPE_MESSAGE";


		switch ( constToResolve ) {
			case ( 0x800 ):
				return "MESSAGE_FROM_APP";
			case ( MESSAGE_FROM_APP | 1 ):
				return "MESSAGE_APP_STRING";
		} /// -> switch


		switch ( constToResolve ) {
			case ( (MSG_TYPE_FILE << 16) ):
				return "NOTIFY_TYPE_FILE";
			case ( NOTIFY_TYPE_FILE | 0x20 ):
				return "NOTIFY_TYPE_FILE_PROGRESS";
			case ( NOTIFY_TYPE_FILE_PROGRESS | 1 ):
				return "NOTIFY_FILE_SEND_PROGRESS";
			case ( NOTIFY_TYPE_FILE_PROGRESS | 2 ):
				return "NOTIFY_FILE_RECEIVE_PROGRESS";
			case ( 0x40 ):
				return "NOTIFY_TOUCHSOURCE";
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
			case ( 0 ):
				return "INPUT_TYPE_CURSOR";
			case ( 1 ):
				return "INPUT_TYPE_FINGER";
			case ( 2 ):
				return "INPUT_TYPE_PEN";
			case ( 4 ):
				return "INPUT_TYPE_MARKER";
			case ( 8 ):
				return "INPUT_TYPE_BLOB";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0x80 ):
				return "NATIVE_EVENT_ERROR_MISC";
			case ( NATIVE_EVENT_ERROR_MISC | 3 ):
				return "NATIVE_EVENT_DATA_CON_FAILED";
			case ( NATIVE_EVENT_ERROR_MISC | 4 ):
				return "NATIVE_EVENT_TOUCH_SOURCE_FAILED";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 1 ):
				return "INPUT_STATE_ADD";
			case ( 2 ):
				return "INPUT_STATE_CHANGE";
			case ( 3 ):
				return "INPUT_STATE_NOCHANGE";
			case ( 4 ):
				return "INPUT_STATE_DROP";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "INPUTSOURCE_COMMAND_INIT";
			case ( 1 ):
				return "INPUTSOURCE_COMMAND_ADD";
			case ( 2 ):
				return "INPUTSOURCE_COMMAND_CHANGE";
			case ( 4 ):
				return "INPUTSOURCE_COMMAND_DROP";
			case ( 6 ):
				return "INPUTSOURCE_COMMAND_CANCEL";
			case ( 8 ):
				return "INPUTSOURCE_COMMAND_FLUSH";
			case ( 0x80 ):
				return "INPUTSOURCE_COMMAND_FOLLOWUP";
		} /// -> switch

		if ( constToResolve == ( 8 ) )
			return "MSG_TYPE_ENVIRONS";
		if ( constToResolve == ( MSG_TYPE_ENVIRONS + 1 ) )
			return "MSG_TYPE_MAX_COUNT";


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
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "MEDIATOR_FILTER_NONE";
			case ( 1 ):
				return "MEDIATOR_FILTER_AREA";
			case ( 2 ):
				return "MEDIATOR_FILTER_AREA_AND_APP";
		/// Disable all devicelist notifications
			case ( 8 ):
				return "MEDIATOR_FILTER_ALL";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "DEVICEINFO_DEVICE_MEDIATOR";
			case ( 1 ):
				return "DEVICEINFO_DEVICE_BROADCAST";
			case ( 2 ):
				return "DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 11 ):
				return "MEDIATOR_BROADCAST_DEVICETYPE_START";
			case ( 12 ):
				return "MEDIATOR_BROADCAST_DEVICEID_START";
			case ( 20 ):
				return "MEDIATOR_BROADCAST_PORTS_START";
			case ( 24 ):
				return "MEDIATOR_BROADCAST_PLATFORM_START";
			case ( 28 ):
				return "MEDIATOR_BROADCAST_DESC_START";
		} /// -> switch
		if ( constToResolve == ( 28 ) )
			return "TYPES_SEPERATOR_1_ENVIRONS";
		if ( constToResolve == ( 28 ) )
			return "MEDIATOR_BROADCAST_SPARE_ID_LEN";
		if ( constToResolve == ( 30 ) )
			return "MAX_NAMEPROPERTY";


		switch ( constToResolve ) {
			case ( 0 ):
				return "DEVICEINFO_DEVICEID_START";
			case ( 4 ):
				return "DEVICEINFO_NATIVE_ID_START";
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
				return "DEVICEINFO_DEVICETYPE_START";
			case ( DEVICEINFO_DEVICETYPE_START + 1 ):
				return "DEVICEINFO_DEVICENAME_START";
			case ( DEVICEINFO_DEVICENAME_START + (MAX_NAMEPROPERTY + 1) ):
				return "DEVICEINFO_AREANAME_START";
			case ( DEVICEINFO_AREANAME_START + (MAX_NAMEPROPERTY + 1) ):
				return "DEVICEINFO_APPNAME_START";
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
			case ( NOTIFY_TYPE_ENVIRONS | 0x2000 ):
				return "NOTIFY_NETWORK";
			case ( NOTIFY_NETWORK | 0x1 ):
				return "NOTIFY_NETWORK_CHANGED";
		} /// -> switch


		switch ( constToResolve ) {
			case ( NOTIFY_TYPE_ENVIRONS | 0x4000 ):
				return "NOTIFY_TRACKER";
			case ( 0x8 ):
				return "NOTIFY_TRACKER_FAILED_FLAG";
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
			case ( 0 ):
				return "DEVICELIST_QUEUE_COMMAND_RELOAD";
			case ( 1 ):
				return "DEVICELIST_QUEUE_COMMAND_CLEAR";
			case ( 2 ):
				return "DEVICELIST_QUEUE_COMMAND_APPEND";
			case ( 3 ):
				return "DEVICELIST_QUEUE_COMMAND_UPDATE";
			case ( 4 ):
				return "DEVICELIST_QUEUE_COMMAND_INSERT_AT";
			case ( 5 ):
				return "DEVICELIST_QUEUE_COMMAND_REMOVE_AT";
			case ( 6 ):
				return "DEVICELIST_QUEUE_COMMAND_INSERT_CALL";
			case ( 7 ):
				return "DEVICELIST_QUEUE_COMMAND_DISPOSE_LIST";
			case ( 8 ):
				return "DEVICELIST_QUEUE_COMMAND_LOCK";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "DISPLAY_ORIENTATION_LANDSCAPE";
			case ( 1 ):
				return "DISPLAY_ORIENTATION_PORTRAIT";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0x1 ):
				return "DEVICE_ACTIVITY_MAIN_CONNECTED";
			case ( 0x2 ):
				return "DEVICE_ACTIVITY_BULK_CONNECTED";
			case ( 0x4 ):
				return "DEVICE_ACTIVITY_UDP_CONNECTED";
			case ( 0x10 ):
				return "DEVICE_ACTIVITY_CONNECTED";
			case ( 0x100 ):
				return "DEVICE_ACTIVITY_REQUESTOR";
			case ( 0x200 ):
				return "DEVICE_ACTIVITY_RESPONDER";
		} /// -> switch
		if ( constToResolve == ( 65535 ) )
			return "MEDIATOR_BUFFER_SIZE_MAX";
		if ( constToResolve == ( (40 * 1024 * 1024) ) )
			return "ENVIRONS_SEND_SIZE_MAX";


		switch ( constToResolve ) {
			case ( -1 ):
				return "RECOGNIZER_GIVE_BACK_INPUTS";
			case ( 0 ):
				return "RECOGNIZER_REJECT";
			case ( 1 ):
				return "RECOGNIZER_HANDLED";
			case ( 2 ):
				return "RECOGNIZER_TAKEN_OVER_INPUTS";
		} /// -> switch


		switch ( constToResolve ) {
			case ( -1 ):
				return "NETWORK_CONNECTION_NO_NETWORK";
			case ( 0 ):
				return "NETWORK_CONNECTION_NO_INTERNET";
			case ( 1 ):
				return "NETWORK_CONNECTION_MOBILE_DATA";
			case ( 2 ):
				return "NETWORK_CONNECTION_WIFI";
			case ( 3 ):
				return "NETWORK_CONNECTION_LAN";
		} /// -> switch
		if ( constToResolve == ( -1 ) )
			return "ERR_LEVEL";
		if ( constToResolve == ( -2 ) )
			return "WARN_LEVEL";


		switch ( constToResolve ) {
			case ( 0x1 ):
				return "DEVICE_INFO_ATTR_DISPOSED";
			case ( 0x2 ):
				return "DEVICE_INFO_ATTR_ISCONNECTED";
			case ( 0x4 ):
				return "DEVICE_INFO_ATTR_CONNECT_PROGRESS";
			case ( 0x10 ):
				return "DEVICE_INFO_ATTR_USER_NAME";
			case ( 0x20 ):
				return "DEVICE_INFO_ATTR_IDENTITY";
			case ( 0x40 ):
				return "DEVICE_INFO_ATTR_DEVICE_PLATFORM";
			case ( 0x41 ):
				return "DEVICE_INFO_ATTR_DEVICE_TYPE";
			case ( 0x80 ):
				return "DEVICE_INFO_ATTR_NATIVEID";
			case ( 0x100 ):
				return "DEVICE_INFO_ATTR_IP";
			case ( 0x200 ):
				return "DEVICE_INFO_ATTR_IPE";
			case ( 0x400 ):
				return "DEVICE_INFO_ATTR_TCP_PORT";
			case ( 0x800 ):
				return "DEVICE_INFO_ATTR_UDP_PORT";
			case ( 0x1000 ):
				return "DEVICE_INFO_ATTR_UNAVAILABLE";
			case ( 0x2000 ):
				return "DEVICE_INFO_ATTR_BROADCAST_FOUND";
			case ( 0x4000 ):
				return "DEVICE_INFO_ATTR_DIRECT_CONTACT";
			case ( 0x8000 ):
				return "DEVICE_INFO_ATTR_APP_CONTEXT";
			case ( 0x10000 ):
				return "DEVICE_INFO_ATTR_PORTAL_CREATED";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0x10000 ):
				return "FILE_INFO_ATTR_CREATED";
			case ( 0x20000 ):
				return "FILE_INFO_ATTR_AVAILABLE";
			case ( 0x40000 ):
				return "FILE_INFO_ATTR_SEND_PROGRESS";
			case ( 0x80000 ):
				return "FILE_INFO_ATTR_RECEIVE_PROGRESS";
		} /// -> switch
		if ( constToResolve == ( 0x200000 ) )
			return "MESSAGE_INFO_ATTR_CREATED";


		switch ( constToResolve ) {
			case ( 0 ):
				return "APP_STATUS_ACTIVE";
			case ( 1 ):
				return "APP_STATUS_SLEEPING";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 60 ):
				return "ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT";
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
		if ( constToResolve == ( 0xFFFF ) )
			return "MEDIATOR_CLIENT_MAX_BUFFER_SIZE";
		if ( constToResolve == ( MEDIATOR_CLIENT_MAX_BUFFER_SIZE ) )
			return "DEVICE_HANDSHAKE_BUFFER_MAX_SIZE";


		switch ( constToResolve ) {
			case ( 0 ):
				return "MEDIATOR_DEVICE_CLASS_ALL";
			case ( 1 ):
				return "MEDIATOR_DEVICE_CLASS_NEARBY";
			case ( 2 ):
				return "MEDIATOR_DEVICE_CLASS_MEDIATOR";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "CALL_WAIT";
			case ( 1 ):
				return "CALL_NOWAIT";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "RENDER_CALLBACK_TYPE_ALL";
			case ( 0x10 ):
				return "RENDER_CALLBACK_TYPE_INIT";
			case ( 0x20 ):
				return "RENDER_CALLBACK_TYPE_AVCONTEXT";
			case ( 0x40 ):
				return "RENDER_CALLBACK_TYPE_DECODER";
			case ( 0x80 ):
				return "RENDER_CALLBACK_TYPE_IMAGE";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "DECODER_AVCONTEXT_TYPE_PIXELS";
			case ( RENDER_CALLBACK_TYPE_AVCONTEXT ):
				return "DECODER_AVCONTEXT_TYPE_AVCONTEXT";
			case ( 2 ):
				return "DECODER_AVCONTEXT_TYPE_JPG";
			case ( 3 ):
				return "DECODER_AVCONTEXT_TYPE_PNG";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "ENVIRONS_AVCONTEXT_SUBTYPE_RGB";
			case ( 1 ):
				return "ENVIRONS_AVCONTEXT_SUBTYPE_RGBA";
			case ( 2 ):
				return "ENVIRONS_AVCONTEXT_SUBTYPE_ARGB";
			case ( 6 ):
				return "ENVIRONS_AVCONTEXT_SUBTYPE_BGR";
			case ( 7 ):
				return "ENVIRONS_AVCONTEXT_SUBTYPE_BGRA";
			case ( 8 ):
				return "ENVIRONS_AVCONTEXT_SUBTYPE_ABGR";
		} /// -> switch
		if ( constToResolve == ( 0 ) )
			return "TYPES_SEPERATOR_2_ENVIRONS";


		switch ( constToResolve ) {
			case ( 0 ):
				return "ENVIRONS_SENSOR_TYPE_ACCELEROMETER";
			case ( 1 ):
				return "ENVIRONS_SENSOR_TYPE_MAGNETICFIELD";
			case ( 2 ):
				return "ENVIRONS_SENSOR_TYPE_GYROSCOPE";
			case ( 3 ):
				return "ENVIRONS_SENSOR_TYPE_ORIENTATION";
		} /// -> switch



		return "Unkown";
	} /// -> get()



#ifdef __cplusplus
} /// -> namespace environs
#endif



