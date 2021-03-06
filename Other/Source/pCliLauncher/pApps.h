/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

#include "../common/common.h"
#include "LauncherVersion.h"

#include "../common/Environment.h"
#include "../common/CtrlHandler.h"
#include <boost/filesystem/path.hpp>

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
	 *   Directory, where PortableApps platform is expected
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

	// extern const std::tstring LOCATIONS[4];

	/******************************************************************************
	 *
	 * getArgv0
	 *   Retrieve argv[0] for the current process:
	 *   order:
	 *     real process executable name (from process snapshot)
	 *     argv[0]
	 *
	 *****************************************************************************/
	boost::filesystem::path getArgv0(const TCHAR* const argv[]);

	/******************************************************************************
	 *
	 * findPAppsDir
	 *   Find standard launcher' directory (where launcher ini exist)
	 *   You will get something like <ROOT>\PortableApps\TccLePortable
	 *
	 *   Launcher executable can be exexcuted from any of
	 *     <ROOT>
	 *     <ROOT>\PortableApps
	 *     <ROOT>\PortableApps\TccLePortable
	 *     <ROOT>\PortableApps\TccLePortable\Other
	 *   In all cases findPAppsDir will point to <ROOT>\PortableApps\TccLePortable
	 *
	 *****************************************************************************/
	boost::optional<boost::filesystem::path> findPAppsDir( const boost::filesystem::path& exePath, const boost::filesystem::path& argv0, p_apps::Environment& mEnv );

	/******************************************************************************
	*
	* env_PAppsC
	*   setup environment as PortableApps platform would set
	*   PortableApps platform, if running sets up many env vars named PortableApps.com*
	*   env_PAppsC tries to setup them even if PortableApps platform was not started
	*
	*****************************************************************************/
	void env_PAppsC(const boost::optional<boost::filesystem::path>& exePath, p_apps::Environment& mEnv);

	/******************************************************************************
	 *
	 * copyCopy
	 *   like boost::filesystem::copy, but if source is directory - copies whole tree
	 *
	 *****************************************************************************/
	bool copyCopy( boost::filesystem::path const& source, boost::filesystem::path const& destination, boost::system::error_code& ec );

	/******************************************************************************
	 *
	 * unquote
	 *   remove existing quotes from the string (probably path)
	 *
	 *****************************************************************************/
	std::tstring unquote( const std::tstring& str );

	/******************************************************************************
	 *
	 * quote
	 *   quote path containing spaces
	 *
	 *****************************************************************************/
	std::tstring quote( std::tstring str );
	std::tstring quote(const boost::filesystem::path& str);
	std::tstring normalize(const boost::filesystem::path& str);

	/******************************************************************************
	 *
	 * getComputerName
	 *   retrieve NetBIOS computer name
	 *
	 *****************************************************************************/
	std::tstring getComputerName();

	/******************************************************************************
	 *
	 * getUserName
	 *   retrieve SamCompatible user name. Something like Engineering\JSmith
	 *
	 *****************************************************************************/
	std::tstring getUserName();

	/******************************************************************************
	*
	* getDomainName
	*   compatible with getUserName
	*
	*****************************************************************************/
	std::tstring getDomainName();

	/******************************************************************************
	*
	* pathToUnc
	*   Convert path to UNC
	*
	*****************************************************************************/
	std::tstring pathToUnc(const boost::filesystem::path netPath);

	/******************************************************************************
	 *
	 * makeDirWriteable
	 *   creates (if required) and check, if directory is writeable
	 *
	 *****************************************************************************/
	bool makeDirWriteable( const boost::filesystem::path dir );

	/******************************************************************************
	*
	* imbueIO
	*   made stdin/stdout/stderr locale-aware
	*
	*****************************************************************************/
	void imbueIO();

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
	void abend( const boost::_tformat msg, int errCode );

	/******************************************************************************
	 *
	 * errnoMsg
	 *   wrapper for _tcserror_s
	 *   clears errno
	 *
	 *****************************************************************************/
	std::tstring errnoMsg();

	/******************************************************************************
	 *
	 * lastErrorMsg
	 *   LastError as a std::_tstring
	 *   clears LastError
	 *
	 *****************************************************************************/
	std::tstring lastErrorMsg();

	/******************************************************************************
	 *
	 * launch
	 *   like _texecve, but ComEmu / parent cmd aware
	 * returns:
	 *   boost::undef
	 *
	 *****************************************************************************/
	enum class tpWait { pWait_Auto, pWait_Wait, pWait_NoWait };

	boost::optional<DWORD> launch( tpWait pWait, const std::tstring& cmdName, const std::vector<std::tstring>& cmdLine, const p_apps::Environment& cmdEnvironment, boost::filesystem::path cwd );

}
