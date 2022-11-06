// *************************************************************
// * Copyright 2011 jacek.banaszczyk@gmail.com                 *
// * Part of pCli project: https://github.com/jbanaszczyk/pCli *
// *************************************************************

#include "stdafx.h"
#include "pAppsUtils.h"

namespace p_apps {

	void imbueIO() {
		std::cout.imbue(std::locale());
		std::cerr.imbue(std::locale());
		std::cin.imbue(std::locale());
		//std::clog.imbue(std::locale());
#ifdef _UNICODE
		(void)_setmode(_fileno(stdout), _O_WTEXT);
		(void)_setmode(_fileno(stdin), _O_WTEXT);
		(void)_setmode(_fileno(stderr), _O_WTEXT);

#endif
	}

}
