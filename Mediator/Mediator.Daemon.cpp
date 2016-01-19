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
#include "stdafx.h"

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#	define DEBUGVERB
//#	define DEBUGVERBVerb
//#	define DEBUGSOCKETWATCH
#endif

#include "Mediator.Daemon.h"
#include "Environs.Release.h"
#include "Environs.Native.h"
#include "Environs.Utils.h"
using namespace environs;

#ifndef _WIN32
#	if defined(OutputDebugStringA)
#		undef OutputDebugStringA
#	endif
#	define OutputDebugStringA(msg)		fputs(msg,stdout)
#endif

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <queue>
using namespace std;

#ifdef WIN32
#	include <conio.h>
#	include "windows.h"
#	include <ws2tcpip.h>
#	include <Winhttp.h>
#	include "Environs.Build.Lnk.h"

// Link to winsock library
#	pragma comment ( lib, "Ws2_32.lib" )

// Link to winsock library
#	pragma comment ( lib, "Winhttp.lib" )
#else
#	include <string.h>

#	include "stdio.h"
#	include "errno.h"
#	include <cerrno>

#	include <termios.h>
#	include <curl/curl.h>
#	include <unistd.h>
#	include <fcntl.h>

int _getch ( )
{

   static int inchar = -1, fd = 0;
   struct termios cur, old;

   fd = fileno ( stdin );
   tcgetattr ( fd, &old );

   cur = old;
   cur.c_lflag &= ~(ICANON|ECHO);

   tcsetattr ( fd, TCSANOW, &cur );

   inchar = getchar ( );
   tcsetattr ( fd, TCSANOW, &old );

   return inchar;
}

#endif

#include <stdarg.h>


#define CLASS_NAME		"Daemon"


#define USE_VERIFYSOCKETS

// Forward declarations

bool				stdlog = true;
bool				logging;
ofstream			logfile;
pthread_mutex_t     logMutex;

static queue<NotifyQueueContext *> notifyQueue;

#ifdef DEBUGSOCKETWATCH
    static map<int, int> socketList;
#endif



#ifdef USE_LOCKFREE_SOCKET_ACCESS

SOCKETSYNC InvalidateThreadSocket ( SOCKETSYNC * psock )
{
    SOCKETSYNC sock = *psock;
    if ( sock != -1 ) {
        sock = ___sync_val_compare_and_swap ( psock, sock, -1 );
    }
    return sock;
}


SOCKETSYNC ReplaceThreadSocket ( SOCKETSYNC * psock, int replace )
{
    SOCKETSYNC sock = *psock;
    sock = ___sync_val_compare_and_swap ( psock, sock, replace );
    return sock;
}


SOCKETSYNC CloseReplaceThreadSocket ( SOCKETSYNC * psock, int replace )
{
    SOCKETSYNC sock = ReplaceThreadSocket ( psock, replace );
    if ( sock != -1 )
    {
        CVerbArg ( "CloseReplaceThreadSocket: Closing [ %i ]", sock );
        
        shutdown ( (int) sock, 2 );
        closesocket ( (int) sock );
    }
    return sock;
}


void ClearSpareSockets ( ThreadInstance * client )
{    
    if ( MutexLockA ( client->spareSocketsLock, "ClearSpareSockets" ) )
    {
        size_t size = client->spareSocketsCount;
        if ( size > 0 )
        {
            INTEROPTIMEVAL check = GetEnvironsTickCount ();
            
            if ( (check - client->spareSocketTime) > maxSpareSocketAlive )
            {
                for ( size_t i = 0; i < size; ++i )
                {
                    int sock = (int) client->spareSockets [i];
                    if ( sock != -1 )
                    {
                        CVerbArg ( "ClearSpareSockets: Closing [ %i ]", sock );
                        
                        shutdown ( sock, 2 );
                        closesocket ( sock );
                    }
                }
				client->spareSocketsCount = 0;
            }
        }
        
        MutexUnlockA ( client->spareSocketsLock, "ClearSpareSockets" );
    }
}


void ReplaceSpareSocket ( ThreadInstance * client, int replace )
{
    SOCKETSYNCNV sock = ReplaceThreadSocket ( &client->spareSocket, replace );
    if ( sock != -1 )
    {
        //if ( MutexLockA ( client->spareSocketsLock, "ReplaceSpareSocket" ) )
        //{
            client->spareSockets [ client->spareSocketsCount ] = (int) sock;
			client->spareSocketsCount++;
            client->spareSocketTime = GetEnvironsTickCount ();
        
            LimitSpareSocketsCount ( client );
            
            MutexUnlockA ( client->spareSocketsLock, "ReplaceSpareSocket" );
        //}
    }
}


bool CloseThreadSocket ( SOCKETSYNC * psock )
{
    SOCKETSYNC sock = ReplaceThreadSocket ( psock, -1 );
    if ( sock != -1 ) {
        CVerbArg ( "CloseThreadSocket: Closing [ %i ]", sock );
        
        shutdown ( (int) sock, 2 );
        closesocket ( (int) sock );
        return true;
    }
    return false;
}

#endif


bool ThreadInstance::Init ()
{
    spareSocketTime = 0;
    
	init = MutexInit ( &lock );
    
    if ( init )
        init = MutexInit ( &spareSocketsLock );
    
	return init;
}

ThreadInstance::~ThreadInstance ()
{    
#ifdef USE_LOCKFREE_SOCKET_ACCESS
    try
    {
        CloseThreadSocket ( &socket );
        
        CloseThreadSocket ( &spareSocket );
    }
    catch (...)
    {
    }
#else
	if ( init )
		Lock ( "ThreadInstance: Destruct" );

    int sock = socket;
    if ( sock != -1 )
    {
        socket = -1;
        
		try
        {
            CVerbArg ( "ThreadInstance: Destruct Closing [ %i ]", sock );
            
			shutdown ( sock, 2 );
			closesocket ( sock );
        }
        catch (...)
		{
		}
    }
    
    sock = spareSocket;
    if ( sock != -1 )
    {
        spareSocket = -1;
        
		try
        {
            CVerbArg ( "ThreadInstance: Destruct Closing [ %i ]", sock );
            
			shutdown ( sock, 2 );
			closesocket ( sock );
        }
        catch (...)
		{
		}
    }
    
	if ( init )
		Unlock ( "ThreadInstance: Destruct" );
#endif
    
    if ( aes.encCtx ) {
        CVerbVerb ( "ThreadInstance: Disposing AES key context." );
        AESDisposeKeyContext ( &aes );
    }

    if ( init )
	{
		//MutexLockA ( spareSocketsLock, "ThreadInstance" );

        if ( spareSocketsCount > 0 ) {
            for ( size_t i = 0; i < (size_t) spareSocketsCount; ++i )
            {
                int sock = ( int ) spareSockets [ i ];
                if ( sock != -1 )
                {
                    CVerbArg ( "ThreadInstance: Destruct Closing [ %i ]", sock );
                    
                    shutdown ( sock, 2 );
                    closesocket ( sock );
                }
            }
            spareSocketsCount = 0;
        }

		//MutexUnlockA ( spareSocketsLock, "ThreadInstance" );
		
		CVerbVerbArg ( "ThreadInstance [ %s : %i ]: Disposing accessMutex.", ips, port );
        MutexDispose ( &lock );
        MutexDispose ( &spareSocketsLock );
    }
}


bool ThreadInstance::Lock ( const char * func )
{
    return MutexLockA ( lock, func );
}

bool ThreadInstance::Unlock ( const char * func )
{
    return MutexUnlockA ( lock, func );
}


ILock1::ILock1 ()
{
	init1 =  MutexInit ( &lock1 );
}

bool ILock1::Init1 ()
{
	if ( !init1 )
		init1 =  MutexInit ( &lock1 );
	return init1;
}

bool ILock1::Lock1 ( const char * func )
{
	return MutexLockA ( lock1, func );
}

bool ILock1::Unlock1 ( const char * func )
{
	return MutexUnlockA ( lock1, func );
}

ILock1::~ILock1 ()
{
	if ( init1 )
		MutexDispose ( &lock1 );
}


#ifdef USE_NONBLOCK_CLIENT_SOCKET

bool WaitForData ( int sock )
{
    fd_set fds;
    struct timeval timeout;

	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
    
    FD_ZERO ( &fds );
    FD_SET ( ( unsigned ) sock, &fds );
    
    int rc = select ( sock + 1, &fds, NULL, NULL, &timeout );
    if ( rc == -1 )
    {
        VerbLogSocketError (); VerbLogSocketError1 ();
        CVerbArg ( "WaitForData [ %i ]:\tconnection/socket closed by someone; rc [ -1 ]!", sock );
        return false;
    }
    if ( rc == 0 ) {
        CVerbVerbArg ( "WaitForData [ %i ]:\tconnection/socket timeoout; rc [ 0 ]!", sock );
        return true;
    }
    if ( !FD_ISSET(sock, &fds) ) {
        CVerbVerbArg ( "WaitForData [ %i ]:\tconnection/socket error; rc [ %i ]!", sock, rc );
        //return false;
    }
    return true;
}

#endif


DeviceInstanceNode::~DeviceInstanceNode ()
{
	clientSP = 0; rootSP = 0;
}


MediatorDaemon::MediatorDaemon ()
{
	CVerb ( "Construct" );

	allocated				= false;
    acceptEnabled           = true;
    anonymousLogon          = true;
    *anonymousUser          = 0;
    *anonymousPassword      = 0;
	
	input					= inputBuffer;

	listeners           .clear ();
	usersDB             .clear ();

	ports               .clear ();
	bannedIPs           .clear ();
    bannedIPConnects    .clear ();
    bannAfterTries      = 3;
	
	areasCounter        = 0;
	areaIDs             .clear ();

	appsCounter         = 0;
	appIDs              .clear ();
    
    notifyTargets       .clear ();

	networkOK			= 0xFFFFFFFF;
	checkLast			= 0;

	usersDBDirty		= false;
	configDirty			= false;
    deviceMappingDirty  = false;
    
    Zero ( aesKey );
    Zero ( aesCtx );
    
	privKey				= 0;
	privKeySize			= 0;
	
	Zero ( notifyThreadID );

	srand ( getRandomValue ( (void *) &checkLast ) );

    /// Android -> win daemon
    /// o = OAEP SHA1 MGF1: +openssl, -win32
    /// r = OAEP SHA256 MGF1: -openssl, -win32
    /// p = PKCS1 - openssl: +openssl, -win32
    /// n = no padding: -openssl, -win32

    ///
    /// Win -> openssl daemon
    /// o = OAEP SHA1 MGF1: +android, -win32 (data too large for modulus)
    /// r = OAEP SHA256 MGF1: -openssl, -win32
    /// p = PKCS1 - openssl: +openssl, -win32
    /// n = no padding: -openssl, -win32
    encPadding              = ENVIRONS_DEFAULT_CRYPT_PAD;

	spareID					= 0;

	sessionCounter			= 1;

	reqAuth					= true;
	logging					= false;

	InitEnvironsCrypt ();
}


MediatorDaemon::~MediatorDaemon ()
{
	CLog ( "Destructor" );

	Dispose ();

    notifyTargets   .clear ();
	areaIDs		    .clear ();
	appIDs          .clear ();

	CloseLog ();

    AESDisposeKeyContext ( &aesCtx );

	ReleaseEnvironsCrypt ();

	if ( allocated ) {
		MutexDispose ( &usersDBLock );
		MutexDispose ( &bannedIPsLock );
		MutexDispose ( &thread_lock );
        MutexDispose ( &notifyLock );
        MutexDispose ( &notifyTargetsLock );
		MutexDispose ( &logMutex );
	}
}


bool MediatorDaemon::InitMediator ()
{
	CVerb ( "InitMediator" );

	if ( !allocated ) {
        if ( !MutexInit ( &thread_lock ) )
            return false;

		if ( pthread_cond_init		( &thread_condition, NULL ) ) {
			CErr ( "InitMediator: Failed to init thread_condition." );
			return false;
        }
        
        if ( pthread_cond_init		( &notifyEvent, NULL ) ) {
            CErr ( "InitMediator: Failed to init notifyEvent." );
            return false;
        }

		if ( pthread_cond_init		( &hWatchdogEvent, NULL ) ) {
			CErr ( "InitMediator: Failed to init hWatchdogEvent." );
			return false;
        }

		if ( !areas.Init () )
			return false;
        areas.list.clear ();
        
        if ( !deviceMappings.Init () )
            return false;
        deviceMappings.list.clear ();
        
        if ( !spareClients.Init () )
            return false;
        spareClients.list.clear ();
        
        if ( !areasMap.Init () )
            return false;
        areasMap.list.clear ();
        
        if ( !sessions.Init () )
            return false;
        sessions.list.clear ();
        
        if ( !acceptClients.Init () )
            return false;
        acceptClients.list.clear ();
        
        if ( !MutexInit ( &bannedIPsLock ) )
            return false;
        
        if ( !MutexInit ( &usersDBLock ) )
            return false;
        
        if ( !MutexInit ( &notifyLock ) )
            return false;
        
        if ( !MutexInit ( &notifyTargetsLock ) )
            return false;
        
        if ( !MutexInit ( &logMutex ) )
            return false;

        allocated = true;
        
        strlcpy ( anonymousUser, MEDIATOR_ANONYMOUS_USER, sizeof ( anonymousUser ) );
            
		strlcpy ( anonymousPassword, MEDIATOR_ANONYMOUS_PASSWORD, sizeof ( anonymousPassword ) );
	}

	return true;
}


void MediatorDaemon::InitDefaultConfig ()
{
	CVerb ( "InitDefaultConfig" );

	ports.clear();
	ports.push_back(DEFAULT_MEDIATOR_PORT);
}


void MediatorDaemon::ReleaseKeys ()
{
	CVerb ( "ReleaseKeys" );
		
	if ( privKey ) {
		DisposePrivateKey ( (void **) &privKey );
	}
	privKeySize				= 0;
}


bool MediatorDaemon::LoadKeys ()
{
	CVerb ( "LoadKeys" );

	ReleaseKeys ();

	if ( !environs::LoadPublicCertificate ( "./ssl/mediator.crt", &certificate ) ) {
		CWarn ( "LoadKeys: Failed to load mediator certificate." );
		return false;
	}
	
	if ( !environs::LoadPrivateKey ( "./ssl/mediator.priv.key", &privKey, &privKeySize ) ) {
		CWarn ( "LoadKeys: Failed to load mediator private key." );
		return false;
	}
    
	return true;
}


bool MediatorDaemon::LoadConfig ()
{
	CVerb ( "LoadConfig" );

    char * aesCryptKey = 0;
    
    string line;
    string prefix;
    string svalue;
    ifstream conffile;
    unsigned int value;
    
	conffile.open ( CONFFILE );
    
	if ( !conffile.good() )
		goto NoConfFile;

	// Adapt default configuration
    
	while ( getline ( conffile, line ) )
	{
		const char * str = line.c_str ();
		if ( strlen ( str ) < 3 )
			continue;

		std::istringstream iss ( line );
		
		if ( str [ 0 ] == 'M' && str [ 1 ] == ':' ) {
			unsigned short port;

			if (!(iss >> prefix >> value >> port)) {
				CLogArg ( "LoadConfig: Invalid config line: %s", str );
			}
			else {
				AddMediator ( value, port );
			}
		}
		else if ( str [ 0 ] == 'N' && str [ 1 ] == ':' ) {
			if ( !( iss >> prefix >> svalue  ) ) {
				CLogArg ( "LoadConfig: Invalid config line: %s", str );
			}
			else {
				struct sockaddr_in addr;
				inet_pton ( AF_INET, svalue.c_str (), &( addr.sin_addr ) );
				networkOK = ( unsigned int ) addr.sin_addr.s_addr;
			}
		}
		else if ( str [ 0 ] == 'B' && str [ 1 ] == ':' ) {
			std::time_t dateTime;

			if (!(iss >> prefix >> value >> dateTime)) {
				CLogArg ( "LoadConfig: Invalid config line: %s", str );
			}
			else {
				bannedIPs [ value ] = dateTime;
			}
        }
        else if ( str [ 0 ] == 'C' && str [ 1 ] == 'V' && str [ 2 ] == ':' ) {
            if (!(iss >> prefix >> value )) {
                CLogArg ( "LoadConfig: Invalid config line: %s", str );
            }
            else {
                if ( value < 100 ) {
                    bannAfterTries = value;
                }
                else {
                    CVerbArg ( "LoadConfig: Invalid try value for bann: %u", value );
                }
            }
        }
        else if ( str [ 0 ] == 'A' && str [ 1 ] == 'N' && str [ 2 ] == ':' ) {
            if (!(iss >> prefix >> value )) {
                CLogArg ( "LoadConfig: Invalid config line: %s", str );
            }
            else {
                anonymousLogon = value ? true : false;
            }
        }
        else if ( str [ 0 ] == 'A' && str [ 1 ] == 'U' && str [ 2 ] == ':' ) {
            if (!(iss >> prefix >> svalue )) {
                CLogArg ( "LoadConfig: Invalid config line: %s", str );
            }
            else {
				strlcpy ( anonymousUser, svalue.c_str(), sizeof ( anonymousUser ) );
            }
        }
        else if ( str [ 0 ] == 'A' && str [ 1 ] == 'P' && str [ 2 ] == ':' ) {
            if (!(iss >> prefix >> svalue )) {
                CLogArg ( "LoadConfig: Invalid config line: %s", str );
            }
            else {
				strlcpy ( anonymousPassword, svalue.c_str(), sizeof ( anonymousPassword ) );
            }
        }
        else if ( str [ 0 ] == 'C' && str [ 1 ] == ':' ) {
            unsigned int tries;
            
            if (!(iss >> prefix >> value >> tries)) {
                CLogArg ( "LoadConfig: Invalid config line: %s", str );
            }
            else {
                bannedIPConnects [ value ] = tries;
            }
        }
		else if ( str [ 0 ] == 'A' && str [ 1 ] == 'E' && str [ 2 ] == 'S' && str [ 3 ] == ':' ) {
			string aesString;

			if (!(iss >> prefix >> aesString)) {
				CLogArg ( "LoadConfig: Invalid config line: %s", str );
			}
			else {
                /// Decrypt the aes key
                unsigned int decLength = ((unsigned int) aesString.length()) >> 1;
                
                char * aesBuffer = (char *) malloc ( decLength + 4 );
                if ( aesBuffer )
                {
                    char * converted = ConvertToByteBuffer ( aesString.c_str(), (unsigned int) aesString.length(), aesBuffer );
                    if ( converted )
                    {                        
                        if ( !DecryptMessage ( privKey, privKeySize, converted, decLength, &aesCryptKey, &decLength ) ) {
                            CWarn ( "LoadConfig: Decrypt with privKey failed!" );
                        }
                        else if ( aesCryptKey ) {
                            if ( decLength <= sizeof(aesKey) )
                                memcpy ( aesKey, aesCryptKey, sizeof(aesKey) );
                            
                            free ( aesCryptKey );
                        }
                    }
                    
                    free ( aesBuffer );
                }
			}
		}
		else if ( str [ 0 ] == 'P' && str [ 1 ] == 'o' && str [ 2 ] == 'r' && str [ 3 ] == 't' && str [ 4 ] == 's' && str [ 5 ] == ':' ) {
			unsigned short port = DEFAULT_MEDIATOR_PORT;

			if (!(iss >> prefix >> port)) { 
				CLogArg ( "LoadConfig: Invalid config line: %s", str );
			}
			else {
				ports.clear();
				ports.push_back ( port );
			}
		}

	}
	
	conffile.close();
    
NoConfFile:
    if ( !aesCryptKey ) {
        BUILD_IV_128 ( aesKey );
        BUILD_IV_128 ( aesKey + 16 );
        
        configDirty = true;
    }
    
    AESDisposeKeyContext ( &aesCtx );
    
    if ( !AESDeriveKeyContext ( aesKey, sizeof(aesKey), &aesCtx ) ) {
        CWarn ( "LoadConfig: Derive AESContext failed!" );
    }


	BuildBroadcastMessage ();

	return true;
}


bool MediatorDaemon::SaveConfig ()
{
	CVerb ( "SaveConfig" );

	ofstream conffile;
	conffile.open ( CONFFILE );
	if ( !conffile.good() )
		return false;
	
	stringstream configs;
    
    configs << "Ports: ";
    
	// Save 
	if ( ports.size() > 0 )
	{

		for ( unsigned int pos = 0; pos < ports.size(); pos++ ) {
			int port = ports[pos];
			configs << port << " ";
		}
	}
	else
		configs << "5899";
	configs << endl;	
	
	if ( MutexLock ( &mediatorLock, "SaveConfig" ) ) 
	{
		MediatorInstance * net = &mediator;

		while ( net && net->ip ) {
			configs << "M: " << net->ip << " " << net->port ;
			
			configs << endl;
			net = net->next;
		}
	
		MutexUnlock ( &mediatorLock, "SaveConfig" );
    }

	if ( networkOK != 0xFFFFFFFF ) 
	{
		struct sockaddr_in addr;
		char ipStr [ INET_ADDRSTRLEN ];

		addr.sin_addr.s_addr = networkOK;
		inet_ntop ( AF_INET, &( addr.sin_addr ), ipStr, INET_ADDRSTRLEN );

		configs << "N: " << ipStr << endl;
	}
    
    configs << "CV: " << bannAfterTries << endl;
    configs << "AN: " << (int)anonymousLogon << endl;
	
    if ( strncmp ( anonymousUser, MEDIATOR_ANONYMOUS_USER, MAX_NAMEPROPERTY ) ) {
        configs << "AU: " << anonymousUser << endl;
        configs << "AP: " << anonymousPassword << endl;
    }
    
    char aesCryptKey [ 514 ];
    memcpy ( aesCryptKey, aesKey, sizeof(aesKey) );
    
    unsigned int length = sizeof(aesKey);
    
    if ( !EncryptMessage ( 0, certificate, aesCryptKey, &length ) ) {
        CWarn ( "SaveConfig: Encrypt with certificate failed!" );
    }
    else if ( length >= 256 ) {
        
        const char * hexStr = ConvertToHexString ( aesCryptKey, length );
        if ( hexStr ) {
			configs << "AES: " << hexStr << " ";
			
			configs << endl;
        }
    }
    	
	if ( MutexLockA ( bannedIPsLock, "SaveConfig" ) )
	{
		std::map<unsigned int, std::time_t>::iterator iter;

		for ( iter = bannedIPs.begin(); iter != bannedIPs.end(); ++iter ) {
			configs << "B: " << iter->first << " " << iter->second << endl;
        }
        
        std::map<unsigned int, unsigned int>::iterator itert;
        
        for ( itert = bannedIPConnects.begin(); itert != bannedIPConnects.end(); ++itert ) {
            configs << "C: " << itert->first << " " << itert->second << endl;
        }
	
		MutexUnlockA ( bannedIPsLock, "SaveConfig" );
	}
    
    conffile << configs.str() << endl;

	conffile.close();
	return true;
}


bool MediatorDaemon::LoadProjectValues ()
{
	CVerb ( "LoadProjectValues" );

	ifstream conffile;
	conffile.open ( DATAFILE );
	if ( !conffile.good() )
		return false;
	
    areasMap.Lock ( "LoadProjectValues" );

	sp ( AppsList )		apps;
	sp ( ListValues )   listValues		= 0;
	
	string line;
	while ( getline ( conffile, line ) )
	{
		const char * str = line.c_str ();
		if ( strlen ( str ) < 3 )
			continue;

		if ( str [ 0 ] == 'P' && str [ 1 ] == ':' ) {
			CVerbArg ( "Loading area [%s]", str );

			apps = make_shared < AppsList > ();
			if ( !apps || !apps->Init () ) {
				CErrArg ( "LoadProjectValues: Failed to create new area [%s].", str + 2 );
				break;
			}
			areasMap.list [ string ( str + 2 ) ] = apps;
			continue;
		}

		if ( str [ 0 ] == 'A' && str [ 1 ] == ':' ) {
			if ( !apps ) {
				CErrArg ( "LoadProjectValues: Missing project for app [%s]!", str + 2 );
				break;
			}
			CVerbArg ( "Loading application [%s]", str );

            listValues = make_shared < ListValues > ();
			if ( !listValues || !listValues->Init () ) {
				CErrArg ( "LoadProjectValues: Failed to create new application [%s].", str + 2 );
				break;
			}
			apps->Lock ( "LoadProjectValues" );

			apps->apps [ string ( str + 2 ) ] = listValues;

			apps->Unlock ( "LoadProjectValues" );
			continue;
		}

		if ( !listValues ) {
			CErr ( "LoadProjectValues: Invalid data file format. Project/App definition for values missing!" );
			break;
		}
		
		std::istringstream iss ( line );
		string key, value;
        std::time_t timestamp;
		unsigned int size = 0;
        
		if ( !(iss >> key >> size >> value >> timestamp) ) {
			CErrArg ( "LoadProjectValues: Failed to read key/value (%s)!", line.c_str() );

			std::istringstream iss1 ( line );
			if ( !( iss1 >> key >> size >> value ) ) {
				CErrArg ( "LoadProjectValues: Failed to read key/value (%s)!", line.c_str () );
				break;
			}

			timestamp = 0;
		}

		sp ( ValuePack ) pack = make_shared < ValuePack > ();
		if ( !pack ) {
			CErrArg ( "LoadProjectValues: Failed to create new value object for %s/%u/%s! Memory low problem!", key.c_str(), size, value.c_str() );
			break;
		}

		pack->timestamp = timestamp;
		pack->value = value;
		pack->size = size;

		listValues->Lock ( "LoadProjectValues" );

		listValues->values [ key ] = pack;

		listValues->Unlock ( "LoadProjectValues" );
	}
	
	/// Sanitinize database
	msp ( string, AppsList )::iterator it = areasMap.list.begin();
	
	while ( it != areasMap.list.end() )
	{
        sp ( AppsList ) appList = it->second;
        
		if ( !appList ) {
			areasMap.list.erase ( it );
			it = areasMap.list.begin();
			continue;
		}
		
		msp ( string, ListValues )::iterator ita = appList->apps.begin();
		while ( ita != appList->apps.end() )
		{
			if ( !ita->second ) {
				appList->apps.erase ( ita );
				ita = appList->apps.begin();
				continue;
			}

			if ( !ita->second->values.size() ) {
				appList->apps.erase ( ita );
				ita = appList->apps.begin();
				continue;
			}
			ita++;
		}

		if ( !appList->apps.size() ) {
			areasMap.list.erase ( it );
			it = areasMap.list.begin();
			continue;
		}

		it++;
	}
    
    areasMap.Unlock ( "LoadProjectValues" );

	conffile.close();

	return true;
}


bool MediatorDaemon::SaveProjectValues ()
{
	bool success = true;

	CLog ( "SaveProjectValues" );

	ofstream conffile;
	conffile.open ( DATAFILE );
	if ( !conffile.good() )
		return false;

	if ( !areasMap.Lock ( "SaveProjectValues" ) ) {
		success = false;
	}
	else {
		// Collect areas
		msp ( string, AppsList ) tmpAreas;
		for ( msp ( string, AppsList )::iterator it = areasMap.list.begin(); it != areasMap.list.end(); ++it )
        {
            sp ( AppsList ) appsList = it->second;
            if ( !appsList )
                continue;

			tmpAreas [ it->first ] = appsList;
		}

		if ( !areasMap.Unlock ( "SaveProjectValues" ) ) {
			success = false;
		}

		// Save areas
		for ( msp ( string, AppsList )::iterator it = tmpAreas.begin (); it != tmpAreas.end (); ++it )
		{
			sp ( AppsList ) appsList = it->second;
			if ( !appsList )
				continue;

			sp ( AreaApps ) areaApps = 0;

			areas.Lock ( "SaveProjectValues" );

			const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( it->first );
			if ( areaIt != areas.list.end () )
				areaApps = areaIt->second;
			else
				areaApps = 0;

			areas.Unlock ( "SaveProjectValues" );

			conffile << "P:" << it->first << endl;

			if ( !appsList->Lock ( "SaveProjectValues" ) )
				continue;

			msp ( string, ListValues ) tmpListValues;

			for ( msp ( string, ListValues )::iterator ita = appsList->apps.begin (); ita != appsList->apps.end (); ita++ )
			{
				sp ( ListValues ) listValues = ita->second;
				if ( !listValues )
					continue;

				tmpListValues [ ita->first ] = listValues;
			}

			appsList->Unlock ( "SaveProjectValues" );


			for ( msp ( string, ListValues )::iterator ita = tmpListValues.begin (); ita != tmpListValues.end (); ita++ )
			{
				sp ( ListValues ) listValues = ita->second;
				if ( !listValues || !listValues->Lock ( "SaveProjectValues" ) )
					continue;

				unsigned int maxID = 0;

				/// Find the appList
				if ( areaApps && areaApps->Lock ( "SaveProjectValues" ) )
				{
					const msp ( string, ApplicationDevices )::iterator appsIt = areaApps->apps.find ( ita->first );
					if ( appsIt != areaApps->apps.end () )
					{
						sp ( ApplicationDevices ) appDevices = appsIt->second;
						if ( appDevices )
							maxID = ( unsigned int ) appDevices->latestAssignedID;
					}

					areaApps->Unlock ( "SaveProjectValues" );
				}

				conffile << "A:" << ita->first << endl;

				if ( maxID > 0 )
					conffile << "0_maxID " << maxID << " ids" << endl;

				for ( msp ( string, ValuePack )::iterator itv = listValues->values.begin (); itv != listValues->values.end (); ++itv )
				{
					sp ( ValuePack ) value = itv->second;
					if ( value ) {
						if ( maxID > 0 && itv->first.c_str () [ 0 ] == '0' )
							continue;
						conffile << itv->first << " " << value->size << " " << value->value << " " << value->timestamp << endl;
					}
				}

				listValues->Unlock ( "SaveProjectValues" );
			}
		}
	}

	conffile.close();
	return success;
}


bool MediatorDaemon::LoadDeviceMappingsEnc ()
{
	CVerb ( "LoadDeviceMappingsEnc" );

	ifstream conffile;
	conffile.open ( DEVMAPFILE );
	if ( !conffile.good() )
        return false;
    
    LoadDeviceMappings ( conffile );
    
	conffile.close();

	return true;
}


bool MediatorDaemon::LoadDeviceMappings ( istream& instream )
{
	if ( !MutexLockA ( usersDBLock, "LoadDeviceMappings" ) )
		return false;
    
    string line;
    while ( getline ( instream, line ) )
    {
        std::istringstream iss ( line );
        string deviceUID;
        unsigned deviceID;
        int authLevel = 0;
        string authToken;
        
        if ( !(iss >> deviceUID >> deviceID >> authLevel >> authToken ) ) {
            CErrArg ( "LoadDeviceMappings: Failed to read device mapping (%s)!", line.c_str() );
            continue;
        }
        
        sp ( DeviceMapping ) mapping;
		mapping.reset ( new DeviceMapping );
		//= make_shared < DeviceMapping > ; // sp ( DeviceMapping ) ( new DeviceMapping ); // calloc ( 1, sizeof(DeviceMapping) );
        if ( !mapping )
            break;
		memset ( mapping.get (), 0, sizeof(DeviceMapping) );
        
        mapping->deviceID = deviceID;
        mapping->authLevel = authLevel;
        if ( authToken.length() > 0 )
            memcpy ( mapping->authToken, authToken.c_str(), authToken.length() );
        
        if ( deviceMappings.Lock ( "LoadDeviceMappings" ) ) {
            deviceMappings.list [ deviceUID ] = mapping;
            
            deviceMappings.Unlock ( "LoadDeviceMappings" );
        }
    }
    
    return true;
}


