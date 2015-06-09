/**
 *	Interface for a gesture recognizer working on touch contacts.
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


#ifndef INCLUDE_HCM_ENVIRONS_ITOUCHRECOGNIZER_H
#define INCLUDE_HCM_ENVIRONS_ITOUCHRECOGNIZER_H


#define	MAX_TOUCH_VISUALS	10


namespace environs 
{
	typedef void ( *PerformEnvironsTouch )(int id, char ev, int x, int y, float angle);


	/**
	* ITouchRecognizer
	*
	*/
	class ITouchRecognizer : public IEnvironsIdent
	{
	public:
		/** Constructor */
		ITouchRecognizer ( ) :
			/** Base class initialization */
			IEnvironsIdent ( InterfaceType::InputRecognizer ),
			/** Default initialization */
			width_capture ( 0 ), height_capture ( 0 ), parent ( 0 ) 
		{};

		virtual ~ITouchRecognizer ( ) {};


		/** Init
		*	@param	void
		*	@return	bool
		*/
		bool Init ( void * _parent, unsigned int _width_capture, unsigned int _height_capture );

		virtual bool Init ( ) = 0;


		/** RecognizeGestures
		*	@param	void
		*	@return	bool
		*/
		virtual bool		Perform ( Input * container, unsigned int count ) = 0;

		unsigned int		width_capture;
		unsigned int		height_capture;
			
		void				PerformTouch ( Input * pack );
		void				UpdatePosition ( int x, int y );
		void				UpdatePortalsize ( unsigned int width, unsigned int height );

	protected:
		void			*	parent;
	};

} /* namespace environs */


#endif /// -> INCLUDE_HCM_ENVIRONS_ITOUCHRECOGNIZER_H