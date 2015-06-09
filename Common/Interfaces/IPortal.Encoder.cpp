/**
 * Interface for a portal encoder/compressor
   (create transport packages)
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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#define DEBUGVERB
//#define ENCODERDEBUGVERB
#endif

#include "Environs.native.h"
#include "Interfaces/IPortal.Encoder.h"

#define CLASS_NAME	"IPortalEncoder"


namespace environs
{
	__forceinline bool ResizeBuffer ( RenderContext * context, unsigned int payloadSize )
	{
		ByteBuffer * buffer = context->outputBuffer;

		if ( buffer && buffer->capacity >= payloadSize ) {
			buffer->payloadSize = payloadSize;
			return true;
		}

		buffer = relocateBuffer ( context->outputBuffer, true, payloadSize + 8 );
		if ( buffer ) {
			context->outputBuffer = buffer;
			buffer->payloadSize = payloadSize;
			return true;
		}
		return false;
	}

	/*
	__forceinline bool ResizeBuffer ( RenderContext * context, unsigned int payloadSize )
	{
		unsigned int		bufferSize;
		char			*	buffer = context->outputBuffer;

		if ( buffer ) {
			// We have a buffer..
			bufferSize = *((unsigned int *)buffer);

			if ( bufferSize <= payloadSize + 8 ) {
				// Size of buffer is too small.. reinit buffer
				CVerbVerbArg ( "ResizeBuffer [%u]: size [%u] too small. Reinit.", context->id, bufferSize );
				free ( buffer );
				buffer = 0;
				context->outputBuffer = 0;
			}
		}

		if ( !buffer ) {
			bufferSize	= payloadSize * 2 + 8;
			CVerbVerbArg ( "ResizeBuffer [%u]: Allocating [%u].", context->id, bufferSize );

			buffer = (char *)malloc ( bufferSize );
			if ( !buffer ) {
				CErrArg ( "ResizeBuffer: Failed to allocate [%d] bytes for new buffer", payloadSize );
				return false;
			}

			*((unsigned int *)buffer) = bufferSize;

			context->outputBuffer = buffer;
		}

		*(((unsigned int *)buffer) + 1) = payloadSize;

		return true;
	}
	*/

}