bool MediatorDaemon::LoadDeviceMappings ()
{
	CVerb ( "LoadDeviceMappings" );
    
    unsigned int size = 0;
    char * decrypted = 0;
    char * ciphers = LoadBinary ( DEVMAPFILE, (int *) &size );
    
    if ( !ciphers )
        return false;
    
    while ( size ) {
        if ( !AESDecrypt ( &aesCtx, ciphers, &size, &decrypted ) || !size || !decrypted )
            break;
        
        istringstream text ( (string ( decrypted )) );
        
        LoadDeviceMappings ( text );
        
        break;
    }
	
	if ( decrypted )
		free ( decrypted );
    free ( ciphers );
	return true;
}


bool MediatorDaemon::SaveDeviceMappings ( )
{
	bool ret = true;

	CVerb ( "SaveDeviceMappings" );

    stringstream plainstream;

	if ( !deviceMappings.Lock ( "SaveDeviceMappings" ) ) {
		ret = false;
	}
	else {
		// Save devcie mappings
		for ( msp ( string, DeviceMapping )::iterator it = deviceMappings.list.begin(); it != deviceMappings.list.end(); it++ )
		{
			if ( !it->second )
				continue;

			plainstream << it->first << " " << it->second->deviceID << " " << it->second->authLevel << " " << it->second->authToken << endl;
		}
        
		if ( !deviceMappings.Unlock ( "SaveDeviceMappings" ) ) {
			ret = false;
		}
	}

    if ( ret ) {
        /// Encrypt plaintext
        string text = plainstream.str ();
        
        char * cipher = 0;
        unsigned int bufferLen = (unsigned int) text.length ();
        
        if ( bufferLen ) {
            ret = false;
            
            if ( AESEncrypt ( &aesCtx, (char *) text.c_str (), &bufferLen, &cipher ) && cipher ) {
                if ( SavePrivateBinary ( DEVMAPFILE, cipher, bufferLen ) ) {
                    ret = true;
                }
                else CErr ( "SaveDeviceMappings: Failed to save into file." );
                
                free ( cipher );
            }
            else CErr ( "SaveDeviceMappings: AESEncrypt failed." );
        }
        else {
            unlink ( DEVMAPFILE );
        }
    }
    
	return ret;
}


bool MediatorDaemon::AddUser ( int authLevel, const char * userName, const char * pass )
{
	CLog ( "AddUser" );
	
	bool ret = false;
	
	char * hash = 0;
	unsigned int len = 0;
    UserItem * item = 0;

	do
	{
        item = new UserItem ();
        if ( !item )
            break;
        
		ret = SHAHashPassword ( pass, &hash, &len );
		if ( !ret ) {
			CErrArg ( "AddUser: Failed to create pass hash for [%s]", userName ); break;
		}

		if ( !hash ) {
			CErrArg ( "AddUser: Invalid null pass hash for [%s]", userName ); break;
		}
		
		string user = userName;
		std::transform ( user.begin(), user.end(), user.begin(), ::tolower );

		const map<string, UserItem *>::iterator iter = usersDB.find ( user );
		if ( iter != usersDB.end () ) {
			CLogArg ( "AddUser: Updating password of user [%s]", userName );
			printf ( "\nAddUser: Updating password of user [%s]\n", userName );
		}
		
        item->authLevel = authLevel;
        item->pass = string ( hash );
        
        
        if ( !MutexLockA ( usersDBLock, "AddUser" ) )
            break;
        
        usersDB [ user ] = item;
        
        if ( MutexUnlockA ( usersDBLock, "AddUser" ) )
            ret = true;
        
        item = 0;
	}
	while ( 0 );

	if ( hash )
		free ( hash );
    
    if ( item )
		delete item;

    if ( ret ) {
        usersDBDirty = true;
	}

	return ret;
}


/// For now we don't encrypt the users.db. For productive use, this should be encrypted before storing
bool MediatorDaemon::LoadUserDBEnc ()
{
	CVerb ( "LoadUserDB" );

	ifstream usersfile;
	usersfile.open ( USERSDBFILE );
	if ( !usersfile.good() )
		return false;
    
    LoadUserDB ( usersfile );
	
	usersfile.close();
	
	return true;
}


bool MediatorDaemon::LoadUserDB ( istream& instream )
{
    string userName;
    string userPassword;
    string line;
    string pre1, pre2;
    int authLevel = 0;
    
    while ( getline ( instream, line ) )
    {
        std::istringstream iss ( line );
        
        if ( !(iss >> userName >> pre1 >> authLevel >> pre2 >> userPassword) ) {
            CErrArg ( "LoadUserDB: Failed to read user entry (%s)!", line.c_str() );
            continue;
        }
        
        if ( userName.find ( '@', 0 ) == string::npos )
            continue;
        
        if ( userPassword.length() < 64 ) {
            continue;
        }
        
        if ( authLevel < 0 || authLevel > 10 )
            authLevel = 3;
        
        const char * pwStr = ConvertToBytes ( userPassword.c_str(), (unsigned int)userPassword.length() );
        if ( pwStr ) {
            
            UserItem * item = new UserItem ();
            if ( item ) {
                string pw = pwStr;
                item->pass = pw;
                item->authLevel = authLevel;
                
                usersDB [ userName ] = item;
            }
        }
    }
    return true;
}


bool MediatorDaemon::LoadUserDB ()
{
	CVerb ( "LoadUserDB" );
    
    unsigned int size = 0;
    char * decrypted = 0;
    char * ciphers = LoadBinary ( USERSDBFILE, (int *) &size );
    
    if ( !ciphers )
        return false;
    
    while ( size ) {
        if ( !AESDecrypt ( &aesCtx, ciphers, &size, &decrypted ) || !size || !decrypted )
            break;
        
        istringstream text ( (string ( decrypted )) );
        
        LoadUserDB ( text );
        break;
    }
	
	if ( decrypted )
		free ( decrypted );
    free ( ciphers );
	return true;
}


bool MediatorDaemon::SaveUserDB ()
{
	bool ret = true;

	CLog ( "SaveUserDB" );
    
    stringstream plainstream;

	if ( !MutexLockA ( usersDBLock, "SaveUserDB" ) )
		ret = false;
	else {
		// Save areas
		for ( map<string, UserItem *>::iterator it = usersDB.begin(); it != usersDB.end(); ++it )
		{
			//CVerbArg ( "SaveUserDB: try saving [%s]", it->first.c_str() );

			if ( it->second )
			{
                UserItem * item = it->second;
                
				const char * pwStr = ConvertToHexString ( item->pass.c_str(), ENVIRONS_USER_PASSWORD_LENGTH );
				if ( pwStr ) {
					//CVerbArg ( "SaveUserDB: Saving pass [%s]", pwStr );

                    plainstream << it->first << " :: " << item->authLevel << " :: " << pwStr << endl;
				}
			}
		}
        
		if ( !MutexUnlockA ( usersDBLock, "SaveUserDB" ) )
			ret = false;
	}
    
    if ( ret ) {
        /// Encrypt plaintext
        string text = plainstream.str ();
        
        char * cipher = 0;
        unsigned int bufferLen = (unsigned int) text.length ();
        
        if ( bufferLen ) {
            ret = false;
            
            if ( AESEncrypt ( &aesCtx, (char *) text.c_str (), &bufferLen, &cipher ) && cipher ) {
                if ( SavePrivateBinary ( USERSDBFILE, cipher, bufferLen ) ) {
                    ret = true;
                }
                else CErr ( "SaveUserDB: Failed to save into file." );
                
                free ( cipher );
            }
            else CErr ( "SaveUserDB: AESEncrypt failed." );
        }
    }

	return ret;
}


bool MediatorDaemon::IsIpBanned ( unsigned int ip )
{
	CVerb ( "IsIpBanned" );

	bool banned = false;

	if ( (ip & 0x00FFFFFF) == networkOK ) {
		CVerb ( "IsIpBanned: IP is OK!" );
		return false;
	}

    MutexLockVA ( bannedIPsLock, "IsIpBanned" );

	const std::map<unsigned int, std::time_t>::iterator iter = bannedIPs.find ( ip );
	if ( iter != bannedIPs.end () )
	{
		CLogArg ( "IsIpBanned: A banned IP [%s] tries to connect again.!", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );

		std::time_t bannedDate = iter->second;
		std::time_t now = std::time(0);

		if ( now - bannedDate < 259200 ) {
			CVerb ( "IsIpBanned: The bann has been set within the last 3 days. Refreshing and refusing connect!" );
			bannedIPs [ ip ] = now;

			banned = true;
			goto Finish;
		}
			
		CLogArg ( "IsIpBanned: The bann was more than %d seconds ago. Let the request try again!", now - bannedDate );
		bannedIPs.erase ( iter );
        
        const std::map<unsigned int, unsigned int>::iterator itert = bannedIPConnects.find ( ip );
        if ( itert != bannedIPConnects.end () )
            bannedIPConnects.erase ( itert );
	}
	
Finish:
    MutexUnlockVA ( bannedIPsLock, "IsIpBanned" );

	return banned;
}


void MediatorDaemon::BannIP ( unsigned int ip )
{
	CVerbArg ( "BannIP: [%s]", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );

	if ( (ip & 0x00FFFFFF) == networkOK ) {
		CVerb ( "IsIpBanned: IP is OK!" );
		return;
    }
    
    if ( !MutexLockA ( bannedIPsLock, "BannIP" ) )
        return;
    
    unsigned int tries = bannedIPConnects [ ip ];
    if ( tries < bannAfterTries ) {
        bannedIPConnects [ ip ] = ++tries;
        CVerbVerbArg ( "BannIP: retain bann due to allowed tries [%s] [%u/%u]", inet_ntoa ( *( ( struct in_addr * ) &ip ) ), tries, bannAfterTries );
    }
    else {
        std::time_t t = std::time(0);
        
        bannedIPs [ ip ] = t;
    }
    
    MutexUnlockVA ( bannedIPsLock, "IsIpBanned" );
    
    configDirty = true;
}


void MediatorDaemon::BannIPRemove ( unsigned int ip )
{
    CVerb ( "BannIPRemove" );
    
    bool ret = false;
    
    if ( !MutexLockA ( bannedIPsLock, "BannIPRemove" ) )
        return;
    
    const std::map<unsigned int, std::time_t>::iterator iter = bannedIPs.find ( ip );
    if ( iter != bannedIPs.end () )
    {
        CVerbArg ( "BannIPRemove: The IP [%s] has been banned before", inet_ntoa ( *( ( struct in_addr * ) &ip ) ) );
        ret = true;

        bannedIPs.erase ( iter );
        
        const std::map<unsigned int, unsigned int>::iterator itert = bannedIPConnects.find ( ip );
        if ( itert != bannedIPConnects.end () )
            bannedIPConnects.erase ( itert );
    }
    
    MutexUnlockVA ( bannedIPsLock, "IsIpBanned" );
    
    if ( ret ) {
        configDirty = true;
	}
}


bool MediatorDaemon::ReCreateAcceptor ()
{
	CVerb ( "ReCreateAcceptor" );

    int s, value, ret;
    
#if !defined(_WIN32) || defined(USE_PTHREADS_FOR_WINDOWS)
    void * res;
#endif

	for ( vsp ( MediatorThreadInstance )::iterator it = listeners.begin (); it != listeners.end (); ++it )
	{
		const sp ( MediatorThreadInstance ) listenerSP = *it;
		if ( !listenerSP )
			continue;

		MediatorThreadInstance * listener = listenerSP.get ();
		if ( !listener )
			continue;

		ThreadInstance * inst = &listener->instance;

		if ( pthread_valid ( inst->threadID ) ) {
			int sock = (int) inst->socket;
			if ( sock != -1 ) {
                inst->socket = -1;
                CVerbArg ( "ReCreateAcceptor: Closing [ %i ]", sock );

				shutdown ( sock, 2 );
				closesocket ( sock );
			}

			s = pthread_join ( inst->threadID, &res );
			if ( s != 0 ) {
				CErrArg ( "ReCreateAcceptor: Error waiting for listener thread (pthread_join:%i)", s );
				ret = false;
			}
			pthread_close ( inst->threadID );
		}

		int sock = ( int ) socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if ( sock < 0 ) {
			CErrArg ( "ReCreateAcceptor: Failed to create socket for listener port (%i)!", inst->port );
			return false;
		}
		inst->socket = sock;

		value = 1;
		ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) );
		if ( ret < 0 ) {
			CErr ( "ReCreateAcceptor: Failed to set reuseAddr on listener socket." ); LogSocketError ();
			return false;
		}

		MutexLockVA ( thread_lock, "ReCreateAcceptor" );

		s = pthread_create ( &inst->threadID, 0, &MediatorDaemon::AcceptorStarter, ( void * ) listener );
		if ( s != 0 ) {
			CErrArg ( "ReCreateAcceptor: Error creating acceptor thread for port %i (pthread_create:%i)", inst->port, s );
			pthread_mutex_unlock ( &thread_lock );
			return false;
		}

		pthread_cond_wait ( &thread_condition, &thread_lock );
		MutexUnlockVA ( thread_lock, "ReCreateAcceptor" );

		CLog ( "ReCreateAcceptor: Created acceptor done." );
	}
	return true;
}


bool MediatorDaemon::CreateThreads ()
{
	CVerb ( "CreateThreads" );

	int s, value, ret;

	ReleaseThreads ();

	isRunning = true;

	for ( unsigned int pos = 0; pos < ports.size(); pos++ )
	{
        sp ( MediatorThreadInstance ) listenerSP = make_shared < MediatorThreadInstance > (); //sp ( MediatorThreadInstance ) ( new MediatorThreadInstance ); // make_shared < MediatorThreadInstance > ();
		if ( !listenerSP ) {
			CErrArg ( "CreateThreads: Error - Failed to allocate memory for new Listener [Nr. %i]", pos );
			return false;
		}
        MediatorThreadInstance * listener = listenerSP.get();
        
		memset ( listener, 0, sizeof(MediatorThreadInstance) );
        
		listener->socketUdp = -1;
		listener->instance.daemon = this;

		ThreadInstance * inst = &listener->instance;
		inst->socket = -1;
		inst->spareSocket = -1;

		inst->port = ports [ pos ];

		int sock = (int) socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if ( sock < 0 ) {
    		CErrArg ( "CreateThreads: Failed to create socket for listener port (%i)!", inst->port );
			return false;
		}
		inst->socket = sock;

		struct linger so_linger;
		Zero ( so_linger );

		so_linger.l_onoff   = 1;
		so_linger.l_linger  = 4;
		
		ret = setsockopt ( sock, SOL_SOCKET, SO_LINGER, ( const char * ) &so_linger, sizeof ( so_linger ) );
		if ( ret < 0 ) {
			CErr ( "CreateThreads: Failed to set SO_LINGER on listener socket." ); LogSocketError ();
			return false;
		}

#ifdef _WIN32
		value = 1;
		ret = setsockopt ( sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, ( const char * ) &value, sizeof ( value ) );
		if ( ret < 0 ) {
			CErr ( "CreateThreads: Failed to set reuseAddr on listener socket." ); LogSocketError ();
			return false;
		}
#else
        value = 0;
        ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &value, sizeof ( value ) );
        if ( ret < 0 ) {
            CErr ( "CreateThreads: Failed to set reuseAddr on listener socket." ); LogSocketError ();
            return false;
        }
        
#ifdef SO_REUSEPORT
        value = 0;
        ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEPORT, ( const char * ) &value, sizeof ( value ) );
        if ( ret < 0 ) {
            CErr ( "CreateThreads: Failed to set reuseAddr on listener socket." ); LogSocketError ();
            return false;
        }
#endif
        
#endif

		/*value = 1;
		ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value) );
		if ( ret < 0 ) {
			CErr ( "CreateThreads: Failed to set reuseAddr on listener socket." ); LogSocketError ();
			return false;
		}*/

		sock = (int) socket ( PF_INET, SOCK_DGRAM, 0 );
		if ( sock < 0 ) {
    		CErrArg ( "CreateThreads: Failed to create udp socket for listener port (%i)!", inst->port );
			return false;
		}
		listener->socketUdp = sock;

		value = 1;
		ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value) );
		if ( ret < 0 ) {
			CErr ( "CreateThreads: Failed to set reuseAddr on listener socket." ); LogSocketError ();
			return false;
		}
        
#ifdef SO_REUSEPORT // Fix for bind error on iOS simulator due to crash of app, leaving the port still bound
        value = 1;
        if ( setsockopt ( sock, SOL_SOCKET, SO_REUSEPORT, (const char *) &value, sizeof (value)) < 0 ) {
            CErr ( "CreateThreads: Failed to set reuseport option on udp listener socket!" );
            return false;
        }
#endif
		MutexLockVA ( thread_lock, "CreateThreads" );

		s = pthread_create ( &inst->threadID, 0, &MediatorDaemon::AcceptorStarter, (void *)listener );
		if ( s != 0 ) {
			CErrArg ( "CreateThreads: Error creating acceptor thread for port %i (pthread_create:%i)", inst->port, s );
			pthread_mutex_unlock ( &thread_lock );
			return false;
		}

		pthread_cond_wait ( &thread_condition, &thread_lock );
		MutexUnlockVA ( thread_lock, "CreateThreads" );

		MutexLockVA ( thread_lock, "CreateThreads" );

		s = pthread_create ( &listener->threadIDUdp, 0, &MediatorDaemon::MediatorUdpThreadStarter, (void *)listener );
		if ( s != 0 ) {
			CErrArg ( "CreateThreads: Error creating udp thread for port %i (pthread_create:%i)", inst->port, s );
			MutexUnlockVA (thread_lock, "CreateThreads" );
			return false;
		}

		pthread_cond_wait ( &thread_condition, &thread_lock );
		MutexUnlockVA ( thread_lock, "CreateThreads" );

		s = pthread_create ( &listener->threadIDWatchdog, 0, &MediatorDaemon::WatchdogThreadStarter, (void *)this );
		if ( s != 0 ) {
			CErrArg ( "CreateThreads: Error creating clients alive status checker port %i (pthread_create:%i)", inst->port, s );
		}
		
		listeners.push_back ( listenerSP );
    }
    
    s = pthread_create ( &notifyThreadID, 0, &MediatorDaemon::NotifyClientsStarter, this );
    if ( s != 0 ) {
        CErrArg ( "CreateThreads: Error creating notifier thread (pthread_create:%i)", s );
    }
	
	Start ();
	
	return true;
}


void MediatorDaemon::Dispose ()
{	
	CLog ( "Dispose" );

	isRunning = false;

	// Wait for each thread to terminate
    ReleaseThreads ();
    
    // There should be no active sessions here.
    // However, in failure cases (e.g. query/lock errors), this may happen.
    // So, we check that and release lingering client sessions
    sessions.Lock ( "Dispose" );
    
    msp ( long long, ThreadInstance )::iterator sessionIt = sessions.list.begin();
    
    vsp ( ThreadInstance ) tmpClients;
    
    while ( sessionIt != sessions.list.end () )
    {
        tmpClients.push_back ( sessionIt->second );
        ++sessionIt;
    }
    
    sessions.list.clear ();
    
    sessions.Unlock ( "Dispose" );
    
    vsp ( ThreadInstance )::iterator sessionItv = tmpClients.begin();
    
    while ( sessionItv != tmpClients.end () )
    {
        sp ( ThreadInstance	)	clientSP = *sessionItv;
        
        if ( clientSP ) {
            ReleaseClient ( clientSP.get () );
        }
        
        ++sessionItv;
    }
    
    tmpClients.clear ();
    
	
	ReleaseDevices ();

    areasMap.Lock ( "Dispose" );
		
	for ( msp ( string, AppsList )::iterator it = areasMap.list.begin(); it != areasMap.list.end(); it++ )
	{
        sp ( AppsList ) appsList = it->second;
		if ( !appsList )
			continue;
		
		appsList->Lock ( "Dispose" );

		for ( msp ( string, ListValues )::iterator ita = appsList->apps.begin(); ita != appsList->apps.end(); ita++ )
		{
            sp ( ListValues ) listValues = ita->second;
			if ( !listValues )
				continue;

			listValues->Lock ( "Dispose" );

			listValues->values.clear ();

			listValues->Unlock ( "Dispose" );

			/*for ( msp ( string, ValuePack )::iterator itv = listValues->values.begin(); itv != listValues->values.end(); ++itv )
			{
				if ( itv->second ) {
                    itv->second = 0;
				}
			}*/
            ita->second = 0;
		}

		appsList->apps.clear ();
        it->second = 0;

		appsList->Unlock ( "Dispose" );
	}
    areasMap.list.clear ();
    
    areasMap.Unlock ( "Dispose" );

	ReleaseDeviceMappings ();
    
    
    for ( map<string, UserItem *>::iterator it = usersDB.begin(); it != usersDB.end(); ++it )
    {
        if ( !it->second ) {
            continue;
        }
        delete ( it->second );
    }
    usersDB.clear ();
}


void MediatorDaemon::ReleaseClient ( ThreadInstance * client )
{
	CVerb ( "ReleaseClient" );

	if ( !client )
		return;

#if !defined(_WIN32) || defined(USE_PTHREADS_FOR_WINDOWS)
	void * res;
#endif
	
	pthread_t thrd = client->threadID;
	pthread_reset ( client->threadID );

#ifdef USE_LOCKFREE_SOCKET_ACCESS
    CloseThreadSocket ( &client->spareSocket );
    
    CloseThreadSocket ( &client->socket );
#else
	client->Lock ( "ReleaseClient" );

	int sock = client->spareSocket;
	if ( sock != -1 ) {
        client->spareSocket = -1;
        CVerbArg ( "ReleaseClient: Closing [ %i ]", sock );

		shutdown ( sock, 2 );
		closesocket ( sock );
	}

	sock = client->socket;
	if ( sock != -1 ) {
        client->socket = -1;
        CVerbArg ( "ReleaseClient: Closing [ %i ]", sock );

		shutdown ( sock, 2 );
		closesocket ( sock );
	}
	
	client->Unlock ( "ReleaseClient" );
#endif
    
	if ( !isRunning ) 
	{		
		client->deviceSP = 0;

		if ( pthread_valid ( thrd ) ) 
		{
			int s = pthread_join ( thrd, &res );
			if ( s != 0 ) {
				CErrArg ( "ReleaseClient: Error waiting for thread (pthread_join:%i)", s );
			}
			pthread_close ( thrd );
		}
        
        client->daemon = 0;
	}
}


bool MediatorDaemon::ReleaseThreads ()
{
	CVerb ( "ReleaseThreads" );

	int s;
	bool ret = true;

#if !defined(_WIN32) || defined(USE_PTHREADS_FOR_WINDOWS)
	void * res;
#endif

	// Signal stop status to threads
	isRunning = false;
	
	Mediator::ReleaseThreads ();

	// Release notifier thread
	pthread_t notifThreadID = notifyThreadID;

    if ( pthread_valid ( notifThreadID ) ) {
		Zero ( notifyThreadID );

        if ( pthread_cond_signal ( &notifyEvent ) ) {
            CErr ( "ReleaseThreads: Error to signal notifier event" );
        }
        
		CVerb ( "ReleaseThreads: Waiting for notifier thread" );

		s = pthread_join ( notifThreadID, &res );
		if ( s != 0 ) {
			CErrArg ( "ReleaseThreads: Error waiting for notifier thread (pthread_join:%i)", s );
			ret = false;
		}
		pthread_close ( notifThreadID );
	}
    
    while ( !notifyQueue.empty () )
    {
        NotifyQueueContext * ctx = notifyQueue.front ();
        notifyQueue.pop ();
        if ( ctx )
            delete ctx;
    }

	for ( unsigned int pos = 0; pos < listeners.size(); pos++ ) {
		sp ( MediatorThreadInstance ) listener = listeners [ pos ];
		if ( !listener )
			continue;

		ThreadInstance * inst = &listener->instance;
		int sock = (int) inst->socket;
		if ( sock != -1 ) {
            inst->socket = -1;
            CVerbArg ( "ReleaseThreads: Closing [ %i ]", sock );

			shutdown ( sock, 2 );
			closesocket ( sock );
		}

		sock = listener->socketUdp;
		if ( sock != -1 ) {
            listener->socketUdp = -1;
            CVerbArg ( "ReleaseThreads: Closing [ %i ]", sock );

			shutdown ( sock, 2 );
			closesocket ( sock );
		}
		
		if ( pthread_valid ( inst->threadID ) ) {
			s = pthread_join ( inst->threadID, &res );
			if ( s != 0 ) {
				CErrArg ( "ReleaseThreads: Error waiting for listener thread (pthread_join:%i)", s );
				ret = false;
			}
			pthread_close ( inst->threadID );
		}
        
		if ( pthread_valid ( listener->threadIDUdp ) ) {
			s = pthread_join ( listener->threadIDUdp, &res );
			if ( s != 0 ) {
				CErrArg ( "ReleaseThreads: Error waiting for udp thread (pthread_join:%i)", s );
				ret = false;
			}
			pthread_close ( listener->threadIDUdp );
		}
        
		if ( pthread_valid ( listener->threadIDWatchdog ) ) {
			if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
				CErr ( "ReleaseThreads: Error to signal watchdog event" );
			}

			s = pthread_join ( listener->threadIDWatchdog, &res );
			if ( s != 0 ) {
				CErrArg ( "ReleaseThreads: Error waiting for clients alive status checker thread (pthread_join:%i)", s );
				ret = false;
			}
			pthread_close ( listener->threadIDWatchdog );
		}
	}
	listeners.clear();
		

    acceptClients.Lock ( "ReleaseThreads" );

	while ( acceptClients.list.size() > 0 )
	{
		sp ( ThreadInstance ) first = acceptClients.list [ 0 ];

		acceptClients.list.erase ( acceptClients.list.begin() );
        
        acceptClients.Unlock ( "ReleaseThreads" );

		ReleaseClient ( first.get () );
        
        acceptClients.Lock ( "ReleaseThreads" );
	}
    
    acceptClients.Unlock ( "ReleaseThreads" );

	return ret;
}


void MediatorDaemon::ReleaseDevices ()
{
	CLog ( "ReleaseDevices" );

	vsp ( AreaApps ) tmpAreas;

	areas.Lock ( "ReleaseDevices" );

	for ( msp ( string, AreaApps )::iterator it = areas.list.begin(); it != areas.list.end(); ++it )
	{
		if ( it->second )
			tmpAreas.push_back ( it->second );
	}

	areas.list.clear ();

	areas.Unlock ( "ReleaseDevices" );


	for ( vsp ( AreaApps )::iterator it = tmpAreas.begin (); it != tmpAreas.end (); ++it )
	{
		sp ( AreaApps ) areaApps = *it;
		if ( !areaApps )
			continue;

		areaApps->Lock ( "ReleaseDevices" );

		vsp ( ApplicationDevices ) tmpAppDevices;

		for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin (); ita != areaApps->apps.end (); ++ita )
		{
			sp ( ApplicationDevices ) appDevices = ita->second;
			if ( ita->second )
				tmpAppDevices.push_back ( appDevices );
		}

		areaApps->apps.clear ();
		areaApps->Unlock ( "ReleaseDevices" );

		for ( vsp ( ApplicationDevices )::iterator ita = tmpAppDevices.begin (); ita != tmpAppDevices.end (); ++ita )
		{
			sp ( ApplicationDevices ) appDevices = *ita;
			if ( appDevices )
			{
				appDevices->Lock ( "ReleaseDevices" );

				vsp ( DeviceInstanceNode ) devSPs;

				DeviceInstanceNode * device = appDevices->devices;

				while ( device ) {
					if ( device->baseSP )
						devSPs.push_back ( device->baseSP );

					//DeviceInstanceNode * toDispose = device;
					device = device->next;
				}

				appDevices->devices = 0;

				appDevices->Unlock ( "ReleaseDevices" );

				for ( vsp ( DeviceInstanceNode )::iterator itd = devSPs.begin (); itd != devSPs.end (); ++itd )
				{
					sp ( DeviceInstanceNode ) devSP = *itd;

					CVerbArg ( "[0x%X].ReleaseDevices: deleting memory occupied by client", devSP->info.deviceID );

					ReleaseClient ( devSP->clientSP.get () );

					devSP->baseSP = 0;
				}

				devSPs.clear ();
			}
		}

		tmpAppDevices.clear ();
	}

	tmpAreas.clear ();
}


void MediatorDaemon::ReleaseDeviceMappings ()
{
    CVerb ( "ReleaseDeviceMappings" );
    
    deviceMappings.Lock ( "ReleaseDeviceMappings" );
	
	for ( msp ( string, DeviceMapping )::iterator it = deviceMappings.list.begin(); it != deviceMappings.list.end(); ++it )
	{
		if ( !it->second ) {
			continue;
		}
		it->second = 0;
	}
    deviceMappings.list.clear ();
    
    deviceMappings.Unlock ( "ReleaseDeviceMappings" );
}


void MediatorDaemon::RemoveDevice ( int deviceID, const char * areaName, const char * appName )
{
	CVerbID ( "RemoveDevice from maps" );

	if ( !areaName || !appName ) {
		CErrID ( "RemoveDevice: Called with NULL argument for areaName or appName?!" );
		return;
	}

	bool					found	= false;
	DeviceInstanceNode	*	device	= 0;
	
	string appsName ( appName );
	sp ( ApplicationDevices )			appDevices = 0;

	string pareaName ( areaName );
	sp ( AreaApps )						areaApps	= 0;

	if ( !areas.Lock ( "RemoveDevice" ) )
		return;

    const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( pareaName );
	if ( areaIt != areas.list.end ( ) )
		areaApps = areaIt->second;

	if ( !areas.Unlock ( "RemoveDevice" ) )
		return;
    
	if ( !areaApps || !areaApps->Lock ( "RemoveDevice" ) ) {
		CWarnArgID ( "RemoveDevice: areaName [%s] not found.", areaName );
		return;
	}

	const msp ( string, ApplicationDevices )::iterator appsIt = areaApps->apps.find ( appsName );
	if ( appsIt != areaApps->apps.end ( ) )
		appDevices = appsIt->second;

	areaApps->Unlock ( "RemoveDevice" );

	if ( !appDevices || !appDevices->Lock ( "RemoveDevice" ) ) {
		CErrArg ( "RemoveDevice: appName [%s] not found.", appName );
		return;
	}
	
	device = appDevices->devices;	

	while ( device ) 
	{
		if ( device->info.deviceID == deviceID ) {	
			found = true;
			break;
		}

		if ( !device->next )
			break;
		device = device->next;
	}

	if ( found && device )
		RemoveDevice ( device, false );

	appDevices->Unlock ( "RemoveDevice" );
}


