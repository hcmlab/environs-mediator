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
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

#include "Mediator.Daemon.h"
#include "Environs.native.h"
#include "Environs.utils.h"
#include "Environs.crypt.h"
using namespace environs;

#ifndef _WIN32
#if defined(OutputDebugStringA)
#undef OutputDebugStringA
#endif
#define OutputDebugStringA(msg)		fputs(msg,stdout)
#endif

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
using namespace std;

#ifdef WIN32
#include <conio.h>
#include "windows.h"
#include <ws2tcpip.h>
#include <Winhttp.h>

#ifndef NDEBUG
#ifdef _USE_VLD
#ifdef _C_X64
#pragma comment(lib, "C:/Program Files (x86)/Visual Leak Detector/lib/Win64/vld.lib")
#else
#pragma comment(lib, "C:/Program Files/Visual Leak Detector/lib/Win32/vld.lib")
#endif
#endif
#endif

// Link to winsock library
#pragma comment ( lib, "Ws2_32.lib" )

// Link to winsock library
#pragma comment ( lib, "Winhttp.lib" )

#else
#include <string.h>

#include "stdio.h"
#include "errno.h"
#include <cerrno>

#include <termios.h>
#include <curl/curl.h>
#include <unistd.h>

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
MediatorDaemon	*	g_mediator = 0;

bool				logging;
ofstream			logfile;
pthread_mutex_t     logMutex;


