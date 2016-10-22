/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#ifndef _ADA9F1AC_71AC_4B94_8384_F6C61C6D74E4_INCLUDED
#define _ADA9F1AC_71AC_4B94_8384_F6C61C6D74E4_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif

#include "../common/common.h"
#include "LauncherVersion.h"

#include <tchar.h>
#include <map>
#include <tlhelp32.h>
#include <psapi.h>
#include <functional>
#include <utility>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <Winternl.h>
#include <Shellapi.h>

#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

// Some strings

namespace pApps {

	extern const std::tstring PORTABLE_APPS;
	extern const std::tstring PORTABLE_APPS_INI;
	extern const std::tstring locations[4];

	// Tools

	// convert tstring to string
	std::string tstring2string ( const std::tstring sou );

}

#endif  // _ADA9F1AC_71AC_4B94_8384_F6C61C6D74E4_INCLUDED
