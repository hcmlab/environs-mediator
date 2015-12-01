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
#	define DEBUGVERB
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


// Forward declarations

bool				logging;
ofstream			logfile;
pthread_mutex_t     logMutex;

static queue<NotifyQueueContext *> notifyQueue;

#ifdef DEBUGSOCKETWATCH
    static map<int, int> socketList;
#endif

ThreadInstance::~ThreadInstance ()
{
    int sock = socket;
    if ( sock != -1 )
    {
        socket = -1;
        
        shutdown ( sock, 2 );
        closesocket ( sock );
    }
    
    sock = spareSocket;
    if ( sock != -1 )
    {
        spareSocket = -1;
        
        shutdown ( sock, 2 );
        closesocket ( sock );
    }

    if ( *ips ) {
        CVerbVerbArg ( "ThreadInstance [ %s:%i ]: Disposing accessMutex.", ips, port );
        MutexDispose ( &accessMutex );
    }
    
    if ( aes.encCtx ) {
        CVerbVerb ( "ThreadInstance: Disposing AES key context." );
        AESDisposeKeyContext ( &aes );
    }
}


DeviceInstanceNode::~DeviceInstanceNode ()
{
	clientSP = 0; rootSP = 0;
}


MediatorDaemon::MediatorDaemon ()
{
	CVerb ( "Construct" );

	allocated				= false;
    anonymousLogon          = true;
    *anonymousUser          = 0;
    *anonymousPassword      = 0;
	
	input					= inputBuffer;

	listeners           .clear ();
	usersDB             .clear ();
	deviceMappings      .clear ();
	acceptClients       .clear ();

	areas	            .clear ();
	ports               .clear ();
	bannedIPs           .clear ();
    bannedIPConnects    .clear ();
    bannAfterTries      = 3;
	
	areasList           .clear ();
	
	areasCounter        = 0;
	areaIDs             .clear ();

	appsCounter         = 0;
	appIDs              .clear ();
    
    notifyTargets       .clear ();

	networkOK			= 0xFFFFFFFF;
	checkLast			= 0;

	usersDBDirty		= false;
	configDirty			= false;
    
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
	sessions                .clear ();

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
		MutexDispose ( &usersDBMutex );
		MutexDispose ( &bannedIPsMutex );
		MutexDispose ( &acceptClientsMutex );
		MutexDispose ( &sessionsMutex );
		MutexDispose ( &areasMutex );
		MutexDispose ( &thread_mutex );
        MutexDispose ( &notifyMutex );
        MutexDispose ( &notifyTargetsMutex );
		MutexDispose ( &logMutex );
	}
}


void MediatorDaemon::PrintSmallHelp ()
{
	printf ( "Press ESC to quit; h for help.\n" );
}


void MediatorDaemon::PrintHelp ()
{
	printf ( "-------------------------------------------------------\n" );
	printf ( "h - print help\n" );
	printf ( "b - send broadcast packet to clients\n" );
	printf ( "i - print interfaces\n" );
	printf ( "m - print mediators\n" );
	printf ( "c - print configuration\n" );
	printf ( "d - print database\n" );
    printf ( "p - print active devices\n" );
    printf ( "g - print client resources\n" );
	printf ( "l - toggle authentication\n" );
	printf ( "a - add/update user\n" );
	printf ( "z - clear bann list\n" );
	printf ( "r - reload pkcs keys\n" );
	printf ( "-------------------------------------------------------\n" );
}


bool MediatorDaemon::InitMediator ()
{
	CVerb ( "InitMediator" );

	if ( !allocated ) {
        if ( !MutexInit ( &thread_mutex ) )
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
        
        if ( !MutexInit ( &areasMutex ) )
            return false;
        
        if ( !MutexInit ( &bannedIPsMutex ) )
            return false;
        
        if ( !MutexInit ( &usersDBMutex ) )
            return false;
        
        if ( !MutexInit ( &acceptClientsMutex ) )
            return false;
        
        if ( !MutexInit ( &sessionsMutex ) )
            return false;
        
        if ( !MutexInit ( &notifyMutex ) )
            return false;
        
        if ( !MutexInit ( &notifyTargetsMutex ) )
            return false;
        
        if ( !MutexInit ( &logMutex ) )
            return false;

        allocated = true;
        
        strcpy_s ( anonymousUser, sizeof(anonymousUser) - 1, MEDIATOR_ANONYMOUS_USER );
        
        Zero ( anonymousPassword );        
        strcpy_s ( anonymousPassword, sizeof(anonymousPassword) - 1, MEDIATOR_ANONYMOUS_PASSWORD );
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
                strcpy_s ( anonymousUser, sizeof(anonymousUser) - 1, svalue.c_str() );
            }
        }
        else if ( str [ 0 ] == 'A' && str [ 1 ] == 'P' && str [ 2 ] == ':' ) {
            if (!(iss >> prefix >> svalue )) {
                CLogArg ( "LoadConfig: Invalid config line: %s", str );
            }
            else {
                strcpy_s ( anonymousPassword, sizeof(anonymousPassword) - 1, svalue.c_str() );
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

		if ( MutexLock ( &thread_mutex, "LoadConfig" ) ) {

			configDirty = true;
			if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
				CErr ( "LoadConfig: Error to signal watchdog event" );
			}

			MutexUnlock ( &thread_mutex, "LoadConfig" );
		}
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
	
	if ( MutexLock ( &mediatorMutex, "SaveConfig" ) ) 
	{
		MediatorInstance * net = &mediator;

		while ( net && net->ip ) {
			configs << "M: " << net->ip << " " << net->port ;
			
			configs << endl;
			net = net->next;
		}
	
		MutexUnlock ( &mediatorMutex, "SaveConfig" );
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
    	
	if ( MutexLock ( &bannedIPsMutex, "SaveConfig" ) ) 
	{
		std::map<unsigned int, std::time_t>::iterator iter;

		for ( iter = bannedIPs.begin(); iter != bannedIPs.end(); ++iter ) {
			configs << "B: " << iter->first << " " << iter->second << endl ;
        }
        
        std::map<unsigned int, unsigned int>::iterator itert;
        
        for ( itert = bannedIPConnects.begin(); itert != bannedIPConnects.end(); ++itert ) {
            configs << "C: " << itert->first << " " << itert->second << endl ;
        }
	
		MutexUnlock ( &bannedIPsMutex, "SaveConfig" );
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
	
	MutexLockV ( &areasMutex, "LoadProjectValues" );

	sp ( AppsList )					apps;
	map<string, ValuePack*>		*	app		= 0;
	
	string line;
	while ( getline ( conffile, line ) )
	{
		const char * str = line.c_str ();
		if ( strlen ( str ) < 3 )
			continue;

		if ( str [ 0 ] == 'P' && str [ 1 ] == ':' ) {
			CVerbArg ( "Loading area [%s]", str );

			apps = make_shared < AppsList > ();
			if ( !apps ) {
				CErrArg ( "LoadProjectValues: Failed to create new area [%s].", str + 2 );
				break;
			}
			areas [ string ( str + 2 ) ] = apps;
			continue;
		}

		if ( str [ 0 ] == 'A' && str [ 1 ] == ':' ) {
			if ( !apps ) {
				CErrArg ( "LoadProjectValues: Missing project for app [%s]!", str + 2 );
				break;
			}
			CVerbArg ( "Loading application [%s]", str );

			app = new map<string, ValuePack*> ();
			if ( !app ) {
				CErrArg ( "LoadProjectValues: Failed to create new application [%s].", str + 2 );
				break;
			}
			apps->apps [ string ( str + 2 ) ] = app;
			continue;
		}

		if ( !app ) {
			CErr ( "LoadProjectValues: Invalid data file format. Project/App definition for values missing!" );
			break;
		}
		
		std::istringstream iss ( line );
		string key, value;
		unsigned int size = 0;
		if ( !(iss >> key >> size >> value) ) {
			CErrArg ( "LoadProjectValues: Failed to read key/value (%s)!", line.c_str() );
			break;
		}

		ValuePack * pack = new ValuePack ();
		if ( !pack ) {
			CErrArg ( "LoadProjectValues: Failed to create new value object for %s/%u/%s! Memory low problem!", key.c_str(), size, value.c_str() );
			break;
		}

		pack->timestamp = 0;
		pack->value = value;
		pack->size = size;
		
		(*app) [ key ] = pack;
	}
	
	/// Sanitinize database
	msp ( string, AppsList )::iterator it = areas.begin();
	
	while ( it != areas.end() )
	{		
		if ( !it->second ) {
			areas.erase ( it );
			it = areas.begin();
			continue;
		}
		
		map<string, map<string, ValuePack*>*>::iterator ita = it->second->apps.begin();
		while ( ita != it->second->apps.end() )
		{
			if ( !ita->second ) {
				it->second->apps.erase ( ita );
				ita = it->second->apps.begin();
				continue;
			}

			if ( !ita->second->size() ) {
				it->second->apps.erase ( ita );
				ita = it->second->apps.begin();
				continue;
			}
			ita++;
		}

		if ( !it->second->apps.size() ) {
			areas.erase ( it );
			it = areas.begin();
			continue;
		}

		it++;
	}
	
	MutexUnlockV ( &areasMutex, "LoadProjectValues" );

	conffile.close();

	return true;
}


bool MediatorDaemon::SaveProjectValues ()
{
	bool ret = true;

	CLog ( "SaveProjectValues" );

	ofstream conffile;
	conffile.open ( DATAFILE );
	if ( !conffile.good() )
		return false;
	
	if ( !MutexLock ( &devicesMutex, "SaveProjectValues" ) )
		return false;

	if ( !MutexLock ( &areasMutex, "SaveProjectValues" ) ) {
		ret = false;
	}
	else {
		sp ( AreaApps )						areaApps;
		sp ( ApplicationDevices )			appDevices;

		// Save areas
		for ( msp ( string, AppsList )::iterator it = areas.begin(); it != areas.end(); it++ )
		{
			if ( !it->second )
				continue;

			msp ( string, AreaApps )::iterator areaIt = areasList.find ( it->first );
			if ( areaIt != areasList.end () )
				areaApps = areaIt->second;
			else
				areaApps = 0;

			conffile << "P:" << it->first << endl;

			for ( map<string, map<string, ValuePack*>*>::iterator ita = it->second->apps.begin(); ita != it->second->apps.end(); ita++ ) 
			{
				if ( !ita->second ) 
					continue;

				unsigned int maxID = 0;

				/// Find the appList
				if ( areaApps ) {
					msp ( string, ApplicationDevices )::iterator appsIt = areaApps->apps.find ( ita->first );
					if ( appsIt != areaApps->apps.end () )
					{
						appDevices = appsIt->second;
						if ( appDevices )
							maxID = (unsigned int) appDevices->latestAssignedID;
					}
				}

				conffile << "A:" << ita->first << endl;

				if ( maxID > 0 )
					conffile << "0_maxID " << maxID << " ids" << endl;

				for ( map<string, ValuePack*>::iterator itv = ita->second->begin(); itv != ita->second->end(); ++itv )
				{
					if ( itv->second ) {
						if ( maxID > 0 && itv->first.c_str() [0] == '0' )
							continue;
						conffile << itv->first << " " << itv->second->size << " " << itv->second->value << endl;
					}
				}

			}
		}

		if ( !MutexUnlock ( &areasMutex, "SaveProjectValues" ) ) {
			ret = false;
		}
	}

	MutexLockV ( &devicesMutex, "SaveProjectValues" );

	conffile.close();
	return ret;
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
	if ( !MutexLock ( &usersDBMutex, "LoadDeviceMappings" ) )
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
        
        sp ( DeviceMapping ) mapping = sp ( DeviceMapping ) ( new DeviceMapping ); // calloc ( 1, sizeof(DeviceMapping) );
        if ( !mapping )
            break;
		memset ( mapping.get (), 0, sizeof(DeviceMapping) );
        
        mapping->deviceID = deviceID;
        mapping->authLevel = authLevel;
        if ( authToken.length() > 0 )
            memcpy ( mapping->authToken, authToken.c_str(), authToken.length() );
        
        deviceMappings [ deviceUID ] = mapping;
    }
    
	MutexUnlock ( &usersDBMutex, "LoadDeviceMappings" );
    
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

	if ( !MutexLock ( &usersDBMutex, "SaveDeviceMappings" ) ) {
		ret = false;
	}
	else {
		// Save devcie mappings
		for ( msp ( string, DeviceMapping )::iterator it = deviceMappings.begin(); it != deviceMappings.end(); it++ )
		{
			if ( !it->second )
				continue;

			plainstream << it->first << " " << it->second->deviceID << " " << it->second->authLevel << " " << it->second->authToken << endl;
		}
        
		if ( !MutexUnlock ( &usersDBMutex, "SaveDeviceMappings" ) ) {
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

	if ( !MutexLock ( &usersDBMutex, "AddUser" ) )
		return false;
	
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

		map<string, UserItem *>::iterator iter = usersDB.find ( user );
		if ( iter != usersDB.end () ) {
			CLogArg ( "AddUser: Updating password of user [%s]", userName );
			printf ( "\nAddUser: Updating password of user [%s]\n", userName );
		}
		
        item->authLevel = authLevel;
        item->pass = string ( hash );
        
		usersDB [ user ] = item;
		ret = true;
        item = 0;
	}
	while ( 0 );

	if ( hash )
		free ( hash );
    
    if ( item )
		delete item;

	if ( !MutexUnlock ( &usersDBMutex, "AddUser" ) )
		return false;

	if ( ret ) {
		if ( MutexLock ( &thread_mutex, "AddUser" ) ) {

			usersDBDirty = true;
			if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
				CErr ( "AddUser: Error to signal watchdog event" );
			}

			MutexUnlock ( &thread_mutex, "AddUser" );
		}
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

	if ( !MutexLock ( &usersDBMutex, "SaveUserDB" ) )
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
        
		if ( !MutexUnlock ( &usersDBMutex, "SaveUserDB" ) )
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

    MutexLockV ( &bannedIPsMutex, "IsIpBanned" );

	std::map<unsigned int, std::time_t>::iterator iter = bannedIPs.find ( ip );
	if ( iter != bannedIPs.end () )
	{
		CLog ( "IsIpBanned: A banned IP tries to connect again.!" );

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
        
        std::map<unsigned int, unsigned int>::iterator itert = bannedIPConnects.find ( ip );
        if ( itert != bannedIPConnects.end () )
            bannedIPConnects.erase ( itert );
	}
	
Finish:
    MutexUnlockV ( &bannedIPsMutex, "IsIpBanned" );

	return banned;
}


void MediatorDaemon::BannIP ( unsigned int ip )
{
	CVerb ( "BannIP" );

	if ( (ip & 0x00FFFFFF) == 0xABFA89 ) {
		CVerb ( "BannIP: IP is one of the allowed networks!" );
		return;
    }
    
    if ( !MutexLock ( &bannedIPsMutex, "BannIP" ) )
        return;
    
    unsigned int tries = bannedIPConnects [ ip ];
    if ( tries < bannAfterTries ) {
        bannedIPConnects [ ip ] = ++tries;
        CVerbVerbArg ( "BannIP: retain bann due to allowed tries [%u/%u]", tries, bannAfterTries );
    }
    else {
        std::time_t t = std::time(0);
        
        bannedIPs [ ip ] = t;
    }
    
    MutexUnlockV ( &bannedIPsMutex, "IsIpBanned" );

	if ( MutexLock ( &thread_mutex, "LoadConfig" ) ) {

		configDirty = true;
		if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
			CErr ( "LoadConfig: Error to signal watchdog event" );
		}

		MutexUnlock ( &thread_mutex, "LoadConfig" );
	}
}


void MediatorDaemon::BannIPRemove ( unsigned int ip )
{
    CVerb ( "BannIPRemove" );
    
    bool ret = false;
    
    if ( !MutexLock ( &bannedIPsMutex, "BannIPRemove" ) )
        return;
    
    std::map<unsigned int, std::time_t>::iterator iter = bannedIPs.find ( ip );
    if ( iter != bannedIPs.end () )
    {
        CVerb ( "BannIPRemove: The IP has been banned before" );
        ret = true;

        bannedIPs.erase ( iter );
        
        std::map<unsigned int, unsigned int>::iterator itert = bannedIPConnects.find ( ip );
        if ( itert != bannedIPConnects.end () )
            bannedIPConnects.erase ( itert );
    }
    
    MutexUnlockV ( &bannedIPsMutex, "IsIpBanned" );
    
	if ( ret ) {
		if ( MutexLock ( &thread_mutex, "IsIpBanned" ) ) {

			configDirty = true;
			if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
				CErr ( "IsIpBanned: Error to signal watchdog event" );
			}

			MutexUnlock ( &thread_mutex, "IsIpBanned" );
		}
	}
}


