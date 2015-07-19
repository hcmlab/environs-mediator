/**
 * FileInstance for iOSX
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
#ifndef INCLUDE_HCM_ENVIRONS_FILEINSTANCE_IOSX_H
#define INCLUDE_HCM_ENVIRONS_FILEINSTANCE_IOSX_H

#include "Environs.iOSX.Imp.h"

/**
 *	FileInstance iOSX
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */
@interface FileInstance : NSObject
{
@public
    int fileID;
    int type;
    NSString * descriptor;
    bool sent;
    long created;
    long size;
    NSString * path;
    
    int sendProgress;
    int receiveProgress;
    
    id device;
}

- (bool) Init:(id) device fid:(int)fileID;

- (NSString *) ToString;

- (NSString *) GetPath;

@end

#endif	/// INCLUDE_HCM_ENVIRONS_FILEINSTANCE_IOSX_H

