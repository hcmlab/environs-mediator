/**
 * EnvironsObserver
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
#import <Foundation/Foundation.h>

@protocol EnvironsObserver <NSObject>

- (void) OnData:(int) deviceID Project:(const char *) projectName App:(const char *) appName withType:(int) type withFileID:(int) fileID withDescriptor:(const char *) descriptor withSize:(int) size;

- (void) OnNotify:(int) deviceID Project:(const char *) projectName App:(const char *) appName withNotify:(int) notification withSource:(int)source withContext:(int)context;

- (void) OnMessage:(int) deviceID Project:(const char *) projectName App:(const char *) appName withType:(int)type withMsg:(const char *) message withLength:(int)msgLength;

- (void) OnStatusMessage:(const char *) message;


- (bool) OnPortalRequestOrProvided:(id) portalInstance;

@end


@protocol PortalObserver <NSObject>

- (void) OnPortalChanged:(id) sender Notify:(int)notification;

@end