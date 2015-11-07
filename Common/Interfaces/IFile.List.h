/**
 * IFileInstance ArrayList Interface Declaration
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
#ifndef INCLUDE_HCM_ENVIRONS_INTERFACES_FILE_ARRAYLIST_H
#define	INCLUDE_HCM_ENVIRONS_INTERFACES_FILE_ARRAYLIST_H

#include "Interfaces/IArray.List.h"
#include "Interfaces/IFile.Instance.h"

namespace environs
{
	namespace lib
    {
		/**
		* IFileList
		*
		*/
		class IFileList : public IArrayList
		{
		public:
			/** Constructor */
			IFileList () {};

			virtual ~IFileList () {};
            
            /**
             * Get the item at the given position.
             *
             * @param position
             * 
             * @ return The object at given position
             */
            sp ( IFileInstance ) at ( size_t pos ) {
                ENVIRONS_I_BUILDSP_LIST ( IFileInstance, pos );
            };
            
            
            /**
             * Get an Interface of the item at the given position.
             *
             * @param position
             *
             * @ return The object at given position
             */
            IFileInstance * atI ( size_t pos ) {
                return (IFileInstance *) item ( pos );
            };
		};
	}
}
            

#endif	/// -> INCLUDE_HCM_ENVIRONS_INTERFACES_FILE_ARRAYLIST_H








