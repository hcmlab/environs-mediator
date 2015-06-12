/* DO NOT EDIT THIS FILE - it is machine generated by j2c.jar (see Environs.framework) */
/* Events, Types, Messages for hcm_environs Types */

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
using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;


namespace hcm.environs
{
	/// <summary>
	/// Events, Types, Messages for hcm_environs Types 
	/// This class defines integer values which identifies status values, events, message types and so on delivered by Environs.
	/// </summary>
	public partial class Environs 
	{

	public static String resolveName ( int constToResolve ) {
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


		switch ( constToResolve ) {
			case ( 0 ):
				return "MEDIATOR_DEVICE_CLASS_AVAILABLE";
			case ( 1 ):
				return "MEDIATOR_DEVICE_CLASS_NEARBY";
			case ( 2 ):
				return "MEDIATOR_DEVICE_CLASS_MEDIATOR";
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
		} /// -> switch

		if ( constToResolve == ( 1 ) )
			return "MSG_TYPE_IMAGE";


		switch ( constToResolve ) {
			case ( 0x40 ):
				return "DATA_STREAM_IFRAME";
			case ( 0x10 ):
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


		switch ( constToResolve ) {
			case ( 0x20 ):
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
		} /// -> switch
		if ( constToResolve == ( 0x800 ) )
			return "NOTIFY_PORTAL";


		switch ( constToResolve ) {
			case ( 0 ):
				return "MSG_PORTAL_REQUEST_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_REQUEST_ID ):
				return "MSG_PORTAL_REQUEST";
			case ( 1 ):
				return "MSG_PORTAL_PROVIDE_STREAM_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_PROVIDE_STREAM_ID ):
				return "MSG_PORTAL_PROVIDE_STREAM";
			case ( 2 ):
				return "MSG_PORTAL_PROVIDE_IMAGES_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_PROVIDE_IMAGES_ID ):
				return "MSG_PORTAL_PROVIDE_IMAGES";
			case ( 3 ):
				return "MSG_PORTAL_REQUEST_FAIL_ID";
			case ( MSG_PORTAL_ERROR 	| MSG_PORTAL_REQUEST_FAIL_ID ):
				return "MSG_PORTAL_REQUEST_FAIL";
			case ( 4 ):
				return "MSG_PORTAL_STOP_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_STOP_ID ):
				return "MSG_PORTAL_STOP";
			case ( 5 ):
				return "MSG_PORTAL_STOP_ACK_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_STOP_ACK_ID ):
				return "MSG_PORTAL_STOP_ACK";
			case ( 6 ):
				return "MSG_PORTAL_STOP_FAIL_ID";
			case ( MSG_PORTAL_ERROR 	| MSG_PORTAL_STOP_FAIL_ID ):
				return "MSG_PORTAL_STOP_FAIL";
			case ( 7 ):
				return "MSG_PORTAL_START_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_START_ID ):
				return "MSG_PORTAL_START";
			case ( 8 ):
				return "MSG_PORTAL_START_ACK_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_START_ACK_ID ):
				return "MSG_PORTAL_START_ACK";
			case ( 9 ):
				return "MSG_PORTAL_START_FAIL_ID";
			case ( MSG_PORTAL_ERROR 	| MSG_PORTAL_START_FAIL_ID ):
				return "MSG_PORTAL_START_FAIL";
			case ( 10 ):
				return "MSG_PORTAL_PAUSE_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_PAUSE_ID ):
				return "MSG_PORTAL_PAUSE";
			case ( 11 ):
				return "MSG_PORTAL_PAUSE_ACK_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_PAUSE_ACK_ID ):
				return "MSG_PORTAL_PAUSE_ACK";
			case ( 12 ):
				return "MSG_PORTAL_PAUSE_FAIL_ID";
			case ( MSG_PORTAL_ERROR 	| MSG_PORTAL_PAUSE_FAIL_ID ):
				return "MSG_PORTAL_PAUSE_FAIL";
			case ( 13 ):
				return "MSG_PORTAL_BUFFER_FULL_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_BUFFER_FULL_ID ):
				return "MSG_PORTAL_BUFFER_FULL";
			case ( 14 ):
				return "MSG_PORTAL_BUFFER_AVAIL_AGAIN_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_BUFFER_AVAIL_AGAIN_ID ):
				return "MSG_PORTAL_BUFFER_AVAIL_AGAIN";
			case ( 15 ):
				return "MSG_PORTAL_IFRAME_REQUEST_ID";
			case ( NOTIFY_PORTAL 	| MSG_PORTAL_IFRAME_REQUEST_ID ):
				return "MSG_PORTAL_IFRAME_REQUEST";
			case ( 15 + 1 ):
				return "MSG_PORTAL_MAX_COUNT";
		} /// -> switch


		switch ( constToResolve ) {
			case ( (MSG_TYPE_PORTAL << 16) ):
				return "NOTIFY_TYPE_PORTAL";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_REQUEST ):
				return "NOTIFY_PORTAL_REQUEST";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_STREAM | PORTAL_DIR_INCOMING ):
				return "NOTIFY_PORTAL_STREAM_INCOMING";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_IMAGES | PORTAL_DIR_INCOMING ):
				return "NOTIFY_PORTAL_IMAGES_INCOMING";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_STREAM | PORTAL_DIR_OUTGOING ):
				return "NOTIFY_PORTAL_PROVIDE_STREAM_ACK";
			case ( NOTIFY_TYPE_PORTAL | MSG_PORTAL_PROVIDE_IMAGES | PORTAL_DIR_OUTGOING ):
				return "NOTIFY_PORTAL_PROVIDE_IMAGES_ACK";
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
				return "MEDIATOR_FILTER_PROJECT";
			case ( 2 ):
				return "MEDIATOR_FILTER_PROJECT_AND_APP";
		/// Disable all devicelist notifications
			case ( 8 ):
				return "MEDIATOR_FILTER_ALL";
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
				return "NOTIFY_MEDIATOR_DEVICELISTS_CHANGED";
			case ( NOTIFY_MEDIATOR | 11 ):
				return "NOTIFY_MEDIATOR_MED_CHANGED";
			case ( NOTIFY_MEDIATOR_DEVICE_CHANGED | NOTIFY_MEDIATOR_SERVER ):
				return "NOTIFY_MEDIATOR_MED_DEVICE_CHANGED";
			case ( NOTIFY_MEDIATOR_DEVICE_ADDED | NOTIFY_MEDIATOR_SERVER ):
				return "NOTIFY_MEDIATOR_MED_DEVICE_ADDED";
			case ( NOTIFY_MEDIATOR_DEVICE_REMOVED | NOTIFY_MEDIATOR_SERVER ):
				return "NOTIFY_MEDIATOR_MED_DEVICE_REMOVED";
			case ( NOTIFY_MEDIATOR | 22 | NOTIFY_MEDIATOR_SERVER ):
				return "NOTIFY_MEDIATOR_MED_STUNT_REG_REQ";
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
				return "DEVICE_INFO_ATTR_USER_NAME";
			case ( 0x2 ):
				return "DEVICE_INFO_ATTR_DEVICE_NAME";
			case ( 0x4 ):
				return "DEVICE_INFO_ATTR_PROJECT_NAME";
			case ( 0x8 ):
				return "DEVICE_INFO_ATTR_APP_NAME";
			case ( 0x40 ):
				return "DEVICE_INFO_ATTR_DEVICE_TYPE";
			case ( 0x100 ):
				return "DEVICE_INFO_ATTR_IP";
			case ( 0x200 ):
				return "DEVICE_INFO_ATTR_IPE";
			case ( 0x400 ):
				return "DEVICE_INFO_ATTR_PORT_TCP";
			case ( 0x800 ):
				return "DEVICE_INFO_ATTR_PORT_UDP";
			case ( 0x1000 ):
				return "DEVICE_INFO_ATTR_BROADCAST_FOUND";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "APP_STATUS_ACTIVE";
			case ( 1 ):
				return "APP_STATUS_SLEEPING";
		} /// -> switch
		if ( constToResolve == ( 60 ) )
			return "ENVIRONS_DIALOG_NO_ACTIVITY_TIMEOUT";


		switch ( constToResolve ) {
			case ( 0 ):
				return "CALL_SYNC";
			case ( 1 ):
				return "CALL_ASYNC";
		} /// -> switch


		switch ( constToResolve ) {
			case ( 0 ):
				return "DECODER_AVCONTEXT_TYPE_PIXELS";
			case ( 1 ):
				return "DECODER_AVCONTEXT_TYPE_AVPACK";
			case ( 2 ):
				return "DECODER_AVCONTEXT_TYPE_JPG";
			case ( 3 ):
				return "DECODER_AVCONTEXT_TYPE_PNG";
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
				return "DECODER_AVCONTEXT_SUBTYPE_RGB";
			case ( 1 ):
				return "DECODER_AVCONTEXT_SUBTYPE_BGRA";
			case ( 2 ):
				return "DECODER_AVCONTEXT_SUBTYPE_ABGR";
			case ( 3 ):
				return "DECODER_AVCONTEXT_SUBTYPE_RGBA";
		} /// -> switch



		return "Unkown";
	} /// -> get()

} /// -> class Types

} /// -> namespace environs 
