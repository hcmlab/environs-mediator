/**
 * Mediator base functionality
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
//#   define DEBUGVERB
//#   define DEBUGVERBVerb
//#   define DEBUGVERBList
#endif

#include "Mediator.h"
#include "Environs.Native.h"
#include "Environs.Utils.h"

#if !defined(MEDIATORDAEMON)
#   include "Environs.Obj.h"
#else
#   include <cstdio>
#endif

#include <errno.h>
#include <fcntl.h>

#ifndef _WIN32
#   include <signal.h>
#   include <unistd.h>
#else
#	include <iphlpapi.h>

#	pragma comment(lib, "iphlpapi.lib")
#endif

#include <map>
#include <string>

#define	CLASS_NAME 	"Mediator . . . . . . . ."


/* Namespace: environs -> */
namespace environs
{

	bool                Mediator::allocatedClass = false;
	NetPack             Mediator::localNets;
    int                 Mediator::localNetsSize = 0;
	pthread_mutex_t     Mediator::localNetsLock;


#ifdef USE_INTEGER_PROJECT_APP_MAPS
	std::map<int, std::string>      projectIDToNames;
	std::map<int, std::string>      appIDToNames;

	std::map<std::string, int>      nameToProjectID;
	std::map<std::string, int>      nameToAppID;
#endif


	ILock::ILock ()
	{
		init =  MutexInit ( &lock );
	}

	bool ILock::Init ()
	{
		if ( !init )
			init =  MutexInit ( &lock );
		return init;
	}

	bool ILock::Lock ( const char * func )
	{
		return MutexLockA ( lock, func );
	}

	bool ILock::Unlock ( const char * func )
	{
		return MutexUnlockA ( lock, func );
	}

	ILock::~ILock ()
	{
		if ( init )
			MutexDispose ( &lock );
    }
    
    
    void LimitSpareSocketsCount ( ThreadInstance * client )
    {
        int size = client->spareSocketsCount;
        
        if ( size >= MAX_SPARE_SOCKETS_IN_QUEUE_CHECK )
        {
            int count = size - MAX_SPARE_SOCKETS_IN_QUEUE;
            
            for ( int i = 0; i < count; ++i )
            {
                int sock = client->spareSockets [i];
                if ( sock != -1 )
                {
                    DisableLinger ( sock );
                    shutdown ( sock, 2 );
                    closesocket ( sock );
                }
            }

            for ( int i = 0; i < MAX_SPARE_SOCKETS_IN_QUEUE; ++i, ++count )
            {
				client->spareSockets [i] = client->spareSockets [count];
            }
			
            client->spareSocketsCount = MAX_SPARE_SOCKETS_IN_QUEUE;
        }
    }

	void DisableLinger ( int sock )
	{
		linger ling;
		ling.l_onoff = 0;
		ling.l_linger = 0;

		if ( setsockopt ( sock, SOL_SOCKET, SO_LINGER, ( char * ) &ling, sizeof ( ling ) ) < 0 ) {
			CVerb ( "DisableLinger: Failed to set SO_LINGER on socket" ); LogSocketError ();
		}
	}


	ApplicationDevices::ApplicationDevices ()
	{
		count				= 0;
		latestAssignedID	= 0;
		devices				= 0;

		devicesCache		= 0;
		deviceCacheCount	= 0;
		deviceCacheDirty	= false;

		access = 1;
	}


	ApplicationDevices::~ApplicationDevices ()
	{
		if ( devicesCache ) {
			free ( devicesCache );
			devicesCache = 0;
		}
	}


	Mediator::Mediator ()
	{
		CVerb ( "Construct" );

		Zero ( mediator );
		mediator.connection.instance.socket = -1;
		mediator.connection.instance.spareSocket = -1;

		allocated               = false;

		isRunning 				= false;
		broadcastSocketID 		= -1;

		Zero ( broadcastThreadID );

		broadcastMessageLen		= 0;
		lastGreetUpdate			= 0;

		certificate				= 0;
	}


	Mediator::~Mediator ()
	{
		CVerb ( "Destructor" );

		Dispose ();

		if ( allocated )
		{
			MutexLockVA ( mediatorLock, "Destructor" );

            MutexDisposeA ( devicesLock );

#ifdef USE_INTEGER_PROJECT_APP_MAPS
			MutexDisposeA ( idMapMutex );
#endif
			projectIDToNames.clear ();
			appIDToNames.clear ();
			nameToProjectID.clear ();
			nameToAppID.clear ();

			MutexUnlockVA ( mediatorLock, "Destructor" );

			MutexDisposeA ( mediatorLock );
		}

#if ( !defined(MEDIATORDAEMON) )
		envSP = 0;
#endif

		CVerb ( "Destructor: Done." );
	}


	bool Mediator::Init ()
	{
		CVerb ( "Init" );

		if ( !InitClass () )
			return false;

		if ( !allocated )
		{
			if ( !MutexInitA ( mediatorLock ) )
				return false;

			if ( !MutexInitA ( devicesLock ) )
                return false;

#ifdef USE_INTEGER_PROJECT_APP_MAPS
			Zero ( idMapMutex );

			projectIDToNames.clear ();
			appIDToNames.clear ();
			nameToProjectID.clear ();
			nameToAppID.clear ();

			if ( !MutexInitA ( idMapMutex ) )
				return false;
#endif
			allocated = true;
		}

		if ( !LoadNetworks () ) {
			CErr ( "Init: Failed to load local ip addresses!" );
			return false;
		}

		lastGreetUpdate			= 0;

		return true;
	}


	bool Mediator::InitClass ()
	{
		CVerb ( "InitClass" );

		if ( allocatedClass )
			return true;

		Zero ( localNets );

		if ( !MutexInitA ( localNetsLock ) )
            return false;

		allocatedClass = true;

		return true;
	}


	void Mediator::DisposeClass ()
	{
		CVerb ( "DisposeClass" );

		ReleaseNetworks ();

		if ( !allocatedClass )
            return;

		MutexDisposeA ( localNetsLock );
	}


