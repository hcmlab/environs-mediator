/**
 * Environs STL Exports
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
#ifndef INCLUDE_HCM_ENVIRONS_STL_EXPORTS_H
#define	INCLUDE_HCM_ENVIRONS_STL_EXPORTS_H

#include "Export.h"
#include <string>

ENVIRONSAPI template class ENVIRONS_LIB_API sp ( environs::lib::DeviceInstance );
ENVIRONSAPI template class ENVIRONS_LIB_API svsp ( environs::lib::DeviceInstance );
ENVIRONSAPI template class ENVIRONS_LIB_API sp ( environs::lib::EnvironsObserver );
ENVIRONSAPI template class ENVIRONS_LIB_API sp ( environs::lib::ChangeObserver );
ENVIRONSAPI template class ENVIRONS_LIB_API vsp ( environs::lib::ChangeObserver );
ENVIRONSAPI template class ENVIRONS_LIB_API vsp ( environs::lib::MessageObserver );
ENVIRONSAPI template class ENVIRONS_LIB_API vsp ( environs::lib::DataObserver );
ENVIRONSAPI template class ENVIRONS_LIB_API vsp ( environs::lib::SensorObserver );
ENVIRONSAPI template class ENVIRONS_LIB_API svsp ( environs::lib::ListObserver );
ENVIRONSAPI template class ENVIRONS_LIB_API svsp ( environs::lib::MessageInstance );
ENVIRONSAPI template class ENVIRONS_LIB_API vsp ( environs::lib::PortalInstance );
ENVIRONSAPI template class ENVIRONS_LIB_API smsp ( int, environs::lib::FileInstance );
ENVIRONSAPI template class ENVIRONS_LIB_API std::basic_string <char>;

#endif	/// -> INCLUDE_HCM_ENVIRONS_STL_EXPORTS_H








