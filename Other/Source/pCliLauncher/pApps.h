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

namespace p_apps {

	std::filesystem::path canonical(const std::filesystem::path& p, const std::filesystem::path& base);

	std::wstring string2wstring(const std::string& str);

	std::vector<std::wstring> tokenize(const std::wstring& str);

	/****************************************************************************
	 * \brief  check if string is already quoted
	 *
	 * \param  str string to be checked
	 * \return true if string is surrounded with ""
	 ***************************************************************************/
	static bool needsQuotation(const std::wstring& str);

	/****************************************************************************
	 * \brief  remove quotation marks
	 *
	 * \param  str string to be unquoted
	 * \return unquoted string
	 * \detail escape character is '^' - tcc escape char
	 ***************************************************************************/
	std::wstring unquote(const std::wstring& str);

	std::wstring quote(const std::optional<std::wstring>& str);

	/****************************************************************************
	 * \brief  quote string if it contains spaces
	 *
	 * \param  str string to be quoted
	 * \return string surrounded with "" if it is required else the same string
	 ***************************************************************************/
	std::wstring quote(const std::wstring& str);

	/****************************************************************************
	 * \brief  quote filename if it contains spaces
	 *
	 * \param  fileName string to be quoted
	 * \return string surrounded with "" if it is required else the same string
	 * \detail please note, that `"` in filename is illegal using windows
	 ***************************************************************************/
	std::wstring quote(const std::filesystem::path& fileName);

	/******************************************************************************
	 *
	 * getComputerName
	 *   retrieve NetBIOS computer name
	 *
	 *****************************************************************************/
	std::wstring getComputerName();

	/******************************************************************************
	 *
	 * getUserName
	 *   retrieve SamCompatible user name. Something like Engineering\JSmith
	 *
	 *****************************************************************************/
	std::wstring getUserName();

	/******************************************************************************
	*
	* getDomainName
	*   compatible with getUserName
	*
	*****************************************************************************/
	std::wstring getDomainName();

	/******************************************************************************
	*
	* pathToUnc
	*   Convert path to UNC
	*
	*****************************************************************************/
	std::wstring pathToUnc(const std::filesystem::path& netPath);

	/****************************************************************************
	 * \brief  create directory if doesn't exists, check if is writeable
	 *
	 * \param  directory to be created
	 ***************************************************************************/
	void makeDirWriteable(const std::filesystem::path& directory);

	/******************************************************************************
	 *
	 * errnoMsg
	 *   wrapper for _tcserror_s
	 *   clears errno
	 *
	 *****************************************************************************/
	std::wstring errnoMsg();

	/******************************************************************************
	 *
	 * lastErrorMsg
	 *   LastError as a std::wstring
	 *   clears LastError
	 *
	 *****************************************************************************/
	std::wstring lastErrorMsg();

	/******************************************************************************
	 *
	 * execute
	 *   like _texecve, but ComEmu / parent cmd aware
	 * returns:
	 *   boost::undef
	 *
	 *****************************************************************************/

	void execute(bool pWait, const std::wstring& cmdName, const std::vector<std::wstring>& cmdLine,
	             const Environment& cmdEnvironment, const std::filesystem::path& cwd);
	
	/******************************************************************************
	*
	* imbueIO
	*   made stdin/stdout/stderr locale-aware
	*
	*****************************************************************************/
	// ReSharper disable once CppInconsistentNaming
	void imbueIO();

}