	void Mediator::BroadcastByeBye ()
	{
		strlcpy ( broadcastMessage + 4, MEDIATOR_BROADCAST_BYEBYE, sizeof ( MEDIATOR_BROADCAST_BYEBYE ) );
		SendBroadcast ();
	}


	void Mediator::Dispose ()
	{
		CVerb ( "Dispose" );

		isRunning = false;

		// Wait for each thread to terminate
		ReleaseThreads ();

		ReleaseMediators ();

		if ( certificate ) {
			free ( certificate );
			certificate = 0;
		}
	}

    
    bool Mediator::IsSocketAlive ( SOCKETSYNC &sock )
	{
		CVerb ( "IsSocketAlive" );

		if ( sock == -1 ) {
			CVerb ( "IsSocketAlive: socket argument is invalid" );
			return false;
		}

		int value;
		socklen_t size = sizeof ( socklen_t );

		int ret = getsockopt ( (int) sock, SOL_SOCKET, SO_REUSEADDR, ( char * ) &value, &size );
		if ( ret < 0 ) {
			CInfo ( "IsSocketAlive: disposing invalid socket!" );
			//LogSocketError ();
#ifdef MEDIATORDAEMON
            try {
                DisableLinger ( (int) sock );
				shutdown ( (int) sock, 2 );
				closesocket ( (int) sock );
			}
			catch ( ... ) {
			}
#else
            DisableLinger ( sock );
			shutdown ( sock, 2 );
			closesocket ( sock );
#endif
			sock = -1;
			return false;
		}

		CVerb ( "IsSocketAlive: socket is alive" );
		return true;
	}


	void Mediator::VerifySockets ( ThreadInstance * inst, bool waitThread )
	{
		CVerb ( "VerifySockets" );
		 
		if ( !IsSocketAlive ( inst->socket ) )
        {
			if ( !waitThread )
				return;

			// Wait for the listening thread to be terminated
			pthread_t thrd = inst->threadID;

			if ( pthread_valid ( thrd ) ) {
				CLog ( "VerifySockets: waiting for mediator listener thread to be termianted." );

				/// EXC_BAD_ACCESS
				int s = pthread_join ( thrd, NULL );
				if ( pthread_wait_fail ( s ) ) {
					CErrArg ( "VerifySockets: Error waiting for mediator listener thread (pthread_join:%i)", s );
				}
				pthread_close ( thrd );
				pthread_reset ( inst->threadID );
			}
		}
	}


	bool Mediator::ReleaseThreads ()
	{
		CVerb ( "ReleaseThreads" );

		int value;
		bool ret = true;

		// Signal stop status to threads
		isRunning = false;

		// Close broadcast listener socket
		value = broadcastSocketID;
		if ( value != -1 ) {
			CVerb ( "ReleaseThreads: Closing broadcast socket." );

            broadcastSocketID = -1;
            DisableLinger ( value );
			shutdown ( value, 2 );
			closesocket ( value );
		}

		// Waiting for broadcast thread
		pthread_t thrd = broadcastThreadID;

		if ( pthread_valid ( thrd ) ) {
			pthread_reset ( broadcastThreadID );

			CVerb ( "ReleaseThreads: Waiting for broadcast thread to be termianted." );

			value = pthread_join ( thrd, NULL );
			if ( pthread_wait_fail ( value ) ) {
				CErrArg ( "ReleaseThreads: Error waiting for broadcast thread (pthread_join:%i)\n", value );
				ret = false;
			}

			pthread_close ( thrd );
		}

		return ret;
	}


	void Mediator::ReleaseNetworks ()
	{
		CVerb ( "ReleaseNetworks" );

        localNetsSize = 0;

        if ( !MutexLockA ( localNetsLock, "ReleaseNetworks" ) )
			return;

		NetPack * net = localNets.next;

		while ( net ) {
			NetPack * toDelete = net;
			net = net->next;
			free ( toDelete );
		}

		Zero ( localNets );

		MutexUnlockA ( localNetsLock, "ReleaseNetworks" );
	}


	void Mediator::ReleaseMediator ( MediatorInstance * med )
	{
		if ( !med ) return;

		ThreadInstance * inst = &med->connection.instance;

		int s = (int) inst->socket;
		if ( s != -1 ) {
			inst->socket = -1;

            CVerb ( "ReleaseMediator: closing cocket" );
            DisableLinger ( s );
			shutdown ( s, 2 );
			closesocket ( s );
		}

		// Wait for the listening thread to be terminated
		pthread_t thrd = inst->threadID;
		if ( pthread_valid ( thrd ) ) {
			pthread_reset ( inst->threadID );

			CVerb ( "ReleaseMediator: waiting for mediator listener thread to be termianted." );

			/// EXC_BAD_ACCESS
			s = pthread_join ( thrd, NULL );
			if ( pthread_wait_fail ( s ) ) {
				CErrArg ( "ReleaseMediator: Error waiting for mediator listener thread (pthread_join:%i)", s );
			}
            
			pthread_close ( thrd );
		}
                
        for ( int i = 0; i < inst->spareSocketsCount; ++i )
        {
            int sock = inst->spareSockets [i];
            if ( sock != -1 )
            {
                DisableLinger ( sock );
                shutdown ( sock, 2 );
                closesocket ( sock );
            }
        }
		inst->spareSocketsCount = 0;
		
		AESDisposeKeyContext ( &med->connection.instance.aes );
		med->connection.instance.encrypt = 0;

		MutexDispose ( &med->connection.rec_mutex );
		MutexDispose ( &med->connection.send_mutex );

		CondDispose ( &med->connection.rec_signal );

		if ( med->connection.buffer ) {
			free ( med->connection.buffer );
			med->connection.buffer = 0;
		}

		if ( med != &mediator )
			free ( med );
	}


	void Mediator::ReleaseMediators ()
	{
		CVerb ( "ReleaseMediators" );

		MutexLockV ( &mediatorLock, "ReleaseMediators" );

		MediatorInstance * inst = mediator.next;

		while ( inst ) {
			MediatorInstance * toDelete = inst;
			inst = inst->next;

			ReleaseMediator ( toDelete );
		}

		if ( mediator.port ) {
			ReleaseMediator ( &mediator );
			Zero ( mediator );
		}

		MutexUnlockV ( &mediatorLock, "ReleaseMediators" );
	}


#define USE_WIN_GETADAPTERSINFO