void MediatorDaemon::RemoveDevice ( DeviceInstanceNode * device, bool useLock )
{
	CVerbVerb ( "RemoveDevice" );

	if ( !device )
		return;

	sp ( ApplicationDevices ) appDevices = device->rootSP;
    if ( !appDevices )
        return;

	ApplicationDevices * appDevs = appDevices.get ();
    if ( !appDevs )
        return;

	if ( useLock )
		appDevs->Lock ( "RemoveDevice" );

	if ( device == appDevs->devices ) {
		if ( device->next ) {
			CVerbArg ( "RemoveDevice: relocating client [0x%X] to root of list", device->next->info.deviceID );
            
            device->prev = 0;
			appDevs->devices = device->next;
            
            device->next = 0;
			appDevs->count--;
		}
		else {
			appDevs->devices = 0;
			appDevs->count = 0;
		}
	}
	else {
		if ( !device->prev ) {
			CErrArg ( "RemoveDevice: Serious inconsistency error!!! Failed to lookup device list. Missing previous device for ID [0x%X]", device->info.deviceID );

			/// Best thing we can do is to assume that we are the root node of the list
			if ( device->next ) {
				CVerbArg ( "RemoveDevice: Relocating client [0x%X] to root of list", device->next->info.deviceID );

				device->next->prev = 0;
				appDevs->devices = device->next;
				appDevs->count--;
			}
			else {
				appDevs->devices = 0;
				appDevs->count = 0;
			}
		}
		else {
			if ( device->next ) {
				CVerbArg ( "RemoveDevice: Relinking client [0x%X] to previous client [0x%X]", device->next->info.deviceID, device->prev->info.deviceID );
				device->prev->next = device->next;
                device->next->prev = device->prev;
			}
			else {
				CVerbArg ( "RemoveDevice: Finish list as the client [0x%X] was the last one", device->prev->info.deviceID );
				device->prev->next = 0;
            }
            
            device->next = 0;

			appDevs->count--;

		}
	}

	if ( useLock)
		appDevs->Unlock ( "RemoveDevice" );

	NotifyClients ( NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED, device->baseSP );
    
    
    CVerbArg ( "RemoveDevice: Disposing device [0x%X]", device->info.deviceID );
	device->baseSP = 0;
}


void MediatorDaemon::RemoveDevice ( unsigned int ip, char * msg )
{
	CVerb ( "RemoveDevice from broadcast" );

	if ( !isRunning || !msg )
		return;

	unsigned int deviceID;
	char * areaName = 0;
	char * appName = 0;

	// Get the id at first (>0)	
	unsigned int * pUIntBuffer = (unsigned int *) (msg + 12);
	deviceID = *pUIntBuffer;
	if ( !deviceID )
		return;
	
	// Get the areaName, appname, etc..
	char * context = NULL;
	char * psem = strtok_s ( msg + MEDIATOR_BROADCAST_DESC_START, ";", &context );
	if ( !psem )
		return;
	
	psem = strtok_s ( NULL, ";", &context );
	if ( !psem )
		return;
	areaName = psem;
	
	psem = strtok_s ( NULL, ";", &context );
	if ( !psem )
		return;
	appName = psem;
	
	CVerbID ( "RemoveDevice BC:" );

	RemoveDevice ( deviceID, areaName, appName );
}


void MediatorDaemon::UpdateDeviceInstance ( sp ( DeviceInstanceNode ) device, bool added, bool changed )
{
	if ( added )
		NotifyClients ( NOTIFY_MEDIATOR_SRV_DEVICE_ADDED, device );
	else if ( changed )
		NotifyClients ( NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED, device );
}


/// 0 no filter
/// 1 show all within the same area
/// 2 show only within same area and apps

void MediatorDaemon::UpdateNotifyTargets ( const sp ( ThreadInstance ) &clientSP, int filterMode )
{	
	if ( !clientSP )
		return;
		
    DeviceInstanceNode * device;
    ThreadInstance * client = clientSP.get ();
    
    sp ( DeviceInstanceNode ) deviceSP = client->deviceSP;
    if ( !deviceSP )
		return;
    
    device = deviceSP.get ( );

    /// Check whether we need to remove the client from a targetlist
    if ( client->filterMode != filterMode )
    {
        if ( client->filterMode == MEDIATOR_FILTER_NONE )
        {
            /// Remove from NoRestrict notifiers
            CVerbArg ( "UpdateNotifyTargets: Looking for deviceID [0x%X / %s / %s] in NONE-filter", client->deviceID, device ? device->info.areaName : "", device ? device->info.appName : "" );

			if ( MutexLockA ( notifyTargetsLock, "UpdateNotifyTargets" ) )
			{
				const msp ( long long, ThreadInstance )::iterator notifyIt = notifyTargets.find ( client->sessionID );

				if ( notifyIt != notifyTargets.end () )
				{
					CVerbArg ( "UpdateNotifyTargets: Removing deviceID [0x%X / %s / %s] from NONE-filter", client->deviceID, device ? device->info.areaName : "", device ? device->info.appName : "" );

					notifyTargets.erase ( notifyIt );
				}

				MutexUnlockVA ( notifyTargetsLock, "UpdateNotifyTargets" );
			}
        }
        else if ( client->filterMode == MEDIATOR_FILTER_AREA ) 
		{            
            while ( device && device->rootSP )
            {
                /// Search for area
                const map<unsigned int, string>::iterator areaIdsIt = areaIDs.find ( device->rootSP->areaId );
                
                if ( areaIdsIt == areaIDs.end () ) {
                    CWarnArg ( "UpdateNotifyTargets: Failed to find area ID [%u]", device->rootSP->areaId );
                    break;
                }

				sp ( AreaApps ) areaApps = 0;

				areas.Lock ( "UpdateNotifyTargets" );

                const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( areaIdsIt->second );

				if ( areaIt == areas.list.end () ) {
					CWarnArg ( "UpdateNotifyTargets: Failed to find area [%s]", areaIdsIt->second.c_str () );
				}
				else 
					areaApps = areaIt->second;

				areas.Unlock ( "UpdateNotifyTargets" );

                if ( areaApps && areaApps->Lock1( "UpdateNotifyTargets" ) ) {
                    CVerbArg ( "UpdateNotifyTargets: Looking for deviceID [0x%X / %s / %s] in Area-filter", client->deviceID, device->info.areaName, device->info.appName );
                    
                    const msp ( long long, ThreadInstance )::iterator notifyIt = areaApps->notifyTargets.find ( client->sessionID );
                    
                    if ( notifyIt != areaApps->notifyTargets.end () )
                    {
                        CVerbArg ( "UpdateNotifyTargets: Removing deviceID [0x%X / %s / %s] from Area-filter", client->deviceID, device->info.areaName, device->info.appName );
                        
                        areaApps->notifyTargets.erase ( notifyIt );
                    }

					areaApps->Unlock1 ( "UpdateNotifyTargets" );
                }
                break;
            }
        }
    }
    
    client->filterMode = (short) filterMode;
    
    if ( filterMode < MEDIATOR_FILTER_NONE )
        return;
    
	if ( filterMode == MEDIATOR_FILTER_ALL ) {
		return;
	}
    
    if ( filterMode == MEDIATOR_FILTER_NONE )
    {
		CVerbArg ( "UpdateNotifyTargets: Adding deviceID [0x%X / %s / %s] to NoRestrict-targets", client->deviceID, device ? device->info.areaName : "", device ? device->info.appName : "" );

		if ( MutexLockA ( notifyTargetsLock, "UpdateNotifyTargets" ) )
		{
			notifyTargets [ client->sessionID ] = clientSP;

			MutexUnlockVA ( notifyTargetsLock, "UpdateNotifyTargets" );
		}
    }
    else if ( filterMode == MEDIATOR_FILTER_AREA ) 
	{
        while ( device && device->rootSP )
        {
            /// Search for area
            const map<unsigned int, string>::iterator areaIDsIt = areaIDs.find ( device->rootSP->areaId );
            
            if ( areaIDsIt == areaIDs.end () ) {
                CWarnArg ( "UpdateNotifyTargets: Failed to find area ID [%u]", device->rootSP->areaId );
                break;
            }

			sp ( AreaApps ) areaApps = 0;

			areas.Lock ( "UpdateNotifyTargets" );

            const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( areaIDsIt->second );
            
            if ( areaIt == areas.list.end () ){
                CWarnArg ( "UpdateNotifyTargets: Failed to find area [%s]", areaIDsIt->second.c_str () );
            }
			else
				areaApps = areaIt->second;

			areas.Unlock ( "UpdateNotifyTargets" );
            
			if ( areaApps && areaApps->Lock1 ( "UpdateNotifyTargets" ) ) {
				CVerbArg ( "UpdateNotifyTargets: Adding deviceID [0x%X / %s / %s] to Area-filter", client->deviceID, device->info.areaName, device->info.appName );

				areaApps->notifyTargets [ client->sessionID ] = clientSP;

				areaApps->Unlock1 ( "UpdateNotifyTargets" );
			}
            break;
        }
    }
    
//Finish:
}


sp ( ApplicationDevices ) MediatorDaemon::GetDeviceList ( char * areaName, char * appName, pthread_mutex_t ** lock, int ** pDevicesAvailable, DeviceInstanceNode ** &list )
{
	sp ( ApplicationDevices	)	appDevices	= 0;
    sp ( AreaApps )				areaApps	= 0;
    sp ( AppsList )				appsList	= 0;
    
	string appsName ( appName );
	string pareaName ( areaName );

	areas.Lock ( "GetDeviceList" );

    const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( pareaName );

	if ( areaIt == areas.list.end ( ) ) {
		/// Create a new one...
		areaApps = make_shared < AreaApps > ();
		if ( areaApps && areaApps->Init () && areaApps->Init1 () ) {
			areasCounter++;
			areaIDs [ areasCounter ] = pareaName;
			
			areaApps->id = areasCounter;
			areaApps->apps.clear ();
            areaApps->notifyTargets.clear ();

			areas.list [ pareaName ] = areaApps;
		}
		else { CErrArg ( "GetDeviceList: Failed to create new area [%s] Low memory problem?!", areaName ); }
	}
	else
		areaApps = areaIt->second;

	areas.Unlock ( "GetDeviceList" );


	if ( !areaApps || !areaApps->Lock ( "GetDeviceList" ) ) {
		CLogArg ( "GetDeviceList: App [%s] not found.", appName );
		return 0;
	}
		
	const msp ( string, ApplicationDevices )::iterator appsIt = areaApps->apps.find ( appsName );
				
	if ( appsIt == areaApps->apps.end ( ) )
    {
		/// Create a new one...
		appDevices = make_shared < ApplicationDevices > ();
        
		if ( appDevices && appDevices->Init () ) 
		{			
			appsCounter++;
			appDevices->id		= appsCounter;
			appDevices->areaId	= areaApps->id;
			appIDs [ appsCounter ] = appsName;

            // Query maxIDs from value packs
			if ( areasMap.Lock ( "GetDeviceList" ) )
			{
				const msp ( string, AppsList )::iterator areaVPIt = areasMap.list.find ( pareaName );
				if ( areaVPIt != areasMap.list.end () )
                    appsList = areaVPIt->second;
                else
                {
                    // Create a new map for the area
                    appsList = make_shared < AppsList > ();
                    if ( !appsList || !appsList->Init () ) {
                        CErrArg ( "GetDeviceList: Failed to create new area [%s].", pareaName.c_str () );
                        
                        appsList.reset ();
                    }
                    else {
                        areasMap.list [ pareaName ] = appsList;
                    }
                }
                
				if ( areasMap.Unlock ( "GetDeviceList" ) && appsList && appsList->Lock ( "GetDeviceList" ) )
				{
					sp ( ListValues ) listValues = 0;

					const msp ( string, ListValues )::iterator appsVPIt = appsList->apps.find ( appsName );
					if ( appsVPIt != appsList->apps.end () )
                        listValues = appsVPIt->second;
                    else
                    {
                        listValues = make_shared < ListValues > (); // new map<string, ValuePack*> ();
                        if ( !listValues || !listValues->Init () ) {
                            CErrArg ( "GetDeviceList: Failed to create new application [%s].", appsName.c_str () );
                            
                            listValues.reset ();
                        }
                        else {
                            appsList->apps [ appName ] = listValues;
                        }
                    }

					if ( appsList->Unlock ( "GetDeviceList" ) )
                    {
                        if ( listValues && listValues->Lock ( "GetDeviceList" ) )
                        {
                            const msp ( string, ValuePack )::iterator valueIt = listValues->values.find ( string ( "0_maxID" ) );
                            
                            if ( valueIt != listValues->values.end () )
                            {
                                sp ( ValuePack ) value = valueIt->second;
                                if ( value )
                                    appDevices->latestAssignedID = value->size;
                            }
                            else {
                                // Create a new one
                                string value = "1";
                                
                                addToArea ( listValues, "0_maxID", value.c_str (), (unsigned int) value.length () );
                            }
                            
                            listValues->Unlock ( "GetDeviceList" );
                        }
                        
                        areaApps->apps [ appsName ] = appDevices;
                    }
				}
			}
		}
		else { CErrArg ( "GetDeviceList: Failed to create new application devicelist [%s].", appName ); }
	}
	else
		appDevices = appsIt->second;
	
	areaApps->Unlock ( "GetDeviceList" );

	if ( !appDevices || appDevices->access <= 0 )
		goto Finish;
	
	__sync_add_and_fetch ( &appDevices->access, 1 );

	if ( lock )
		*lock = &appDevices->lock;
	if ( pDevicesAvailable )
		*pDevicesAvailable = &appDevices->count;

	list = &appDevices->devices;

Finish:

	return appDevices;
}

bool printStdOut    = true;


void printDevice ( DeviceInstanceNode * device )
{
    CLogArg ( "\nDevice      = [ 0x%X : %s : %s ]", device->info.deviceID, device->info.deviceName, device->info.broadcastFound ? "on same network" : "by mediator" );
    if ( printStdOut )
        printf ( "\nDevice      = [0x%X : %s : %s ]\n", device->info.deviceID, device->info.deviceName, device->info.broadcastFound ? "on same network" : "by mediator" );
    CLogArg ( "UID         = [ %d : %s]", device->clientSP ? device->clientSP->authLevel : -1, device->clientSP ? device->clientSP->uid : ".-." );
    if ( printStdOut )
        printf ( "UID         = [ %d : %s ]\n", device->clientSP ? device->clientSP->authLevel : -1, device->clientSP ? device->clientSP->uid : ".-." );
    if ( device->info.ip != device->info.ipe ) {
        CLogArg ( "Device IPe != IP [ %s ]", inet_ntoa ( *((struct in_addr *) &device->info.ip) ) );
        if ( printStdOut )
            printf ( "Device IPe != IP [ %s ]\n", inet_ntoa ( *((struct in_addr *) &device->info.ip) ) );
    }
    CLogArg ( "Area/App    = [ %s : %s ]", device->info.areaName, device->info.appName );
    if ( printStdOut )
        printf ( "Area/App    = [ %s : %s ]\n", device->info.areaName, device->info.appName );
    CLogArg ( "Device  IPe = [ %s (from socket) [ tcp %d ]  [ udp %d ]]", inet_ntoa ( *((struct in_addr *) &device->info.ipe) ), device->info.tcpPort, device->info.udpPort );
    if ( printStdOut )
        printf ( "Device  IPe = [ %s (from socket) [ tcp %d ]  [ udp %d ]]\n", inet_ntoa ( *((struct in_addr *) &device->info.ipe) ), device->info.tcpPort, device->info.udpPort );
}


void printDeviceList ( DeviceInstanceNode * device )
{
	while ( device ) 
	{
        printDevice ( device );
        
		device = device->next;
	}
}


void MediatorDaemon::PrintSmallHelp ()
{
    printf ( "Press ESC to quit; h for help.\n" );
}


void MediatorDaemon::PrintHelp ()
{
    printf ( "-------------------------------------------------------\n" );
    printf ( "a - add/update user\n" );
    printf ( "b - send broadcast packet to clients\n" );
    printf ( "c - print configuration\n" );
    printf ( "d - print database\n" );
    printf ( "e - error case - reset acceptor\n" );
    printf ( "g - print client resources\n" );
    printf ( "h - print help\n" );
    printf ( "i - print interfaces\n" );
    printf ( "j - toggle Acceptor\n" );
    printf ( "l - toggle logging to file\n" );
    printf ( "m - print mediators\n" );
    printf ( "o - toggle logging to output (std)\n" );
    printf ( "p - print active devices\n" );
    printf ( "r - reload pkcs keys\n" );
    printf ( "s - toggle output to stdout\n" );
    printf ( "t - toggle authentication\n" );
    printf ( "z - clear bann list\n" );
    printf ( "-------------------------------------------------------\n" );
}


void MediatorDaemon::Run ()
{
	CVerb ( "Run" );

	isRunning = true;
	//static int keyCount = 0;
	//static int valueCount = 0;

	CLog ( "Service is running..." );
	
	PrintSmallHelp ();
    printf ( "-------------------------------------------------------\n" );

	char command = 0;
	bool hideInput = false;
	string userName;
	string passPhrase;
    int accessLevel = 0;
	
	do
	{
#if defined(SLEEPRUNNER)	
		Sleep ( 10000 );
#else
		int c = _getch();
        //printf ( "-------------------------------------------------------\n" );
		//CLog ( "Mediator exit requested!" );
		if ( c == 27 ) {
            printf ( "-------------------------------------------------------\n" );
			CLog ( "Mediator exit requested!" );

			// Stop the threads
			break;
		}
        //printf ( "c [%i]\n", c );
        
		if ( c == 8 || c == 127 ) {
			if ( input >= inputBuffer ) {
				if ( input > inputBuffer )
					input--;
				*input = 0;
                
                CLog ( "" );
                CLogArg ( "%s", inputBuffer );
                if ( printStdOut ) {
                    printf ( "\r\n" );
                    printf ( "%s", inputBuffer );
                }
			}
			continue;
		}
		
		if ( input == inputBuffer && !command ) {
			if ( c == 'a' ) {
                CLog ( "Add new user:" );
                CLog ( "----------------------------------------------------------------" );
                if ( printStdOut ) {
                    printf ( "Add new user:\n" );
                    printf ( "----------------------------------------------------------------\n" );
                }
                CLog ( "Please enter an access level (0 - 10). Default [3]:" );
                if ( printStdOut )
                    printf ( "Please enter an access level (0 - 10). Default [3]: " );
				command = 'a';
				accessLevel = -1;
				userName = "a";
				continue;
			}
			else if ( c == 'b' ) {
				SendBroadcast ();
				continue;
			}
            else if ( c == 'c' ) {
                CLog ( "Configuration:" );
                CLog ( "----------------------------------------------------------------" );
                CLog ( "Ports: " );
                if ( printStdOut ) {
                    printf ( "Configuration:\n" );
                    printf ( "----------------------------------------------------------------\n" );
                    printf ( "Ports:\n " );
                }
				for ( unsigned int pos = 0; pos < ports.size (); pos++ ) {
                    CLogArg ( " %i", ports [ pos ] );
                    if ( printStdOut )
                        printf ( " %i\n", ports [ pos ] );
				}
                //printf ( "\n" );
                CLog ( "----------------------------------------------------------------" );
                if ( printStdOut )
                    printf ( "----------------------------------------------------------------\n" );
				continue;
			}
			else if ( c == 'd' ) {
                CLog ( "Database:" );
                CLog ( "----------------------------------------------------------------" );
                if ( printStdOut ) {
                    printf ( "Database:\n" );
                    printf ( "----------------------------------------------------------------\n" );
                }
				if ( areasMap.Lock ( "Run" ) )
				{
					for ( msp ( string, AppsList )::iterator it = areasMap.list.begin (); it != areasMap.list.end (); it++ )
					{
						sp ( AppsList ) appsList = it->second;
						if ( !appsList || !appsList->Lock ( "Run" ) )
							continue;

						CLogArg ( "Area: %s", it->first.c_str () );

						for ( msp ( string, ListValues )::iterator ita = appsList->apps.begin (); ita != appsList->apps.end (); ita++ )
						{
							sp ( ListValues ) values = ita->second;
							if ( !values || !values->Lock ( "Run" ) )
								continue;

							CLogArg ( "App: %s", ita->first.c_str () );

							for ( msp ( string, ValuePack )::iterator itv = values->values.begin (); itv != values->values.end (); ++itv )
							{
								sp ( ValuePack ) value = itv->second;
								if ( value ) {
									CLogArg ( "\tKey: %s\tValue: %s", itv->first.c_str (), value->value.c_str () );
								}
							}
							values->Unlock ( "Run" );
						}

						appsList->Unlock ( "Run" );
					}

					areasMap.Unlock ( "Run" );
				}
				printf ( "----------------------------------------------------------------\n" );
				if ( MutexLockA ( usersDBLock, "Run" ) )
				{
					for ( map<string, UserItem *>::iterator it = usersDB.begin (); it != usersDB.end (); ++it )
					{
						if ( it->second )
						{
							UserItem * item = it->second;

							const char * pwStr = ConvertToHexString ( item->pass.c_str (), ENVIRONS_USER_PASSWORD_LENGTH );
							if ( pwStr ) {
								CLogArg ( "User: %s\t[%s]", it->first.c_str (), pwStr );

							}
						}
					}

					MutexUnlockVA ( usersDBLock, "Run" );
				}
				printf ( "----------------------------------------------------------------\n" );
				continue;
			}
			else if ( c == 'e' ) {
				CLog ( "Reseting acceptor thread ..." );
                if ( printStdOut )
                    printf ( "Reseting acceptor thread ...\n" );
				ReCreateAcceptor ();
				continue;
			}
			else if ( c == 'g' ) {
				CLog ( "Active clients:" );
				CLog ( "----------------------------------------------------------------" );
                
                if ( printStdOut ) {
                    printf ( "Active clients:\n" );
                    printf ( "----------------------------------------------------------------\n" );
                }
                
				if ( acceptClients.Lock ( "Run" ) )
				{
					for ( size_t i = 0; i < acceptClients.list.size (); i++ )
					{
						sp ( ThreadInstance ) client = acceptClients.list [ i ];
						if ( client )
						{
							sp ( DeviceInstanceNode ) deviceSP = client->deviceSP;
							if ( deviceSP )
							{
								CLogArg ( "[%i]: [%i]", i, client->deviceID );

                                if ( printStdOut )
                                    printf ( "[%zu]: [%i]\n", i, client->deviceID );
                                
								printDevice ( deviceSP.get () );
							}
						}
                        else {
                            CErrArg ( "[%i]: **** Invalid Client\n\n", i );
                            if ( printStdOut )
                                printf ( "[%zu]: **** Invalid Client\n\n", i );
						}
						CLog ( "----------------------------------------------------------------" );
                        if ( printStdOut )
                            printf ( "----------------------------------------------------------------\n" );
					}
					acceptClients.Unlock ( "Run" );
				}
                CLog ( "----------------------------------------------------------------" );
                if ( printStdOut )
                    printf ( "----------------------------------------------------------------\n" );
				continue;
			}
			else if ( c == 'h' ) { // Help requested
				PrintHelp ();
				continue;
			}
			else if ( c == 'i' ) {
				if ( !MutexLock ( &localNetsLock, "Run" ) )
					continue;

				NetPack * net = &localNets;
				while ( net ) {
					CLogArg ( "Interface ip:%s", inet_ntoa ( *( ( struct in_addr * ) &net->ip ) ) );
					CLogArg ( "Interface bcast:%s", inet_ntoa ( *( ( struct in_addr * ) &net->bcast ) ) );
                    
                    if ( printStdOut ) {
                        printf ( "Interface ip:%s\n", inet_ntoa ( *( ( struct in_addr * ) &net->ip ) ) );
                        printf ( "Interface bcast:%s\n", inet_ntoa ( *( ( struct in_addr * ) &net->bcast ) ) );
                    }
					net = net->next;
				}

				MutexUnlockV ( &localNetsLock, "Run" );
				continue;
            }
            else if ( c == 'j' ) {
                acceptEnabled = !acceptEnabled;
                if ( printStdOut )
                    printf ( "Run: Acceptor is now [%s]\n", acceptEnabled ? "enabled" : "disabled" );
                CLogArg ( "Run: Acceptor is now [%s]", acceptEnabled ? "enabled" : "disabled" );
                continue;
            }
			else if ( c == 'l' ) {
                logging = !logging;
                if ( printStdOut )
                    printf ( "Run: File logging is now [%s]\n", logging ? "enabled" : "disabled" );
				CLogArg ( "Run: File logging is now [%s]", logging ? "enabled" : "disabled" );
				continue;
			}
			else if ( c == 'm' ) {
				if ( !mediator.ip ) {
					CLog ( "No more mediators known yet!" );
                    if ( printStdOut )
                        printf ( "No more mediators known yet!\n" );
					continue;
				}
				else {
					if ( !MutexLock ( &mediatorLock, "Run" ) )
						continue;

					CLog ( "Mediators:" );
                    CLog ( "----------------------------------------------------------------" );
                    if ( printStdOut ) {
                        printf ( "Mediators:\n" );
                        printf ( "----------------------------------------------------------------\n" );
                    }

					MediatorInstance * net = &mediator;

					while ( net ) {
                        CLogArg ( "\tIP: %s\tPorts: ", inet_ntoa ( *( ( struct in_addr * ) &net->ip ) ) );
                        if ( printStdOut )
                            printf ( "\tIP: %s\tPorts: \n", inet_ntoa ( *( ( struct in_addr * ) &net->ip ) ) );

						for ( int i=0; i<MAX_MEDIATOR_PORTS; i++ ) {
							if ( !net->port )
								break;
                            CLogArg ( "%i ", net->port );
                            if ( printStdOut )
                                printf ( "%i \n", net->port );
						}
                        CLog ( "\n" );
                        if ( printStdOut )
                            printf ( "\n" );
						net = net->next;
                    }
                    CLog ( "----------------------------------------------------------------" );
                    if ( printStdOut )
                        printf ( "----------------------------------------------------------------\n" );

					MutexUnlockV ( &mediatorLock, "Run" );
				}
				continue;
			}
			else if ( c == 'o' ) {
                stdlog = !stdlog;
                if ( printStdOut )
                    printf ( "Run: Std output logging is now [%s]\n", stdlog ? "enabled" : "disabled" );
				CLogArg ( "Run: Std output logging is now [%s]", stdlog ? "enabled" : "disabled" );
				continue;
			}
			else if ( c == 'p' ) {
				CLog ( "Active devices:" );
                CLog ( "----------------------------------------------------------------" );
                if ( printStdOut ) {
                    printf ( "Active devices:\n" );
                    printf ( "----------------------------------------------------------------\n" );
                }

				if ( areas.Lock ( "Run" ) )
				{
					for ( msp ( string, AreaApps )::iterator it = areas.list.begin(); it != areas.list.end(); ++it )
					{
						if ( it->second ) {
							sp ( AreaApps ) areaApps = it->second;
							CLogArg ( "P: [%s]", it->first.c_str () );
                            if ( printStdOut )
                                printf ( "P: [%s]\n", it->first.c_str () );

							if ( !areaApps || !areaApps->Lock ( "Run" ) )
								continue;

							for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin(); ita != areaApps->apps.end(); ++ita )
							{
								sp ( ApplicationDevices ) appDevices = ita->second;
								if ( appDevices ) 
								{					
									if ( appDevices->devices ) 
                                    {
                                        CLog ( "----------------------------------------------------------------" );
                                        CLogArg ( "A: [%s]", ita->first.c_str () );
                                        if ( printStdOut ) {
                                            printf ( "----------------------------------------------------------------\n" );
                                            printf ( "A: [%s]\n", ita->first.c_str () );
                                        }

										// Deadlock here
										if ( appDevices->Lock ( "Run" ) )
										{
											printDeviceList ( appDevices->devices );

											appDevices->Unlock ( "Run" );
										}
                                        CLog ( "----------------------------------------------------------------" );
                                        if ( printStdOut )
                                            printf ( "----------------------------------------------------------------\n" );
									}
                                    else {
                                        CLogArg ( "A: No devices in [%s]", ita->first.c_str () );
                                        if ( printStdOut )
                                            printf ( "A: No devices in [%s]\n", ita->first.c_str () );
									}
								}
							}

							areaApps->Unlock ( "Run" );
						}
					}

					areas.Unlock ( "Run" );
				}
				CLog ( "----------------------------------------------------------------" );
                if ( printStdOut )
                    printf ( "----------------------------------------------------------------\n" );
				continue;
            }
			else if ( c == 'q' ) {
				printf ( "-------------------------------------------------------\n" );
				CLog ( "Mediator exit requested!" );

				// Stop the threads
				break;
			}
            else if ( c == 'r' ) {
                if ( printStdOut )
                    printf ( "Run: Reloading keys\n" );
				CLog ( "Run: Reloading keys" );
				LoadKeys ();
				continue;
			}
			else if ( c == 's' ) {
                printStdOut = !printStdOut;
                if ( printStdOut )
                    printf ( "Run: Stdout is now [%s]\n", printStdOut ? "required" : "disabled" );
				CLogArg ( "Run: Stdout is now [%s]", printStdOut ? "required" : "disabled" );
				continue;
            }
            else if ( c == 't' ) {
                reqAuth = !reqAuth;
                if ( printStdOut )
                    printf ( "Run: Authentication is now [%s]\n", reqAuth ? "required" : "disabled" );
                CLogArg ( "Run: Authentication is now [%s]", reqAuth ? "required" : "disabled" );
                continue;
            }
			else if ( c == 'z' ) {
                if ( MutexLockA ( bannedIPsLock, "Run" ) ) {
                    if ( printStdOut )
                        printf ( "Run: Clearing bannlist with [%u] entries.\n", ( unsigned int ) bannedIPs.size () );
					CLogArg ( "Run: Clearing bannlist with [%u] entries.", ( unsigned int ) bannedIPs.size () );
					bannedIPs.clear ();
					bannedIPConnects.clear ();

					MutexUnlockVA ( bannedIPsLock, "Run" );
				}
				continue;
			}
		}
		if (c == 13 || c == 10 ) { // Return key
			// Close input string
			*input = 0;

			// Analyse input string
			if ( command == 'a' ) {
                if ( accessLevel < 0 ) {
                    if ( sscanf_s ( inputBuffer, "%d", &accessLevel ) <= 0 || accessLevel < 0 || accessLevel > 10 )
                        accessLevel = 3;
                    input = inputBuffer;
                    
                    CLog ( "Please enter the username (email):" );
                    if ( printStdOut )
                        printf ( "\nPlease enter the username (email): " );
                }
				else if ( userName.c_str()[0] == 'a' && userName.length() == 1 ) {
					userName = inputBuffer;
					if ( userName.find ( '@', 0 ) == string::npos ) {
						command = 0;
						userName = "";
                        CLog ( "The username must be a valid email address!" );
                        if ( printStdOut )
                            printf ( "The username must be a valid email address!\n" );
					}
					else {
                        CLog ( "Please enter the passphrase:" );
                        if ( printStdOut )
                            printf ( "\nPlease enter the passphrase: " );
						input = inputBuffer;
						hideInput = true;
						continue;
					}
				}
				else {
					hideInput = false;
					if ( AddUser ( accessLevel, userName.c_str(), inputBuffer ) ) {
                        CLogArg ( "User: [%s] successfully added.", userName.c_str() );
                        if ( printStdOut )
                            printf ( "\nUser: [%s] successfully added.\n", userName.c_str() );
					}
					else {
                        CErrArg ( "User: Failed to add user [%s]", userName.c_str() );
                        if ( printStdOut )
                            printf ( "\nUser: Failed to add user [%s]\n", userName.c_str() );
					}
					command = 0;
					userName = "";
					printf ( "----------------------------------------------------------------\n" );
				}
			}
            
            CLog ("");
            if ( printStdOut )
                printf ( "\n" );
			input = inputBuffer;
			continue;
		}
		else {
			*input = (char) c;
			if ( hideInput )
				printf ( "*" );
			else
				printf ( "%c", c );

			input++;
			if (input == inputBuffer + MEDIATOR_CLIENT_MAX_BUFFER_SIZE - 1) {
				// Buffer overflow now
				input = inputBuffer;

                CLog ("\nInput too long! Try again!");
                if ( printStdOut )
                    printf ("\nInput too long! Try again!\n");
				PrintSmallHelp ();
			}
		}
#endif
	} while ( isRunning );
}


void * MediatorDaemon::MediatorUdpThreadStarter ( void * arg )
{
	if ( arg  ) {
		ThreadInstance * listener = (ThreadInstance *) arg;
		if ( listener->daemon )
			// Execute thread
			return ((MediatorDaemon *)listener->daemon)->MediatorUdpThread ( arg );
	}	
	return 0;
}