bool MediatorDaemon::CreateThreads ()
{
	CVerb ( "CreateThreads" );

	//pthread_attr_t attr;
	int s, value, ret;

	ReleaseThreads ();

	isRunning = true;

	for ( unsigned int pos = 0; pos < ports.size(); pos++ )
	{
		sp ( MediatorThreadInstance ) listenerSP = sp ( MediatorThreadInstance ) ( new MediatorThreadInstance ); // make_shared < MediatorThreadInstance > ();
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

		value = 1;
		ret = setsockopt ( sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value) );
		if ( ret < 0 ) {
			CErr ( "CreateThreads: Failed to set reuseAddr on listener socket." ); LogSocketError ();
			return false;
		}

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
		MutexLockV ( &thread_mutex, "CreateThreads" );

		s = pthread_create ( &inst->threadID, 0, &MediatorDaemon::AcceptorStarter, (void *)listener );
		if ( s != 0 ) {
			CErrArg ( "CreateThreads: Error creating acceptor thread for port %i (pthread_create:%i)", inst->port, s );
			pthread_mutex_unlock ( &thread_mutex );
			return false;
		}

		pthread_cond_wait ( &thread_condition, &thread_mutex );
		MutexUnlockV ( &thread_mutex, "CreateThreads" );

		MutexLockV ( &thread_mutex, "CreateThreads" );

		s = pthread_create ( &listener->threadIDUdp, 0, &MediatorDaemon::MediatorUdpThreadStarter, (void *)listener );
		if ( s != 0 ) {
			CErrArg ( "CreateThreads: Error creating udp thread for port %i (pthread_create:%i)", inst->port, s );
			MutexUnlockV ( &thread_mutex, "CreateThreads" );
			return false;
		}

		pthread_cond_wait ( &thread_condition, &thread_mutex );
		MutexUnlockV ( &thread_mutex, "CreateThreads" );

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
	
	ReleaseDevices ();

    MutexLockV ( &areasMutex, "Dispose" );
		
	for ( msp ( string, AppsList )::iterator it = areas.begin(); it != areas.end(); it++ )
	{
		if ( !it->second )
			continue;

		for ( map<string, map<string, ValuePack*>*>::iterator ita = it->second->apps.begin(); ita != it->second->apps.end(); ita++ ) 
		{
			if ( !ita->second ) 
				continue;

			for ( map<string, ValuePack*>::iterator itv = ita->second->begin(); itv != ita->second->end(); ++itv )
			{
				if ( itv->second ) {
					delete itv->second;
				}
			}
			delete ita->second;
		}
		//delete it->second;
	}
    areas.clear ();
    
    MutexUnlockV ( &areasMutex, "Dispose" );

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

	MutexLockV ( &client->accessMutex, "ReleaseClient" );

	int sock = client->spareSocket;
	if ( sock != -1 ) {
        client->spareSocket = -1;

		shutdown ( sock, 2 );
		closesocket ( sock );
	}

	sock = client->socket;
	if ( sock != -1 ) {
        client->socket = -1;

		shutdown ( sock, 2 );
		closesocket ( sock );
	}
	
	MutexUnlockV ( &client->accessMutex, "ReleaseClient" );

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
		int sock = inst->socket;
		if ( sock != -1 ) {
            inst->socket = -1;

			shutdown ( sock, 2 );
			closesocket ( sock );
		}

		sock = listener->socketUdp;
		if ( sock != -1 ) {
            listener->socketUdp = -1;

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
		

	MutexLockV ( &acceptClientsMutex, "ReleaseThreads" );

	while ( acceptClients.size() > 0 ) 
	{
		sp ( ThreadInstance ) first = acceptClients [ 0 ];

		acceptClients.erase ( acceptClients.begin() );

		MutexUnlockV ( &acceptClientsMutex, "ReleaseThreads" );

		ReleaseClient ( first.get () );

		MutexLockV ( &acceptClientsMutex, "ReleaseThreads" );
	}

	MutexUnlockV ( &acceptClientsMutex, "ReleaseThreads" );

	return ret;
}


void MediatorDaemon::ReleaseDevices ()
{
	CLog ( "ReleaseDevices" );

    MutexLockV ( &devicesMutex, "ReleaseDevices" );

	for ( msp ( string, AreaApps )::iterator it = areasList.begin(); it != areasList.end(); ++it )
	{
		if ( it->second ) {
			sp ( AreaApps ) areaApps = it->second;

			for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin(); ita != areaApps->apps.end(); ++ita )
			{
				sp ( ApplicationDevices ) appDevices = ita->second;
				if ( appDevices ) 
				{
					MutexLockV ( &appDevices->mutex, "ReleaseDevices" );

					DeviceInstanceNode * device = appDevices->devices;
			
					while ( device ) {
						DeviceInstanceNode * toDispose = device;
						device = device->next;

						CVerbArg ( "[0x%X].ReleaseDevices: deleting memory occupied by client", toDispose->info.deviceID );

						ReleaseClient ( toDispose->clientSP.get () );


						toDispose->baseSP = 0;
					}	

					appDevices->devices = 0;

					MutexUnlockV ( &appDevices->mutex, "ReleaseDevices" );

					MutexDispose ( &appDevices->mutex );

					ita->second = 0;
				}
			}

			it->second = 0;
		}
	}
    areasList.clear ();
    
    MutexUnlockV ( &devicesMutex, "ReleaseDevices" );
}


void MediatorDaemon::ReleaseDeviceMappings ()
{
    CVerb ( "ReleaseDeviceMappings" );
    
    MutexLockV ( &usersDBMutex, "ReleaseDeviceMappings" );
	
	for ( msp ( string, DeviceMapping )::iterator it = deviceMappings.begin(); it != deviceMappings.end(); ++it ) 
	{
		if ( !it->second ) {
			continue;
		}
		it->second = 0;
	}
    deviceMappings.clear ();
    
    MutexUnlockV ( &usersDBMutex, "ReleaseDeviceMappings" );
}


void MediatorDaemon::RemoveDevice ( int deviceID, const char * areaName, const char * appName )
{
	CVerbID ( "RemoveDevice from maps" );

	if ( !areaName || !appName ) {
		CErrID ( "RemoveDevice: Called with NULL argument for areaName or appName?!" );
		return;
	}
	
	if ( !MutexLock ( &devicesMutex, "RemoveDevice" ) )
        return;

	bool					found	= false;
	DeviceInstanceNode	*	device	= 0;
	
	string appsName ( appName );
	sp ( ApplicationDevices )			appDevices = 0;

	string pareaName ( areaName );
	sp ( AreaApps )						areaApps;

	msp ( string, ApplicationDevices )::iterator appsIt;

    msp ( string, AreaApps )::iterator areaIt = areasList.find ( pareaName );
	if ( areaIt != areasList.end ( ) )
		areaApps = areaIt->second;
    
	if ( !areaApps ) {
		CWarnArgID ( "RemoveDevice: areaName [%s] not found.", areaName );
		goto Finish;
	}		

	appsIt = areaApps->apps.find ( appsName );				
	if ( appsIt != areaApps->apps.end ( ) )
		appDevices = appsIt->second;

	if ( !appDevices ) {
		CErrArg ( "RemoveDevice: appName [%s] not found.", appName );
		goto Finish;
	}

	MutexLockV ( &appDevices->mutex, "RemoveDevice" );
	
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
	
	MutexUnlockV ( &appDevices->mutex, "RemoveDevice" );

Finish:
	MutexUnlockV ( &devicesMutex, "RemoveDevice" );
}


void MediatorDaemon::RemoveDevice ( DeviceInstanceNode * device, bool useLock )
{
	CVerbVerb ( "RemoveDevice" );

	if ( !device || ( useLock && !MutexLock ( &devicesMutex, "RemoveDevice" ) ) )
		return;

	sp ( ApplicationDevices ) appDevices = device->rootSP;

	if ( useLock )
		MutexLockV ( &appDevices->mutex, "RemoveDevice" );

	if ( device == appDevices->devices ) {
		if ( device->next ) {
			CVerbArg ( "[0x%X].RemoveDevice: relocating client to root of list", device->next->info.deviceID );

			appDevices->devices = device->next;
			appDevices->devices->prev = 0;
			appDevices->count--;
		}
		else {
			appDevices->devices = 0;
			appDevices->count = 0;
		}
	}
	else {
		if ( !device->prev ) {
			CErrArg ( "RemoveDevice: Serious inconsistency error!!! Failed to lookup device list. Missing previous device for ID [0x%X]", device->info.deviceID );

			/// Best thing we can do is to assume that we are the root node of the list
			if ( device->next ) {
				CVerbArg ( "[0x%X].RemoveDevice: relocating client to root of list", device->next->info.deviceID );

				device->next->prev = 0;
				appDevices->devices = device->next;
				appDevices->count--;
			}
			else {
				appDevices->devices = 0;
				appDevices->count = 0;
			}
		}
		else {
			if ( device->next ) {
				CVerbArg ( "[0x%X].RemoveDevice: relinking client to previous client [0x%X]", device->next->info.deviceID, device->prev->info.deviceID );
				device->prev->next = device->next;
				device->next->prev = device->prev;
			}
			else {
				CVerbArg ( "[0x%X].RemoveDevice: finish list as the client was the last one", device->prev->info.deviceID );
				device->prev->next = 0;
			}

			appDevices->count--;

		}
	}

	if ( useLock)
		MutexUnlockV ( &appDevices->mutex, "RemoveDevice" );

	NotifyClients ( NOTIFY_MEDIATOR_SRV_DEVICE_REMOVED, device->info.areaName, device->info.appName, device->info.deviceID );
    
    
    CVerbArg ( "[0x%X].RemoveDevice: Disposing device", device->info.deviceID );
	device->baseSP = 0;

	if ( useLock )
		MutexUnlockV ( &devicesMutex, "RemoveDevice" );
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
		NotifyClients ( NOTIFY_MEDIATOR_SRV_DEVICE_ADDED, device->info.areaName, device->info.appName, device->info.deviceID );
	else if ( changed )
		NotifyClients ( NOTIFY_MEDIATOR_SRV_DEVICE_CHANGED, device->info.areaName, device->info.appName, device->info.deviceID );
}


/// 0 no filter
/// 1 show all within the same area
/// 2 show only within same area and apps

void MediatorDaemon::UpdateNotifyTargets ( const sp ( ThreadInstance ) &clientSP, int filterMode )
{	
	if ( !clientSP || !MutexLock ( &notifyTargetsMutex, "UpdateNotifyTargets" ) )
		return;

    msp ( long long, ThreadInstance )::iterator notifyIt;
	
    DeviceInstanceNode * device;
    ThreadInstance * client = clientSP.get ();
    
    sp ( DeviceInstanceNode ) deviceSP = client->deviceSP;
    if ( !deviceSP )
        goto Finish;
    
    device = deviceSP.get ( );

    /// Check whether we need to remove the client from a targetlist
    if ( client->filterMode != filterMode )
    {
        if ( client->filterMode == MEDIATOR_FILTER_NONE )
        {
            /// Remove from NoRestrict notifiers
            CVerbArg ( "UpdateNotifyTargets: Looking for deviceID [0x%X / %s / %s] in NONE-filter", client->deviceID, device ? device->info.areaName : "", device ? device->info.appName : "" );
            
            notifyIt = notifyTargets.find ( client->sessionID );
            
            if ( notifyIt != notifyTargets.end () )
            {
                CVerbArg ( "UpdateNotifyTargets: Removing deviceID [0x%X / %s / %s] from NONE-filter", client->deviceID, device ? device->info.areaName : "", device ? device->info.appName : "" );
                
                notifyTargets.erase ( notifyIt );
            }
        }
        else if ( client->filterMode == MEDIATOR_FILTER_AREA ) 
		{            
            while ( device && device->rootSP )
            {
                /// Search for area
                map<unsigned int, string>::iterator areaIdsIt = areaIDs.find ( device->rootSP->areaId );
                
                if ( areaIdsIt == areaIDs.end () ) {
                    CWarnArg ( "UpdateNotifyTargets: Failed to find area ID [%u]", device->rootSP->areaId );
                    break;
                }
                
                msp ( string, AreaApps )::iterator areaIt = areasList.find ( areaIdsIt->second );
                
                if ( areaIt == areasList.end ( ) ) {
                    CWarnArg ( "UpdateNotifyTargets: Failed to find area [%s]", areaIdsIt->second.c_str () );
                    break;
                }
                
                sp ( AreaApps ) areaApps = areaIt->second;
                if ( areaApps ) {
                    CVerbArg ( "UpdateNotifyTargets: Looking for deviceID [0x%X / %s / %s] in Area-filter", client->deviceID, device->info.areaName, device->info.appName );
                    
                    notifyIt = areaApps->notifyTargets.find ( client->sessionID );
                    
                    if ( notifyIt != areaApps->notifyTargets.end () )
                    {
                        CVerbArg ( "UpdateNotifyTargets: Removing deviceID [0x%X / %s / %s] from Area-filter", client->deviceID, device->info.areaName, device->info.appName );
                        
                        areaApps->notifyTargets.erase ( notifyIt );
                    }
                }
                break;
            }
        }
    }
    
    client->filterMode = (short) filterMode;
    
    if ( filterMode < MEDIATOR_FILTER_NONE )
        goto Finish;
    
	if ( filterMode == MEDIATOR_FILTER_ALL ) {
        goto Finish;
	}
    
    if ( filterMode == MEDIATOR_FILTER_NONE )
    {
		CVerbArg ( "UpdateNotifyTargets: Adding deviceID [0x%X / %s / %s] to NoRestrict-targets", client->deviceID, device ? device->info.areaName : "", device ? device->info.appName : "" );
        
        notifyTargets [ client->sessionID ] = clientSP;
    }
    else if ( filterMode == MEDIATOR_FILTER_AREA ) 
	{
        while ( device && device->rootSP )
        {
            /// Search for area
            map<unsigned int, string>::iterator areaIDsIt = areaIDs.find ( device->rootSP->areaId );
            
            if ( areaIDsIt == areaIDs.end () ) {
                CWarnArg ( "UpdateNotifyTargets: Failed to find area ID [%u]", device->rootSP->areaId );
                break;
            }
            
            msp ( string, AreaApps )::iterator areaIt = areasList.find ( areaIDsIt->second );
            
            if ( areaIt == areasList.end () ){
                CWarnArg ( "UpdateNotifyTargets: Failed to find area [%s]", areaIDsIt->second.c_str () );
                break;
            }
            
			CVerbArg ( "UpdateNotifyTargets: Adding deviceID [0x%X / %s / %s] to Area-filter", client->deviceID, device->info.areaName, device->info.appName );
            sp ( AreaApps ) areaApps = areaIt->second;
            
            if ( areaApps )
                areaApps->notifyTargets [ client->sessionID ] = clientSP;
            break;
        }
    }
    
Finish:
	MutexUnlockV ( &notifyTargetsMutex, "UpdateNotifyTargets" );
}


