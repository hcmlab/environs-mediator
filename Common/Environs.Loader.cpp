/**
 * Environs Loader
 * Loads the Environs library and creates an instance of Environs.
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
#endif

#include "Environs.h"
#include "Environs.Native.h"

#include <string>

#ifdef _WIN32
# include <direct.h>
#endif

using namespace std;
using namespace environs::lib;


#define CLASS_NAME	"Environs.Loader. . . . ."


namespace environs
{
	namespace lib
	{
		/// C function implemented in the loader (statically build into each app)
		/*sp ( IEnvirons ) ENVIRONS_CreateInstance ()
		{
			sp ( IEnvirons ) obj ( (IEnvirons *) ENVIRONS_CreateInstance1 ( ENVIRONS_BUILD_CRT ) );
			return obj;
		}
		*/

        
		/// C function implemented in the loader (statically build into each app)
		/*void * CallConv ENVIRONS_CreateInstanceObject ()
		{
			return ENVIRONS_CreateInstance1 ( ENVIRONS_BUILD_CRT );
		}
		*/


		void EnvironsDisposer ( IEnvironsDispose * obj )
        {
			if ( obj ) ((IEnvironsDispose *)obj)->Release ();
		}

        
		namespace Loader
		{
			/// Forward declarations
			IEnvirons * LocateLoadEnvirons ( COBSTR module, int crt );

			HMODULE		g_EnvironsModuleHandle	= 0;

            
            /**
             * Create an Environs object.
             *
             * @return   An Environs object wrapped into a smart pointer.
             */
			sp ( IEnvirons ) ENVIRONS_CreateInstance ()
			{
				sp ( IEnvirons ) obj ( LocateLoadEnvirons ( "Environs", ENVIRONS_BUILD_CRT ) );
				return obj;
			}

            
            /**
             * Create an Environs object.
             *
             * @param 	appName		The application name for the application environment.
             * @param  	areaName	The area name for the application environment.
             *
             * @return   An Environs object wrapped into a smart pointer.
             */
			sp ( IEnvirons ) ENVIRONS_CreateInstance ( const char * appName, const char * areaName )
			{
				sp ( IEnvirons ) obj ( LocateLoadEnvirons ( "Environs", ENVIRONS_BUILD_CRT ) );
				if ( obj )
					obj->LoadSettings ( appName, areaName );
				return obj;
			}

            
            /**
             * Create an Environs object.
             *
             * @return   An Environs object interface.
             */
			IEnvirons * ENVIRONS_CreateInstanceObject ()
			{
				/// Load Environs.dll and create an instance
				return LocateLoadEnvirons ( "Environs", ENVIRONS_BUILD_CRT );
			}

            
            /**
             * Create an Environs object.
             *
             * @param 	appName		The application name for the application environment.
             * @param  	areaName	The area name for the application environment.
             *
             * @return   An Environs object interface.
             */
			IEnvirons * ENVIRONS_CreateInstanceObject ( const char * appName, const char * areaName )
			{
				/// Load Environs.dll and create an instance
				IEnvirons * env = LocateLoadEnvirons ( "Environs", ENVIRONS_BUILD_CRT );
				if ( env )
					env->LoadSettings ( appName, areaName );
				return env;
			}


			IEnvirons * LocateLoadEnvirons ( COBSTR module, int crt )
			{
				CVerbN ( "LocateLoadEnvirons" );
				
				char absPath [1024];

				HMODULE hModLib = g_EnvironsModuleHandle;
				if ( !hModLib )
				{
					sprintf ( absPath, ENVLIBPREFIX "%s" LIBEXTENSION, module );

					hModLib = dlopen ( module, RTLD_LAZY );
					if ( !hModLib )
					{
						CVerbVerbN ( "LocateLoadEnvirons: Not found in system search path." );

						do
						{
							sprintf ( absPath, "./" ENVLIBPREFIX "%s" LIBEXTENSION, module );

							hModLib = dlopen ( absPath, RTLD_LAZY );
							if ( hModLib )
								break;
							CVerbVerbN ( "LocateLoadEnvirons: Not found in system search path [./]." );

							sprintf ( absPath, "./" LIBNAME_EXT_DIR "/" ENVLIBPREFIX "%s" LIBEXTENSION, module );

							hModLib = dlopen ( absPath, RTLD_LAZY );
							if ( hModLib )
								break;
#ifdef _WIN32
							sprintf ( absPath, LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/%s" LIBEXTENSION, module );

							hModLib = dlopen ( absPath, RTLD_LAZY );
							if ( hModLib )
								break;
							CVerbVerbN ( "LocateLoadEnvirons: Not found in toolset search path [" LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/]" );

							if ( !_getcwd ( absPath, 1024 ) )
								break;

							int pos = (int)strlen ( absPath );
							if ( pos <= 0 )
								break;

							sprintf ( absPath + pos, "/" LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/%s" LIBEXTENSION, module );

							CVerbN ( "LocateLoadEnvirons: Trying [/" LIBNAME_EXT_DIR "/" ENVIRONS_TSDIR "/]" );

							hModLib = dlopen ( absPath, RTLD_LAZY );
							if ( hModLib )
								break;
							CVerbVerbN ( "LocateLoadEnvirons: Not found in toolset working directory path." );
#else
							sprintf ( absPath, LIBNAME_EXT_DIR "/" ENVLIBPREFIX "%s" LIBEXTENSION, module );

							hModLib = dlopen ( absPath, RTLD_LAZY );
							if ( hModLib )
								break;
#endif
							if ( !hModLib ) {
#ifdef _WIN32
								CLogN ( "LocateLoadEnvirons: Not found." );
#else
								CLogArgN ( "LocateLoadEnvirons: [%s]", dlerror () );
#endif
							}
						} 
						while ( 0 );
					}
				}

				if ( !hModLib ) {
					CErrN ( "LocateLoadEnvirons: Cannot find Environs library." );
					return 0;
				}

				ENVIRONS_PP ( ENVIRONS_CreateInstance1 ) pCreate = 0;

				pCreate = (ENVIRONS_PP ( ENVIRONS_CreateInstance1 ))
					dlsym ( hModLib, ENVIRONS_TOSTRING ( ENVIRONS_CreateInstance1 ) );

				if ( pCreate ) {
					g_EnvironsModuleHandle = hModLib;

					CVerbN ( "LocateLoadEnvirons: Creating Instance." );

					return (IEnvirons *) pCreate ( ENVIRONS_BUILD_CRT );
				}
					
				CErrN ( "LocateLoadEnvirons: Cannot find " ENVIRONS_TOSTRING ( ENVIRONS_CreateInstance1 ) " in Environs library." );

				dlclose ( hModLib );

				g_EnvironsModuleHandle = 0;
				return 0;
			}
		}
	}
}




