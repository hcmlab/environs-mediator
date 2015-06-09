/**
 * Environs Native Layer API exposed by the libraries for large display devices
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
#ifndef INCLUDE_HCM_ENVIRONS_NATIVE_DISPLAY_API_H
#define INCLUDE_HCM_ENVIRONS_NATIVE_DISPLAY_API_H


namespace environs 
{
#ifdef DISPLAYDEVICE

#include "Environs.lib.inc.h"
	
	namespace API 
	{

#ifdef __cplusplus
		extern "C"
		{
#endif

			LIBEXPORT void CallConv			setUseDeviceMarkerAutomatic ( EBOOL enable );
			LIBEXPORT void CallConv			setDeviceMarkerReducedPrecision1 ( EBOOL enable );
            
            LIBEXPORT int CallConv			setUseTracker ( const char * moduleName );
            LIBEXPORT int CallConv			getUseTracker ( const char * moduleName );
            LIBEXPORT EBOOL CallConv		disposeTracker ( const char * moduleName );
            
            LIBEXPORT EBOOL CallConv		getTrackerEnabled ( int index );
            LIBEXPORT EBOOL CallConv		setTrackerParams ( int index, int channels, int width, int height, int stride );
            LIBEXPORT EBOOL CallConv		setTrackerImage ( int index, void * rawImage, int size );
            LIBEXPORT EBOOL CallConv		pushTrackerCommand ( int index, int command );

#ifdef __cplusplus
		}
#endif

	}
    
#endif
}

#endif	// INCLUDE_HCM_ENVIRONS_NATIVE_DISPLAY_API_H