void * MediatorDaemon::MediatorUdpThread ( void * arg )
{
	CVerb ( "MediatorUdpThread started." );

	if ( !arg  ) {
		CErr ( "MediatorUdpThread: called with invalid (NULL) argument." );
		return 0;
	}

	up ( char [ ] ) bufferUP = up ( char [ ] ) ( new char [ 64000 ] );
	if ( !bufferUP )
		return 0;
	
	char				*	buffer			= bufferUP.get ();
	char				*	decrypted		= 0;
    char				*	msgDec			= 0;
	unsigned int			msgDecLength;
	unsigned int			msgLength;
	sp ( ThreadInstance	)	client;

	struct 	sockaddr_in		listenAddr;
	MediatorThreadInstance * listener = (MediatorThreadInstance *) arg;

	unsigned short port = listener->instance.port;
	unsigned int sock = listener->socketUdp;
	
	CVerbArg ( "MediatorUdpThread for port [%d] started.", port );

	// Send started signal
	MutexLockVA ( thread_lock, "Udp" );

	if ( pthread_cond_signal ( &thread_condition ) ) {
		CErr ( "MediatorUdpThread: Error to signal event" );
	}

	MutexUnlockVA ( thread_lock, "Udp" );

	memset ( &listenAddr, 0, sizeof(listenAddr) );
	
	listenAddr.sin_family		= AF_INET;
	listenAddr.sin_addr.s_addr	= INADDR_ANY; //htonl ( INADDR_BROADCAST ); // INADDR_ANY );
    listenAddr.sin_port			= htons ( port );

	int ret = ::bind ( sock, (struct sockaddr *)&listenAddr, sizeof(listenAddr) );
	if ( ret < 0 ) {
		CErrArg ( "MediatorUdpThread: Failed to bind listener socket to port [%i]", port );
		LogSocketError ();
		return 0;
	}
	else {
        CVerbArg ( "MediatorUdpThread bound socket to port [%i]", port );
    }
		
    struct 	sockaddr_in		addr;
    socklen_t 				addrLen = sizeof(addr);
	int						length;
    
    CLogArg ( "MediatorUdpThread listen on port [%i]", port );
    
	// Create socket for 
	while ( isRunning ) {
		memset ( &addr, 0, sizeof(addr) );
		
		int bytesReceived = (int)recvfrom ( sock, buffer, 1024, 0, (sockaddr *) &addr, &addrLen );

		if ( bytesReceived <= 0 ) {
			CLogArg ( "Udp: Socket on port [%i] has been closed.", port );
			break;
		}
					
		UdpEncHelloPacket * hp = (UdpEncHelloPacket *)buffer;
		
		msgLength = hp->size;
		unsigned flags = 0xF0000000 & msgLength;		
         
        /// Message is encrypted                
        if ( flags & 0x80000000 )
        {
			msgLength &= 0xFFFFFFF;
			if ( msgLength > (unsigned int) bytesReceived ) {
				CWarnArg ( "Udp: Message size [%u] is larger than bytes received.", msgLength );
				continue;
			}

			client = GetSessionClient ( hp->sessionID );
			if ( !client ) {
				CWarnArg ( "Udp: Requested device session does not exist [%u].", (unsigned int)hp->sessionID );
				continue;
			}
			msgDecLength = msgLength - sizeof(UdpEncHelloPacket);

            /// Message is AES encrypted
			if ( !AESDecrypt ( &client->aes, &hp->aes, &msgDecLength, &decrypted ) )
                goto ContinueRec;
            msgDec = decrypted;
        }
		else {
			msgDecLength = bytesReceived;
			msgDec = buffer;
		}			


		CVerbArg ( "Udp: Received %d bytes from IP [%s] Port [%d]", msgDecLength, inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );

		if ( msgDec [ 0 ] == 'u' && msgDec [ 1 ] == ';' && msgDec [ 2 ] == ';' && msgDec [ 3 ] == ';' ) 
		{
			STUNReqPacket * stunPacket = (STUNReqPacket *) msgDec;

			unsigned int sourceID = stunPacket->sourceID;
			unsigned int destID = stunPacket->destID;

			if ( !destID ) {
				CErr ( "Udp: Invalid deviceID 0" );
				continue;
			}
			
			// Find source device and check validity of IP
			sp ( ThreadInstance ) clientSP;

			if ( bytesReceived >= (int)sizeof(STUNReqPacket) ) 
			{
				msgDec [ msgDecLength ] = 0;
				const char * areaName;
				const char * appName;

				sp ( ApplicationDevices ) appDevices	= GetApplicationDevices ( stunPacket->areaName, stunPacket->appName );
			
				if ( !appDevices )
					continue;

				if ( !appDevices->Lock ( "Udp" ) ) {
					UnlockApplicationDevices ( appDevices.get () );
					continue;
				}

				DeviceInstanceNode * device = GetDeviceInstance ( destID, appDevices->devices );
				if ( !device ) {
					CErrArg ( "Udp: requested STUN device [%d] does not exist", destID );
					goto Continue;
				}

				clientSP = device->clientSP;

				UnlockApplicationDevices ( appDevices.get () );

				appDevices->Unlock ( "Udp" );

				if ( !clientSP ) {
					CErrArg ( "Udp: requested client of STUN device [%d] does not exist", destID );
					goto ContinueRec;
				}

				// Reply with mediator ack message to establish the temporary route entry in requestor's NAT
				length	= (int) sendto ( sock, "y;;-    ", 8, 0, (const sockaddr *) &addr, sizeof(struct sockaddr_in) );
				if ( length != 8 ) {
					CErrArg ( "Udp: Failed to send UDP ACK to device [%d]", sourceID );
				}

				areaName = stunPacket->areaName;
				if ( *stunPacket->areaNameSrc )
					areaName = stunPacket->areaNameSrc;

				appName = stunPacket->appName;
				if ( *stunPacket->appNameSrc )
					appName = stunPacket->appNameSrc;

				HandleSTUNRequest ( clientSP.get (), sourceID, areaName, appName, (unsigned int) addr.sin_addr.s_addr, ntohs ( addr.sin_port ) );

				goto ContinueRec;
Continue:
				UnlockApplicationDevices ( appDevices.get () );

				appDevices->Unlock ( "Udp" );
			}
		}	

ContinueRec:
		if ( decrypted ) {
			free ( decrypted );
			decrypted = 0;
		}
	}
   
	if ( decrypted ) free ( decrypted );

	CLogArg ( "MediatorUdpThread: Thread for port [%d] terminated.", port );

	return 0;
}


void * MediatorDaemon::AcceptorStarter ( void * arg )
{
	if ( arg  ) {
		ThreadInstance * listener = (ThreadInstance *) arg;
		if ( listener->daemon )
			// Execute thread
			return ((MediatorDaemon *)listener->daemon)->Acceptor ( arg );
	}
	return 0;
}


void * MediatorDaemon::Acceptor ( void * arg )
{
	CVerb ( "Acceptor started." );

	if ( !arg  ) {
		CErr ( "Acceptor: Called with (NULL) argument." );
		return 0;
	}
	
	//pthread_attr_t attr;
	struct 	sockaddr_in		listenAddr;
	ThreadInstance *        listener    = (ThreadInstance *) arg;
	int s;

	unsigned short port = listener->port;
	
	CVerbArg ( "Acceptor for port %d started.", port );

	// Send started signal
	MutexLockVA ( thread_lock, "Acceptor" );

	if ( pthread_cond_signal ( &thread_condition ) ) {
		CErr ( "Acceptor: Error to signal event" );
	}

	MutexUnlockVA ( thread_lock, "Acceptor" );

	memset ( &listenAddr, 0, sizeof(listenAddr) );
	
	listenAddr.sin_family		= AF_INET;
	listenAddr.sin_addr.s_addr	= INADDR_ANY;
    listenAddr.sin_port			= htons ( port );

	int ret = ::bind ( (int) listener->socket, (struct sockaddr *)&listenAddr, sizeof(listenAddr) );
	if ( ret < 0 ) {
		CErrArg ( "Acceptor: Failed to bind listener socket to port [ %i ]!", port );
		LogSocketError ();
		return 0;
	}	
	CVerbArg ( "Acceptor bound to port [ %i ]", port );

	ret = listen ( (int) listener->socket, SOMAXCONN );
	if ( ret < 0 ) {
		CErrArg ( "Acceptor: Failed to listen on socket to port [ %i ]!", port );
		LogSocketError ();
		return 0;
	}
	CLogArg ( "Acceptor listen on port [ %i ]", port );
	
    struct 	sockaddr_in		addr;
    socklen_t 				addrLen = sizeof(addr);

	while ( isRunning )
    {
		Zero ( addr );
		
		int sock = (int) accept ( (int) listener->socket, (struct sockaddr *)&addr, &addrLen );

		if ( sock < 0 ) {
			CLogArg ( "Acceptor: Socket [ %i ] on port [ %i ] has been closed!", sock, port );
			break;
		}

		CLog ( "\n" );
        const char * ips = inet_ntoa ( addr.sin_addr );
        
		CLogArg ( "Acceptor: New socket [ %i ] connection with IP [ %s : %d ]", sock, ips, ntohs ( addr.sin_port ) );
        CLog ( "\n" );
        
        if ( acceptEnabled )
        {
            sp ( ThreadInstance ) client;
            
            client = make_shared < ThreadInstance > ();
            if ( !client ) {
                CErr ( "Acceptor: Failed to allocate memory for client request!" );
                goto NextClient;
            }
            memset ( client.get (), 0, sizeof ( ThreadInstance ) - ( sizeof ( sp ( DeviceInstanceNode ) ) + sizeof ( sp ( ThreadInstance ) ) ) );
            
            if ( !client->Init () )
                goto NextClient;
            
            client->socket		= sock;
            client->spareSocket	= -1;
            client->port		= port;
            client->filterMode	= 2;
            client->aliveLast	= checkLast;
            client->daemon		= this;
            client->stuntTarget = 0;
            
#ifdef MEDIATOR_LIMIT_STUNT_REG_REQUESTS
            client->stuntLastSend   = 0;
            client->stunLastSend    = 0;
#endif
            
            client->subscribedToNotifications   = true;
            client->subscribedToMessages        = true;
            client->connectTime = GetEnvironsTickCount ();
            
            memcpy ( &client->addr, &addr, sizeof(addr) );
            
            strlcpy ( client->ips, ips, sizeof ( client->ips ) );
            
            if ( !acceptClients.Lock ( "Acceptor" ) )
                goto NextClient;
            
            client->clientSP = client;
            
            pthread_reset ( client->threadID );
            
            // Create client thread
            s = pthread_create ( &client->threadID, 0, &MediatorDaemon::ClientThreadStarter, (void *)client.get () );
            if ( s != 0 ) {
                CWarnArg ( "Acceptor: pthread_create failed [%i]", s );
                client->clientSP = 0;
            }
            else {
                sock = -1;
                acceptClients.list.push_back ( client->clientSP );
            }
            
            acceptClients.Unlock ( "Acceptor" );
        }
        
    NextClient:
        if ( sock != -1 ) {
            CVerbArg ( "Acceptor: Closing [ %i ]", sock );
            
            shutdown ( sock, 2 ); closesocket ( sock );
        }
	}
   
	CLogArg ( "Acceptor: Thread for port [ %d ] terminated.", port );

	return 0;
}


bool MediatorDaemon::addToArea ( const char * area, const char * app, const char * pKey, const char * pValue )
{
	bool ret = false;

	// Search for the area at first
	sp ( ListValues )			values	= 0;
	sp ( AppsList )             appsList;
		
	if ( !area || !app ) {
		CErr ( "addToArea: Invalid parameters. Missing area/app name!" );
		return false;
	}

	if ( !areasMap.Lock ( "addToArea" ) )
		return false;
	
	string areaName ( area );
	string appName ( app );

	const msp ( string, AppsList )::iterator areaIt = areasMap.list.find ( areaName );

	if ( areaIt == areasMap.list.end () )
	{		
		appsList = make_shared < AppsList > ();
		if ( !appsList || !appsList->Init () ) {
			CErrArg ( "addToArea: Failed to create new area [%s].", area );
			return false;
		}
		areasMap.list [ areaName ] = appsList;
	}
	else
        appsList = areaIt->second;
    
    if ( !areasMap.Unlock ( "addToArea" ) )
        return false;
	
    if ( !appsList || !appsList->Lock ( "addToArea" ) )
        return false;
    
	const msp ( string, ListValues )::iterator appsIt = appsList->apps.find ( appName );

	if ( appsIt == appsList->apps.end () )
	{		
		values = make_shared < ListValues > (); //new map<string, ValuePack*> ();
		if ( !values || !values->Init () ) {
			CErrArg ( "addToArea: Failed to create new application [%s].", appName.c_str () );
			return false;
		}
		appsList->apps [ appName ] = values;
	}
	else
		values = appsIt->second;

	appsList->Unlock ( "addToArea" );

	if ( values && values->Lock ( "addToArea" ) ) {
		ret = addToArea ( values, pKey, pValue, ( unsigned int ) strlen ( pValue ) );

		values->Unlock ( "addToArea" );
	}

//EndWithStatus:
	return ret;
}


bool MediatorDaemon::addToArea ( sp ( ListValues ) &values, const char * pKey, const char * pValue, unsigned int valueSize )
{
	bool ret = false;
	
	if ( !values || !pKey || !pValue || valueSize <= 0 ) {
		CErr ( "addToArea: Invalid parameters. Missing key/value!" );
		return false;
	}

	sp ( ValuePack ) pack = 0;
	string key = pKey;

	// Look whether we already have a value for the key
	const msp ( string, ValuePack )::iterator valueIt = values->values.find ( key );
	
    if ( valueIt != values->values.end() )
        pack = valueIt->second;
    
    // Update the old value
    if ( pack ) {
        pack->timestamp = std::time(0);
        pack->value = string ( pValue );
        pack->size = valueSize;
    }
    else
    {
        pack = make_shared < ValuePack > (); // new ValuePack ();
        if ( !pack ) {
            CErrArg ( "addToArea: Failed to create new value object for %s! Memory low problem!", pKey );
            goto EndWithStatus;
        }
        pack->timestamp = std::time(0);
        pack->value = string ( pValue );
        pack->size = valueSize;
        
        values->values [ key ] = pack;
    }

	ret = true;

	//CVerbArg ( "addToArea: added %s -> %s", pKey, pValue );

EndWithStatus:
	return ret;
}


bool MediatorDaemon::sendDatabase ( int sock, struct sockaddr * addr )
{
	return true;
	/*
	bool ret = true;
	
	size_t length, sentBytes;

	char buffer [ BUFFERSIZE ];
	
	if ( pthread_mutex_lock ( &areasMutex ) ) {
		CErr ( "sendDatabase: Failed to aquire mutex on areas/values! Skipping client request!" );
		return false;
	}

	for ( map<string, map<string, ValuePack*>*>::iterator it = areas.begin(); it != areas.end(); ++it )
	{
		CLogArg ( "Area: %s", it->first.c_str() );

		if ( it->second ) {
			for ( map<string, ValuePack*>::iterator itv = it->second->begin(); itv != it->second->end(); ++itv )
			{
				// Parse values: "<P>Area Key Value"
				// Parse values: "<P>Area Key Value<P>Project Key Value"
				// Parse values: "<P>Area Key Value<P><EOF>"
				if ( itv->second ) {
					sprintf_s ( buffer, BUFFERSIZE, "<P>%s %s %s", it->first.c_str(), itv->first.c_str(), itv->second->value.c_str() );
									
					length		= strlen ( buffer );
					sentBytes	= sendto ( sock, buffer, (int) length, 0, addr, sizeof(struct sockaddr) );
					if ( sentBytes != length ) {
						CErrArg ( "sendDatabase: Failed to send response %s", buffer );
						ret = false;
						goto EndOfGetAll;
					}
				}
			}
		}
	}

EndOfGetAll:
	const char * resp = "<P><EOF>";

	length		= strlen ( resp );
	sentBytes	= sendto ( sock, resp, (int) length, 0, addr, sizeof(struct sockaddr) );

	if ( sentBytes != length ) {
		CErrArg ( "sendDatabase: Failed to send %s", buffer );
	}

	if ( pthread_mutex_unlock ( &areasMutex ) ) {
		CErr ( "sendDatabase: Failed to release mutex on areas/values!" );
	}

	return ret;
	*/
}


bool endsWith (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}


bool MediatorDaemon::HandleRequest ( char * buffer, ThreadInstance * client )
{
	bool success = false;

	MediatorGetPacket * query = (MediatorGetPacket *)buffer;
	
	unsigned int msgLength = query->size;	
	
	CLogArg ( "HandleRequest [ %i ]: [%s]", client->socket, buffer + 4 );

	// Scan parameters
    char empty [2] = {0};
    
	char * params [ 4 ];
	memset ( params, 0, sizeof(params) );
	
	if ( !ScanForParameters ( buffer + sizeof(MediatorGetPacket), msgLength - sizeof(MediatorGetPacket), ";", params, 4 ) ) {
		CErrArg ( "HandleRequest [ %i ]: Invalid parameters.", client->socket );
		return false;
	}

	if ( !params [ 0 ] || !params [ 1 ] || !params [ 2 ] ) {
		CErrArg ( "HandleRequest [ %i ]: Invalid parameters. area and/or key missing!", client->socket );
		return false;
    }
    
    sp ( DeviceInstanceNode ) deviceSP = client->deviceSP;
		
	if ( *(params [ 0 ]) == ' ' ) {
		/// Use area name of the client
		params [ 0 ] = deviceSP ? deviceSP->info.areaName : empty;
	}
		
	if ( *(params [ 1 ]) == ' ' ) {
		/// Use app name of the client
		params [ 1 ] = deviceSP ? deviceSP->info.appName : empty;
	}
		
	// Search for the area at first
	sp ( ListValues )	values	= 0;
	sp ( AppsList )		apps    = 0;

	if ( !areasMap.Lock ( "HandleRequest" ) )
		return false;

	string areaName ( params [ 0 ] );

	const msp ( string, AppsList )::iterator areaIt = areasMap.list.find ( areaName );

	// COMMAND: Set New Value
	if ( query->cmd == MEDIATOR_CMD_SET )
	{
		if ( areaIt == areasMap.list.end () )
		{
			// Create a new map for the area
			apps = make_shared < AppsList > ();
			if ( !apps || !apps->Init () ) {
				CErrArg ( "HandleRequest [ %i ]: Failed to create new area [%s].", client->socket, params [ 0 ] );
				return false;
			}
			areasMap.list [ areaName ] = apps;
		}
		else
			apps = areaIt->second;

        areasMap.Unlock ( "HandleRequest" );
        
        if ( !apps || !apps->Lock ( "HandleRequest" ) )
            return false;
		
		const msp ( string, ListValues )::iterator appsIt = apps->apps.find ( string ( params [ 1 ] ) );
		if ( appsIt == apps->apps.end () ) 
		{
			string appName ( params [ 1 ] );

            values = make_shared < ListValues > (); // new map<string, ValuePack*> ();
			if ( !values || !values->Init () ) {
                CErrArg ( "HandleRequest [ %i ]: Failed to create new application [%s].", client->socket, params [ 1 ] );
                return false;
			}
			apps->apps [ appName ] = values;
		}
		else
			values = appsIt->second;

		apps->Unlock ( "HandleRequest" );
		
		if ( !values || !params [ 2 ] || !params [ 3 ] ) {
            CErrArg ( "HandleRequest [ %i ]: Invalid parameters. Missing key for [%s]", client->socket, params [ 1 ] );
            return false;
		}
			
		char * key = params [ 2 ];
		if ( endsWith ( string ( key ), string ( "_push" ) ) ) {
			int len = (int) strlen ( key );
			key [ len - 5 ] = 0;
			int clientID = -1;
			sscanf_s ( key, "%d", &clientID );
			if ( clientID > 0 ) {
				CLogArg ( "HandleRequest [ %i ]: sending push notification to client [%i: %s]", client->socket, clientID, params [ 3 ] );
/*
				if ( !SendPushNotification ( values.get(), clientID, params [ 2 ] ) ) {
					CErrArg ( "HandleRequest: sending push notification to client [%i] failed. [%s]", clientID, params [ 3 ] );
				}
				else success = true;
                */
            }
            return false;
		}
		
		int valueSize = (int) (msgLength - (params [ 3 ] - buffer));
		if ( valueSize <= 0 ) {
            CErrArg ( "HandleRequest [ %i ]: size of value of [%i] invalid!", client->socket, valueSize );
            return false;
		}

		if ( !values->Lock ( "HandleRequest" ) )
			return false;

		if ( !addToArea ( values, params [ 2 ], params [ 3 ], (unsigned) valueSize ) ) {
            CErrArg ( "HandleRequest [ %i ]: Adding key [%s] failed!", client->socket, params [ 2 ] );
			success = false;
		}

		if ( !values->Unlock ( "HandleRequest" ) )
			return false;

		if ( success )
			CLogArg ( "HandleRequest [ %i ]: [%s/%s] +key [%s] value [%s]", client->socket, params [ 0 ], params [ 1 ], params [ 2 ], params [ 3 ] );
        return success;
	}
		
	// Look whether we already have a value for the key
	if ( query->cmd == MEDIATOR_CMD_GET )
	{			
		const char * response = "---";
		size_t length	= strlen ( response );

		if ( areaIt != areasMap.list.end () )
			apps = areaIt->second;
        
        areasMap.Unlock ( "HandleRequest" );
        
        if ( !apps )
            return false;

		sp ( ValuePack ) value = 0;
        
        const msp ( string, ListValues )::iterator appsIt = apps->apps.find ( string ( params [ 1 ]) );
        if ( appsIt != apps->apps.end () ) {
            values = appsIt->second;
            
            if ( !values || !values->Lock ( "HandleRequest" ) )
                return false;

            const msp ( string, ValuePack )::iterator valueIt = values->values.find ( string ( params [ 2 ] ) );
            
            if ( valueIt != values->values.end() )
				value = valueIt->second;

			values->Unlock ( "HandleRequest" );
                
            if ( !value )
                return false;

            response = value->value.c_str ();
            length = value->size;
        }

		if ( length > 0 ) {
			// Add 4 bytes for the message size
			length += 5;

			char * sendBuffer = ( char * ) malloc ( length );
			if ( !sendBuffer ) {
                CErrArg ( "HandleRequest [ %i ]: Failed to allocate buffer of size [%u] for sending the requested value.", client->socket, ( unsigned int ) length );
                return false;
			}

			*( ( unsigned int * ) sendBuffer ) = ( unsigned int ) length;
			memcpy ( sendBuffer + 4, response, length - 5 );
			sendBuffer [ length - 1 ] = 0;

			int sentBytes = SendBuffer ( client, sendBuffer, ( unsigned int ) length );
			if ( sentBytes != ( int ) length ) {
				CErrArg ( "HandleRequest [ %i ]: Failed to response value [%s] for key [%s]", client->socket, response, params [ 1 ] );
			}
			else success = true;

			free ( sendBuffer );
		}

        return success;
    }
    
    areasMap.Unlock ( "HandleRequest" );

	CWarnArg ( "HandleRequest [ %i ]: command [%c] not supported anymore", client->socket, query->cmd );
    return false;
}


sp ( ThreadInstance ) MediatorDaemon::GetSessionClient ( long long sessionID )
{
	CVerbVerbArg ( "GetSessionClient [%d]", sessionID );

	sp ( ThreadInstance ) client = 0;

	sessions.Lock ( "GetSessionClient" );

	//sid |= ((long long)((client->device->root->id << 16) | client->device->root->areaId)) << 32;
			
	const msp ( long long, ThreadInstance )::iterator sessionIt = sessions.list.find ( sessionID );
	if ( sessionIt != sessions.list.end () )
	{
		client = sessionIt->second;
	}

	sessions.Unlock ( "GetSessionClient" );
	
	CVerbVerbArg ( "GetSessionClient: [%s]", client ? client->uid : "NOT found" );

	return client;
}


int MediatorDaemon::HandleRegistration ( int &deviceID, const sp ( ThreadInstance ) &client, unsigned int bytesLeft, char * msg, unsigned int msgLen )
{
	CVerbArg ( "HandleRegistration [ %s : %i ]", client->ips, client->socket );

	int ret = -1;
	char * decrypted = 0;

	if ( msg [ 7 ] == 'S' ) {
		do
		{
			bool					useSession	= true;
			sp ( ThreadInstance )	relClient	= 0;	
			SpareSockDecPack	*	regPack		= (SpareSockDecPack *) msg;

			/// Get the size of the registration packet
			unsigned int regLen =  0;

			if ( regPack->sizeReq >= 16 ) 
			{
				regLen =  regPack->sizePayload;
				if ( regLen + 12 > bytesLeft ) {
					CWarnArg ( "HandleRegistration [ %s : %i ]:\tSpare socket registration packet overflow.", client->ips, client->socket ); break;
				}
                
				relClient = GetSessionClient ( regPack->sessionID );
			}
			else {
				useSession = false;

				regLen = *((unsigned int *)(msg + 12));
				if ( regLen + 12 > bytesLeft ) {
					CWarnArg ( "HandleRegistration [ %s : %i ]:\tSpare socket registration packet overflow.", client->ips, client->socket ); break;
				}

				/// Get the spare id
				unsigned int spareIDa = *((unsigned int *) (msg + 8));

				if ( !spareClients.Lock  ( "HandleRegistration" ) )
					break;

				const msp ( long long, ThreadInstance )::iterator iter = spareClients.list.find ( spareIDa );
				if ( iter == spareClients.list.end () ) {
					CLogArg ( "HandleRegistration [ %s : %i ]:\tSpare id [%u] not found.", client->ips, client->socket, spareIDa );
                    
                    spareClients.Unlock  ( "HandleRegistration" );
					break;
				}

				relClient = iter->second;
                spareClients.list.erase ( iter );
                
                if ( !spareClients.Unlock  ( "HandleRegistration" ) )
                    break;
			}
						
			if ( !relClient ) {
				CWarnArg ( "HandleRegistration [ %s : %i ]:\tSpare socket client does not exist.", client->ips, client->socket ); break;
			}

			//CLogArg ( "HandleRegistration: Encrypted [%s]", ConvertToHexSpaceString ( msg + 16, regLen ) );

			if ( !AESDecrypt ( &relClient->aes, useSession ? &regPack->payload : (msg + 16), &regLen, &decrypted ) )
				break;
						
			if ( regLen != MEDIATOR_BROADCAST_SPARE_ID_LEN ) {
				CWarnArg ( "HandleRegistration [ %s : %i ]:\tSpare socket registration packet is not of correct type.", client->ips, client->socket ); break;
			}

			HandleSpareSocketRegistration ( client.get (), relClient, decrypted + 4, regLen );
			CVerbArg ( "HandleRegistration [ %s : %i ]:\tClosing spare socket reg. thread [0x%X].", client->ips, client->socket, relClient->deviceID );

			ret = 0;
		}
		while ( 0 );
	}
	else if ( msg [ 7 ] == MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID ) {
		CLogArg ( "HandleRegistration [ %s : %i ]:\tAssigning next device ID.", client->ips, client->socket );

		msg [ msgLen ] = 0;
		MediatorReqMsg	* req = (MediatorReqMsg *) msg;
		
        sp ( ApplicationDevices ) appDevices;
		int nextID = 0;
		int mappedID = 0;
        
		if ( deviceMappings.Lock ( "HandleRegistration" ) )
		{
			sp ( DeviceMapping ) mapping;

            if ( *req->deviceUID ) {
                CVerbArg ( "HandleRegistration [ %s : %i ]:\tLooking for a mapping to deviceUID [%s].", client->ips, client->socket, req->deviceUID );
                
				const msp ( string, DeviceMapping )::iterator devIt = deviceMappings.list.find ( string ( req->deviceUID ) );

                if ( devIt != deviceMappings.list.end ( ) )
                {
					mapping = devIt->second;
                    nextID = mappedID = mapping->deviceID;
                    
                    CVerbArg ( "HandleRegistration [ %s : %i ]:\tMapping found deviceID [0x%X] authLevel [%i] authToken [...].", client->ips, client->socket, nextID, mapping->authLevel );
				}			
			}

            if ( !mapping ) {
                CVerbArg ( "HandleRegistration [ %s : %i ]:\tNo mapping found. Creating new.", client->ips, client->socket );
                
				mapping.reset ( new DeviceMapping );
				//mapping = make_shared < DeviceMapping > (); //sp ( DeviceMapping ) ( new DeviceMapping ); // calloc ( 1, sizeof(DeviceMapping) );
				if ( !mapping )
                    goto PreFailExit;
				memset ( mapping.get (), 0, sizeof(DeviceMapping) );
			}

            DeviceInstanceNode	**	deviceList;
            
			appDevices = GetDeviceList ( req->areaName, req->appName, 0, 0, deviceList );
			if ( appDevices ) 
			{
				if ( appDevices->Lock ( "HandleRegistration" ) )
				{
					/// Find the next free deviceID			
					DeviceInstanceNode	* device = appDevices->devices;

					if ( !nextID )
						nextID = (int) __sync_add_and_fetch ( &appDevices->latestAssignedID, 1 );
				
					while ( device ) 
					{
						if ( device->info.deviceID > appDevices->latestAssignedID ) {
							___sync_test_and_set ( &appDevices->latestAssignedID, device->info.deviceID );
						}

						if ( device->info.deviceID == nextID )
							nextID = (int) __sync_add_and_fetch ( &appDevices->latestAssignedID, 1 );
						else {
							if ( device->info.deviceID > nextID )
								break;
						}

						device = device->next;
					}

					appDevices->Unlock ( "HandleRegistration" );
				}

				UnlockApplicationDevices ( appDevices.get () );
			}

			if ( !nextID ) {
				srand ( ( unsigned ) GetEnvironsTickCount () );

				int randID = 0;
				while ( randID == 0 ) {
					randID = rand () % 0xFFFFFF;
				}

				unsigned int ip = ( unsigned int ) client->addr.sin_addr.s_addr;

				randID <<= 8;
				randID |= ( ip & 0xFF );

				nextID = randID;
			}

			if ( nextID && (nextID != mappedID) ) {
				mapping->deviceID = nextID;
                
                CVerbArg ( "HandleRegistration [ %s : %i ]:\tCreated a mapping with deviceID [0x%X].", client->ips, client->socket, nextID );

				deviceMappings.list [ string ( req->deviceUID ) ] = mapping;
			}
            
		PreFailExit:
            deviceMappings.Unlock ( "HandleRegistration" );
            
            if ( !mapping ) {
                CErrArg ( "HandleRegistration [ %s : %i ]:\tFailed to find or create a mapping.", client->ips, client->socket );
                return false;
            }
            
            if ( nextID )
                deviceMappingDirty = true;
		}	
				
		CLogArg ( "HandleRegistration [ %s : %i ]:\tAssigning device ID [%u] to [%s].", client->ips, client->socket, nextID, *req->deviceUID ? req->deviceUID : "Unknown" );

		SendBuffer ( client.get (), &nextID, sizeof(nextID) );
		return 1;
	}
	else {
        /// Message seems to be already decrypted
        
		deviceID = *( (unsigned int *) (msg + 16) );
		
		CLogArgID ( "HandleRegistration [ %s : %i ]:\t[%c%c%c%c]", client->ips, client->socket, msg [ 4 ], msg [ 5 ], msg [ 6 ], msg [ 7 ] );
					
		/*if ( msgLen == MEDIATOR_BROADCAST_SPARE_ID_LEN ) {
			HandleSpareSocketRegistration ( client, deviceID );
			CVerbID ( "HandleRegistration: Closing spare socket reg. thread." );
			return 0;
		}
		else */
		if ( msgLen >= (MEDIATOR_BROADCAST_DESC_START + 3) ) {
			client->deviceID = deviceID;

			if ( !HandleDeviceRegistration ( client, (unsigned int) client->addr.sin_addr.s_addr, msg + 4 ) ) {
				CWarnArgID ( "HandleRegistration [ %s : %i ]:\tDevice registration failed.", client->ips, client->socket );
			}
            else {
                CVerbArgID ( "HandleRegistration [ %s : %i ]:\tSending socket buffer sizes.", client->ips, client->socket );
                
#ifdef MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_CLIENT
                char buffer [ 20 ];
                
                int recSize = 0;
                int sendSize = 0;
                
                socklen_t retSize = sizeof ( recSize );
                
                // - Load send buffer size
                int rc = getsockopt ( (int) client->socket, SOL_SOCKET, SO_RCVBUF, ( char * ) &recSize, &retSize );
                if ( rc < 0 ) {
                    CErrArgID ( "HandleRegistration [ %s : %i ]: Failed to query receive buffer size!", client->ips, client->socket ); LogSocketError ();
                }
                else {
                    CVerbArgID ( "HandleRegistration [ %s : %i ]: receive buffer size [%i]", client->ips, client->socket, recSize );
                }
                
                // - Load send buffer size
                retSize = sizeof ( sendSize );
                
                rc = getsockopt ( (int) client->socket, SOL_SOCKET, SO_SNDBUF, ( char * ) &sendSize, &retSize );
                if ( rc < 0 ) {
                    CErrArgID ( "HandleRegistration [ %s : %i ]: Failed to query send buffer size!", client->ips, client->socket ); LogSocketError ();
                }
                else {
                    CVerbArgID (  "HandleRegistration [ %s : %i ]: send buffer size [%i]", client->ips, client->socket, sendSize );
                }
                
                if ( sendSize && recSize )
                {
                    int * pUI = reinterpret_cast<int *>( buffer );
                    *pUI = MEDIATOR_MSG_SOCKET_BUFFERS_SIZE;
                    
                    buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
                    buffer [ 5 ] = MEDIATOR_CMD_SET_SOCKET_BUFFERS;
                    buffer [ 6 ] = ';';
                    buffer [ 7 ] = ';';
                    
                    pUI += 2;
                    
                    *pUI = recSize; pUI++;
                    *pUI = sendSize;
                    
                    if ( !SendBuffer ( client.get (), buffer, MEDIATOR_MSG_SOCKET_BUFFERS_SIZE ) ) {
                        CErrArgID ( "HandleRegistration [ %s : %i ]: Failed to inform about socket buffer sizes!", client->ips, client->socket );
                    }
                }
#endif
				CLogArgID ( "HandleRegistration [ %s : %i ]:\tDevice registration succeeded.", client->ips, client->socket );
                
				return 1;
			}
		}
	}
	
	if ( decrypted) free ( decrypted );

	return ret;
}