DeviceInstanceNode ** MediatorDaemon::GetDeviceList ( char * areaName, char * appName, pthread_mutex_t ** mutex, int ** pDevicesAvailable, void * pappDevices )
{
	DeviceInstanceNode ** list = 0;

	if ( !MutexLock ( &devicesMutex, "GetDeviceList" ) )
		return 0;

	sp ( ApplicationDevices	)			appDevices;
	sp ( AreaApps )						areaApps;
	msp ( string, ApplicationDevices )::iterator appsIt;


	map<string, ValuePack*> *			values = 0;
	sp ( AppsList )						apps;
	map<string, map<string, ValuePack*>*>::iterator appsVPIt;
	map<string, ValuePack*>::iterator	valueIt;
	msp ( string, AppsList )::iterator areaVPIt;

	string appsName ( appName );
	string pareaName ( areaName );

    msp ( string, AreaApps )::iterator areaIt = areasList.find ( pareaName );	

	if ( areaIt == areasList.end ( ) ) {
		/// Create a new one...
		areaApps = make_shared < AreaApps > ();
		if ( areaApps ) {
			areasCounter++;
			areaIDs [ areasCounter ] = pareaName;
			
			areaApps->id = areasCounter;
			areaApps->apps.clear ();
            areaApps->notifyTargets.clear ();

			areasList [ pareaName ] = areaApps;
		}
		else { CErrArg ( "GetDeviceList: Failed to create new area [%s] Low memory problem?!", areaName ); }
	}
	else
		areaApps = areaIt->second;
    
	if ( !areaApps ) {
		CLogArg ( "GetDeviceList: App [%s] not found.", appName );
		goto Finish;
	}
		
	appsIt = areaApps->apps.find ( appsName );
				
	if ( appsIt == areaApps->apps.end ( ) ) {
		/// Create a new one...
		appDevices = sp ( ApplicationDevices ) ( new ApplicationDevices ); // make_shared < ApplicationDevices > ();
		if ( appDevices ) {
			memset ( appDevices.get (), 0, sizeof(ApplicationDevices) );

			MutexInit ( &appDevices->mutex );

			appDevices->access = 1;			
			areaApps->apps [ appsName ] = appDevices;
			
			appsCounter++;
			appDevices->id = appsCounter;
			appDevices->areaId = areaApps->id;
			appIDs [ appsCounter ] = appsName;

			// Query maxIDs from value packs			
			areaVPIt = areas.find ( pareaName );
			if ( areaVPIt != areas.end () ) {
				apps = areaVPIt->second;

				appsVPIt = apps->apps.find ( appsName );
				if ( appsVPIt != apps->apps.end () ) {
					values = appsVPIt->second;

					valueIt = values->find ( string ( "0_maxID" ) );
					if ( valueIt != values->end () ) {
						appDevices->latestAssignedID = valueIt->second->size;
					}
				}
			}
		}
		else { CErrArg ( "GetDeviceList: Failed to create new application devicelist [%s] Low memory problem?!", appName ); }
	}
	else
		appDevices = appsIt->second;

	if ( !appDevices || appDevices->access <= 0 )
		goto Finish;
	
	__sync_add_and_fetch ( &appDevices->access, 1 );

	if ( mutex )
		*mutex = &appDevices->mutex;
	if ( pDevicesAvailable )
		*pDevicesAvailable = &appDevices->count;
	if ( pappDevices )
		*((sp ( ApplicationDevices ) *)pappDevices) = appDevices;

	list = &appDevices->devices;

Finish:
	if ( !MutexUnlock ( &devicesMutex, "GetDeviceList" ) )
		return 0;

	return list;
}


void printDevice ( DeviceInstanceNode * device )
{
    CLogArg ( "Device      = [0x%X / %s / %s]", device->info.deviceID, device->info.deviceName, device->info.broadcastFound ? "on same network" : "by mediator" );
    CLogArg ( "UID         = [%d / %s]", device->clientSP ? device->clientSP->authLevel : -1, device->clientSP ? device->clientSP->uid : ".-." );
    if ( device->info.ip != device->info.ipe ) {
        CLogArg ( "Device IPe != IP [%s]", inet_ntoa ( *((struct in_addr *) &device->info.ip) ) );
    }
    CLogArg ( "Area/App = [%s / %s]", device->info.areaName, device->info.appName );
    CLogArg ( "Device  IPe = [%s (from socket), tcp [%d], udp [%d]]", inet_ntoa ( *((struct in_addr *) &device->info.ipe) ), device->info.tcpPort, device->info.udpPort );
}


void printDeviceList ( DeviceInstanceNode * device )
{
	while ( device ) 
	{
        printDevice ( device );
        
		device = device->next;
	}
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

				printf ( "\r\n" );
				printf ( "%s", inputBuffer );
			}
			continue;
		}
		
		if ( input == inputBuffer && !command ) {
			if (c == 'h' ) { // Help requested
				PrintHelp ();
				continue;
			}
			else if ( c == 'q' ) {
				printf ( "-------------------------------------------------------\n" );
				CLog ( "Mediator exit requested!" );

				// Stop the threads
				break;
			}
			else if ( c == 'b' ) {
				SendBroadcast ();
				continue;
			}
			else if ( c == 'l' ) { 
				reqAuth = !reqAuth;
				printf ( "Run: Authentication is now [%s]\n", reqAuth ? "required" : "disabled" );
				CLogArg ( "Run: Authentication is now [%s]\n", reqAuth ? "required" : "disabled" );
				continue;
			}
			else if ( c == 'r' ) {
				printf ( "Run: Reloading keys" );
				CLog ( "Run: Reloading keys" );
                LoadKeys ();
				continue;
			}
			else if ( c == 'z' ) {
				if ( MutexLock ( &bannedIPsMutex, "Run" ) ) {
					printf ( "Run: Clearing bannlist with [%u] entries.\n", (unsigned int)bannedIPs.size() );
					CLogArg ( "Run: Clearing bannlist with [%u] entries.\n", (unsigned int)bannedIPs.size() );
                    bannedIPs.clear ();
                    bannedIPConnects.clear ();

					MutexUnlockV ( &bannedIPsMutex, "Run" );
				}
				continue;
			}
			else if ( c == 'i' ) {	
				if ( !MutexLock ( &localNetsMutex, "Run" ) )
					continue;
				
				NetPack * net = &localNets;
				while ( net ) {
					CLogArg ( "Interface ip:%s", inet_ntoa ( *((struct in_addr *) &net->ip) ) );
					CLogArg ( "Interface bcast:%s", inet_ntoa ( *((struct in_addr *) &net->bcast) ) );
					net = net->next;
				}
	
				MutexUnlockV ( &localNetsMutex, "Run" );
				continue;
			}
			else if ( c == 'm' ) {
				if ( !mediator.ip ) {
					CLog ( "No more mediators known yet!" );
					continue;
				}
				else {
					if ( !MutexLock ( &mediatorMutex, "Run" ) )
						continue;
					
					CLog ( "Mediators:" );
					CLog ( "----------------------------------------------------------------" );

					MediatorInstance * net = &mediator;

					while ( net ) {
						CLogArg ( "\tIP: %s\tPorts: ", inet_ntoa ( *((struct in_addr *) &net->ip) ) );

						for ( int i=0; i<MAX_MEDIATOR_PORTS; i++ ) {
							if ( !net->port )
								break;
							CLogArg ( "%i ", net->port );
						}
						CLog ( "\n" );
						net = net->next;
					}
					printf ( "----------------------------------------------------------------\n" );

					MutexUnlockV ( &mediatorMutex, "Run" );
				}
				continue;
			}
			else if ( c == 'c' ) {
				CLog ( "Configuration:" );
				printf ( "----------------------------------------------------------------\n" );
				CLog ( "Ports: " ); 
				for ( unsigned int pos = 0; pos < ports.size(); pos++ ) {
					CLogArg ( " %i", ports[pos] );
				}
				//printf ( "\n" );
				printf ( "----------------------------------------------------------------\n" );
				continue;
			}
			else if ( c == 'd' ) {
				CLog ( "Database:" );
				printf ( "----------------------------------------------------------------\n" );
				if ( MutexLock ( &areasMutex, "Run" ) ) 
				{		
					for ( msp ( string, AppsList )::iterator it = areas.begin(); it != areas.end(); it++ )
					{
						if ( !it->second )
							continue;
						CLogArg ( "Area: %s", it->first.c_str() );

						for ( map<string, map<string, ValuePack*>*>::iterator ita = it->second->apps.begin(); ita != it->second->apps.end(); ita++ ) 
						{
							if ( !ita->second ) 
								continue;
							CLogArg ( "App: %s", ita->first.c_str() );

							for ( map<string, ValuePack*>::iterator itv = ita->second->begin(); itv != ita->second->end(); ++itv )
							{
								if ( itv->second ) {
									CLogArg ( "\tKey: %s\tValue: %s", itv->first.c_str(), itv->second->value.c_str() );
								}
							}
						}
					}

					MutexUnlockV ( &areasMutex, "Run" );
				}
                printf ( "----------------------------------------------------------------\n" );
				if ( MutexLock ( &usersDBMutex, "Run" ) )
				{
                    for ( map<string, UserItem *>::iterator it = usersDB.begin(); it != usersDB.end(); ++it )
                    {
                        if ( it->second )
                        {
                            UserItem * item = it->second;
                            
                            const char * pwStr = ConvertToHexString ( item->pass.c_str(), ENVIRONS_USER_PASSWORD_LENGTH );
                            if ( pwStr ) {
                                CLogArg ( "User: %s\t[%s]", it->first.c_str(), pwStr );
                                
                            }
                        }
                    }

					MutexUnlockV ( &usersDBMutex, "Run" );
                }
                printf ( "----------------------------------------------------------------\n" );
				continue;
			}
			else if ( c == 'p' ) {
				CLog ( "Active devices:" );
				CLog ( "----------------------------------------------------------------" );
				
				if ( MutexLock ( &devicesMutex, "Run" ) )
				{
					for ( msp ( string, AreaApps )::iterator it = areasList.begin(); it != areasList.end(); ++it )
					{
						if ( it->second ) {
							sp ( AreaApps ) areaApps = it->second;
							CLogArg ( "P: [%s]", it->first.c_str () );

							for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin(); ita != areaApps->apps.end(); ++ita )
							{
								sp ( ApplicationDevices ) appDevices = ita->second;
								if ( appDevices ) 
								{					
									if ( appDevices->devices ) 
									{
										CLog ( "----------------------------------------------------------------" );
										CLogArg ( "A: [%s]", ita->first.c_str () );

										// Deadlock here
										if ( MutexLock ( &appDevices->mutex, "Run" ) )
										{
											printDeviceList ( appDevices->devices );

											MutexUnlockV ( &appDevices->mutex, "Run" );
										}
										CLog ( "----------------------------------------------------------------" );
									}
									else {
										CLogArg ( "A: No devices in [%s]", ita->first.c_str () );
									}
								}
							}
						}
					}

					MutexUnlockV ( &devicesMutex, "Run" );
				}
				CLog ( "----------------------------------------------------------------" );
				continue;
            }
            else if ( c == 'g' ) {
                CLog ( "Active clients:" );
                CLog ( "----------------------------------------------------------------" );
                
                if ( MutexLock ( &acceptClientsMutex, "Run" ) )
                {
                    for ( size_t i = 0; i < acceptClients.size(); i++ )
                    {
                        sp ( ThreadInstance ) client = acceptClients [i];
                        if ( client )
                        {
                            sp ( DeviceInstanceNode ) deviceSP = client->deviceSP;
                            if ( deviceSP )
                            {
                                CLogArg ( "[%i]: [%i]", i, client->deviceID );
                                
                                printDevice ( deviceSP.get () );
                            }
                        }
                        else {
                            CErrArg ( "[%i]: **** Invalid Client\n\n", i );
                        }
                        CLog ( "----------------------------------------------------------------" );
                    }
                    MutexUnlock ( &acceptClientsMutex, "Run" );
                }
                CLog ( "----------------------------------------------------------------" );
                continue;
            }
			else if ( c == 'a' ) {
				CLog ( "Add new user:" );
				printf ( "Add new user:\n" );
				printf ( "----------------------------------------------------------------\n" );
				CLog ( "Please enter an access level (0 - 10). Default [3]:" );
				printf ( "Please enter an access level (0 - 10). Default [3]: " );
				command = 'a';
                accessLevel = -1;
				userName = "a";
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
                    printf ( "\nPlease enter the username (email): " );
                }
				else if ( userName.c_str()[0] == 'a' && userName.length() == 1 ) {
					userName = inputBuffer;
					if ( userName.find ( '@', 0 ) == string::npos ) {
						command = 0;
						userName = "";
						CLog ( "The username must be a valid email address!" );
						printf ( "The username must be a valid email address!\n" );
					}
					else {
						CLog ( "Please enter the passphrase:" );
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
						printf ( "\nUser: [%s] successfully added.\n", userName.c_str() );
					}
					else {
						CErrArg ( "User: Failed to add user [%s]", userName.c_str() );
						printf ( "\nUser: Failed to add user [%s]\n", userName.c_str() );
					}
					command = 0;
					userName = "";
					printf ( "----------------------------------------------------------------\n" );
				}
			}

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
	MutexLockV ( &thread_mutex, "Udp" );

	if ( pthread_cond_signal ( &thread_condition ) ) {
		CErr ( "MediatorUdpThread: Error to signal event" );
	}

	MutexUnlockV ( &thread_mutex, "Udp" );

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

				if ( !MutexLock ( &appDevices->mutex, "Udp" ) ) {
					UnlockApplicationDevices ( appDevices.get () );
					continue;
				}

				DeviceInstanceNode * device = GetDeviceInstance ( destID, appDevices->devices );
				if ( !device ) {
					CErrArg ( "Udp: requested STUN device [%d] does not exist", destID );
					goto Continue;
				}

				clientSP = device->clientSP;
				if ( !clientSP ) {
					CErrArg ( "Udp: requested client of STUN device [%d] does not exist", destID );
					goto Continue;
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
					
Continue:
				UnlockApplicationDevices ( appDevices.get () );

				MutexUnlockV ( &appDevices->mutex, "Udp" );
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
	ThreadInstance * listener = (ThreadInstance *) arg;
	int s;

	unsigned short port = listener->port;
	
	CVerbArg ( "Acceptor for port %d started.", port );

	// Send started signal
	MutexLockV ( &thread_mutex, "Acceptor" );

	if ( pthread_cond_signal ( &thread_condition ) ) {
		CErr ( "Acceptor: Error to signal event" );
	}

	MutexUnlockV ( &thread_mutex, "Acceptor" );

	memset ( &listenAddr, 0, sizeof(listenAddr) );
	
	listenAddr.sin_family		= AF_INET;
	listenAddr.sin_addr.s_addr	= INADDR_ANY; //htonl ( INADDR_BROADCAST ); // INADDR_ANY );
    listenAddr.sin_port			= htons ( port );

	int ret = ::bind ( listener->socket, (struct sockaddr *)&listenAddr, sizeof(listenAddr) );
	if ( ret < 0 ) {
		CErrArg ( "Acceptor: Failed to bind listener socket to port [%i]!", port );
		LogSocketError ();
		return 0;
	}	
	CVerbArg ( "Acceptor bound to port [%i]", port );

	ret = listen ( listener->socket, SOMAXCONN );
	if ( ret < 0 ) {
		CErrArg ( "Acceptor: Failed to listen on socket to port [%i]!", port );
		LogSocketError ();
		return 0;
	}
	CLogArg ( "Acceptor listen on port [%i]", port );
	
    struct 	sockaddr_in		addr;
    socklen_t 				addrLen = sizeof(addr);

	// Create socket for 
	while ( isRunning ) {
		Zero ( addr );
		
		int sock = (int) accept ( listener->socket, (struct sockaddr *)&addr, &addrLen );

		if ( sock < 0 ) {
			CLogArg ( "Acceptor: Socket [%i] on port %i has been closed!", sock, port );
			break;
		}		

		CLog ( "\n" );
        const char * ips = inet_ntoa ( addr.sin_addr );
        
		CLogArg ( "Acceptor: New socket [%i] connection with IP [%s], Port [%d]", sock, ips, ntohs ( addr.sin_port ) );
		CLog ( "\n" );
		
		sp ( ThreadInstance ) client = sp ( ThreadInstance ) ( new ThreadInstance ); //make_shared < ThreadInstance > ();
		if ( !client ) {
            CErr ( "Acceptor: Failed to allocate memory for client request!" );
            goto NextClient;
		}
		memset ( client.get (), 0, sizeof ( ThreadInstance ) - ( sizeof ( sp ( DeviceInstanceNode ) ) + sizeof ( sp ( ThreadInstance ) ) ) );

		if ( !MutexInit ( &client->accessMutex ) )
			goto NextClient;
		
		//pthread_cond_init ( &client->socketSignal, NULL );
		
		client->socket		= sock;
		client->spareSocket	= -1;
		client->port		= port;
		client->filterMode	= 2;
		client->aliveLast	= checkLast;
		client->daemon		= this;
		client->connectTime = GetEnvironsTickCount ();

		memcpy ( &client->addr, &addr, sizeof(addr) );
        
        strcpy_s ( client->ips, sizeof(client->ips) - 1, ips );

        
		if ( !MutexLock ( &acceptClientsMutex, "Acceptor" ) )
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
            acceptClients.push_back ( client->clientSP );
        }

		MutexUnlockV ( &acceptClientsMutex, "Acceptor" );
        
    NextClient:
        if ( sock != -1 ) {
            shutdown ( sock, 2 ); closesocket ( sock );
        }
	}
   
	CLogArg ( "Acceptor: Thread for port %d terminated.", port );

	return 0;
}


