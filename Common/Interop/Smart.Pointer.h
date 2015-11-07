/**
* Environs Smart Pointer Macros
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
#ifndef INCLUDE_HCM_ENVIRONS_SMART_POINTER_H
#define INCLUDE_HCM_ENVIRONS_SMART_POINTER_H


#ifdef __cplusplus

#include <memory>
#include <vector>
#include <map>


#define sp(type)        std::shared_ptr < type >

#define vct(type)       std::vector < type >
#define spv(type)       std::shared_ptr < std::vector < type > >
#define vsp(type)       std::vector < std::shared_ptr < type > >
#define svsp(type)      std::shared_ptr < std::vector < std::shared_ptr < type > > >

#define msp(key,type)	std::map < key, std::shared_ptr < type > >
#define smsp(key,type)	std::shared_ptr < std::map < key, std::shared_ptr < type > > >

#define up(type)        std::unique_ptr < type >

#define sp_autorelease(type,obj)    std::shared_ptr < type > ( obj, environs::lib::EnvironsDisposer );

// Not supported by vs 2010
            //template<typename T>
            //using spd = std::shared_ptr<T>;
#else


#define sp(type)

#define vct(type)
#define spv(type)
#define vsp(type)
#define svsp(type)

#define msp(key,type)
#define smsp(key,type)

#define up(type)

#define sp_autorelease(type,obj)

#endif

#endif // INCLUDE_HCM_ENVIRONS_SMART_POINTER_H