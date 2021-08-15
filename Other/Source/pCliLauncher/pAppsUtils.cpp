/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#include "stdafx.h"

#include "pAppsUtils.h"
#include "LauncherVersion.h"

namespace p_apps {

	/*****************************************************/

	auto tstring2string(const std::tstring sou) -> std::string {
#ifdef _UNICODE
		size_t pReturnValue;
		mbstate_t mbstate;
		auto wstr = sou.c_str();
		auto err = wcsrtombs_s(&pReturnValue, nullptr, 0, &wstr, sou.length(), &mbstate);
		if (err){
			return "";
		}
		auto chars = new(std::nothrow) char[ pReturnValue ];
		if (chars == nullptr){
			return "";
		}
		err = wcsrtombs_s(&pReturnValue, chars, pReturnValue, &wstr, _TRUNCATE, &mbstate);
		if (err){
			*chars = '\0';
		}
		return chars;
#else
		return sou;
#endif
	}
}
