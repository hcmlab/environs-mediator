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
#ifndef ENVIRONS_MEDIATOR_H
#define ENVIRONS_MEDIATOR_H

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

#define ENABLE_LIST_CLEANER	

#define CONFFILE						"mediator.conf"
#define DATAFILE						"mediator.data"
#define DEVMAPFILE						"mediator.devmap"
#define LOGFILE							"mediator.log"
#define USERSDBFILE						"users.db"


// Default configuration

typedef struct _ValuePack
{
	string			value;
	unsigned int	size;
	int				timestamp;
} ValuePack;


typedef struct _areaApps
{
	unsigned int								id;
	std::map<std::string, ApplicationDevices*>	apps;

    map<long long, ThreadInstance *>            notifyTargets;
}
AreaApps;


typedef struct _AppsList
{
	map<string, map<string, ValuePack*>*> apps;
}
AppsList;


typedef struct _DeviceMapping
{
	unsigned int	deviceID;
    int             authLevel;
	char			authToken [ MAX_NAMEPROPERTY + 1 ];
}
DeviceMapping;


typedef struct _UserItem
{
    int             authLevel;
    string			pass;
}
UserItem;


class MediatorDaemon : public environs::Mediator
{
public:
	MediatorDaemon ( );
	~MediatorDaemon ( );
	
	bool									InitMediator ( );
	void									PrintSmallHelp ( );
	void									PrintHelp ( );

	bool									OpenLog ( );
	bool									CloseLog ( );

	void									InitDefaultConfig ( );
	bool									LoadConfig ( );
	bool									LoadProjectValues ( );
	bool									LoadUserDBEnc ( );
	bool									LoadUserDB ( );
    bool                                    LoadUserDB ( istream& instream );
	bool									LoadKeys ( );
	void									ReleaseKeys ( );
	bool									SaveConfig ( );
	bool									SaveProjectValues ( );
	bool									SaveUserDB ( );
	bool									AddUser ( int authLevel, const char * userName, const char * pass );
	
	bool									LoadDeviceMappingsEnc ();
	bool									LoadDeviceMappings ();
    bool                                    LoadDeviceMappings ( istream& instream );
	bool									SaveDeviceMappings ();

	bool									CreateThreads ( );
	void									Run ( );

	void									Dispose ( );
	bool									ReleaseThreads ( );
	
	void									ReleaseDevices ( );
	void									ReleaseDeviceMappings ( );
	void									ReleaseClient ( ThreadInstance * client );

	void									BuildBroadcastMessage ( );
	
private:
	bool									allocated;
	pthread_mutex_t							thread_mutex;
	pthread_cond_t							thread_condition;

	vector<unsigned short>					ports;
	vector<MediatorThreadInstance *>		listeners;

	pthread_mutex_t							acceptClientsMutex;
	vector<ThreadInstance *>				acceptClients;

	ApplicationDevices *					GetApplicationDevices ( const char * areaName, const char * appName );
	void									UnlockApplicationDevices ( ApplicationDevices * appDevices );
	DeviceInstanceList *					GetDeviceInstance ( int deviceID, DeviceInstanceList * devices );

	ThreadInstance						*	GetSessionClient ( long long sessionID );

    map<long long, ThreadInstance *>        notifyTargets;
    void                                    UpdateNotifyTargets ( ThreadInstance * client, int filterMode );

	unsigned int							CollectDevicesCount ( DeviceInstanceList * sourceDevice, int filterMode );

	pthread_mutex_t							areasMutex;
	map<string, AppsList * >				areas;

	bool									reqAuth;
	pthread_mutex_t							usersDBMutex;
	map<string, UserItem *>                 usersDB;

	map<string, DeviceMapping *>			deviceMappings;
    
    bool									anonymousLogon;
    char                                    anonymousUser [ MAX_NAMEPROPERTY + 1 ];
    char                                    anonymousPassword [ ENVIRONS_USER_PASSWORD_LENGTH + 1 ];
    
	char									inputBuffer [MEDIATOR_CLIENT_MAX_BUFFER_SIZE];
	char	*								input;
		
	unsigned int							sessionCounter;
	map<long long, ThreadInstance *>        sessions;

	unsigned int							spareID;
	map<unsigned int, ThreadInstance *>		spareClients;

	char								*	privKey;
	unsigned int							privKeySize;
    unsigned int                            encPadding;
    AESContext                              aesCtx;
    char                                    aesKey [ 32 ];
	
