/**
* Dirent wrapper that includes either dirent.h (POSIX)
* or provides wrapping around win32
* --------------------------------------------------------------------
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

#include <direntw.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>


struct DIR
{
	char                *	pattern;

	intptr_t				hFind;
	struct _finddata_t		dFind;

	struct dirent			item;
};


DIR * opendir ( const char * name )
{
	if ( !name || !*name )
		return 0;

	bool success = false;
	DIR * dir = 0;

	size_t length = strlen ( name );

	dir = (DIR *) calloc ( 1, sizeof ( DIR ) );
	if ( dir ) 
	{
		dir->pattern = (char *) malloc ( length + 4 );
		if ( dir->pattern )
		{
			memcpy ( dir->pattern, name, length );

			char last = dir->pattern [length - 1];

			if ( last != '/' && last != '\\' )
			{
				dir->pattern [length] = '/'; length++;
			}
			dir->pattern [length] = '*';
			dir->pattern [length + 1] = 0;

			dir->hFind = _findfirst ( dir->pattern, &dir->dFind );
			if ( dir->hFind != -1 )
			{
				memset ( &dir->item, 0, sizeof ( dir->item ) );
				success = true;
			}
		}
	}

	if ( !success && dir ) {
		if ( dir->pattern )
			free ( dir->pattern );
		free ( dir );
		dir = 0;
	}
	return dir;
}


int closedir ( DIR * dir )
{
	int success = -1;

	if ( dir )
	{
		if ( dir->hFind != -1 )
			success = _findclose ( dir->hFind );

		if ( dir->pattern )
			free ( dir->pattern );
		free ( dir );
	}

	return success;
}


struct dirent * readdir ( DIR * dir )
{
	if ( !dir || dir->hFind == -1 )
		return 0;

	struct dirent * nextDir = 0;

	if ( !dir->item.d_name || _findnext ( dir->hFind, &dir->dFind ) != -1 )
	{
		nextDir         = &dir->item;
		nextDir->d_name = dir->dFind.name;

		if ( dir->dFind.attrib & _A_SUBDIR )
			nextDir->d_type = DT_DIR;
		else
			nextDir->d_type = DT_REG;
	}

	return nextDir;
}

