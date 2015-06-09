/**
 *	Interface for a gesture recognizer working on touch contacts 
    on mobile devices.
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
#include "Interface.Exports.h"
#include "IEnvirons.Ident.h"
#include "Human.Input.Decl.h"

#ifndef INCLUDE_HCM_ENVIRONS_IHUMAN_INPUT_RECOGNIZER_H
#define INCLUDE_HCM_ENVIRONS_IHUMAN_INPUT_RECOGNIZER_H

#define	MAX_TOUCH_VISUALS	10


namespace environs 
{
	/**
	* IInputRecognizer
	*
	*/
	class IInputRecognizer : public IEnvironsIdent
	{
	public:
		/** Constructor */
		IInputRecognizer () :
			/** Base class initialization */
			IEnvironsIdent ( InterfaceType::InputRecognizer ),
			/** Default initialization */
			triggerTouchCount ( 0 ), deviceBase ( 0 ), device_width ( 0 ), device_height ( 0 )
		{};

		virtual ~IInputRecognizer ( ) {};
        
        
        void            *   deviceBase;

		/** Init
		*	@param	device_width
		*	@param	device_height
		*	@return	bool
		*/
		bool Init ( void * deviceBase, unsigned int device_width, unsigned int device_height )
		{
            this->deviceBase = deviceBase;
            this->device_width = device_width; this->device_height = device_height;
			return Init ();
		};

		virtual bool Init () = 0;


		/** RecognizeGestures
		*	@param	void
		*	@return	bool
		*/
		unsigned int		triggerTouchCount;
		virtual int			Trigger ( InputPackRec **	inputs, unsigned int inputCount ) = 0;

		virtual int			Perform ( InputPackRec **	inputs, unsigned int inputCount ) = 0;
        
	protected:
		unsigned int		device_width;
		unsigned int		device_height;
	};

} /* namespace environs */


#endif /// -> INCLUDE_HCM_ENVIRONS_IHUMAN_INPUT_RECOGNIZER_H


