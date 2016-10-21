/*****************************************************************************
 *                                                                           
 * Copyright 2013 Jacek.Banaszczyk@gmail.com                                 
 * part of PortableApps Launcher project                                      
 * http://sourceforge.net/projects/tccleportable/                            
 * Release 1.0.0                                                             
 *                                                                           
 *****************************************************************************/

#include "LauncherVersion.h" 

#ifdef _UNICODE
	#define _T_(x)      L ## x
#else
	#define _T_(x)      x
#endif

#define STRINGIZATOR2(s) _T_(#s)
#define STRINGIZATOR(s) STRINGIZATOR2(s)

#define VER_PRODUCT_VERSION         VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define VER_PRODUCT_VERSION_STR     STRINGIZATOR(VERSION_MAJOR)             \
                                    _T_(".") STRINGIZATOR(VERSION_MINOR)    \
                                    _T_(".") STRINGIZATOR(VERSION_REVISION) \
                                    _T_(".") STRINGIZATOR(VERSION_BUILD)

#ifdef _DEBUG
	#define VER_FILEFLAGS           VS_FF_DEBUG
#else
	#define VER_FILEFLAGS           0x0L
#endif