void * MediatorDaemon::ClientThreadStarter ( void *arg )
{
    if ( !arg  )
		return 0;
	
	ThreadInstance * client = ( ThreadInstance * ) arg;

	CVerbArg ( "ClientThreadStarter: Address of arg [0x%p].", &client );
	
	MediatorDaemon * daemon = (MediatorDaemon *)client->daemon;
	if ( !daemon )
		return 0;

	sp ( ThreadInstance ) clientSP = client->clientSP;
	if ( clientSP )
	{	
		if ( daemon ) {
			// Execute thread
			daemon->ClientThread ( client );
		}
	}

	if ( daemon->isRunning ) 
	{
		MutexLockV ( &daemon->acceptClientsMutex, "Client" );
	
		// find the one in our vector
		size_t size = daemon->acceptClients.size ();

		for ( size_t i = 0; i < size; ++i ) 
		{
			if ( daemon->acceptClients[i].get () == client ) {
				CVerbArg ( "ClientThreadStarter [ %s:%i ]:\tErasing [%i] from client list", client->ips, client->port, i );

				daemon->acceptClients.erase ( daemon->acceptClients.begin() + i );
				size--;
			}
		}
        
		MutexUnlockV ( &daemon->acceptClientsMutex, "Client" );
		

		CVerbArg ( "Client [ %s:%i ]:\tDisposing memory for client", client->ips, client->port );
        
        sp ( DeviceInstanceNode ) deviceSP = client->deviceSP;
        
        if ( deviceSP ) {
            CVerbArg ( "Client [ %s:%i ]:\tReleasing deviceSP", client->ips, client->port );
            
			daemon->RemoveDevice ( deviceSP.get () );
			client->deviceSP = 0;
		}
		
		MutexLockV ( &daemon->sessionsMutex, "Client" );

		msp ( long long, ThreadInstance )::iterator sessionIt = daemon->sessions.find ( client->sessionID );
		if ( sessionIt != daemon->sessions.end () ) {
			daemon->sessions.erase ( sessionIt );
		}
		
		MutexUnlockV ( &daemon->sessionsMutex, "Client" );
		
        client->daemon = 0;
	}
	return 0;
}


bool MediatorDaemon::addToArea ( const char * area, const char * app, const char * pKey, const char * pValue )
{
	bool ret = false;

	// Search for the area at first
	map<string, ValuePack*>							*	values	= 0;
	sp ( AppsList )										apps;
	map<string, map<string, ValuePack*>*>::iterator		appsIt;
		
	if ( !area || !app ) {
		CErr ( "addToArea: Invalid parameters. Missing area/app name!" );
		return false;
	}

	if ( !MutexLock ( &areasMutex, "addToArea" ) )
		return false;
	
	string areaName ( area );
	string appName ( app );

	msp ( string, AppsList )::iterator areaIt = areas.find ( areaName );			

	if ( areaIt == areas.end () )
	{		
		apps = make_shared < AppsList > ();
		if ( !apps ) {
			CErrArg ( "addToArea: Failed to create new area [%s].", area );
			goto EndWithStatus;
		}
		areas [ areaName ] = apps;
	}
	else
		apps = areaIt->second;
	
	appsIt = apps->apps.find ( appName );

	if ( appsIt == apps->apps.end () )
	{		
		values = new map<string, ValuePack*> ();
		if ( !values ) {
			CErrArg ( "addToArea: Failed to create new application [%s].", appName.c_str () );
			goto EndWithStatus;
		}
		apps->apps [ appName ] = values;
	}
	else
		values = appsIt->second;

	ret = addToArea ( values, pKey, pValue, (unsigned int) strlen(pValue) );

EndWithStatus:
	if ( !MutexUnlock ( &areasMutex, "addToArea" ) )
		return false;
	return ret;
}


bool MediatorDaemon::addToArea ( map<string, ValuePack*> * values, const char * pKey, const char * pValue, unsigned int valueSize )
{
	bool ret = false;
	
	if ( !pKey || !pValue || valueSize <= 0 ) {
		CErr ( "addToArea: Invalid parameters. Missing key/value!" );
		return false;
	}

	string key = pKey;

	// Look whether we already have a value for the key
	map<string, ValuePack*>::iterator valueIt = values->find ( key );
	
	if ( valueIt != values->end() )
	{
		// Delete the old value
		if ( valueIt->second )
			delete valueIt->second;
		values->erase ( valueIt );
	}

	ValuePack * pack = new ValuePack ();
	if ( !pack ) {
		CErrArg ( "addToArea: Failed to create new value object for %s! Memory low problem!", pKey );
		goto EndWithStatus;
	}
	pack->timestamp = 0;
	pack->value = string ( pValue );
	pack->size = valueSize;

	(*values) [ key ] = pack;
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
	bool ret = false;

	MediatorGetPacket * query = (MediatorGetPacket *)buffer;
	
	unsigned int msgLength = query->size;	
	
	CLogArg ( "HandleRequest: [%s]", buffer + 4 );

	// Scan parameters
    char empty [2] = {0};
    
	char * params [ 4 ];
	memset ( params, 0, sizeof(params) );
	
	if ( !ScanForParameters ( buffer + sizeof(MediatorGetPacket), msgLength - sizeof(MediatorGetPacket), ";", params, 4 ) ) {
		CErr ( "HandleRequest: Invalid parameters." );
		return false;
	}

	if ( !params [ 0 ] || !params [ 1 ] || !params [ 2 ] ) {
		CErr ( "HandleRequest: Invalid parameters. area and/or key missing!" );
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
	map<string, ValuePack*> *				values	= 0;
	sp ( AppsList )							apps;
	map<string, map<string, ValuePack*>*>::iterator appsIt;
	map<string, ValuePack*>::iterator valueIt;

	if ( !MutexLock ( &areasMutex, "HandleRequest" ) )
		return false;

	string areaName ( params [ 0 ] );

	msp ( string, AppsList )::iterator areaIt = areas.find ( areaName );			

	// COMMAND: Set New Value
	if ( query->cmd == MEDIATOR_CMD_SET )
	{
		if ( areaIt == areas.end() )
		{
			// Create a new map for the area
			apps = make_shared < AppsList > ();
			if ( !apps ) {
				CErrArg ( "HandleRequest: Failed to create new area [%s].", params [ 0 ] );
				goto Continue;
			}
			areas [ areaName ] = apps;
		}
		else
			apps = areaIt->second;
		
		appsIt = apps->apps.find ( string ( params [ 1 ] ) );
		if ( appsIt == apps->apps.end () ) 
		{
			string appName ( params [ 1 ] );

			values = new map<string, ValuePack*> ();
			if ( !values ) {
				CErrArg ( "HandleRequest: Failed to create new application [%s].", params [ 1 ] );
				goto Continue;
			}
			apps->apps [ appName ] = values;
		}
		else
			values = appsIt->second;
		
		if ( !params [ 2 ] || !params [ 3 ] ) {
			CErrArg ( "HandleRequest: Invalid parameters. Missing key for [%s]", params [ 1 ] );
			goto Continue;
		}
			
		char * key = params [ 2 ];
		if ( endsWith ( string ( key ), string ( "_push" ) ) ) {
			int len = (int) strlen ( key );
			key [ len - 5 ] = 0;
			int clientID = -1;
			sscanf_s ( key, "%d", &clientID );
			if ( clientID > 0 ) {
				CLogArg ( "HandleRequest: sending push notification to client [%i: %s]", clientID, params [ 3 ] );

				if ( !SendPushNotification ( values, clientID, params [ 2 ] ) ) {
					CErrArg ( "HandleRequest: sending push notification to client [%i] failed. [%s]", clientID, params [ 3 ] );
				}
				else ret = true;
			}
			goto Continue;
		}
		
		int valueSize = (int) (msgLength - (params [ 3 ] - buffer));
		if ( valueSize <= 0 ) {
			CErrArg ( "HandleRequest: size of value of [%i] invalid!", valueSize );
			goto Continue;
		}
		if ( !addToArea ( values, params [ 2 ], params [ 3 ], (unsigned) valueSize ) ) {
			CErrArg ( "HandleRequest: Adding key [%s] failed!", params [ 2 ] );
			goto Continue;
		}

		CLogArg ( "HandleRequest: [%s/%s] +key [%s] value [%s]", params [ 0 ], params [ 1 ], params [ 2 ], params [ 3 ] );
		
		goto Continue;	
	}
		
	// Look whether we already have a value for the key
	if ( query->cmd == MEDIATOR_CMD_GET )
	{			
		const char * response = "---";
		size_t length	= strlen ( response );

		if ( areaIt != areas.end() ) {
			apps = areaIt->second;
				
			appsIt = apps->apps.find ( string ( params [ 1 ]) );
			if ( appsIt != apps->apps.end () ) {
				values = appsIt->second;

				valueIt = values->find ( string ( params [ 2 ] ) );

				if ( valueIt != values->end() ) {
					response = valueIt->second->value.c_str ();
					length = valueIt->second->size;
				}
			}
		}

		// Add 4 bytes for the message size
		length += 5;

		char * sendBuffer = (char *) malloc ( length  );
		if ( !sendBuffer ) {
			CErrArg ( "HandleRequest: Failed to allocate buffer of size [%u] for sending the requested value.", (unsigned int)length );
			goto Continue;
		}
		
		*((unsigned int *)sendBuffer) = (unsigned int) length;
		memcpy ( sendBuffer + 4, response, length - 5 );
		sendBuffer [ length - 1 ] = 0;
		
		int sentBytes = SendBuffer ( client, sendBuffer, (unsigned int)length );
		if ( sentBytes != (int)length ) {
			CErrArg ( "HandleRequest: Failed to response value [%s] for key [%s]", response, params [ 1 ] );
		}
		else ret = true;

		free ( sendBuffer );

		goto Continue;
	}

	CWarnArg ( "HandleRequest: command [%c] not supported anymore", query->cmd );
					
Continue:
	MutexUnlock ( &areasMutex, "HandleRequest" );
	return ret;
}


sp ( ThreadInstance ) MediatorDaemon::GetSessionClient ( long long sessionID )
{
	CVerbVerbArg ( "GetSessionClient [%d]", sessionID );

	sp ( ThreadInstance ) client = 0;

	MutexLockV ( &sessionsMutex, "GetSessionClient" );

	//sid |= ((long long)((client->device->root->id << 16) | client->device->root->areaId)) << 32;
			
	msp ( long long, ThreadInstance )::iterator sessionIt = sessions.find ( sessionID );
	if ( sessionIt != sessions.end () ) 
	{
		client = sessionIt->second;
	}

	MutexUnlockV ( &sessionsMutex, "GetSessionClient" );
	
	CVerbVerbArg ( "GetSessionClient: [%s]", client ? client->uid : "NOT found" );

	return client;
}


int MediatorDaemon::HandleRegistration ( int &deviceID, const sp ( ThreadInstance ) &client, unsigned int bytesLeft, char * msg, unsigned int msgLen )
{
	CVerbArg ( "HandleRegistration [ %s ]", client->ips );

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
					CWarnArg ( "HandleRegistration [ %s ]:\tSpare socket registration packet overflow.", client->ips ); break;
				}

				if ( !MutexLock ( &acceptClientsMutex, "HandleRegistration" ) )
					break;
				relClient = GetSessionClient ( regPack->sessionID );
			}
			else {
				useSession = false;

				regLen = *((unsigned int *)(msg + 12));
				if ( regLen + 12 > bytesLeft ) {
					CWarnArg ( "HandleRegistration [ %s ]:\tSpare socket registration packet overflow.", client->ips ); break;
				}

				/// Get the spare id
				unsigned int spareIDa = *((unsigned int *) (msg + 8));

				if ( !MutexLock ( &acceptClientsMutex, "HandleRegistration" ) )
					break;

				msp ( unsigned int, ThreadInstance )::iterator iter = spareClients.find ( spareIDa );
				if ( iter == spareClients.end () ) {
					CLogArg ( "HandleRegistration [ %s ]:\tSpare id [%u] not found.", client->ips, spareIDa );
                
					MutexUnlockV ( &acceptClientsMutex, "HandleRegistration" );
					break;
				}

				relClient = iter->second;
				spareClients.erase ( iter );
			}

			if ( !MutexUnlock ( &acceptClientsMutex, "HandleRegistration" ) )
				break;
						
			if ( !relClient ) {
				CWarnArg ( "HandleRegistration [ %s ]:\tSpare socket client does not exist.", client->ips ); break;
			}

			//CLogArg ( "HandleRegistration: Encrypted [%s]", ConvertToHexSpaceString ( msg + 16, regLen ) );

			if ( !AESDecrypt ( &relClient->aes, useSession ? &regPack->payload : (msg + 16), &regLen, &decrypted ) )
				break;
						
			if ( regLen != MEDIATOR_BROADCAST_SPARE_ID_LEN ) {
				CWarnArg ( "HandleRegistration [ %s ]:\tSpare socket registration packet is not of correct type.", client->ips ); break;
			}

			HandleSpareSocketRegistration ( client.get (), relClient, decrypted + 4, regLen );
			CVerbArg ( "HandleRegistration [ %s ]:\tClosing spare socket reg. thread [0x%X].", client->ips, relClient->deviceID );

			ret = 0;
		}
		while ( 0 );
	}
	else if ( msg [ 7 ] == MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID ) {
		CLogArg ( "HandleRegistration [ %s ]:\tAssigning next device ID.", client->ips );

		msg [ msgLen ] = 0;
		MediatorReqMsg	* req = (MediatorReqMsg *) msg;
		
        sp ( ApplicationDevices ) appDevices;
		int nextID = 0;
		int mappedID = 0;

		if ( MutexLock ( &usersDBMutex, "HandleRegistration" ) )
		{
			sp ( DeviceMapping ) mapping;

            if ( *req->deviceUID ) {
                CVerbArg ( "HandleRegistration [ %s ]:\tLooking for a mapping to deviceUID [%s].", client->ips, req->deviceUID );
                
				msp ( string, DeviceMapping )::iterator devIt = deviceMappings.find ( string ( req->deviceUID ) );	

                if ( devIt != deviceMappings.end ( ) )
                {
					mapping = devIt->second;
                    nextID = mappedID = mapping->deviceID;
                    
                    CVerbArg ( "HandleRegistration [ %s ]:\tMapping found deviceID [0x%X] authLevel [%i] authToken [...].", client->ips, nextID, mapping->authLevel );
				}			
			}

            if ( !mapping ) {
                CVerbArg ( "HandleRegistration [ %s ]:\tNo mapping found. Creating new.", client->ips );
                
				mapping = sp ( DeviceMapping ) ( new DeviceMapping ); // calloc ( 1, sizeof(DeviceMapping) );
				if ( !mapping )
                    goto PreFailExit;
				memset ( mapping.get (), 0, sizeof(DeviceMapping) );
			}

			GetDeviceList ( req->areaName, req->appName, 0, 0, &appDevices );
			if ( appDevices ) 
			{
				/// Find the next free deviceID			
				DeviceInstanceNode	*	device				= appDevices->devices;

				if ( MutexLock ( &appDevices->mutex, "HandleRegistration" ) )
				{
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

					MutexUnlockV ( &appDevices->mutex, "HandleRegistration" );
				}

				UnlockApplicationDevices ( appDevices.get () );
			}

			if ( nextID && (nextID != mappedID) ) {
				mapping->deviceID = nextID;
                
                CVerbArg ( "HandleRegistration [ %s ]:\tCreated a mapping with deviceID [0x%X].", client->ips, nextID );

				deviceMappings [ string ( req->deviceUID ) ] = mapping;
			}
            
		PreFailExit:
			MutexUnlockV ( &usersDBMutex, "HandleRegistration" );
            
            if ( !mapping ) {
                CErrArg ( "HandleRegistration [ %s ]:\tFailed to find or create a mapping.", client->ips );
                return false;
            }
            
            if ( nextID )
                SaveDeviceMappings ();
		}	
				
		CLogArg ( "HandleRegistration [ %s ]:\tAssigning device ID [%u] to [%s].", client->ips, nextID, *req->deviceUID ? req->deviceUID : "Unknown" );

		SendBuffer ( client.get (), &nextID, sizeof(nextID) );
		return 1;
	}
	else {
        /// Message seems to be already decrypted
        
		deviceID = *( (unsigned int *) (msg + 16) );
		
		CLogArgID ( "HandleRegistration [ %s ]:\t[%c%c%c%c]", client->ips, msg [ 4 ], msg [ 5 ], msg [ 6 ], msg [ 7 ] );
					
		/*if ( msgLen == MEDIATOR_BROADCAST_SPARE_ID_LEN ) {
			HandleSpareSocketRegistration ( client, deviceID );
			CVerbID ( "HandleRegistration: Closing spare socket reg. thread." );
			return 0;
		}
		else */
		if ( msgLen >= (MEDIATOR_BROADCAST_DESC_START + 3) ) {
			client->deviceID = deviceID;

			if ( !HandleDeviceRegistration ( client, (unsigned int) client->addr.sin_addr.s_addr, msg + 4 ) ) {
				CWarnArgID ( "HandleRegistration [ %s ]:\tDevice registration failed.", client->ips );
			}
			else {
				CLogArgID ( "HandleRegistration [ %s ]:\tDevice registration succeeded.", client->ips );
				return 1;
			}
		}
	}
	
	if ( decrypted) free ( decrypted );

	return ret;
}


