/**
 *	Interface for a gesture recognizer working on touch contacts.
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
#include "stdafx.h"
#include "Interfaces/ITouch.Recognizer.h"
#include "Device/Device.Base.h"


namespace environs
{
	bool ITouchRecognizer::Init ( void * _parent, unsigned int _width_capture, unsigned int _height_capture ) {
		parent	= _parent;

		if ( !parent )
			return false;

		width_capture	= _width_capture;
		height_capture	= _height_capture;

		return Init ( );
	}


	void ITouchRecognizer::PerformTouch ( Input * pack )
	{
		((DeviceBase *) parent)->PerformEnvironsTouch ( pack );
	}
	

	void ITouchRecognizer::UpdatePosition ( int x, int y )
	{
		((DeviceBase *)parent)->UpdatePosition ( 0, x, y, -1 );
	}


	void ITouchRecognizer::UpdatePortalsize ( unsigned int width, unsigned int height )
	{
		((DeviceBase *)parent)->UpdatePortalSize ( 0, width, height );
	}
}
