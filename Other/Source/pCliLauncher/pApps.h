/*************************************************************************/ /**
 * \file      pApps.cpp
 * \brief     Some commonly used functions
 *
 * \copyright (c) 2011 jacek.banaszczyk@gmail.com
 * \short     Part of pCli project: https://github.com/jbanaszczyk/pCli
 ****************************************************************************/

#pragma once

#include "../common/common.h"
#include "LauncherVersion.h"

#include "../common/Environment.h"
#include "../common/CtrlHandler.h"

namespace p_apps {
	/*****************************************************************************
	 * \details Required: VER_PRODUCTNAME_STR is defined in version.h
	 ****************************************************************************/

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
	 *   Not legacy name of application 32-bit directory ( ie. "App\\32" )
	 *
	 * PORTABLE_APPS_APP_LE_64
	 *   Not legacy name of application 64-bit directory ( ie. "App\\64" )
	 *
	 * PORTABLE_APPS_DATA
	 *   Legacy name of data directory ( ie. "Data" )
	 *
	 * PORTABLE_APPS_DEFAULT
	 *   Legacy name of defaults directory ( ie. "App\\DefaultData" )
	 *
	 *****************************************************************************/
	extern const std::filesystem::path LAUNCHER_INI;
	extern const std::filesystem::path PORTABLE_APPS;
	extern const std::filesystem::path PORTABLE_APPS_APP;
	extern const std::filesystem::path PORTABLE_APPS_DATA;
	extern const std::filesystem::path PORTABLE_APPS_DEFAULT_DATA;

	std::filesystem::path canonical(const std::filesystem::path& p, const std::filesystem::path& base);

	std::wstring string2wstring(const std::string& str);

	std::vector<std::tstring> tokenize(const std::tstring& str);

	/****************************************************************************
	 * \brief  check if string is already quoted
	 *
	 * \param  str string to be checked
	 * \return true if string is surrounded with ""
	 ***************************************************************************/
	static bool needsQuotation(const std::tstring& str);

	/****************************************************************************
	 * \brief  remove quotation marks
	 *
	 * \param  str string to be unquoted
	 * \return unquoted string
	 * \detail escape character is '^' - tcc escape char
	 ***************************************************************************/
	std::tstring unquote(const std::tstring& str);

	/****************************************************************************
	 * \brief  quote string if it contains spaces
	 *
	 * \param  str string to be quoted
	 * \return string surrounded with "" if it is required else the same string
	 ***************************************************************************/
	std::tstring quote(const std::tstring& str);

	/****************************************************************************
	 * \brief  quote filename if it contains spaces
	 *
	 * \param  fileName string to be quoted
	 * \return string surrounded with "" if it is required else the same string
	 * \detail please note, that `"` in filename is illegal using windows
	 ***************************************************************************/
	std::tstring quote(const std::filesystem::path& fileName);

	/****************************************************************************
	 * \brief  "normalize" filesystem path - sanitize, resolve links, canonical
	 *
	 * \param  fileName filesystem path to be cleaned
	 * \param  quoted add quotation marks if needed
	 * \return clean filesystem path
	 ***************************************************************************/
	std::tstring normalize(const std::filesystem::path& fileName, bool quoted = true);

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
	std::tstring pathToUnc(const std::filesystem::path& netPath);

	/****************************************************************************
	 * \brief  create directory if doesn't exists, check if is writeable
	 *
	 * \param  directory to be created
	 ***************************************************************************/
	void makeDirWriteable(const std::filesystem::path& directory);

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
	void abend(const boost::_tformat& msg, int errCode);

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

	boost::optional<DWORD> execute(tpWait pWait, const std::tstring& cmdName, const std::vector<std::tstring>& cmdLine,
	                               const Environment& cmdEnvironment, const std::filesystem::path& cwd);


}
