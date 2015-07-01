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
    /// A DeviceDisplay object serves as container for information about a device's screen sizes.
    /// Environs usually creates a DeviceDisplay object (as a result of calls to GetDeviceScreenSizes)
    /// in order to notify about the device's screen sizes of a connected device.
    /// </summary>
    public class DeviceDisplay
    {
	    /** The device ID within the environment */
        public int      deviceID;

        public int      width;
        public int      width_mm;
        public int      height;
        public int      height_mm;
        public int      orientation;
        public float    dpi;

        override
        public string ToString()
        {
            return ("deviceID [" + deviceID + "] width [" + width + "] height ["
                    + height + "] width_mm [" + width_mm + "] height_mm [" + height_mm + "]");
        }
    }
}
