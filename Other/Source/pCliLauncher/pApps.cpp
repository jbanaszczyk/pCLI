/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#include "stdafx.h"
#include "./pApps.h"
#include "../common/SysInfo.h"
#include "../common/IniFile.h"

#include <windows.h>

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
	const boost::filesystem::path LAUNCHER_INI(_T(VER_PRODUCTNAME_STR) _T(".ini"));

	const boost::filesystem::path PORTABLE_APPS_APP = _T("App");

	const boost::filesystem::path PORTABLE_APPS_DEFAULT = _T("App\\DefaultData");

	const boost::filesystem::path PORTABLE_APPS_DATA = _T("Data");

	const boost::filesystem::path PORTABLE_APPS = _T("PortableApps");
	const boost::filesystem::path PORTABLE_APPS_INI = _T("Data\\PortableAppsMenu.ini");

	const boost::filesystem::path PORTABLE_APPS_DOCUMENTS = _T("Documents");





	/******************************************************************************
	 *
	 * tstring2string
	 * string2tstring
	 *   Simple conversions tstring / string
	 *
	 *****************************************************************************/
	auto tstring2string(const std::tstring& sou) -> std::string {
#ifdef _UNICODE
		const std::locale loc("");
		const auto from = sou.c_str();
		const auto len = sou.size();
		std::vector<char> buffer(len + 1);
		std::use_facet<std::ctype<wchar_t>>(loc).narrow(from, from + len, '_', &buffer[0]);
		return std::string(&buffer[0], &buffer[len]);
#else
		return sou;
#endif
	}

	/******************************************************************************
	 *
	 * string2tstring
	 *   Simple conversion string -> tstring
	 *
	 *****************************************************************************/
	auto string2tstring(const std::string& sou) -> std::tstring {
#ifdef _UNICODE
		const std::locale loc("");
		const auto from = sou.c_str();
		const auto len = sou.size();
		std::vector<wchar_t> buffer(len + 1);
		std::use_facet<std::ctype<wchar_t>>(loc).widen(from, from + len, &buffer[0]);
		auto s = std::tstring(&buffer[0], &buffer[len]);
		return std::tstring(&buffer[0], &buffer[len]);
#else
		return sou;
#endif
	}


	/******************************************************************************
	 *
	 * copyCopy
	 *   did you remember CopyCopy from ZX Spectrum ?
	 *   like boost::filesystem::copy, but if source is directory - copies whole tree
	 *
	 *****************************************************************************/
	auto copyCopy(const boost::filesystem::path& source, const boost::filesystem::path& destination, boost::system::error_code& ec) -> bool {
		ec.clear();
		auto statSource = status(source);
		auto statDestination = status(destination);
		if (statSource != statDestination) {
			copy(source, destination, ec);
		}
		if (boost::system::errc::success != ec.value()) {
			return false;
		}
		if (statSource.type() == boost::filesystem::directory_file) {
			for (boost::filesystem::directory_iterator file(source); file != boost::filesystem::directory_iterator(); ++file) {
				if (!copyCopy(source / file->path().filename(), destination / file->path().filename(), ec)) {
					return false;
				}
			}
		}
		return true;
	}

	/******************************************************************************
	 *
	 * unquote
	 *   remove existing quotes from the string (probably path)
	 *
	 *****************************************************************************/
	auto unquote(const std::tstring& str) -> std::tstring {
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
	auto normalize(const boost::filesystem::path& str) -> std::tstring {
		boost::system::error_code errCode;

		auto result = str._tstring();
		boost::erase_all(result, _T("\"")); // unquote; double quote is not allowed at all

		static const std::tstring illegalFnameChars = _T("<>|"); // Reference: http://msdn.microsoft.com/en-us/library/aa365247%28VS.85%29.aspx
		auto found = result.find_first_of(illegalFnameChars);
		if (found != std::string::npos) {
			result.erase(result.find_first_of(illegalFnameChars));
		} // cut off unexpected redirections

		auto normalized = canonical(absolute(boost::filesystem::path(result), boost::filesystem::current_path(errCode)), errCode);
		if (boost::system::errc::success != errCode.value()) {
			normalized = result;
		} // undo canonical on error
		result = normalized._tstring();
		boost::replace_all(result, "/", "\\"); // error: canonical() translates the first '\' to '/'
		return result;
	}

	/******************************************************************************
	*
	* quote
	*   quote path containing spaces
	*
	*****************************************************************************/
	auto quote(const boost::filesystem::path& str) -> std::tstring {
		auto result = unquote(str._tstring());
		if ((!result.empty()) && (result.find(_T(' ')) != std::string::npos)) {
			return _T("\"") + result + _T("\"");
		}
		return result;
	}

	/******************************************************************************
	 *
	 * quote
	 *   quote string containing spaces
	 *
	 *****************************************************************************/
	auto quote(std::tstring str) -> std::tstring {
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
	auto getComputerName() -> std::tstring {
		DWORD bufSize = 0;
		GetComputerName(nullptr, &bufSize);

		std::unique_ptr<TCHAR[]> buf(new(std::nothrow) TCHAR[bufSize]);
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
	auto getUserName() -> std::tstring {
		DWORD bufSize = 0;
		GetUserNameEx(NameSamCompatible, nullptr, &bufSize);

		std::unique_ptr<TCHAR[]> buf(new(std::nothrow) TCHAR[bufSize]);
		if (buf) {
			if (GetUserNameEx(NameSamCompatible, buf.get(), &bufSize)) {
				std::tstring result = buf.get();

				// FIXME review boost iterators to make it prettier

				if ('\\' == result.back()) {
					result.pop_back();
				}

				if (boost::find_first(result, _T("\\"))) {
					return result;
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
	auto getDomainName() -> std::tstring {
		auto userName = getUserName();
		return userName.substr(0, userName.find(_T('\\')));
	}

	/******************************************************************************
	*
	* pathToUnc
	*   Convert path to UNC
	*
	*****************************************************************************/

	auto pathToUnc(const boost::filesystem::path netPath) -> std::tstring {
		DWORD bufSize = 0;
		UNIVERSAL_NAME_INFO* nothing = nullptr;
		if (ERROR_MORE_DATA == WNetGetUniversalName(netPath._tstring().c_str(), UNIVERSAL_NAME_INFO_LEVEL, &nothing, &bufSize)) {
			std::unique_ptr<BYTE[]> buf(new(std::nothrow) BYTE[bufSize]);
			if (buf) {
				if (WNetGetUniversalName(netPath._tstring().c_str(), UNIVERSAL_NAME_INFO_LEVEL, buf.get(), &bufSize) == NO_ERROR) {
					auto pUni = (UNIVERSAL_NAME_INFO*)buf.get();
					return pUni->lpUniversalName;
				}
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
	auto makeDirWriteable(const boost::filesystem::path dir) -> bool {
		auto result = false;
		boost::system::error_code errCode;
		auto testDir = dir / boost::filesystem::unique_path(_T("%%%%-%%%%-%%%%-%%%%"));
		if (boost::system::errc::success != errCode.value()) {
			testDir = dir / _T("test_dir");
		}
		if (exists(testDir, errCode)) {
			// unlikely
			boost::filesystem::remove(testDir, errCode);
		}
		create_directories(testDir, errCode);
		if (boost::system::errc::success == errCode.value()) {
			boost::filesystem::remove(testDir); // remove only the top one
			result = true;
		}
		return result;
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

	auto abend(const boost::_tformat msg, int errCode) -> void {
		if (GetConsoleWindow()) {
			std::_tcerr << msg << std::endl;
			if (SysInfo::ownsConsole()) {
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
	auto errnoMsg() -> std::tstring {
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
	auto lastErrorMsg() -> std::tstring {
		auto err = GetLastError();
		SetLastError(ERROR_SUCCESS);
		if (err) {
			LPCTSTR lpMsgBuf = nullptr;
			auto bufLen = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, err, 0, (LPTSTR)&lpMsgBuf, 0, nullptr);
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
	auto execute(tpWait pWait, const std::tstring& cmdName, const std::vector<std::tstring>& cmdLine, const Environment& cmdEnvironment, boost::filesystem::path cwd) -> boost::optional<DWORD> {
		if (tpWait::pWait_Auto == pWait) {
			if (!SysInfo::ownsConsole()) {
				pWait = tpWait::pWait_Wait;
			}
			if (tpWait::pWait_Auto == pWait) {
				auto conEmu = SysInfo::getDllName(_T("ConEmuHk.dll"));
				if (!conEmu) {
					conEmu = SysInfo::getDllName(_T("ConEmuHk64.dll"));
				}
				if (conEmu) {
					pWait = tpWait::pWait_Wait;
				}
			}
			if (tpWait::pWait_Auto == pWait) {
				pWait = tpWait::pWait_NoWait;
			}
		}

		// pWait jest nie ustawione ok????
		pWait = tpWait::pWait_Wait;

		//-------------------------------------------- lpApplicationName
		//-------------------------------------------- lpCommandLine

		auto argv = boost::algorithm::join(cmdLine, _T(" "));
		std::unique_ptr<TCHAR[]> commandLine(new(std::nothrow) TCHAR[argv.length() + 1]);
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
		std::unique_ptr<TCHAR[]> env;
		cmdEnvironment.dump(env);
		//-------------------------------------------- lpCurrentDirectory
		//-------------------------------------------- lpStartupInfo
		STARTUPINFO startupInfo;
		ZeroMemory(&startupInfo, sizeof startupInfo); // SecureZeroMemory(&si, sizeof(STARTUPINFO)); ?
		STARTUPINFO myStartupInfoPtr;
		GetStartupInfo(&myStartupInfoPtr);
		CopyMemory(&startupInfo, &myStartupInfoPtr, sizeof startupInfo);

		startupInfo.lpTitle = nullptr;
		startupInfo.dwFlags &= ~(STARTF_USESHOWWINDOW);
		startupInfo.cb = sizeof startupInfo;
		//-------------------------------------------- lpProcessInformation
		PROCESS_INFORMATION processInfo;
		ZeroMemory(&processInfo, sizeof processInfo);
		processInfo.hProcess = nullptr;
		processInfo.hThread = nullptr;
		//-------------------------------------------- go

		auto ok = CreateProcess(cmdName.c_str(), commandLine.get(), nullptr, nullptr, FALSE, creationFlags, env.get(), cwd.c_str(), &startupInfo, &processInfo);

		// env.reset();
		// commandLine.reset();
		if (!ok) {
			abend(boost::_tformat(_T("Cann't execute %1%: %2%")) % cmdName % lastErrorMsg(), 1);
		}
		SetPriorityClass(processInfo.hProcess, SysInfo::getProcessPriorityClass());

		if (ResumeThread(processInfo.hThread) == static_cast<DWORD>(-1)) {
			abend(boost::_tformat(_T("Cann't execute %1%: %2%")) % cmdName % lastErrorMsg(), 1);
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