void * MediatorDaemon::ClientThread ( void * arg )
{
	CVerb ( "ClientThread started" );

	ThreadInstance * client = ( ThreadInstance * ) arg;
    
    CLogArg ( "ClientThread started for [ %s ]", client->ips );
   
    sp ( ThreadInstance ) clientSP = client->clientSP;
	client->clientSP = 0;

	int deviceID = 0;
	char * buffer = 0;

	socklen_t addrLen = sizeof(client->addr);

	CVerbArg ( "Client [ %s ]:\tSource port [%u] socket [%i]", client->ips, ntohs ( client->addr.sin_port ), client->socket );

	char		*	msg;
	char		*	msgEnd;
	int				bytesReceived, sock = client->socket;
	unsigned int	remainingSize	= MEDIATOR_CLIENT_MAX_BUFFER_SIZE - 1;
    char		*	msgDec			= 0;
	unsigned int	msgDecLength;
    char		*	decrypted		= 0;
	up ( char [ ] ) bufferUP;

    bool isBanned = IsIpBanned ( client->addr.sin_addr.s_addr );
	if ( isBanned )
		goto ShutdownClient;

	bufferUP = up ( char [ ] ) ( new char [ MEDIATOR_CLIENT_MAX_BUFFER_SIZE + 1 ] );
	if ( !bufferUP )
		goto ShutdownClient;

	msgEnd = buffer = bufferUP.get ();
	
#ifndef NDEBUG
	try
	{
#endif
	while ( isRunning ) {
		bytesReceived = (int)recvfrom ( sock, msgEnd, remainingSize, 0, (struct sockaddr*) &client->addr, &addrLen );
		if ( bytesReceived <= 0 ) {
			CVerbArgID ( "Client [ %s ]:\tconnection/socket [%i] closed by someone; Bytes [%i]!", client->ips, sock, bytesReceived );
			//LogSocketError ();
			break;
		}

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

			CVerbVerbArgID ( "Client [ %s ]:\tmsgLength [%d] Bytes left [%i]!", client->ips, msgLength, bytesLeft );

			if ( msgLength > (unsigned int) bytesLeft ) 
			{
				if ( msgLength >= MEDIATOR_CLIENT_MAX_BUFFER_SIZE ) {
					CErrArgID ( "Client [ %s ]:\tmsgLength [%d] >= [%i]!", client->ips, msgLength, MEDIATOR_CLIENT_MAX_BUFFER_SIZE );

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
                            CWarnArgID ( "Client [ %s ]:\tDecrypt with privKey failed!", client->ips ); goto ShutdownClient;
                        }
                        msgDec = decrypted;
                    }
					else {
						CErrArgID ( "Client [ %s ]:\tUnknown encryption flags", client->ips );
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
					CWarnArgID ( "Client [ %s ]:\tTried to establish a safe channel again. Not allowed.", client->ips );
					goto ShutdownClient;
				}

				if ( !SecureChannelAuth ( client ) ) {
					CWarnArgID ( "Client [ %s ]:\tEstablishing Security / Authentication failed.", client->ips  );
					goto ShutdownClient;
				}
				CLogArgID ( "Client [ %s ]:\tEstablishing Security / Authentication succeeded.", client->ips );
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
					CVerbArgID ( "Client [ %s ]:\tHandle registration failed.", client->ips );
					goto ShutdownClient;
				}
		
				CLogArgID ( "Client [ %s ]:\tInvalid message [%c%c%c%c]. Refusing client.", client->ips, msgDec [ 4 ], msgDec [ 5 ], msgDec [ 6 ], msgDec [ 7 ] );
				goto ShutdownClient;
			}

			CVerbArg ( "Client [ %s ]:\t[%c%c%c%c]", client->ips, msgDec [ 4 ], msgDec [ 5 ], msgDec [ 6 ], msgDec [ 7 ] );

			if ( !client->deviceID ) {
				CWarnArgID ( "Client [ %s ]:\tRequest has not been identified. kicking connect.", client->ips );

				if ( !client->authenticated )
					BannIP ( client->addr.sin_addr.s_addr );
				goto ShutdownClient;
			}

			if ( msgDec [ 4 ] < MEDIATOR_PROTOCOL_VERSION_MIN ) {
				CWarnArgID ( "Client [ %s ]:\tVersion [%c] not supported anymore", client->ips, msgDec [ 4 ] );
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
					CErrArgID ( "Client [ %s ]:\tFailed to response NAT value [%d]", client->ips, behindNAT );
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
			// COMMAND: 
			else if ( command == MEDIATOR_CMD_REQ_SPARE_ID ) 
			{
				MutexLockV ( &acceptClientsMutex, "Client" );

				unsigned int sid = spareID++; // Make use of sync_fetch_and_add?

				MutexUnlockV ( &acceptClientsMutex, "Client" );

				CVerbArgID ( "Client [ %s ]:\tAssigned spare ID [%u]", client->ips, sid );

				spareClients [ sid ] = clientSP;
				
				*((unsigned int *) msgDec) = MEDIATOR_NAT_REQ_SIZE;
				*((unsigned int *) (msgDec + 4)) = sid;
				
				int sentBytes = SendBuffer ( client, msgDec, MEDIATOR_NAT_REQ_SIZE );

				if ( sentBytes != MEDIATOR_NAT_REQ_SIZE ) {
					CErrArgID ( "Client [ %s ]:\tFailed to response spare client ID [%u]", client->ips, sid );
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
                    CErrArgID ( "Client [ %s ]:\tFailed to response Mediator version", client->ips );
                }
            }
			else {
				msgDec [ msgDecLength - 1 ] = 0;

                HandleRequest ( msgDec, client );
                
                /*if ( !HandleRequest ( msgDec, client ) ) {
                    *((unsigned int *) msg) = 8;
                    msg [ 6 ] = 'e';
                    int sentBytes = SendBuffer ( client, msg, 8 );
                    if ( sentBytes != 8 ) {
                        CErrID ( "Client: Failed to response error" );
                    }
                }*/
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
	
#ifndef NDEBUG
	}
	catch (const std::exception &exc)
	{
		// catch anything thrown within try block that derives from std::exception
		CErrArg ( "Client: [%s]", exc.what() );
#if ( defined(_WIN32) )
		::_CrtDbgBreak ();
#endif
	}
#endif

ShutdownClient:
    MutexLockV ( &client->accessMutex, "Client" );
    
    sock = client->socket;
    if ( sock != -1 ) {
        client->socket = -1;
        
        shutdown ( sock, 2 );
        closesocket ( sock );
    }
    
    MutexUnlockV ( &client->accessMutex, "Client" );

    if ( decrypted ) free ( decrypted );

	if ( isRunning )
	{
		if ( !client->deviceID && client->addr.sin_addr.s_addr ) {
            if ( !isBanned ) {
				CWarnArgID ( "Client [ %s:%i ]:\tRequest has not been identified. kicking connect.", client->ips, client->port );
                BannIP ( client->addr.sin_addr.s_addr );
                isBanned = true;
            }
		}
        
        if ( !isBanned )
            UpdateNotifyTargets ( clientSP, -1 );

		pthread_t thrd = client->threadID;
		if ( pthread_valid ( thrd ) ) 
		{
			pthread_reset ( client->threadID );
			pthread_detach_handle ( thrd );
		}
	}
    else {
        UpdateNotifyTargets ( clientSP, -1 );
    }

	CLogArgID ( "Client [ %s:%i ]:\tDisconnected", client->ips, client->port );
	return 0;
}


DeviceInstanceNode * MediatorDaemon::GetDeviceInstance ( int deviceID, DeviceInstanceNode * device )
{
	while ( device ) {
		// Exception here
		if ( device->info.deviceID == deviceID ) {
			return device;
		}
		device = device->next;
	}
	return 0;
}


bool MediatorDaemon::HandleShortMessage ( ThreadInstance * sourceClient, char * msg )
{
	CVerb ( "HandleShortMessage" );

	// size of message (including size prefix)
	// 2m;;
	// 1. destination ID
	// 2. message ( size - 12 )

	ShortMsgPacket	*	shortMsg			= (ShortMsgPacket *)msg;
	bool				sendError			= true;

	char			*	sendBuffer			= msg;
	int					length				= shortMsg->size;

	unsigned int		destID				= shortMsg->deviceID;
	unsigned int		deviceID			= sourceClient->deviceID;
	const char		*	areaName			= 0;
	const char		*	appName				= 0;
	int					sentBytes;
	DeviceInstanceNode	* device;
	    
	// find the destination client
	sp ( ThreadInstance ) destClient;
				
	DeviceInstanceNode  *       destList    = 0;
    pthread_mutex_t     *       destMutex   = 0;
    sp ( ApplicationDevices )   appDevices  = 0;

	if ( shortMsg->version >= '3' ) {
		if ( *shortMsg->areaName && *shortMsg->appName ) {
			areaName = shortMsg->areaName;
			appName = shortMsg->appName;
		}
	}

	sp ( DeviceInstanceNode ) sourceDeviceSP = sourceClient->deviceSP;

	DeviceInstanceNode * sourceDevice = sourceDeviceSP.get ( );

    
    if ( !areaName || !appName ) {
        //areaName = sourceDevice->info.areaName;
        //appName = sourceDevice->info.appName;

		if ( !sourceDevice )
			return false;

        destMutex = &sourceDevice->rootSP->mutex;
    }
    else {
        appDevices = GetApplicationDevices ( areaName, appName );
        if ( !appDevices ) {
            goto SendResponse;
        }
        
        destMutex = &appDevices->mutex;
    }
    
    if ( !MutexLock ( destMutex, "HandleShortMessage" ) ) {
        destMutex = 0;
        goto SendResponse;
    }

    if ( appDevices ) {
        destList = appDevices->devices;
        
		UnlockApplicationDevices ( appDevices.get () );
    }
    else
        destList = sourceDevice->rootSP->devices;
	
    if ( !destList )
        goto SendResponse;
    
	device = GetDeviceInstance ( destID, destList );
	if ( device )
		destClient = device->clientSP;

	if ( !destClient || destClient->socket == -1 ) {
		CErrArgID ( "HandleShortMessage: Failed to find device connection for id [0x%X]!", destID );
		goto SendResponse;
	}
	
	shortMsg->deviceID = deviceID;
    if ( sourceDevice ) {
        if ( *sourceDevice->info.areaName )
            strcpy_s ( shortMsg->areaName, sizeof(shortMsg->areaName) - 1, sourceDevice->info.areaName );
        if ( *sourceDevice->info.appName )
            strcpy_s ( shortMsg->appName, sizeof(shortMsg->appName) - 1, sourceDevice->info.appName );
    }
        
	CLogArgID ( "HandleShortMessage: send message to device [%u] IP [%u bytes -> %s]", destID, length, inet_ntoa ( destClient->addr.sin_addr ) );
	
	sentBytes = SendBuffer ( destClient.get (), sendBuffer, length );
	if ( sentBytes == length ) {
		CLogID ( "HandleShortMessage: successfully sent message." );
		sendError = false;
	}

SendResponse:
	if ( destMutex )
		MutexUnlockV ( destMutex, "HandleShortMessage" );

	length = 12;
	*((unsigned int *) sendBuffer) = length;

	if ( sendError ) {
		sendBuffer [ 6 ] = 'e';	
	}
	else {
		sendBuffer [ 6 ] = 's';	
	}
	
	CLogArgID ( "HandleShortMessage: send reply to IP [%s]", inet_ntoa ( sourceClient->addr.sin_addr ) );
	
	sentBytes = SendBuffer ( sourceClient, sendBuffer, length );
	if ( sentBytes != length ) {
		CErrArgID ( "HandleShortMessage: Failed to send reply to requestor IP [%s]", inet_ntoa ( sourceClient->addr.sin_addr ) );
		LogSocketError ();
		return false;
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

	unsigned int listCount	= appDevices->count;
	
	if ( startIndex > listCount ) {
		startIndex -= listCount; 
		return true;
	}
	
	const unsigned int	maxDeviceCount		= (MEDIATOR_BUFFER_SIZE_MAX - DEVICES_HEADER_SIZE) / DEVICE_PACKET_SIZE;
    if ( resultCount >= maxDeviceCount )
        return false;
    
	bool                    ret					= true;
	DeviceInfoNode		*	curDevice			= resultList;
	unsigned int            currentStart;
	DeviceInstanceNode	*	device; //				= appDevices->devices;
		
	if ( !MutexLock ( &appDevices->mutex, "CollectDevices" ) )
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
					ret = false; goto Finish;
				}
                
                memcpy ( newDevice, device, sizeof(DeviceInfo) );
				//*newDevice->userName = 0;
                newDevice->next = 0;
                
                resultList = newDevice;
                resultCount = 1;
				ret = false; goto Finish; /// Stop the query, We have found the device.
            }
            
            device = device->next;
            continue;
        }
        
        /// We have a device to insert into the result list
		DeviceInfoNode * newDevice = (DeviceInfoNode *) malloc ( sizeof( DeviceInfoNode ) );
        //DeviceInstanceNode * newDevice = new DeviceInstanceNode;
        if ( !newDevice ) {
			ret = false; goto Finish;
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
			ret = false; goto Finish;
		}

		device = device->next;
	}
    
Finish:
	if ( !MutexUnlock ( &appDevices->mutex, "CollectDevices" ) )
		return false;
	return ret;
}