	bool Mediator::LoadNetworks ()
	{
		CVerb ( "LoadNetworks" );

		unsigned int	ip;
		unsigned int	bcast;

		MutexLockVA ( localNetsLock, "LoadNetworks" );

#ifdef _WIN32
		bool adaptersFound = 0;

		IP_ADAPTER_INFO tmpAdapter;
		Zero ( tmpAdapter );

		struct sockaddr_in addr;

		PIP_ADAPTER_INFO adapters = &tmpAdapter;
		PIP_ADAPTER_INFO adapter = nill;
		DWORD success = 0;
		int i = 0;

		ULONG bufSize = sizeof ( tmpAdapter );

		if ( GetAdaptersInfo ( adapters, &bufSize ) != ERROR_BUFFER_OVERFLOW ) 
		{
			success = NO_ERROR;
		}
		else {
			adapters = ( PIP_ADAPTER_INFO ) malloc ( bufSize );

			if ( adapters == nill ) {
				CErr ( "LoadNetworks: Failed to allocate memory." );
				goto EndOfMethod;
			}

			success = GetAdaptersInfo ( adapters, &bufSize );
		}

		if ( success == NO_ERROR ) 
		{
			adapter = adapters;
			while ( adapter )
			{
				i++;

				inet_pton ( AF_INET, adapter->IpAddressList.IpAddress.String, &( addr.sin_addr ) );
				ip = ( unsigned int ) addr.sin_addr.s_addr;

				inet_pton ( AF_INET, adapter->IpAddressList.IpMask.String, &( addr.sin_addr ) );
				unsigned int mask = ( unsigned int ) addr.sin_addr.s_addr;

				if ( ip && mask ) {
					CLogArg ( "Local IP %i: [ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
					CVerbArg ( "Local SN %i: [ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &mask ) ) );

					bcast = GetBroadcast ( ip, mask );
					if ( !bcast ) {
						CErrArg ( "LoadNetworks: Failed to calculate broadcast address for interface [ %i ]!", i );
						continue;
					}

					inet_pton ( AF_INET, adapter->GatewayList.IpAddress.String, &( addr.sin_addr ) );
					unsigned int gw = ( unsigned int ) addr.sin_addr.s_addr;
					if ( gw ) {
						CLogArg ( "Local GW %i: [ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &gw ) ) );
					}

					CVerbArg ( "LoadNetworks: Local BC %i: [ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &bcast ) ) );

					AddNetwork ( ip, bcast, mask, gw );
					adaptersFound++;
				}

				adapter = adapter->Next;
			}
		}

		if ( adapters != &tmpAdapter )
			free ( adapters );

		if ( !adaptersFound )
		{
			char ac [ 256 ];
			if ( gethostname ( ac, sizeof ( ac ) ) == SOCKET_ERROR ) {
				CErrArg ( "LoadNetworks: Error %i when getting local host name.", WSAGetLastError () );
				goto EndOfMethod;
			}
			CLogArg ( "LoadNetworks: Host name [%s]", ac );

			SOCKET sock = WSASocket ( PF_INET, SOCK_DGRAM, 0, 0, 0, 0 );
			if ( sock == SOCKET_ERROR ) {
				CErrArg ( "LoadNetworks: Failed to get a socket. Error %i", WSAGetLastError () );
				goto EndOfMethod;
			}
			DisableSIGPIPE ( sock );

			INTERFACE_INFO InterfaceList [ 20 ];
			unsigned long nBytesReturned;

			if ( WSAIoctl ( sock, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
				sizeof ( InterfaceList ), &nBytesReturned, 0, 0 ) == SOCKET_ERROR )
			{
                CErrArg ( "LoadNetworks: Failed calling WSAIoctl: error %i", WSAGetLastError () );
				shutdown ( sock, 2 );
				closesocket ( sock );
				goto EndOfMethod;
			}

			int nNumInterfaces = nBytesReturned / sizeof ( INTERFACE_INFO );
			CVerbArg ( "LoadNetworks: There are [%i] interfaces:", nNumInterfaces );

			// Skip loopback interfaces
			for ( i = 0; i < nNumInterfaces; ++i ) {
				u_long nFlags = InterfaceList [ i ].iiFlags;
				if ( nFlags & IFF_LOOPBACK ) {
					CVerb ( "LoadNetworks: Omiting loopback interface." );
					continue;
				}

				sockaddr_in * pAddress;
				//int max_length = 20;

				// Retrieve ip address
				pAddress = ( sockaddr_in * ) & ( InterfaceList [ i ].iiAddress );

				ip = pAddress->sin_addr.s_addr;

				// Retrieve netmask	
				pAddress = ( sockaddr_in * ) & ( InterfaceList [ i ].iiNetmask );
				unsigned int mask = pAddress->sin_addr.s_addr;

				CLogArg ( "Local IP %i: [ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
				CVerbArg ( "Local SN %i: [ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &mask ) ) );

				bcast = GetBroadcast ( ip, mask );
				if ( !bcast ) {
					CErrArg ( "LoadNetworks: Failed to calculate broadcast address for interface [%i]!", i );
					continue;
				}

				CVerbArg ( "LoadNetworks: Local BC %i: [ %s ]", i, inet_ntoa ( *( ( struct in_addr * ) &bcast ) ) );

				AddNetwork ( ip, bcast, mask, 0 );
            }

			shutdown ( sock, 2 );
			closesocket ( sock );
		}
#else

#ifdef ANDROID
		int sock, rval;
		struct ifreq ifreqs [ 20 ];
		struct ifreq ifr_mask;
		struct ifconf ifconf;
		int  nifaces, i;

		Zero ( ifconf );
		ifconf.ifc_buf = ( char * ) ifreqs;
		ifconf.ifc_len = sizeof ( ifreqs );

		sock = socket ( PF_INET, SOCK_STREAM, 0 );
		if ( sock < 0 ) {
			CErr ( "LoadNetworks: Failed to create socket for ioctl!" );
			goto EndOfMethod;
		}
		DisableSIGPIPE ( sock );

		rval = ioctl ( sock, SIOCGIFCONF, ( char* ) &ifconf );
		if ( rval < 0 ) {
            CErr ( "LoadNetworks: ioctl SIOCGIFCONF failed!" );
            DisableLinger ( sock );
			shutdown ( sock, 2 );
			closesocket ( sock );
			goto EndOfMethod;
		}

		nifaces =  ifconf.ifc_len / sizeof ( struct ifreq );

		CVerbArg ( "LoadNetworks: Interfaces (count = %d)", nifaces );

		for ( i = 0; i < nifaces; i++ )
		{
			ip = ( ( struct sockaddr_in * )&ifreqs [ i ].ifr_ifru.ifru_addr )->sin_addr.s_addr;

			CLogArg ( "LoadNetworks: Interface name: '%s' - ip: [ %s ]", ifreqs [ i ].ifr_name, inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );

			if ( strlen ( ifreqs [ i ].ifr_name ) < 1 ) {
				CVerb ( "LoadNetworks: Omiting invalid interface name." );
				continue;
			}

			if ( !ip ) {
				CVerb ( "LoadNetworks: Omiting invalid interface." );
				continue;
			}

			// Skip loopback
			if ( ip == 0x0100007F ) {
				CVerb ( "LoadNetworks: Omiting loopback interface." );
				continue;
			}

			// Retrieve netmask	
			ifr_mask.ifr_addr.sa_family = PF_INET;

			strlcpy ( ifr_mask.ifr_name, ifreqs [ i ].ifr_name, IFNAMSIZ - 1 );

			rval = ioctl ( sock, SIOCGIFNETMASK, &ifr_mask );
			if ( rval < 0 ) {
				CErr ( "LoadNetworks: ioctl SIOCGIFNETMASK failed!" );
				continue;
			}

			unsigned int mask = ( ( struct sockaddr_in * )&ifr_mask.ifr_addr )->sin_addr.s_addr;

			bcast = GetBroadcast ( ip, mask );
			if ( !bcast ) {
				CErr ( "LoadNetworks: Failed to calculate broadcast address!" );
				continue;
			}
			CVerbArg ( "LoadNetworks: Netmask:   [ %s ]'", inet_ntoa ( *( ( struct in_addr * ) &mask ) ) );
			CVerbArg ( "LoadNetworks: Broadcast: [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &bcast ) ) );

			AddNetwork ( ip, bcast, mask, 0 );
        }
        DisableLinger ( sock );
		shutdown ( sock, 2 );
		closesocket ( sock );

#else // LINUX or MAC or IOS

		struct ifaddrs  * ifList  = NULL;
		struct ifaddrs  * ifa           = NULL;
		void            * tmp    = NULL;

		if ( getifaddrs ( &ifList ) != 0 ) {
			CErr ( "LoadNetworks: getifaddrs failed!" );
			goto EndOfMethod;
		}

		for ( ifa = ifList; ifa != NULL; ifa = ifa->ifa_next )
		{
			if ( ifa->ifa_addr->sa_family == PF_INET ) {
				// IPv4
				ip = ( ( struct sockaddr_in * ) ifa->ifa_addr )->sin_addr.s_addr;

				CLogArg ( "LoadNetworks: [%s] => IP: [ %s ]", ifa->ifa_name, inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );

				if ( !ip ) {
					CVerb ( "LoadNetworks: Omiting invalid interface." );
					continue;
				}

				// Skip loopback
				if ( ip == 0x0100007F ) {
					CVerb ( "LoadNetworks: Omiting loopback interface." );
					continue;
				}

#ifdef XCODE
				// Skip any connection but ethernet en...
				if ( !strstr ( ifa->ifa_name, "en" ) ) {
					CVerbArg ( "LoadNetworks: Not an ethernet interface. Ommiting [%s]", ifa->ifa_name ? ifa->ifa_name : "---" );
					continue;
				}
#endif
				unsigned int mask = ( ( struct sockaddr_in * )ifa->ifa_netmask )->sin_addr.s_addr;

				bcast = GetBroadcast ( ip, mask );
				if ( !bcast ) {
					CErr ( "LoadNetworks: ERROR - Failed to calculate broadcast address!" );
					continue;
				}
				CVerbArg ( "LoadNetworks: Netmask:   [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &mask ) ) );
				CVerbArg ( "LoadNetworks: Broadcast: [ %s ]", inet_ntoa ( *( ( struct in_addr * ) &bcast ) ) );

				AddNetwork ( ip, bcast, mask, 0 );
			}
			else if ( ifa->ifa_addr->sa_family == PF_INET6 ) {
				// We do not support IPv6 yet
				tmp=&( ( struct sockaddr_in6 * )ifa->ifa_addr )->sin6_addr;
				char addressBuffer [ INET6_ADDRSTRLEN ];
				inet_ntop ( PF_INET6, tmp, addressBuffer, INET6_ADDRSTRLEN );
				CVerbArg ( "LoadNetworks: IPv6 not supported yet: [%s] IP Address [ %s ]", ifa->ifa_name, addressBuffer );
			}
		}

		if ( ifList )
			freeifaddrs ( ifList );
#endif

#endif

	EndOfMethod:
		MutexUnlockVA ( localNetsLock, "LoadNetworks" );

		return true;
	}


	unsigned int Mediator::GetBroadcast ( unsigned int ip, unsigned int netmask )
	{
		u_long host_ip = ip;
		u_long net_mask = netmask;
		u_long net_addr = host_ip & net_mask;
		u_long bcast_addr = net_addr | ( ~net_mask );

		return ( unsigned int ) bcast_addr;
	}


	void Mediator::AddNetwork ( unsigned int ip, unsigned int bcast, unsigned int netmask, unsigned int gw )
	{
		NetPack * net = &localNets;
		NetPack * pack = net;

		while ( net ) {
			if ( net->ip == ip )
				return;

			pack = net;
			net = net->next;
		}

		if ( localNets.ip ) {
			// This NetPack has already been completed, create and attach a new one
			NetPack * newPack = ( NetPack * ) calloc ( 1, sizeof ( NetPack ) );
			if ( !newPack ) {
				CErr ( "AddNetwork: Failed to allocate memory!" );
				return;
			}
			pack->next = newPack;
			pack = newPack;
		}

		if ( gw && pack != &localNets ) 
		{
			// Swap current network with the first one
			memcpy ( pack, &localNets, sizeof ( NetPack ) );						
			
			pack->next = 0;
			pack = &localNets;
		}

		pack->ip	= ip;
		pack->bcast = bcast;
		pack->mask	= netmask;
		pack->gw	= gw;
        
        localNetsSize++;
	}


	bool Mediator::IsStarted ()
	{
		return isRunning;
	}


	bool Mediator::Start ()
	{
		CVerb ( "Start" );

		isRunning = true;

		// Create socket for broadcast thread
		if ( broadcastSocketID == -1 ) {
			int sock = ( int ) socket ( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
			if ( sock == -1 ) {
				CErr ( "Start: Failed to create broadcast socket!" );
				return false;
			}
			DisableSIGPIPE ( sock );

			broadcastSocketID = sock;

			int value = 1;
			if ( setsockopt ( sock, SOL_SOCKET, SO_BROADCAST, ( const char * ) &value, sizeof ( value ) ) == -1 ) {
				CErr ( "Start: Failed to set broadcast option on socket!" );
				return false;
			}

			value = 1;
			if ( setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) ) != 0 ) {
				CErr ( "Start: Failed to set reuseaddr option on socket!" );
				return false;
			}

#ifdef SO_REUSEPORT
			value = 1;
			if ( setsockopt ( sock, SOL_SOCKET, SO_REUSEPORT, ( const char * ) &value, sizeof ( value ) ) != 0 ) {
				CErr ( "Start: Failed to set reuseaddr option on socket!" );
				return false;
			}
#endif
			/// Create broadcast thread
			int s = pthread_create ( &broadcastThreadID, 0, BroadcastThreadStarter, ( void * )this );
			if ( s ) {
				CErrArg ( "Start: Creating thread for broadcast failed. (pthread_create [%i])", s );
				return false;
			}

			Sleep ( 300 );
		}

		OnStarted ();

		SendBroadcast ();

		CVerb ( "Start: success" );
		return true;
	}


	void Mediator::OnStarted ()
	{
	}


	bool Mediator::SendBroadcast ()
	{
		CVerb ( "SendBroadcast" );

		if ( broadcastSocketID == -1 )
			return false;

		bool ret = true;
		size_t sentBytes = 0;

		CVerbArg ( "SendBroadcast: Broadcasting message [%s] (%d)...", broadcastMessage + 4, broadcastMessageLen );

		struct 	sockaddr_in		broadcastAddr;
		Zero ( broadcastAddr );

		broadcastAddr.sin_family = PF_INET;
		broadcastAddr.sin_port = htons ( DEFAULT_BROADCAST_PORT );

		broadcastAddr.sin_addr.s_addr = inet_addr ( "255.255.255.255" );

		sentBytes = sendto ( broadcastSocketID, broadcastMessage + 4, broadcastMessageLen, 0, ( struct sockaddr * ) &broadcastAddr, sizeof ( struct sockaddr ) );
		if ( sentBytes != broadcastMessageLen )
        {
			CErr ( "SendBroadcast: Broadcast failed!" );
			return false;
		}

		/// We need to broadcast to each interface, because (at least with win32): the default 255.255.... broadcasts only on one (most likely the main or internet) network interface
		NetPack * net = 0;

#ifdef NDEBUG
        if ( localNetsSize <= 1 )
#else
        if ( localNetsSize <= 0 )
#endif
            return true;
        
		if ( !MutexLockA ( localNetsLock, "SendBroadcast" ) ) {
			return false;
		}

		net = &localNets;
		while ( net ) {
			broadcastAddr.sin_addr.s_addr = net->bcast;

			sentBytes = sendto ( broadcastSocketID, broadcastMessage + 4, broadcastMessageLen, 0, ( struct sockaddr * ) &broadcastAddr, sizeof ( struct sockaddr ) );
			if ( sentBytes != broadcastMessageLen ) {
				CErr ( "SendBroadcast: Broadcast failed!" );
				ret = false;
			}

			net = net->next;
		}

		MutexUnlockVA ( localNetsLock, "SendBroadcast" );

		return ret;
	}


	bool Mediator::IsLocalIP ( unsigned int ip )
	{
		bool ret = false;

		CVerb ( "IsLocalIP" );

		if ( !MutexLockA ( localNetsLock, "IsLocalIP" ) )
			return false;

		NetPack * net = &localNets;

		while ( net ) {
			if ( ip == net->ip ) {
				ret = true;
				break;
			}
			net = net->next;
		}

		if ( !MutexUnlockA ( localNetsLock, "IsLocalIP" ) )
			return false;
		return ret;
	}


	unsigned int Mediator::GetLocalIP ()
	{
		CVerb ( "GetLocalIP" );

		if ( !localNets.ip )
			if ( !LoadNetworks () )
				return 0;

		return localNets.ip;
	}


	unsigned int Mediator::GetLocalSN ()
	{
		CVerb ( "GetLocalSN" );

		return localNets.mask;
	}


	bool Mediator::IsAnonymousUser ( const char * user )
	{
		if ( !user || !strlen ( user ) )
			return true;

		int maxLen = ( int ) sizeof ( MEDIATOR_ANONYMOUS_USER );

		const char * anonUser = MEDIATOR_ANONYMOUS_USER;

		while ( *user && maxLen >= 0 )
		{
			if ( *user++ != *anonUser++ )
				break;
			maxLen--;
		}
		if ( maxLen <= 1 )
			return true;
		return false;
	}


	void Mediator::VanishedDeviceWatcher ()
	{
		DeviceInstanceNode	**	listRoot	= 0;
		DeviceInstanceNode	*	device		= 0;
		pthread_mutex_t		*	mutex		= 0;

		GetDeviceList ( nill, nill, &mutex, nill, listRoot );

		if ( !listRoot )
			return;

		if ( !MutexLock ( mutex, "VanishedDeviceWatcher" ) )
            return;
        
        device = *listRoot;

		while ( device )
		{
			CVerbVerbArg ( "VanishedDeviceWatcher: Checking [0x%X] Area [%s] App [%s]", device->info.deviceID, device->info.areaName, device->info.appName );

			if ( device->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST && ( lastGreetUpdate - device->info.updates ) > 90000 )
			{
				// Device has vanished
				DeviceInstanceNode	*	vanished = device;

				//device = vanished->next;

				//if ( !vanished->prev ) {
				//	// The root device vanished
				//	if ( device )
				//		device->prev = 0;

				//	*listRoot = device;
				//}
				//else
				//	vanished->prev->next = device;

				//vanished->next = 0;

				// Remove device and relink the list
				RemoveDevice ( vanished, false );

				device = *listRoot;
				continue;
			}

			device = device->next;
		}

		MutexUnlockV ( mutex, "VanishedDeviceWatcher" );
	}


	sp ( DeviceInstanceNode ) Mediator::UpdateDevices ( unsigned int ip, char * msg, char ** uid, bool * created, char broadcastFound )
	{
		CVerbVerb ( "UpdateDevices" );

		if ( !msg )
			return 0;

		int				value;
		//unsigned int	flags		= 0;
		bool			found		= false;
		bool			changed		= false;
		char		*	deviceName	= 0;
		char		*	areaName	= 0;
		char		*	appName		= 0;
		char		*	userName	= 0;

		DeviceInstanceNode * device		= 0;
		DeviceInstanceNode * devicePrev	= 0;
		DeviceInstanceNode ** listRoot	= 0;

		sp ( DeviceInstanceNode ) deviceSP = 0;

		pthread_mutex_t * mutex		= 0;

		int *						pDevicesAvailable	= 0;

        sp ( ApplicationDevices ) appDevices			= 0;

		// Get the id at first (>0)	
		int * pIntBuffer = ( int * ) ( msg + MEDIATOR_BROADCAST_DEVICEID_START );
		value = *pIntBuffer;
		if ( !value )
			return 0;

		// Get the areaName, appname, etc..
		char * context = NULL;
		char * psem = strtok_s ( msg + MEDIATOR_BROADCAST_DESC_START, ";", &context );
		if ( !psem )
			return 0;
		deviceName = psem;

		psem = strtok_s ( NULL, ";", &context );
		if ( !psem )
			return 0;
		areaName = psem;

		psem = strtok_s ( NULL, ";", &context );
		if ( !psem )
			return 0;
		appName = psem;

		if ( uid ) {
			psem = strtok_s ( NULL, ";", &context );
			if ( psem )
				*uid = psem;
		}
		/*psem = strtok_s ( NULL, ";", &context );
		if ( psem ) {
		userName = psem;
		}*/

#if !defined(MEDIATORDAEMON)
		if ( value == env->deviceID && !strncmp ( areaName, env->areaName, sizeof ( env->areaName ) )
			&& !strncmp ( appName, env->appName, sizeof ( env->appName ) - 1 ) )
			return 0;

		if ( env->mediatorFilterLevel > MEDIATOR_FILTER_NONE ) {
			if ( strncmp ( areaName, env->areaName, sizeof ( env->areaName ) - 1 ) )
				return 0;

			if ( env->mediatorFilterLevel > MEDIATOR_FILTER_AREA ) {
				if ( strncmp ( appName, env->appName, sizeof ( env->appName ) - 1 ) )
					return 0;
			}
		}
#endif

		appDevices = GetDeviceList ( areaName, appName, &mutex, &pDevicesAvailable, listRoot );
		if ( !listRoot )
			return 0;

		if ( !MutexLock ( mutex, "UpdateDevices" ) )
			goto Finish;

		device = *listRoot;

		while ( device )
		{
			CVerbVerbArg ( "UpdateDevices: Comparing [0x%X / 0x%X] Area [%s / %s] App [%s / %s]", device->info.deviceID, value, device->info.areaName, areaName, device->info.appName, appName );

			if ( device->info.deviceID == value
#ifndef MEDIATORDAEMON
				&& !strncmp ( device->info.areaName, areaName, sizeof ( device->info.areaName ) ) && !strncmp ( device->info.appName, appName, sizeof ( device->info.appName ) )
#endif
				) {
				if ( strncmp ( device->info.deviceName, deviceName, sizeof ( device->info.deviceName ) ) )
				{
					// Another device with the same identifiers has already been registered. Let's ignore this.
					MutexUnlockV ( mutex, "UpdateDevices" );
					goto Finish;
				}

				found = true; device->info.unavailable = true;
				break;
			}

			if ( !device->next || device->info.deviceID > value )
				break;

			devicePrev = device;
			device = device->next;
		}

#ifdef MEDIATORDAEMON
		if ( found ) {
			deviceSP = device->baseSP;
			MutexUnlockV ( mutex, "UpdateDevices" );
			return deviceSP;
		}
		if ( created )
			*created = true;
#endif

		while ( !found ) {
			// Create a new list node
			DeviceInstanceNode	*		dev		= 0;
			deviceSP	= sp_make ( DeviceInstanceNode );

			if ( !deviceSP || ( dev = deviceSP.get () ) == 0 ) {
				CErr ( "UpdateDevices: Failed to allocate memory for new device!" );
				device = 0;
				break;
			}
            dev->baseSP = deviceSP;

			dev->info.deviceID = value;
			( *pDevicesAvailable )++;

#ifdef MEDIATORDAEMON
			dev->rootSP = appDevices;
#else
			dev->hEnvirons = env->hEnvirons;
			dev->info.broadcastFound = broadcastFound;

			// Build the key
#ifdef USE_MEDIATOR_OPT_KEY_MAPS_COMP
			*( ( int * ) dev->key ) = value;

			int copied = snprintf ( dev->key + 4, sizeof ( dev->key ) - 4, "%s %s", areaName, appName );
#else
			int copied = snprintf ( dev->key, sizeof ( dev->key ), "%011i %s %s", value, areaName, appName );
#endif
			if ( copied <= 0 ) {
				CErr ( "UpdateDevices: Failed to build the key for new device!" );
			}
#endif
			if ( devicePrev )
			{
				dev->prev = devicePrev;

				if ( devicePrev->next ) {
					// Glue next links together
					dev->next = devicePrev->next;
					devicePrev->next = dev;

					// Glue previous links together
					dev->next->prev = dev;
				}
				else
					devicePrev->next = dev;
			}
			else {
				if ( device ) {
					if ( device->info.deviceID > value ) {
						/// Device must be the listRoot (because there is no previous device)
						dev->next = device;
						device->prev = dev;
						*listRoot = dev;
					}
					else {
						dev->prev = device;

						if ( device->next ) {
							// Glue next links together
							dev->next = device->next;
							device->next = dev;

							// Glue previous links together
							dev->next->prev = dev;
						}
						else
							device->next = dev;
					}
				}
				else
					*listRoot = dev;
			}
			device = dev;

			changed = true; //flags |= DEVICE_INFO_ATTR_AREA_NAME | DEVICE_INFO_ATTR_DEVICE_NAME;
			strlcpy ( device->info.areaName, areaName, sizeof ( device->info.areaName ) );
			strlcpy ( device->info.appName, appName, sizeof ( device->info.appName ) );
			//if ( userName ) {
			//	strlcpy ( device->userName, sizeof(device->userName) - 1, userName );
			//	//flags |= DEVICE_INFO_ATTR_USER_NAME;
			//}
			break;
		}

		if ( device ) {
			device->info.updates = ( unsigned int ) lastGreetUpdate;

#ifndef MEDIATORDAEMON
			if ( !device->info.ipe || device->info.broadcastFound == DEVICEINFO_DEVICE_BROADCAST )
#endif
				if ( ip != device->info.ipe ) {
					device->info.ipe = ip; changed = true; //flags |= DEVICE_INFO_ATTR_IPE;
				}

			int platform = *( ( int * ) ( msg + MEDIATOR_BROADCAST_PLATFORM_START ) );

			if ( device->info.platform != platform ) {
				device->info.platform = platform; changed = true; //flags |= DEVICE_INFO_ATTR_DEVICE_TYPE;
			}

			pIntBuffer += 1;

			// Get the ip
			value = *pIntBuffer;
			if ( device->info.ip != ( unsigned ) value ) {
				device->info.ip = value; changed = true; //flags |= DEVICE_INFO_ATTR_IP;
			}
			// Get ports
			unsigned short * pUShortBuffer = ( unsigned short * ) ( msg + MEDIATOR_BROADCAST_PORTS_START );
			unsigned short svalue = *pUShortBuffer++;
			if ( svalue != device->info.tcpPort ) {
				device->info.tcpPort = svalue; changed = true; //flags |= DEVICE_INFO_ATTR_PORT_TCP;
			}

			svalue = *pUShortBuffer;
			if ( svalue != device->info.udpPort ) {
				device->info.udpPort = svalue; changed = true; //flags |= DEVICE_INFO_ATTR_PORT_UDP;
			}

			if ( strncmp ( device->info.deviceName, deviceName, sizeof ( device->info.deviceName ) - 1 ) ) {
				strlcpy ( device->info.deviceName, deviceName, sizeof ( device->info.deviceName ) ); changed = true; //flags |= DEVICE_INFO_ATTR_DEVICE_NAME;
			}

#ifdef MEDIATORDAEMON
			if ( device->info.broadcastFound != broadcastFound ) {
				device->info.broadcastFound = broadcastFound; changed = true; //flags |= DEVICE_INFO_ATTR_BROADCAST_FOUND;
		}
#endif
			if ( userName && *userName && strncmp ( device->userName, userName, sizeof ( device->userName ) - 1 ) ) {
				strlcpy ( device->userName, userName, sizeof ( device->userName ) ); changed = true; //flags |= DEVICE_INFO_ATTR_USER_NAME;
			}

			if ( changed ) {
				CVerbArg ( "UpdateDevices: Device      = [0x%X / %s / %s]", device->info.deviceID, device->info.deviceName, device->info.broadcastFound ? "on same network" : "by mediator" );
				if ( device->info.ip != device->info.ipe ) {
					CVerbArg ( "UpdateDevices: Device IPe != IP  [%s]", inet_ntoa ( *( ( struct in_addr * ) &device->info.ip ) ) );
					CVerbArg ( "UpdateDevices: Device IP  != IPe [%s]", inet_ntoa ( *( ( struct in_addr * ) &device->info.ipe ) ) );
				}
				else {
					CListLogArg ( "UpdateDevices: Device IPe [%s]", inet_ntoa ( *( ( struct in_addr * ) &device->info.ipe ) ) );
				}

				CListLogArg ( "UpdateDevices: Area/App = [%s / %s]", device->info.areaName, device->info.appName );
				CListLogArg ( "UpdateDevices: Device  IPe = [%s (from socket), tcp [%d], udp [%d]]", inet_ntoa ( *( ( struct in_addr * ) &ip ) ), device->info.tcpPort, device->info.udpPort );
			}
		}

		if ( !MutexUnlock ( mutex, "UpdateDevices" ) ) {
			if ( device ) {
				device->baseSP = 0;
				device = 0;
			}
		}
		else {
            if ( deviceSP )
                UpdateDeviceInstance ( deviceSP, !found, changed );
		}

	Finish:

#ifdef MEDIATORDAEMON
		if ( appDevices )
			__sync_sub_and_fetch ( &appDevices->access, 1 );
#endif

		return deviceSP;
}


	void Mediator::DevicesHasChanged ( int type )
	{
		CVerb ( "DevicesHasChanged" );
	}


	void printDevices ( DeviceInstanceNode * device )
	{
		while ( device )
		{
			CLogArg ( "printDevices: Device id      = 0x%X", device->info.deviceID );
			CLogArg ( "printDevices: Area name      = %s", device->info.areaName );
			CLogArg ( "printDevices: App name       = %s", device->info.appName );
			CLogArg ( "printDevices: Platform       = %i", device->info.platform );
			CLogArg ( "printDevices: Device IPe     = %s (from socket)", inet_ntoa ( *( ( struct in_addr * ) &device->info.ip ) ) );
			CLogArg ( "printDevices: Device IPe != IP (%s)", inet_ntoa ( *( ( struct in_addr * ) &device->info.ipe ) ) );
			CLogArg ( "printDevices: Device tcpPort = %d", device->info.tcpPort );
			CLogArg ( "printDevices: Device udpPort = %d", device->info.udpPort );
			CLogArg ( "printDevices: Device name    = %s", device->info.deviceName );

			device = device->next;
		}
	}


	MediatorInstance * Mediator::IsKnownMediator ( unsigned int ip, unsigned short port )
	{
		CVerb ( "IsKnownMediator" );

		MediatorInstance * ret = 0;

		MediatorInstance * med = &mediator;

		while ( med ) {
			if ( ip == med->ip && med->port == port ) {
				ret = med;
				//med->available = true;

				// Register again
				CVerb ( "IsKnownMediator: already know." );
				/*
				#ifndef MEDIATORDAEMON
				if ( (opt_useDefaultMediator || opt_useCustomMediator) && environs::ID ) {
				#endif
				if ( pthread_mutex_trylock ( &mediatorLock ) ) {
				CVerb ( "IsKnownMediator: Failed to aquire mutex on mediator!" );
				return false;
				}

				RegisterAtMediator ( med );

				if ( pthread_mutex_unlock ( &mediatorLock ) ) {
				CErr ( "IsKnownMediator: Failed to release mutex on mediator!" );
				}
				#ifndef MEDIATORDAEMON
				}
				#endif
				*/
				break;
			}
			med = med->next;
		}
		return ret;
	}


	bool Mediator::RegisterAtMediator ( MediatorInstance * med )
	{
		return true;
	}


	void * Mediator::BroadcastThreadStarter ( void *arg )
	{
		if ( !arg ) {
			CErr ( "BroadcastThreadStarter: Called with invalid (NULL) argument.\n" );
			return 0;
		}

		Mediator * mediator = ( Mediator * ) arg;

		// Execute thread
		return mediator->BroadcastThread ();
	}


	bool Mediator::AddMediator ( unsigned int ip, unsigned short port )
	{
		CVerbArg ( "AddMediator: IP [%s] Port [%d]", inet_ntoa ( *( ( struct in_addr * ) &ip ) ), port );

		if ( !port ) {
			CWarn ( "AddMediator: port [0] is invalid." );
			return false;
		}

		if ( IsKnownMediator ( ip, port ) ) {
			CVerbArg ( "AddMediator: mediator (%s) already available.", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
			return true;
		}

		MediatorInstance * med = ( MediatorInstance * ) calloc ( 1, sizeof ( MediatorInstance ) );
		if ( !med ) {
			CErr ( "AddMediator: Failed to allocate memory!" );
			return false;
		}

#ifndef MEDIATORDAEMON
		med->connection.env = env;
#endif

		if ( !MutexInit ( &med->connection.rec_mutex ) )
			goto Failed;

		Zero ( med->connection.rec_signal );

		if ( pthread_cond_manual_init ( &med->connection.rec_signal, NULL ) ) {
			CErr ( "AddMediator: Failed to init rec_signal!" );
			goto Failed;
		}

		if ( !MutexInit ( &med->connection.send_mutex ) )
			goto Failed;

        med->available = false;
        med->listening = false;
		med->connection.instance.socket = -1;
		med->connection.instance.spareSocket = -1;
		med->ip = ip;
		med->port = port;

		med->connection.buffer = ( char * ) malloc ( MEDIATOR_BUFFER_SIZE_MAX );
		if ( !med->connection.buffer ) {
			CErr ( "AddMediator: Failed to allocate memory!" );
			goto Failed;
		}

		AddMediator ( med );

		return true;

	Failed:
		free ( med );
		return false;
	}


	MediatorInstance * Mediator::AddMediator ( MediatorInstance * med )
	{
		CVerb ( "AddMediator" );

		if ( !med ) {
			CErr ( "AddMediator: Invalid argument!" );
			return 0;
		}
		MediatorInstance * added = med;

		if ( !MutexLock ( &mediatorLock, "AddMediator" ) ) {
			CErr ( "AddMediator: Failed to aquire mutex on mediator!" );
			added = 0;
			goto Failed;
		}

		if ( mediator.ip ) {
			// Find the last and make sure that we do not have this mediator already in the list
			MediatorInstance * t = &mediator;
			while ( t->next ) {
				t = t->next;
				if ( t->ip == med->ip && t->port == med->port ) {
					CVerbArg ( "AddMediator: Mediator [%s] Port [%d] is already in our list.", inet_ntoa ( *((struct in_addr *) &t->ip) ), t->port );
					added = 0;
					goto FinishUnlock;
				}
			}

			// Attach the new one
			t->next = med;
			med = 0;
		}
		else {
			memcpy ( &mediator, med, sizeof ( MediatorInstance ) );

			med->connection.buffer = 0;

			added = &mediator;
		}

	FinishUnlock:
		MutexUnlockV ( &mediatorLock, "AddMediator" );

	Failed:
		if ( med ) {
			if ( med->connection.buffer )
				free ( med->connection.buffer );
			free ( med );
		}
		return added;
	}


	bool Mediator::RemoveMediator ( unsigned int ip )
	{
		bool ret = true;

		CVerb ( "RemoveMediator" );

		if ( !MutexLock ( &mediatorLock, "RemoveMediator" ) )
			return false;

		// Find the mediator
		MediatorInstance * s = 0;
		MediatorInstance * t = &mediator;
		do {
			if ( ip == t->ip )
				break;

			s = t;
			t = t->next;
		}
		while ( t );

		if ( t ) {
			MediatorInstance * src = t->next;
			if ( t == &mediator ) {
				if ( src ) {
					// There is a next instance
					memcpy ( t, src, sizeof ( MediatorInstance ) );
					ReleaseMediator ( src );
				}
				else {
					// No more instances
					ReleaseMediator ( src );
					memset ( t, 0, sizeof ( MediatorInstance ) );
				}
			}
			else {
				s->next = t->next;
				ReleaseMediator ( t );
			}
		}

		if ( !MutexUnlock ( &mediatorLock, "RemoveMediator" ) )
			ret = false;

		return ret;
	}



} /* namepace environs */
