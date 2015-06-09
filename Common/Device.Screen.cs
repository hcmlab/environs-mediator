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
    /// A DeviceScreen object serves as container for information about a device's screen sizes.
    /// Environs usually creates a DeviceScreen object (as a result of calls to GetDeviceScreenSizes)
    /// in order to notify about the device's screen sizes of a connected device.
    /// </summary>
    public class DeviceScreen
    {
	    /** The device ID within the environment */
        public Int32    deviceID;
        public Int32    width;
	    public double	width_mm;
        public Int32    height;
	    public double	height_mm;

        override
        public string ToString()
        {
            return ("DeviceID [" + deviceID + "] width [" + width + "] height ["
                    + height + "] width_mm [" + width_mm + "] height_mm [" + height_mm + "]");
        }
    }
}
