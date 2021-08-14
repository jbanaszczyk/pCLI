/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

#include "../common/common.h"

// Some strings

namespace p_apps {

	extern const std::tstring PORTABLE_APPS;
	extern const std::tstring PORTABLE_APPS_INI;
	extern const std::tstring locations[4];

	// Tools

	// convert tstring to string
	std::string tstring2string ( const std::tstring sou );

}
