/**
* Notify Context Declarations
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
#ifndef INCLUDE_HCM_ENVIRONS_NOTIFY_CONTEXT_DECLARATIONS_H_
#define INCLUDE_HCM_ENVIRONS_NOTIFY_CONTEXT_DECLARATIONS_H_


namespace environs
{
    typedef struct NotifyContext
    {
        int nativeID;
        int notification;
        int sourceIdent;
        void * contextPtr;
        int hEnvirons;
    }
    NotifyContext;
    
    
    typedef struct ObserverMessageContext
    {
        int             destID;
        const char *    areaName;
        const char *    appName;
        int             type;
        const char *    message;
        int             length;
        char            connection;
    }
    ObserverMessageContext;
    
    
    typedef struct ObserverDataContext
    {
        int             nativeID;
        int             type;
        int             fileID;
        const char *    fileDescriptor;
        int             size;
    }
    ObserverDataContext;
    
}


#endif /* INCLUDE_HCM_ENVIRONS_NOTIFY_CONTEXT_DECLARATIONS_H_ */