void * MediatorDaemon::ClientThreadStarter ( void *arg )
{
    if ( !arg  )
        return 0;
    
    ThreadInstance * client = ( ThreadInstance * ) arg;
    
    CVerbArg ( "ClientThreadStarter: Address of arg [ 0x%p ].", &client );
    
    MediatorDaemon * daemon = (MediatorDaemon *)client->daemon;
    if ( !daemon )
        return 0;
    
    sp ( ThreadInstance ) clientSP = client->clientSP;
    if ( clientSP )
    {
#ifdef MEDIATOR_USE_TCP_NODELAY
        int value = 1;
        int ret = setsockopt ( (int) client->socket, IPPROTO_TCP, TCP_NODELAY, ( const char * ) &value, sizeof ( value ) );
        if ( ret < 0 ) {
            CErr ( "ClientThreadStarter: Failed to set TCP_NODELAY on socket" ); LogSocketError ();
        }
#endif
        
#ifdef USE_NONBLOCK_CLIENT_SOCKET
#ifdef _WIN32
        u_long ul = 1;
        
        if ( ioctlsocket ( (int) client->socket, FIONBIO, &ul ) != NO_ERROR )
#else
        long arg;
        arg = fcntl ( (int) client->socket, F_GETFL, 0 );
        if ( fcntl ( (int) client->socket, F_SETFL, arg | O_NONBLOCK ) < 0 )
#endif
        {
            CErr ( "ClientThreadStarter: Failed to set NONBLOCK on socket" ); LogSocketError ();
        }
#endif
        // Execute thread
        daemon->ClientThread ( client );
    }
    
    if ( daemon->isRunning )
    {
        daemon->RemoveAcceptClient ( client );
        
        
        CVerbArg ( "Client [ %s : %i ]:\tDisposing memory for client", client->ips, client->port );
        
        sp ( DeviceInstanceNode ) deviceSP = client->deviceSP;
        
        if ( deviceSP ) {
            CVerbArg ( "Client [ %s : %i ]:\tReleasing deviceSP", client->ips, client->port );
            
            daemon->RemoveDevice ( deviceSP.get () );
            client->deviceSP = 0;
        }
        
        daemon->sessions.Lock ( "Client" );
        
        const msp ( long long, ThreadInstance )::iterator sessionIt = daemon->sessions.list.find ( client->sessionID );
        if ( sessionIt != daemon->sessions.list.end () ) {
            daemon->sessions.list.erase ( sessionIt );
        }
        
        daemon->sessions.Unlock ( "Client" );
        
        pthread_t thrd = client->threadID;
        if ( pthread_valid ( thrd ) )
        {
            pthread_reset ( client->threadID );
            pthread_detach_handle ( thrd );
        }
        client->daemon = 0;
    }
    return 0;
}

#ifdef MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_SERVER
/*
void ApplySocketBufferSizes ( ThreadInstance * client, char * msg )
{
    int deviceID = client->deviceID;
    
    CVerbArgID ( "ApplySocketBufferSizes [ %s : %i ]", client->ips, client->socket );
    
    int rc;
    int * pUI = reinterpret_cast<int *>( msg );
    
    int recSize = *pUI; pUI++;
    int sendSize = *pUI;
    
    if ( sendSize > 32000 && sendSize < 2000000 )
    {
        rc = setsockopt ( (int) client->socket, SOL_SOCKET, SO_RCVBUF, ( const char * ) &sendSize, sizeof ( sendSize ) );
        if ( rc < 0 ) {
            CErrArgID ( "ApplySocketBufferSizes [ %s : %i ]: Failed to set receive buffer size.", client->ips, client->socket ); LogSocketError ();
        }
        else {
            CVerbArgID ( "ApplySocketBufferSizes [ %s : %i ]: Receive buffer size set to [ %i ].", client->ips, client->socket, sendSize );
        }
    }
    
    if ( recSize > 32000 && recSize < 2000000 )
    {
        rc = setsockopt ( (int) client->socket, SOL_SOCKET, SO_SNDBUF, ( const char * ) &recSize, sizeof ( recSize ) );
        if ( rc < 0 ) {
            CErrArgID ( "ApplySocketBufferSizes [ %s : %i ]: Failed to set send buffer size.", client->ips, client->socket ); LogSocketError ();
        }
        else {
            CVerbArgID ( "ApplySocketBufferSizes [ %s : %i ]: Send buffer size set to [ %i ].", client->ips, client->socket, recSize );
        }
    }
}
*/
#endif


void * MediatorDaemon::ClientThread ( void * arg )
{
	CVerb ( "ClientThread started" );

	ThreadInstance * client = ( ThreadInstance * ) arg;
    
    CLogArg ( "ClientThread started for [ %s ] socket [ %i ]", client->ips, client->socket );
   
    sp ( ThreadInstance ) clientSP = client->clientSP;
	client->clientSP = 0;

	int deviceID = 0;
	char * buffer = 0;

	//socklen_t addrLen = sizeof(client->addr);

	CVerbArg ( "Client [ %s : %i ]:\tSource port [ %u ]", client->ips, client->socket, ntohs ( client->addr.sin_port ) );

	char		*	msg;
	char		*	msgEnd;
	int				bytesReceived, sock = (int) client->socket, sockID = sock;
	unsigned int	remainingSize	= MEDIATOR_CLIENT_MAX_BUFFER_SIZE - 1;
    char		*	msgDec			= 0;
	unsigned int	msgDecLength;
    char		*	decrypted		= 0;
    up ( char [ ] ) bufferUP;
    
#ifdef USE_NONBLOCK_CLIENT_SOCKET
    fd_set fds;
    struct timeval timeout;
    
    int rc;
#endif

    bool isBanned = IsIpBanned ( client->addr.sin_addr.s_addr );
	if ( isBanned )
		goto ShutdownClient;

	bufferUP = up ( char [ ] ) ( new char [ MEDIATOR_CLIENT_MAX_BUFFER_SIZE + 1 ] );
	if ( !bufferUP )
		goto ShutdownClient;

	msgEnd = buffer = bufferUP.get ();
	
    while ( isRunning ) {
        CVerbVerbArgID ( "Client [ %s : %i ]:\tGoing into recv ... buffer free [ %i ]!", client->ips, sock, remainingSize );
        
#ifdef USE_NONBLOCK_CLIENT_SOCKET
		FD_ZERO ( &fds );
		FD_SET ( ( unsigned ) sock, &fds );

		timeout.tv_sec	= 120;
		timeout.tv_usec = 0;
        
        rc = select ( sock + 1, &fds, NULL, NULL, &timeout );
        if ( rc == -1 ) {
            VerbLogSocketError (); VerbLogSocketError1 ();
            CVerbArgID ( "Client [ %s : %i ]:\tconnection/socket closed by someone; rc [ -1 ]!", client->ips, sock );
            break;
        }
        if ( rc == 0 ) {
            CVerbVerbArgID ( "Client [ %s : %i ]:\tconnection/socket timeoout; rc [ 0 ]!", client->ips, sock );
            continue;
        }
		if ( !FD_ISSET ( sock, &fds ) ) {
			CVerbVerbArgID ( "Client [ %s : %i ]:\tconnection/socket error; rc [ %i ]!", client->ips, sock, rc );
			continue;
		}

		bytesReceived = ( int ) recvfrom ( sock, msgEnd, remainingSize, 0, ( struct sockaddr* ) &client->addr, &addrLen );
		if ( bytesReceived <= 0 ) {
			SOCKETRETRY ();

			VerbLogSocketError (); VerbLogSocketError1 ();
			CVerbArgID ( "Client [ %s : %i ]:\tconnection/socket closed by someone; Bytes [ %i ]!", client->ips, sock, bytesReceived );
			break;
		}
#else

        //bytesReceived = ( int ) recvfrom ( sock, msgEnd, remainingSize, 0, ( struct sockaddr* ) &client->addr, &addrLen );
        bytesReceived = ( int ) recv ( sock, msgEnd, remainingSize, 0 );
		if ( bytesReceived <= 0 ) {
			VerbLogSocketError (); VerbLogSocketError1 ();
			CVerbArgID ( "Client [ %s : %i ]:\tconnection/socket closed by someone; Bytes [ %i ]!", client->ips, sock, bytesReceived );
			break;
		}
#endif

		msgEnd [bytesReceived] = 0;
		msg = buffer;

		unsigned int msgLength;
        char command;
		
		int bytesLeft = (int) (bytesReceived + (msgEnd - msg));
		while ( bytesLeft >= 8 ) 
		{
			msgLength = *((unsigned int *) msg);
			unsigned flags = 0xF0000000 & msgLength;
			
			msgLength &= 0xFFFFFFF;

			CVerbVerbArgID ( "Client [ %s : %i ]:\tmsgLength [%d] Bytes left [ %i ]!", client->ips, sock, msgLength, bytesLeft );

			if ( msgLength > (unsigned int) bytesLeft ) 
			{
				if ( msgLength >= MEDIATOR_CLIENT_MAX_BUFFER_SIZE ) {
					CErrArgID ( "Client [ %s : %i ]:\tmsgLength [ %d ] >= [ %i ]!", client->ips, sock, msgLength, MEDIATOR_CLIENT_MAX_BUFFER_SIZE );

					/// Reset channel until received bytes is 0?
					goto ShutdownClient;
				}
				break;
			}
            
            if ( flags )
            {
                /// Message is encrypted                
                if ( flags & 0x40000000 )
                {
					msgDecLength = msgLength;

                    /// Message is AES encrypted
                    if ( !AESDecrypt ( &client->aes, msg, &msgDecLength, &decrypted ) )
                        break;
                    msgDec = decrypted;
                }
                else {
					msgDecLength = msgLength - 4;

                    if ( flags == 0x80000000 ) {
                        /// Message is encrypted with public certificate
                        if ( !DecryptMessage ( privKey, privKeySize, msg + 4, msgDecLength, &decrypted, &msgDecLength ) ) {
                            CWarnArgID ( "Client [ %s : %i ]:\tDecrypt with privKey failed!", client->ips, sock ); goto ShutdownClient;
                        }
                        msgDec = decrypted;
                    }
					else {
						CErrArgID ( "Client [ %s : %i ]:\tUnknown encryption flags", client->ips, sock );
						goto ShutdownClient; /// Unknown encryption flags
					}
                }
            }
            else {
                msgDecLength = msgLength; msgDec = msg;
            }

			if ( msgDec [ 4 ] == 'H' && msgDec [ 5 ] == 'C' && msgDec [ 6 ] == 'L' && msgDec [ 7 ] == 'S' ) 
			{
				if ( client->encrypt ) {
					CWarnArgID ( "Client [ %s : %i ]:\tTried to establish a safe channel again. Not allowed.", client->ips, sock );
					goto ShutdownClient;
				}

				if ( !SecureChannelAuth ( client ) ) {
					CWarnArgID ( "Client [ %s : %i ]:\tEstablishing Security / Authentication failed.", client->ips, sock  );
					goto ShutdownClient;
				}
				CLogArgID ( "Client [ %s : %i ]:\tEstablishing Security / Authentication succeeded.", client->ips, sock );
				goto Continue;
			}

			if ( msgDec [ 4 ] == 'E' ) {
				if ( msgDec [ 5 ] == '.' && msgDec [ 6 ] == 'D' )
				{
					int ret = HandleRegistration ( deviceID, clientSP, bytesLeft, msgDec, msgDecLength );
                    if ( ret == 1 ) {
                        BannIPRemove ( client->addr.sin_addr.s_addr );
                        goto Continue;
                    }
					if ( ret == 0 ) {
                        if ( decrypted ) free ( decrypted );
						return 0;
					}
					CVerbArgID ( "Client [ %s : %i ]:\tHandle registration failed.", client->ips, sock );
					goto ShutdownClient;
				}
		
				CLogArgID ( "Client [ %s : %i ]:\tInvalid message [%c%c%c%c]. Refusing client.", client->ips, sock, msgDec [ 4 ], msgDec [ 5 ], msgDec [ 6 ], msgDec [ 7 ] );
				goto ShutdownClient;
			}

			CVerbArgID ( "Client [ %s : %i ]:\t[%c%c%c%c]", client->ips, sock, msgDec [ 4 ], msgDec [ 5 ], msgDec [ 6 ], msgDec [ 7 ] );

			if ( !client->deviceID ) {
				CWarnArgID ( "Client [ %s : %i ]:\tRequest has not been identified. kicking connect.", client->ips, sock );

				if ( !client->authenticated )
					BannIP ( client->addr.sin_addr.s_addr );
				goto ShutdownClient;
			}

			if ( msgDec [ 4 ] < MEDIATOR_PROTOCOL_VERSION_MIN ) {
				CWarnArgID ( "Client [ %s : %i ]:\tVersion [ %c ] not supported anymore", client->ips, sock, msgDec [ 4 ] );
				goto ShutdownClient;
			}

			client->aliveLast = checkLast;

			command = msgDec [ 5 ];
		
			if ( command == MEDIATOR_CMD_NATSTAT ) 
			{				
				bool behindNAT = (*((unsigned int *) (msgDec + 8)) != (unsigned int) client->addr.sin_addr.s_addr);
				
				*((unsigned int *) msgDec) = MEDIATOR_NAT_REQ_SIZE;
				msgDec [ 4 ] = behindNAT ? 1 : 0;
				
				int sentBytes = SendBuffer ( client, msgDec, MEDIATOR_NAT_REQ_SIZE );

				if ( sentBytes != MEDIATOR_NAT_REQ_SIZE ) {
					CErrArgID ( "Client [ %s : %i ]:\tFailed to response NAT value [ %d ]", client->ips, sock, behindNAT );
				}
			}
			// COMMAND: Return port of destination and request STUNT for client with deviceID (IP, IPe, Port)
			else if ( command == MEDIATOR_CMD_STUNT ) 
			{
				HandleSTUNTRequest ( client, (STUNTReqPacket *) msgDec );
			}
			// COMMAND: 
			else if ( command == MEDIATOR_CMD_STUN ) 
			{
				HandleSTUNRequest ( client, msgDec );
			}
			// COMMAND: 
			else if ( command == MEDIATOR_CMD_GET_DEVICES ) 
			{
                HandleQueryDevices ( clientSP, msgDec );
			}
			// COMMAND: 
			else if ( command == MEDIATOR_CMD_SHORT_MESSAGE ) 
			{
				HandleShortMessage ( client, msgDec );
			}
			// COMMAND: // This is probably DEPRECATED
			else if ( command == MEDIATOR_CMD_REQ_SPARE_ID ) 
			{
                spareClients.Lock  ( "HandleRegistration" );
                
				unsigned int sid = ++spareID;

				CVerbArgID ( "Client [ %s : %i ]:\tAssigned spare ID [ %u ]", client->ips, sock, sid );

				spareClients.list [ (unsigned int) sid ] = clientSP;
                
                spareClients.Unlock  ( "HandleRegistration" );
                
				*((unsigned int *) msgDec) = MEDIATOR_NAT_REQ_SIZE;
				*((unsigned int *) (msgDec + 4)) = (unsigned int) sid;
				
				int sentBytes = SendBuffer ( client, msgDec, MEDIATOR_NAT_REQ_SIZE );

				if ( sentBytes != MEDIATOR_NAT_REQ_SIZE ) {
					CErrArgID ( "Client [ %s : %i ]:\tFailed to response spare client ID [ %u ]", client->ips, sock, sid );
				}
			}
			// COMMAND:
			else if ( command == MEDIATOR_CMD_HELP_TLS_GEN )
			{
                if ( msgDec [ 6 ] == 's' )
                    HandleCertSign ( client, msgDec );
                else
                    HandleCLSGenHelp ( client );
			}
			// COMMAND: 
			else if ( command == MEDIATOR_CMD_SET_FILTERMODE )
            {
                if ( client->authLevel >= 3 ) {
                    MediatorMsg * medMsg = (MediatorMsg *) msgDec;
                    UpdateNotifyTargets ( clientSP, medMsg->ids.id2.msgID );
                }
			}
			// COMMAND: 
			else if ( command == MEDIATOR_CMD_HEARTBEAT ) 
			{
				goto Continue;
            }
            // COMMAND:
            else if ( command == MEDIATOR_CMD_GET_VERSION )
            {
                // 0x00 relCount, 0x000 minor, 0x000 major; 0x00000000 revision
                unsigned int version = BUILD_MAJOR_VERSION;
                version |= BUILD_MINOR_VERSION << 12;
                version |= BUILD_RELEASE_COUNTER << 24;
                
                unsigned int versionBuffer [5];
                versionBuffer[0] = MEDIATOR_MSG_VERSION_SIZE;
                versionBuffer[1] = version;
                versionBuffer[2] = BUILD_REVISION;
                
                int sentBytes = SendBuffer ( client, versionBuffer, MEDIATOR_MSG_VERSION_SIZE );
                
                if ( sentBytes != MEDIATOR_MSG_VERSION_SIZE ) {
                    CErrArgID ( "Client [ %s : %i ]:\tFailed to response Mediator version", client->ips, sock );
                }
            }
            // COMMAND:
            else if ( command == MEDIATOR_CMD_NOTIFICATION_SUBSCRIBE )
            {
                MediatorMsg * medMsg = (MediatorMsg *) msgDec;
                
                client->subscribedToNotifications = (medMsg->ids.id2.msgID == 1 ? true : false);
                
//                BannIPRemove ( client->addr.sin_addr.s_addr );
            }
            // COMMAND:
            else if ( command == MEDIATOR_CMD_MESSAGE_SUBSCRIBE )
            {
                MediatorMsg * medMsg = (MediatorMsg *) msgDec;
                
                client->subscribedToMessages = (medMsg->ids.id2.msgID == 1 ? true : false);
                
//                BannIPRemove ( client->addr.sin_addr.s_addr );
            }
#ifdef MEDIATOR_USE_SOCKET_BUFFERS_APPLY_AT_SERVER
            /*else if ( command == MEDIATOR_CMD_SET_SOCKET_BUFFERS )
            {
                ApplySocketBufferSizes ( client, msgDec + 8 );
            }
            */
#endif
			else {
				msgDec [ msgDecLength - 1 ] = 0;
 
                HandleRequest ( msgDec, client );
			}
Continue:
			bytesLeft -= msgLength;			
			msg += msgLength;

            if ( decrypted ) { free ( decrypted ); decrypted = 0; }
		}

		remainingSize = MEDIATOR_CLIENT_MAX_BUFFER_SIZE - 1;

		if ( bytesLeft > 0 ) {
			msgEnd += bytesReceived;

			refactorBuffer ( msg, buffer, bytesLeft, msgEnd );
			remainingSize -= bytesLeft;
		}
		else
			msgEnd = buffer;
	}

ShutdownClient:
    
#ifdef USE_LOCKFREE_SOCKET_ACCESS
    CloseThreadSocket ( &client->socket );
#else
    client->Lock ( "Client" );
    
    sock = client->socket;
    if ( sock != -1 ) {
        client->socket = -1;
        CVerbArg ( "Client: Closing [ %i ]", sock );
        
        shutdown ( sock, 2 );
        closesocket ( sock );
    }
    
    client->Unlock ( "Client" );
#endif

    if ( decrypted ) free ( decrypted );

	if ( isRunning )
	{
		if ( !client->deviceID && client->addr.sin_addr.s_addr ) {
            if ( !isBanned && !client->authenticated ) {
				CWarnArgID ( "Client [ %s : %i : %i ]:\tRequest has not been identified. kicking connect.", client->ips, client->port, sockID );
                BannIP ( client->addr.sin_addr.s_addr );
                isBanned = true;
            }
		}
        
        if ( !isBanned )
            UpdateNotifyTargets ( clientSP, -1 );
	}
    else {
        UpdateNotifyTargets ( clientSP, -1 );
    }

	CLogArgID ( "Client [ %s : %i : %i ]:\tDisconnected", client->ips, client->port, sockID );
	return 0;
}


DeviceInstanceNode * MediatorDaemon::GetDeviceInstance ( int deviceID, DeviceInstanceNode * device )
{
	while ( device ) {
		if ( device->info.deviceID == deviceID ) {
			return device;
		}
		device = device->next;
	}
	return 0;
}


sp ( ThreadInstance ) MediatorDaemon::GetThreadInstance ( ThreadInstance * sourceClient, int deviceID, const char * areaName, const char * appName )
{
    sp ( ApplicationDevices )   appDevices  = 0;
    pthread_mutex_t     *       destMutex   = 0;
    DeviceInstanceNode  *       destList    = 0;
    
    sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
    
    DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ( );
    
    if ( *areaName && *appName )
    {
        if ( sourceDevice && !strncmp ( areaName, sourceDevice->info.areaName, sizeof ( sourceDevice->info.areaName ) )
            && !strncmp ( appName, sourceDevice->info.appName, sizeof ( sourceDevice->info.appName ) ) )
        {
            destMutex = &sourceDevice->rootSP->lock;
        }
        else {
            appDevices = GetApplicationDevices ( areaName, appName );
            if ( !appDevices ) {
                return 0;
            }
            
            destMutex = &appDevices->lock;
        }
    }
    else {
        destMutex = &sourceDevice->rootSP->lock;
    }
    
    if ( !MutexLock ( destMutex, "GetThreadInstance" ) ) {
        return 0;
    }
    
    if ( appDevices ) {
        destList = appDevices->devices;
        
        UnlockApplicationDevices ( appDevices.get () );
    }
    else
        destList = sourceDevice->rootSP->devices;
    
    if ( !destList )
        return 0;
    
    sp ( ThreadInstance ) destClient;
    
    DeviceInstanceNode	* device = GetDeviceInstance ( deviceID, destList );
    if ( device )
        destClient = device->clientSP;
    
    MutexUnlockV ( destMutex, "GetThreadInstance" );
    
    return destClient;
}


bool MediatorDaemon::HandleShortMessage ( ThreadInstance * sourceClient, char * msg )
{
    CVerbVerbArg ( "HandleShortMessage [ %i ]", sourceClient->socket );

	// size of message (including size prefix)
	// 2m;;
	// 1. destination ID
	// 2. message ( size - 12 )

	ShortMsgPacket	*	shortMsg			= (ShortMsgPacket *)msg;
    char                version             = shortMsg->version;
	bool				sendError			= true;

	char			*	sendBuffer			= msg;
	int					length				= shortMsg->size;

    int                 destID				= shortMsg->deviceID;
    int                 deviceID			= sourceClient->deviceID;
    int					sentBytes;
    
    CVerbArgID ( "HandleShortMessage [ %i ]", sourceClient->socket );
    
    sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
    
    DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ( );
    
    
    // find the destination client
    sp ( ThreadInstance ) destClient = GetThreadInstance ( sourceClient, destID, shortMsg->areaName, shortMsg->appName );
    
	if ( !destClient || destClient->socket == -1 || !destClient->subscribedToMessages ) {
		CErrArgID ( "HandleShortMessage [ %i ]: Failed to find device connection for id [ 0x%X ]!", sourceClient->socket, destID );
		goto SendResponse;
    }
    
	shortMsg->deviceID = deviceID;
    if ( sourceDevice ) {
        if ( *sourceDevice->info.areaName )
			strlcpy ( shortMsg->areaName, sourceDevice->info.areaName, sizeof ( shortMsg->areaName ) );
        if ( *sourceDevice->info.appName )
			strlcpy ( shortMsg->appName, sourceDevice->info.appName, sizeof ( shortMsg->appName ) );
    }
        
	CLogArgID ( "HandleShortMessage [ %i ]: Send message to device [ 0x%X ] IP [ %u bytes -> %s ]", sourceClient->socket, destID, length, inet_ntoa ( destClient->addr.sin_addr ) );
	
	sentBytes = SendBuffer ( destClient.get (), sendBuffer, length );
	if ( sentBytes == length ) {
		CLogArgID ( "HandleShortMessage [ %i ]: Successfully sent message.", sourceClient->socket );
		sendError = false;
	}

SendResponse:
    if ( version < '4' )
    {
        length = 12;
        *((unsigned int *) sendBuffer) = length;
        
        if ( sendError ) {
            sendBuffer [ 6 ] = 'e';
        }
        else {
            sendBuffer [ 6 ] = 's';
        }
        
        CLogArgID ( "HandleShortMessage [ %i ]: Send reply to IP [ %s ]", sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
        
        sentBytes = SendBuffer ( sourceClient, sendBuffer, length );
        if ( sentBytes != length ) {
            CErrArgID ( "HandleShortMessage [ %i ]: Failed to send reply to requestor IP [ %s ]", sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
            LogSocketError ();
            return false;
        }
    }

	return true;
}

//#define FAKE42

bool CollectDevices ( ThreadInstance * sourceClient, unsigned int &startIndex,
	int reqDeviceID, const char * areaName, const char * appName,
	ApplicationDevices * appDevices, DeviceInfoNode *& resultList, unsigned int &resultCount )
{
	if ( !appDevices || !appDevices->count )
		return true;

	bool                    success				= true;
	DeviceInfoNode		*	curDevice			= resultList;
	unsigned int            currentStart;
	DeviceInstanceNode	*	device; //				= appDevices->devices;

	if ( !appDevices->Lock ( "CollectDevices" ) )
		return false;

	unsigned int listCount	= appDevices->count;
	
	if ( startIndex > listCount ) {
		startIndex -= listCount; 

		appDevices->Unlock ( "CollectDevices" );
		return true;
	}
	
	const unsigned int	maxDeviceCount		= (MEDIATOR_BUFFER_SIZE_MAX - DEVICES_HEADER_SIZE) / DEVICE_PACKET_SIZE;
    if ( resultCount >= maxDeviceCount )
        return false;
		
	device = appDevices->devices;
		
    if ( startIndex ) {
        currentStart = 0;
        while ( device ) {
            if ( currentStart >= startIndex )
                break;
            
            currentStart++;
            device = device->next;
        }
    }
    startIndex = 0;
    
	DeviceInfoNode * prev = curDevice;

	while ( device )
    {
        if ( reqDeviceID ) {
            if ( reqDeviceID == device->info.deviceID && !strncmp ( device->info.areaName, areaName, MAX_NAMEPROPERTY )
                && !strncmp ( device->info.appName, appName, MAX_NAMEPROPERTY ) )
            {
				DeviceInfoNode * newDevice = (DeviceInfoNode *) malloc ( sizeof( DeviceInfoNode ) );
                if ( !newDevice ) {
					success = false; goto Finish;
				}
                
                memcpy ( newDevice, device, sizeof(DeviceInfo) );
				//*newDevice->userName = 0;
                newDevice->next = 0;
                
                resultList = newDevice;
                resultCount = 1;
				success = false; goto Finish; /// Stop the query, We have found the device.
            }
            
            device = device->next;
            continue;
        }
        
        /// We have a device to insert into the result list
		DeviceInfoNode * newDevice = (DeviceInfoNode *) malloc ( sizeof( DeviceInfoNode ) );
        //DeviceInstanceNode * newDevice = new DeviceInstanceNode;
        if ( !newDevice ) {
			success = false; goto Finish;
		}
         
        memcpy ( newDevice, device, sizeof(DeviceInfo) );
		//*newDevice->userName = 0;
        newDevice->next = 0;
        
        
        /// Find the position in result list
        if ( !curDevice ) {
            /// We have an empty list; Create a new item and set as root
            resultList = newDevice;
			prev = newDevice;
        }
        else {
            /// Find the first id that is larger than the current            
            while ( curDevice ) {
                if ( curDevice->info.deviceID > device->info.deviceID ) {
                    break;
                }
                prev = curDevice;
                curDevice = curDevice->next;
            }
			curDevice = prev;

			if ( curDevice->info.deviceID > device->info.deviceID && curDevice == resultList ) {
				resultList = newDevice;
				newDevice->next = curDevice;
			}
			else {
				if ( curDevice->next )
					newDevice->next = curDevice->next;
				curDevice->next = newDevice;
			}
        }

        curDevice = newDevice;
        
        resultCount++;
        
        if ( resultCount >= maxDeviceCount ) {
			success = false; goto Finish;
		}

		device = device->next;
	}
    
Finish:
	if ( !appDevices->Unlock ( "CollectDevices" ) )
		return false;
	return success;
}


unsigned int MediatorDaemon::CollectDevicesCount ( DeviceInstanceNode * sourceDevice, int filterMode )
{
	if ( !sourceDevice || !sourceDevice->rootSP )
		return 0;

	unsigned int				deviceCount = 0;
    sp ( AreaApps )				areaApps	= 0;
    sp ( ApplicationDevices )	appDevices	= 0;

	/// Get number of devices within the same application environment
	deviceCount += sourceDevice->rootSP->count;

	if ( filterMode < 1 ) { 
		/// No filtering, get them all		
		if ( !areas.Lock ( "CollectDevicesCount" ) )
			goto Finish;

		/// Iterate over all areas
		for ( msp ( string, AreaApps )::iterator it = areas.list.begin(); it != areas.list.end (); ++it )
		{						
			areaApps = it->second;
			if ( !areaApps || !areaApps->Lock ( "CollectDevicesCount" ) )
				continue;

			for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin(); ita != areaApps->apps.end (); ++ita )
			{
				appDevices = ita->second;
				if ( !appDevices || appDevices.get () == sourceDevice->rootSP.get () )
					continue;

				deviceCount += appDevices->count;
			}

			areaApps->Unlock ( "CollectDevicesCount" );
		}

		areas.Unlock ( "CollectDevicesCount" );
	}
	else if ( filterMode < 2 ) {
		/// Get number of devices of other application environments within the same area
		string areaName ( sourceDevice->info.areaName );

		if ( !areas.Lock ( "CollectDevicesCount" ) )
			goto Finish;

		const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( areaName );

		if ( areaIt == areas.list.end ( ) ) {
			CLogArg ( "CollectDevicesCount: Area [%s] not found.", sourceDevice->info.areaName );
		}
		else
			areaApps = areaIt->second;

		areas.Unlock ( "CollectDevicesCount" );
	
		if ( !areaApps || !areaApps->Lock ( "CollectDevicesCount" ) ) {
			CLog ( "CollectDevicesCount: Invalid area name." );
			goto Finish;
		}
				
		for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin(); ita != areaApps->apps.end (); ++ita )
		{
			appDevices = ita->second;
			if ( !appDevices || appDevices.get () == sourceDevice->rootSP.get () )
				continue;

			deviceCount += appDevices->count;
		}

		areaApps->Unlock ( "CollectDevicesCount" );
	}

Finish:
	return deviceCount;
}


