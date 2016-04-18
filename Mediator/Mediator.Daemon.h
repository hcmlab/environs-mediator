/**
 * Mediator Server main class
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
#ifndef ENVIRONS_MEDIATOR_SERVER_H
#define ENVIRONS_MEDIATOR_SERVER_H

#include "Mediator.h"
using namespace environs;

#include <vector>
#include <map>
#include <string>
#include <ctime>
using namespace std;

#ifdef WIN32
// -> Windows specific includes
#include <WinSock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#define CONFFILE						"mediator.conf"
#define DATAFILE						"mediator.data"
#define DEVMAPFILE						"mediator.devmap"
#define LOGFILE							"mediator.log"
#define USERSDBFILE						"users.db"

#define MEDIATOR_CONTEXT_BUFFER_SIZE_MAX	(MEDIATOR_REC_BUFFER_SIZE_MAX_MOBILE >> 1)


// Default configuration

namespace environs
{
	class ILock1
	{
		pthread_mutex_t lock1;
		bool			init1;

	public:
		bool Init1 ();
		bool Lock1 ( const char * func );
		bool Unlock1 ( const char * func );
		ILock1 ();
		~ILock1 ();
	};


	typedef struct _ValuePack
	{
		string          value;
		unsigned int    size;
		std::time_t     timestamp;
	} ValuePack;


	class ListValues : public ILock
	{
	public:
		msp ( string, ValuePack )	values;
	};


	class AppsList : public ILock
	{
	public:
		msp ( string, ListValues ) apps;
	};


	class AreaApps : public ILock, public ILock1
	{
	public:
		unsigned int						id;
		string                              name;
		msp ( string, ApplicationDevices )	apps;
		msp ( long long, ThreadInstance )   notifyTargets;
	};


	class AreasList : public ILock
	{
	public:
		msp ( string, AreaApps ) list;
	};


	class AreasMap : public ILock
	{
	public:
		msp ( string, AppsList ) list;
	};


	typedef struct _DeviceMapping
	{
		int     deviceID;
		int     authLevel;
		char    authToken [ MAX_NAMEPROPERTY + 1 ];
	}
	DeviceMapping;


	class DeviceMappings : public ILock
	{
	public:
		msp ( string, DeviceMapping ) list;
	};


	class InstanceMap : public ILock
	{
	public:
		msp ( long long, ThreadInstance ) list;
	};


	class InstanceList : public ILock
	{
	public:
		vsp ( ThreadInstance ) list;
	};


	typedef struct _UserItem
	{
		int             authLevel;
		string			pass;
	}
	UserItem;


	class NotifyQueueContext
	{
	public:
		int                         notify;
		sp ( DeviceInstanceNode )   device;
    };
    
    
#ifdef ENABLE_SINGLE_CLIENT_THREAD
    
    class ClientContext : public EnvLock
    {
    public:
        ClientContext () : clientCount ( 0 )
#ifdef ENABLE_WINSOCK_CLIENT_THREADS
			, revent ( 0 )
#else
			, desc ( 0 ), descCapacity ( 0 )
#endif		
		{}
        
        ~ClientContext ()
        {
            clients.clear();            

#ifdef ENABLE_WINSOCK_CLIENT_THREADS
			CloseWSAHandle_n ( revent );
#else
			if ( desc )
				free ( desc );
#endif
        }
        
        bool Add ( const sp ( ThreadInstance ) &clientSP );
        bool Remove ( const sp ( ThreadInstance ) &clientSP );
        
        vsp ( ThreadInstance ) clients;
        size_t  clientCount;

#ifdef ENABLE_WINSOCK_CLIENT_THREADS
		HANDLE	revent;
#else
		struct pollfd * desc;
		size_t descCapacity;
#endif
    };
    
#endif
    
    
    class MediatorDaemon : public environs::Mediator
	{
	public:
		MediatorDaemon ();
		~MediatorDaemon ();

		bool									InitMediator ();
		void									PrintSmallHelp ();
		void									PrintHelp ();

		bool									OpenLog ();
		bool									CloseLog ();

		void									InitDefaultConfig ();
		bool									LoadConfig ();
		bool									LoadProjectValues ();
		bool									LoadUserDBEnc ();
		bool									LoadUserDB ();
		bool                                    LoadUserDB ( istream& instream );
		bool									LoadKeys ();
		void									ReleaseKeys ();
		bool									SaveConfig ();
		bool									SaveProjectValues ();
		bool									SaveUserDB ();
		bool									AddUser ( int authLevel, const char * userName, const char * pass );

		bool									LoadDeviceMappingsEnc ();
		bool									LoadDeviceMappings ();
		bool                                    LoadDeviceMappings ( istream& instream );
		bool									SaveDeviceMappings ();

		bool									CreateThreads ();
		void									Run ();
        
        static void                             DisposeClass ();
		void									Dispose ();
		bool									ReleaseThreads ();

		void									ReleaseDevices ();
		void									ReleaseDeviceMappings ();
		void									ReleaseClient ( ThreadInstance * client );

		void									BuildBroadcastMessage ( bool withStatus = true );

	private:
		bool									allocated;
		bool									acceptEnabled;
		pthread_mutex_t							thread_lock;
		pthread_cond_t							thread_condition;

		vector<unsigned short>					ports;
		vsp ( MediatorThreadInstance )          listeners;
		unsigned int							networkOK;
		unsigned int							networkMask;
		bool									usersDBDirty;
		bool									configDirty;
		bool									deviceMappingDirty;


		InstanceList                            acceptClients;
		bool                                    RemoveAcceptClient ( ThreadInstance * );

		sp ( ApplicationDevices )				GetApplicationDevices ( const char * appName, const char * areaName );
		void									UnlockApplicationDevices ( ApplicationDevices * appDevices );
		DeviceInstanceNode *					GetDeviceInstance ( int deviceID, DeviceInstanceNode * devices );

		sp ( ThreadInstance )                   GetThreadInstance ( ThreadInstance * sourceClient, int deviceID, const char * areaName, const char * appName );

		sp ( ThreadInstance ) 					GetSessionClient ( long long sessionID );

		msp ( long long, ThreadInstance )       notifyTargets;
		void                                    UpdateNotifyTargets ( const sp ( ThreadInstance ) &client, int filterMode );

		unsigned int							CollectDevicesCount ( DeviceInstanceNode * sourceDevice, int filterMode );

		AreasMap                                areasMap;

		bool									reqAuth;
		pthread_mutex_t							usersDBLock;
		map<string, UserItem *>                 usersDB;

		DeviceMappings                          deviceMappings;

		bool									anonymousLogon;
		char                                    anonymousUser [ MAX_NAMEPROPERTY + 1 ];
		char                                    anonymousPassword [ ENVIRONS_USER_PASSWORD_LENGTH + 1 ];

		char									inputBuffer [ MEDIATOR_CLIENT_MAX_BUFFER_SIZE ];
		char	*								input;

		unsigned int							sessionCounter;
		InstanceMap                             sessions;

		unsigned int                            stuntID;
		InstanceMap                             stuntClients;

		char								*	privKey;
		unsigned int							privKeySize;
		unsigned int                            encPadding;
		AESContext                              aesCtx;
		char                                    aesKey [ 32 ];

		sp ( ApplicationDevices )				GetDeviceList ( char * areaName, char * appName, pthread_mutex_t ** mutex,
			int ** pDevicesAvailable, DeviceInstanceNode ** &list );

		unsigned int							areasCounter;
		map<unsigned int, string>				areaIDs;

		unsigned int							appsCounter;
		map<unsigned int, string>				appIDs;

		AreasList 								areas;
        queue<NotifyQueueContext *>             notifyQueue;
        
		void									RemoveDevice ( unsigned int ip, char * msg );
        void									RemoveDevice ( DeviceInstanceNode * device, bool useLock = true );
        void									RemoveDeviceUnlock ( ApplicationDevices * appDevs, DeviceInstanceNode * device );
		void									RemoveDevice ( int deviceID, const char * areaName, const char * appName );

#ifdef __cplusplus
		void									UpdateDeviceInstance ( const sp ( DeviceInstanceNode ) & device, bool added, bool changed );
#endif

		unsigned int                            bannAfterTries;
		pthread_mutex_t							bannedIPsLock;
		std::map<unsigned int, std::time_t>		bannedIPs;
		std::map<unsigned int, unsigned int>	bannedIPConnects;

		bool									IsIpBanned ( unsigned int ip );
		void									BannIP ( unsigned int ip );
		void									BannIPRemove ( unsigned int ip );

		int										ScanForParameters ( char * buffer, unsigned int maxLen, const char * delim, char ** params, int maxParams );

		bool									AddToArea ( sp ( ListValues ) &values, const char * key, const char * value, unsigned int valueSize );
		bool									AddToArea ( const char * project, const char * app, const char * key, const char * value );
		bool									sendDatabase ( int sock, struct sockaddr * addr );

        int										SendBuffer ( ThreadInstance * client, void * msg, unsigned int msgLen );

		bool									SendPushNotification ( map<string, ValuePack*> * values, int clientID, const char * value );
		bool									HTTPPostRequest ( string domain, string path, string key, string jsonData );

		void *									BroadcastThread ();

		static void *							AcceptorStarter ( void * arg );
		void *									Acceptor ( void * arg );

		static void *							MediatorUdpThreadStarter ( void * arg );
		void *									MediatorUdpThread ( void * arg );

		static void *							ClientThreadStarter ( void * arg );
		int                                     ClientThread ( const sp ( ThreadInstance ) &clientSP );

#ifdef ENABLE_SINGLE_CLIENT_THREAD
        unsigned int							clientThreadCount;
        EnvSignal                               clientEvent;
        EnvThread							**	clientThreads;
        ClientContext                       **  clientContexts;
        
        bool                                    clientThreadsAlive;
        bool                                    StartClientThreads ();
        void                                    StopClientThreads ();
        
        static void                         *	ClientThreadsStarter ( void * arg );
        void                                    ClientThreads ( int threadNr );
        
        void                                    ClientRemove ( ThreadInstance * client );
#endif
		bool									HandleRequest ( ThreadInstance * client, char * buffer );
		bool									UpdateDeviceRegistry ( sp ( DeviceInstanceNode ) device, unsigned int ip, char * msg );

		int										HandleRegistration ( int &deviceID, const sp ( ThreadInstance ) &clientSP, unsigned int bytesLeft, char * msg, unsigned int msgLen );
		int										HandleRegistrationV4 ( int &deviceID, const sp ( ThreadInstance ) &clientSP, unsigned int bytesLeft, char * msg, unsigned int msgLen );

		bool									HandleDeviceRegistration ( const sp ( ThreadInstance ) &clientSP, unsigned int ip, char * msg );
		bool									HandleDeviceRegistrationV4 ( const sp ( ThreadInstance ) &clientSP, unsigned int ip, char * msg );
		bool									SecureChannelAuth ( ThreadInstance * client );
		void									HandleStuntSocketRegistration ( ThreadInstance * stuntClient, sp ( ThreadInstance ) orgClient, char * msg, unsigned int msgLen );
        bool									HandleSTUNTRequest ( const sp ( ThreadInstance ) &clientSP, STUNTReqPacketV6 * msg );
        bool									HandleSTUNTRequestV5 ( const sp ( ThreadInstance ) &clientSP, STUNTReqPacket * msg );
		bool									HandleSTUNTRequestV4 ( const sp ( ThreadInstance ) &clientSP, STUNTReqPacketV4 * msg );
		bool									NotifySTUNTRegRequest ( ThreadInstance * client );
		int                                     GetNextDeviceID ( char * areaName, char * appName, unsigned int ip );

		void									HandleCLSGenHelp ( ThreadInstance * client );
		void									HandleCertSign ( ThreadInstance * client, char * msg );

		void									HandleDeviceFlagSet ( ThreadInstance * client, char * msg );

		bool									HandleSTUNRequest ( ThreadInstance * sourceClient, char * msg );
		bool									HandleSTUNRequest ( ThreadInstance * destClient, int sourceID, const char * areaName, const char * appName, unsigned int IP, unsigned int Port );
		bool									HandleSTUNRequestV4 ( ThreadInstance * destClient, int sourceID, const char * areaName, const char * appName, unsigned int IP, unsigned int Port );

        bool									HandleQueryDevices ( const sp ( ThreadInstance ) &client, char * msg );
        bool									HandleQueryDevicesV5 ( const sp ( ThreadInstance ) &client, char * msg );
		bool									HandleQueryDevicesV4 ( const sp ( ThreadInstance ) &client, char * msg );

		bool									HandleShortMessage ( ThreadInstance * client, char * msg, unsigned int size );
        
#ifdef USE_MEDIATOR_DAEMON_SEND_THREAD
		unsigned int							sendThreadCount;
		EnvThread							**	sendThreads;

#ifdef ENABLE_WINSOCK_SEND_THREADS
		HANDLE									sendEvents [2];
#else
		EnvSignal                               sendEvent;
#endif
        
        bool                                    sendThreadsAlive;
        bool                                    StartSendThreads ();
        void                                    StopSendThreads ();
        
        static void                         *	SendThreadStarter ( void * arg );
        
        void                                    SendThread ( int threadNr );
        bool                                    PushSend ( ThreadInstance * client, const sp ( SendLoad ) &dataSP, unsigned int size );
        bool                                    PushSend ( ThreadInstance * client, void * buffer, unsigned int size, bool copy = true, unsigned int seqNr = 0 );

		bool                                    PushSend ( ThreadInstance * client, const sp ( SendLoad ) &dataSP, char * toSend, unsigned int toSendSize, unsigned int toSendCurrent );
		bool                                    PushSend ( ThreadInstance * client, char * toSend, unsigned int toSendSize, unsigned int toSendCurrent, bool copy, unsigned int seqNr = 0 );

		bool									SendBufferOrEnqueue ( ThreadInstance * client, void * msg, unsigned int size, bool copy = true, unsigned int seqNr = 0 );
		int										SendBufferOrEnqueue ( ThreadInstance * client, const sp ( SendLoad ) &dataSP, unsigned int size );
        int										SendBuffer ( ThreadInstance * client, SendContext * ctx );
#endif
        pthread_t								notifyThreadID;
		pthread_cond_t							notifyEvent;
		pthread_mutex_t							notifyLock;
		pthread_mutex_t							notifyTargetsLock;

#ifdef __cplusplus
		void									NotifyClients ( unsigned int notify, const sp ( DeviceInstanceNode ) &device );
#endif
		static void 						*	NotifyClientsStarter ( void * daemon );
		void									NotifyClientsThread ();
		void									NotifyClients ( NotifyQueueContext * ctx );

	public:
		pthread_cond_t							hWatchdogEvent;

	private:
		INTEROPTIMEVAL							checkLast;
		static void 						*	WatchdogThreadStarter ( void * arg );
		void									WatchdogThread ( MediatorThreadInstance * listeners );

		void                                    CheckProjectValues ();

	};

}

#endif	// ENVIRONS_MEDIATOR_SERVER_H


