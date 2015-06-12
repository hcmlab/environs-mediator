/**
 * Environs Native Layer API exposed by the libraries for mobile devices
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
#ifndef INCLUDE_HCM_ENVIRONS_NATIVE_MOBILE_API_H
#define INCLUDE_HCM_ENVIRONS_NATIVE_MOBILE_API_H


namespace environs 
{
#ifndef DISPLAYDEVICE

#include "Environs.lib.inc.h"
	
	namespace API 
	{

#ifdef __cplusplus
		extern "C"
		{
#endif
			//LIBEXPORT void CallConv			allocNative ();
			LIBEXPORT void CallConv			setOrientations ( float x, float y, float z );
			LIBEXPORT void CallConv			setAccelerometers ( float x, float y, float z );

			LIBEXPORT void CallConv			touch ( int deviceID, void * buffer, int count, EBOOL init );
			LIBEXPORT void CallConv			touch1 ( int deviceID, int id, int x, int y, unsigned char event );
			LIBEXPORT void CallConv			touch2 ( int deviceID, int id, int x, int y, float angle, float size, float axisx, float axisy, unsigned char event );

			LIBEXPORT int CallConv			getSizeOfBufferHeader ();
			LIBEXPORT void * CallConv		receiveStreamPack ( int deviceID, const char * projName, const char * appName, int portalID );

			LIBEXPORT int CallConv			getSizeOfTouchPack ();

			LIBEXPORT int CallConv          getActivePortalID ();
			LIBEXPORT int CallConv          getRequestedPortalID ();

			LIBEXPORT void CallConv			setPortalViewDimsAuto ( EBOOL enable );
			LIBEXPORT EBOOL CallConv		getPortalViewDimsAuto ();
			LIBEXPORT void CallConv			setPortalViewDims ( int deviceID, const char * projName, const char * appName, int left, int top, int right, int bottom );

			LIBEXPORT EBOOL CallConv		setPortalWidthHeight ( int deviceID, int width, int height );

			LIBEXPORT EBOOL CallConv		getUseSensors ();
			LIBEXPORT void CallConv			setUseSensors ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		getUsePushNotifications ();
			LIBEXPORT void CallConv			setUsePushNotifications ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		getPortalTCP ();
			LIBEXPORT void CallConv			setPortalTCP ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		getUseNativeDecoder1 ();
			LIBEXPORT void CallConv			setUseNativeDecoder1 ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		getUseHardwareEncoder ();
			LIBEXPORT void CallConv			setUseHardwareEncoder1 ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		getPortalNativeResolution ();
			LIBEXPORT void CallConv			setPortalNativeResolution ( EBOOL enable );

			LIBEXPORT EBOOL CallConv		getPortalAutoStart ();
			LIBEXPORT void CallConv			setPortalAutoStart ( EBOOL enable );

#ifdef __cplusplus
		}
#endif

	}
    
#endif
}

#endif	// INCLUDE_HCM_ENVIRONS_NATIVE_MOBILE_API_H