char * FlattenDeviceList ( DeviceInfoNode * devices, unsigned int capacity )
{
    if ( !devices || !capacity )
        return 0;

    DeviceInfo * dest = 0;
    char * buffer = (char *) malloc ( (capacity * DEVICE_PACKET_SIZE) + DEVICES_HEADER_SIZE + DEVICES_HEADER_SIZE );
    if ( !buffer ) {
        goto Finish;
    }
    
    dest = (DeviceInfo *) (buffer + DEVICES_HEADER_SIZE);
    
    while ( devices ) {
		DeviceInfoNode * current = devices;
        devices = devices->next;

        memcpy ( dest, current, sizeof(DeviceInfo) );
        dest++;

        free ( current );
    }
    
Finish:
    if ( devices ) {
        while ( devices ) {
			DeviceInfoNode * current = devices;
            
            devices = devices->next;
            free ( current );
        }
    }
    return buffer;
}


bool MediatorDaemon::HandleQueryDevices ( const sp ( ThreadInstance ) &sourceClient, char * msg )
{
	sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
	if ( !sourceDeviceSP )
		return false;
	DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ( );

	MediatorQueryResponse * response		= (MediatorQueryResponse *)msg;
	MediatorQueryMsg *	query				= (MediatorQueryMsg *)msg;
	char			*	sendBuffer			= msg;
	int					length				= 8;
	unsigned int		availableDevices	= 0;
	unsigned int		deviceID			= query->msgID;
	unsigned int		deviceIDReq			= 0;

	bool				error				= true;
	unsigned int		startIndex;
	int					filterMode			= sourceClient->filterMode;
	char				subCmd;
	DeviceInfoNode  *   resultList			= 0;
    unsigned int        resultCount         = 0;

	const char		*	areaName = sourceDevice->info.areaName;
	const char		*	appName = sourceDevice->info.appName;

	msp ( string, ApplicationDevices ) * apps;
	 
	sp ( ApplicationDevices ) appDevices = sourceDevice->rootSP;
	
	/// Take over filterMode	
	subCmd = query->opt1;

	if ( subCmd != MEDIATOR_OPT_NULL ) {
		filterMode = subCmd;
    }
    
    if ( sourceClient->authLevel < 3 )
        filterMode = MEDIATOR_FILTER_AREA_AND_APP;

    if ( filterMode != sourceClient->filterMode )
		UpdateNotifyTargets ( sourceClient, filterMode );

	// Send number of available devices
	if ( query->opt0 == MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT ) {
		CVerbArgID ( "HandleQueryDevices [ %i ]: requested count", sourceClient->socket );
		
        length = MEDIATOR_CMD_GET_DEVICES_COUNT_RESP_LEN;
        
        query->size = length;
        query->cmdVersion = MEDIATOR_PROTOCOL_VERSION;
        query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
        query->opt0 = MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT;
        
        query->msgID = CollectDevicesCount ( sourceDevice, filterMode );

		CVerbArgID ( "HandleQueryDevices [ %i ]: Number of devices [%u]", sourceClient->socket, query->msgID );
		error = false; goto SendResponse;
	}
	
	availableDevices = appDevices->count;

	startIndex = query->startIndex;
		
	
	subCmd = query->opt0;

	if ( subCmd == MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID ) {
		deviceIDReq = query->deviceID;
		if ( *query->areaName && *query->appName ) {
			areaName = query->areaName;
			appName = query->appName;

			CVerbArgID ( "HandleQueryDevices [ %i ]: Query for deviceID [%u] [%s/%s]", sourceClient->socket, deviceIDReq, areaName, appName );
		}
		else {
			areaName = sourceDevice->info.areaName;
			appName = sourceDevice->info.appName;

			CVerbArgID ( "HandleQueryDevices [ %i ]: Query for deviceID [%u] of default appEnv [%s/%s]", sourceClient->socket, deviceIDReq, areaName, appName );
		}
	}

	CVerbArgID ( "HandleQueryDevices [ %i ]: start index [%u] filterMode [%i]", sourceClient->socket, startIndex, filterMode );

	/// Lets start with the devices within the same appEnv	

	if ( appDevices->count && !CollectDevices ( sourceClient.get (), startIndex, deviceIDReq, areaName, appName,
		appDevices.get (), resultList, resultCount ) ) 
		goto Finish;
	
	if ( filterMode < 1 ) {
		/// No filtering, get them all	
		if ( !areas.Lock ( "HandleQueryDevices" ) )
			goto Finish;

		vsp ( AreaApps ) searchAreas;

		/// Iterate over all areas
		for ( msp ( string, AreaApps )::iterator it = areas.list.begin(); it != areas.list.end (); ++it )
		{
			searchAreas.push_back ( it->second );
		}

		areas.Unlock ( "HandleQueryDevices" );

		for ( vsp ( AreaApps )::iterator its = searchAreas.begin (); its != searchAreas.end (); its++ )
		{
			AreaApps * areaApps	= its->get ();

			if ( !areaApps || !areaApps->Lock ( "HandleQueryDevices" ) )
				continue;
			apps = &areaApps->apps;

			for ( msp ( string, ApplicationDevices )::iterator ita = apps->begin (); ita != apps->end (); ita++ )
			{
				appDevices = ita->second;
				if ( !appDevices || appDevices.get () == sourceDevice->rootSP.get () || !appDevices->count )
					continue;

				availableDevices += appDevices->count;

				if ( !CollectDevices ( sourceClient.get (), startIndex, deviceIDReq, areaName, appName,
					appDevices.get (), resultList, resultCount ) ) 
				{
					areaApps->Unlock ( "HandleQueryDevices" );
					goto Finish;
				}
			}

			areaApps->Unlock ( "HandleQueryDevices" );
		}
	}
	else if ( filterMode < 2 ) {
		/// Get number of devices of other application environments within the same area
		string pareaName ( areaName );

		sp ( AreaApps ) areaApps = 0;

		if ( !areas.Lock ( "HandleQueryDevices" ) )
			goto Finish;

		const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( pareaName );

		if ( areaIt != areas.list.end () )
			areaApps = areaIt->second;

		areas.Unlock ( "HandleQueryDevices" );
				
		if ( !areaApps || !areaApps->Lock ( "HandleQueryDevices" ) ) {
			CLogArg ( "HandleQueryDevices [ %i ]: Area [%s] not found.", sourceClient->socket, areaName );
			goto Finish;
		}

		apps = &areaApps->apps;

		for ( msp ( string, ApplicationDevices )::iterator ita = apps->begin(); ita != apps->end (); ita++ )
		{
			appDevices = ita->second;
			if ( !appDevices || appDevices == sourceDevice->rootSP || !appDevices->count )
				continue;

			availableDevices += appDevices->count;

			if ( !CollectDevices ( sourceClient.get (), startIndex, deviceIDReq, areaName, appName,
				appDevices.get (), resultList, resultCount ) ) {
					break;
			}
		}

		areaApps->Unlock ( "HandleQueryDevices" );
	}

Finish:
	error = false;
    response = (MediatorQueryResponse *) FlattenDeviceList ( resultList, resultCount );
    if ( !response ) {
		query->size = length;
        query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
        query->opt0 = MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR;
		goto SendResponse;
	}

        
	sendBuffer = (char *) response;
    
	//response->cmd0 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
	response->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY_RESPONSE;
	
	response->cmd0 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
	response->deviceHead.deviceCountAvailable = availableDevices;
	response->deviceHead.startIndex = startIndex;
	response->deviceHead.deviceCount = resultCount;
	
	length = DEVICES_HEADER_SIZE + (resultCount * DEVICE_PACKET_SIZE);
	response->size = length;

SendResponse:	
	if ( error ) {
		sendBuffer = msg;
        query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
        query->opt0 = MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR;
		length = query->size = 8;
	}

	CLogArgID ( "HandleQueryDevices [ %i ]: send devicelist%s [%d bytes -> %s]", sourceClient->socket, sendBuffer [ 5 ] == 'c' ? "/count" : "", length, inet_ntoa ( sourceClient->addr.sin_addr ) );
	
	int sentBytes = SendBuffer ( sourceClient.get (), sendBuffer, length );

	if ( sendBuffer != msg )
		free ( sendBuffer );
	
	if ( sentBytes != length ) {
		CErrArgID ( "HandleQueryDevices [ %i ]: Failed to send devicelist response to IP [%s]", sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
		LogSocketError ();
		return false;
	}

	return true;
}



void MediatorDaemon::HandleCLSGenHelp ( ThreadInstance * client )
{
	char buffer [ ENVIRONS_MAX_KEYSIZE ];

	unsigned int deviceID = client->deviceID;
    CLogID ( "HandleCLSGenHelp: CLS cert/key help requested." );
    
    char * cert = 0, * key = 0;
    
    if ( !GenerateCertificate ( &key, &cert ) || !cert || !key ) {
        return;
    }
    
    unsigned int certlen = *((unsigned int *) cert) & 0xFFFF;
    unsigned int keylen = *((unsigned int *) key) & 0xFFFF;
    
    buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
    buffer [ 5 ] = MEDIATOR_CMD_HELP_TLS_GEN;
    buffer [ 6 ] = MEDIATOR_OPT_NULL;
    buffer [ 7 ] = MEDIATOR_OPT_NULL;
    
    int sentSize = certlen + keylen + 16;
    //*((unsigned int *) buffer) = (unsigned int) sentSize;
	memcpy ( buffer, &sentSize, sizeof(sentSize) ); // to prevent strict-alias bugs (due to compiler opts)
    
    memcpy ( buffer + 8, cert, certlen + 4 );
    memcpy ( buffer + 8 + certlen + 4, key, keylen + 4 );
    
    int sentBytes = SendBuffer ( client, buffer, sentSize );
    
	if ( sentBytes != sentSize ) {
		CErrArg ( "[0x%X].HandleCLSGenHelp: Failed to send certificate/key to [%s]!", client->deviceID, inet_ntoa ( client->addr.sin_addr ) );
	}
    
    free ( cert );
    free ( key );
}


void MediatorDaemon::HandleCertSign ( ThreadInstance * client, char * msg )
{
    
	char buffer [ ENVIRONS_MAX_KEYSIZE ];
    
	unsigned int deviceID = client->deviceID;
    CLogID ( "HandleCertSign: Cert signature requested." );
    
    char * cert = 0;
    
    if ( !SignCertificate ( privKey, privKeySize, msg + 8, &cert ) || !cert ) {
        CLogID ( "HandleCertSign: Signing failed." );

		/// We draw on the assumption that signing may be not implemented yet. Generate valid cert/keys and send that to the client
		HandleCLSGenHelp ( client );
        return;
    }
    
    unsigned int certlen = *((unsigned int *) cert) & 0xFFFF;
    
    buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
    buffer [ 5 ] = MEDIATOR_CMD_HELP_TLS_GEN;
    buffer [ 6 ] = MEDIATOR_OPT_SIGNED_CERT_RESPONSE;
    buffer [ 7 ] = MEDIATOR_OPT_NULL;
    
    int sentSize = certlen + 8;
    //*((unsigned int *) buffer) = (unsigned int) sentSize;
	memcpy ( buffer, &sentSize, sizeof(sentSize) );  // to prevent strict-alias bugs (due to compiler opts)
    
    memcpy ( buffer + 8, cert, certlen + 4 );
    
    int sentBytes = SendBuffer ( client, buffer, sentSize );
    
	if ( sentBytes != sentSize ) {
		CErrArg ( "[0x%X].HandleCertSign: Failed to send signed certificate to [%s]!", client->deviceID, inet_ntoa ( client->addr.sin_addr ) );
	}
    free ( cert );
}


bool MediatorDaemon::HandleSTUNRequest ( ThreadInstance * sourceClient, char * msg )
{
	char buffer [ 32 ];

	sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
	if ( !sourceDeviceSP )
		return false;
	DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ( );
	
	// SourceID -> DestID
	unsigned int * pUI		= (unsigned int *) (msg + 8);
	unsigned int SourceID	= *pUI; pUI++;
	unsigned int DestID		= *pUI;

	CVerbArg ( "[0x%X].HandleSTUNRequest [ %i ]: TCPrec -> [0x%X]", SourceID, sourceClient->socket, DestID );

	// find the source client
	sp ( ThreadInstance ) destClient;

	sp ( ApplicationDevices ) rootSP = sourceDevice->rootSP;

	if ( !rootSP || !rootSP->Lock ( "HandleSTUNRequest" ) )
		return false;
	
	DeviceInstanceNode * device = GetDeviceInstance ( DestID, rootSP->devices );
	
	if ( device )
		destClient = device->clientSP;

	if ( !destClient ) {
		CWarnArg ( "[0x%X].HandleSTUNRequest [ %i ] -> Destination device -> [0x%X] not found.", SourceID, sourceClient->socket, DestID );
		
		rootSP->Unlock ( "HandleSTUNRequest" );
		return false;
	}
    
	buffer [ 4 ] = MEDIATOR_CMD_STUN;
	buffer [ 5 ] = MEDIATOR_OPT_NULL;
	buffer [ 6 ] = MEDIATOR_OPT_NULL;
	buffer [ 7 ] = MEDIATOR_OPT_NULL;

	pUI = (unsigned int *) buffer;
	*pUI = MEDIATOR_STUN_RESP_SIZE; pUI += 2;

	*pUI = SourceID; pUI++;
	*pUI = (unsigned int) sourceClient->addr.sin_addr.s_addr; pUI++;
	*pUI = sourceClient->portUdp;

	CLogArg ( "[0x%X].HandleSTUNRequest [ %i ]: send STUN request to device IP [%s]!", DestID, sourceClient->socket, inet_ntoa ( destClient->addr.sin_addr ) );
	
	int sentBytes = SendBuffer ( destClient.get (), buffer, MEDIATOR_STUN_RESP_SIZE );
	if ( sentBytes != MEDIATOR_STUN_RESP_SIZE ) {
		CErrArg ( "[0x%X].HandleSTUNRequest [ %i ]: Failed to send STUN request to device IP [%s]!", DestID, sourceClient->socket, inet_ntoa ( destClient->addr.sin_addr ) );
		//return false;
	}
	
	buffer [ 3 ] = '-';
	pUI = (unsigned int *) buffer; pUI++;
	*pUI = DestID; pUI++;
	*pUI = (unsigned int) destClient->addr.sin_addr.s_addr; pUI++;
	*pUI = destClient->portUdp; pUI++;
	*pUI = 0;
	
	CLogArg ( "[0x%X].HandleSTUNRequest [ %i ]: send STUN reply to device IP [%s]!", DestID, sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
	
	sentBytes = SendBuffer ( sourceClient, buffer, MEDIATOR_STUN_RESP_SIZE );

	rootSP->Unlock ( "HandleSTUNRequest" );

	if ( sentBytes != MEDIATOR_STUN_RESP_SIZE ) {
		CErrArg ( "[0x%X].HandleSTUNRequest [ %i ]: Failed to send STUN reply to device IP [%s]!", DestID, sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
		return false;
	}

	return true;
}


bool MediatorDaemon::HandleSTUNRequest ( ThreadInstance * destClient, int sourceID, const char * areaName, const char * appName, unsigned int IPe, unsigned int Porte )
{
#ifdef MEDIATOR_LIMIT_STUNT_REG_REQUESTS
    INTEROPTIMEVAL now = GetEnvironsTickCount();
    
    INTEROPTIMEVAL sendDiff = ( now - destClient->stunLastSend );
    if ( sendDiff < 300 )
        return true;
    
    destClient->stunLastSend = now;
#endif
    
	STUNReqReqPacket	request;
	Zero ( request );

	CVerbArg ( "[0x%X].HandleSTUNRequest: UDPrec -> TCPsend [0x%X]", sourceID, destClient->deviceID );

	request.size = sizeof ( request );
	memcpy ( request.ident, "y;;;", 4 );

	request.deviceID = sourceID;
	request.IPe = IPe;
	request.Porte = ( unsigned short ) Porte;
    
    sp ( DeviceInstanceNode ) deviceSP = destClient->deviceSP;
	if ( deviceSP ) {
		request.IPi     = deviceSP->info.ip;
		request.Porti   = deviceSP->info.udpPort;
	}

	strlcpy ( request.areaName, areaName, sizeof ( request.areaName ) );
	strlcpy ( request.appName, appName, sizeof ( request.appName ) );

	CLogArg ( "[0x%X].HandleSTUNRequest: Send STUN request to device IP [%s] Port [%u/%u]!", destClient->deviceID, inet_ntoa ( destClient->addr.sin_addr ), Porte, request.Porti );

	int sentBytes = SendBuffer ( destClient, &request, sizeof ( request ) );
	if ( sentBytes != sizeof ( request ) )
	{
		CErrArg ( "[0x%X].HandleSTUNRequest: Failed to send STUN request to device IP [%s]!", destClient->deviceID, inet_ntoa ( destClient->addr.sin_addr ) );
		return false;
	}

	return true;
}


bool MediatorDaemon::NotifySTUNTRegRequest ( ThreadInstance * client )
{
    int deviceID = client->deviceID;
    
#ifdef MEDIATOR_LIMIT_STUNT_REG_REQUESTS
    INTEROPTIMEVAL now = GetEnvironsTickCount();
    
    INTEROPTIMEVAL sendDiff = ( now - client->stuntLastSend );
    if ( sendDiff < 200 )
        return true;
    
    client->stuntLastSend = now;
#endif
    
	CVerbID ( "NotifySTUNTRegRequest" );

	STUNTRegReqPacket	req;
	Zero ( req );
	
	/// Notify a spare socket registration request

	req.size    = MEDIATOR_CMD_MEDIATOR_NOTIFY_SIZE;
	req.version = MEDIATOR_PROTOCOL_VERSION;

	memcpy ( req.ident, "i;;", 3 );

	req.notify = NOTIFY_MEDIATOR_SRV_STUNT_REG_REQ;
	    
	CLogArgID ( "NotifySTUNTRegRequest: Send spare socket register request to device [ %s ]", inet_ntoa ( client->addr.sin_addr ) );

	int sentBytes = SendBuffer ( client, &req, MEDIATOR_CMD_MEDIATOR_NOTIFY_SIZE, false );
		
	if ( sentBytes != MEDIATOR_CMD_MEDIATOR_NOTIFY_SIZE ) {
		CErrArgID ( "NotifySTUNTRegRequest: Failed to send spare socket register request to device IP [ %s ]!", inet_ntoa ( client->addr.sin_addr ) ); LogSocketError ();
		return false;
	}

	return true;
}


bool MediatorDaemon::HandleSTUNTRequest ( ThreadInstance * sourceClient, STUNTReqPacket * req )
{
	sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;
	if ( !sourceDeviceSP )
		return false;
	DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ( );

	int					sentBytes;

	STUNTRespReqPacket	response;
	Zero ( response );

	STUNTRespPacket	*	reqResponse = (STUNTRespPacket *) &response;

	unsigned int		sendSize		= 0;
	unsigned int		IP				= 0, IPe = 0;

	char				channelType		= req->channel;
	bool				extResp			= (channelType != ';');
	
	unsigned short		portSource;
	unsigned short		portDest;
	int                 deviceID		= sourceDevice->info.deviceID; // *pUI;
    int                 destID          = req->deviceID;

	bool				sourceLocked	= false;
	bool				destLocked		= false;
	bool				unlockAppDevice	= false;
	int					status			= 0;
    const char  *		areaName        = 0;
    const char  *		appName         = 0;
	DeviceInstanceNode *	destDevice		= 0;
	sp ( DeviceInstanceNode ) destDeviceSP	= 0;
    
	// find the destination client
	sp ( ThreadInstance )	destClient		= 0;	
    sp ( ApplicationDevices ) appDevices	= 0;
    
    if ( req->size >= sizeof(STUNTReqPacket) ) {
        if ( *req->areaName )
            areaName = req->areaName;
        if ( *req->appName )
            appName = req->appName;
    }
    
    if ( !areaName || !appName ) {
        //areaName = sourceDevice->info.areaName;
        //appName = sourceDevice->info.appName;

		appDevices = sourceDevice->rootSP;
    }
    else {
        appDevices = GetApplicationDevices ( areaName, appName );
		unlockAppDevice = true;
    }

	if ( !appDevices )
		goto Quit;

	if ( !appDevices->Lock ( "HandleSTUNTRequest" ) ) {
		if ( unlockAppDevice )
			UnlockApplicationDevices ( appDevices.get () );
		goto Quit;
	}

	if ( unlockAppDevice )
		UnlockApplicationDevices ( appDevices.get () );
	
	destDevice = GetDeviceInstance ( destID, appDevices->devices );
	if ( destDevice ) {
		destDeviceSP = destDevice->baseSP;
		destClient = destDevice->clientSP;
	}

	appDevices->Unlock ( "HandleSTUNTRequest" );

	if ( !destClient || !destDeviceSP ) {
		CErrArgID ( "HandleSTUNTRequest [ %i ]: Failed to find device connection for id [0x%X]!", sourceClient->socket, destID ); goto Quit;
    }
    
    sourceClient->stuntTarget = destClient.get();
    
    if ( destClient->stuntTarget == sourceClient ) {
        CVerbArg ( "HandleSTUNTRequest [ %i ]: The destination client is already stunt connecting to us.", sourceClient->socket );
        sourceClient->stuntTarget = 0;
        
        status = -1; goto Quit;
    }
	
	// Acquire the mutex on sourceClient
	if ( !sourceClient->Lock ( "HandleSTUNTRequest" ) ) goto UnlockQuit;
	sourceLocked = true;

	///
	/// Check whether the source client is ready for a stunt
	///
	portSource = sourceClient->sparePort;
	if ( !portSource ) {
		CLogArgID ( "HandleSTUNTRequest [ %i ]: Source client has not renewed the spare socket yet!", sourceClient->socket );

		NotifySTUNTRegRequest ( sourceClient ); status = -1; goto UnlockQuit;
	}

	IP = sourceDevice->info.ip;
	//IPe = sourceDevice->IPe; // the IPe of deviceInfo could potentially be collected by broadcast from sometime before...
	// use the ip address of the source received


	// Acquire the mutex on destClient
	if ( !destClient->Lock ( "HandleSTUNTRequest" ) ) goto UnlockQuit;
	destLocked = true;
	
	///
	/// Check whether the destination client is ready for a stunt
	///
	portDest = destClient->sparePort;
	if ( !portDest ) {
		CLogArgID ( "HandleSTUNTRequest [ %i ]: Destination client [0x%X] has not renewed the spare socket yet!", sourceClient->socket, destID );

		NotifySTUNTRegRequest ( destClient.get () );  status = -1; goto UnlockQuit;
	}
	
	IPe = sourceClient->addr.sin_addr.s_addr;

	if ( !IP || !IPe ) {
		CErrArgID ( "HandleSTUNTRequest [ %i ]: Invalid ip [%s] IPe [%x] Port [%d] for sourceDevice or invalid Port[%d] for destinationDevice in database!", sourceClient->socket,
			inet_ntoa ( *((struct in_addr *) &IP) ), IPe, portSource, portDest );
		goto UnlockQuit;
    }

	response.size = sizeof(response);

    response.ident [ 0 ] = MEDIATOR_CMD_STUNT;
    response.ident [ 1 ] = MEDIATOR_OPT_NULL;
    response.ident [ 2 ] = MEDIATOR_OPT_NULL;
    
	//memcpy ( response.ident, "x;;", 3 );
    
	response.channel    = channelType;
	response.deviceID   = sourceClient->deviceID;
	response.ip         = IP;
	response.ipe        = IPe;
    response.porti      = sourceDevice->info.tcpPort;
    response.porte      = portSource;

	strlcpy ( response.areaName, sourceDevice->info.areaName, sizeof ( response.areaName ) );
	strlcpy ( response.appName, sourceDevice->info.appName, sizeof ( response.appName ) );
	
	CLogArgID ( "STUNTRequest [ %i ]: Send request to device [0x%X] IP [%s], port [%d]", sourceClient->socket, destID, inet_ntoa ( destClient->addr.sin_addr ), portSource );
	
	sentBytes = SendBuffer ( destClient.get (), &response, sizeof(response), false );

	if ( sentBytes != sizeof(response) ) {
		CErrArgID ( "HandleSTUNTRequest [ %i ]: Failed to send STUNT request to device [0x%X] IP [%s]!", sourceClient->socket, destID, inet_ntoa ( destClient->addr.sin_addr ) );
		LogSocketError ();
		goto UnlockQuit;
	}

	sendSize = extResp ? MEDIATOR_STUNT_ACK_EXT_SIZE : MEDIATOR_STUNT_ACK_SIZE;
	reqResponse->size = sendSize;

	reqResponse->respCode = 'p';

    reqResponse->porte = portDest;
    reqResponse->porti = destDeviceSP ? destDeviceSP->info.tcpPort : 0;
	//reqResponse->portUdp = destClient->portUdp;
	
	if ( extResp ) {
		reqResponse->ip = destDeviceSP->info.ip;
		reqResponse->ipe = destClient->addr.sin_addr.s_addr;
	}

	CLogArgID ( "STUNTRequest [ %i ]: Send response to device [0x%X] IP [%s:%d]", sourceClient->socket, destID, inet_ntoa ( sourceClient->addr.sin_addr ), portDest );

	destClient->sparePort = 0; // Clear the port
		
	sentBytes = SendBuffer ( sourceClient, reqResponse, sendSize, false );

	if ( sentBytes != (int) sendSize ) {
		CErrArgID ( "HandleSTUNTRequest [ %i ]: Failed to send STUNT response (Port) to sourceClient device IP [%s]!", sourceClient->socket, inet_ntoa ( sourceClient->addr.sin_addr ) );
        LogSocketError ();
        goto UnlockQuit;
	}

	destClient->sparePort   = 0;
	sourceClient->sparePort = 0;

    status = 1;
    sourceClient->stuntTarget = 0;
    
UnlockQuit:	
	/// Release the mutex on destClient
	if ( destLocked )
		destClient->Unlock ( "HandleSTUNTRequest" );
	
	/// Release the mutex on sourceClient
	if ( sourceLocked )
		sourceClient->Unlock ( "HandleSTUNTRequest" );
	
Quit:
	if ( status > 0 )
		return true;

	reqResponse->size = MEDIATOR_STUNT_ACK_SIZE;

	if ( status < 0 )
		reqResponse->respCode = 'r';
	else		
		reqResponse->respCode = 'e';
	
	sentBytes = SendBuffer ( sourceClient, reqResponse, MEDIATOR_STUNT_ACK_SIZE );
	
	if ( sentBytes != MEDIATOR_STUNT_ACK_SIZE ) {
		CErrArgID ( "HandleSTUNTRequest [ %i ]: Failed to send %s message to sourceClient", sourceClient->socket, reqResponse->respCode == 'e' ? "Failed" : "Retry" );
		LogSocketError ();
    }
    sourceClient->stuntTarget = 0;
	
	return false;
}


void MediatorDaemon::BuildBroadcastMessage ( )
{
	CVerb ( "buildBroadcastMessage" );

	// Format E.Mediator port1 port2 ...
	strlcpy ( broadcastMessage + 4, MEDIATOR_BROADCAST, MEDIATOR_BROADCAST_DESC_START );
	
	broadcastMessageLen = 16;

	unsigned short * pShortBuffer = (unsigned short *) (broadcastMessage + broadcastMessageLen);
	
	*pShortBuffer = (unsigned short) ports [ 0 ];
	
	broadcastMessageLen += 4;

    unsigned int * pInt = (unsigned int *) (broadcastMessage + broadcastMessageLen);
	*pInt = 1; // We use the platform 1 for now


	unsigned int bcml = broadcastMessageLen + 4;

	broadcastMessage [ bcml ] = 0;

	//*((unsigned int *) broadcastMessage) = broadcastMessageLen + 4;
	memcpy ( broadcastMessage, &bcml, sizeof(bcml) );  // to prevent strict-alias bugs (due to compiler opts)
}


int MediatorDaemon::ScanForParameters ( char * buffer, unsigned int maxLen, const char * delim, char ** params, int iParams )
{
	CVerbVerb ( "ScanForParameters" );

	int i = 0;

	char * context = NULL;
	char * psem = strtok_s ( buffer, delim, &context );
  
	while ( psem != NULL )
	{
		params [ i ] = psem;
			
		i++;
		if ( i >= (iParams - 1) ) {
			unsigned int length = (unsigned int) ((psem + strlen ( psem )) - buffer + 1);
			if ( length < maxLen ) {
				params [ i ] = psem + strlen ( psem ) + 1;			
				i++;
			}
			break;
		}

		psem = strtok_s ( NULL, delim, &context );
	}

	return i;
}


void * MediatorDaemon::BroadcastThread ( )
{
	int ret, bytesReceived;
    socklen_t addrLen;
	char buffer [ BUFFERSIZE ];
	
	CVerb ( "BroadcastThread started." );

	if ( broadcastSocketID <= 0 ) {
		CErr ( "BroadcastThread: Invalid broadcast socket!" );
		return 0;
	}
	
	struct 	sockaddr_in		listenAddr;
	memset ( &listenAddr, 0, sizeof(listenAddr) );
	
	listenAddr.sin_family		= AF_INET;
	listenAddr.sin_addr.s_addr	= INADDR_ANY; //htonl ( INADDR_BROADCAST ); // INADDR_ANY );
    listenAddr.sin_port         = htons ( DEFAULT_BROADCAST_PORT );

	ret = ::bind ( broadcastSocketID, (struct sockaddr *) &listenAddr, sizeof(listenAddr) );
	if ( ret < 0 ) {
		CErrArg ( "BroadcastThread: Failed to bind broadcast listener socket to port %i!", DEFAULT_BROADCAST_PORT );
		LogSocketError ();
		//return 0;
	}
    else {
        CVerbArg ( "BroadcastThread bound socket to port %i", DEFAULT_BROADCAST_PORT );
    }

	// Send Broadcast
	SendBroadcast ();

	// Wait for broadcast
	addrLen = sizeof(listenAddr);
    
    CLogArg ( "BroadcastThread listen on port %i", DEFAULT_BROADCAST_PORT );
    
	while ( isRunning ) {
		bytesReceived = (int)recvfrom ( broadcastSocketID, buffer, BUFFERSIZE, 0, (struct sockaddr*) &listenAddr, &addrLen );

		if ( bytesReceived < 0 ) {
			CLog ( "BroadcastThread: Socket has been closed!" );
			break;
		}

		if ( bytesReceived < 10 || buffer [ 0 ] != 'E' || buffer [ 1 ] != '.' || bytesReceived >= BUFFERSIZE )
			continue;
				
		buffer [ bytesReceived ] = 0;

		if ( buffer [ 2 ] == 'D' ) {
			// if message contains EHLO, then broadcast our data
			if ( buffer [ 4 ] == 'H' ) {
				SendBroadcast ();
				CVerbArg ( "BroadcastThread: Replied to client request from %s;\n\t\tMessage: %s", inet_ntoa ( *((struct in_addr *) &(listenAddr.sin_addr.s_addr)) ), buffer);
			}
		}
		else {
			CVerbArg ( "BroadcastThread: read %d bytes <- IP %s;\n\t\tMessage: %s", bytesReceived, inet_ntoa ( *((struct in_addr *) &(listenAddr.sin_addr.s_addr)) ), buffer);
		}
	}
   
	CLog ( "BroadcastThread: Thread terminated." );

	return 0;
}


bool MediatorDaemon::UpdateDeviceRegistry ( sp ( DeviceInstanceNode ) device, unsigned int ip, char * msg )
{
	CVerb ( "UpdateDeviceRegistry" );

    char * keyCat;
    char * value;

	if ( !device.get () )
		return false;

	// Update the values in our database

	char * pareaName = device->info.areaName;
		
	if ( !pareaName || !areasMap.Lock ( "UpdateDeviceRegistry" ) ) {
		CErr ( "UpdateDeviceRegistry: Failed to aquire mutex on areas/values (alt: areaName invalid)! Skipping client request!" );

		return false;
	}
		
	// Search for the area at first
	sp ( AppsList )						appsList = 0;

	sp ( ListValues )                  values = 0;

		
	string areaName ( pareaName );
	string appName ( device->info.appName );

	const msp ( string, AppsList )::iterator areaIt = areasMap.list.find ( areaName );

	if ( areaIt == areasMap.list.end () )
	{		
		appsList = make_shared < AppsList > ();
		if ( !appsList || !appsList->Init () ) {
			CErrArg ( "UpdateDeviceRegistry: Failed to create new area [%s].", pareaName );
            
            areasMap.Unlock ( "UpdateDeviceRegistry" );

			return false;
		}
		areasMap.list [ areaName ] = appsList;
	}
	else
        appsList = areaIt->second;
    
    areasMap.Unlock ( "UpdateDeviceRegistry" );

    
    if ( !appsList || !appsList->Lock ( "UpdateDeviceRegistry" ) ) {
        return false;
    }
    
	const msp ( string, ListValues )::iterator appsIt = appsList->apps.find ( appName );

	if ( appsIt == appsList->apps.end () )
	{		
        values = make_shared < ListValues > (); // new map<string, ValuePack*> ();
		if ( !values || !values->Init () ) {
			CErrArg ( "UpdateDeviceRegistry: Failed to create new application [%s].", appName.c_str () );
			goto Continue;
		}
		appsList->apps [ appName ] = values;
	}
	else
		values = appsIt->second;

	appsList->Unlock ( "UpdateDeviceRegistry" );

	if ( !values || !values->Lock ( "UpdateDeviceRegistry" ) )
		return false;
	

	char keyBuffer [ 128 ];
	char valueBuffer [ 256 ];
	if ( snprintf ( keyBuffer, 128, "%i_", device->info.deviceID ) < 0 )
        goto Failed;

	keyCat = keyBuffer + strlen ( keyBuffer );
	
	// ip in register message
	strlcat ( keyCat, "ip", 100 );

	value = inet_ntoa ( *((struct in_addr *) &device->info.ip) );

	if ( !addToArea ( values, keyBuffer, value, (unsigned int) strlen ( value ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}		
	
	// extern ip
	*keyCat = 0;
	strlcat ( keyCat, "ipe", 100 );

	value = inet_ntoa ( *((struct in_addr *) &ip) );

	if ( !addToArea ( values, keyBuffer, value, (unsigned int) strlen ( value ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}		
	
	// tcp port
	*keyCat = 0;
	strlcat ( keyCat, "cport", 100 );
    if ( snprintf ( valueBuffer, 128, "%u", device->info.tcpPort ) < 0 )
        goto Failed;

	if ( !addToArea ( values, keyBuffer, valueBuffer, (unsigned int) strlen ( valueBuffer ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}		
	
	// udp port
	*keyCat = 0;
	strlcat ( keyCat, "dport", 100 );
    if ( snprintf ( valueBuffer, 128, "%u", device->info.udpPort ) < 0 )
        goto Failed;

	if ( !addToArea ( values, keyBuffer, valueBuffer, (unsigned int) strlen ( valueBuffer ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}
	
	// device type
	*keyCat = 0;
	strlcat ( keyCat, "type", 100 );
    if ( snprintf ( valueBuffer, 128, "%i", device->info.platform ) < 0 )
        goto Failed;

	if ( !addToArea ( values, keyBuffer, valueBuffer, (unsigned int) strlen ( valueBuffer ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}
	
	// device name
	*keyCat = 0;
	strlcat ( keyCat, "name", 100 );

	if ( !addToArea ( values, keyBuffer, device->info.deviceName, (unsigned int) strlen ( device->info.deviceName ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}
	
Failed:
	values->Unlock ( "UpdateDeviceRegistry" );

Continue:
    configDirty = true;

	return true;
}


bool MediatorDaemon::HandleDeviceRegistration ( sp ( ThreadInstance ) clientSP, unsigned int ip, char * msg )
{
	CVerb ( "HandleDeviceRegistration" );
	
    MediatorReqMsg  regMsg;
    int             sentBytes;
    
    if ( !clientSP ) {
        CErr ( "HandleRegistration:\tInvalid client!." );
		return false;
    }
    
    ThreadInstance  * client = clientSP.get();

	int deviceID = client->deviceID;
    if ( !deviceID ) {
        CErrArg ( "HandleRegistration [ %s ]:\tInvalid deviceID.", client->ips );
		return false;
    }

	// Clear all lingering devices in the list with the same deviceID

	if ( !RemoveAcceptClient ( client ) ) {
		CErrArgID ( "HandleDeviceRegistration [ %s ]:\tFailed to lookup the client in acceptClients.", client->ips );
		return false;
	}

	char		*	uid		= 0;
	bool			created = false;

	sp ( DeviceInstanceNode ) deviceSP = UpdateDevices ( ip, msg, &uid, &created );
	if ( !deviceSP ) {
		CErrArgID ( "HandleDeviceRegistration [ %s ]:\tFailed to parse registration.", client->ips );
		return false;
	}
	if ( !created ) {
		MediatorMsg resp;
		Zero ( resp );
		
		resp.size = sizeof(MediatorMsg);
		resp.cmd1 = MEDIATOR_OPT_NULL;
		resp.opt0 = MEDIATOR_OPT_NULL;
		resp.opt1 = MEDIATOR_OPT_NULL;
		
		sp ( ThreadInstance ) busySP = deviceSP->clientSP;
		if ( busySP )
        {
            // Check socket
            if ( !IsSocketAlive ( busySP->socket ) ) {
                VerifySockets ( busySP.get (), false );
                
                /// Return try again
                resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_RETRY;
            }
            else {
                if ( (checkLast - busySP->aliveLast) > 0 )
                {
                    /// Request alive response
                    resp.cmd0 = MEDIATOR_SRV_CMD_ALIVE_REQUEST;
                    
                    busySP->aliveLast -= 2;
                    SendBuffer ( busySP.get (), &resp, resp.size );
                    
                    Sleep ( 100 );
                    
                    /// Trigger Watchdog
                    if ( MutexLockA ( thread_lock, "Watchdog" ) ) {
                        
                        if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
                            CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tWatchdog signal failed.", client->ips );
                        }
                        
                        MutexUnlockA ( thread_lock, "Watchdog" );
                    }
                    /// Return try again
                    resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_RETRY;
                }
                else {
                    /// Return slot busy
                    resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_LOCKED;
                    CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tDevice instance slot locked.", client->ips );
                }
            }            
		}
		else {
			RemoveDevice ( deviceSP.get () );

			/// Return try again
			resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_RETRY;
			CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tDevice instance slot lingering. Responded retry advice.", client->ips );
		}
			
		SendBuffer ( client, &resp, resp.size );

		CErrArgID ( "HandleDeviceRegistration [ %s ]:\tA device instance for this registration already exists.", client->ips );
		return false;
	}

	client->deviceSP = deviceSP->baseSP;

	DeviceInstanceNode * device = deviceSP.get ();
	device->clientSP = clientSP;

	*client->uid = 0;
	if ( uid && *uid && device->info.deviceID )
    {
		// Transfor lowercase of uid
		string suid ( uid );
		std::transform ( suid.begin(), suid.end(), suid.begin(), ::tolower );
        
        CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tUsing lowercase uid [%s].", client->ips, suid.c_str () );

		strlcpy ( client->uid, suid.c_str (), sizeof ( client->uid ) );
        
		if ( deviceMappings.Lock ( "HandleDeviceRegistration" ) )
		{			
            sp ( DeviceMapping ) mapping;
            
            Zero ( regMsg );
            
            CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tLooking for mapping of uid [%s].", client->ips, client->uid );
            
			const msp ( string, DeviceMapping )::iterator devIt = deviceMappings.list.find ( string ( client->uid ) );

            if ( devIt != deviceMappings.list.end ( ) ) {
                CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tFound.", client->ips );
				mapping = devIt->second;
			}
            else {
				mapping.reset ( new DeviceMapping );
				//mapping = make_shared < DeviceMapping > (); //sp ( DeviceMapping ) ( new DeviceMapping ); // calloc ( 1, sizeof(DeviceMapping) );
				if ( !mapping )
					goto PreFailExit;
                memset ( mapping.get (), 0, sizeof(DeviceMapping) );
                
                CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tNot found. Created new.", client->ips );
			}
			mapping->deviceID = device->info.deviceID;
            
            /// Check for authToken
            if ( !*mapping->authToken || client->createAuthToken )
            {
                CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tCreating new auth token.", client->ips );
                
				srand ( getRandomValue ( &client ) );

                unsigned int tokCount = sizeof(mapping->authToken) - (rand () % 5);
                
                for ( unsigned int i=0; i<tokCount; i++ ) {
                    mapping->authToken [ i ] = (rand () % 93) + 33;
                }
				mapping->authToken [ tokCount ] = 0;
                
                regMsg.size = sizeof(MediatorMsg) + tokCount - sizeof(long long);
                regMsg.cmd0 = MEDIATOR_PROTOCOL_VERSION;
                regMsg.cmd1 = MEDIATOR_CMD_AUTHTOKEN_ASSIGN;
                regMsg.opt0 = MEDIATOR_OPT_NULL;
                regMsg.opt1 = MEDIATOR_OPT_NULL;
                
                memcpy ( regMsg.areaName, mapping->authToken, tokCount );
				regMsg.areaName [tokCount] = 0;
                
                mapping->authLevel = client->authLevel;
                
                CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tMapping created deviceID [0x%X] authLevel [%i] authToken [...].", client->ips, deviceID, client->authLevel );
            }

			deviceMappings.list [ string ( client->uid ) ] = mapping;

        PreFailExit:
			if ( !deviceMappings.Unlock ( "HandleDeviceRegistration" ) )
				return false;
            
            if ( !mapping ) {
                CErrArgID ( "HandleDeviceRegistration [ %s ]:\tFailed to find or create a mapping.", client->ips );
                return false;
            }
            
            if ( regMsg.size ) {
                CErrArgID ( "HandleDeviceRegistration [ %s ]:\tSending new auth token to client.", client->ips );
                
                /// Send authToken to client
				sentBytes = SendBuffer ( clientSP.get (), &regMsg, regMsg.size );
                
                if ( sentBytes != (int)regMsg.size ) {
                    CErrArgID ( "HandleDeviceRegistration [ %s ]:\tFailed to assign authtoken!", client->ips );
                    return false;
                }
            }
            
            deviceMappingDirty = true;
		}
	}

	if ( !client->version )
		client->version = *msg;

    if ( *msg < '4' )
        UpdateDeviceRegistry ( deviceSP, ip, msg );

	long long sid;

	if ( !sessions.Lock ( "HandleDeviceRegistration" ) )
		return false;

	/// Assign a session id
	sid = sessionCounter++;
	sid |= ((long long)((device->rootSP->id << 16) | device->rootSP->areaId)) << 32;

    sessions.list [sid] = clientSP;
    
    CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tAssinged session id [%i]", client->ips, sid );

	if ( !sessions.Unlock ( "HandleDeviceRegistration" ) )
		return false;

	client->sessionID = sid;

	/// Send session id to client
	MediatorMsg * sessMsg = (MediatorMsg *) &regMsg;
	
	sessMsg->size = sizeof(MediatorMsg);
	sessMsg->cmd0 = MEDIATOR_PROTOCOL_VERSION;
	sessMsg->cmd1 = MEDIATOR_CMD_SESSION_ASSIGN;
	sessMsg->opt0 = MEDIATOR_OPT_NULL;
	sessMsg->opt1 = MEDIATOR_OPT_NULL;
	sessMsg->ids.sessionID = sid;
    
    CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tSending session id [%i] to client.", client->ips, sid );
    
	sentBytes = SendBuffer ( client, sessMsg, sessMsg->size );
	if ( sentBytes != (int)sessMsg->size ) {
		CErrID ( "HandleDeviceRegistration: Failed to assign session id!" );
		return false;
	}

	return true;
}


bool MediatorDaemon::SecureChannelAuth ( ThreadInstance * client )
{
	CVerb ( "SecureChannelAuth" );

	bool			ret			= false;
	char			buffer [ ENVIRONS_MAX_KEYBUFFER_SIZE ]; /// cert + challenge + instruct + size(4)
	unsigned int *	pUI			= (unsigned int *) buffer;
	int				length		= 0;
	int				recvLength	= 0;
	int				sentBytes   = -1;
	int				addrLen		= sizeof(struct sockaddr);	

    if ( !certificate ) {
		CErr ( "SecureChannelAuth: Certificate missing!" );
		return false;
    }
	
	/// get random number
	unsigned int challenge = rand ();

	/// Send instruction (4 bytes: 1234 means multiply with 42) and random number (4 bytes)
	*pUI = 1234; pUI++;
	*pUI = challenge; pUI++;
	*pUI = MEDIATOR_PROTOCOL_VERSION; pUI++;
	length += 12;
	
	/// Send certificate (4 bytes size, x bytes cert)
	unsigned int certSize = (*((unsigned int *)certificate) & 0xFFFF) + 4; /// We add 4 bytes for the formatSize "header"
	*pUI = certSize; pUI++;
	memcpy ( pUI, certificate, certSize );
	length += certSize + 4;
    
    /// Add the padding id
    *pUI |= encPadding;

    
#ifdef USE_LOCKFREE_SOCKET_ACCESS
    int sock = (int) client->socket;
    
    if ( sock != -1 )
        sentBytes = (int) send ( sock, buffer, length, 0 );
        //sentBytes = (int)sendto ( sock, buffer, length, 0, (struct sockaddr *) &client->addr, sizeof(struct sockaddr) );
#else
	client->Lock ( "SecureChannelAuth" );

	if ( client->socket != -1 )
		sentBytes = (int)sendto ( client->socket, buffer, length, 0, (struct sockaddr *) &client->addr, sizeof(struct sockaddr) );

	client->Unlock ( "SecureChannelAuth" );
#endif

	if ( ( int ) length != sentBytes ) {
		LogSocketError ();
		CVerbArg ( "SecureChannelAuth [ %i ]: Sending of auth token failed [ %u ] != [ %i ].", client->socket, length, sentBytes );
		return false;
	}
    
    char *			recBuffer		= buffer;
    unsigned int	recBufferSize	= ENVIRONS_MAX_KEYBUFFER_SIZE - 1;
    int             msgSize			= 0;
    int				bytesReceived	= 0;
    
ReceiveNext:
    
#ifdef USE_NONBLOCK_CLIENT_SOCKET
    if ( !WaitForData ( ( int ) client->socket ) ) {
        return false;
    }

	/// Wait for response
	bytesReceived = ( int ) recvfrom ( ( int ) client->socket, recBuffer, recBufferSize, 0, ( struct sockaddr* ) &client->addr, ( socklen_t * ) &addrLen );
	if ( bytesReceived <= 0 ) {
        SOCKETRETRYGOTO ( ReceiveNext );

		CLogArg ( "SecureChannelAuth [ %i ]: Socket closed; Bytes [%i]!", client->socket, bytesReceived ); return false;
	}
#else
	/// Wait for response
	bytesReceived = ( int ) recvfrom ( ( int ) client->socket, recBuffer, recBufferSize, 0, ( struct sockaddr* ) &client->addr, ( socklen_t * ) &addrLen );
	if ( bytesReceived <= 0 ) {
		CLogArg ( "SecureChannelAuth [ %i ]: Socket closed; Bytes [%i]!", client->socket, bytesReceived ); return false;
	}
#endif

    length += bytesReceived;
    
    if ( !msgSize ) {
        unsigned int * pUI1 = reinterpret_cast<unsigned int *>( buffer );
        
        msgSize = *pUI1;
        
        if ( msgSize >= ENVIRONS_MAX_KEYBUFFER_SIZE ) {
            CErrArg ( "SecureChannelAuth [ %i ]: Message size [ %u ] would overflow receive buffer [ %u ]. Aborting transfer.", client->socket, msgSize, ENVIRONS_MAX_KEYBUFFER_SIZE );
            return false;;
        }
        
        if ( msgSize < 260 ) {
            CWarnArg ( "SecureChannelAuth [ %i ]: Expected size of response [ %i ] is less than the required 260 bytes; Bytes [ %i ]!", client->socket, msgSize, length ); return false;
        }
    }
    
    if ( length < msgSize ) {
        CVerbArg ( "SecureChannelAuth [ %i ]: Received message is not not complete.", client->socket );
        
        recBuffer += bytesReceived;
        recBufferSize -= bytesReceived;
        goto ReceiveNext;
    }
    
	recvLength = length;
	buffer [ recvLength ] = 0;
	

	/// Decrypt the response
	char *	msg = 0;
	char *	hash = 0;

	do
	{
		pUI = ( unsigned int * ) buffer;
		length = *pUI++;

		if ( length < 260 ) {
			CLogArg ( "SecureChannelAuth [ %i ]: Received response is not a correct hash; Bytes [ %i ]!", client->socket, length ); break;
		}
		length = 256; // Hard limit length to prevent buffer overrun

		unsigned int msgLen = 0;

		if ( !DecryptMessage ( privKey, privKeySize, buffer + 4, length, &msg, &msgLen ) ) {
			CWarnArg ( "SecureChannelAuth [ %i ]: Failed to decrypt response!", client->socket ); break;
		}

		/// Compute the challenge
		challenge *= 42;

		pUI = (unsigned int *) (msg + 4);
		unsigned int	hashLen		= 0;
		unsigned int	padLen		= 0;

		/// Parse the username
		char		*	userName	= 0;
		const char	*	pass		= 0;
						length		= *pUI;

		if ( reqAuth ) {
			if ( !length ) {
				CWarnArg ( "SecureChannelAuth [ %i ]: Authentication required but no username supplied!", client->socket ); break;
			}
			userName = msg + 8;
            
			client->version = *msg;
            
			if ( length & MEDIATOR_MESSAGE_LENGTH_FLAG_PAD ) {
				ClearBit ( length, MEDIATOR_MESSAGE_LENGTH_FLAG_PAD );

				padLen = GetPad ( length, MEDIATOR_MESSAGE_UNIT_ALIGN );
			}

			if ( length >= recvLength || length <= 0 ) {
				CErrArg ( "SecureChannelAuth [ %i ]: Invalid length of username [ %i ]!", client->socket, length ); break;
			}

			pUI = (unsigned int *) (userName + length + padLen);

			msgLen = *pUI; /// length of the hash
			//*pUI = 0; 
			userName [ length ] = 0;
            pUI++;
            
            /// Look through db for user
            string user = userName;

			if ( anonymousLogon ) {
				if ( !strncmp ( user.c_str (), anonymousUser, MAX_NAMEPROPERTY ) ) {
					CLogArg ( "SecureChannelAuth [ %i ]: Anonymous logon.", client->socket );
					pass = anonymousPassword;
					client->authLevel = 0;
				}
			}

			if ( !pass ) {
				std::transform ( user.begin (), user.end (), user.begin (), ::tolower );

				const map<string, UserItem *>::iterator iter = usersDB.find ( user );
				if ( iter == usersDB.end () ) {
					CLogArg ( "SecureChannelAuth [ %i ]: User [ %s ] not found.", client->socket, user.c_str () );
				}
				else {
					UserItem * item = iter->second;

					pass = item->pass.c_str ();
					client->createAuthToken = true;
					client->authLevel = item->authLevel;
				}

				if ( !pass ) {
					CVerbArg ( "SecureChannelAuth [ %i ]: No password for User [ %s ] available. Treating username as deviceUID and looking for authToken.", client->socket, user.c_str () );

					string deviceUID = userName;
					std::transform ( deviceUID.begin (), deviceUID.end (), deviceUID.begin (), ::tolower );

					if ( deviceMappings.Lock ( "SecureChannelAuth" ) )
					{
						const msp ( string, DeviceMapping )::iterator devIt = deviceMappings.list.find ( deviceUID );

						if ( devIt != deviceMappings.list.end () ) {
							CVerbArg ( "SecureChannelAuth [ %i ]: Found auth token for the deviceUID.", client->socket );
							pass = devIt->second->authToken;
							if ( !*pass )
								pass = 0;
							client->authLevel = devIt->second->authLevel;
						}
						deviceMappings.Unlock ( "SecureChannelAuth" );
					}
				}
			}

            if ( !pass ) {
                CVerbArg ( "SecureChannelAuth [ %i ]: No username and deviceUID [ %s ] found.", client->socket, userName );
                break;
            }
		}

		CVerbVerbArg ( "SecureChannelAuth [ %i ]: Auth user [ %s ] pass [ %s ]", client->socket, userName, ConvertToHexSpaceString ( pass, ( int ) strlen ( pass ) ) );

		if ( !BuildEnvironsResponse ( challenge, userName, pass, &hash, &hashLen ) ) {
			CErrArg ( "SecureChannelAuth [ %i ]: Failed to build response.", client->socket ); break;
		}

		CVerbVerbArg ( "SecureChannelAuth [ %i ]: Response [ %s ]", client->socket, ConvertToHexSpaceString ( hash, hashLen ) );

		if ( hashLen > msgLen ) {
			CWarnArg ( "SecureChannelAuth [ %i ]: Response/Challenge sizes not equal.", client->socket ); break;
		}

		/// Accept or not
		if ( memcmp ( pUI, hash, hashLen ) ) {
			CWarnArg ( "SecureChannelAuth [ %i ]: Response/Challenge invalid.", client->socket ); break;
		}

		/// If Accept, then derive session key from the computed AES response
		if ( !AESDeriveKeyContext ( hash, hashLen, &client->aes ) ) {
			CErrArg ( "SecureChannelAuth [ %i ]: Failed to derive AES session keys.", client->socket ); break;
		}

		client->encrypt = 1;

		msgLen = 4;
		memcpy ( buffer, "ae;;", msgLen );

		sentBytes = SendBuffer ( client, buffer, msgLen );

		if ( sentBytes != ( int ) msgLen ) {
			CWarnArg ( "SecureChannelAuth [ %i ]: Failed to send AES ACK message.", client->socket ); break;
		}

		client->authenticated = 1;
		ret = true;
	}
	while ( 0 );
	
    free_n ( msg );
    free_n ( hash );
    
	return ret;
}


#ifdef USE_NONBLOCK_CLIENT_SOCKET
int MediatorDaemon::SendBuffer ( ThreadInstance * client, void * msg, unsigned int msgLen, bool useLock, int retries )
#else
int MediatorDaemon::SendBuffer ( ThreadInstance * client, void * msg, unsigned int msgLen, bool useLock )
#endif
{
	CVerbVerbArg ( "SendBuffer [ %i ]", client->socket );
	
	int             rc          = -1,
                    sock;
	char *          cipher      = 0;
	unsigned int    toSendLen   = msgLen;
    int             deviceID    = client->deviceID;
    
    CVerbArgID ( "SendBuffer [ %i ]", client->socket );

    sock = (int) client->socket;
    if ( sock == -1 )
        return -1;
    
	if ( client->encrypt ) {
		if ( !AESEncrypt ( &client->aes, (char *)msg, &toSendLen, &cipher ) || !cipher ) {
			CErrArgID ( "SendBuffer [ %i ]: Failed to encrypt AES message.", client->socket );
			return rc;
		}
		msg = cipher;
    }
    
#ifdef USE_NONBLOCK_CLIENT_SOCKET
Retry:
#endif
    
    CVerbArgID ( "SendBuffer [ %i ]: [ %i ] bytes", client->socket, toSendLen );
    
#ifdef USE_LOCKFREE_SOCKET_ACCESS
    
    sock = (int) client->socket;
    
    if ( sock != -1 )
        rc = (int) send ( sock, (char *)msg, toSendLen, 0 );
        //rc = (int)sendto ( sock, (char *)msg, toSendLen, 0, (struct sockaddr *) &client->addr, sizeof(struct sockaddr) );

#   ifdef USE_NONBLOCK_CLIENT_SOCKET
    if ( rc <= 0 ) {
        if ( retries > 0 ) {
            retries--;
            goto Retry;
        }
    }
#   endif
#else

	if ( useLock )
		client->Lock ( "SendBuffer" );
    
    sock = client->socket;
    
    if ( sock != -1 )
        rc = (int)sendto ( sock, (char *)msg, toSendLen, 0, (struct sockaddr *) &client->addr, sizeof(struct sockaddr) );
	
	if ( useLock )
		client->Unlock ( "SendBuffer" );
#endif
    
    CVerbArgID ( "SendBuffer [ %i ]: [ %i ] bytes. Done.", client->socket, toSendLen );
    
	if ( cipher )
		free ( cipher );
	if ( rc == (int)toSendLen )
		return msgLen;
	return rc;
}


bool MediatorDaemon::RemoveAcceptClient ( ThreadInstance * client )
{
    bool found = false;
    
    if ( !acceptClients.Lock ( "RemoveAcceptClient" ) )
        return false;
    
    vector < sp ( ThreadInstance ) > &list = acceptClients.list;
    
    // find the one in our vector
    size_t size = list.size ();
    
    for ( size_t i = 0; i < size; ++i )
    {
        if ( list [ i ].get () == client )
        {
            CVerbArg ( "RemoveAcceptClient: Erasing [%i] from client list", i );
            
            list.erase ( list.begin() + i );
            found = true;
            break;
        }
    }
    
    acceptClients.Unlock ( "RemoveAcceptClient" );
    return found;
}


void MediatorDaemon::HandleSpareSocketRegistration ( ThreadInstance * spareClient, sp ( ThreadInstance ) orgClient, char * msg, unsigned int msgLen )
{
	int deviceID = *( (int *) (msg + 12) );
		
	CVerbID ( "HandleSpareSocketRegistration" );
		
    int							sock = -1;
#ifdef USE_LOCKFREE_SOCKET_ACCESS
    int							spareSocket = -1;
#endif

	DeviceInstanceNode *		device;
	
	sp ( DeviceInstanceNode )	deviceSP;

    RemoveAcceptClient ( spareClient );

	// We need at first the deviceID
	if ( !deviceID ) {
		CWarnID ( "HandleSpareSocketRegistration: Invalid device [0] for spare socket mapping!" ); goto Finish;
	}

	/// Verify that the registration is correct
	deviceSP = orgClient->deviceSP;
	if ( !deviceSP || ( device = deviceSP.get () ) == 0 ) {
		CWarnID ( "HandleSpareSocketRegistration: Related client is missing a device instance!" ); goto Finish;
	}
	
	if ( deviceID != device->info.deviceID ){
		CWarnID ( "HandleSpareSocketRegistration: deviceIDs don't match!" ); goto Finish;
	}	
	
	// Check for matching IP
	if ( !orgClient->daemon || spareClient->addr.sin_addr.s_addr != orgClient->addr.sin_addr.s_addr ) {
		CWarnID ( "HandleSpareSocketRegistration: Requestor has been disposed or IP address of requestor does not match!" ); goto Finish;
    }
    
#ifdef USE_LOCKFREE_SOCKET_ACCESS
    spareSocket = (int) ReplaceThreadSocket ( &spareClient->socket, -1 );
    
    ReplaceSpareSocket ( orgClient.get (), spareSocket );
    
    orgClient->sparePort = ntohs ( spareClient->addr.sin_port );
    
#else
	// Acquire the mutex on orgClient
	orgClient->Lock ( "HandleSpareSocketRegistration" );

	// Apply the port
	sock = orgClient->spareSocket;
	if ( sock != -1 ) {
        CVerbVerbArg ( "HandleSpareSocketRegistration: Closing spare soket of org client [%i].", sock );
        
        CVerbArg ( "HandleSpareSocketRegistration: Closing [ %i ]", sock );
		shutdown ( sock, 2 );
		closesocket ( sock );
    }
    
    CVerbVerbArg ( "HandleSpareSocketRegistration: Take over spare soket of spare client [%i].", spareClient->socket );
	orgClient->spareSocket = spareClient->socket;
	orgClient->sparePort = ntohs ( spareClient->addr.sin_port );
		
    spareClient->socket = -1;
	//pthread_cond_signal ( &orgClient->socketSignal );
	
	// Release the mutex on orgClient
	orgClient->Unlock ( "HandleSpareSocketRegistration" );
#endif
    
	sock = 1;
	
Finish:
	/*pthread_t thrd = spareClient->threadID;
	if ( pthread_valid ( thrd ) ) {
		pthread_detach_handle ( thrd );
	}
    */

	if ( sock != 1 ) {        
#ifdef USE_LOCKFREE_SOCKET_ACCESS
        CloseThreadSocket ( &spareClient->socket );
#else
        spareClient->Lock ( "HandleSpareSocketRegistration" );

		sock = spareClient->socket;
        if ( sock != -1 ) {
			spareClient->socket = -1;

            CVerbVerbArg ( "HandleSpareSocketRegistration: Closing spare socket of spare client [%i].", sock );
            CVerbArg ( "HandleSpareSocketRegistration: Closing [ %i ]", sock );
            
			shutdown ( sock, 2 );
			closesocket ( sock );
        }
        spareClient->Unlock ( "HandleSpareSocketRegistration" );
#endif
        
		CLogID ( "HandleSpareSocketRegistration: Failed to register spare socket" );
	}
	else {
		CLogArgID ( "HandleSpareSocketRegistration: Successfully registered spare socket on port [%d].", orgClient->sparePort );
	}
	return;
}


std::string escapeJsonString(const std::string& input) {
    std::ostringstream ss;
    for (std::string::const_iterator iter = input.begin(); iter != input.end(); iter++) {
        switch (*iter) {
            case '\\': ss << "\\\\"; break;
            case '"': ss << "\\\""; break;
            case '/': ss << "\\/"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default: ss << *iter; break;
        }
    }
    return ss.str();
}


bool MediatorDaemon::SendPushNotification ( map<string, ValuePack*> * values, int clientID, const char * value )
{ 
	bool ret = false;

	// check which push notification service is registered for the client
	
	// look for the area
	string details;
	std::stringstream ss;
			
    ss << clientID << "_pn";
	CVerbArg ( "Client: looking for details key %s!", ss.str().c_str() );

	const map<string, ValuePack*>::iterator valueIt = values->find ( ss.str() );

	if ( valueIt == values->end() ) {
		CErrArg ( "Client: push notification details for client %i not found!", clientID );
		goto Finish;
	}
	
	details = valueIt->second->value;
	if ( details.size() < 3 ) {
		CErrArg ( "Client: push notification details are invalid (<3 char): %s!", details.c_str() );
		goto Finish;
	}

	if ( details[0] == 'g' && details[1] == 'c' && details[2] == 'm' ) {
		string clientRegID ( details.begin() + 3, details.end() );

		// Do we have a GCM api key for this area?
		const map<string, ValuePack*>::iterator notifierIt = values->find ( string ( "0_gcm" ) );
		
		if ( notifierIt == values->end() ) {
			CErr ( "Client: gcm api key not found!" );
			goto Finish;
		}

		string gcmKey = notifierIt->second->value;

		string gcmURL = "android.googleapis.com/gcm/send";
		/*
		$fields = array(
	                        "registration_ids" => array ( $regId ),
	                        "data" => array ( 'msg' => $value ),
	                        );
							{
								registration_ids: [ "clientRegID" ], 
								data: [ 
									{ msg: "value" } 
									],
							}*/
		std::stringstream ssJson;
		ssJson << "{ \"registration_ids\" : [ \"" << escapeJsonString ( clientRegID ) << "\" ],";
		ssJson << "\"data\" : { \"msg\": \"" << escapeJsonString ( string ( value ) ) << "\" }, }";

		ret = HTTPPostRequest ( string ( "android.googleapis.com" ), string ( "gcm/send" ), gcmKey, ssJson.str() );
	}
	else if ( details[0] == 'a' && details[1] == 'p' && details[2] == 's' ) {
		string clientRegID ( details.begin() + 3, details.end() );

		// Do we have a GCM api key for this project?
		const map<string, ValuePack*>::iterator notifierIt = values->find ( string ( "0_aps" ) );
		
		if ( notifierIt == values->end() ) {
			CErr ( "Client: aps api key not found!" );
			goto Finish;
		}

		string gcmKey = notifierIt->second->value;

		string gcmURL = "android.googleapis.com/gcm/send";
		/*
		$fields = array(
	                        "registration_ids" => array ( $regId ),
	                        "data" => array ( 'msg' => $value ),
	                        );
							{
								registration_ids: [ "clientRegID" ], 
								data: [ 
									{ msg: "value" } 
									],
							}*/
		std::stringstream ssJson;
		ssJson << "{ \"aps\" : { ";
					ssJson << "	\"alert\" : { ";
									ssJson << "\"body\" : \"Environs wants to connect. " << escapeJsonString ( string ( value ) ) << "\",";
									ssJson << "\"action-loc-key\" : \"Connect\"";
									ssJson << "}, \"sound\" : \"default\" }, } ";

		ret = HTTPPostRequest ( string ( "android.googleapis.com" ), string ( "gcm/send" ), gcmKey, ssJson.str() );
	}
		
Finish:

	return ret;
}


void MediatorDaemon::NotifyClients ( unsigned int notifyID, sp ( DeviceInstanceNode ) &device )
{
    if ( !device )
        return;
    
	NotifyQueueContext * ctx = new NotifyQueueContext;
	if ( !ctx )
		return;

	ctx->notify     = notifyID;
	ctx->device     = device;

	if ( !MutexLockA ( notifyLock, "NotifyClients" ) ) {
        delete ctx;
        return;
	}
    
	notifyQueue.push ( ctx );
	CVerb ( "NotifyClients: Enqueue" );
    
    /// Trigger Notifier
    if ( pthread_cond_signal ( &notifyEvent ) ) {
        CVerb ( "NotifyClients: Watchdog signal failed." );
    }
    
	MutexUnlockVA ( notifyLock, "NotifyClients" );
}


void * MediatorDaemon::NotifyClientsStarter ( void * daemon )
{
    if ( daemon ) {
        MediatorDaemon * meddae = ( MediatorDaemon * ) daemon;
        meddae->NotifyClientsThread ();
    }
    return 0;
}


void MediatorDaemon::NotifyClientsThread ()
{
	CVerbVerb ( "NotifyClientsThread" );

	NotifyQueueContext * ctx = 0;

	while ( isRunning )
	{
		MutexLockVA ( notifyLock, "NotifyClientsThread" );

    Retry:
        if ( notifyQueue.empty () )
        {
            pthread_cond_wait ( &notifyEvent, &notifyLock );

            if ( !isRunning ) {
                MutexUnlockVA ( notifyLock, "NotifyClientsThread" );
                return;
            }
            goto Retry;
        }
        
        ctx = notifyQueue.front ();
        notifyQueue.pop ();
        
        MutexUnlockVA ( notifyLock, "NotifyClientsThread" );
        
        if ( ctx ) {
            NotifyClients ( ctx );
            delete ctx;
        }

		CVerbArg ( "NotifyClientsThread: next [%i]", notifyQueue.size () );
	}

	CVerbVerb ( "NotifyClientsThread: done" );
}


sp ( ApplicationDevices ) MediatorDaemon::GetApplicationDevices ( const char * areaName, const char * appName )
{
	CVerb ( "GetApplicationDevices" );
	
	if ( !areaName || !appName ) {
		CErr ( "GetApplicationDevices: Called with NULL argument." );
		return 0;
	}

	sp ( AreaApps )				areaApps	= 0;
	sp ( ApplicationDevices )	appDevices	= 0;
	
	string appsName ( appName );
	string pareaName ( areaName );

	if ( !areas.Lock ( "GetApplicationDevices" ) )
		return 0;

    const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( pareaName );

	if ( areaIt != areas.list.end ( ) )
		areaApps = areaIt->second;

	if ( !areas.Unlock ( "GetApplicationDevices" ) )
		return 0;
    
	if ( !areaApps || !areaApps->Lock ( "GetApplicationDevices" ) ) {
		CLogArg ( "GetApplicationDevices: App [%s] not found.", appName );
		return 0;
	}
		
	const msp ( string, ApplicationDevices )::iterator appsIt = areaApps->apps.find ( appsName );
				
	if ( appsIt != areaApps->apps.end ( ) )
		appDevices = appsIt->second;	
	
	areaApps->Unlock ( "GetApplicationDevices" );

	if ( !appDevices ) {
		CLogArg ( "GetApplicationDevices: Devicelist of App [%s] not found.", appName );
		CErr ( "GetApplicationDevices: Invalid appDevices!" );
		goto Finish;
	}

	if ( appDevices->access <= 0 ) {
		CWarn ( "GetApplicationDevices: appDevices is about to be disposed!" );
		appDevices = 0;
		goto Finish;
	}
	
	__sync_add_and_fetch ( &appDevices->access, 1 );
	
Finish:
	return appDevices;
}


void MediatorDaemon::UnlockApplicationDevices ( ApplicationDevices * appDevices )
{
	__sync_sub_and_fetch ( &appDevices->access, 1 );
}


void MediatorDaemon::NotifyClients ( NotifyQueueContext * nctx )
{
    if ( !nctx->device )
        return;
    
    DeviceInstanceNode  * device = nctx->device.get();
    
	MediatorNotify	msg;
	Zero ( msg );

	msg.cmd0 = MEDIATOR_PROTOCOL_VERSION;

	msg.cmd1 = MEDIATOR_CMD_MEDIATOR_NOTIFY;
	msg.opt0 = MEDIATOR_OPT_NULL;
	msg.opt1 = MEDIATOR_OPT_NULL;
	msg.msgID = nctx->notify;
	msg.notifyDeviceID = device->info.deviceID;

	unsigned int sendSize = sizeof ( MediatorNotify );

    const char * areaName = device->info.areaName;
    const char * appName = device->info.appName;

	if ( *areaName && *appName > 0 ) {
		strlcpy ( msg.areaName, areaName, sizeof ( msg.areaName ) );
		strlcpy ( msg.appName, appName, sizeof ( msg.appName ) );
		sendSize = sizeof ( MediatorNotify );
	}
	else sendSize = sizeof ( MediatorMsg );

	msg.size = sendSize;
    
    size_t                      size;
	vsp ( ThreadInstance )      dests;
    sp ( ThreadInstance )		clientSP;
    sp ( ApplicationDevices	)	appDevices;

	CLogArg ( "NotifyClients: broadcasting notify [%s]", environs::resolveName( nctx->notify ) );

#ifdef NDEBUG
	CLogArg ( "NotifyClients: broadcasting notify [0x%X]", nctx->notify );
#endif

	
	if ( !MutexLockA ( notifyTargetsLock, "NotifyClients" ) )
		return;
    
    /// Get the no filter clients
    do
    {
        msp ( long long, ThreadInstance )::iterator clientIt = notifyTargets.begin ();
        
        while ( clientIt != notifyTargets.end () )
        {
            if ( clientIt->second->socket != -1 && clientIt->second->subscribedToNotifications )
                dests.push_back ( clientIt->second );
            clientIt++;
        }
    }
    while ( 0 );

	MutexUnlockA ( notifyTargetsLock, "NotifyClients" );
    
    /// Get the AreaApps
    do
    {
		sp ( AreaApps ) areaApps = 0;

		if ( !areas.Lock ( "NotifyClients" ) )
			return;

		const msp ( string, AreaApps )::iterator areaIt = areas.list.find ( areaName );

		if ( areaIt != areas.list.end ( ) )
			areaApps = areaIt->second;

		areas.Unlock ( "NotifyClients" );

		if ( !areaApps || !areaApps->Lock1 ( "NotifyClients" ) ) {
			CLogArg ( "NotifyClients: Area [%s] not found.", areaName );
			break;
		}
        
        msp ( long long, ThreadInstance )::iterator clientIt = areaApps->notifyTargets.begin ();
        
        while ( clientIt != areaApps->notifyTargets.end () )
        {
            if ( clientIt->second->socket != -1 && clientIt->second->subscribedToNotifications )
                dests.push_back ( clientIt->second );
            clientIt++;
        }

		areaApps->Unlock1 ( "NotifyClients" );
    }
    while ( 0 );
    
	appDevices	= GetApplicationDevices ( areaName, appName );
	if ( appDevices )
	{
		if ( appDevices->Lock ( "NotifyClients" ) )
        {
            device = appDevices->devices;
			while ( device )
			{		
				clientSP = device->clientSP;

                if ( clientSP && clientSP->filterMode == MEDIATOR_FILTER_AREA_AND_APP && clientSP->socket != -1 && clientSP->subscribedToNotifications )
                {
                    dests.push_back ( clientSP );
                }
				device = device->next;
			}

			CVerbVerb ( "NotifyClients: unlock." );
			appDevices->Unlock ( "NotifyClients" );
		}
		
		UnlockApplicationDevices ( appDevices.get () );		
    }
    
	
    size = dests.size();
    
	for ( size_t i = 0; i < size; i++ )
    {
        const sp ( ThreadInstance ) &destClient = dests [ i ];
        if ( !destClient )
            continue;
        
        ThreadInstance * dest = destClient.get ();
        //CLogArg ( "NotifyClients: checking device [0x%X]", dest->deviceID );
        
        if ( !dest->deviceID || dest->socket == -1 || !dest->subscribedToNotifications )
            continue;
        
#ifdef USE_LOCKFREE_SOCKET_ACCESS
        SOCKETSYNC sock = dest->socket;

		if ( sock != -1 ) {
            CLogArg ( "NotifyClients: Notify device [0x%X]", dest->deviceID );

            send ( (int) sock, (char *) &msg, sendSize, 0 );
            //sendto ( (int) sock, (char *)&msg, sendSize, 0, (struct sockaddr *) &dest->addr, sizeof(struct sockaddr) );
        }
#else
		if ( !dest->Lock ( "NotifyClients" ) )
			continue;

        //if ( IsSocketAlive ( dest->socket ) ) {
			CLogArg ( "NotifyClients: Notify device [0x%X]", dest->deviceID );
        
			sendto ( dest->socket, (char *)&msg, sendSize, 0, (struct sockaddr *) &dest->addr, sizeof(struct sockaddr) );
		//}

        dest->Unlock ( "NotifyClients" );
#endif
	}
    
//Finish:
    dests.clear ();
    
	CVerbVerb ( "NotifyClients: done." );
}


void MediatorDaemon::CheckProjectValues ()
{
    std::time_t now = std::time(0);
    
	const unsigned long long maxAliveSecs = 60 * 60 * 1;

    if ( !areasMap.Lock ( "CheckProjectValues" ) )
        return;
    
    // Collect areas
    msp ( string, AppsList ) tmpAreas;
    for ( msp ( string, AppsList )::iterator it = areasMap.list.begin(); it != areasMap.list.end(); ++it )
    {
        sp ( AppsList ) appsList = it->second;
        if ( !appsList )
            continue;
        
        tmpAreas [ it->first ] = appsList;
    }
    
    if ( !areasMap.Unlock ( "CheckProjectValues" ) )
        return;
    
    // Save areas
    for ( msp ( string, AppsList )::iterator it = tmpAreas.begin (); it != tmpAreas.end (); ++it )
    {
        sp ( AppsList ) appsList = it->second;
        if ( !appsList )
            continue;
        
        if ( !appsList->Lock ( "CheckProjectValues" ) )
            continue;
        
        msp ( string, ListValues ) tmpListValues;
        
        for ( msp ( string, ListValues )::iterator ita = appsList->apps.begin (); ita != appsList->apps.end (); ita++ )
        {
            sp ( ListValues ) listValues = ita->second;
            if ( !listValues )
                continue;
            
            tmpListValues [ ita->first ] = listValues;
        }
        
        appsList->Unlock ( "CheckProjectValues" );
        
        
        for ( msp ( string, ListValues )::iterator ita = tmpListValues.begin (); ita != tmpListValues.end (); ita++ )
        {
            sp ( ListValues ) listValues = ita->second;
            
            if ( !listValues || !listValues->Lock ( "CheckProjectValues" ) )
                continue;
            
            
            for ( msp ( string, ValuePack )::iterator itv = listValues->values.begin (); itv != listValues->values.end (); )
            {
                bool addIt = true;
                
                sp ( ValuePack ) value = itv->second;
                if ( value ) {
                    if ( (now - value->timestamp) > (signed) maxAliveSecs ) {
                        listValues->values.erase ( itv++ );
                        addIt = false;
                    }
                }
                if ( addIt )
                    itv++;
            }
            
            listValues->Unlock ( "CheckProjectValues" );
        }
    }
}


void * MediatorDaemon::WatchdogThreadStarter ( void * daemon )
{
	if ( daemon )
		((MediatorDaemon *)daemon)->WatchdogThread ();
	return 0;
}


void MediatorDaemon::WatchdogThread ()
{
	CLog ( "Watchdog started..." );
    
#ifndef USE_LOCKFREE_SOCKET_ACCESS
	int						sock;
#endif
	sp ( ThreadInstance	)	clientSP;
	ThreadInstance		*	client				= 0;
	const unsigned int		checkDuration		= 1000 * 60 * 2; // 2 min. (in ms)
    const unsigned int      maxTimeout			= checkDuration * 3;
	const unsigned int		logRollDurationMin	= 1000 * 60 * 20; // 20 min. (in ms)
    INTEROPTIMEVAL			timeLogRollLast		= 0;
    const unsigned int		projectValuesDurationMin	= 1000 * 60 * 60; // 1 h. (in ms)
    INTEROPTIMEVAL			timeProjectValues	= 0;

    
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
    unsigned int timeout = checkDuration;
#else
    struct timeval	now;    
    struct timespec timeout;
#endif
    
	while ( isRunning )
	{	
        CVerbVerb ( "Watchdog: checking..." );
        
        if ( !sessions.Lock ( "Watchdog" ) ) break;
		
        INTEROPTIMEVAL lastCheckTime = checkLast;
        
        checkLast = GetEnvironsTickCount ();
        
        INTEROPTIMEVAL compensate = (checkLast - lastCheckTime);
        
        if ( compensate > checkDuration )
            compensate -= checkDuration;
        else
            compensate = 0;

		msp ( long long, ThreadInstance )::iterator sessionIt = sessions.list.begin();
		
        vsp ( ThreadInstance ) tmpClients;
        
        while ( sessionIt != sessions.list.end () )
        {
            tmpClients.push_back ( sessionIt->second );
			++sessionIt;
        }
        
        if ( !sessions.Unlock ( "Watchdog" ) ) break;
        
        
        vsp ( ThreadInstance )::iterator sessionItv = tmpClients.begin();
        
		while ( sessionItv != tmpClients.end () )
		{
			clientSP = *sessionItv;
            
			if ( clientSP ) {
				client = clientSP.get ();

				CVerbArg ( "Watchdog: Checking deviceID [ 0x%X : %s ] socket [ %i ]", client->deviceID, client->ips, client->socket );

				if ( (checkLast - client->aliveLast) > (maxTimeout + compensate) )
				{
					CLogArg ( "Watchdog: Disconnecting [ 0x%X : %s ]  socket [ %i ] due to expired heartbeat...", client->deviceID, client->ips, client->socket );
                    
#ifdef USE_LOCKFREE_SOCKET_ACCESS
                    CloseThreadSocket ( &client->spareSocket );
                    
                    CloseThreadSocket ( &client->socket );
#else
                    /*if ( client->Lock ( "Watchdog" ) )
                    {*/
                        sock = client->spareSocket;
                        if ( sock != -1 ) {
                            client->spareSocket = -1;
                            CVerbArg ( "Watchdog: Closing [ %i ]", sock );
                            
                            shutdown ( sock, 2 );
                            closesocket ( sock );
                        }
                        
                        sock = client->socket;
                        if ( sock != -1 ) {
                            client->socket = -1;
                            CVerbArg ( "Watchdog: Closing [ %i ]", sock );
                            
                            shutdown ( sock, 2 );
                            closesocket ( sock );
                        }
                        //client->Unlock ( "Watchdog" );
                    //}
#endif
				}
                else {
                    /*if ( client->spareSockets.size () > 0 && (checkLast - client->spareSocketTime) > maxSpareSocketAlive ) {
                        ClearSpareSockets ( client );
                    }*/
                    
#ifdef USE_VERIFYSOCKETS
					VerifySockets ( client, false );
#endif
                }
			}

			++sessionItv;
		}
        
        tmpClients.clear ();

		
		//
		// Check acceptClients
		//
		if ( !acceptClients.Lock ( "Watchdog" ) ) break;

        vector < sp (ThreadInstance) > &list = acceptClients.list;
        
		for ( size_t i = 0; i < list.size(); )
        {
			clientSP = list [i];
            if ( clientSP )
            {
				INTEROPTIMEVAL diff = (checkLast - clientSP->connectTime);

				if ( checkLast > clientSP->connectTime && diff > checkDuration )
				{
                    tmpClients.push_back ( clientSP );

					list.erase ( list.begin () + i );
				}
				else {
					i++; 
				}
            }
            else {
                CErrArg ( "Watchdog: **** Invalid Client [%i]", i );
                
                list.erase ( list.begin () + i );
            }
        }

		if ( !acceptClients.Unlock ( "Watchdog" ) ) break;
        
        
        for ( size_t i = 0; i < tmpClients.size(); ++i )
        {
            clientSP = tmpClients [i];
            if ( clientSP )
            {
                ReleaseClient ( clientSP.get () );
            }
            else {
                CErrArg ( "Watchdog: **** Invalid Client [%i]", i );
            }
        }

		if ( !MutexLockA ( thread_lock, "Watchdog" ) ) break;
		
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
#else
		gettimeofday ( &now, NULL );

		timeout.tv_sec = now.tv_sec + (checkDuration / 1000);
		timeout.tv_nsec = now.tv_usec * 1000;
#endif
		if ( ( checkLast - timeLogRollLast ) > logRollDurationMin ) {
			timeLogRollLast = checkLast;
			CloseLog ();
			OpenLog ();
        }
        
        if ( ( checkLast - timeProjectValues ) > projectValuesDurationMin ) {
            timeProjectValues = checkLast;
            CheckProjectValues ();
        }

        pthread_cond_timedwait ( &hWatchdogEvent, &thread_lock, &timeout );
        
        if ( !MutexUnlockA ( thread_lock, "Watchdog" ) )
            break;
		
		if ( usersDBDirty ) {
			usersDBDirty = false;

			SaveUserDB ();
		}

		if ( configDirty ) {
			configDirty = false;

			SaveConfig ();
        }
        
        if ( deviceMappingDirty ) {
            deviceMappingDirty = false;
            
            SaveDeviceMappings ();
        }
	}
	
	
	CVerb ( "Watchdog: bye bye..." );
}


#ifdef WIN32
std::wstring * s2ws(const std::string& s)
{
	int slength = (int)s.length () + 1;

	int len = MultiByteToWideChar ( CP_ACP, 0, s.c_str(), slength, 0, 0 ); 
	wchar_t* buf = new wchar_t [ len ];
	MultiByteToWideChar ( CP_ACP, 0, s.c_str(), slength, buf, len );
	wstring * wbuf = new wstring(buf);
	delete[] buf;

	return wbuf;
}

bool MediatorDaemon::HTTPPostRequest ( string domain, string path, string key, string jsonData )
{ 
	BOOL ret = false;

	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR responseBuffer = 0;
	wstring * wDomain = 0;
	wstring * wPath = 0;
	wstring * wsAK = 0;

	HINTERNET  httpOpen = NULL, 
				hConnect = NULL,
				hRequest = NULL;

	httpOpen = WinHttpOpen ( L"Mozilla/4.0 (compatible; MSIE 5.22)", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );

	while ( httpOpen ) {
		wDomain = s2ws ( domain );
		hConnect = WinHttpConnect ( httpOpen, wDomain->c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0 );
		if( !hConnect )
			break;
		
		wPath = s2ws ( path );
		hRequest = WinHttpOpenRequest ( hConnect, L"POST", wPath->c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE );		
		if ( !hRequest )
			break;

		std::stringstream ssAK;
		ssAK << "Authorization: key=" << key;
		wsAK = s2ws(ssAK.str());

		ret = WinHttpAddRequestHeaders  ( hRequest, wsAK->c_str(), (ULONG)wsAK->size(), WINHTTP_ADDREQ_FLAG_ADD );
		if ( !ret )
			break;

		ret = WinHttpAddRequestHeaders  ( hRequest, L"Content-Type: application/json", (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD );
		if ( !ret )
			break;

		ret = WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)jsonData.c_str(), (DWORD) jsonData.size(), (DWORD) jsonData.size(), 0 );
		if ( !ret )
			break;

		ret = WinHttpReceiveResponse( hRequest, NULL );
		if ( !ret )
			break;

		ret = false;

		do 
		{
			// Check for available data.
			dwSize = 0;
			if( !WinHttpQueryDataAvailable( hRequest, &dwSize ) ) {
				CErrArg ( "HTTPPostRequest: Error %u while query available response data.", GetLastError () );
				break;
			}

			// Allocate space for the buffer.
			responseBuffer = new char [ dwSize+1 ];
			if ( !responseBuffer )
			{
				CErrArg ( "HTTPPostRequest: Failed to allocate %i bytes for response.", dwSize + 1 );
				dwSize = 0;
			}
			else
			{
				// Read the data.
				ZeroMemory ( responseBuffer, dwSize + 1 );

				if ( !WinHttpReadData ( hRequest, (LPVOID)responseBuffer,  dwSize, &dwDownloaded ) )
				{
					CErrArg ( "HTTPPostRequest: Failed to read response data with error %u.", GetLastError () );
				}
				else {
					CLogArg ( "HTTPPostRequest: Response - %s", responseBuffer );
					if ( strstr ( responseBuffer, "\"success\":1" ) ) {
						ret = true;
					}
				}

				delete [] responseBuffer;
			}
		} while ( dwSize > 0 );

		break;
	}
	if ( wDomain )
		delete ( wDomain );
	if ( wsAK )
		delete ( wsAK );

	if ( ret )
		printf ( "HTTPPostRequest: success.\n" );
	else
        printf ( "HTTPPostRequest: Error %u\n", GetLastError() );

	if( hRequest ) 
		WinHttpCloseHandle( hRequest );
	if( hConnect ) 
		WinHttpCloseHandle( hConnect );
	if( httpOpen ) 
		WinHttpCloseHandle( httpOpen );
		

	return ret ? true : false;
}

#else

size_t write_to_string(void *ptr, size_t size, size_t count, void *stream) {
  ((string*)stream)->append((char*)ptr, 0, size*count);
  return size*count;
}

bool MediatorDaemon::HTTPPostRequest ( string domain, string path, string key, string jsonData  )
{ 
	bool ret = false;

    CURL * curl;
    CURLcode res;
    struct curl_slist * headers = NULL;
        
    curl_global_init ( CURL_GLOBAL_DEFAULT );
    
    curl = curl_easy_init ();
    
    while ( curl ) {
        
        std::stringstream ssUrl;
        ssUrl << "https://" << domain << "/" << path;
        
        curl_easy_setopt ( curl, CURLOPT_URL, ssUrl.str ().c_str () );
		CVerbArg ( "HTTPPostRequest: curl opt url: %s", ssUrl.str ().c_str () );

        curl_easy_setopt ( curl, CURLOPT_POST, 1L );
                
        std::stringstream ssKey;
        ssKey << "Authorization: key=" << key;
        headers = curl_slist_append ( headers, ssKey.str().c_str() );
        headers = curl_slist_append ( headers, "Content-Type: application/json" );
        res = curl_easy_setopt ( curl, CURLOPT_HTTPHEADER, headers );
        
		CVerbArg ( "HTTPPostRequest: curl post data: %s", jsonData.c_str() );
        curl_easy_setopt ( curl, CURLOPT_POSTFIELDS, jsonData.c_str() );
        curl_easy_setopt ( curl, CURLOPT_POSTFIELDSIZE, (curl_off_t)jsonData.size());

		string response;
		curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, write_to_string );
		curl_easy_setopt ( curl, CURLOPT_WRITEDATA, &response );
		
		res = curl_easy_perform ( curl );        
        if ( res != CURLE_OK ) {
			CErrArg ( "HTTPPostRequest: curl_easy_perform() failed: %s", curl_easy_strerror ( res ) );
		}
		else {
			CVerbArg ( "HTTPPostRequest: curl reponse:%s", response.c_str() );

			if ( strstr ( response.c_str(), "\"success\":1" ) ) {
				ret = true;
			}
		}

        break;
    }
    
    if ( headers )
        curl_slist_free_all ( headers );

    if ( curl )
        curl_easy_cleanup ( curl );
    
    curl_global_cleanup ();

	return ret;
}
#endif


// Logging to file

void getTimeString ( char * timeBuffer, unsigned int bufferSize )
{
	time_t now;
	struct tm timeInfo;

	time ( &now );
#ifdef WIN32
	if ( localtime_s ( &timeInfo, &now ) ) {
#else
	if ( !localtime_s ( &timeInfo, &now ) ) {
#endif
		timeBuffer [ 0 ] = 'e';
		timeBuffer [ 1 ] = 0;
	}

	strftime ( timeBuffer, bufferSize, "%a %b %d %H:%M:%S: ", &timeInfo );
}

    
void MLogArg ( const char * format, ... )
{
	char timeString [ 256 ];
	getTimeString ( timeString, sizeof(timeString) );

	char logBuffer [ 2048 ];
	va_list argList;	
	va_start ( argList, format );
#ifdef WIN32
	vsprintf_s ( logBuffer, (sizeof(logBuffer) - 2), format, argList );
#else
	vsnprintf ( logBuffer, (sizeof(logBuffer) - 2), format, argList );
	//vsprintf ( logBuffer, format, argList );
	/*printf ( "%s", timeString );
	printf ( "%s", logBuffer );*/
#endif
	va_end ( argList );

	if ( stdlog ) {
		OutputDebugStringA ( timeString );
		OutputDebugStringA ( logBuffer );
	}

	if ( !logging )
		return;
	
	stringstream line;

	// Save 
    line << logBuffer;
    
    if ( pthread_mutex_lock ( &logMutex ) ) {
        OutputDebugStringA ( "MLog: Failed to aquire mutex!" );
        return;
    }
	
	logfile << timeString << line.str() << std::flush; // << endl;
	logfile.flush ();

	if ( pthread_mutex_unlock ( &logMutex ) ) {
		OutputDebugStringA ( "MLog: Failed to release mutex on logfile!" );
	}	
}

void MLog ( const char * msg )
{	
	char timeString [ 256 ];
	getTimeString ( timeString, sizeof(timeString) );
	
	if ( stdlog ) {
#ifdef WIN32
		OutputDebugStringA ( timeString );
		OutputDebugStringA ( msg );
#else
		printf ( "%s", timeString );
		printf ( "%s", msg );
#endif
	}

	if ( !logging )
		return;
	stringstream line;

	// Save 
    line << msg;
    
    if ( pthread_mutex_lock ( &logMutex ) ) {
        OutputDebugStringA ( "MLog: Failed to aquire mutex!" );
        return;
    }

	logfile << timeString << line.str(); // << std::flush; // << endl;
	//logfile.flush ();

	if ( pthread_mutex_unlock ( &logMutex ) ) {
		OutputDebugStringA ( "MLog: Failed to release mutex!" );
	}	
}


int logStatusBefore = 0;


bool MediatorDaemon::OpenLog ()
{
	CVerb ( "OpenLog" );
	
	if ( pthread_mutex_lock ( &logMutex ) ) {
		CErr ( "OpenLog: Failed to aquire mutex on logfile!" );
		return false;
	}	

	if ( logfile.is_open () ) {
		CWarn ( "OpenLog: logile is already opened!" );
		goto Finish;
	}
	
	if ( GetSizeOfFile ( LOGFILE )  >= 300 * 1024 * 1024 ) 
	{
		// Find next available
		int i = 0;
		bool rename = false;
		do {
			stringstream tmp;
			tmp << LOGFILE << "." << i;

			ifstream tmpFile ( tmp.str() );
			if ( !tmpFile.good () ) {
				rename = true;
			}
			tmpFile.close ();

			if ( rename ) {
				std::rename(LOGFILE, tmp.str ().c_str ()); 
			}
			i++;
		}
		while ( !rename );
	}

	logfile.open ( LOGFILE, ios_base::app );
	if ( !logfile.good() ) {
		CErr ( "OpenLog: Failed to open logfile!" );
		goto Finish;
	}	

	if ( logStatusBefore >= 0 )
		logging = true;
	
Finish:
	if ( pthread_mutex_unlock ( &logMutex ) ) {
		CErr ( "CloseLog: Failed to release mutex on logfile!" );
		return false;
	}

	return true;
}


bool MediatorDaemon::CloseLog ()
{
	CLog ( "CloseLog" );
	
	if ( pthread_mutex_lock ( &logMutex ) ) {
		OutputDebugStringA ( "CloseLog: Failed to aquire mutex on logfile!" );
		return false;
	}	

	if ( !logfile.is_open () ) {
		OutputDebugStringA ( "CloseLog: logfile is not openend!" );
		goto Finish;
	}
	
	OutputDebugStringA ( "CloseLog: Closing log file..." );

	if ( logging == true )
		logStatusBefore = 1;
	else
		logStatusBefore = -1;
	logging = false;

	logfile.close();
	
Finish:
	if ( pthread_mutex_unlock ( &logMutex ) ) {
		OutputDebugStringA ( "CloseLog: Failed to release mutex on logfile!" );
		return false;
	}

	return true;
}