	DeviceInstanceList **					GetDeviceList ( char * areaName, char * appName, pthread_mutex_t ** mutex,
													int ** pDevicesAvailable, ApplicationDevices ** appDevices );
	
	unsigned int							areasCounter;
	map<unsigned int, string>				areaIDs;

	unsigned int							appsCounter;
	map<unsigned int, string>				appIDs;

	std::map<std::string, AreaApps * >		areasList;
	
	void									RemoveDevice ( unsigned int ip, char * msg );
	void									RemoveDevice ( DeviceInstanceList * device, bool useLock = true );
	void									RemoveDevice ( int deviceID, const char * areaName, const char * appName );
	void									UpdateDeviceInstance ( DeviceInstanceList * device, bool added, bool changed );

    unsigned int                            bannAfterTries;
	pthread_mutex_t							bannedIPsMutex;
    std::map<unsigned int, std::time_t>		bannedIPs;
    std::map<unsigned int, unsigned int>	bannedIPConnects;

    bool									IsIpBanned ( unsigned int ip );
    void									BannIP ( unsigned int ip );
	void									BannIPRemove ( unsigned int ip );

	int										ScanForParameters ( char * buffer, unsigned int maxLen, const char * delim, char ** params, int maxParams );

	bool									addToArea ( map<string, ValuePack*> * values, const char * key, const char * value, unsigned int valueSize );
	bool									addToArea ( const char * project, const char * app, const char * key, const char * value );
	bool									sendDatabase ( int sock, struct sockaddr * addr );
	
	int										SendBuffer ( ThreadInstance * client, void * msg, unsigned int msgLen, bool useLock = true );
	bool									SendPushNotification ( map<string, ValuePack*> * values, int clientID, const char * value );
	bool									HTTPPostRequest ( string domain, string path, string key, string jsonData );

	void *									BroadcastThread ( );

	static void *							AcceptorStarter ( void *arg );
	void *									Acceptor ( void *arg );

	static void *							MediatorUdpThreadStarter ( void *arg );
	void *									MediatorUdpThread ( void *arg );

	static void *							ClientThreadStarter ( void *arg );
	void *									ClientThread ( void *arg );

	bool									HandleRequest ( char * buffer, ThreadInstance * client );
	bool									UpdateDeviceRegistry ( DeviceInstanceList * device, unsigned int ip, char * msg );

	int										HandleRegistration ( int &deviceID, ThreadInstance * client, unsigned int bytesLeft, char * msg, unsigned int msgLen );

	bool									HandleDeviceRegistration ( ThreadInstance * client, unsigned int ip, char * msg );
	bool									SecureChannelAuth ( ThreadInstance * client );
//	bool									SecureChannel ( ThreadInstance * client );
	void									HandleSpareSocketRegistration ( ThreadInstance * spareClient, ThreadInstance * orgClient, char * msg, unsigned int msgLen );
	//void									HandleSpareSocketRegistration ( ThreadInstance * spareClient, unsigned int deviceID );
	bool									HandleSTUNTRequest ( ThreadInstance * client, STUNTReqPacket * msg );
	bool									NotifySTUNTRegRequest ( ThreadInstance * client );

	void									HandleCLSGenHelp ( ThreadInstance * client );
	void									HandleCertSign ( ThreadInstance * client, char * msg );
	
    bool									HandleSTUNRequest ( ThreadInstance * client, char * msg );
	bool									HandleSTUNRequest ( ThreadInstance * destClient, int sourceID, const char * areaName, const char * appName, unsigned int IP, unsigned int Port );

	bool									HandleQueryDevices ( ThreadInstance * client, char * msg );
	bool									HandleShortMessage ( ThreadInstance * client, char * msg );

	unsigned int							notify;
	unsigned int							notifyDeviceID;
	char								*	notifyAreaName;
	char								*	notifyAppName;

	void									NotifyClientsStart ( unsigned int notify, const char * areaName, const char * appName, int deviceID );
	static void 						*	NotifyClientsStarter ( void * daemon );
	void									NotifyClients ( unsigned int notify );
	
	pthread_cond_t							hWatchdogEvent;

	long									checkLast;
	static void 						*	WatchdogThreadStarter ( void * daemon );
	void									WatchdogThread ();

};


#endif	// ENVIRONS_MEDIATOR_H