MediatorDaemon::MediatorDaemon ()
{
	CVerb ( "Construct" );

	g_mediator				= this;
	allocated				= false;
	
	input					= inputBuffer;

	listeners           .clear ();
	usersDB             .clear ();
	deviceMappings      .clear ();
	acceptClients       .clear ();

	projects            .clear ();
	ports               .clear ();
	bannedIPs           .clear ();
    bannedIPConnects    .clear ();
    bannAfterTries      = 3;
	
	projectsList        .clear ();
	
	projectCounter      = 0;
	projectIDs          .clear ();

	appsCounter         = 0;
	appIDs              .clear ();
    
    notifyTargets       .clear ();

	checkLast			= 0;
    
    Zero ( aesKey );
    Zero ( aesCtx );
    
	privKey				= 0;
	privKeySize			= 0;

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

	notifyDeviceID			= 0;
	notifyProjectName		= 0;
	notifyAppName			= 0;
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

    notifyTargets    .clear ();
	projectIDs      .clear ();
	appIDs          .clear ();

	CloseLog ();

    AESDisposeKeyContext ( &aesCtx );

	ReleaseEnvironsCrypt ();
	
	if ( notifyProjectName ) {
		free ( notifyProjectName );
		notifyProjectName = 0;
	}
	if ( notifyAppName ) {
		free ( notifyAppName );
		notifyAppName = 0;
	}

	if ( allocated ) {
		pthread_mutex_destroy ( &usersDBMutex );
		pthread_mutex_destroy ( &bannedIPsMutex );
		pthread_mutex_destroy ( &acceptClientsMutex );
		pthread_mutex_destroy ( &projectsMutex );
		pthread_mutex_destroy ( &thread_mutex );
		pthread_mutex_destroy ( &logMutex );
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
		if ( pthread_mutex_init		( &thread_mutex, NULL ) ) {
			CErr ( "InitMediator: Failed to init thread_mutex." );
			return false;
		}

		if ( pthread_cond_init		( &thread_condition, NULL ) ) {
			CErr ( "InitMediator: Failed to init thread_condition." );
			return false;
		}

		if ( pthread_cond_init		( &hWatchdogEvent, NULL ) ) {
			CErr ( "InitMediator: Failed to init hWatchdogEvent." );
			return false;
		}

		if ( pthread_mutex_init		( &projectsMutex, NULL ) ) {
			CErr ( "InitMediator: Failed to init projectsMutex." );
			return false;
		}
		
		if ( pthread_mutex_init		( &bannedIPsMutex, NULL ) ) {
			CErr ( "InitMediator: Failed to init bannedIPsMutex." );
			return false;
		}

		if ( pthread_mutex_init		( &usersDBMutex, NULL ) ) {
			CErr ( "InitMediator: Failed to allinitocated usersDBMutex." );
			return false;
		}

		if ( pthread_mutex_init		( &acceptClientsMutex, NULL ) ) {
			CErr ( "InitMediator: Failed to init acceptClientsMutex." );
			return false;
		}

		if ( pthread_mutex_init		( &logMutex, NULL ) ) {
			CErr ( "InitMediator: Failed to init logMutex." );
			return false;
		}

		allocated = true;
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
	ifstream conffile;
    
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
			string prefix;
			unsigned int ip;
			unsigned short port;

			if (!(iss >> prefix >> ip >> port)) { 
				CLogArg ( "LoadConfig: Invalid config line: %s", str );
			}
			else {
				AddMediator ( ip, port );
			}
		}
		else if ( str [ 0 ] == 'B' && str [ 1 ] == ':' ) {
			string prefix;
			unsigned int ip;
			std::time_t dateTime;

			if (!(iss >> prefix >> ip >> dateTime)) { 
				CLogArg ( "LoadConfig: Invalid config line: %s", str );
			}
			else {
				bannedIPs [ ip ] = dateTime;
			}
        }
        else if ( str [ 0 ] == 'C' && str [ 1 ] == 'V' && str [ 2 ] == ':' ) {
            string prefix;
            unsigned int tries;
            
            if (!(iss >> prefix >> tries )) {
                CLogArg ( "LoadConfig: Invalid config line: %s", str );
            }
            else {
                if ( tries < 100 ) {
                    bannAfterTries = tries;
                }
                else {
                    CVerbArg ( "LoadConfig: Invalid try value for bann: %u", tries );
                }
            }
        }
        else if ( str [ 0 ] == 'C' && str [ 1 ] == ':' ) {
            string prefix;
            unsigned int ip;
            unsigned int tries;
            
            if (!(iss >> prefix >> ip >> tries)) {
                CLogArg ( "LoadConfig: Invalid config line: %s", str );
            }
            else {
                bannedIPConnects [ ip ] = tries;
            }
        }
		else if ( str [ 0 ] == 'A' && str [ 1 ] == 'E' && str [ 2 ] == 'S' && str [ 3 ] == ':' ) {
			string prefix;
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
		else if ( str [ 0 ] == 'P' && str [ 1 ] == 'o' && str [ 2 ] == 'r' && str [ 3 ] == 't' && str [ 4 ] == 's' && str [ 5 ] == ' ' ) {
			string prefix;
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
        
        SaveConfig ();
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
	
	stringstream line;

	// Save 
	if ( ports.size() > 0 )
	{	
		line << "Ports ";

		for ( unsigned int pos = 0; pos < ports.size(); pos++ ) {
			int port = ports[pos];
			line << port << " ";
		}
	}
	else
		line << "Ports 5899";

	conffile << line.str() << endl;
	
	
	if ( pthread_mutex_lock ( &mediatorMutex ) ) {
		CErr ( "SaveConfig: Failed to aquire mutex on mediator!" );
	}
	else {
		MediatorInstance * net = &mediator;

		while ( net && net->ip ) {
			line.clear();
			line.str("");
			line << "M: " << net->ip << " " << net->port ;
			
			conffile << line.str() << endl;
			net = net->next;
		}
	
		if ( pthread_mutex_unlock ( &mediatorMutex ) ) {
			CErr ( "SaveConfig: Failed to release mutex on mediator!" );
		}
    }
    
    line.clear();
    line.str("");
    line << "CV: " << bannAfterTries << endl;
    
    conffile << line.str() << endl;
    
	
    char aesCryptKey [ 514 ];
    memcpy ( aesCryptKey, aesKey, sizeof(aesKey) );
    
    unsigned int length = sizeof(aesKey);
    
    if ( !EncryptMessage ( 0, certificate, aesCryptKey, &length ) ) {
        CWarn ( "SaveConfig: Encrypt with certificate failed!" );
    }
    else if ( length >= 256 ) {
        
        const char * hexStr = ConvertToHexString ( aesCryptKey, length );
        if ( hexStr ) {
			line.clear();
			line.str("");
			line << "AES: " << hexStr << " ";
			
			conffile << line.str() << endl;
            
        }
    }
    
	
	if ( pthread_mutex_lock ( &bannedIPsMutex ) ) {
		CErr ( "SaveConfig: Failed to aquire mutex on banned IPs!" );
	}
	else {
		line.clear();
		line.str("");
		std::map<unsigned int, std::time_t>::iterator iter;

		for ( iter = bannedIPs.begin(); iter != bannedIPs.end(); ++iter ) {
			line << "B: " << iter->first << " " << iter->second << endl ;
        }
        conffile << line.str() << endl;
        
        line.clear();
        line.str("");
        std::map<unsigned int, unsigned int>::iterator itert;
        
        for ( itert = bannedIPConnects.begin(); itert != bannedIPConnects.end(); ++itert ) {
            line << "C: " << itert->first << " " << itert->second << endl ;
        }
        conffile << line.str() << endl;
	
		if ( pthread_mutex_unlock ( &bannedIPsMutex ) ) {
			CErr ( "SaveConfig: Failed to release mutex on banned IPs!" );
		}
	}


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
	
	if ( pthread_mutex_lock ( &projectsMutex ) ) {
		CErr ( "LoadProjectValues: Failed to aquire mutex on project/values!" );
	}	

	AppsList					*	apps	= 0;
	map<string, ValuePack*>		*	app		= 0;
	
	string line;
	while ( getline ( conffile, line ) )
	{
		const char * str = line.c_str ();
		if ( strlen ( str ) < 3 )
			continue;

		if ( str [ 0 ] == 'P' && str [ 1 ] == ':' ) {
			CVerbArg ( "Loading project [%s]", str );

			apps = new AppsList ();
			if ( !apps ) {
				CErrArg ( "LoadProjectValues: Failed to create new project [%s].", str + 2 );
				break;
			}
			projects [ string ( str + 2 ) ] = apps;
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
	map<string, AppsList*>::iterator it = projects.begin();
	
	while ( it != projects.end() )
	{		
		if ( !it->second ) {
			projects.erase ( it );
			it = projects.begin();
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
			projects.erase ( it );
			it = projects.begin();
			continue;
		}

		it++;
	}

	if ( pthread_mutex_unlock ( &projectsMutex ) ) {
		CErr ( "LoadProjectValues: Failed to release mutex on project/values!" );
	}

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
	
	if ( pthread_mutex_lock ( &projectsMutex ) ) {
		CErr ( "SaveProjectValues: Failed to aquire mutex on project/values!" );
		ret = false;
	}
	else {
		// Save Projects
		for ( map<string, AppsList * >::iterator it = projects.begin(); it != projects.end(); it++ )
		{
			if ( !it->second )
				continue;

			conffile << "P:" << it->first << endl;

			for ( map<string, map<string, ValuePack*>*>::iterator ita = it->second->apps.begin(); ita != it->second->apps.end(); ita++ ) 
			{
				if ( !ita->second ) 
					continue;

				conffile << "A:" << ita->first << endl;

				for ( map<string, ValuePack*>::iterator itv = ita->second->begin(); itv != ita->second->end(); ++itv )
				{
					if ( itv->second ) {
						conffile << itv->first << " " << itv->second->size << " " << itv->second->value << endl;
					}
				}
			}
		}
			
		if ( pthread_mutex_unlock ( &projectsMutex ) ) {
			CErr ( "SaveProjectValues: Failed to release mutex on project/values!" );
			ret = false;
		}
	}

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
	
	if ( pthread_mutex_lock ( &usersDBMutex ) ) {
		CErr ( "LoadDeviceMappingsEnc: Failed to aquire mutex!" );
        return false;
	}	
	
	string line;
	while ( getline ( conffile, line ) )
	{		
		std::istringstream iss ( line );
		string deviceUID;
		unsigned deviceID;
		string authToken;

		if ( !(iss >> deviceUID >> deviceID) ) {
			//CErrArg ( "LoadDeviceMappings: Failed to read key/value (%s)!", line.c_str() );
			
			if ( !(iss >> deviceUID >> deviceID >> authToken) ) {
				CErrArg ( "LoadDeviceMappingsEnc: Failed to read key/value (%s)!", line.c_str() );
				break;
			}
		}
		
		DeviceMapping * mapping = (DeviceMapping *) calloc ( 1, sizeof(DeviceMapping) );
		if ( !mapping )
			break;

		mapping->deviceID = deviceID;
		if (authToken.length() > 0 )
			memcpy ( mapping->authToken, authToken.c_str(), authToken.length() );

		deviceMappings [ deviceUID ] = mapping;
    }

	if ( pthread_mutex_unlock ( &usersDBMutex ) ) {
		CErr ( "LoadDeviceMappingsEnc: Failed to release mutex!" );
	}

	conffile.close();

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
    
	if ( pthread_mutex_lock ( &usersDBMutex ) ) {
        CErr ( "LoadDeviceMappings: Failed to aquire mutex!" ); return false;
	}
    
    while ( size ) {
        if ( !AESDecrypt ( &aesCtx, ciphers, &size, &decrypted ) || !size || !decrypted )
            break;
        
        istringstream text ( (string ( decrypted )) );
        
        string line;
        while ( getline ( text, line ) )
        {
            std::istringstream iss ( line );
            string deviceUID;
            unsigned deviceID;
            string authToken;
            
            if ( !(iss >> deviceUID >> deviceID >> authToken) ) {
                //CErrArg ( "LoadDeviceMappings: Failed to read key/value (%s)!", line.c_str() );

				if ( !(iss >> deviceUID >> deviceID) ) {
					CErrArg ( "LoadDeviceMappings: Failed to read key/value (%s)!", line.c_str() );
					break;
				}
            }
            
            DeviceMapping * mapping = (DeviceMapping *) calloc ( 1, sizeof(DeviceMapping) );
            if ( !mapping )
                break;
            
            mapping->deviceID = deviceID;
            if (authToken.length() > 0 )
                memcpy ( mapping->authToken, authToken.c_str(), authToken.length() );
            
            deviceMappings [ deviceUID ] = mapping;
        }
        
        break;
    }
    
	if ( pthread_mutex_unlock ( &usersDBMutex ) ) {
		CErr ( "LoadDeviceMappings: Failed to release mutex!" );
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
    
	if ( pthread_mutex_lock ( &usersDBMutex ) ) {
		CErr ( "SaveDeviceMappings: Failed to aquire mutex!" );
		ret = false;
	}
	else {
		// Save Projects
		for ( map<string, DeviceMapping *>::iterator it = deviceMappings.begin(); it != deviceMappings.end(); it++ )
		{
			if ( !it->second )
				continue;

			//conffile << it->first << " " << it->second->deviceID << " " << it->second->authToken << endl;
			plainstream << it->first << " " << it->second->deviceID << " " << it->second->authToken << endl;
		}
        
		if ( pthread_mutex_unlock ( &usersDBMutex ) ) {
			CErr ( "SaveDeviceMappings: Failed to release mutex!" );
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
    }
    
	return ret;
}


bool MediatorDaemon::AddUser ( const char * userName, const char * pass )
{
	CLog ( "AddUser" );
    
	if ( pthread_mutex_lock ( &usersDBMutex ) ) {
		CErr ( "AddUser: Failed to aquire mutex on usersDB!" );
		return false;
	}
	
	bool ret = false;
	
	char * hash = 0;
	unsigned int len = 0;

	do
	{
		ret = SHAHashPassword ( pass, &hash, &len );
		if ( !ret ) {
			CErrArg ( "AddUser: Failed to create pass hash for [%s]", userName ); break;
		}

		if ( !hash ) {
			CErrArg ( "AddUser: Invalid null pass hash for [%s]", userName ); break;
		}
		
		string user = userName;
		std::transform ( user.begin(), user.end(), user.begin(), ::tolower );

		map<string, string>::iterator iter = usersDB.find ( user );
		if ( iter != usersDB.end () ) {
			CLogArg ( "AddUser: Updating password of user [%s]", userName );
			printf ( "\nAddUser: Updating password of user [%s]\n", userName );
		}
		
		//string pw = hash;
		usersDB [ user ] = string ( hash );
		ret = true;
	}
	while ( 0 );

	if ( hash )
		free ( hash );
    
	if ( pthread_mutex_unlock ( &usersDBMutex ) ) {
		CErr ( "AddUser: Failed to release mutex on usersDB!" );
		return false;
	}

	if ( ret )
		ret = SaveUserDB ();

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

	bool hasUserName = false;

	string userName;
	string userPassword;
	string line;
	while ( getline ( usersfile, line ) )
	{
		const char * str = line.c_str ();
		if ( strlen ( str ) < 3 )
			continue;

		std::istringstream iss ( line );
		
		if ( !hasUserName ) {
			if ( line.find ( '@', 0 ) == string::npos )
				continue;
			iss >> userName;
			hasUserName = true;
			continue;
		}

		hasUserName = false;
		iss >> userPassword;
		if ( userPassword.length() < 64 ) {
			continue;
		}

		const char * pwStr = ConvertToBytes ( userPassword.c_str(), (unsigned int)userPassword.length() );
		if ( pwStr ) {
			string pw = pwStr;
			usersDB [ userName ] = pw;
		}
	}
	
	usersfile.close();
	
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
        
        bool hasUserName = false;
        
        string userName;
        string userPassword;
        string line;
        
        while ( getline ( text, line ) )
        {
            const char * str = line.c_str ();
            if ( strlen ( str ) < 3 )
                continue;
            
            std::istringstream iss ( line );
            
            if ( !hasUserName ) {
                if ( line.find ( '@', 0 ) == string::npos )
                    continue;
                iss >> userName;
                hasUserName = true;
                continue;
            }
            
            hasUserName = false;
            iss >> userPassword;
            if ( userPassword.length() < 64 ) {
                continue;
            }
            
            const char * pwStr = ConvertToBytes ( userPassword.c_str(), (unsigned int)userPassword.length() );
            if ( pwStr ) {
                string pw = pwStr;
                usersDB [ userName ] = pw;
            }
        }
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
    
	if ( pthread_mutex_lock ( &usersDBMutex ) ) {
		CErr ( "SaveUserDB: Failed to aquire mutex on usersDB!" );
		ret = false;
	}
	else {
		// Save Projects
		for ( map<string, string>::iterator it = usersDB.begin(); it != usersDB.end(); ++it )
		{
			//CVerbArg ( "SaveUserDB: try saving [%s]", it->first.c_str() );

			if ( it->second.length () )
			{
				const char * pwStr = ConvertToHexString ( it->second.c_str(), ENVIRONS_USER_PASSWORD_LENGTH );
				if ( pwStr ) {
					//CVerbArg ( "SaveUserDB: Saving pass [%s]", pwStr );

					//usersfile << it->first << endl;
					//usersfile << pwStr << endl;

					plainstream << it->first << endl;
					plainstream << pwStr << endl;
				}
			}
		}
        
		if ( pthread_mutex_unlock ( &usersDBMutex ) ) {
			CErr ( "SaveUserDB: Failed to release mutex on usersDB!" );
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

	if ( (ip & 0x00FFFFFF) == 0xABFA89 ) {
		CVerb ( "IsIpBanned: IP is one of the allowed networks!" );
		return false;
	}

	if ( pthread_mutex_lock ( &bannedIPsMutex ) ) {
		CErr ( "IsIpBanned: Failed to acquire mutex!" );
	}

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
	if ( pthread_mutex_unlock ( &bannedIPsMutex ) ) {
		CErr ( "IsIpBanned: Failed to release mutex!" );
	}

	return banned;
}


void MediatorDaemon::BannIP ( unsigned int ip )
{
	CVerb ( "BannIP" );

	if ( (ip & 0x00FFFFFF) == 0xABFA89 ) {
		CVerb ( "BannIP: IP is one of the allowed networks!" );
		return;
	}

	if ( pthread_mutex_lock ( &bannedIPsMutex ) ) {
		CErr ( "BannIP: Failed to aquire mutex!" );
		return;
	}
    
    unsigned int tries = bannedIPConnects [ ip ];
    if ( tries < bannAfterTries ) {
        bannedIPConnects [ ip ] = ++tries;
        CVerbVerbArg ( "BannIP: retain bann due to allowed tries [%u/%u]", tries, bannAfterTries );
    }
    else {
        std::time_t t = std::time(0);
        
        bannedIPs [ ip ] = t;
    }

	if ( pthread_mutex_unlock ( &bannedIPsMutex ) ) {
		CErr ( "BannIP: Failed to release mutex!" );
	}

	SaveConfig ();
}


void MediatorDaemon::BannIPRemove ( unsigned int ip )
{
    CVerb ( "BannIPRemove" );
    
	bool ret = false;

    if ( pthread_mutex_lock ( &bannedIPsMutex ) ) {
        CErr ( "BannIPRemove: Failed to aquire mutex!" );
        return;
    }
    
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
    
    if ( pthread_mutex_unlock ( &bannedIPsMutex ) ) {
        CErr ( "BannIPRemove: Failed to release mutex!" );
    }
    
	if ( ret )
		SaveConfig ();
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
		MediatorThreadInstance * listener = new MediatorThreadInstance ();
		if ( !listener ) {
			CErrArg ( "CreateThreads: Error - Failed to allocate memory for new Listener (Nr. %i). Probably low memory.", pos );
			return false;
		}
		memset ( listener, 0, sizeof(MediatorThreadInstance) );
		listener->socketUdp = -1;

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
		pthread_mutex_lock ( &thread_mutex );

		s = pthread_create ( &inst->threadID, 0, &MediatorDaemon::AcceptorStarter, (void *)listener );
		if ( s != 0 ) {
			CErrArg ( "CreateThreads: Error creating acceptor thread for port %i (pthread_create:%i)", inst->port, s );
			pthread_mutex_unlock ( &thread_mutex );
			return false;
		}

		pthread_cond_wait ( &thread_condition, &thread_mutex );
		pthread_mutex_unlock ( &thread_mutex );

		pthread_mutex_lock ( &thread_mutex );

		s = pthread_create ( &listener->threadIDUdp, 0, &MediatorDaemon::MediatorUdpThreadStarter, (void *)listener );
		if ( s != 0 ) {
			CErrArg ( "CreateThreads: Error creating udp thread for port %i (pthread_create:%i)", inst->port, s );
			pthread_mutex_unlock ( &thread_mutex );
			return false;
		}

		pthread_cond_wait ( &thread_condition, &thread_mutex );
		pthread_mutex_unlock ( &thread_mutex );

		s = pthread_create ( &listener->threadIDWatchdog, 0, &MediatorDaemon::WatchdogThreadStarter, (void *)this );
		if ( s != 0 ) {
			CErrArg ( "CreateThreads: Error creating clients alive status checker port %i (pthread_create:%i)", inst->port, s );
		}
		
		listeners.push_back ( listener );
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

	if ( pthread_mutex_lock ( &projectsMutex ) ) {
		CErr ( "Dispose: Failed to aquire mutex on project/values!" );
	}
		
	for ( map<string, AppsList * >::iterator it = projects.begin(); it != projects.end(); it++ )
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
		delete it->second;
	}
	projects.clear ();

	if ( pthread_mutex_unlock ( &projectsMutex ) ) {
		CErr ( "Dispose: Failed to release mutex on project/values!" );
	}

	ReleaseDeviceMappings ();
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

	if ( !isRunning ) 
	{		
		if ( pthread_valid ( thrd ) ) 
		{
			int s = pthread_join ( thrd, &res );
			if ( s != 0 ) {
				CErrArg ( "ReleaseClient: Error waiting for thread (pthread_join:%i)", s );
			}
			pthread_close ( thrd );
		}

		if ( client->aes.encCtx )
			AESDisposeKeyContext ( &client->aes );
		 
		if ( pthread_mutex_destroy ( &client->accessMutex ) ) {
			CVerb ( "ReleaseClient: Failed to destroy mutex." );
		}

		delete client;
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

	for ( unsigned int pos = 0; pos < listeners.size(); pos++ ) {
		MediatorThreadInstance * listener = listeners [ pos ];
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

		delete listener;
	}
	listeners.clear();
		
		
	if ( pthread_mutex_lock ( &acceptClientsMutex ) ) {
		CErr ( "ReleaseThreads: Failed to aquire mutex on clients!" );
	}

	for ( unsigned int pos = 0; pos < acceptClients.size(); pos++ ) {
		ReleaseClient ( acceptClients [ pos ] );
	}
	acceptClients.clear();

	if ( pthread_mutex_unlock ( &acceptClientsMutex ) ) {
		CErr ( "ReleaseThreads: Failed to release mutex on clients!" );
	}

	return ret;
}


void MediatorDaemon::ReleaseDevices ()
{
	CLog ( "ReleaseDevices" );

	if ( pthread_mutex_lock ( &devicesMutex ) ) {
		CErr ( "ReleaseDevices: Failed to aquire mutex on project/values!" );
	}

	for ( map<string, ProjectApps*>::iterator it = projectsList.begin(); it != projectsList.end(); ++it )
	{
		if ( it->second ) {
			ProjectApps * projApps = it->second;

			for ( map<string, ApplicationDevices *>::iterator ita = projApps->apps.begin(); ita != projApps->apps.end(); ++ita )
			{
				ApplicationDevices * appDevices = ita->second;
				if ( appDevices ) 
				{					
					if ( pthread_mutex_lock ( &appDevices->mutex ) ) {
						CErr ( "ReleaseDevices: Failed to aquire mutex on application device list!" );
					}
					DeviceInstanceList * device = appDevices->devices;
			
					while ( device ) {
						DeviceInstanceList * toDelete = device;
						device = device->next;
						CVerbArg ( "[0x%X].ReleaseDevices: deleting memory occupied by client", toDelete->info.id );

						ReleaseClient ( toDelete->client );

						free ( toDelete );
					}	

					if ( pthread_mutex_unlock ( &appDevices->mutex ) ) {
						CErr ( "ReleaseDevices: Failed to aquire mutex on application device list!" );
					}

					pthread_mutex_destroy ( &appDevices->mutex );
					free ( appDevices );
				}
			}

			delete projApps;
		}
	}
	projectsList.clear ();

	if ( pthread_mutex_unlock ( &devicesMutex ) ) {
		CErr ( "ReleaseDevices: Failed to release mutex on project/values!" );
	}
}


void MediatorDaemon::ReleaseDeviceMappings ()
{
    CVerb ( "ReleaseDeviceMappings" );

	if ( pthread_mutex_lock ( &usersDBMutex ) ) {
		CErr ( "ReleaseDeviceMappings: Failed to aquire mutex (devMap)!" );
	}
	
	for ( map<string, DeviceMapping *>::iterator it = deviceMappings.begin(); it != deviceMappings.end(); ++it ) 
	{
		if ( !it->second ) {
			continue;
		}
		free ( it->second );
	}
	deviceMappings.clear ();
    
	if ( pthread_mutex_unlock ( &usersDBMutex ) ) {
		CErr ( "ReleaseDeviceMappings: Failed to release mutex (devMap)!" );
	}
}


void MediatorDaemon::RemoveDevice ( int deviceID, const char * projectName, const char * appName )
{
	CVerbID ( "RemoveDevice from maps" );

	if ( !projectName || !appName ) {
		CErrID ( "RemoveDevice: Called with NULL argument for projectName or appName?!" );
		return;
	}
	
	if ( pthread_mutex_lock ( &devicesMutex ) ) {
		CErrID ( "RemoveDevice: Failed to acquire mutex on project list!" );
		return;
	}

	bool				found	= false;
	bool				unlock	= true;
	DeviceInstanceList	*	device	= 0;
	DeviceInstanceList	*	prevDevice = 0;
	
	string appsName ( appName );
	ApplicationDevices					* appDevices = 0;

	string projName ( projectName );
	ProjectApps							* projApps	= 0;

	map<string, ApplicationDevices * >::iterator appsIt;

    map<string, ProjectApps * >::iterator projIt = projectsList.find ( projName );
	if ( projIt != projectsList.end ( ) )
		projApps = projIt->second;
    
	if ( !projApps ) {
		CWarnArgID ( "RemoveDevice: projectName [%s] not found.", projectName );
		goto Finish;
	}		

	appsIt = projApps->apps.find ( appsName );				
	if ( appsIt != projApps->apps.end ( ) )
		appDevices = appsIt->second;

	if ( !appDevices ) {
		CErrArg ( "RemoveDevice: appName [%s] not found.", appName );
		goto Finish;
	}
	
	if ( pthread_mutex_lock ( &appDevices->mutex ) ) {
		CErr ( "RemoveDevice: Failed to acquire mutex on app devices!" );
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
		prevDevice = device;
		device = device->next;
	}

	if ( found ) {
		if ( device == appDevices->devices ) {
			if ( device->next ) {		
				CVerbArg ( "[0x%X].RemoveDevice: relocating client to root of list", device->next->info.deviceID );
			
				DeviceInstanceList * toDelete = device;

				appDevices->devices = device->next;
				appDevices->count--;
			
				CVerbArg ( "[0x%X].RemoveDevice: deleting temporary prior occupied memory for client", toDelete->info.deviceID );
				free ( toDelete );
			}
			else {
				free ( device );

#ifdef ENABLE_LIST_CLEANER
				appDevices->devices = 0;
				appDevices->count = 0;
#else
				/*if ( appDevices->access ) 
				{
					/// We need to wait until no one has a lock anymore...
				}*/
				pthread_mutex_unlock ( &appDevices->mutex );
				unlock = false;

				projApps->erase ( appsIt );

				pthread_mutex_destroy ( &appDevices->mutex );
				free ( appDevices );

				if ( projApps->size() == 0 ) {
					projectsList.erase ( projIt );
					delete projApps;
				}
#endif
			}
		}
		else {
			if ( !prevDevice ) {
				CVerbArgID ( "RemoveDevice: Failed to lookup device list. Missing previous device for ID [0x%X]", device->info.deviceID );
			}
			else {
				if ( device->next ) {
					CVerbArg ( "[0x%X].RemoveDevice: relinking client to previous client [0x%X]", device->next->info.id, prevDevice->info.deviceID );
					prevDevice->next = device->next;
				}
				else {
					CVerbArg ( "[0x%X].RemoveDevice: finish list since client was the last one", prevDevice->info.deviceID );
					prevDevice->next = 0;
				}
				
				appDevices->count--;

				CVerbArg ( "[0x%X].RemoveDevice: deleting memory occupied by client", device->info.deviceID );
				free ( device );
			}
		}
	}
	
	if ( unlock && pthread_mutex_unlock ( &appDevices->mutex ) ) {
		CErr ( "RemoveDevice: Failed to release mutex on app devices!" );
	}
	
	NotifyClientsStart ( NOTIFY_MEDIATOR_MED_DEVICE_REMOVED, projName.c_str(), appsName.c_str(), deviceID );

Finish:
	if ( pthread_mutex_unlock ( &devicesMutex ) ) {
		CErr ( "RemoveDevice: Failed to release mutex on project list!" );
	}
}


void MediatorDaemon::RemoveDevice ( DeviceInstanceList * device, bool useLock )
{
	CVerbVerb ( "RemoveDevice" );

	if ( !device )
		return;
	
	RemoveDevice ( device->info.deviceID, device->info.projectName, device->info.appName );
}


void MediatorDaemon::RemoveDevice ( unsigned int ip, char * msg )
{
	CVerb ( "RemoveDevice from broadcast" );

	if ( !isRunning || !msg )
		return;

	unsigned int deviceID;
	char * projectName = 0;
	char * appName = 0;

	// Get the id at first (>0)	
	unsigned int * pUIntBuffer = (unsigned int *) (msg + 12);
	deviceID = *pUIntBuffer;
	if ( !deviceID )
		return;
	
	// Get the projectname, appname, etc..
	char * context = NULL;
	char * psem = strtok_s ( msg + MEDIATOR_BROADCAST_DESC_START, ";", &context );
	if ( !psem )
		return;
	
	psem = strtok_s ( NULL, ";", &context );
	if ( !psem )
		return;
	projectName = psem;
	
	psem = strtok_s ( NULL, ";", &context );
	if ( !psem )
		return;
	appName = psem;
	
	CVerbID ( "RemoveDevice BC:" );

	RemoveDevice ( deviceID, projectName, appName );
}


/// 0 no filter
/// 1 show only within same project
/// 2 show only within same project and apps

void MediatorDaemon::UpdateNotifyTargets ( ThreadInstance * client, int filterMode )
{
    map<long long, ThreadInstance *>::iterator notifyIt;
    
    if ( pthread_mutex_lock ( &devicesMutex ) ) {
		CErr ( "UpdateNotifyTargets: Failed to acquire mutex!" );
		return;
	}
	
    /// Check whether we need to remove the client from a targetlist
    if ( client->filterMode == MEDIATOR_FILTER_NONE )
    {
        /// Remove from project notifiers
		CVerbArg ( "UpdateNotifyTargets: Looking for deviceID [0x%X / %s / %s] in NONE-filter", client->deviceID, client->device ? client->device->info.projectName : "", client->device ? client->device->info.appName : "" );

        notifyIt = notifyTargets.find ( client->sessionID );
        if ( notifyIt != notifyTargets.end () ) {
			CVerbArg ( "UpdateNotifyTargets: Removing deviceID [0x%X / %s / %s] from NONE-filter", client->deviceID, client->device ? client->device->info.projectName : "", client->device ? client->device->info.appName : ""  );
            notifyTargets.erase ( notifyIt );
        }
    }
    else if ( client->filterMode == MEDIATOR_FILTER_PROJECT ) {
        while ( client->device && client->device->root )
        {
            /// Search for project
            map<unsigned int, string>::iterator projIDsIt = projectIDs.find ( client->device->root->projId );
            
            if ( projIDsIt == projectIDs.end () ) {
                CWarnArg ( "UpdateNotifyTargets: Failed to find project ID [%u]", client->device->root->projId );
                break;
            }
            
            map<string, ProjectApps *>::iterator projIt = projectsList.find ( projIDsIt->second );
            
            if ( projIt == projectsList.end ( ) ){
                CWarnArg ( "UpdateNotifyTargets: Failed to find project [%s]", projIDsIt->second.c_str () );
                break;
            }
            
            ProjectApps * projApps = projIt->second;
            
			CVerbArg ( "UpdateNotifyTargets: Looking for deviceID [0x%X / %s / %s] in Project-filter", client->deviceID, client->device ? client->device->info.projectName : "", client->device ? client->device->info.appName : ""  );

            notifyIt = projApps->notifyTargets.find ( client->sessionID );
            if ( notifyIt != projApps->notifyTargets.end () ) {
				CVerbArg ( "UpdateNotifyTargets: Removing deviceID [0x%X / %s / %s] from Project-filter", client->deviceID, client->device ? client->device->info.projectName : "", client->device ? client->device->info.appName : ""  );
                projApps->notifyTargets.erase ( notifyIt );
            }
            
            break;
        }
    }
    
    if ( filterMode < MEDIATOR_FILTER_NONE )
        goto Finish;
    
    client->filterMode = (unsigned short)filterMode;
    
	if ( filterMode == MEDIATOR_FILTER_ALL ) {
        goto Finish;
	}
    
    if ( filterMode == MEDIATOR_FILTER_NONE )
    {
		CVerbArg ( "UpdateNotifyTargets: Adding deviceID [0x%X / %s / %s] to NONE-filter", client->deviceID, client->device ? client->device->info.projectName : "", client->device ? client->device->info.appName : ""  );
        notifyTargets [ client->sessionID ] = client;
    }
    else if ( filterMode == MEDIATOR_FILTER_PROJECT ) {
        while ( client->device && client->device->root )
        {
            /// Search for project
            map<unsigned int, string>::iterator projIDsIt = projectIDs.find ( client->device->root->projId );
            
            if ( projIDsIt == projectIDs.end () ) {
                CWarnArg ( "UpdateNotifyTargets: Failed to find project ID [%u]", client->device->root->projId );
                break;
            }
            
            map<string, ProjectApps *>::iterator projIt = projectsList.find ( projIDsIt->second );
            
            if ( projIt == projectsList.end ( ) ){
                CWarnArg ( "UpdateNotifyTargets: Failed to find project [%s]", projIDsIt->second.c_str () );
                break;
            }
            
			CVerbArg ( "UpdateNotifyTargets: Adding deviceID [0x%X / %s / %s] to Project-filter", client->deviceID, client->device ? client->device->info.projectName : "", client->device ? client->device->info.appName : ""  );
            ProjectApps * projApps = projIt->second;
            projApps->notifyTargets [ client->sessionID ] = client;
            
            break;
        }
    }
    
Finish:
    if ( pthread_mutex_unlock ( &devicesMutex ) ) {
		CErr ( "UpdateNotifyTargets: Failed to release mutex!" );
		return;
	}
}


DeviceInstanceList ** MediatorDaemon::GetDeviceList ( char * projectName, char * appName, pthread_mutex_t ** mutex, int ** pDevicesAvailable, ApplicationDevices ** pappDevices )
{
	DeviceInstanceList ** list = 0;

	if ( pthread_mutex_lock ( &devicesMutex ) ) {
		CErr ( "GetDeviceList: Failed to acquire mutex!" );
		return 0;
	}	

	ApplicationDevices	*				appDevices	= 0;
	ProjectApps			*				projApps	= 0;
	map<string, ApplicationDevices * >::iterator appsIt;
	
	string appsName ( appName );
	string projName ( projectName );

    map<string, ProjectApps *>::iterator projIt = projectsList.find ( projName );	

	if ( projIt == projectsList.end ( ) ) {
		/// Create a new one...
		projApps = new ProjectApps ();
		if ( projApps ) {
			projectCounter++;
			projectIDs [ projectCounter ] = projName;
			
			projApps->id = projectCounter;
			projApps->apps.clear ();
            projApps->notifyTargets.clear ();

			projectsList [ projName ] = projApps;
		}
		else { CErrArg ( "GetDeviceList: Failed to create new project [%s] Low memory problem?!", projectName ); }
	}
	else
		projApps = projIt->second;
    
	if ( !projApps ) {
		CLogArg ( "GetDeviceList: App [%s] not found.", appName );
		goto Finish;
	}
		
	appsIt = projApps->apps.find ( appsName );
				
	if ( appsIt == projApps->apps.end ( ) ) {
		/// Create a new one...
		appDevices = (ApplicationDevices *) calloc ( 1, sizeof(ApplicationDevices) );
		if ( appDevices ) {
			pthread_mutex_init ( &appDevices->mutex, 0 );

			appDevices->access = 1;			
			projApps->apps [ appsName ] = appDevices;
			
			appsCounter++;
			appDevices->id = appsCounter;
			appDevices->projId = projApps->id;
			appIDs [ appsCounter ] = appsName;
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
		*pappDevices = appDevices;

	list = &appDevices->devices;

Finish:
	if ( pthread_mutex_unlock ( &devicesMutex ) ) {
		CErr ( "GetDeviceList: Failed to release mutex!" );
		return 0;
	}

	return list;
}


void printDeviceList ( DeviceInstanceList * device )
{
	while ( device ) 
	{
		CLogArg ( "Device      = [0x%X / %s / %s]", device->info.deviceID, device->info.deviceName, device->info.broadcastFound ? "on same network" : "by mediator" );
		if ( device->info.ip != device->info.ipe ) {
			CLogArg ( "Device IPe != IP [%s]", inet_ntoa ( *((struct in_addr *) &device->info.ip) ) );
		}
		CLogArg ( "Project/App = [%s / %s]", device->info.projectName, device->info.appName );
		CLogArg ( "Device  IPe = [%s (from socket), tcp [%d], udp [%d]]", inet_ntoa ( *((struct in_addr *) &device->info.ipe) ), device->info.tcpPort, device->info.udpPort );

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
				if ( pthread_mutex_lock ( &bannedIPsMutex ) ) {
					CErr ( "Failed to aquire mutex on banned IPs!" );
				}
				else {
					printf ( "Run: Clearing bannlist with [%u] entries.\n", (unsigned int)bannedIPs.size() );
					CLogArg ( "Run: Clearing bannlist with [%u] entries.\n", (unsigned int)bannedIPs.size() );
                    bannedIPs.clear ();
                    bannedIPConnects.clear ();
					if ( pthread_mutex_unlock ( &bannedIPsMutex ) ) {
						CErr ( "Failed to release mutex on banned IPs!" );
					}
				}
				continue;
			}
			else if ( c == 'i' ) {	
				if ( pthread_mutex_lock ( &localNetsMutex ) ) {
					CErr ( "Run: Failed to aquire mutex on localNets!" );
					continue;
				}
				
				NetPack * net = &localNets;
				while ( net ) {
					CLogArg ( "Interface ip:%s", inet_ntoa ( *((struct in_addr *) &net->ip) ) );
					CLogArg ( "Interface bcast:%s", inet_ntoa ( *((struct in_addr *) &net->bcast) ) );
					net = net->next;
				}
	
				if ( pthread_mutex_unlock ( &localNetsMutex ) ) {
					CErr ( "Run: Failed to release mutex on localNets!" );
				}

				continue;
			}
			else if ( c == 'm' ) {
				if ( !mediator.ip ) {
					CLog ( "No more mediators known yet!" );
					continue;
				}
				else {
					if ( pthread_mutex_lock ( &mediatorMutex ) ) {
						CErr ( "Run: Failed to aquire mutex on mediator!" );
						continue;
					}
					
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
	
					if ( pthread_mutex_unlock ( &mediatorMutex ) ) {
						CErr ( "Run: Failed to release mutex on mediator!" );
					}
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
				if ( pthread_mutex_lock ( &projectsMutex ) ) {
					CErr ( "Failed to aquire mutex on project/values!" );
				}
				else {		
					for ( map<string, AppsList * >::iterator it = projects.begin(); it != projects.end(); it++ )
					{
						if ( !it->second )
							continue;
						CLogArg ( "Project: %s", it->first.c_str() );

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
			
					if ( pthread_mutex_unlock ( &projectsMutex ) ) {
						CErr ( "Failed to release mutex on project/values!" );
					}
				}
                printf ( "----------------------------------------------------------------\n" );
                if ( pthread_mutex_lock ( &usersDBMutex ) ) {
                    CErr ( "Failed to aquire mutex on usersDB!" );
                }
                else {
                    for ( map<string, string>::iterator it = usersDB.begin(); it != usersDB.end(); ++it )
                    {
                        if ( it->second.length () )
                        {
                            const char * pwStr = ConvertToHexString ( it->second.c_str(), ENVIRONS_USER_PASSWORD_LENGTH );
                            if ( pwStr ) {
                                CLogArg ( "User: %s\t[%s]", it->first.c_str(), pwStr );
                                
                            }
                        }
                    }
                    
                    if ( pthread_mutex_unlock ( &usersDBMutex ) ) {
                        CErr ( "Failed to release mutex on usersDB!" );
                    }
                }
                printf ( "----------------------------------------------------------------\n" );
				continue;
			}
			else if ( c == 'p' ) {
				CLog ( "Active devices:" );
				CLog ( "----------------------------------------------------------------" );
				

				if ( pthread_mutex_lock ( &devicesMutex ) ) {
					CErr ( "Failed to aquire mutex on devices!" );
				}
				else {
					for ( map<string, ProjectApps*>::iterator it = projectsList.begin(); it != projectsList.end(); ++it )
					{
						if ( it->second ) {
							ProjectApps * projApps = it->second;
							CLogArg ( "P: [%s]", it->first.c_str () );

							for ( map<string, ApplicationDevices *>::iterator ita = projApps->apps.begin(); ita != projApps->apps.end(); ++ita )
							{
								ApplicationDevices * appDevices = ita->second;
								if ( appDevices ) 
								{					
									CLog ( "----------------------------------------------------------------" );
									CLogArg ( "A: [%s]", ita->first.c_str () );

									if ( pthread_mutex_lock ( &appDevices->mutex ) ) {
										CErr ( "Failed to aquire mutex on application device list!" );
									}
									else {
										printDeviceList ( appDevices->devices );

										if ( pthread_mutex_unlock ( &appDevices->mutex ) ) {
											CErr ( "Failed to rlease mutex on application device list!" );
										}
									}
									CLog ( "----------------------------------------------------------------" );
								}
							}
						}
					}

					if ( pthread_mutex_unlock ( &devicesMutex ) ) {
						CErr ( "Failed to release mutex on project/values!" );
					}
				}
				CLog ( "----------------------------------------------------------------" );
				continue;
			}
			else if ( c == 'a' ) {
				CLog ( "Add new user:" );
				printf ( "Add new user:\n" );
				printf ( "----------------------------------------------------------------\n" );
				CLog ( "Please enter the username (email):" );
				printf ( "Please enter the username (email): " );
				command = 'a';
				userName = "a";
				continue;
			}
			
			//else if ( c == 't' ) {
			//	char keys [ 256 ];
			//	char values [ 256 ];
			//	sprintf_s ( keys, 256, "key_%i", ++keyCount );
			//	sprintf_s ( values, 256, "value_%i", ++valueCount );
			//	SetParam ( "test", keys, values );
			//	continue;
			//}	
			//else if ( c == 'u' ) {
			//	char keys [ 256 ];
			//	char values [ 256 ];
			//	sprintf_s ( keys, 256, "key_%i", keyCount );
			//	sprintf_s ( values, 256, "value_%i", valueCount );
			//	SetParam ( "test", keys, values );
			//	continue;
			//}	
			//else if ( c == 'v' ) {
			//	char keys [ 256 ];
			//	sprintf_s ( keys, 256, "key_%i", keyCount );
			//	GetParam ( "test", keys );
			//	continue;
			//}			
		}
		if (c == 13 || c == 10 ) { // Return key
			// Close input string
			*input = 0;

			// Analyse input string
			if ( command == 'a' ) {
				if ( userName.c_str()[0] == 'a' && userName.length() == 1 ) {
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
					if ( AddUser ( userName.c_str(), inputBuffer ) ) {
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
			if (input == inputBuffer + BUFFERSIZE - 1) {
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
	if ( !arg  ) {
		CErr ( "MediatorUdpThreadStarter: Called with (NULL) argument." );
		return 0;
	}
	
	// Execute thread
	return g_mediator->MediatorUdpThread ( arg );
}


void * MediatorDaemon::MediatorUdpThread ( void * arg )
{
	CVerb ( "MediatorUdpThread started." );

	if ( !arg  ) {
		CErr ( "MediatorUdpThread: called with invalid (NULL) argument." );
		return 0;
	}

	char buffer [ 64000 ];
	
	char				*	decrypted		= 0;
    char				*	msgDec			= 0;
	unsigned int			msgDecLength;
	unsigned int			msgLength;
	ThreadInstance		*	client;

	struct 	sockaddr_in		listenAddr;
	MediatorThreadInstance * listener = (MediatorThreadInstance *) arg;

	unsigned short port = listener->instance.port;
	unsigned int sock = listener->socketUdp;
	
	CVerbArg ( "MediatorUdpThread for port [%d] started.", port );

	// Send started signal
	pthread_mutex_lock ( &thread_mutex );

	if ( pthread_cond_signal ( &thread_condition ) ) {
		CErr ( "MediatorUdpThread: Error to signal event" );
	}

	pthread_mutex_unlock ( &thread_mutex );

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
			ThreadInstance * client = 0;			
			
			if ( bytesReceived >= (int)sizeof(STUNReqPacket) ) 
			{
				msgDec [ msgDecLength ] = 0;
				const char * projName;
				const char * appName;

				ApplicationDevices * appDevices	= GetApplicationDevices ( stunPacket->projectName, stunPacket->appName );
			
				if ( !appDevices )
					continue;
					
				if ( pthread_mutex_lock ( &appDevices->mutex ) ) {
					CVerb ( "Udp: Failed to aquire mutex!" );
					UnlockApplicationDevices ( appDevices );
					continue;
				}

				DeviceInstanceList * device = GetDeviceInstance ( destID, appDevices->devices );
				if ( !device ) {
					CErrArg ( "Udp: requested STUN device [%d] does not exist", destID );
					goto Continue;
				}

				client = device->client;
				if ( !client ) {
					CErrArg ( "Udp: requested client of STUN device [%d] does not exist", destID );
					goto Continue;
				}

				// Reply with mediator ack message to establish the temporary route entry in requestor's NAT
				length	= (int) sendto ( sock, "y;;-    ", 8, 0, (const sockaddr *) &addr, sizeof(struct sockaddr_in) );
				if ( length != 8 ) {
					CErrArg ( "Udp: Failed to send UDP ACK to device [%d]", sourceID );
				}

				projName = stunPacket->projectName;
				if ( *stunPacket->projectNameSrc )
					projName = stunPacket->projectNameSrc;

				appName = stunPacket->appName;
				if ( *stunPacket->appNameSrc )
					appName = stunPacket->appNameSrc;

				HandleSTUNRequest ( client, sourceID, projName, appName, (unsigned int) addr.sin_addr.s_addr, ntohs ( addr.sin_port ) );
					
Continue:
				UnlockApplicationDevices ( appDevices );

				if ( pthread_mutex_unlock ( &appDevices->mutex ) ) {
					CVerb ( "Udp: Failed to release mutex!" );
				}
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
	if ( !arg  ) {
		CErr ( "AcceptorStarter: Called with (NULL) argument." );
		return 0;
	}
	
	// Execute thread
	return g_mediator->Acceptor ( arg );
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
	pthread_mutex_lock ( &thread_mutex );

	if ( pthread_cond_signal ( &thread_condition ) ) {
		CErr ( "Acceptor: Error to signal event" );
	}

	pthread_mutex_unlock ( &thread_mutex );

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

		CLogArg ( "Acceptor: New socket [%i] connection with IP [%s], Port [%d]", sock, inet_ntoa ( addr.sin_addr ), ntohs ( addr.sin_port ) );
		
		ThreadInstance * client = new ThreadInstance();
		if ( !client ) {
			CErr ( "Acceptor: Failed to allocate memory for client request! Low memory issue?!" );
			continue;
		}
		
		memset ( client, 0, sizeof(ThreadInstance) );

		pthread_mutex_init ( &client->accessMutex, 0 );
		//pthread_cond_init ( &client->socketSignal, NULL );
		
		client->socket		= sock;
		client->spareSocket	= -1;
		client->port		= port;
		client->filterMode	= 2;
		client->aliveLast	= checkLast;

		memcpy ( &client->addr, &addr, sizeof(addr) );
				
		if ( pthread_mutex_lock ( &acceptClientsMutex ) ) {
			CErr ( "Acceptor: Failed to aquire mutex on clients!" );
			goto NextClient;
		}

		// Create client thread	
		s = pthread_create ( &client->threadID, 0, &MediatorDaemon::ClientThreadStarter, (void *)client );
		if ( s != 0 ) {
			CWarnArg ( "Acceptor: pthread_create failed [%i]", s );
			goto NextClient;
		}

		acceptClients.push_back ( client );
		client = 0;

NextClient:
		if ( client ) {
			delete client;
		}
		
		if ( pthread_mutex_unlock ( &acceptClientsMutex ) ) {
			CErr ( "Acceptor: Failed to release mutex on clients!" );
		}
	}
   
	CLogArg ( "Acceptor: Thread for port %d terminated.", port );

	return 0;
}


void * MediatorDaemon::ClientThreadStarter ( void *arg )
{
	if ( !arg  ) {
		CErr ( "ClientThreadStarter: Called with (NULL) argument." );
		return 0;
	}
	
	// Execute thread
	return g_mediator->ClientThread ( arg );
}


bool MediatorDaemon::addToProject ( const char * project, const char * app, const char * pKey, const char * pValue )
{
	bool ret = false;

	// Search for the project at first
	map<string, ValuePack*>							*	values	= 0;
	AppsList										*	apps	= 0;
	map<string, map<string, ValuePack*>*>::iterator		appsIt;
		
	if ( !project || !app ) {
		CErr ( "addToProject: Invalid parameters. Missing project/app name!" );
		return false;
	}

	if ( pthread_mutex_lock ( &projectsMutex ) ) {
		CErr ( "addToProject: Failed to aquire mutex on project/values!" );
		return false;
	}
	
	string projectName ( project );
	string appName ( app );

	map<string, AppsList * >::iterator projectIt = projects.find ( projectName );			

	if ( projectIt == projects.end () )
	{		
		apps = new AppsList ();
		if ( !apps ) {
			CErrArg ( "addToProject: Failed to create new project [%s].", project );
			goto EndWithStatus;
		}
		projects [ projectName ] = apps;
	}
	else
		apps = projectIt->second;
	
	appsIt = apps->apps.find ( appName );

	if ( appsIt == apps->apps.end () )
	{		
		values = new map<string, ValuePack*> ();
		if ( !values ) {
			CErrArg ( "addToProject: Failed to create new application [%s].", appName.c_str () );
			goto EndWithStatus;
		}
		apps->apps [ appName ] = values;
	}
	else
		values = appsIt->second;

	ret = addToProject ( values, pKey, pValue, (unsigned int) strlen(pValue) );

EndWithStatus:
	if ( pthread_mutex_unlock ( &projectsMutex ) ) {
		CErr ( "addToProject: Failed to release mutex on project/values!" );
		return false;
	}

	return ret;
}


bool MediatorDaemon::addToProject ( map<string, ValuePack*> * values, const char * pKey, const char * pValue, unsigned int valueSize )
{
	bool ret = false;
	
	if ( !pKey || !pValue || valueSize <= 0 ) {
		CErr ( "addToProject: Invalid parameters. Missing key/value!" );
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
		CErrArg ( "addToProject: Failed to create new value object for %s! Memory low problem!", pKey );
		goto EndWithStatus;
	}
	pack->timestamp = 0;
	pack->value = string ( pValue );
	pack->size = valueSize;

	(*values) [ key ] = pack;
	ret = true;

	//CVerbArg ( "addToProject: added %s -> %s", pKey, pValue );

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
	
	if ( pthread_mutex_lock ( &projectsMutex ) ) {
		CErr ( "sendDatabase: Failed to aquire mutex on project/values! Skipping client request!" );
		return false;
	}

	for ( map<string, map<string, ValuePack*>*>::iterator it = projects.begin(); it != projects.end(); ++it )
	{
		CLogArg ( "Project: %s", it->first.c_str() );

		if ( it->second ) {
			for ( map<string, ValuePack*>::iterator itv = it->second->begin(); itv != it->second->end(); ++itv )
			{
				// Parse values: "<P>Project Key Value"
				// Parse values: "<P>Project Key Value<P>Project Key Value"
				// Parse values: "<P>Project Key Value<P><EOF>"
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

	if ( pthread_mutex_unlock ( &projectsMutex ) ) {
		CErr ( "sendDatabase: Failed to release mutex on project/values!" );
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
	char * params [ 4 ];
	memset ( params, 0, sizeof(params) );
	
	if ( !ScanForParameters ( buffer + sizeof(MediatorGetPacket), msgLength - sizeof(MediatorGetPacket), ";", params, 4 ) ) {
		CErr ( "HandleRequest: Invalid parameters." );
		return false;
	}

	if ( !params [ 0 ] || !params [ 1 ] || !params [ 2 ] ) {
		CErr ( "HandleRequest: Invalid parameters. project and/or key missing!" );
		return false;
	}
		
	if ( *(params [ 0 ]) == ' ' ) {
		/// Use project name of the client
		params [ 0 ] = client->device->info.projectName;
	}
		
	if ( *(params [ 1 ]) == ' ' ) {
		/// Use project name of the client
		params [ 1 ] = client->device->info.appName;
	}
		
	// Search for the project at first
	map<string, ValuePack*> *				values	= 0;
	AppsList							*	apps	= 0;
	map<string, map<string, ValuePack*>*>::iterator appsIt;
	map<string, ValuePack*>::iterator valueIt;
		
	if ( pthread_mutex_lock ( &projectsMutex ) ) {
		CErr ( "HandleRequest: Failed to aquire mutex on project/values! Skipping client request!" );
		return false;
	}

	string projectName ( params [ 0 ] );

	map<string, AppsList * >::iterator projectIt = projects.find ( projectName );			

	// COMMAND: Set New Value
	if ( query->cmd == MEDIATOR_CMD_SET )
	{
		if ( projectIt == projects.end() )
		{
			// Create a new map for the project
			apps = new AppsList ();
			if ( !apps ) {
				CErrArg ( "HandleRequest: Failed to create new project [%s].", params [ 0 ] );
				goto Continue;
			}
			projects [ projectName ] = apps;
		}
		else
			apps = projectIt->second;
		
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
		if ( !addToProject ( values, params [ 2 ], params [ 3 ], (unsigned) valueSize ) ) {
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

		if ( projectIt != projects.end() ) {
			apps = projectIt->second;
				
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
	if ( pthread_mutex_unlock ( &projectsMutex ) ) {
		CErr ( "HandleRequest: Failed to release mutex on project/values!" );
	}
	return ret;
}


ThreadInstance * MediatorDaemon::GetSessionClient ( long long sessionID )
{
	//sid |= ((long long)((client->device->root->id << 16) | client->device->root->projId)) << 32;
			
	map<long long, ThreadInstance *>::iterator sessionIt = sessions.find ( sessionID );
	if ( sessionIt != sessions.end () ) {
		return sessionIt->second;
	}
	return 0;
	/*
	unsigned int id = (unsigned int)((sessionID >> 32) & 0xFFFF);

	map<unsigned int, string>::iterator projIDsIt = projectIDs.find ( id );

	if ( projIDsIt == projectIDs.end () ) {
		CWarnArg ( "GetSessionClient: Failed to find project ID [%u]", id );
		return 0;
	}

	map<string, ProjectApps *>::iterator projIt = projectsList.find ( projIDsIt->second );	

	if ( projIt == projectsList.end ( ) ){
		CWarnArg ( "GetSessionClient: Failed to find project [%s]", projIDsIt->second.c_str () );
		return 0;
	}

	ProjectApps * projApps = projIt->second;

	id = (unsigned int)((sessionID >> 48) & 0xFFFF);
	
	map<unsigned int, string>::iterator appIDsIt = appIDs.find ( id );

	if ( appIDsIt == appIDs.end () ) {
		CWarnArg ( "GetSessionClient: Failed to find app ID [%u]", id );
		return 0;
	}
		
	map<string, ApplicationDevices * >::iterator appsIt = projApps->apps.find ( appIDsIt->second );
	
	if ( appsIt == projApps->apps.end ( ) ){
		CWarnArg ( "GetSessionClient: Failed to find app [%s]", appIDsIt->second.c_str () );
		return 0;
	}

	ApplicationDevices * appDevices = appsIt->second;
	*/
}


int MediatorDaemon::HandleRegistration ( int &deviceID, ThreadInstance * client, unsigned int bytesLeft, char * msg, unsigned int msgLen )
{
	CVerb ( "HandleRegistration" );

	int ret = -1;
	char * decrypted = 0;

	if ( msg [ 7 ] == 'S' ) {
		do
		{
			bool					useSession	= true;
			ThreadInstance		*	relClient	= 0;	
			SpareSockDecPack	*	regPack		= (SpareSockDecPack *) msg;

			/// Get the size of the registration packet
			unsigned int regLen =  0;

			if ( regPack->sizeReq >= 16 ) 
			{
				regLen =  regPack->sizePayload;
				if ( regLen + 12 > bytesLeft ) {
					CWarn ( "HandleRegistration: Spare socket registration packet overflow." ); break;
				}

				if ( pthread_mutex_lock ( &acceptClientsMutex ) ) {
					CErr ( "HandleRegistration: Failed to acquire mutex!" ); break;
				}
				relClient = GetSessionClient ( regPack->sessionID );
			}
			else {
				useSession = false;

				regLen = *((unsigned int *)(msg + 12));
				if ( regLen + 12 > bytesLeft ) {
					CWarn ( "HandleRegistration: Spare socket registration packet overflow." ); break;
				}

				/// Get the spare id
				unsigned int spareID = *((unsigned int *)(msg + 8));

				if ( pthread_mutex_lock ( &acceptClientsMutex ) ) {
					CErr ( "HandleRegistration: Failed to acquire mutex!" ); break;
				}

				map<unsigned int, ThreadInstance *>::iterator iter = spareClients.find ( spareID );
				if ( iter == spareClients.end () ) {
					CLogArg ( "HandleRegistration: Spare id [%u] not found.", spareID );
                
					if ( pthread_mutex_unlock ( &acceptClientsMutex ) ) {
						CErr ( "HandleRegistration: Failed to release mutex!" );
					}
					break;
				}

				relClient = iter->second;
				spareClients.erase ( iter );
			}

            if ( pthread_mutex_unlock ( &acceptClientsMutex ) ) {
                CErr ( "HandleRegistration: Failed to release mutex!" ); break;
            }
						
			if ( !relClient ) {
				CWarn ( "HandleRegistration: Spare socket client does not exist." ); break;
			}

			//CLogArg ( "HandleRegistration: Encrypted [%s]", ConvertToHexSpaceString ( msg + 16, regLen ) );

			if ( !AESDecrypt ( &relClient->aes, useSession ? &regPack->payload : (msg + 16), &regLen, &decrypted ) )
				break;
						
			if ( regLen != MEDIATOR_BROADCAST_SPARE_ID_LEN ) {
				CWarn ( "HandleRegistration: Spare socket registration packet is not of correct type." ); break;
			}

			HandleSpareSocketRegistration ( client, relClient, decrypted + 4, regLen );
			CVerbArg ( "[0x%X].HandleRegistration: Closing spare socket reg. thread.", relClient->deviceID );

			ret = 0;
		}
		while ( 0 );
	}
	else if ( msg [ 7 ] == MEDIATOR_OPT_DEVICE_LIST_DEVICE_ID ) {
		CLog ( "HandleRegistration: Assigning next device ID." );

		msg [ msgLen ] = 0;
		MediatorReqMsg	* req = (MediatorReqMsg *) msg;
		
        ApplicationDevices * appDevices = 0;
		int nextID = 0;
		int mappedID = 0;
        
		if ( pthread_mutex_lock ( &usersDBMutex ) ) {
			CErr ( "HandleRegistration: Failed to aquire mutex (devMap)!" );
		}
        else {
			DeviceMapping * mapping = 0;

			if ( *req->deviceUID ) {
				map<string, DeviceMapping *>::iterator devIt = deviceMappings.find ( string ( req->deviceUID ) );	

				if ( devIt != deviceMappings.end ( ) ) {
					mapping = devIt->second;
					nextID = mappedID = mapping->deviceID;
				}			
			}

			if ( !mapping ) {
				mapping = (DeviceMapping *) calloc ( 1, sizeof(DeviceMapping) );
				if ( !mapping )
                    goto PreFailExit;
			}

			GetDeviceList ( req->projectName, req->appName, 0, 0, &appDevices );
			if ( appDevices ) 
			{
				/// Find the next free deviceID			
				DeviceInstanceList	*	device				= appDevices->devices;
		
				if ( pthread_mutex_lock ( &appDevices->mutex ) ) {
					CErr ( "HandleRegistration: Failed to aquire mutex!" );
				}
				else {
					bool assigned = true;
					if ( nextID )
						assigned = false;
					else
						nextID = appDevices->latestAssignedID + 1;
				
					while ( device ) {
						if ( device->info.deviceID == nextID )
							nextID++;
						else 
						if ( device->info.deviceID > nextID )
							break;

						device = device->next;
					}

					if ( assigned )
						appDevices->latestAssignedID = nextID;

					if ( pthread_mutex_unlock ( &appDevices->mutex ) ) {
						CErr ( "HandleRegistration: Failed to release mutex!" );
					}
				}

				UnlockApplicationDevices ( appDevices );
			}

			if ( nextID && (nextID != mappedID) ) {
				mapping->deviceID = nextID;

				deviceMappings [ string ( req->deviceUID ) ] = mapping;
			}
            
        PreFailExit:
			if ( pthread_mutex_unlock ( &usersDBMutex ) ) {
				CErr ( "HandleRegistration: Failed to release mutex (devMap)!" );
            }
            
            if ( !mapping )
                return false;
            
            if ( nextID )
                SaveDeviceMappings ();
		}	
				
		CLogArg ( "HandleRegistration: Assigning device ID [%u] to [%s].", nextID, *req->deviceUID ? req->deviceUID : "Unknown" );

		SendBuffer ( client, &nextID, sizeof(nextID) );
		return 1;
	}
	else {
        /// Unencrypted mode
		deviceID = *( (unsigned int *) (msg + 16) );
		
		CLogArgID ( "HandleRegistration: [%c%c%c%c]", msg [ 4 ], msg [ 5 ], msg [ 6 ], msg [ 7 ] );
					
		/*if ( msgLen == MEDIATOR_BROADCAST_SPARE_ID_LEN ) {
			HandleSpareSocketRegistration ( client, deviceID );
			CVerbID ( "HandleRegistration: Closing spare socket reg. thread." );
			return 0;
		}
		else */
		if ( msgLen >= (MEDIATOR_BROADCAST_DESC_START + 3) ) {
			client->deviceID = deviceID;

			if ( !HandleDeviceRegistration ( client, (unsigned int) client->addr.sin_addr.s_addr, msg + 4 ) ) {
				CWarnID ( "HandleRegistration: Device registration failed." );
			}
			else {
				CLogID ( "HandleRegistration: Device registration succeeded." );
				return 1;
			}
		}
	}
	
	if ( decrypted) free ( decrypted );

	return ret;
}


void * MediatorDaemon::ClientThread ( void * arg )
{
	CLog ( "ClientThread started" );

	ThreadInstance * client = (ThreadInstance *) arg;
   
    int deviceID = 0;

	char buffer [ BUFFERSIZE ];

	socklen_t addrLen = sizeof(client->addr);

	CVerbArg ( "ClientThread started: port [%u]", ntohs ( client->addr.sin_port ) );

	char		*	msg;
	char		*	msgEnd			= buffer;
	int				bytesReceived, sock = client->socket;
	unsigned int	remainingSize	= BUFFERSIZE - 1;
    char		*	msgDec			= 0;
	unsigned int	msgDecLength;
    char		*	decrypted		= 0;
	
    bool isBanned = IsIpBanned ( client->addr.sin_addr.s_addr );
	if ( isBanned )
		goto ShutdownClient;

	while ( isRunning ) {
		bytesReceived = (int)recvfrom ( sock, msgEnd, remainingSize, 0, (struct sockaddr*) &client->addr, &addrLen );
		if ( bytesReceived <= 0 ) {
			CVerbArgID ( "Client: connection/socket [%i] closed by someone; Bytes [%i]!", sock, bytesReceived );
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

			if ( msgLength > (unsigned int) bytesLeft ) 
			{
				if ( msgLength >= BUFFERSIZE ) goto ShutdownClient;
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
                            CWarn ( "Client: Decrypt with privKey failed!" ); goto ShutdownClient;
                        }
                        msgDec = decrypted;
                    }
                    else {
                        /// Unknown encryption flags
                        goto ShutdownClient;
                    }
                }
            }
            else {
                msgDecLength = msgLength; msgDec = msg;
            }

			if ( msgDec [ 4 ] == 'H' && msgDec [ 5 ] == 'C' && msgDec [ 6 ] == 'L' && msgDec [ 7 ] == 'S' ) 
			{
				if ( client->encrypt ) {
					CWarnID ( "Client: tried to establish a safe channel again. Not allowed." );
					goto ShutdownClient;
				}

				if ( !SecureChannelAuth ( client ) ) {
					CWarnID ( "Client: Establishing Security / Authentication failed." );
					goto ShutdownClient;
				}
				CLogID ( "Client: Establishing Security / Authentication succeeded." );
				goto Continue;
			}

			if ( msgDec [ 4 ] == 'E' ) {
				if ( msgDec [ 5 ] == '.' && msgDec [ 6 ] == 'D' )
				{
					int ret = HandleRegistration ( deviceID, client, bytesLeft, msgDec, msgDecLength );
                    if ( ret == 1 ) {
                        BannIPRemove ( client->addr.sin_addr.s_addr );
                        goto Continue;
                    }
					if ( ret == 0 ) {
						if ( decrypted ) free ( decrypted );
						return 0;
					}
					goto ShutdownClient;
				}
		
				CLogArgID ( "Client: Invalid message [%c%c%c%c]. Refusing client.", msgDec [ 4 ], msgDec [ 5 ], msgDec [ 6 ], msgDec [ 7 ] );
				goto ShutdownClient;
			}

			CVerbArgID ( "Client: [%c%c%c%c]", msgDec [ 4 ], msgDec [ 5 ], msgDec [ 6 ], msgDec [ 7 ] );

			if ( !client->deviceID ) {
				CWarnArgID ( "Client: Request has not been identified. kicking connect from [%s]", inet_ntoa ( client->addr.sin_addr ) );

				if ( !client->authenticated )
					BannIP ( client->addr.sin_addr.s_addr );
				goto ShutdownClient;
			}

			if ( msgDec [ 4 ] < MEDIATOR_PROTOCOL_VERSION_MIN ) {
				CWarnArgID ( "Client: Version [%c] not supported anymore", msgDec [ 4 ] );
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
					CErrArgID ( "Client: Failed to response NAT value [%d]", behindNAT );
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
				HandleQueryDevices ( client, msgDec );
			}
			// COMMAND: 
			else if ( command == MEDIATOR_CMD_SHORT_MESSAGE ) 
			{
				HandleShortMessage ( client, msgDec );
			}
			// COMMAND: 
			else if ( command == MEDIATOR_CMD_REQ_SPARE_ID ) 
			{
				if ( pthread_mutex_lock ( &acceptClientsMutex ) ) {
					CErrID ( "Client: Failed to aquire mutex on clients!" ); 
				}

				unsigned int sid = spareID++;
				
				if ( pthread_mutex_unlock ( &acceptClientsMutex ) ) {
					CErrID ( "Client: Failed to release mutex on clients!" ); 
				}
				CVerbArgID ( "Client: Assigned spare ID [%u]", sid ); 

				spareClients [ sid ] = client;
				
				*((unsigned int *) msgDec) = MEDIATOR_NAT_REQ_SIZE;
				*((unsigned int *) (msgDec + 4)) = sid;
				
				int sentBytes = SendBuffer ( client, msgDec, MEDIATOR_NAT_REQ_SIZE );

				if ( sentBytes != MEDIATOR_NAT_REQ_SIZE ) {
					CErrArgID ( "Client: Failed to response spare client ID [%u]", sid );
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
				MediatorMsg * medMsg = (MediatorMsg *) msgDec;
				UpdateNotifyTargets ( client, medMsg->ids.id2.msgID );
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
                    CErrID ( "Client: Failed to response Mediator version" );
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

		remainingSize = BUFFERSIZE - 1;

		if ( bytesLeft > 0 ) {
			refactorBuffer ( msg, buffer, bytesLeft, msgEnd );
			remainingSize -= bytesLeft;
		}
		else
			msgEnd = buffer;
	}

ShutdownClient:
	sock = client->socket;
	if ( sock != -1 ) {
		client->socket = -1;

		shutdown ( sock, 2 );
		closesocket ( sock );
	}

	sock = client->spareSocket;
	if ( sock != -1 ) {
		client->spareSocket = -1;

		shutdown ( sock, 2 );
		closesocket ( sock );
	}

    if ( decrypted ) free ( decrypted );

	if ( isRunning )
	{
		if ( !client->deviceID && client->addr.sin_addr.s_addr ) {
            if ( !isBanned ) {
                CWarnArgID ( "Client: Request has not been identified. kicking connect from [%s]", inet_ntoa ( client->addr.sin_addr ) );
                BannIP ( client->addr.sin_addr.s_addr );
                isBanned = true;
            }
		}
        
        if ( !isBanned )
            UpdateNotifyTargets ( client, -1 );

		if ( client->device )
			RemoveDevice ( client->device );

		pthread_t thrd = client->threadID;
		if ( pthread_valid ( thrd ) ) 
		{
			pthread_reset ( client->threadID );
			pthread_detach_handle ( thrd );
		}

		bool useLock = (client->port != 0);
		
		if ( useLock && pthread_mutex_lock ( &acceptClientsMutex ) ) {
			// We proceed on the assumption that the main thread keeps the mutex and is responsible for disposing the resources
			CErrID ( "Client: Failed to aquire mutex!" ); 
		}
	
		// find the one in our vector
		for ( unsigned i = 0; i < acceptClients.size (); i++ ) {
			if ( acceptClients[i] == client ) {
				CVerbArgID ( "Client: Erasing [%i] from client list", i );
				acceptClients.erase ( acceptClients.begin() + i );
				break;
			}
		}
		
		map<long long, ThreadInstance *>::iterator sessionIt = sessions.find ( client->sessionID );
		if ( sessionIt != sessions.end () ) {
			sessions.erase ( sessionIt );
		}

		CVerbID ( "Client: Disposing memory for client" );
		
		if ( pthread_mutex_destroy ( &client->accessMutex ) ) {
			CErrID ( "Client: Failed to desroy mutex!" );
		}

		if ( client->aes.encCtx )
			AESDisposeKeyContext ( &client->aes );

		delete client;
		
		if ( useLock && pthread_mutex_unlock ( &acceptClientsMutex ) ) {
			CErrID ( "Client: Failed to release mutex!" );
		}
	}

	CLogID ( "Client: Disconnected" );

	return 0;
}


DeviceInstanceList * MediatorDaemon::GetDeviceInstance ( int deviceID, DeviceInstanceList * device )
{
	while ( device ) {
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
	const char		*	projName			= 0;
	const char		*	appName				= 0;
	int					sentBytes;
	DeviceInstanceList	* device;
	    
	// find the destination client
	ThreadInstance * destClient = 0;
				
    DeviceInstanceList * destList		= 0;
    pthread_mutex_t *	destMutex		= 0;
    ApplicationDevices * appDevices		= 0;

	if ( shortMsg->version >= '3' ) {
		if ( *shortMsg->projectName && *shortMsg->appName ) {
			projName = shortMsg->projectName;
			appName = shortMsg->appName;
		}
	}
    
    if ( !projName || !appName ) {
        projName = sourceClient->device->info.projectName;
        appName = sourceClient->device->info.appName;

        destMutex = &sourceClient->device->root->mutex;
        destList = sourceClient->device->root->devices;
    }
    else {
        appDevices = GetApplicationDevices(projName, appName);
        if ( !appDevices ) {
            goto SendResponse;
        }
        destList = appDevices->devices;
        destMutex = &appDevices->mutex;
    }

    
	if ( pthread_mutex_lock ( destMutex ) ) {
		CErrID ( "HandleSTUNTRequest: Failed to aquire mutex!" );
	}
    if ( appDevices )
        UnlockApplicationDevices ( appDevices );
	
	device = GetDeviceInstance ( destID, destList );
	if ( device )
		destClient = device->client;

	if ( !destClient || destClient->socket == -1 ) {
		CErrArgID ( "HandleShortMessage: Failed to find device connection for id [0x%X]!", destID );
		goto SendResponse;
	}
	
	shortMsg->deviceID = deviceID;
	CLogArgID ( "HandleShortMessage: send message to device [%u] IP [%u bytes -> %s]", destID, length, inet_ntoa ( destClient->addr.sin_addr ) );
	
	sentBytes = SendBuffer ( destClient, sendBuffer, length );
	if ( sentBytes == length ) {
		CLogID ( "HandleShortMessage: successfully sent message." );
		sendError = false;
	}

SendResponse:
	if ( destMutex && pthread_mutex_unlock ( destMutex ) ) {
		CErrID ( "HandleShortMessage: Failed to release mutex!" );
	}

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
	int reqDeviceID, const char * projName, const char * appName, 
	ApplicationDevices * appDevices, DeviceInstanceList *& resultList, unsigned int &resultCount )
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
	DeviceInstanceList  *	curDevice			= resultList;
	unsigned int            currentStart;
	DeviceInstanceList	*	device				= appDevices->devices;
		

	if ( pthread_mutex_lock ( &appDevices->mutex ) ) {
		CErr ( "CollectDevices: Failed to aquire mutex!" );
		return false;
	}

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
    
    DeviceInstanceList * prev = curDevice;

	while ( device )
    {
        if ( reqDeviceID ) {
            if ( reqDeviceID == device->info.deviceID && !strncmp ( device->info.projectName, projName, MAX_NAMEPROPERTY )
                && !strncmp ( device->info.appName, appName, MAX_NAMEPROPERTY ) )
            {
                DeviceInstanceList * newDevice = (DeviceInstanceList *) malloc ( sizeof(DeviceInstanceList) );
                if ( !newDevice ) {
					ret = false; goto Finish;
				}
                
                memcpy ( newDevice, device, sizeof(DeviceInfo) );
                newDevice->next = 0;
                
                resultList = newDevice;
                resultCount = 1;
				ret = false; goto Finish; /// Stop the query, We have found the device.
            }
            
            device = device->next;
            continue;
        }
        
        /// We have a device to insert into the result list
        DeviceInstanceList * newDevice = (DeviceInstanceList *) malloc ( sizeof(DeviceInstanceList) );
        if ( !newDevice ) {
			ret = false; goto Finish;
		}
        
        memcpy ( newDevice, device, sizeof(DeviceInfo) );
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
	if ( pthread_mutex_unlock ( &appDevices->mutex ) ) {
		CErr ( "CollectDevices: Failed to release mutex!" );
		return false;
	}
	return ret;
}


unsigned int MediatorDaemon::CollectDevicesCount ( DeviceInstanceList * sourceDevice, int filterMode )
{
	if ( !sourceDevice || !sourceDevice->root )
		return 0;

	unsigned int            deviceCount = 0;
    ProjectApps         *   projApps;
    ApplicationDevices  *   appDevices;

	/// Get number of devices within the same application environment
	deviceCount += sourceDevice->root->count;

	if ( filterMode < 1 ) { 
		/// No filtering, get them all				
		if ( pthread_mutex_lock ( &devicesMutex ) ) {
			CErr ( "CollectDevicesCount: Failed to acquire mutex!" );
			goto Finish;
		}

		/// Iterate over all projects
		for ( map<string, ProjectApps*>::iterator it = projectsList.begin(); it != projectsList.end (); ++it )
		{
			if ( !it->second )
				continue;
						
			projApps = it->second;

			for ( map<string, ApplicationDevices *>::iterator ita = projApps->apps.begin(); ita != projApps->apps.end (); ++ita )
			{
				appDevices = ita->second;
				if ( !appDevices || appDevices == sourceDevice->root )
					continue;

				deviceCount += appDevices->count;
			}
		}
			
		if ( pthread_mutex_unlock ( &devicesMutex ) ) {
			CErr ( "CollectDevicesCount: Failed to release mutex!" );
		}
	}
	else if ( filterMode < 2 ) {
		/// Get number of devices of other application environments within the same project
		string projName ( sourceDevice->info.projectName );
		
		if ( pthread_mutex_lock ( &devicesMutex ) ) {
			CErr ( "CollectDevicesCount: Failed to acquire mutex!" );
			goto Finish;
		}

		map<string, ProjectApps * >::iterator projIt = projectsList.find ( projName );	

		if ( projIt == projectsList.end ( ) ) {
			CLogArg ( "CollectDevicesCount: Project [%s] not found.", sourceDevice->info.projectName );
			goto FinishLimitApps;
		}
	
		projApps = projIt->second;
    
		if ( !projApps ) {
			CLog ( "CollectDevicesCount: Invalid project name." );
			goto FinishLimitApps;
		}
				
		for ( map<string, ApplicationDevices *>::iterator ita = projApps->apps.begin(); ita != projApps->apps.end (); ++ita )
		{
			appDevices = ita->second;
			if ( !appDevices || appDevices == sourceDevice->root )
				continue;

			deviceCount += appDevices->count;
		}

FinishLimitApps:		
		if ( pthread_mutex_unlock ( &devicesMutex ) ) {
			CErr ( "CollectDevicesCount: Failed to release mutex!" );
		}
	}

Finish:
	return deviceCount;
}


char * FlattenDeviceList ( DeviceInstanceList * devices, unsigned int capacity )
{    
    DeviceInfo * dest = 0;
    char * buffer = (char *) malloc ( (capacity * DEVICE_PACKET_SIZE) + DEVICES_HEADER_SIZE + DEVICES_HEADER_SIZE );
    if ( !buffer ) {
        goto Finish;
    }

    if ( !devices || !capacity )
        goto Finish;
    
    dest = (DeviceInfo *) (buffer + DEVICES_HEADER_SIZE);
    
    while ( devices ) {
        DeviceInstanceList * current = devices;
        devices = devices->next;

        memcpy ( dest, current, sizeof(DeviceInfo) );
        dest++;

        free ( current );
    }
    
Finish:
    if ( devices ) {
        while ( devices ) {
            DeviceInstanceList * current = devices;
            
            devices = devices->next;
            free ( current );
        }
    }
    return buffer;
}


bool MediatorDaemon::HandleQueryDevices ( ThreadInstance * sourceClient, char * msg )
{
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
	const char		*	projName			= sourceClient->device->info.projectName;
	const char		*	appName				= sourceClient->device->info.appName;
	char				subCmd;
    DeviceInstanceList  *   resultList          = 0;
    unsigned int        resultCount         = 0;

	map<string, ApplicationDevices *> * apps;
	 
	// sourceClient->device may be invalid which causes an exception
	ApplicationDevices * appDevices = sourceClient->device->root;
	
	/// Take over filterMode	
	if ( query->cmdVersion < MEDIATOR_PROTOCOL_VERSION )
		/// Deprecated, to be removed as soon as the public tree has been updated
		subCmd = query->opt0;
	else
		subCmd = query->opt1;

	if ( subCmd != MEDIATOR_OPT_NULL ) {
		filterMode = subCmd;
	}

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

			response->deviceHead.deviceCountAvailable = CollectDevicesCount ( sourceClient->device, filterMode );
		}
		else {
			query->size = length;
			query->cmdVersion = MEDIATOR_PROTOCOL_VERSION;
			query->cmd1 = MEDIATOR_CMD_DEVICE_LIST_QUERY;
			query->opt0 = MEDIATOR_CMD_DEVICE_LIST_QUERY_COUNT;

			query->msgID = CollectDevicesCount ( sourceClient->device, filterMode );
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
		if ( *query->projectName && *query->appName ) {
			projName = query->projectName;
			appName = query->appName;

			CVerbArgID ( "HandleQueryDevices: Query for deviceID [%u] [%s/%s]", deviceIDReq, projName, appName );
		}
		else {
			projName = sourceClient->device->info.projectName;
			appName = sourceClient->device->info.appName;

			CVerbArgID ( "HandleQueryDevices: Query for deviceID [%u] of default appEnv [%s/%s]", deviceIDReq, projName, appName );
		}
	}

	CVerbArgID ( "HandleQueryDevices: start index [%u] filterMode [%i]", startIndex, filterMode );

	/// Lets start with the devices within the same appEnv	

	if ( appDevices->count && !CollectDevices ( sourceClient, startIndex, deviceIDReq, projName, appName, 
		appDevices, resultList, resultCount ) ) 
		goto Finish;
	
	if ( filterMode < 1 ) { 
		/// No filtering, get them all		
		if ( pthread_mutex_lock ( &devicesMutex ) ) {
			CErr ( "HandleQueryDevices: Failed to acquire mutex!" );
			goto Finish;
		}

		/// Iterate over all projects
		for ( map<string, ProjectApps*>::iterator it = projectsList.begin(); it != projectsList.end (); ++it )
		{
			if ( !it->second )
				continue;
			apps = &it->second->apps;

			for ( map<string, ApplicationDevices *>::iterator ita = apps->begin(); ita != apps->end (); ita++ )
			{
				appDevices = ita->second;
				if ( !appDevices || appDevices == sourceClient->device->root || !appDevices->count  )
					continue;

				availableDevices += appDevices->count;

				if ( !CollectDevices ( sourceClient, startIndex, deviceIDReq, projName, appName, 
					appDevices, resultList, resultCount ) ) {
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
		/// Get number of devices of other application environments within the same project
		string projectName ( projName );
		
		if ( pthread_mutex_lock ( &devicesMutex ) ) {
			CErr ( "HandleQueryDevices: Failed to acquire mutex!" );
			goto Finish;
		}

		map<string, ProjectApps * >::iterator projIt = projectsList.find ( projectName );	

		if ( projIt == projectsList.end ( ) || !projIt->second ) {
			CLogArg ( "HandleQueryDevices: Project [%s] not found.", projName );
			goto FinishLimitProject;
		}

		apps = &projIt->second->apps;
				
		for ( map<string, ApplicationDevices *>::iterator ita = apps->begin(); ita != apps->end (); ita++ )
		{
			appDevices = ita->second;
			if ( !appDevices || appDevices == sourceClient->device->root || !appDevices->count )
				continue;

			availableDevices += appDevices->count;

			if ( !CollectDevices ( sourceClient, startIndex, deviceIDReq, projName, appName, 
				appDevices, resultList, resultCount ) ) {
					goto FinishLimitProject;
			}
		}

FinishLimitProject:
		if ( pthread_mutex_unlock ( &devicesMutex ) ) {
			CErr ( "HandleQueryDevices: Failed to release mutex!" );
		}
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
	
	int sentBytes = SendBuffer ( sourceClient, sendBuffer, length );

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
	
	// SourceID -> DestID
	unsigned int * pUI = (unsigned int *) (msg + 8);
	unsigned int SourceID = *pUI; pUI++;
	unsigned int DestID = *pUI;

	CVerbArg ( "[0x%X].HandleSTUNRequest: TCPrec -> [0x%X]", SourceID, DestID );

	// find the source client
	ThreadInstance * destClient = 0;
	
	if ( pthread_mutex_lock ( &sourceClient->device->root->mutex ) ) {
		CErr ( "HandleSTUNRequest: Failed to aquire mutex!" ); 
		return false;
	}
	
	DeviceInstanceList * device = GetDeviceInstance ( DestID, sourceClient->device->root->devices );
	
	if ( device )
		destClient = device->client;

	if ( !destClient ) {
		CWarnArg ( "[0x%X].HandleSTUNRequest -> Destination device -> [0x%X] not found.", SourceID, DestID );
		
		if ( pthread_mutex_unlock ( &sourceClient->device->root->mutex ) ) {
			CWarn ( "HandleSTUNRequest: Failed to release mutex!" );
		}
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
	
	int sentBytes = SendBuffer ( destClient, buffer, MEDIATOR_STUN_RESP_SIZE );
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
	
	if ( pthread_mutex_unlock ( &sourceClient->device->root->mutex ) ) {
		CWarn ( "HandleSTUNRequest: Failed to release mutex!" );
	}

	if ( sentBytes != MEDIATOR_STUN_RESP_SIZE ) {
		CErrArg ( "[0x%X].HandleSTUNRequest: Failed to send STUN reply to device IP [%s]!", DestID, inet_ntoa ( sourceClient->addr.sin_addr ) );
		return false;
	}

	return true;
}


bool MediatorDaemon::HandleSTUNRequest ( ThreadInstance * destClient, unsigned int sourceID, const char * projName, const char * appName, unsigned int IP, unsigned int Port )
{
	STUNReqReqPacket	request;
	Zero ( request );

	//char buffer [ 48 ];

	CVerbArg ( "[0x%X].HandleSTUNRequest: UDPrec -> TCPsend [0x%X]", sourceID, destClient->deviceID );

	request.size = sizeof(request);
	memcpy ( request.ident, "y;;;", 4 );

	request.deviceID = sourceID;
	request.IP = IP;
	request.Port = Port;
	
	strcpy_s ( request.projectName, sizeof(request.projectName), projName );
	strcpy_s ( request.appName, sizeof(request.appName), appName );

	/*unsigned int * pUI = (unsigned int *) buffer;
	*pUI = MEDIATOR_STUN_RESP_SIZE;

	buffer [ 4 ] = 'y';
	buffer [ 5 ] = ';';
	buffer [ 6 ] = ';';
	buffer [ 7 ] = ';';
	pUI += 2;

	*pUI = sourceID; pUI++;
	*pUI = IP; pUI++;
	*pUI = Port; pUI++;*/
	
	CLogArg ( "[0x%X].HandleSTUNRequest: Send STUN request to device IP [%s] Port [%u]!", destClient->deviceID, inet_ntoa ( destClient->addr.sin_addr ), Port );
	
	
	int sentBytes = SendBuffer ( destClient, &request, sizeof(request) );
	if ( sentBytes != sizeof(request) ) 
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

	req.notify = NOTIFY_MEDIATOR_MED_STUNT_REG_REQ;
	    
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
	int					sentBytes;
    DeviceInstanceList *	sourceDevice;

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
    const char  *		projName        = 0;
    const char  *		appName         = 0;
    DeviceInstanceList *	device			= 0;
    
	// find the destination client
	ThreadInstance *	destClient		= 0;				
    DeviceInstanceList *	destList		= 0;
    pthread_mutex_t *	destMutex		= 0;
    ApplicationDevices * appDevices		= 0;
    
    if ( req->size >= sizeof(STUNTReqPacket) ) {
        if ( *req->projectName )
            projName = req->projectName;
        if ( *req->appName )
            appName = req->appName;
    }
    
    if ( !projName || !appName ) {
        projName = sourceClient->device->info.projectName;
        appName = sourceClient->device->info.appName;

        destMutex = &sourceClient->device->root->mutex;
        destList = sourceClient->device->root->devices;
    }
    else {
        appDevices = GetApplicationDevices ( projName, appName );
        if ( !appDevices ) {
            goto UnlockQuit;
        }
        destList = appDevices->devices;
        destMutex = &appDevices->mutex;
    }

    
	if ( pthread_mutex_lock ( destMutex ) ) {
		CErrID ( "HandleSTUNTRequest: Failed to aquire mutex!" );
	}
    if ( appDevices )
        UnlockApplicationDevices ( appDevices );
	
	device = GetDeviceInstance ( deviceID, destList );
	if ( device )
		destClient = device->client;

	if ( !destClient ) {
		CErrID ( "HandleSTUNTRequest: Failed to find device connection for id!" ); goto UnlockQuit;
	}

	sourceDevice = sourceClient->device;
	if ( !sourceDevice ) {
		CErrID ( "HandleSTUNTRequest: Failed to find sourceClient device in database!" ); goto UnlockQuit;
	}

	// Acquire the mutex on sourceClient
	if ( pthread_mutex_lock ( &sourceClient->accessMutex ) ) {
		CErrID ( "HandleSTUNTRequest: Failed to acquire mutex on sourceClient." ); goto UnlockQuit;
	}
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
	if ( pthread_mutex_lock ( &destClient->accessMutex ) ) {
		CErrID ( "HandleSTUNTRequest: Failed to acquire mutex on destClient." ); goto UnlockQuit;
	}
	destLocked = true;
	
	///
	/// Check whether the destination client is ready for a stunt
	///
	portDest = destClient->sparePort;
	if ( !portDest ) {
		CLogID ( "HandleSTUNTRequest: Destination client has not renewed the spare socket yet!" );

		NotifySTUNTRegRequest ( destClient );  status = -1; goto UnlockQuit;
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
	response.port = portSource;

    strcpy_s ( response.projectName, sizeof(response.projectName), sourceClient->device->info.projectName );
    strcpy_s ( response.appName, sizeof(response.appName), sourceClient->device->info.appName );
	
	CLogArgID ( "STUNTRequest: Send request to device IP [%s], port [%d]", inet_ntoa ( destClient->addr.sin_addr ), portSource );
	
	sentBytes = SendBuffer ( destClient, &response, sizeof(response), false );

	if ( sentBytes != sizeof(response) ) {
		CErrArgID ( "HandleSTUNTRequest: Failed to send STUNT request to device IP [%s]!", inet_ntoa ( destClient->addr.sin_addr ) );
		LogSocketError ();
		goto UnlockQuit;
	}

	sendSize = extResp ? MEDIATOR_STUNT_ACK_EXT_SIZE : MEDIATOR_STUNT_ACK_SIZE;
	reqResponse->size = sendSize;

	reqResponse->respCode = 'p';

	reqResponse->port = portDest;
	reqResponse->portUdp = destClient->portUdp;
	
	if ( extResp ) {
		reqResponse->ip = destClient->device->info.ip;
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
	if ( destLocked && pthread_mutex_unlock ( &destClient->accessMutex ) ) {
		CErrID ( "HandleSTUNTRequest: Failed to release mutex on destClient." );
	}
	
	/// Release the mutex on sourceClient
	if ( sourceLocked && pthread_mutex_unlock ( &sourceClient->accessMutex ) ) {
		CErrID ( "HandleSTUNTRequest: Failed to release mutex on sourceClient." );
	}
	
	if ( destMutex && pthread_mutex_unlock ( destMutex ) ) {
		CErrID ( "HandleSTUNTRequest: Failed to release mutex on clients!" );
	}

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

	*((unsigned int *) (broadcastMessage + broadcastMessageLen)) = 1; // We use the platform 1 for now


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

		if ( bytesReceived < 10 || buffer [ 0 ] != 'E' || buffer [ 1 ] != '.' )
			continue;
		
#ifdef DEBUGVERB
		unsigned int ip = listenAddr.sin_addr.s_addr;
#endif		
		buffer [ bytesReceived ] = 0;

		if ( buffer [ 2 ] == 'D' ) {
			// if message contains EHLO, then broadcast our data
			if ( buffer [ 4 ] == 'H' ) {
				SendBroadcast ();
				CVerbArg ( "BroadcastThread: Replied to client request from %s;\n\t\tMessage: %s", inet_ntoa ( *((struct in_addr *) &ip) ), buffer);
			}
		}
		else {
			CVerbArg ( "BroadcastThread: read %d bytes <- IP %s;\n\t\tMessage: %s", bytesReceived, inet_ntoa ( *((struct in_addr *) &ip) ), buffer);
		}
	}
   
	CLog ( "BroadcastThread: Thread terminated." );

	return 0;
}


bool MediatorDaemon::UpdateDeviceRegistry ( DeviceInstanceList * device, unsigned int ip, char * msg )
{
	CVerb ( "UpdateDeviceRegistry" );

    char * keyCat;
    char * value;

	if ( !device ) {
		return false;
	}

	// Update the values in our database
	if ( pthread_mutex_lock ( &devicesMutex ) ) {
		CErr ( "UpdateDeviceRegistry: Failed to aquire mutex on devices!" );
		return false;
	}

	char * pprojectName = device->info.projectName;
		
	if ( !pprojectName || pthread_mutex_lock ( &projectsMutex ) ) {
		CErr ( "UpdateDeviceRegistry: Failed to aquire mutex on project/values (alt: projectName invalid)! Skipping client request!" );

		if ( pthread_mutex_unlock ( &devicesMutex ) ) {
			CErr ( "UpdateDeviceRegistry: Failed to release mutex on devices!" );
		}
		return false;
	}
		
	// Search for the project at first
	map<string, ValuePack*>					*	values = 0;
	AppsList								*	apps	= 0;
	map<string, map<string, ValuePack*>*>::iterator appsIt;

		
	string projectName ( pprojectName );
	string appName ( device->info.appName );

	map<string, AppsList * >::iterator projectIt = projects.find ( projectName );			

	if ( projectIt == projects.end () )
	{		
		apps = new AppsList ();
		if ( !apps ) {
			CErrArg ( "UpdateDeviceRegistry: Failed to create new project [%s].", pprojectName );
			goto Continue;
		}
		projects [ projectName ] = apps;
	}
	else
		apps = projectIt->second;
	
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
	sprintf_s ( keyBuffer, 128, "%u_", device->info.deviceID );

	keyCat = keyBuffer + strlen ( keyBuffer );
	
	// ip in register message
	strcat_s ( keyCat, 100, "ip" );

	value = inet_ntoa ( *((struct in_addr *) &device->info.ip) );

	if ( !addToProject ( values, keyBuffer, value, (unsigned int) strlen ( value ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}		
	
	// extern ip
	*keyCat = 0;
	strcat_s ( keyCat, 100, "ipe" );

	value = inet_ntoa ( *((struct in_addr *) &ip) );

	if ( !addToProject ( values, keyBuffer, value, (unsigned int) strlen ( value ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}		
	
	// tcp port
	*keyCat = 0;
	strcat_s ( keyCat, 100, "cport" );
	sprintf_s ( valueBuffer, 128, "%u", device->info.tcpPort );

	if ( !addToProject ( values, keyBuffer, valueBuffer, (unsigned int) strlen ( valueBuffer ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}		
	
	// udp port
	*keyCat = 0;
	strcat_s ( keyCat, 100, "dport" );
	sprintf_s ( valueBuffer, 128, "%u", device->info.udpPort );

	if ( !addToProject ( values, keyBuffer, valueBuffer, (unsigned int) strlen ( valueBuffer ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}
	
	// device type
	*keyCat = 0;
	strcat_s ( keyCat, 100, "type" );
	sprintf_s ( valueBuffer, 128, "%c", device->info.deviceType );

	if ( !addToProject ( values, keyBuffer, valueBuffer, (unsigned int) strlen ( valueBuffer ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}
	
	// device name
	*keyCat = 0;
	strcat_s ( keyCat, 100, "name" );

	if ( !addToProject ( values, keyBuffer, device->info.deviceName, (unsigned int) strlen ( device->info.deviceName ) ) ) {
		CWarnArg ( "UpdateDeviceRegistry: Adding key %s failed!", keyBuffer );
	}
			
Continue:
	if ( pthread_mutex_unlock ( &projectsMutex ) ) {
		CErr ( "UpdateDeviceRegistry: Failed to release mutex on project/values!" );
	}

	if ( pthread_mutex_unlock ( &devicesMutex ) ) {
		CErr ( "UpdateDeviceRegistry: Failed to release mutex on devices!" );
	}

	//SaveConfig ();

	return true;
}


bool MediatorDaemon::HandleDeviceRegistration ( ThreadInstance * client, unsigned int ip, char * msg )
{
	CVerb ( "HandleDeviceRegistration" );
	
    MediatorReqMsg  regMsg;
    int             sentBytes;
    
	if ( !client )
		return false;	

	unsigned int deviceID = client->deviceID;
	if ( !deviceID )
		return false;

	// Clear all lingering devices in the list with the same deviceID

	if ( pthread_mutex_lock ( &acceptClientsMutex ) ) {
		CErrID ( "HandleDeviceRegistration: Failed to aquire mutex!" );
		return false;
	}
	
	bool found = false;

	for ( unsigned int i = 0; i < acceptClients.size (); i++ ) 
	{		
		// find the one in our vector
		for ( unsigned i = 0; i < acceptClients.size (); i++ ) {
			if ( acceptClients[i] == client ) {
				CVerbArgID ( "HandleDeviceRegistration: Erasing [%i] from client list", i );
				acceptClients.erase ( acceptClients.begin() + i );

				found = true;
				break;
			}
		}
	}

	if ( pthread_mutex_unlock ( &acceptClientsMutex ) ) {
		CErrID ( "HandleDeviceRegistration: Failed to release mutex!" );
		return false;
	}

	if ( !found ) {
		CErrID ( "HandleDeviceRegistration: Failed to lookup the client in acceptClients." );
		return false;
	}

	char		*	uid		= 0;
	bool			created = false;

	DeviceInstanceList	* device = UpdateDevices ( ip, msg, &uid, &created );
	if ( !device ) {
		CErrID ( "HandleDeviceRegistration: Failed to parse registration." );
		return false;
	}
	if ( !created ) {
        /// TODO: to test. seems to be a better solution
		/// Remove an active session at first
		if ( device->client ) {
            int sock = device->client->socket;
            if ( sock != -1 ) {
				device->client->socket = -1;
				try {
					closesocket ( sock );
					//shutdown ( sock, 2 );
				} catch (...) {
				}
            }
//			ReleaseClient ( device->client );
		}
		else
			RemoveDevice ( device );

		CErrID ( "HandleDeviceRegistration: A device instance for this registration already exists." );
		return false;
	}

	client->device = device;
	device->client = client;

	*client->uid = 0;
	if ( uid && *uid && device->info.deviceID ) {		
		strcpy_s ( client->uid, sizeof(client->uid), uid );
        
		if ( pthread_mutex_lock ( &usersDBMutex ) ) {
			CErr ( "HandleDeviceRegistration: Failed to acquire mutex (devMap)!" );
		}
		else {
			
            DeviceMapping * mapping = 0;
            
            Zero ( regMsg );

			map<string, DeviceMapping *>::iterator devIt = deviceMappings.find ( string ( client->uid ) );	

			if ( devIt != deviceMappings.end ( ) ) {
				mapping = devIt->second;
			}
			else {
				mapping = (DeviceMapping *) calloc ( 1, sizeof(DeviceMapping) );
				if ( !mapping )
					goto PreFailExit;
			}
			mapping->deviceID = device->info.deviceID;
            
            /// Check for authToken
            if ( !*mapping->authToken || client->createAuthToken ) {
				srand ( getRandomValue ( &client ) );

                unsigned int tokCount = sizeof(mapping->authToken) - (rand () % 5);
                
                for ( unsigned int i=0; i<tokCount; i++ ) {
                    mapping->authToken [ i ] = (rand () % 93) + 33;
                }
                
                regMsg.size = sizeof(MediatorMsg) + tokCount - sizeof(long long);
                regMsg.cmd0 = MEDIATOR_PROTOCOL_VERSION;
                regMsg.cmd1 = MEDIATOR_CMD_AUTHTOKEN_ASSIGN;
                regMsg.opt0 = MEDIATOR_OPT_NULL;
                regMsg.opt1 = MEDIATOR_OPT_NULL;
                
                memcpy ( regMsg.projectName, mapping->authToken, tokCount );
            }

			deviceMappings [ string ( client->uid ) ] = mapping;

        PreFailExit:
			if ( pthread_mutex_unlock ( &usersDBMutex ) ) {
                CErr ( "HandleDeviceRegistration: Failed to release mutex (devMap)!" );
                return false;
			}
            
            if ( !mapping )
                return false;
            
            if ( regMsg.size ) {
                /// Send authToken to client
                sentBytes = SendBuffer ( client, &regMsg, regMsg.size );
                
                if ( sentBytes != (int)regMsg.size ) {
                    CErrID ( "HandleDeviceRegistration: Failed to assign authtoken!" );
                    return false;
                }
            }
            
            SaveDeviceMappings ();
		}		
	}

	client->version = *msg;

	UpdateDeviceRegistry ( device, ip, msg );

	long long sid;

	if ( pthread_mutex_lock ( &acceptClientsMutex ) ) {
		CErrID ( "HandleDeviceRegistration: Failed to aquire mutex!" );
		return false;
	}

	/// Assign a session id
	sid = sessionCounter++;
	sid |= ((long long)((client->device->root->id << 16) | client->device->root->projId)) << 32;

	sessions[sid] = client;

	if ( pthread_mutex_unlock ( &acceptClientsMutex ) ) {
		CErrID ( "HandleDeviceRegistration: Failed to release mutex!" );
		return false;
	}

	client->sessionID = sid;

	/// Send session id to client
	MediatorMsg * sessMsg = (MediatorMsg *) &regMsg;
	
	sessMsg->size = sizeof(MediatorMsg);
	sessMsg->cmd0 = MEDIATOR_PROTOCOL_VERSION;
	sessMsg->cmd1 = MEDIATOR_CMD_SESSION_ASSIGN;
	sessMsg->opt0 = MEDIATOR_OPT_NULL;
	sessMsg->opt1 = MEDIATOR_OPT_NULL;
	sessMsg->ids.sessionID = sid;

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
	int				sentBytes;
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
	

	pthread_mutex_lock ( &client->accessMutex );

	sentBytes = (int)sendto ( client->socket, buffer, length, 0, (struct sockaddr *) &client->addr, sizeof(struct sockaddr) );

	pthread_mutex_unlock ( &client->accessMutex );

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

			if ( length >= recvLength ) {
				CErrArg ( "SecureChannelAuth: Invalid length of username [%i]!", length ); break;
			}

			pUI = (unsigned int *) (userName + length + padLen);

			msgLen = *pUI; /// length of the hash
			//*pUI = 0; 
			userName [ length ] = 0;
			pUI++;
					
            /// Look in db for user
			if ( pthread_mutex_lock ( &usersDBMutex ) ) {
				CErr ( "SecureChannelAuth: Failed to aquire mutex on usersDB!" ); break;
			}		

			string user = userName;

			std::transform ( user.begin(), user.end(), user.begin(), ::tolower );

			map<string, string>::iterator iter = usersDB.find ( user );
			if ( iter == usersDB.end () ) {
				CLogArg ( "SecureChannelAuth: User [%s] not found.", userName );
			}
			else {
				pass = iter->second.c_str ();
				client->createAuthToken = true;
			}

			if ( !pass ) {
				CVerbArg ( "SecureChannelAuth: No password for User [%s] available. Treating username as deviceUID and looking for authToken.", userName );
                
				string deviceUID = userName;
                map<string, DeviceMapping *>::iterator devIt = deviceMappings.find ( deviceUID );
                
                if ( devIt != deviceMappings.end ( ) ) {
                    pass = devIt->second->authToken;
					if ( !*pass )
						pass = 0;
                }
			}
            
            if ( pthread_mutex_unlock ( &usersDBMutex ) ) {
                CErr ( "SecureChannelAuth: Failed to release mutex on usersDB!" ); break;
            }
            if ( !pass ) {
                CVerbArg ( "SecureChannelAuth: No username and deviceUID [%s] found.", userName );
                break;
            }
		}

		CVerbVerbArg ( "SecureChannelAuth: Auth user[%s] pass[%s]", userName, ConvertToHexSpaceString ( pass, 64 ) );

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

	if ( client->encrypt ) {
		if ( !AESEncrypt ( &client->aes, (char *)msg, &toSendLen, &cipher ) || !cipher ) {
			CErr ( "SendBuffer: Failed to encrypt AES message." );
			return rc;
		}
		msg = cipher;
	}

	if ( useLock )
		pthread_mutex_lock ( &client->accessMutex );

	rc = (int)sendto ( client->socket, (char *)msg, toSendLen, 0, (struct sockaddr *) &client->addr, sizeof(struct sockaddr) );
	
	if ( useLock )
		pthread_mutex_unlock ( &client->accessMutex );

	if ( cipher )
		free ( cipher );
	if ( rc == (int)toSendLen )
		return msgLen;
	return rc;
}


void MediatorDaemon::HandleSpareSocketRegistration ( ThreadInstance * spareClient, ThreadInstance * orgClient, char * msg, unsigned int msgLen )
{
	int deviceID = *( (int *) (msg + 12) );
		
	CVerbID ( "HandleSpareSocketRegistration" );
		
	int					sock = -1;
	DeviceInstanceList *	device;
	
	if ( pthread_mutex_lock ( &acceptClientsMutex ) ) {
		CErrID ( "HandleSpareSocketRegistration: Failed to aquire mutex!" );
	}
	
	// find the one in our vector
	for ( unsigned i = 0; i < acceptClients.size (); i++ ) {
		if ( acceptClients[i] == spareClient ) {
			CVerbArgID ( "HandleSpareSocketRegistration: Erasing [%i] from client list", i );

			acceptClients.erase ( acceptClients.begin() + i );
			break;
		}
	}

	if ( pthread_mutex_unlock ( &acceptClientsMutex ) ) {
		CErrID ( "HandleSpareSocketRegistration: Failed to release mutex!" );
	}

	// We need at first the deviceID
	if ( !deviceID ) {
		CWarnID ( "HandleSpareSocketRegistration: Invalid device [0] for spare socket mapping!" ); goto Finish;
	}

	/// Verify that the registration is correct
	device	= orgClient->device;
	if ( !device ) {
		CWarnID ( "HandleSpareSocketRegistration: Related client is missing a device instance!" ); goto Finish;
	}
	
	if ( deviceID != device->info.deviceID ){
		CWarnID ( "HandleSpareSocketRegistration: deviceIDs don't match!" );
		goto Finish;
	}	
	
	// Check for matching IP
	if ( spareClient->addr.sin_addr.s_addr != orgClient->addr.sin_addr.s_addr ) {
		CWarnID ( "HandleSpareSocketRegistration: IP address of requestor does not match!" );
		goto Finish;
	}	
	
	// Acquire the mutex on orgClient
	if ( pthread_mutex_lock ( &orgClient->accessMutex ) ) {
		CErrID ( "HandleSpareSocketRegistration: Failed to aquire mutex on org client!" );
	}

	// Apply the port
	sock = orgClient->spareSocket;
	if ( sock != -1 ) {
		shutdown ( sock, 2 );
		closesocket ( sock );
	}
	orgClient->spareSocket = spareClient->socket;
	orgClient->sparePort = ntohs ( spareClient->addr.sin_port );
		
	//pthread_cond_signal ( &orgClient->socketSignal );
	
	// Release the mutex on orgClient
	if ( pthread_mutex_unlock ( &orgClient->accessMutex ) ) {
		CErrID ( "HandleSpareSocketRegistration: Failed to release mutex on org client!" );
	}

	sock = 1;
	
Finish:
	pthread_t thrd = spareClient->threadID;
	if ( pthread_valid ( thrd ) ) {
		pthread_detach_handle ( thrd );
	}
		
	if ( pthread_mutex_destroy ( &spareClient->accessMutex ) ) {
		CErrID ( "HandleSpareSocketRegistration: Failed to destroy mutex!" );
	}

	if ( sock != 1 ) {
		sock = spareClient->socket;
		if ( sock != -1 ) {
			shutdown ( sock, 2 );
			closesocket ( sock );
		}
		CLogID ( "Failed to register spare socket" );
	}
	else {
		CLogArgID ( "successfully registered spare socket on port [%d].", orgClient->sparePort );
	}

	delete spareClient;
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
	
	// look for the project
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

		// Do we have a GCM api key for this project?
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

static long notifyRunner = 0;

void MediatorDaemon::NotifyClientsStart ( unsigned int notify, const char * projectName, const char * appName, int deviceID )
{
	/// If a notification is already running, then spin around, wait and try for max. 5 seconds
	int trys = 26;

	while ( trys > 0 )
	{
		if ( ___sync_val_compare_and_swap ( &notifyRunner, 0, 1 ) == 0 ) {
			trys = 26;
			break;
		}

		Sleep ( 200 );
		trys--;
	}
	
	if ( trys <= 0 ) {
		CWarn ( "NotifyClientsStart: Failed to get a notification slot." );
		return;
	}

	pthread_t threadID;

	this->notify = notify;

	if ( notifyProjectName ) {
		free ( notifyProjectName );
		notifyProjectName = 0;
	}
	if ( notifyAppName ) {
		free ( notifyAppName );
		notifyAppName = 0;
	}

	int s;
	unsigned int len;
	char * name;

	if ( projectName ) {
		len = (unsigned int) strlen ( projectName );
		name = (char *) malloc ( len + 1 );
		if ( !name ) {
			CErr ( "NotifyClientsStart: Failed to allocate memory for projectName." );
			goto Failed;
		}
		memcpy ( name, projectName, len );
		name [ len ] = 0;
		notifyProjectName = name;
	}

	if ( appName ) {
		len = (unsigned int) strlen ( appName );
		name = (char *) malloc ( len + 1 );
		if ( !name ) {
			CErr ( "NotifyClientsStart: Failed to allocate memory for appName." );
			goto Failed;
		}
		memcpy ( name, appName, len );
		name [ len ] = 0;
		notifyAppName = name;
	}

	notifyDeviceID = deviceID;
	
	s = pthread_create ( &threadID, 0, &MediatorDaemon::NotifyClientsStarter, (void *)this );
	if ( s == 0 ) {
		pthread_detach_handle ( threadID );
		return;
	}

	CErr ( "NotifyClientsStart: Error creating thread for notifying..." );

Failed:
	if ( notifyProjectName ) {
		free ( notifyProjectName );
		notifyProjectName = 0;
	}
	if ( notifyAppName ) {
		free ( notifyAppName );
		notifyAppName = 0;
	}

	if ( ___sync_val_compare_and_swap ( &notifyRunner, 1, 0 ) != 1 ) {
		CWarnArg ( "NotifyClientsStart: Something went wrong with modifying the notify runner access lock [%u]", (unsigned int) notifyRunner );
		notifyRunner = 0;
	}
}


void * MediatorDaemon::NotifyClientsStarter ( void * daemon )
{
	if ( !daemon )
		return 0;

	MediatorDaemon * meddae = (MediatorDaemon *)daemon;
	meddae->NotifyClients ( meddae->notify );
	
	if ( ___sync_val_compare_and_swap ( &notifyRunner, 1, 0 ) != 1 ) {
		CWarnArg ( "NotifyClientsStarter: Something went wrong with modifying the notify runner access lock [%u]", (unsigned int) notifyRunner );
		notifyRunner = 0;
	}

	return 0;
}


typedef struct _NotifyContext
{
	unsigned int		deviceID;
	int					sock;
	struct sockaddr		addr;
//	pthread_mutex_t	*	mutex;
}
NotifyContext;


ApplicationDevices * MediatorDaemon::GetApplicationDevices ( const char * projectName, const char * appName )
{
	CVerb ( "GetApplicationDevices" );
	
	if ( !projectName || !appName ) {
		CErr ( "GetApplicationDevices: Called with NULL argument." );
		return 0;
	}
	
	if ( pthread_mutex_lock ( &devicesMutex ) ) {
		CErr ( "GetApplicationDevices: Failed to acquire mutex!" );
		return 0;
	}	

	ApplicationDevices				*	appDevices	= 0;
	ProjectApps						*	projApps	= 0;
	map<string, ApplicationDevices * >::iterator appsIt;
	
	string appsName ( appName );
	string projName ( projectName );

    map<string, ProjectApps * >::iterator projIt = projectsList.find ( projName );	

	if ( projIt == projectsList.end ( ) ) {
		CLogArg ( "GetApplicationDevices: Project [%s] not found.", projectName );
		goto Finish;
	}
	
	projApps = projIt->second;
    
	if ( !projApps ) {
		CLogArg ( "GetApplicationDevices: App [%s] not found.", appName );
		goto Finish;
	}
		
	appsIt = projApps->apps.find ( appsName );
				
	if ( appsIt == projApps->apps.end ( ) ) {
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
	if ( pthread_mutex_unlock ( &devicesMutex ) ) {
		CErr ( "GetApplicationDevices: Failed to release mutex!" );
	}	
	return appDevices;
}


void MediatorDaemon::UnlockApplicationDevices ( ApplicationDevices * appDevices )
{
	__sync_sub_and_fetch ( &appDevices->access, 1 );
}


void MediatorDaemon::NotifyClients ( unsigned int notify )
{
	MediatorNotify	msg;
	Zero ( msg );

	msg.cmd0 = MEDIATOR_PROTOCOL_VERSION;

	msg.cmd1 = MEDIATOR_CMD_MEDIATOR_NOTIFY;
	msg.opt0 = MEDIATOR_OPT_NULL;
	msg.opt1 = MEDIATOR_OPT_NULL;
	msg.msgID = notify;
	msg.notifyDeviceID = notifyDeviceID;

	unsigned int sendSize = sizeof(MediatorNotify);

	if ( notifyProjectName && notifyAppName ) {
		strcpy_s ( msg.projectName, sizeof(msg.projectName), notifyProjectName );
		strcpy_s ( msg.appName, sizeof(msg.appName), notifyAppName );
		sendSize = sizeof(MediatorNotify);
	}
	else sendSize = sizeof(MediatorMsg);

	msg.size = sendSize;
    
	vector<NotifyContext *> dests;
	dests.clear();

	CVerbArg ( "NotifyClients: broadcasting notify [%s]", environs::resolveName(notify) );

#ifdef NDEBUG
	CLogArg ( "NotifyClients: broadcasting notify [0x%X]", notify );
#endif

	ThreadInstance		*	destClient	= 0;
	ApplicationDevices	*	appDevices	= 0;

	if ( pthread_mutex_lock ( &devicesMutex ) ) {
		CVerb ( "NotifyClients: Failed to aquire mutex!" );
		return;
	}    
    
    /// Get the no filter clients
    do
    {
        map<long long, ThreadInstance *>::iterator clientIt = notifyTargets.begin ();
        
        while ( clientIt != notifyTargets.end () )
        {
            destClient = clientIt->second;
            
			NotifyContext * ctx = (NotifyContext *) malloc ( sizeof(NotifyContext) );
			if ( !ctx )
				break;
			ctx->deviceID	= destClient->deviceID;
			ctx->sock		= destClient->socket;
            //			ctx->mutex		= &destClient->accessMutex;
			
			memcpy ( &ctx->addr, &destClient->addr, sizeof(struct sockaddr) );
			dests.push_back ( ctx );
            
            clientIt++;
        }
    }
    while ( 0 );
    
    /// Get the projectApps
    do
    {
		map<string, ApplicationDevices * >::iterator appsIt;
	
		string projName ( notifyProjectName );
			
		/*if ( pthread_mutex_lock ( &devicesMutex ) ) {
            CErr ( "NotifyClients: Failed to acquire mutex!" ); break;
		}*/

		map<string, ProjectApps * >::iterator projIt = projectsList.find ( projName );	

		if ( projIt == projectsList.end ( ) ) {
			CLogArg ( "NotifyClients: Project [%s] not found.", notifyProjectName );	

			/*if ( pthread_mutex_unlock ( &devicesMutex ) ) {
				CErr ( "NotifyClients: Failed to release mutex!" );
			}*/
			break;
		}
        
        ProjectApps * projApps = projIt->second;
        
        map<long long, ThreadInstance *>::iterator clientIt = projApps->notifyTargets.begin ();
        
        while ( clientIt != projApps->notifyTargets.end () )
        {
            destClient = clientIt->second;
            
			NotifyContext * ctx = (NotifyContext *) malloc ( sizeof(NotifyContext) );
			if ( !ctx )
				break;
			ctx->deviceID	= destClient->deviceID;
			ctx->sock		= destClient->socket;
            //			ctx->mutex		= &destClient->accessMutex;
			
			memcpy ( &ctx->addr, &destClient->addr, sizeof(struct sockaddr) );
			dests.push_back ( ctx );
            
            clientIt++;
        }
		/*if ( pthread_mutex_unlock ( &devicesMutex ) ) {
			CErr ( "NotifyClients: Failed to release mutex!" );
		}*/
    }
    while ( 0 );
	
	CVerbVerb ( "NotifyClients: unlock." );
	if ( pthread_mutex_unlock ( &devicesMutex ) ) {
        CErr ( "NotifyClients: Failed to release mutex!" ); goto Finish;
	}
    
	appDevices	= GetApplicationDevices ( notifyProjectName, notifyAppName );
	if ( appDevices )
	{
		unsigned int deviceID;
		DeviceInstanceList * device = appDevices->devices;
		
		if ( pthread_mutex_lock ( &appDevices->mutex ) ) {
			CVerb ( "NotifyClients: Failed to aquire mutex!" );
		}
		else {
			while ( device )
			{		
				destClient = device->client;
				if ( !destClient )
					goto Continue;
		
				deviceID = destClient->deviceID;
				//CLogArg ( "NotifyClients: checking device [0x%X]", deviceID );

				if ( !deviceID || destClient->socket == -1 || destClient->version < '3' )
					goto Continue;

				if ( !IsSocketAlive ( destClient->socket ) )
					goto Continue;

				if ( destClient->filterMode == MEDIATOR_FILTER_NONE && destClient->socket != -1 )
				{
					NotifyContext * ctx = (NotifyContext *) malloc ( sizeof(NotifyContext) );
					if ( !ctx )
						break;
					ctx->deviceID	= destClient->deviceID;
					ctx->sock		= destClient->socket;
		//			ctx->mutex		= &destClient->accessMutex;
			
					memcpy ( &ctx->addr, &destClient->addr, sizeof(struct sockaddr) );
					dests.push_back ( ctx );
				}

			Continue:
				device = device->next;
			}

			CVerbVerb ( "NotifyClients: unlock." );
			if ( pthread_mutex_unlock ( &appDevices->mutex ) ) {
				CErr ( "NotifyClients: Failed to release mutex!" );
			}
		}
		
		UnlockApplicationDevices ( appDevices );		
	}

	
	for ( unsigned i = 0; i < dests.size(); i++ )
	{
		NotifyContext * ctx = dests[i];
		
		if ( ctx ) {
			CLogArg ( "NotifyClients: Notify device [0x%X]", ctx->deviceID );
			sendto ( ctx->sock, (char *)&msg, sendSize, 0, (struct sockaddr *) &ctx->addr, sizeof(struct sockaddr) );
			free ( ctx );
		}
	}
	dests.clear ();
    
Finish:
	CVerbVerb ( "NotifyClients: done." );
}


void * MediatorDaemon::WatchdogThreadStarter ( void * daemon )
{
	if ( !daemon )
		return 0;

	((MediatorDaemon *)daemon)->WatchdogThread ();

	return 0;
}


void MediatorDaemon::WatchdogThread ()
{
	CLog ( "Watchdog started..." );
					
	int						sock;
	ThreadInstance		*	client			= 0;
	const unsigned int		checkDuration	= 1000 * 60 * 2; // 2 min. (in ms)

    
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
    unsigned int timeout = checkDuration;
#else
    struct timeval	now;    
    struct timespec timeout;
#endif
    
	while ( isRunning )
	{	
		CVerbVerb ( "Watchdog: checking..." );
		
		if ( pthread_mutex_lock ( &acceptClientsMutex ) ) {
			CErr ( "Watchdog: Failed to aquire mutex!" ); break;
		}
		
		checkLast++;

		map<long long, ThreadInstance *>::iterator sessionIt = sessions.begin();
		
		while ( sessionIt != sessions.end () ) 
		{
			client = sessionIt->second;
			if ( client ) {
				CVerbArg ( "Watchdog: Checking deviceID [%u]", client->deviceID );

				if ( (checkLast - client->aliveLast) > 3 ) 
				{
					CLogArg ( "Watchdog: Disconnecting [%u] due to expired heartbeat...", client->deviceID );

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

		if ( pthread_mutex_unlock ( &acceptClientsMutex ) ) {
			CErr ( "Watchdog: Failed to release mutex!" ); break;
		}
		
		if ( pthread_mutex_lock ( &thread_mutex ) ) {
			CErr ( "Watchdog: Failed to acquire thread mutex!" ); break;
		}
		
#if defined(_WIN32) && !defined(USE_PTHREADS_FOR_WINDOWS)
#else
		gettimeofday ( &now, NULL );

		timeout.tv_sec = now.tv_sec + (checkDuration / 1000);
		timeout.tv_nsec = now.tv_usec * 1000;
#endif
		pthread_cond_timedwait ( &hWatchdogEvent, &thread_mutex, &timeout );

		if ( pthread_mutex_unlock ( &thread_mutex ) ) {
			CErr ( "Watchdog: Failed to release thrad mutex!" ); break;
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
        printf ( "HTTPPostRequest: Error %d\n", GetLastError() );

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
