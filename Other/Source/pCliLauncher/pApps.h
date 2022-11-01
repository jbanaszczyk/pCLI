/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

#include "../common/common.h"
#include "LauncherVersion.h"

#include "../common/Environment.h"
#include "../common/CtrlHandler.h"

namespace p_apps {
	/******************************************************************************
	 *
	 * Required: VER_PRODUCTNAME_STR
	 *   is defined in version.h
	 *
	 *****************************************************************************/

	/******************************************************************************
	 *
	 * Some handy strings
	 *
	 * LAUNCHER_INI
	 *   Name of launcher INI file
	 *
	 * PORTABLE_APPS
	 *   Directory, where PortableApps paltform is expected
	 *
	 * PORTABLE_APPS_APP
	 *   Legacy name of application directory ( ie. "App" )
	 *
	 * PORTABLE_APPS_APP_LE_32
	 *   Not legacy name of application 32-bit directory ( ie. "App\\32" ) TCC/LE
	 *
	 * PORTABLE_APPS_APP_LE_64
	 *   Not legacy name of application 64-bit directory ( ie. "App\\64" ) TCC/LE
	 *
	 * PORTABLE_APPS_APP_FULL_32
	 *   Not legacy name of application 32-bit directory ( ie. "App\\32" ) TCC
	 *
	 * PORTABLE_APPS_APP_FULL_64
	 *   Not legacy name of application 64-bit directory ( ie. "App\\64" ) TCC
	 *
	 * PORTABLE_APPS_DATA
	 *   Legacy name of data directory ( ie. "Data" )
	 *
	 * PORTABLE_APPS_DEFAULT
	 *   Legacy name of defaults directory ( ie. "App\\DefaultData" )
	 *
	 * PORTABLE_APPS_INI
	 *   Name (relative to PORTABLE_APPS)
	 *
	 * PORTABLE_APPS_OTHER_LOCALE
	 *   Location of launcher' locale files
	 *
	 * LOCATIONS[]
	 *   where proper launcher' directory should be located relative to this exe
	 *
	 *****************************************************************************/
	extern const boost::filesystem::path LAUNCHER_INI;
	extern const boost::filesystem::path PORTABLE_APPS;
	extern const boost::filesystem::path PORTABLE_APPS_APP;
	extern const boost::filesystem::path PORTABLE_APPS_DATA;
	extern const boost::filesystem::path PORTABLE_APPS_DEFAULT;
	extern const boost::filesystem::path PORTABLE_APPS_INI;
	extern const boost::filesystem::path PORTABLE_APPS_OTHER_LOCALE;
	extern const boost::filesystem::path PORTABLE_APPS_DOCUMENTS;
	

	/******************************************************************************
	 *
	 * If compiled with _DEBUG
	 *   you can your own value of argv0
	 *
	 *****************************************************************************/

	// extern const std::tstring envArgv0Name;

	/******************************************************************************
	 *
	 * tstring2string
	 * string2tstring
	 *   Simple conversions tstring / string
	 *
	 *****************************************************************************/
	auto tstring2string(const std::tstring& sou) -> std::string;
	auto string2tstring(const std::string& sou) -> std::tstring;

	/******************************************************************************
	 *
	 * copyCopy
	 *   like boost::filesystem::copy, but if source is directory - copies whole tree
	 *
	 *****************************************************************************/
	auto copyCopy(const boost::filesystem::path& source, const boost::filesystem::path& destination, boost::system::error_code& ec) -> bool;

	/******************************************************************************
	 *
	 * unquote
	 *   remove existing quotes from the string (probably path)
	 *
	 *****************************************************************************/
	auto unquote(const std::tstring& str) -> std::tstring;

	/******************************************************************************
	 *
	 * quote
	 *   quote path containing spaces
	 *
	 *****************************************************************************/
	auto quote(std::tstring str) -> std::tstring;
	auto quote(const boost::filesystem::path& str) -> std::tstring;
	auto normalize(const boost::filesystem::path& str) -> std::tstring;

	/******************************************************************************
	 *
	 * getComputerName
	 *   retrieve NetBIOS computer name
	 *
	 *****************************************************************************/
	auto getComputerName() -> std::tstring;

	/******************************************************************************
	 *
	 * getUserName
	 *   retrieve SamCompatible user name. Something like Engineering\JSmith
	 *
	 *****************************************************************************/
	auto getUserName() -> std::tstring;

	/******************************************************************************
	*
	* getDomainName
	*   compatible with getUserName
	*
	*****************************************************************************/
	auto getDomainName() -> std::tstring;

	/******************************************************************************
	*
	* pathToUnc
	*   Convert path to UNC
	*
	*****************************************************************************/
	auto pathToUnc(boost::filesystem::path netPath) -> std::tstring;

	/******************************************************************************
	 *
	 * makeDirWriteable
	 *   creates (if required) and check, if directory is writeable
	 *
	 *****************************************************************************/
	auto makeDirWriteable(boost::filesystem::path dir) -> bool;


	/******************************************************************************
	 *
	 * abend
	 *   abnormal end of the thread
	 *     shows message (can be translated) vi cerr or message box
	 *     and exits.
	 *	   adds to the pool of boost::locale::translate'd strings:
	 *       _T( "Press [ENTER] key to exit." )
	 *       _T( "Cann't to continue." )
	 *
	 *****************************************************************************/
	auto abend(boost::_tformat msg, int errCode) -> void;

	/******************************************************************************
	 *
	 * errnoMsg
	 *   wrapper for _tcserror_s
	 *   clears errno
	 *
	 *****************************************************************************/
	auto errnoMsg() -> std::tstring;

	/******************************************************************************
	 *
	 * lastErrorMsg
	 *   LastError as a std::_tstring
	 *   clears LastError
	 *
	 *****************************************************************************/
	auto lastErrorMsg() -> std::tstring;

	/******************************************************************************
	 *
	 * execute
	 *   like _texecve, but ComEmu / parent cmd aware
	 * returns:
	 *   boost::undef
	 *
	 *****************************************************************************/
	enum class tpWait {
		pWait_Auto,
		pWait_Wait,
		pWait_NoWait
	};

	auto execute(tpWait pWait, const std::tstring& cmdName, const std::vector<std::tstring>& cmdLine, const Environment& cmdEnvironment, boost::filesystem::path cwd) -> boost::optional<DWORD>;

}
