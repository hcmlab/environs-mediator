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
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace hcm.environs
{
    /// <summary>
    /// A DeviceInfo object serves as container for device information.
    /// Environs usually creates an array of DeviceInfo objects (as a result of calls to GetDevicesAvailable, GetDevicesNearby, GetDevicesFromMediator)
    /// in order to notify about the available devices within the environment.
    /// </summary>
    public class DeviceInfo : INotifyPropertyChanged
    {

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string name)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(name));
        }

        /** The device ID within the environment */
        public Int32 id;
        public Int32 ID
        {
            get { return id; }
            set
            {
                if (id == value)
                    return;
                id = value;
                OnPropertyChanged("ID");
            }
        }

        /** IP from device. The IP address reported by the device which it has read from network configuration. */
        public string ip;
        public string IP
        {
            get { return ip; }
            set
            {
                if (ip == value)
                    return;
                ip = value;
                OnPropertyChanged("IP");
            }
        }

        /** IP external. The IP address which was recorded by external sources (such as the Mediator) during socket connections.
         * This address could be different from IP due to NAT, Router, Gateways behind the device.
         */
        public string ipe;
        public string IPe
        {
            get { return ipe; }
            set
            {
                if (ipe == value)
                    return;
                ipe = value;
                OnPropertyChanged("IPe");
            }
        }

        /** The tcp port on which the device listens for device connections. */
        public ushort tcpPort;
        public ushort TCPPort
        {
            get { return tcpPort; }
            set
            {
                if (tcpPort == value)
                    return;
                tcpPort = value;
                OnPropertyChanged("TCPPort");
            }
        }

        /** The udp port on which the device listens for device connections. */
        public ushort udpPort;
        public ushort UdpPort
        {
            get { return udpPort; }
            set
            {
                if (udpPort == value)
                    return;
                udpPort = value;
                OnPropertyChanged("UdpPort");
            }
        }

        /** The number of alive updates noticed by the mediator layer since its appearance within the application environment. */
        public uint updates;
        public uint Updates
        {
            get { return updates; }
            set
            {
                if (updates == value)
                    return;
                updates = value;
                OnPropertyChanged("Updates");
            }
        }

        /** BroadcastFound is true if the device has been seen on the broadcast channel of the current network. */
        public bool broadcastFound;
        public bool BroadcastFound
        {
            get { return broadcastFound; }
            set
            {
                if (broadcastFound == value)
                    return;
                broadcastFound = value;
                OnPropertyChanged("BroadcastFound");
            }
        }

        public bool unavailable;
        public bool Unavailable
        {
            get { return unavailable; }
            set
            {
                if (unavailable == value)
                    return;
                unavailable = value;
                OnPropertyChanged("Unavailable");
            }
        }

        /** isConnected is true if the device is currently in the connected state. */
        public bool isConnected;
        public bool IsConnected
        {
            get { return isConnected; }
            set
            {
                if (isConnected == value)
                    return;
                isConnected = value;
                OnPropertyChanged("IsConnected");
            }
        }

        /** The device type, which match the constants DEVICE_TYPE_* . */
        public char deviceType;
        public char DeviceType
        {
            get { return deviceType; }
            set
            {
                if (deviceType == value)
                    return;
                deviceType = value;
                OnPropertyChanged("DeviceType");
            }
        }

        private string _deviceType;
        public string DeviceTypeString
        {
            get
            {
                if (String.IsNullOrEmpty(_deviceType))
                {
                    switch (deviceType)
                    {
                        case 'D':
                            _deviceType = "Display";
                            break;
                        case 'R':
                            _deviceType = "Surface 1";
                            break;
                        case 'S':
                            _deviceType = "Surface 2";
                            break;
                        case 'T':
                            _deviceType = "Tablet";
                            break;
                        case 'P':
                            _deviceType = "Smartphone";
                            break;
                        default:
                            _deviceType = "Unknown";
                            break;
                    }
                }
                return _deviceType;
            }
            set
            {
                if (_deviceType == value)
                    return;
                _deviceType = value;
                OnPropertyChanged("DeviceTypeString");
            }
        }

        /** The device name. */
        public string deviceName;
        public string DeviceName
        {
            get { return deviceName; }
            set
            {
                if (deviceName.Equals(value))
                    return;
                deviceName = value;
                OnPropertyChanged("DeviceName");
            }
        }

        /** The project name of the appliction environment. */
        public string projectName;
        public string ProjectName
        {
            get { return projectName; }
            set
            {
                if (projectName.Equals(value))
                    return;
                projectName = value;
                OnPropertyChanged("ProjectName");
            }
        }

        /** The applcation name of the appliction environment. */
        public string appName;
        public string AppName
        {
            get { return appName; }
            set
            {
                if (appName.Equals(value))
                    return;
                appName = value;
                OnPropertyChanged("AppName");
            }
        }

        public void updateDevice(DeviceInfo newDevice)
        {
            id = newDevice.id;
            ip = newDevice.ip;
            ipe = newDevice.ipe;
            TCPPort = newDevice.TCPPort;
            UdpPort = newDevice.UdpPort;
            Updates = newDevice.Updates;
            BroadcastFound = newDevice.BroadcastFound;
            Unavailable = newDevice.Unavailable;
            IsConnected = newDevice.IsConnected;
            DeviceType = newDevice.DeviceType;
            DeviceName = newDevice.DeviceName;
            ProjectName = newDevice.ProjectName;
            AppName = newDevice.AppName;
        }

        public void Update(DeviceInfo device)
        {
            updateDevice(device);
        }

        override
        public string ToString()
        {
            return "ID " + this.ID + ": " + DeviceTypeString + ", "
                    + this.deviceName + ", [" + this.appName + "/" + this.projectName + "], IP [" + this.IPe + "/" + this.IP + "]" + (this.broadcastFound ? "" : " M");
        }
    }
}
