/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#include "stdafx.h"
#include "./pApps.h"
#include "../common/SysPidInfo.h"
#include "../common/IniFile.h"

#include <windows.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <boost/filesystem/operations.hpp>

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
	static const boost::filesystem::path LAUNCHER_INI(_T(VER_PRODUCTNAME_STR) _T(".ini"));

	static const boost::filesystem::path PORTABLE_APPS_APP = _T("App");

	static const boost::filesystem::path PORTABLE_APPS_DEFAULT = _T("App\\DefaultData");

	static const boost::filesystem::path PORTABLE_APPS_DATA = _T("Data");

	static const boost::filesystem::path PORTABLE_APPS = _T("PortableApps");
	static const boost::filesystem::path PORTABLE_APPS_INI = _T("Data\\PortableAppsMenu.ini");

	static const boost::filesystem::path PORTABLE_APPS_DOCUMENTS = _T("Documents");

	static const boost::filesystem::path LOCATIONS[] = {
		_T(VER_PRODUCTNAME_STR),
		_T("."),
		PORTABLE_APPS / _T(VER_PRODUCTNAME_STR),
		_T("..")
	};

	static const boost::filesystem::path PORTABLE_APPS_OTHER_LOCALE = _T("Other\\Locale");

	/******************************************************************************
	 *
	 * getArgv0
	 *   Retrieve argv[0] for the current process:
	 *   order:
	 *     real process executable name (from process snapshot)
	 *     argv[0]
	 *
	 *****************************************************************************/
    boost::filesystem::path getArgv0(const TCHAR* const argv[], IEnvironment& mEnv) {
#ifdef _DEBUG
		if (mEnv.exists(ENV_ARGV0_NAME)) {
            const boost::filesystem::path argv0 = mEnv.get(ENV_ARGV0_NAME);
			mEnv.erase(ENV_ARGV0_NAME);
			return boost::filesystem::absolute(argv0);
		}
#endif		
		return boost::filesystem::absolute(p_apps::sysPidInfo->getExeName(argv[0]));
	}

	/******************************************************************************
	 *
	 * findPAppsDir
	 *   Find standard launcher' directory (where launcher ini exist)
	 *   You will get something like <ROOT>\PortableApps\TccLePortable
	 *
	 *   Launcher executable can be executed from any of
	 *     <ROOT>
	 *     <ROOT>\PortableApps
	 *     <ROOT>\PortableApps\TccLePortable
	 *     <ROOT>\PortableApps\TccLePortable\Other
	 *   In all cases findPAppsDir will point to <ROOT>\PortableApps\TccLePortable
	 *
	 *****************************************************************************/
	boost::optional<boost::filesystem::path> findPAppsDir(const boost::filesystem::path& exePath, const boost::filesystem::path& argv0, Environment& mEnv) {
		boost::filesystem::path pAppsDir(argv0);
		if (pAppsDir.has_filename()) {
			pAppsDir = pAppsDir.parent_path();
		}	// strip filename
		bool pAppsDirValid = false;
		for (size_t idx = 0; _countof(LOCATIONS) > idx; ++idx) {
			boost::system::error_code errCode;
			if (boost::filesystem::exists(pAppsDir / LOCATIONS[idx] / exePath, errCode)) {
				pAppsDir /= LOCATIONS[idx];
				pAppsDirValid = true;
				break;
			}
		}
		if (!pAppsDirValid) {
			return boost::none;
		}
		boost::system::error_code errCode;
		pAppsDir = boost::filesystem::absolute(pAppsDir);
		current_path(pAppsDir, errCode);
		boost::filesystem::initial_path(errCode);
		return pAppsDir;
	}

	/******************************************************************************
	*
	* env_PAppsC
	*   setup environment as PortableApps platform would set
	*   PortableApps platform, if running sets up many env vars named PortableApps.com*
	*   env_PAppsC tries to setup them even if PortableApps platform was not started
	*
	*****************************************************************************/
	void env_PAppsC(const boost::optional<boost::filesystem::path>& exePath, p_apps::Environment& mEnv) {
		if (mEnv.exists(_T("PortableApps.comLanguageName")) || mEnv.exists(_T("PortableApps.comLanguageName_INTERNAL"))) {
			return;						// executed from pApps platform, nothing to do
		}
		if (!exePath) {
			return;
		}

		boost::filesystem::path platformIni = exePath.get() / _T("..\\PortableApps.com\\Data\\PortableAppsMenu.ini");
		boost::system::error_code errCode;
		if (!boost::filesystem::exists(platformIni, errCode)) {
			return;
		}

		// executed standalone, but PortableApps platform can be found in specific location, let's read and implement environment variables

		boost::optional<std::tstring> iniValue;

		iniValue = iniFile::iniRead(platformIni, _T("DisplayOptions"), _T("DisableSplashScreens"));
		if (iniValue) {
			mEnv.set(_T("PortableApps.comDisableSplash"), iniValue.get());
		}

		iniValue = iniFile::iniRead(platformIni, _T("Localization"), _T("DisableAppLanguageSwitching"));
		if (!iniValue || boost::iequals(iniValue.get(), "true")) {
			return;
		}

		boost::optional<std::tstring> lang = iniFile::iniRead(platformIni, _T("DisplayOptions"), _T("Language"));
		if (!lang) {
			return;
		}

		boost::filesystem::path languageIni = exePath.get() / (_T("..\\PortableApps.com\\App\\Locale\\") + lang.get() + _T(".locale"));
		if (!boost::filesystem::exists(languageIni, errCode)) {
			return;
		}

		//---------------------------------------------------------------------------------------------------------------------- englishGB: EnglishGB
		mEnv.set(_T("PortableApps.comLanguageName"), lang.get());
		//---------------------------------------------------------------------------------------------------------------------- englishGB: LANG_ENGLISHGB
		std::tstring langName = std::tstring(_T("LANG_")) + lang.get();
		std::transform(langName.begin(), langName.end(), langName.begin(), ::toupper);
		mEnv.set(_T("PortableApps.comLanguageNSIS"), langName);
		mEnv.set(_T("PortableApps.comLocaleWinName"), langName);
		//---------------------------------------------------------------------------------------------------------------------- englishGB: en_GB
		iniValue = iniFile::iniRead(languageIni, _T("PortableApps.comLocaleDetails"), _T("Localeglibc"));
		if (iniValue) {
			mEnv.set(_T("PortableApps.comLanguageGlibc"), iniValue.get());
			mEnv.set(_T("PortableApps.comLocaleglibc"), iniValue.get());
			mEnv.set(_T("LC_CTYPE"), iniValue.get());
		}
		//---------------------------------------------------------------------------------------------------------------------- englishGB: en-gb
		iniValue = iniFile::iniRead(languageIni, _T("PortableApps.comLocaleDetails"), _T("LanguageCode"));
		if (iniValue) {
			mEnv.set(_T("PortableApps.comLanguageCode"), iniValue.get());
			mEnv.set(_T("PortableApps.comLocaleCode"), iniValue.get());
		}
		//---------------------------------------------------------------------------------------------------------------------- englishGB: en
		iniValue = iniFile::iniRead(languageIni, _T("PortableApps.comLocaleDetails"), _T("LocaleCode2"));
		if (iniValue) {
			mEnv.set(_T("PortableApps.comLanguageCode2"), iniValue.get());
			mEnv.set(_T("PortableApps.comLocaleCode2"), iniValue.get());
		}
		//---------------------------------------------------------------------------------------------------------------------- englishGB: eng
		iniValue = iniFile::iniRead(languageIni, _T("PortableApps.comLocaleDetails"), _T("LocaleCode3"));
		if (iniValue) {
			mEnv.set(_T("PortableApps.comLanguageCode3"), iniValue.get());
			mEnv.set(_T("PortableApps.comLocaleCode3"), iniValue.get());
		}
		//---------------------------------------------------------------------------------------------------------------------- englishGB: 2057
		iniValue = iniFile::iniRead(languageIni, _T("PortableApps.comLocaleDetails"), _T("LocaleID"));
		if (iniValue) {
			mEnv.set(_T("PortableApps.comLanguageLCID"), iniValue.get());
			mEnv.set(_T("PortableApps.comLocaleID"), iniValue.get());
		}
	}

	/******************************************************************************
	 *
	 * copyCopy
	 *   did you remember CopyCopy from ZX Spectrum ?
	 *   like boost::filesystem::copy, but if source is directory - copies whole tree
	 *
	 *****************************************************************************/
	bool copyCopy(boost::filesystem::path const& source, boost::filesystem::path const& destination, boost::system::error_code& ec) {
		ec.clear();
		boost::filesystem::file_status statSource = boost::filesystem::status(source);
		boost::filesystem::file_status statDestination = boost::filesystem::status(destination);
		if (statSource != statDestination) {
			boost::filesystem::copy(source, destination, ec);
		}
		if (boost::system::errc::success != ec.value()) {
			return false;
		}
		if (statSource.type() == boost::filesystem::directory_file)
			for (boost::filesystem::directory_iterator file(source); file != boost::filesystem::directory_iterator(); ++file)
				if (!copyCopy(source / file->path().filename(), destination / file->path().filename(), ec)) {
					return false;
				}
		return true;
	}

	/******************************************************************************
	 *
	 * unquote
	 *   remove existing quotes from the string (probably path)
	 *
	 *****************************************************************************/
	std::tstring unquote(const std::tstring& str) {
		if (boost::starts_with(str, _T("\"")) && boost::ends_with(str, _T("\""))) {
			return str.substr(1, str.length() - 2);
		}
		return str;
	}

	/******************************************************************************
	 *
	 * normalize
	 *   "normalize" path - sanitize, resolve links
	 *
	 *****************************************************************************/
	std::tstring normalize(const boost::filesystem::path& str) {
		boost::system::error_code errCode;

		std::tstring retVal = str._tstring();
		boost::erase_all(retVal, _T("\""));                             // unquote; double quote is not allowed at all

		static const std::tstring illegalFnameChars = _T("<>|");		// Reference: http://msdn.microsoft.com/en-us/library/aa365247%28VS.85%29.aspx
		std::size_t found = retVal.find_first_of(illegalFnameChars);
		if (found != std::string::npos) {
			retVal.erase(retVal.find_first_of(illegalFnameChars));
		}      // cut off unexpected redirections

		boost::filesystem::path normalized = boost::filesystem::canonical(boost::filesystem::absolute(boost::filesystem::path(retVal), boost::filesystem::current_path(errCode)), errCode);
		if (boost::system::errc::success != errCode.value()) {
			normalized = retVal;
		}                                        // undo canonical on error
		retVal = normalized._tstring();
		boost::replace_all(retVal, "/", "\\");                          // error: canonical() translates the first '\' to '/'
		return retVal;
	}

	/******************************************************************************
	*
	* quote
	*   quote path containing spaces
	*
	*****************************************************************************/
	std::tstring quote(const boost::filesystem::path& str) {
		std::tstring retVal = unquote(str._tstring());
		if ((!retVal.empty()) && (retVal.find(_T(' ')) != std::string::npos)) {
			return _T("\"") + retVal + _T("\"");
		}
		return retVal;
	}

	/******************************************************************************
	 *
	 * quote
	 *   quote string containing spaces
	 *
	 *****************************************************************************/
	std::tstring quote(std::tstring str) {
		// boost:io:quote has no wchar_t support
		str = unquote(str);
		if ((!str.empty()) && (str.find(_T(' ')) != std::string::npos)) {
			return _T("\"") + str + _T("\"");
		}
		return str;
	}

	/******************************************************************************
	 *
	 * getComputerName
	 *   retrieve NetBIOS computer name
	 *
	 *****************************************************************************/
	std::tstring getComputerName() {
		DWORD bufSize = 0;
		GetComputerName(nullptr, &bufSize);

		std::unique_ptr< TCHAR[] > buf(new (std::nothrow) TCHAR[bufSize]);
		if (buf) {
			if (GetComputerName(buf.get(), &bufSize)) {
				return buf.get();
			}
		}
		return _T("localhost");
	}

	/******************************************************************************
	 *
	 * getUserName
	 *   retrieve SamCompatible user name. Something like Engineering\JSmith
	 *
	 *****************************************************************************/
	std::tstring getUserName() {
		DWORD bufSize = 0;
		GetUserNameEx(NameSamCompatible, nullptr, &bufSize);

		std::unique_ptr< TCHAR[] > buf(new (std::nothrow) TCHAR[bufSize]);
		if (buf) {
			if (GetUserNameEx(NameSamCompatible, buf.get(), &bufSize)) {
				std::tstring retVal = buf.get();
				boost::trim_if(retVal, std::bind2nd(std::equal_to<TCHAR>(), _T('\\')));  // domain version can add trailing backslashes
				if (boost::find_first(retVal, _T("\\"))) {
					return retVal;
				}
			}
		}
		return getComputerName() + _T("\\user");
	}

	/******************************************************************************
	*
	* getDomainName
	*   uses getUserName
	*
	*****************************************************************************/
	std::tstring getDomainName() {
		std::tstring userName = getUserName();
		return userName.substr(0, userName.find(_T('\\')));
	}

	/******************************************************************************
	*
	* pathToUnc
	*   Convert path to UNC
	*
	*****************************************************************************/

	std::tstring pathToUnc(const boost::filesystem::path netPath) {
		DWORD bufSize = 0;
		UNIVERSAL_NAME_INFO* nothing = nullptr;
		if (ERROR_MORE_DATA == WNetGetUniversalName(netPath._tstring().c_str(), UNIVERSAL_NAME_INFO_LEVEL, &nothing, &bufSize)) {
			std::unique_ptr< BYTE[] > buf(new (std::nothrow) BYTE[bufSize]);
			if (buf) {
				WNetGetUniversalName(netPath._tstring().c_str(), UNIVERSAL_NAME_INFO_LEVEL, buf.get(), &bufSize);
				UNIVERSAL_NAME_INFO* pUni = (UNIVERSAL_NAME_INFO*)buf.get();
				return pUni->lpUniversalName;
			}
		}
		return _T("");
	}

	/******************************************************************************
	 *
	 * makeDirWriteable
	 *   creates (if required) and check, if directory is writeable
	 *
	 *****************************************************************************/
	bool makeDirWriteable(const boost::filesystem::path dir) {
		bool retVal = false;
		boost::system::error_code errCode;
		boost::filesystem::path testDir = dir / boost::filesystem::unique_path(_T("%%%%-%%%%-%%%%-%%%%"));
		if (boost::system::errc::success != errCode.value()) {
			testDir = dir / _T("test_dir");
		}
		if (boost::filesystem::exists(testDir, errCode)) { // unlikely
			boost::filesystem::remove(testDir, errCode);
		}
		boost::filesystem::create_directories(testDir, errCode);
		if (boost::system::errc::success == errCode.value()) {
			boost::filesystem::remove(testDir); // remove only the top one
			retVal = true;
		}
		return retVal;
	}

	/******************************************************************************
	*
	* imbueIO
	*   made stdin/stdout/stderr locale-aware
	*
	*****************************************************************************/
	void imbueIO() {
		std::cout.imbue(std::locale());
		std::cerr.imbue(std::locale());
		std::cin.imbue(std::locale());
#ifdef _UNICODE
		(void) _setmode(_fileno(stdout), _O_WTEXT);
		(void)_setmode(_fileno(stdin), _O_WTEXT);
		(void)_setmode(_fileno(stderr), _O_WTEXT);
#endif
	}
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

	void abend(const boost::_tformat msg, int errCode) {
		if (p_apps::sysPidInfo.hasConsole()) {
			std::_tcerr << msg << std::endl;
			if (p_apps::sysPidInfo.ownsConsole()) {
				std::_tcerr << _T("Press [ENTER] key to exit.") << std::endl;
				std::_tcin.get();
			}
		}
		else {
			MessageBox(nullptr, msg.str().c_str(), (boost::_tformat(_T("Cann't continue."))).str().c_str(), MB_OK | MB_ICONERROR);
		}
		exit(errCode);
	}

	/******************************************************************************
	 *
	 * errnoMsg
	 *   wrapper for _tcserror_s
	 *   clears errno
	 *
	 *****************************************************************************/
	std::tstring errnoMsg() {
		const size_t msgSize = 120;
		wchar_t buf[msgSize];
		_tcserror_s(buf, errno);
		return buf;
	}

	/******************************************************************************
	 *
	 * lastErrorMsg
	 *   LastError as a std::_tstring
	 *   clears LastError
	 *
	 *****************************************************************************/
	std::tstring lastErrorMsg() {
		DWORD err = GetLastError();
		SetLastError(ERROR_SUCCESS);
		if (err) {
			LPCTSTR lpMsgBuf = nullptr;
			DWORD bufLen = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, err, 0, (LPTSTR)& lpMsgBuf, 0, nullptr);
			if (bufLen) {
				std::tstring result(lpMsgBuf, lpMsgBuf + bufLen);
				LocalFree((HLOCAL)lpMsgBuf);
				return result;
			}
		}
		return _T("");
	}

	/*****************************************************************************
	* like _texecve, but ComEmu / parent cmd aware. And more.
	*****************************************************************************/
	boost::optional<DWORD> launch(tpWait pWait, const std::tstring& cmdName, const std::vector<std::tstring>& cmdLine, const p_apps::Environment& cmdEnvironment, boost::filesystem::path cwd) {
		if (tpWait::pWait_Auto == pWait) {
			if (!p_apps::sysPidInfo.ownsConsole()) {
				pWait = tpWait::pWait_Wait;
			}
			if (tpWait::pWait_Auto == pWait) {
				if (p_apps::sysPidInfo.hasInjectedDll(_T("ConEmuHk.dll")) || p_apps::sysPidInfo.hasInjectedDll(_T("ConEmuHk64.dll")))
					pWait = tpWait::pWait_Wait;
			}
			if (tpWait::pWait_Auto == pWait) {
				pWait = tpWait::pWait_NoWait;
			}
		}

		// pWait jest nie ustawione ok????
		pWait = tpWait::pWait_Wait;

		//-------------------------------------------- lpApplicationName
		//-------------------------------------------- lpCommandLine

		std::tstring argv = boost::algorithm::join(cmdLine, _T(" "));
		std::unique_ptr<TCHAR[]> commandLine(new (std::nothrow) TCHAR[argv.length() + 1]);
		if (commandLine) {
			wcscpy_s(commandLine.get(), argv.length() + 1, argv.c_str());
		}
		//-------------------------------------------- lpProcessAttributes
		//-------------------------------------------- lpThreadAttributes
		//-------------------------------------------- bInheritHandles
		//-------------------------------------------- dwCreationFlags
		DWORD creationFlags = CREATE_SUSPENDED;
#ifdef _UNICODE
		creationFlags |= CREATE_UNICODE_ENVIRONMENT;
#endif
		//-------------------------------------------- lpEnvironment
		std::unique_ptr< TCHAR[]> env = cmdEnvironment.dump();

		//-------------------------------------------- lpCurrentDirectory
		//-------------------------------------------- lpStartupInfo
		STARTUPINFO startupInfo;
		ZeroMemory(&startupInfo, sizeof startupInfo);
		STARTUPINFO myStartupInfoPtr;
		if (p_apps::sysPidInfo.getStartupInfo(&myStartupInfoPtr)) {
			CopyMemory(&startupInfo, &myStartupInfoPtr, sizeof startupInfo);
		}
		startupInfo.lpTitle = nullptr;
		startupInfo.dwFlags &= ~(STARTF_USESHOWWINDOW);
		startupInfo.cb = sizeof startupInfo;
		//-------------------------------------------- lpProcessInformation
		PROCESS_INFORMATION processInfo;
		ZeroMemory(&processInfo, sizeof processInfo);
		processInfo.hProcess = nullptr;
		processInfo.hThread = nullptr;
		//-------------------------------------------- go
		BOOL ok = p_apps::sysPidInfo.createProcess(cmdName.c_str(), commandLine.get(), nullptr, nullptr, FALSE, creationFlags, env.get(), cwd.c_str(), &startupInfo, &processInfo);
		env.reset();
		commandLine.reset();
		if (!ok) {
			p_apps::abend(boost::_tformat(_T("Cann't execute %1%: %2%")) % cmdName % lastErrorMsg(), 1);
		}
		p_apps::sysPidInfo.setPriorityClass(processInfo.hProcess, p_apps::sysPidInfo.getPriorityClass());

		boost::optional<std::tstring> execError;
		if (!p_apps::sysPidInfo.resumeThread(processInfo.hThread)) {
			execError = lastErrorMsg();
		}

		if (execError) {
			p_apps::abend(boost::_tformat(_T("Cann't execute %1%: %2%")) % cmdName % execError.get(), 1);
		}

		if (tpWait::pWait_Wait == pWait && processInfo.hProcess != nullptr) {
			WaitForSingleObject(processInfo.hProcess, INFINITE);
		}

		if (processInfo.hProcess != nullptr) {
			CloseHandle(processInfo.hProcess);
			processInfo.hProcess = nullptr;
		}
		if (processInfo.hThread != nullptr) {
			CloseHandle(processInfo.hThread);
			processInfo.hThread = nullptr;
		}

		return boost::none;
	}
}

