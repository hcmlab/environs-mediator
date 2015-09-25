/**
* Environs build options
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
#ifndef INCLUDE_HCM_ENVIRONS_BUILD_OPTIONS_COMMON_H
#define INCLUDE_HCM_ENVIRONS_BUILD_OPTIONS_COMMON_H

#ifdef _WIN32
#ifdef MEDIATORDAEMON
#define _USE_VLD
#endif
//#define _USE_VLD
//#define USE_CRT_MLC

#ifdef _USE_VLD
#define _C_X64
#endif

#ifdef _USE_VLD
#ifdef _C_X64
#include "C:/Program Files (x86)/Visual Leak Detector/include/vld.h"
#else
#include "C:/Program Files/Visual Leak Detector/include/vld.h"
#endif
#endif

#ifdef USE_CRT_MLC
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#endif


#endif


