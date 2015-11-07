/**
 * MessageInstance CPP Object
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
#ifndef INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_H
#define INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_H

#include "Interop/Smart.Pointer.h"
#include "Interfaces/IMessage.Instance.h"
#include <string>

/**
 *	MessageInstance
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 *	@remarks
 * ****************************************************************************************
 */

/* Namespace: environs -> */
namespace environs
{
    namespace lib
    {
        
        class DeviceInstance;
        
		class MessageInstance : public IMessageInstance
        {
            friend class DeviceList;
            friend class DeviceInstance;
            friend class MessageInstanceProxy;
            
        public:
			ENVIRONS_LIB_API MessageInstance ();
			ENVIRONS_LIB_API ~MessageInstance ();
            
            /**
             * disposed is true if the object is no longer valid. Nothing will be updated anymore.
             * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
             * */
            ENVIRONS_LIB_API bool disposed ();
            
            /**
             * sent is true if this MessageInstance is data that was sent or received (false).
             * */
			ENVIRONS_LIB_API bool sent ();
            
            /**
             * created is a posix timestamp that determines the time and date that this MessageInstance
             * has been received or sent.
             * */
			ENVIRONS_LIB_API unsigned long long created ();
            
            /**
             * The length of the text message in bytes (characters).
             * */
			ENVIRONS_LIB_API int length ();
            
            /**
             * The text message.
             * */
			ENVIRONS_LIB_API const char * text ();
            
            /**
             * Determins the type of connection (channel type) used to exchange this message.
             * c = in connected state
             * d = in not connected state through a direct device to device channel.
             * m = in not connected state by means of a Mediator service.
             * */
			ENVIRONS_LIB_API char connection ();
            
            /**
             * A reference to the DeviceInstance that is responsible for this FileInstance.
             * */
            sp ( DeviceInstance ) device_;
            
			ENVIRONS_LIB_API IDeviceInstance * deviceRetainedI ();
            
			ENVIRONS_LIB_API const char * ToString ();
			ENVIRONS_LIB_API const char * ShortText ();

			/**
			* Release ownership on this interface and mark it disposeable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
             */
            ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

		private:
			ENVIRONS_OUTPUT_ALLOC_RESOURCE ( MessageInstance );
            
            void            Dispose ();
            
            void            PlatformDispose ();
            
            /**
             * disposed is true if the object is no longer valid. Nothing will be updated anymore.
             * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
             * */
            bool			disposed_;

            std::string     toString;
            char            textShortEnd;
            char            connection_;
            char *          text_;
            int             length_;
            bool            sent_;
            
            unsigned long long created_;
            
            static sp ( MessageInstance ) Create ( char * line, int length, const sp ( DeviceInstance ) &device );
            
            static bool HasPrefix ( const char * line, int length );
            
        };
    }
}

#endif	/// INCLUDE_HCM_ENVIRONS_MESSAGEINSTANCE_H