unsigned int MediatorDaemon::CollectDevicesCount ( DeviceInstanceNode * sourceDevice, int filterMode )
{
	if ( !sourceDevice || !sourceDevice->rootSP )
		return 0;

	unsigned int				deviceCount = 0;
    sp ( AreaApps )				areaApps;
    sp ( ApplicationDevices )	appDevices;

	/// Get number of devices within the same application environment
	deviceCount += sourceDevice->rootSP->count;

	if ( filterMode < 1 ) { 
		/// No filtering, get them all		
		if ( !MutexLock ( &devicesMutex, "CollectDevices" ) )
			goto Finish;

		/// Iterate over all areas
		for ( msp ( string, AreaApps )::iterator it = areasList.begin(); it != areasList.end (); ++it )
		{
			if ( !it->second )
				continue;
						
			areaApps = it->second;

			for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin(); ita != areaApps->apps.end (); ++ita )
			{
				appDevices = ita->second;
				if ( !appDevices || appDevices.get () == sourceDevice->rootSP.get () )
					continue;

				deviceCount += appDevices->count;
			}
		}
			
		if ( pthread_mutex_unlock ( &devicesMutex ) ) {
			CErr ( "CollectDevicesCount: Failed to release mutex!" );
		}
	}
	else if ( filterMode < 2 ) {
		/// Get number of devices of other application environments within the same area
		string areaName ( sourceDevice->info.areaName );

		if ( !MutexLock ( &devicesMutex, "CollectDevices" ) )
			goto Finish;

		msp ( string, AreaApps )::iterator areaIt = areasList.find ( areaName );

		if ( areaIt == areasList.end ( ) ) {
			CLogArg ( "CollectDevicesCount: Area [%s] not found.", sourceDevice->info.areaName );
			goto FinishLimitApps;
		}
	
		areaApps = areaIt->second;    
		if ( !areaApps ) {
			CLog ( "CollectDevicesCount: Invalid area name." );
			goto FinishLimitApps;
		}
				
		for ( msp ( string, ApplicationDevices )::iterator ita = areaApps->apps.begin(); ita != areaApps->apps.end (); ++ita )
		{
			appDevices = ita->second;
			if ( !appDevices || appDevices.get () == sourceDevice->rootSP.get () )
				continue;

			deviceCount += appDevices->count;
		}

	FinishLimitApps:		
		MutexUnlockV ( &devicesMutex, "CollectDevices" );
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
	if ( query->cmdVersion < MEDIATOR_PROTOCOL_VERSION )
		/// Deprecated, to be removed as soon as the public tree has been updated
		subCmd = query->opt0;
	else
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
		CVerbID ( "HandleQueryDevices: requested count" );
		
        length = MEDIATOR_CMD_GET_DEVICES_COUNT_RESP_LEN;

		if ( query->cmdVersion < MEDIATOR_PROTOCOL_VERSION ) {
			/// Deprecated, to be removed as soon as the public tree has been updated
			response->size = length;
			response->cmd0 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			response->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT;
            
			response->deviceHead.deviceCountAvailable = CollectDevicesCount ( sourceDevice, filterMode );
		}
		else {
			query->size = length;
			query->cmdVersion = MEDIATOR_PROTOCOL_VERSION;
			query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			query->opt0 = MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT;

			query->msgID = CollectDevicesCount ( sourceDevice, filterMode );
		}

		CVerbArgID ( "HandleQueryDevices: Number of devices [%u]", query->msgID );
		error = false; goto SendResponse;
	}
	
	availableDevices = appDevices->count;

	startIndex = query->startIndex;
		
	if ( query->cmdVersion < MEDIATOR_PROTOCOL_VERSION && startIndex > 0 )
		/// Deprecated, to be removed as soon as the public tree has been updated
		startIndex--;
	
	if ( query->cmdVersion < MEDIATOR_PROTOCOL_VERSION )
		/// Deprecated, to be removed as soon as the public tree has been updated
		subCmd = query->opt1;
	else
		subCmd = query->opt0;

	if ( subCmd == MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID ) {
		deviceIDReq = query->deviceID;
		if ( *query->areaName && *query->appName ) {
			areaName = query->areaName;
			appName = query->appName;

			CVerbArgID ( "HandleQueryDevices: Query for deviceID [%u] [%s/%s]", deviceIDReq, areaName, appName );
		}
		else {
			areaName = sourceDevice->info.areaName;
			appName = sourceDevice->info.appName;

			CVerbArgID ( "HandleQueryDevices: Query for deviceID [%u] of default appEnv [%s/%s]", deviceIDReq, areaName, appName );
		}
	}

	CVerbArgID ( "HandleQueryDevices: start index [%u] filterMode [%i]", startIndex, filterMode );

	/// Lets start with the devices within the same appEnv	

	if ( appDevices->count && !CollectDevices ( sourceClient.get (), startIndex, deviceIDReq, areaName, appName,
		appDevices.get (), resultList, resultCount ) ) 
		goto Finish;
	
	if ( filterMode < 1 ) { 
		/// No filtering, get them all	
		if ( !MutexLock ( &devicesMutex, "HandleQueryDevices" ) )
			goto Finish;

		/// Iterate over all areas
		for ( msp ( string, AreaApps )::iterator it = areasList.begin(); it != areasList.end (); ++it )
		{
			if ( !it->second )
				continue;
			apps = &it->second->apps;

			for ( msp ( string, ApplicationDevices )::iterator ita = apps->begin(); ita != apps->end (); ita++ )
			{
				appDevices = ita->second;
				if ( !appDevices || appDevices.get () == sourceDevice->rootSP.get () || !appDevices->count  )
					continue;

				availableDevices += appDevices->count;

				if ( !CollectDevices ( sourceClient.get (), startIndex, deviceIDReq, areaName, appName,
					appDevices.get (), resultList, resultCount ) ) {
					goto FinishNoLimit;
				}
			}
		}

FinishNoLimit:		
		if ( pthread_mutex_unlock ( &devicesMutex ) ) {
			CErr ( "HandleQueryDevices: Failed to release mutex!" );
		}
		goto Finish;
	}
	else if ( filterMode < 2 ) {
		/// Get number of devices of other application environments within the same area
		string pareaName ( areaName );

		if ( !MutexLock ( &devicesMutex, "HandleQueryDevices" ) )
			goto Finish;

		msp ( string, AreaApps )::iterator areaIt = areasList.find ( pareaName );

		if ( areaIt == areasList.end ( ) || !areaIt->second ) {
			CLogArg ( "HandleQueryDevices: Area [%s] not found.", areaName );
			goto FinishLimitArea;
		}

		apps = &areaIt->second->apps;
				
		for ( msp ( string, ApplicationDevices )::iterator ita = apps->begin(); ita != apps->end (); ita++ )
		{
			appDevices = ita->second;
			if ( !appDevices || appDevices == sourceDevice->rootSP || !appDevices->count )
				continue;

			availableDevices += appDevices->count;

			if ( !CollectDevices ( sourceClient.get (), startIndex, deviceIDReq, areaName, appName,
				appDevices.get (), resultList, resultCount ) ) {
					goto FinishLimitArea;
			}
		}

	FinishLimitArea:
		MutexUnlockV ( &devicesMutex, "HandleQueryDevices" );
		goto Finish;
	}

Finish:
	error = false;
    response = (MediatorQueryResponse *) FlattenDeviceList ( resultList, resultCount );
    if ( !response ) {
		query->size = length;
		if ( query->cmdVersion < MEDIATOR_PROTOCOL_VERSION ) {
			/// Deprecated, to be removed as soon as the public tree has been updated
			query->cmdVersion = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR;
		}
		else {
			query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			query->opt0 = MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR;
		}
		goto SendResponse;
	}

        
	sendBuffer = (char *) response;
    
	response->cmd0 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
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
		if ( query->cmdVersion < MEDIATOR_PROTOCOL_VERSION ) {
			/// Deprecated, to be removed as soon as the public tree has been updated
			query->cmdVersion = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR;
		}
		else {
			query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			query->opt0 = MEDIATOR_CMD_DEVICE_LIST_QUERY_ERROR;
		}
		length = query->size = 8;
	}

	CLogArgID ( "HandleQueryDevices: send devicelist%s [%d bytes -> %s]", sendBuffer [ 5 ] == 'c' ? "/count" : "", length, inet_ntoa ( sourceClient->addr.sin_addr ) );
	
	int sentBytes = SendBuffer ( sourceClient.get (), sendBuffer, length );

	if ( sendBuffer != msg )
		free ( sendBuffer );
	
	if ( sentBytes != length ) {
		CErrArgID ( "HandleQueryDevices: Failed to send devicelist response to IP [%s]", inet_ntoa ( sourceClient->addr.sin_addr ) );
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
    
    memcpy ( buffer + 4, "3t;;", 4 );
    buffer [ 4 ] = MEDIATOR_PROTOCOL_VERSION;
    buffer [ 5 ] = MEDIATOR_CMD_HELP_TLS_GEN;
    
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
	unsigned int * pUI = (unsigned int *) (msg + 8);
	unsigned int SourceID = *pUI; pUI++;
	unsigned int DestID = *pUI;

	CVerbArg ( "[0x%X].HandleSTUNRequest: TCPrec -> [0x%X]", SourceID, DestID );

	// find the source client
	sp ( ThreadInstance ) destClient;

	if ( !MutexLock ( &sourceDevice->rootSP->mutex, "HandleSTUNRequest" ) )
		return false;
	
	DeviceInstanceNode * device = GetDeviceInstance ( DestID, sourceDevice->rootSP->devices );
	
	if ( device )
		destClient = device->clientSP;

	if ( !destClient ) {
		CWarnArg ( "[0x%X].HandleSTUNRequest -> Destination device -> [0x%X] not found.", SourceID, DestID );
		
		MutexUnlockV ( &sourceDevice->rootSP->mutex, "HandleSTUNRequest" );
		return false;
	}
    
	buffer [ 4 ] = 'y';
	buffer [ 5 ] = ';';
	buffer [ 6 ] = ';';
	buffer [ 7 ] = ';';

	pUI = (unsigned int *) buffer;
	*pUI = MEDIATOR_STUN_RESP_SIZE; pUI += 2;

	*pUI = SourceID; pUI++;
	*pUI = (unsigned int) sourceClient->addr.sin_addr.s_addr; pUI++;
	*pUI = sourceClient->portUdp;

	CLogArg ( "[0x%X].HandleSTUNRequest: send STUN request to device IP [%s]!", DestID, inet_ntoa ( destClient->addr.sin_addr ) );
	
	int sentBytes = SendBuffer ( destClient.get (), buffer, MEDIATOR_STUN_RESP_SIZE );
	if ( sentBytes != MEDIATOR_STUN_RESP_SIZE ) {
		CErrArg ( "[0x%X].HandleSTUNRequest: Failed to send STUN request to device IP [%s]!", DestID, inet_ntoa ( destClient->addr.sin_addr ) );
		//return false;
	}
	
	buffer [ 3 ] = '-';
	pUI = (unsigned int *) buffer; pUI++;
	*pUI = DestID; pUI++;
	*pUI = (unsigned int) destClient->addr.sin_addr.s_addr; pUI++;
	*pUI = destClient->portUdp; pUI++;
	*pUI = 0;
	
	CLogArg ( "[0x%X].HandleSTUNRequest: send STUN reply to device IP [%s]!", DestID, inet_ntoa ( sourceClient->addr.sin_addr ) );
	
	sentBytes = SendBuffer ( sourceClient, buffer, MEDIATOR_STUN_RESP_SIZE );

	MutexUnlockV ( &sourceDevice->rootSP->mutex, "HandleSTUNRequest" );

	if ( sentBytes != MEDIATOR_STUN_RESP_SIZE ) {
		CErrArg ( "[0x%X].HandleSTUNRequest: Failed to send STUN reply to device IP [%s]!", DestID, inet_ntoa ( sourceClient->addr.sin_addr ) );
		return false;
	}

	return true;
}


bool MediatorDaemon::HandleSTUNRequest ( ThreadInstance * destClient, int sourceID, const char * areaName, const char * appName, unsigned int IPe, unsigned int Porte )
{
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
		request.IPi = deviceSP->info.ip;
		request.Porti = deviceSP->info.udpPort;
	}

	strcpy_s ( request.areaName, sizeof ( request.areaName ) - 1, areaName );
	strcpy_s ( request.appName, sizeof ( request.appName ) - 1, appName );

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
	CVerb ( "NotifySTUNTRegRequest" );

	STUNTRegReqPacket	req;
	Zero ( req );
	
	/// Notify a spare socket registration request

	req.size = MEDIATOR_CMD_MEDIATOR_NOTIFY_SIZE;
	req.version = MEDIATOR_PROTOCOL_VERSION;

	memcpy ( req.ident, "i;;", 3 );

	req.notify = NOTIFY_MEDIATOR_SRV_STUNT_REG_REQ;
	    
	CLogArg ( "NotifySTUNTRegRequest: Send spare socket register request to device [0x%X]", client->deviceID );

	int sentBytes = SendBuffer ( client, &req, MEDIATOR_CMD_MEDIATOR_NOTIFY_SIZE, false );
		
	if ( sentBytes != MEDIATOR_CMD_MEDIATOR_NOTIFY_SIZE ) {
		CErrArg ( "NotifySTUNTRegRequest: Failed to send spare socket register request to device IP [%s]!", inet_ntoa ( client->addr.sin_addr ) ); LogSocketError ();
		return false;
	}

	return true;
}


bool MediatorDaemon::HandleSTUNTRequest ( ThreadInstance * sourceClient, STUNTReqPacket * req )
{
#ifndef NDEBUG
	try
	{
#endif
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
	unsigned int		deviceID		= req->deviceID; // *pUI;

	bool				sourceLocked	= false;
	bool				destLocked		= false;
	int					status			= 0;
    const char  *		areaName        = 0;
    const char  *		appName         = 0;
	DeviceInstanceNode *	destDevice			= 0;
    
	// find the destination client
	sp ( ThreadInstance )	destClient		= 0;				
	DeviceInstanceNode *	destList		= 0;
    pthread_mutex_t *		destMutex		= 0;
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

        destMutex = &sourceDevice->rootSP->mutex;
    }
    else {
        appDevices = GetApplicationDevices ( areaName, appName );
        if ( !appDevices ) {
            goto UnlockQuit;
        }
        destMutex = &appDevices->mutex;
    }

	if ( !MutexLock ( destMutex, "HandleSTUNTRequest" ) ) {
		destMutex = 0; goto UnlockQuit;
	}

    if ( appDevices ) {
        destList = appDevices->devices;
		UnlockApplicationDevices ( appDevices.get () );
	}
	else
        destList = sourceDevice->rootSP->devices;

	/// Exception herer
	destDevice = GetDeviceInstance ( deviceID, destList );
	if ( destDevice )
		destClient = destDevice->clientSP;

	if ( !destClient ) {
		CErrArg ( "HandleSTUNTRequest: Failed to find device connection for id [0x%X]!", deviceID ); goto UnlockQuit;
	}
	
	// Acquire the mutex on sourceClient
	if ( !MutexLock ( &sourceClient->accessMutex, "HandleSTUNTRequest" ) ) goto UnlockQuit;
	sourceLocked = true;

	///
	/// Check whether the source client is ready for a stunt
	///
	portSource = sourceClient->sparePort;
	if ( !portSource ) {
		CLogArgID ( "HandleSTUNTRequest: Source client [0x%X] has not renewed the spare socket yet!", sourceClient->deviceID );

		NotifySTUNTRegRequest ( sourceClient ); status = -1; goto UnlockQuit;
	}

	IP = sourceDevice->info.ip;
	//IPe = sourceDevice->IPe; // the IPe of deviceInfo could potentially be collected by broadcast from sometime before...
	// use the ip address of the source received


	// Acquire the mutex on destClient
	if ( !MutexLock ( &destClient->accessMutex, "HandleSTUNTRequest" ) ) goto UnlockQuit;
	destLocked = true;
	
	///
	/// Check whether the destination client is ready for a stunt
	///
	portDest = destClient->sparePort;
	if ( !portDest ) {
		CLogID ( "HandleSTUNTRequest: Destination client has not renewed the spare socket yet!" );

		NotifySTUNTRegRequest ( destClient.get () );  status = -1; goto UnlockQuit;
	}
	
	IPe = sourceClient->addr.sin_addr.s_addr;

	if ( !IP || !IPe ) {
		CErrArgID ( "HandleSTUNTRequest: Invalid ip [%s] IPe [%x] Port [%d] for sourceDevice or invalid Port[%d] for destinationDevice in database!", 
			inet_ntoa ( *((struct in_addr *) &IP) ), IPe, portSource, portDest );
		goto UnlockQuit;
	}

	response.size = sizeof(response);

	memcpy ( response.ident, "x;;", 3 );
	response.channel = channelType;
	response.deviceID = sourceClient->deviceID;
	response.ip = IP;
	response.ipe = IPe;
    response.porti = sourceDevice->info.tcpPort;
    response.porte = portSource;

    strcpy_s ( response.areaName, sizeof(response.areaName) - 1, sourceDevice->info.areaName );
    strcpy_s ( response.appName, sizeof(response.appName) - 1, sourceDevice->info.appName );
	
	CLogArgID ( "STUNTRequest: Send request to device IP [%s], port [%d]", inet_ntoa ( destClient->addr.sin_addr ), portSource );
	
	sentBytes = SendBuffer ( destClient.get (), &response, sizeof(response), false );

	if ( sentBytes != sizeof(response) ) {
		CErrArgID ( "HandleSTUNTRequest: Failed to send STUNT request to device IP [%s]!", inet_ntoa ( destClient->addr.sin_addr ) );
		LogSocketError ();
		goto UnlockQuit;
	}

	sendSize = extResp ? MEDIATOR_STUNT_ACK_EXT_SIZE : MEDIATOR_STUNT_ACK_SIZE;
	reqResponse->size = sendSize;

	reqResponse->respCode = 'p';

    reqResponse->porte = portDest;
    reqResponse->porti = destDevice ? destDevice->info.tcpPort : 0;
	//reqResponse->portUdp = destClient->portUdp;
	
	if ( extResp ) {
		reqResponse->ip = destDevice->info.ip;
		reqResponse->ipe = destClient->addr.sin_addr.s_addr;
	}

	CLogArgID ( "STUNTRequest: Send response to device IP [%s:%d]", inet_ntoa ( sourceClient->addr.sin_addr ), portDest );

	destClient->sparePort = 0; // Clear the port
		
	sentBytes = SendBuffer ( sourceClient, reqResponse, sendSize, false );

	if ( sentBytes != (int) sendSize ) {
		CErrArgID ( "HandleSTUNTRequest: Failed to send STUNT response (Port) to sourceClient device IP [%s]!", inet_ntoa ( sourceClient->addr.sin_addr ) );
		LogSocketError ();
		return false;
	}

	destClient->sparePort = 0;
	sourceClient->sparePort = 0;

	status = 1;


UnlockQuit:	
	/// Release the mutex on destClient
	if ( destLocked )
		MutexUnlockV ( &destClient->accessMutex, "HandleSTUNTRequest" );
	
	/// Release the mutex on sourceClient
	if ( sourceLocked )
		MutexUnlockV ( &sourceClient->accessMutex, "HandleSTUNTRequest" );
	
	if ( destMutex )
		MutexUnlockV ( destMutex, "HandleSTUNTRequest" );

	if ( status > 0 )
		return true;

	reqResponse->size = MEDIATOR_STUNT_ACK_SIZE;

	if ( status < 0 )
		reqResponse->respCode = 'r';
	else		
		reqResponse->respCode = 'e';
	
	sentBytes = SendBuffer ( sourceClient, reqResponse, MEDIATOR_STUNT_ACK_SIZE );
	
	if ( sentBytes != MEDIATOR_STUNT_ACK_SIZE ) {
		CErrArgID ( "HandleSTUNTRequest: Failed to send %s message to sourceClient device", reqResponse->respCode == 'e' ? "Failed" : "Retry" );
		LogSocketError ();
	}
	
#ifndef NDEBUG
	}
	catch (const std::exception &exc)
	{
		// catch anything thrown within try block that derives from std::exception
		CErrArg ( "HandleSTUNTRequest: [%s]", exc.what() );
#if ( defined(_WIN32) )
		::_CrtDbgBreak ();
#endif
	}
