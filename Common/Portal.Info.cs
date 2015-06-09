/**
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
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace hcm.environs
{
    /// <summary>
    /// A PortalInfo object serves as container for portal information.
    /// Environs makes use of such objects to get/set portal details.
    /// </summary>
    public class PortalInfo
    {
        public Int32 deviceID;
        public int portalID;
        public int flags;

	    public int centerX;
	    public int centerY;
	    public int width;
	    public int height;
	    public float orientation;

        public void resetFlags() {
            flags = 0;
        }
        
        public void setLocation(int centerX, int centerY)
        {
            this.centerX = centerX;
            this.centerY = centerY;
            flags |= Environs.PORTAL_INFO_FLAG_LOCATION;
        }

        public void setSize(int width, int height)
        {
            this.width = width;
            this.height = height;
            flags |= Environs.PORTAL_INFO_FLAG_SIZE;
        }

        public void setOrientation(float angle)
        {
            this.orientation = angle;
            flags |= Environs.PORTAL_INFO_FLAG_ANGLE;
        }

	    public String toString() {
		
            return ("Portal: center coordinates [ " + centerX + " / " + centerY + " ], size [ " 
            + width + " / " + height + " ], orientation [ " + orientation + " ]");
        }
    }
}
