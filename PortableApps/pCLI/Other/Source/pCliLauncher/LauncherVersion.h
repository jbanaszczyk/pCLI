/*****************************************************************************
 *                                                                           
 * Copyright 2013 Jacek.Banaszczyk@gmail.com                                 
 * part of PortableApps Launcher project                                      
 * http://sourceforge.net/projects/tccleportable/                            
 * Release 1.0.0                                                             
 *                                                                           
 *****************************************************************************/

#ifndef _E334A8BE_A9B3_4D8F_8990_D1DDC7F93AA5_INCLUDED
#define _E334A8BE_A9B3_4D8F_8990_D1DDC7F93AA5_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif

/*****************************************************************************
 *
 * version.h is included as #include "version.h"
 *   it should be added to "Additional Include Directories"
 *
 * Symbols defined there are widely used:
 *   All of them influence on Version resource
 *   Definition
 *     VER_TARGETNAME_STR=$(TargetFileName);
 *     should be added to both ClCompile/PreprocessorDefinitions and
 *     ResourceCompile/PreprocessorDefinitions
 *   VER_PRODUCTNAME_STR is widely used to identify
 *     locale translations files, launcher file itself
 *
 *****************************************************************************/

#define VERSION_MAJOR             1
#define VERSION_MINOR             0
#define VERSION_REVISION          0
#define VERSION_BUILD             1
#define VER_PRODUCTNAME_STR      "TccLePortable"
#define VER_COPYRIGHT_STR        "(C)2013 Jacek Banaszczyk"
#define VER_FILE_COMPANY_STR     "Jacek Banaszczyk"
#define VER_FILE_DESCRIPTION_STR "TCC/LE Portable Launcher"
#define VER_TRADEMARKS_STR       "PortableApps.com is a Trademark of Rare Ideas, LLC."
#define VER_URL_STR              "http://sourceforge.net/projects/tccleportable"

#ifndef VER_TARGETNAME_STR
	#pragma message( "RC_Warning: Probably VER_TARGETNAME_STR is not defined, #define VER_TARGETNAME_STR=$(TargetFileName) for both resources and source code." )
#endif

#ifndef VER_TARGETNAME_STR
	#define VER_FILENAME_STR            VER_PRODUCTNAME_STR _T_(".exe")
#else
	#define VER_FILENAME_STR            STRINGIZATOR(VER_TARGETNAME_STR)
#endif

#endif
