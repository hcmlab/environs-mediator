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
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace hcm.environs
{
    /// <summary>
    /// A DeviceInstance object serves as container for device information.
    /// Environs usually creates an array of DeviceInstance objects (as a result of calls to GetDevicesAvailable, GetDevicesNearby, GetDevicesFromMediator)
    /// in order to notify about the available devices within the environment.
    /// </summary>
    public class DeviceInstance : INotifyPropertyChanged
    {
        private const String className = "DeviceInstance";

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(String name)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(name));
        }

        public EnvironsObserver observers = null;

        /** disposed is true, if this DeviceInstance object is not managed by Environs anymore. */
        private bool disposed_ = false;
        public bool disposed
        {
            get { return disposed_; }
            internal set
            {
                if (disposed_ == value)
                    return;
                disposed_ = value;
                OnPropertyChanged("disposed"); NotifyObservers(-1);
            }
        }

        /** The device ID within the environment */
        internal int deviceID_;
        public int deviceID
        {
            get { return deviceID_; }
            set
            {
                if (deviceID_ == value)
                    return;
                deviceID_ = value;
                OnPropertyChanged("deviceID"); NotifyObservers(Environs.DEVICE_INFO_ATTR_IDENTITY);
            }
        }

        /** ip from device. The ip address reported by the device which it has read from network configuration. */
        internal String ip_;
        public String ip
        {
            get { return ip_; }
            set
            {
                if (ip_ == value)
                    return;
                ip_ = value;
                OnPropertyChanged("ip"); NotifyObservers(Environs.DEVICE_INFO_ATTR_IP);
            }
        }

        /** ip external. The ip address which was recorded by external sources (such as the Mediator) during socket connections.
         * This address could be different from ip due to NAT, Router, Gateways behind the device.
         */
        internal String ipe_;
        public String ipe
        {
            get { return ipe_; }
            set
            {
                if (ipe_ == value)
                    return;
                ipe_ = value;
                OnPropertyChanged("ipe"); NotifyObservers(Environs.DEVICE_INFO_ATTR_IPE);
            }
        }

        /** The tcp port on which the device listens for device connections. */
        internal ushort tcpPort_;
        public ushort tcpPort
        {
            get { return tcpPort_; }
            set
            {
                if (tcpPort_ == value)
                    return;
                tcpPort_ = value;
                OnPropertyChanged("tcpPort"); NotifyObservers(Environs.DEVICE_INFO_ATTR_TCP_PORT);
            }
        }

        /** The udp port on which the device listens for device connections. */
        internal ushort udpPort_;
        public ushort udpPort
        {
            get { return udpPort_; }
            set
            {
                if (udpPort_ == value)
                    return;
                udpPort_ = value;
                OnPropertyChanged("udpPort"); NotifyObservers(Environs.DEVICE_INFO_ATTR_UDP_PORT);
            }
        }

        /** The number of alive updates_ noticed by the mediator layer since its appearance within the application environment. */
        internal uint updates_;
        public uint updates
        {
            get { return updates_; }
            set
            {
                if (updates_ == value)
                    return;
                updates_ = value;
                OnPropertyChanged("updates");
            }
        }

        /** A value that describes the device platform_. */
        internal Environs.Platforms platform_;
        public Environs.Platforms platform
        {
            get { return platform_; }
            set
            {
                if (platform_ == value)
                    return;
                platform_ = value;
                OnPropertyChanged("platform"); NotifyObservers(Environs.DEVICE_INFO_ATTR_DEVICE_PLATFORM);
            }
        }

        /** BroadcastFound is a value of DEVICEINFO_DEVICE_* and determines whether the device has been seen on the broadcast channel of the current network and/or from a Mediator service. */
        internal int broadcastFound_;
        public int broadcastFound
        {
            get { return broadcastFound_; }
            set
            {
                if (broadcastFound_ == value)
                    return;
                broadcastFound_ = value;
                OnPropertyChanged("broadcastFound"); NotifyObservers(Environs.DEVICE_INFO_ATTR_BROADCAST_FOUND);
            }
        }

        internal bool unavailable_;
        public bool unavailable
        {
            get { return unavailable_; }
            set
            {
                if (unavailable_ == value)
                    return;
                unavailable_ = value;
                OnPropertyChanged("unavailable"); NotifyObservers(Environs.DEVICE_INFO_ATTR_UNAVAILABLE);
            }
        }

        /** isConnected_ is true if the device is currently in the connected state. */
        internal bool isConnected_;
        public bool isConnected
        {
            get { return isConnected_; }
            set
            {
                if (isConnected_ == value)
                    return;
                isConnected_ = value;
                OnPropertyChanged("isConnected"); NotifyObservers(Environs.DEVICE_INFO_ATTR_ISCONNECTED);
            }
        }

        /** The device type, which match the constants DEVICE_TYPE_* . */
        internal char deviceType_;
        public char deviceType
        {
            get { return deviceType_; }
            set
            {
                if (deviceType_ == value)
                    return;
                deviceType_ = value;
                OnPropertyChanged("deviceType"); NotifyObservers(Environs.DEVICE_INFO_ATTR_DEVICE_PLATFORM);
            }
        }

        /** The device name. */
        internal String deviceName_;
        public String deviceName
        {
            get { return deviceName_; }
            set
            {
                if (deviceName_.Equals(value))
                    return;
                deviceName_ = value;
                OnPropertyChanged("deviceName"); NotifyObservers(Environs.DEVICE_INFO_ATTR_IDENTITY);
            }
        }

        /** The project name of the appliction environment. */
        internal String projectName_;
        public String projectName
        {
            get { return projectName_; }
            set
            {
                if (projectName_ == null)
                    value = Environs.GetProjectName();
                else if (projectName_.Equals(value))
                    return;
                projectName_ = value;
                OnPropertyChanged("projectName"); NotifyObservers(Environs.DEVICE_INFO_ATTR_IDENTITY);
            }
        }

        /** The applcation name of the appliction environment. */
        internal String appName_;
        public String appName
        {
            get { return appName_; }
            set
            {
                if (appName_ == null)
                    value = Environs.GetProjectName();
                else if (appName_.Equals(value))
                    return;
                appName_ = value;
                OnPropertyChanged("appName"); NotifyObservers(Environs.DEVICE_INFO_ATTR_IDENTITY);
            }
        }

        
        /** A progress value of the current connection status (if any) */
        internal int connectProgress_ = 0;
        public int connectProgress
        {
            get { return connectProgress_; }
            set
            {
                if (connectProgress_ == value)
                    return;
                connectProgress_ = value;
                OnPropertyChanged("connectProgress"); NotifyObservers(Environs.DEVICE_INFO_ATTR_CONNECT_PROGRESS);
            }
        }


        /** Determines whether the device is in physical contact (1) or not (0) with this device. */
        internal int directStatus_ = 0;
        public int directStatus
        {
            get { return directStatus_; }
            set
            {
                if (directStatus_ == value)
                    return;
                directStatus_ = value;
                OnPropertyChanged("directStatus"); NotifyObservers(Environs.NOTIFY_CONTACT_DIRECT_CHANGED);
            }
        }


        public void AddObserver(EnvironsObserver observer)
        {
            if (observers != null)
                observers += observer;
        }

        public void RemoveObserver(EnvironsObserver observer)
        {
            if (observers != null)
                observers -= observer;
        }


        void NotifyObservers(int changed)
        {
            if (observers != null)
                observers(this, changed);
        }


        public static bool isPlatformType(Environs.Platforms srcPlatform, Environs.Platforms platform)
        {
            int src = (int)srcPlatform;
            int dst = (int)platform;
            return ((src & dst) == dst);
        }

        internal String _deviceType;
        public String deviceTypeString
        {
            get
            {
                if (isPlatformType(platform_, Environs.Platforms.Tablet_Flag))
                    return "Tablet";
                else if (isPlatformType(platform_, Environs.Platforms.Smartphone_Flag))
                    return "Smartphone";
                else if (isPlatformType(platform_, Environs.Platforms.MSSUR01))
                    return "Surface 1";
                else if (isPlatformType(platform_, Environs.Platforms.SAMSUR40))
                    return "Surface 2";
                else if (isPlatformType(platform_, Environs.Platforms.Tabletop_Flag))
                    return "Tabletop";
                else if (isPlatformType(platform_, Environs.Platforms.Display_Flag))
                    return "Display";
                return "Unknown";
            }
            set
            {
                if (_deviceType == value)
                    return;
                _deviceType = value;
                OnPropertyChanged("deviceTypeString");
            }
        }

               

        public bool CopyInfo(DeviceInstance device)
        {
            if (device == null)
                return false;

            deviceID_ = device.deviceID_;
            ip_ = device.ip_;
            ipe_ = device.ipe_;
            tcpPort_ = device.tcpPort;
            udpPort_ = device.udpPort;
            updates_ = device.updates;
            broadcastFound_ = device.broadcastFound;
            unavailable_ = device.unavailable;
            isConnected_ = device.isConnected;
            deviceType_ = device.deviceType;
            deviceName_ = device.deviceName;
            projectName_ = device.projectName;
            appName_ = device.appName;
            return true;
        }


        public void Update(DeviceInstance device)
        {
            deviceID = device.deviceID_;
            ip = device.ip_;
            ipe = device.ipe_;
            tcpPort = device.tcpPort_;
            udpPort = device.udpPort_;
            updates = device.updates_;
            broadcastFound = device.broadcastFound_;
            unavailable = device.unavailable_;
            isConnected = device.isConnected_;
            deviceType = device.deviceType_;
            deviceName = device.deviceName_;
            projectName = device.projectName_;
            appName = device.appName_;
        }


        public bool EqualsID(DeviceInstance equalTo)
        {
            return (deviceID_ == equalTo.deviceID_ && EqualsAppEnv(equalTo));
        }

        public bool EqualsID(int deviceID, String projectName, String appName)
        {
            return (deviceID == this.deviceID_ && EqualsAppEnv(projectName, appName));
        }


        public bool EqualsAppEnv(DeviceInstance equalTo)
        {
            return EqualsAppEnv(equalTo.projectName_, equalTo.appName_);
        }

        public bool EqualsAppEnv(String projectName, String appName)
        {
            if (projectName == null)
                projectName = Environs.GetProjectName();
            if (appName == null)
                appName = Environs.GetApplicationName();

            return (appName_.Equals(appName) && projectName_.Equals(projectName));
        }

        public bool LowerThanAppEnv(DeviceInstance equalTo)
        {
            return LowerThanAppEnv(equalTo.projectName_, equalTo.appName_);
        }


        public bool LowerThanAppEnv(String projectName, String appName)
        {
            if (projectName == null)
                projectName = Environs.GetProjectName();
            if (appName == null)
                appName = Environs.GetApplicationName();

            return (projectName_.CompareTo(projectName) < 0 || appName_.CompareTo(appName) < 0);
        }


        internal String GetBroadcastString(bool fullText)
        {
            switch (broadcastFound_)
            {
                case Environs.DEVICEINFO_DEVICE_BROADCAST:
                    return fullText ? "Nearby" : "B";
                case Environs.DEVICEINFO_DEVICE_MEDIATOR:
                    return fullText ? "Mediator" : "M";
                case Environs.DEVICEINFO_DEVICE_BROADCAST_AND_MEDIATOR:
                    return fullText ? "Med+Near" : "MB";                   
            }
            return "U";
        }

        override
        public String ToString()
        {
            return "ID " + this.deviceID_ + ": " + deviceTypeString + ", "
                    + this.deviceName_ + ", [" + this.appName_ + "/" + this.projectName_ + "], ip [" + this.ipe + "/" + this.ip + "]" + GetBroadcastString(true);
        }


        public void SetDirectContact(int status)
        {
            if (directStatus == status)
                return;
            directStatus = status;
        }


        public void Dispose()
        {
            if (disposed)
            {
                Utils.Log(6, className, "Dispose: " + deviceID_ + " already disposed.");
                return;
            }
            disposed = true;
            PropertyChanged = null;
        }


        public bool Connect()
        {
            return Environs.DeviceConnect(deviceID_, projectName_, appName_, Environs.CALL_ASYNC) != 0;
        }


        public bool Disconnect()
        {
            int count = devicePortals.Count;

            while (devicePortals.Count > 0 && count > 0)
            {
                try
                {
                    ((PortalInstance)devicePortals[0]).Dispose();
                }
                catch (Exception)
                {
                }
                count--;
            }
            return Environs.DeviceDisconnect(deviceID_, projectName_, appName_, Environs.CALL_ASYNC);
        }

        /**
	     * Send a file from the local filesystem to this device.&nbsp;The devices must be connected before for this call.
	     *
	     * @param fileID        A user-customizable id that identifies the file to be send.
	     * @param fileDescriptor (e.g. filename)
	     * @param filePath      The path to the file to be send.
	     * @return success
	     */
        public bool SendFile(int fileID, String fileDescriptor, String filePath)
        {
            return Environs.SendFile(deviceID_, projectName_, appName_, Environs.CALL_ASYNC, fileID, fileDescriptor, filePath);
        }

        /**
	     * Send a buffer with bytes to a device.&nbsp;The devices must be connected before for this call.
	     *
	     * @param fileID        A user-customizable id that identifies the file to be send.
	     * @param fileDescriptor (e.g. filename)
	     * @param buffer        A buffer to be send.
	     * @param bytesToSend number of bytes in the buffer to send
	     * @return success
	     */
        public bool SendBuffer(int fileID, String fileDescriptor, IntPtr buffer, int bytesToSend)
        {
            return Environs.SendBuffer(deviceID_, projectName_, appName_, Environs.CALL_ASYNC, fileID, fileDescriptor, buffer, bytesToSend);
        }

        /**
	     * Send a string message to a device through one of the following ways.&nbsp;
	     * If a connection with the destination device has been established, then use that connection.
	     * If the destination device is not already connected, then distinguish the following cases:
	     * (1) If the destination is within the same network, then try establishing a direct connection.
	     * (2) If the destination is not in the same network, then try sending through the Mediator (if available).
	     * (3) If the destination is not in the same network and the Mediator is not available, then try establishing
	     * 		a STUNT connection with the latest connection details that are available.
	     *
	     * On successful transmission, Environs returns true if the devices already had an active connection,
	     * or in case of a not connected status, Environs notifies the app by means of a NOTIFY_SHORT_MESSAGE_ACK through
	     * a registered EnvironsObserver instance.
	     *
	     * @param message       A message to be send.
	     * @return success
	     */
        public bool SendMessage(String message)
        {
            return Environs.SendMessage(deviceID_, projectName_, appName_, Environs.CALL_ASYNC, message);
        }



        #region Portal Instance Management

        /** A collection of PortalInstances that this device has established or is managing. */
        internal ArrayList devicePortals = new ArrayList();



        /**
         * Creates a portal instance.
         *
         * @param request   The portal request.
         * @return PortalInstance-object
         */
        public PortalInstance PortalCreate(int request)
        {
            return PortalCreate(request & Environs.PORTAL_DIR_MASK, (Environs.PortalType)(request & Environs.PORTAL_TYPE_MASK));
        }

        /**
         * Creates a portal instance.
         *
         * @param Environs_PORTAL_DIR   A value of PORTAL_DIR_* that determines whether an outgoing or incoming portal.
         * @param portalType	        Project name of the application environment
         * @return PortalInstance-object
         */
        public PortalInstance PortalCreate(int Environs_PORTAL_DIR, Environs.PortalType portalType)
        {
            if (!isConnected_)
                return null;

            PortalInstance portal = new PortalInstance();

            if (!portal.Init(this, Environs_PORTAL_DIR, portalType))
                return null;

            return portal;
        }


        /**
         * Creates a portal instance with a given portalID.
         *
         * @param portalID   The portalID received from native layer.
         * @return PortalInstance-object
         */
        public PortalInstance PortalCreateID(int portalID)
        {
            Utils.Log(6, className, "PortalCreateID: " + portalID);

            if (!isConnected)
                return null;

            PortalInstance portal = new PortalInstance();

            if (!portal.Init(this, portalID))
                return null;

            return portal;
        }


        /**
         * Query the first PortalInstance that manages an outgoing portal.
         *
         * @return PortalInstance-object
         */
        public PortalInstance PortalGetOutgoing()
        {
            return PortalGet(true);
        }

        /**
         * Query the first PortalInstance that manages an incoming portal.
         *
         * @return PortalInstance-object
         */
        public PortalInstance PortalGetIncoming()
        {
            return PortalGet(false);
        }


        internal PortalInstance PortalGet(bool outgoing)
        {
            int count = devicePortals.Count;

            lock (devicePortals)
            {
                foreach (PortalInstance portal in devicePortals)
                {
                    if (portal != null && portal.outgoing == outgoing)
                        return portal;
                }
            }
            return null;
        }

        
        /**
         * Query the first PortalInstance that manages a waiting/temporary incoming/outgoing portal.
         *
         * @return PortalInstance-object
         */
        internal PortalInstance PortalGetWaiting(bool outgoing)
        {
            int count = devicePortals.Count;

            lock (devicePortals)
            {
                foreach (PortalInstance portal in devicePortals)
                {
                    if (portal != null && portal.portalID < 0 && portal.outgoing == outgoing)
                        return portal;
                }
            }
            return null;
        }


        #endregion
    }
}
