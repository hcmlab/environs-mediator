/**
* Device screen declaration
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
#ifndef INCLUDE_HCM_ENVIRONS_DEVICESCREEN_DECLARATIONS_H_
#define INCLUDE_HCM_ENVIRONS_DEVICESCREEN_DECLARATIONS_H_


namespace environs
{
	typedef struct _DeviceScreen
	{
		int				width;
		int				height;
		double			width_mm;
		double			height_mm;
	}
	DeviceScreen;
}


#endif /* INCLUDE_HCM_ENVIRONS_DEVICESCREEN_DECLARATIONS_H_ */
