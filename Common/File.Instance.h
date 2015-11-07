/**
 * FileInstance CPP Object
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
#ifndef INCLUDE_HCM_ENVIRONS_FILEINSTANCE_H
#define INCLUDE_HCM_ENVIRONS_FILEINSTANCE_H

#include "Interop/Smart.Pointer.h"
#include "Interfaces/IFile.Instance.h"
#include <string>

/**
 *	FileInstance CPP Object
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
        
		class FileInstance : public IFileInstance
		{
            friend class DeviceList;
            friend class DeviceInstance;
            friend class FileInstanceProxy;

		public:
			ENVIRONS_LIB_API FileInstance ();
            ENVIRONS_LIB_API ~FileInstance ();
            
            /**
             * disposed is true if the object is no longer valid. Nothing will be updated anymore.
             * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
             * */
            ENVIRONS_LIB_API bool disposed ();
            
            /**
             * An integer type identifier to uniquely identify this FileInstance between two DeviceInstances.
             * A value of 0 indicates an invalid fileID.
             * */
			ENVIRONS_LIB_API int fileID ();
            
            /**
             * Used internally.
             * */
			ENVIRONS_LIB_API int type ();
            
            /**
             * A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
             * */
			ENVIRONS_LIB_API const char * descriptor ();
            
            /**
             * sent is true if this FileInstance is data that was sent or received (false).
             * */
			ENVIRONS_LIB_API bool sent ();
            
            /**
             * created is a posix timestamp that determines the time and date that this FileInstance
             * has been received or sent.
             * */
			ENVIRONS_LIB_API unsigned long long created ();
            
            /**
             * The size in bytes of a buffer to send or data received.
             * */
			ENVIRONS_LIB_API long size ();
            
            /**
             * The absolute path to the file if this FileInstance originates from a call to SendFile or received data.
             * */
			ENVIRONS_LIB_API const char * path ();
            
            /**
             * sendProgress is a value between 0-100 (percentage) that reflects the percentage of the
             * file or buffer that has already been sent.
             * If this value changes, then the corresponding device's DeviceObserver is notified
             * with this FileInstance object as the sender
             * and the change-flag FILE_INFO_ATTR_SEND_PROGRESS
             * */
			ENVIRONS_LIB_API int sendProgress ();
            
            /**
             * receiveProgress is a value between 0-100 (percentage) that reflects the percentage of the
             * file or buffer that has already been received.
             * If this value changes, then the corresponding device's DeviceObserver is notified
             * with this FileInstance object as the sender
             * and the change-flag FILE_INFO_ATTR_RECEIVE_PROGRESS
             * */
			ENVIRONS_LIB_API int receiveProgress ();
            
            /**
             * Get the DeviceInstance that this FileInstance is attached to.
             *
             * */
			ENVIRONS_LIB_API IDeviceInstance * deviceRetainedI ();
            
            
			ENVIRONS_LIB_API const char * ToString ();
            
			ENVIRONS_LIB_API const char * GetPath ();


			/**
			* Release ownership on this interface and mark it disposeable.
			* Release must be called once for each Interface that the Environs framework returns to client code.
			* Environs will dispose the underlying object if no more ownership is hold by anyone.
			*
             */
            ENVIRONS_OUTPUT_DE_ALLOC_DECL ();

        private:
            ENVIRONS_OUTPUT_ALLOC_RESOURCE ( FileInstance );

            void Dispose ();
            
            /**
             * disposed is true if the object is no longer valid. Nothing will be updated anymore.
             * disposed will be notified through Environs.ENVIRONS_OBJECT_DISPOSED to DeviceObservers.
             * */
            bool			disposed_;
            
            void            PlatformDispose ();

			/**
			* An integer type identifier to uniquely identify this FileInstance between two DeviceInstances.
			* A value of 0 indicates an invalid fileID.
			* */
			int             fileID_;

			/**
			* Used internally.
			* */
			int             type_;

			/**
			* A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
			* */
			std::string     descriptor_;

			/**
			* sent is true if this FileInstance is data that was sent or received (false).
			* */
			bool            sent_;

			/**
			* created is a posix timestamp that determines the time and date that this FileInstance
			* has been received or sent.
			* */
			unsigned long long created_;

			/**
			* The size in bytes of a buffer to send or data received.
			* */
			long            size_;

			/**
			* The absolute path to the file if this FileInstance originates from a call to SendFile or received data.
			* */
			std::string     path_;

			/**
			* sendProgress is a value between 0-100 (percentage) that reflects the percentage of the
			* file or buffer that has already been sent.
			* If this value changes, then the corresponding device's DeviceObserver is notified
			* with this FileInstance object as the sender
			* and the change-flag FILE_INFO_ATTR_SEND_PROGRESS
			* */
			int             sendProgress_;

			/**
			* receiveProgress is a value between 0-100 (percentage) that reflects the percentage of the
			* file or buffer that has already been received.
			* If this value changes, then the corresponding device's DeviceObserver is notified
			* with this FileInstance object as the sender
			* and the change-flag FILE_INFO_ATTR_RECEIVE_PROGRESS
			* */
			int             receiveProgress_;

			/**
			* A reference to the DeviceInstance that is responsible for this FileInstance.
			* */
            sp ( DeviceInstance ) device_;
            
            /**
             * A utf-8 descriptor that was attached to this FileInstance in SendFile/SendBuffer
             * */
            std::string toString_;

			static sp ( FileInstance ) Create ( const sp ( DeviceInstance ) &device, int fileID, char * fullPath, size_t length );
		};
	}
}

#endif	/// INCLUDE_HCM_ENVIRONS_FILEINSTANCE_IOSX_H
