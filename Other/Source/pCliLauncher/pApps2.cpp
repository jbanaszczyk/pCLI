/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#include "stdafx.h"

#include "./pApps2.h"
#include "LauncherVersion.h"

namespace p_apps {

	static const std::tstring PORTABLE_APPS;
	static const std::tstring PORTABLE_APPS_INI;
	static const std::tstring locations[] = {
		_T ( "..\\" ),
		_T ( ".\\" ),
		_T ( VER_PRODUCTNAME_STR ),
		PORTABLE_APPS + _T ( "\\" ) + _T ( VER_PRODUCTNAME_STR ),
	};

	/*****************************************************/

	std::string tstring2string ( const std::tstring sou ) {
#ifdef _UNICODE
		size_t pReturnValue;
		mbstate_t mbstate;
		const TCHAR *wstr = sou.c_str();
		errno_t err = wcsrtombs_s ( &pReturnValue, NULL, 0, &wstr, sou.length(), &mbstate );
		if ( err )
			return "";
		char *chars = new ( std::nothrow ) char[ pReturnValue ];
		if (chars  == nullptr )
			return "";
		err = wcsrtombs_s ( &pReturnValue, chars, pReturnValue, &wstr, _TRUNCATE , &mbstate );
		if ( err )
			*chars = '\0';
		return chars;
#else
		return sou;
#endif
	}

}