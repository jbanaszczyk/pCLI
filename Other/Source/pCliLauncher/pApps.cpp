/*************************************************************************/ /**
 * \file      pApps.cpp
 * \brief     Some commonly used functions
 * 
 * \copyright (c) 2011 jacek.banaszczyk@gmail.com
 * \short     Part of pCli project: https://github.com/jbanaszczyk/pCli
 ****************************************************************************/

#include "stdafx.h"
#include "./pApps.h"
#include "../common/SysInfo.h"
#include "../common/IniFile.h"

#include <windows.h>

namespace p_apps {

	const boost::filesystem::path LAUNCHER_INI(_T(VER_PRODUCTNAME_STR) _T(".ini"));

	const boost::filesystem::path PORTABLE_APPS = _T("PortableApps");

	const boost::filesystem::path PORTABLE_APPS_APP = _T("App");

	const boost::filesystem::path PORTABLE_APPS_DATA = _T("Data");

	const boost::filesystem::path PORTABLE_APPS_DEFAULT = _T("App\\DefaultData");

	/******************************************************************************
	 *
	 * copyCopy
	 *   like boost::filesystem::copy, but if source is directory - copies whole tree
	 *
	 *****************************************************************************/
	bool copyCopy(const boost::filesystem::path& source, const boost::filesystem::path& destination, boost::system::error_code& ec) {
		ec.clear();
		const auto statSource = status(source);
		const auto statDestination = status(destination);
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

	/****************************************************************************
	 * \brief  check if string is already quoted
	 * 
	 * \param  str string to be checked
	 * \return true if string is surrounded with ""
	 ***************************************************************************/
	static bool isQuoted(const std::tstring& str) {
		return boost::starts_with(str, _T("\"")) && boost::ends_with(str, _T("\""));
	}

	/****************************************************************************
	 * \brief  check if string is already quoted
	 *
	 * \param  str string to be checked
	 * \return true if string is surrounded with ""
	 ***************************************************************************/
	static bool needsQuotation(const std::tstring& str) {
		return str.empty() || str.find(_T(' ')) != std::string::npos;
	}

	/****************************************************************************
	 * \brief  remove quotation marks
	 * 
	 * \param  str string to be unquoted
	 * \return unquoted string
	 * \detail escape character is '^' - tcc escape char
	 ***************************************************************************/
	std::tstring unquote(const std::tstring& str) {
		if (!isQuoted(str)) {
			return str;
		}
		std::wstring result;
		std::wistringstream ss(str);
		ss >> std::quoted(result, _T('\"'), _T('^'));
		return result;
	}

	/****************************************************************************
	 * \brief  quote string if it contains spaces
	 * 
	 * \param  str string to be quoted
	 * \return string surrounded with "" if it is required else the same string
	 ***************************************************************************/
	std::tstring quote(const std::tstring& str) {
		if (isQuoted(str) || !needsQuotation(str)) {
			return str;
		}
		std::wostringstream oss;
		oss << std::quoted(str,_T('\"'), _T('^'));
		return oss.str();
	}

	/****************************************************************************
	 * \brief  quote filename if it contains spaces
	 *
	 * \param  fileName string to be quoted
	 * \return string surrounded with "" if it is required else the same string
	 * \detail please note, that `"` in filename is illegal using windows
	 ***************************************************************************/
	std::tstring quote(const boost::filesystem::path& fileName) {
		return quote(fileName._tstring());
	}

	/****************************************************************************
	 * \brief  "normalize" filesystem path - sanitize, resolve links, canonical
	 * 
	 * \param  fileName filesystem path to be cleaned
	 * \param  quoted add quotation marks if needed
	 * \return clean filesystem path 
	 ***************************************************************************/
	std::tstring normalize(const boost::filesystem::path& fileName, const bool quoted) {
		auto result = unquote(fileName._tstring());
		boost::erase_all(result, _T("\"")); // double quotes are not allowed at all

		static const std::tstring illegalFilenameChars = _T("<>|"); // Reference: http://msdn.microsoft.com/en-us/library/aa365247%28VS.85%29.aspx

		while (const auto found = result.find_first_of(illegalFilenameChars) != std::string::npos) {
			result.erase(found);
		}

		boost::system::error_code errCode;
		const auto currentPath = boost::filesystem::current_path();
		const auto absolutePath = absolute(boost::filesystem::path(result), currentPath, errCode);
		if (errCode.value() == boost::system::errc::success) {
			const auto canonicalPath = canonical(absolutePath, currentPath, errCode);
			if (errCode.value() == boost::system::errc::success) {
				result = canonicalPath._tstring();
			}
		}

		boost::replace_all(result, "/", "\\");
		return quoted
			       ? quote(result)
			       : result;
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

		const std::unique_ptr<TCHAR[]> buf(new(std::nothrow) TCHAR[bufSize]);
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

		const std::unique_ptr<TCHAR[]> buf(new(std::nothrow) TCHAR[bufSize]);
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
	std::tstring getDomainName() {
		auto userName = getUserName();
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
			const std::unique_ptr<BYTE[]> buf(new(std::nothrow) BYTE[bufSize]);
			if (buf) {
				if (WNetGetUniversalName(netPath._tstring().c_str(), UNIVERSAL_NAME_INFO_LEVEL, buf.get(), &bufSize) == NO_ERROR) {
					const auto pUni = (UNIVERSAL_NAME_INFO*)buf.get();
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
	bool makeDirWriteable(const boost::filesystem::path dir) {
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
	 *       _T( "Can't to continue." )
	 *
	 *****************************************************************************/

	void abend(const boost::_tformat msg, int errCode) {
		if (GetConsoleWindow()) {
			std::_tcerr << msg << std::endl;
			if (SysInfo::ownsConsole()) {
				std::_tcerr << _T("Press [ENTER] key to exit.") << std::endl;
				std::_tcin.get();
			}
		} else {
			MessageBox(nullptr, msg.str().c_str(), (boost::_tformat(_T("Cann't continue."))).str().c_str(), MB_OK | MB_ICONERROR);
		}
		exit(errCode); // NOLINT(concurrency-mt-unsafe)
	}

	/******************************************************************************
	 *
	 * errnoMsg
	 *   wrapper for _tcserror_s
	 *   clears errno
	 *
	 *****************************************************************************/
	std::tstring errnoMsg() {
		constexpr size_t msgSize = 120;
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
		const auto err = GetLastError();
		SetLastError(ERROR_SUCCESS);
		if (err) {
			LPCTSTR lpMsgBuf = nullptr;
			auto bufLen = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, err, 0,
			                            reinterpret_cast<LPTSTR>(&lpMsgBuf),
			                            0, nullptr);
			if (bufLen) {
				std::tstring result(lpMsgBuf, lpMsgBuf + bufLen);
				LocalFree(HLOCAL(lpMsgBuf));
				return result;
			}
		}
		return _T("");
	}

	/*****************************************************************************
	* like _texecve, but ComEmu / parent cmd aware. And more.
	*****************************************************************************/
	boost::optional<DWORD> execute(tpWait pWait, const std::tstring& cmdName, const std::vector<std::tstring>& cmdLine,
	                               const Environment& cmdEnvironment, boost::filesystem::path cwd) {
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

		const auto argv = boost::algorithm::join(cmdLine, _T(" "));
		const std::unique_ptr<TCHAR[]> commandLine(new(std::nothrow) TCHAR[argv.length() + 1]);
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

		const auto ok = CreateProcess(cmdName.c_str(), commandLine.get(), nullptr, nullptr, FALSE, creationFlags, env.get(), cwd.c_str(), &startupInfo, &processInfo);

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