#endif
	return false;
}


void MediatorDaemon::BuildBroadcastMessage ( )
{
	CVerb ( "buildBroadcastMessage" );

	// Format E.Mediator port1 port2 ...
	strncpy_s ( broadcastMessage + 4, MEDIATOR_BROADCAST_DESC_START, MEDIATOR_BROADCAST, MEDIATOR_BROADCAST_DESC_START );
	
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


bool MediatorDaemon::UpdateDeviceRegistry ( DeviceInstanceNode * device, unsigned int ip, char * msg )
{
	CVerb ( "UpdateDeviceRegistry" );

    char * keyCat;
    char * value;

	if ( !device ) {
		return false;
	}

	// Update the values in our database
	if ( !MutexLock ( &devicesMutex, "UpdateDeviceRegistry" ) )
		return false;

	char * pareaName = device->info.areaName;
		
	if ( !pareaName || !MutexLock ( &areasMutex, "UpdateDeviceRegistry" ) ) {
		CErr ( "UpdateDeviceRegistry: Failed to aquire mutex on areas/values (alt: areaName invalid)! Skipping client request!" );

		MutexUnlockV ( &devicesMutex, "UpdateDeviceRegistry" );
		return false;
	}
		
	// Search for the area at first
	sp ( AppsList )								apps;

	map<string, ValuePack*>					*	values = 0;
	map<string, map<string, ValuePack*>*>::iterator appsIt;

		
	string areaName ( pareaName );
	string appName ( device->info.appName );

	msp ( string, AppsList )::iterator areaIt = areas.find ( areaName );

	if ( areaIt == areas.end () )
	{		
		apps = make_shared < AppsList > ();
		if ( !apps ) {
			CErrArg ( "UpdateDeviceRegistry: Failed to create new area [%s].", pareaName );
			goto Continue;
		}
		areas [ areaName ] = apps;
	}
	else
		apps = areaIt->second;
	
	appsIt = apps->apps.find ( appName );

	if ( appsIt == apps->apps.end () )
	{		
		values = new map<string, ValuePack*> ();
		if ( !values ) {
			CErrArg ( "UpdateDeviceRegistry: Failed to create new application [%s].", appName.c_str () );
			goto Continue;
		}
		apps->apps [ appName ] = values;
	}
	else
		values = appsIt->second;

	
	char keyBuffer [ 128 ];
	char valueBuffer [ 256 ];
	sprintf_s ( keyBuffer, 128, "%i_", device->info.deviceID );

	keyCat = keyBuffer + strlen ( keyBuffer );
	
	// ip in register message
	strcat_s ( keyCat, 100, "ip" );

	value = inet_ntoa ( *((struct in_addr *) &device->info.ip) );

	if ( !addToArea ( values, keyBuffer, value, (unsigned int) strlen ( value ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}		
	
	// extern ip
	*keyCat = 0;
	strcat_s ( keyCat, 100, "ipe" );

	value = inet_ntoa ( *((struct in_addr *) &ip) );

	if ( !addToArea ( values, keyBuffer, value, (unsigned int) strlen ( value ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}		
	
	// tcp port
	*keyCat = 0;
	strcat_s ( keyCat, 100, "cport" );
	sprintf_s ( valueBuffer, 128, "%u", device->info.tcpPort );

	if ( !addToArea ( values, keyBuffer, valueBuffer, (unsigned int) strlen ( valueBuffer ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}		
	
	// udp port
	*keyCat = 0;
	strcat_s ( keyCat, 100, "dport" );
	sprintf_s ( valueBuffer, 128, "%u", device->info.udpPort );

	if ( !addToArea ( values, keyBuffer, valueBuffer, (unsigned int) strlen ( valueBuffer ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}
	
	// device type
	*keyCat = 0;
	strcat_s ( keyCat, 100, "type" );
	sprintf_s ( valueBuffer, 128, "%i", device->info.platform );

	if ( !addToArea ( values, keyBuffer, valueBuffer, (unsigned int) strlen ( valueBuffer ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}
	
	// device name
	*keyCat = 0;
	strcat_s ( keyCat, 100, "name" );

	if ( !addToArea ( values, keyBuffer, device->info.deviceName, (unsigned int) strlen ( device->info.deviceName ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}
			
Continue:
	MutexUnlockV ( &areasMutex, "UpdateDeviceRegistry" );

	MutexUnlockV ( &devicesMutex, "UpdateDeviceRegistry" );

	if ( MutexLock ( &thread_mutex, "UpdateDeviceRegistry" ) ) {

		configDirty = true;
		if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
			CErr ( "UpdateDeviceRegistry: Error to signal watchdog event" );
		}

		MutexUnlock ( &thread_mutex, "UpdateDeviceRegistry" );
	}

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

	if ( !MutexLock ( &acceptClientsMutex, "HandleDeviceRegistration" ) )
		return false;
	
	bool found = false;

	// find the one in our vector
	for ( size_t i = 0; i < acceptClients.size(); i++ )
    {
		if ( acceptClients [i].get() == client )
        {
			CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tErasing [%i] from client list", client->ips, i );

            acceptClients.erase ( acceptClients.begin() + i );

			found = true;
			break;
		}
	}

	if ( !MutexUnlock ( &acceptClientsMutex, "HandleDeviceRegistration" ) )
		return false;

	if ( !found ) {
		CErrArgID ( "HandleDeviceRegistration [ %s ]:\tFailed to lookup the client in acceptClients.", client->ips );
		return false;
	}

	char		*	uid		= 0;
	bool			created = false;

	DeviceInstanceNode	* device = UpdateDevices ( ip, msg, &uid, &created );
	if ( !device ) {
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
		
		sp ( ThreadInstance ) busySP = device->clientSP;
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
                    if ( MutexLock ( &thread_mutex, "Watchdog" ) ) {
                        
                        if ( pthread_cond_signal ( &hWatchdogEvent ) ) {
                            CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tWatchdog signal failed.", client->ips );
                        }
                        
                        MutexUnlock ( &thread_mutex, "Watchdog" );
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
			RemoveDevice ( device );

			/// Return try again
			resp.cmd0 = MEDIATOR_SRV_CMD_SESSION_RETRY;
			CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tDevice instance slot lingering. Responded retry advice.", client->ips );
		}
			
		SendBuffer ( client, &resp, resp.size );

		CErrArgID ( "HandleDeviceRegistration [ %s ]:\tA device instance for this registration already exists.", client->ips );
		return false;
	}

	client->deviceSP = device->baseSP;
	device->clientSP = clientSP;

	*client->uid = 0;
	if ( uid && *uid && device->info.deviceID )
    {
		// Transfor lowercase of uid
		string suid ( uid );
		std::transform ( suid.begin(), suid.end(), suid.begin(), ::tolower );
        
        CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tUsing lowercase uid [%s].", client->ips, suid.c_str () );

		strcpy_s ( client->uid, sizeof(client->uid) - 1, suid.c_str () );
        		
		if ( MutexLock ( &usersDBMutex, "HandleDeviceRegistration" ) )
		{			
            sp ( DeviceMapping ) mapping;
            
            Zero ( regMsg );
            
            CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tLooking for mapping of uid [%s].", client->ips, client->uid );
            
			msp ( string, DeviceMapping )::iterator devIt = deviceMappings.find ( string ( client->uid ) );

            if ( devIt != deviceMappings.end ( ) ) {
                CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tFound.", client->ips );
				mapping = devIt->second;
			}
            else {
				mapping = sp ( DeviceMapping ) ( new DeviceMapping ); // calloc ( 1, sizeof(DeviceMapping) );
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

			deviceMappings [ string ( client->uid ) ] = mapping;

        PreFailExit:
			if ( !MutexUnlock ( &usersDBMutex, "HandleDeviceRegistration" ) )
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
            
            SaveDeviceMappings ();
		}		
	}

	client->version = *msg;

	UpdateDeviceRegistry ( device, ip, msg );

	long long sid;

	if ( !MutexLock ( &sessionsMutex, "HandleDeviceRegistration" ) )
		return false;

	/// Assign a session id
	sid = sessionCounter++;
	sid |= ((long long)((device->rootSP->id << 16) | device->rootSP->areaId)) << 32;

    sessions [sid] = clientSP;
    
    CVerbArgID ( "HandleDeviceRegistration [ %s ]:\tAssinged session id [%i]", client->ips, sid );

	if ( !MutexUnlock ( &sessionsMutex, "HandleDeviceRegistration" ) )
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


	MutexLockV ( &client->accessMutex, "SecureChannelAuth" );

	if ( client->socket != -1 )
		sentBytes = (int)sendto ( client->socket, buffer, length, 0, (struct sockaddr *) &client->addr, sizeof(struct sockaddr) );

	MutexUnlockV ( &client->accessMutex, "SecureChannelAuth" );

	if ( (int)length != sentBytes ) {
		CVerbArg ( "SecureChannelAuth: Sending of auth token failed [%u] != [%i].", length, sentBytes );
		return false;
	}
	
	/// Wait for response
	length = (int)recvfrom ( client->socket, buffer, ENVIRONS_MAX_KEYBUFFER_SIZE - 1, 0, (struct sockaddr*) &client->addr, (socklen_t *) &addrLen );
	if ( length <= 0 ) {
		CLogArg ( "SecureChannelAuth: Socket [%i] closed; Bytes [%i]!", client->socket, length ); return false;
	}

	if ( length < 260 ) {
		CWarnArg ( "SecureChannelAuth: Received response is less than the required 260 bytes; Bytes [%i]!", length ); return false;
	}
	recvLength = length;
    buffer [ recvLength ] = 0;
	

	/// Decrypt the response
	char *	msg = 0;
	char *	hash = 0;

	do
	{
		pUI = (unsigned int *) buffer;
		length = *pUI++;
        
		if ( length < 260 ) {
			CLogArg ( "SecureChannelAuth: Received response is not a correct hash; Bytes [%i]!", length ); break;
		}
		length = 256; // Hard limit length to prevent buffer overrun

		unsigned int msgLen = 0;

		if ( !DecryptMessage ( privKey, privKeySize, buffer + 4, length, &msg, &msgLen ) ) {
			CWarn ( "SecureChannelAuth: Failed to decrypt response!" ); break;
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
				CWarn ( "SecureChannelAuth: Authentication required but no username supplied!" ); break;
			}
			userName = msg + 8;

			if ( length & MEDIATOR_MESSAGE_LENGTH_FLAG_PAD ) {
				ClearBit ( length, MEDIATOR_MESSAGE_LENGTH_FLAG_PAD );

				padLen = GetPad ( length, MEDIATOR_MESSAGE_UNIT_ALIGN );
			}

			if ( length >= recvLength || length <= 0 ) {
				CErrArg ( "SecureChannelAuth: Invalid length of username [%i]!", length ); break;
			}

			pUI = (unsigned int *) (userName + length + padLen);

			msgLen = *pUI; /// length of the hash
			//*pUI = 0; 
			userName [ length ] = 0;
            pUI++;
            
            /// Look through db for user
			if ( !MutexLock ( &usersDBMutex, "SecureChannelAuth" ) ) break;
            
            string user = userName;
            
            if (anonymousLogon) {
                if ( !strncmp ( user.c_str(), anonymousUser, MAX_NAMEPROPERTY ) ) {
                    CLog ( "SecureChannelAuth: Anonymous logon." );
                    pass = anonymousPassword;
                    client->authLevel = 0;
                }
            }
            
            if ( !pass ) {
                std::transform ( user.begin(), user.end(), user.begin(), ::tolower );
                
                map<string, UserItem *>::iterator iter = usersDB.find ( user );
                if ( iter == usersDB.end () ) {
					CLogArg ( "SecureChannelAuth: User [%s] not found.", user.c_str () );
                }
                else {
                    UserItem * item = iter->second;
                    
                    pass = item->pass.c_str ();
                    client->createAuthToken = true;
                    client->authLevel = item->authLevel;
                }
                
                if ( !pass ) {
                    CVerbArg ( "SecureChannelAuth: No password for User [%s] available. Treating username as deviceUID and looking for authToken.", user.c_str () );
                    
                    string deviceUID = userName;
					std::transform ( deviceUID.begin(), deviceUID.end(), deviceUID.begin(), ::tolower );

                    msp ( string, DeviceMapping )::iterator devIt = deviceMappings.find ( deviceUID );
                    
                    if ( devIt != deviceMappings.end ( ) ) {
						CVerb ( "SecureChannelAuth: Found auth token for the deviceUID." );
                        pass = devIt->second->authToken;
                        if ( !*pass )
                            pass = 0;
                        client->authLevel = devIt->second->authLevel;
                    }
                }
            }

			if ( !MutexUnlock ( &usersDBMutex, "SecureChannelAuth" ) ) break;

            if ( !pass ) {
                CVerbArg ( "SecureChannelAuth: No username and deviceUID [%s] found.", userName );
                break;
            }
		}

		CVerbVerbArg ( "SecureChannelAuth: Auth user [%s] pass [%s]", userName, ConvertToHexSpaceString ( pass, (int)strlen(pass) ) );

		if ( !BuildEnvironsResponse ( challenge, userName, pass, &hash, &hashLen ) ) {
			CErr ( "SecureChannelAuth: Failed to build response." ); break;
		}

		CVerbVerbArg ( "SecureChannelAuth: Response [%s]", ConvertToHexSpaceString ( hash, hashLen ) );

		if ( hashLen > msgLen ) {
			CWarn ( "SecureChannelAuth: Response/Challenge sizes not equal." ); break;
		}

		/// Accept or not
		if ( memcmp ( pUI, hash, hashLen ) ) {
			CWarn ( "SecureChannelAuth: Response/Challenge invalid." ); break;
		}

		/// If Accept, then derive session key from the computed AES response
		if ( !AESDeriveKeyContext ( hash, hashLen, &client->aes ) ) {
			CErr ( "SecureChannelAuth: Failed to derive AES session keys." ); break;
		}

		client->encrypt = 1;

		msgLen = 4;
		memcpy ( buffer, "ae;;", msgLen );

		sentBytes = SendBuffer ( client, buffer, msgLen );
		
		if ( sentBytes != (int)msgLen ) {
			CWarn ( "SecureChannelAuth: Failed to send AES ACK message." ); break;
		}

		client->authenticated = 1;
		ret = true;
	}
	while ( 0 );
	
	if ( msg )		free ( msg );
	if ( hash )		free ( hash );
	return ret;
}


int MediatorDaemon::SendBuffer ( ThreadInstance * client, void * msg, unsigned int msgLen, bool useLock )
{
	CVerb ( "SendBuffer" );
	
	int rc = -1;
	char * cipher = 0;
	unsigned int toSendLen = msgLen;

	if ( client->socket == -1 || client->encrypt ) {
		if ( !AESEncrypt ( &client->aes, (char *)msg, &toSendLen, &cipher ) || !cipher ) {
			CErr ( "SendBuffer: Failed to encrypt AES message." );
			return rc;
		}
		msg = cipher;
	}

	if ( useLock )
		MutexLockV ( &client->accessMutex, "SendBuffer" );

	if ( client->socket != -1 )
		rc = (int)sendto ( client->socket, (char *)msg, toSendLen, 0, (struct sockaddr *) &client->addr, sizeof(struct sockaddr) );
	
	if ( useLock )
		MutexUnlockV ( &client->accessMutex, "SendBuffer" );

	if ( cipher )
		free ( cipher );
	if ( rc == (int)toSendLen )
		return msgLen;
	return rc;
}


void MediatorDaemon::HandleSpareSocketRegistration ( ThreadInstance * spareClient, sp ( ThreadInstance ) orgClient, char * msg, unsigned int msgLen )
{
	int deviceID = *( (int *) (msg + 12) );
		
	CVerbID ( "HandleSpareSocketRegistration" );
		
	int							sock = -1;

	DeviceInstanceNode *		device;
	
	sp ( DeviceInstanceNode )	deviceSP;


	MutexLockV ( &acceptClientsMutex, "HandleSpareSocketRegistration" );
	
	// find the one in our vector
	size_t size = acceptClients.size ();

	for ( size_t i = 0; i < size; ++i ) 
	{
		if ( acceptClients [ i ].get () == spareClient )
		{
			CVerbArgID ( "HandleSpareSocketRegistration: Erasing [%i] from client list", i );

            acceptClients.erase ( acceptClients.begin() + i );
            break;
		}
	}

	MutexUnlockV ( &acceptClientsMutex, "HandleSpareSocketRegistration" );

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
		CWarnID ( "HandleSpareSocketRegistration: deviceIDs don't match!" );
		goto Finish;
	}	
	
	// Check for matching IP
	if ( !orgClient->daemon || spareClient->addr.sin_addr.s_addr != orgClient->addr.sin_addr.s_addr ) {
		CWarnID ( "HandleSpareSocketRegistration: Requestor has been disposed or IP address of requestor does not match!" );
		goto Finish;
	}	
	
	// Acquire the mutex on orgClient
	MutexLockV ( &orgClient->accessMutex, "HandleSpareSocketRegistration" );

	// Apply the port
	sock = orgClient->spareSocket;
	if ( sock != -1 ) {
        CVerbVerbArg ( "HandleSpareSocketRegistration: Closing spare soket of org client [%i].", sock );
		shutdown ( sock, 2 );
		closesocket ( sock );
    }
    
    CVerbVerbArg ( "HandleSpareSocketRegistration: Take over spare soket of spare client [%i].", spareClient->socket );
	orgClient->spareSocket = spareClient->socket;
	orgClient->sparePort = ntohs ( spareClient->addr.sin_port );
		
    spareClient->socket = -1;
	//pthread_cond_signal ( &orgClient->socketSignal );
	
	// Release the mutex on orgClient
	MutexUnlockV ( &orgClient->accessMutex, "HandleSpareSocketRegistration" );

	sock = 1;
	
Finish:
	pthread_t thrd = spareClient->threadID;
	if ( pthread_valid ( thrd ) ) {
		pthread_detach_handle ( thrd );
	}

	if ( sock != 1 ) {
		sock = spareClient->socket;
        if ( sock != -1 ) {
			spareClient->socket = -1;

            CVerbVerbArg ( "HandleSpareSocketRegistration: Closing spare socket of spare client [%i].", sock );
			shutdown ( sock, 2 );
			closesocket ( sock );
		}
		CLogID ( "HandleSpareSocketRegistration: Failed to register spare socket" );
	}
	else {
		CLogArgID ( "HandleSpareSocketRegistration: Successfully registered spare socket on port [%d].", orgClient->sparePort );
	}
	return;
}


std::string escapeJsonString(const std::string& input) {
    std::ostringstream ss;
    //for (auto iter = input.cbegin(); iter != input.cend(); iter++) {
    //C++98/03:
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
	map<string, ValuePack*>::iterator valueIt;
	string details;
	std::stringstream ss;
			
    ss << clientID << "_pn";
	CVerbArg ( "Client: looking for details key %s!", ss.str().c_str() );

	valueIt = values->find ( ss.str() );

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
		map<string, ValuePack*>::iterator notifierIt = values->find ( string ( "0_gcm" ) );
		
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
		map<string, ValuePack*>::iterator notifierIt = values->find ( string ( "0_aps" ) );
		
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


void MediatorDaemon::NotifyClients ( unsigned int notifya, const char * areaName, const char * appName, int deviceID )
{
	NotifyQueueContext * ctx = new NotifyQueueContext;
	if ( !ctx )
		return;

	ctx->notify = notifya;
	ctx->appName = appName;
	ctx->areaName = areaName;
	ctx->deviceID = deviceID;

	MutexLockV ( &notifyMutex, "NotifyClients" );

	notifyQueue.push ( ctx );
	CVerb ( "NotifyClients: Enqueue" );
    
    /// Trigger Notifier
    if ( pthread_cond_signal ( &notifyEvent ) ) {
        CVerb ( "NotifyClients: Watchdog signal failed." );
    }
    
	MutexUnlockV ( &notifyMutex, "NotifyClients" );
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
		MutexLockV ( &notifyMutex, "NotifyClientsThread" );

    Retry:
        if ( notifyQueue.empty () )
        {
            pthread_cond_wait ( &notifyEvent, &notifyMutex );

            if ( !isRunning ) {
                MutexUnlockV ( &notifyMutex, "NotifyClientsThread" );
                return;
            }
            goto Retry;
        }
        
        ctx = notifyQueue.front ();
        notifyQueue.pop ();
        
        MutexUnlockV ( &notifyMutex, "NotifyClientsThread" );
        
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
	
	if ( !MutexLock ( &devicesMutex, "GetApplicationDevices" ) )
		return 0;

	sp ( AreaApps )						areaApps;
	sp ( ApplicationDevices )			appDevices;
	msp ( string, ApplicationDevices )::iterator appsIt;
	
	string appsName ( appName );
	string pareaName ( areaName );

    msp ( string, AreaApps )::iterator areaIt = areasList.find ( pareaName );	

	if ( areaIt == areasList.end ( ) ) {
		CLogArg ( "GetApplicationDevices: Area [%s] not found.", areaName );
		goto Finish;
	}
	
	areaApps = areaIt->second;
    
	if ( !areaApps ) {
		CLogArg ( "GetApplicationDevices: App [%s] not found.", appName );
		goto Finish;
	}
		
	appsIt = areaApps->apps.find ( appsName );
				
	if ( appsIt == areaApps->apps.end ( ) ) {
		CLogArg ( "GetApplicationDevices: Devicelist of App [%s] not found.", appName );
		goto Finish;
	}
	
	appDevices = appsIt->second;

	if ( !appDevices ) {
		CErr ( "GetApplicationDevices: Invalid appDevices found!" );
		goto Finish;
	}

	if ( appDevices->access <= 0 ) {
		CWarn ( "GetApplicationDevices: appDevices is about to be disposed!" );
		appDevices = 0;
		goto Finish;
	}
	
	__sync_add_and_fetch ( &appDevices->access, 1 );
	
Finish:
	MutexUnlockV ( &devicesMutex, "GetApplicationDevices" );
	return appDevices;
}


void MediatorDaemon::UnlockApplicationDevices ( ApplicationDevices * appDevices )
{
	__sync_sub_and_fetch ( &appDevices->access, 1 );
}


void MediatorDaemon::NotifyClients ( NotifyQueueContext * nctx )
{
	MediatorNotify	msg;
	Zero ( msg );

	msg.cmd0 = MEDIATOR_PROTOCOL_VERSION;

	msg.cmd1 = MEDIATOR_CMD_MEDIATOR_NOTIFY;
	msg.opt0 = MEDIATOR_OPT_NULL;
	msg.opt1 = MEDIATOR_OPT_NULL;
	msg.msgID = nctx->notify;
	msg.notifyDeviceID = nctx->deviceID;

	unsigned int sendSize = sizeof ( MediatorNotify );

	if ( nctx->areaName.length () > 0 && nctx->appName.length () > 0 ) {
		strcpy_s ( msg.areaName, sizeof ( msg.areaName ) - 1, nctx->areaName.c_str () );
		strcpy_s ( msg.appName, sizeof ( msg.appName ) - 1, nctx->appName.c_str () );
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

	
	if ( !MutexLock ( &notifyTargetsMutex, "NotifyClients" ) )
		return;
    
    /// Get the no filter clients
    do
    {
        msp ( long long, ThreadInstance )::iterator clientIt = notifyTargets.begin ();
        
        while ( clientIt != notifyTargets.end () )
        {
			dests.push_back ( clientIt->second );
            clientIt++;
        }
    }
    while ( 0 );
    
    /// Get the AreaApps
    do
    {	
		msp ( string, AreaApps )::iterator areaIt = areasList.find ( nctx->areaName );

		if ( areaIt == areasList.end ( ) ) {
			CLogArg ( "NotifyClients: Area [%s] not found.", nctx->areaName.c_str () );
			break;
		}
        
        sp ( AreaApps ) areaApps = areaIt->second;
        if ( !areaApps )
            break;
        
        msp ( long long, ThreadInstance )::iterator clientIt = areaApps->notifyTargets.begin ();
        
        while ( clientIt != areaApps->notifyTargets.end () )
        {
            dests.push_back ( clientIt->second );
            clientIt++;
        }
    }
    while ( 0 );
    
	appDevices	= GetApplicationDevices ( nctx->areaName.c_str (), nctx->appName.c_str () );
	if ( appDevices )
	{
		if ( MutexLock ( &appDevices->mutex, "NotifyClients" ) )
        {
            DeviceInstanceNode * device = appDevices->devices;
			while ( device )
			{		
				clientSP = device->clientSP;
                
                if ( clientSP && clientSP->filterMode == MEDIATOR_FILTER_AREA_AND_APP )
                {
                    dests.push_back ( clientSP );
                }
				device = device->next;
			}

			CVerbVerb ( "NotifyClients: unlock." );
			MutexUnlockV ( &appDevices->mutex, "NotifyClients" );
		}
		
		UnlockApplicationDevices ( appDevices.get () );		
    }
    
    CVerbVerb ( "NotifyClients: unlock." );
    if ( !MutexUnlock ( &notifyTargetsMutex, "NotifyClients" ) )
        goto Finish;
	
    size = dests.size();
    
	for ( size_t i = 0; i < size; i++ )
    {
        const sp ( ThreadInstance ) &destClient = dests [ i ];
        if ( !destClient )
            continue;
        
        ThreadInstance * dest = destClient.get ();
        //CLogArg ( "NotifyClients: checking device [0x%X]", dest->deviceID );
        
        if ( !dest->deviceID || dest->socket == -1 )
            continue;
        
		if ( !MutexLock ( &dest->accessMutex, "NotifyClients" ) )
			continue;

        if ( IsSocketAlive ( dest->socket ) ) {
			CLogArg ( "NotifyClients: Notify device [0x%X]", dest->deviceID );
        
			sendto ( dest->socket, (char *)&msg, sendSize, 0, (struct sockaddr *) &dest->addr, sizeof(struct sockaddr) );
		}

        MutexUnlock ( &dest->accessMutex, "NotifyClients" );
	}
    
Finish:
    dests.clear ();
    
	CVerbVerb ( "NotifyClients: done." );
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
					
	int						sock;
	sp ( ThreadInstance	)	clientSP;
	ThreadInstance		*	client			= 0;
	const unsigned int		checkDuration	= 1000 * 60 * 2; // 2 min. (in ms)
    const unsigned int      maxTimeout      = checkDuration * 3;

    
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
    unsigned int timeout = checkDuration;
#else
    struct timeval	now;    
    struct timespec timeout;
#endif
    
	while ( isRunning )
	{	
		CVerbVerb ( "Watchdog: checking..." );

		if ( !MutexLock ( &sessionsMutex, "Watchdog" ) ) break;
		
        checkLast = GetEnvironsTickCount ();

		msp ( long long, ThreadInstance )::iterator sessionIt = sessions.begin();
		
		while ( sessionIt != sessions.end () ) 
		{
			clientSP = sessionIt->second;
			if ( clientSP ) {
				client = clientSP.get ();

				CVerbArg ( "Watchdog: Checking deviceID [0x%X : %s]", client->deviceID, client->ips );

				if ( (checkLast - client->aliveLast) > maxTimeout )
				{
					CLogArg ( "Watchdog: Disconnecting [0x%X : %s] due to expired heartbeat...", client->deviceID, client->ips );

					sock = client->spareSocket;
					if ( sock != -1 ) {
						client->spareSocket = -1;

						shutdown ( sock, 2 );
						closesocket ( sock );
					}

					sock = client->socket;
					if ( sock != -1 ) {
						client->socket = -1;

						shutdown ( sock, 2 );
						closesocket ( sock );
					}
				}
				else
					VerifySockets ( client, false );
			}

			sessionIt++;
		}

		if ( !MutexUnlock ( &sessionsMutex, "Watchdog" ) ) break;
		
		//
		// Check acceptClients
		//
		if ( !MutexLock ( &acceptClientsMutex, "Watchdog" ) ) break;

		for ( size_t i = 0; i < acceptClients.size(); )
        {
			clientSP = acceptClients [i];
            if ( clientSP )
            {
				INTEROPTIMEVAL diff = (checkLast - clientSP->connectTime);

				if ( checkLast > clientSP->connectTime && diff > checkDuration )
				{
					CLogArg ( "Watchdog: Invalidating Client [%i : %u ms]", i, diff );

					acceptClients.erase ( acceptClients.begin() + i );

					MutexUnlockV ( &acceptClientsMutex, "Watchdog" );

					ReleaseClient ( clientSP.get () );

					MutexLockV ( &acceptClientsMutex, "Watchdog" );
				}
				else {
					i++; 
				}
				continue;
            }
            else {
                CErrArg ( "Watchdog: **** Invalid Client [%i]", i );
            }
			acceptClients.erase ( acceptClients.begin() + i );
        }

		if ( !MutexUnlock ( &acceptClientsMutex, "Watchdog" ) ) break;


		if ( !MutexLock ( &thread_mutex, "Watchdog" ) ) break;		
		
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
#else
		gettimeofday ( &now, NULL );

		timeout.tv_sec = now.tv_sec + (checkDuration / 1000);
		timeout.tv_nsec = now.tv_usec * 1000;
#endif
		pthread_cond_timedwait ( &hWatchdogEvent, &thread_mutex, &timeout );
		
		if ( usersDBDirty ) {
			usersDBDirty = false;

			if ( !MutexUnlock ( &thread_mutex, "Watchdog" ) ) break;

			SaveUserDB ();

			if ( !MutexLock ( &thread_mutex, "Watchdog" ) ) break;
		}

		if ( configDirty ) {
			configDirty = false;

			if ( !MutexUnlock ( &thread_mutex, "Watchdog" ) ) break;

			SaveConfig ();
		}
		else {
			if ( !MutexUnlock ( &thread_mutex, "Watchdog" ) ) break;
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

	OutputDebugStringA ( timeString );
	OutputDebugStringA ( logBuffer );

	if ( !logging )
		return;

	if ( pthread_mutex_lock ( &logMutex ) ) {
		OutputDebugStringA ( "MLog: Failed to aquire mutex!" );
		return;
	}
	
	stringstream line;

	// Save 
	line << logBuffer;
	
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
	
#ifdef WIN32
	OutputDebugStringA ( timeString );
	OutputDebugStringA ( msg );
#else
	printf ( "%s", timeString );
	printf ( "%s", msg );
#endif

	if ( !logging )
		return;
	stringstream line;

	if ( pthread_mutex_lock ( &logMutex ) ) {
		OutputDebugStringA ( "MLog: Failed to aquire mutex!" );
		return;
	}	

	// Save 
	line << msg;

	logfile << timeString << line.str() << std::flush; // << endl;
	logfile.flush ();

	if ( pthread_mutex_unlock ( &logMutex ) ) {
		OutputDebugStringA ( "MLog: Failed to release mutex!" );
	}	
}


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

	logfile.open ( LOGFILE, ios_base::app );
	if ( !logfile.good() ) {
		CErr ( "OpenLog: Failed to open logfile!" );
		goto Finish;
	}	

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
	logging = false;

	logfile.close();
	
Finish:
	if ( pthread_mutex_unlock ( &logMutex ) ) {
		OutputDebugStringA ( "CloseLog: Failed to release mutex on logfile!" );
		return false;
	}

	return true;
}
