/**
 * PortalsInstance iOSX
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
#ifndef INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_IOSX_H
#define INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_IOSX_H

#import "Environs.observer.h"
#include "Environs.iosx.imp.h"
#include "Environs.types.h"
#include "Portal.Info.h"
using namespace environs;


/**
 *	PortalsInstance iOSX
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
@interface PortalInstance :

#ifdef ENVIRONS_IOS
    NSObject<UIAlertViewDelegate>
#else
    NSObject
#endif
{
@public
    int                     portalID;
    bool                    disposed;
    id                      device;
    
    int                     status;
    bool                    established;
    bool                    initiator;
    
    bool                    outgoing;
    PortalType::PortalType  portalType;    
}


- (bool) Init:(id) deviceInstance Direction:(int)Environs_PORTAL_DIR_ Type:(PortalType::PortalType)type;
- (bool) Init:(id) deviceInstance PortalID:(int)destID;

- (void) Dispose;
+ (void) Dispose:(int) portalID;

- (void) AddObserver:(id<PortalObserver>) observer;
- (void) RemoveObserver:(id<PortalObserver>) observer;
- (int) GetObserverCount;

- (void) NotifyObservers:(int) notification;

- (bool) Establish:(bool) askForType;

- (bool) Start;
- (bool) Stop;

- (bool) SetRenderSurface:(id)surface;
- (bool) ReleaseRenderSurface;

- (PortalInfo *) GetPortalInfo;
- (bool) SetPortalInfo:(PortalInfo *) info;

+ (void) Update:(int) deviceID Project:(const char *) projectName App:(const char *) appName Notfy:(int) notification PortalID:(int) portalID;
+ (bool) PresentPortalToObservers:(PortalInstance *) portal;

@end


#endif	/// INCLUDE_HCM_ENVIRONS_PORTALINSTANCE_IOSX_H

