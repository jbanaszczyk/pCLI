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

#include <windows.h>
#include <random>

namespace p_apps {

	const std::filesystem::path LAUNCHER_INI(_T(VER_PRODUCTNAME_STR) _T(".ini"));

	const std::filesystem::path PORTABLE_APPS = _T("PortableApps");

	const std::filesystem::path PORTABLE_APPS_APP = _T("App");

	const std::filesystem::path PORTABLE_APPS_DATA = _T("Data");

	const std::filesystem::path PORTABLE_APPS_DEFAULT_DATA = _T("App\\DefaultData");

	std::filesystem::path canonical(const std::filesystem::path& p, const std::filesystem::path& base) {
		auto absolutePath = p.is_absolute()
			                    ? absolute(p)
			                    : absolute(base / p);

		std::error_code ec;
		auto result = canonical(absolutePath, ec);

		return !ec
			       ? result
			       : absolutePath;
	}

	std::vector<std::tstring> tokenize(const std::tstring& str) {
		std::vector<std::tstring> result;
		boost::escaped_list_separator<wchar_t> Separator(_T('^'), _T(' '), _T('\"'));
		boost::tokenizer<boost::escaped_list_separator<wchar_t>, std::wstring::const_iterator, std::wstring> tok(str, Separator);
		for (auto it = tok.begin(); it != tok.end(); ++it) {
			result.push_back(*it);
		}

		return result;
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
		std::wstring result;
		std::wistringstream ss(str);
		ss >> std::quoted(result, _T('\"'), _T('^'));
		return result;
	}

	std::tstring quote(const boost::optional<std::tstring>& str) {
		if (str) {
			return quote(str.value());
		}
		return _T("");
	}

	/****************************************************************************
	 * \brief  quote string if it contains spaces
	 * 
	 * \param  str string to be quoted
	 * \return string surrounded with "" if it is required else the same string
	 ***************************************************************************/
	std::tstring quote(const std::tstring& str) {
		auto unquoted = unquote(str);
		if (!needsQuotation(str)) {
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
	std::tstring quote(const std::filesystem::path& fileName) {
		return quote(fileName._tstring());
	}

	/****************************************************************************
	 * \brief  "normalize" filesystem path - sanitize, resolve links, canonical
	 * 
	 * \param  fileName filesystem path to be cleaned
	 * \param  quoted add quotation marks if needed
	 * \return clean filesystem path 
	 ***************************************************************************/
	std::tstring normalize(const std::filesystem::path& fileName, const bool quoted) {
		auto result = unquote(fileName._tstring());
		boost::erase_all(result, _T("\"")); // double quotes are not allowed at all

		static const std::tstring illegalFilenameChars = _T("<>|"); // Reference: http://msdn.microsoft.com/en-us/library/aa365247%28VS.85%29.aspx

		while (const auto found = result.find_first_of(illegalFilenameChars) != std::string::npos) {
			result.erase(found);
		}

		std::error_code errCode;
		const auto currentPath = std::filesystem::current_path();
		const auto absolutePath = absolute(std::filesystem::path(result), errCode);
		if (!errCode) {
			const auto canonicalPath = canonical(absolutePath, errCode);
			if (!errCode) {
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

	std::tstring pathToUnc(const std::filesystem::path& netPath) {
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

	std::tstring generateRandomAlphanumericString(std::size_t len) {
		std::tstring str(_T("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));
		std::random_device rd;
		std::mt19937 generator(rd());
		std::shuffle(str.begin(), str.end(), generator);
		return str.substr(0, len);
	}

	std::wstring string2wstring(const std::string& str) {
		size_t cchRequired = 0;
		errno_t ret = ::mbstowcs_s(&cchRequired, nullptr, 0, str.c_str(), 0); // w/ null terminator
		std::wstring result(cchRequired, L'\0');
		size_t cchActual = cchRequired;
		ret = ::mbstowcs_s(&cchRequired, &result[0], cchActual, str.c_str(), cchActual);
		return result;
	}

	/****************************************************************************
	 * \brief  create directory if doesn't exists, check if is writeable
	 * 
	 * \param  directory to be created

	 ***************************************************************************/
	void makeDirWriteable(const std::filesystem::path& directory) {
		std::error_code errCode;

		create_directories(directory, errCode);
		if (errCode) {
			fail(_T("[%s] Cann't create directory: %s\nReason: %s"), _T(__FUNCTION__), directory._tstring(), string2wstring(errCode.message()));
		}

		constexpr auto maxTry = 8;
		for (auto counter = 0; counter < maxTry; counter++) {
			auto testPath = directory / generateRandomAlphanumericString(8);

			FILE* stream;
			_wfopen_s(&stream, testPath.c_str(), _T("wx"));

			if (stream) {
				[[maybe_unused]] auto ignored = fclose(stream);
				std::error_code ignoredErrorCode;
				std::filesystem::remove(testPath, ignoredErrorCode);
				return;
			}
		}

		fail(_T("[%s] Cann't create files in directory: %s"), _T(__FUNCTION__), directory._tstring());
	}


	/******************************************************************************
	 *
	 * abend
	 *   abnormal end of the thread
	 *     shows message (can be translated) via std::wcerr or message box
	 *     and exits.
	 *	   adds to the pool of boost::locale::translate'd strings:
	 *       _T( "Press [ENTER] key to exit." )
	 *       _T( "Can't to continue." )
	 *
	 *****************************************************************************/

	void abend(const boost::_tformat& msg, int errCode) {
		std::_tcerr << msg << std::endl;
		if (SysInfo::ownsConsole()) {
			std::_tcerr << _T("Press [ENTER] key to exit.") << std::endl;
			std::_tcin.get();

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
	boost::optional<DWORD> execute(bool pWait, const std::tstring& cmdName, const std::vector<std::tstring>& cmdLine, const Environment& cmdEnvironment, const std::filesystem::path& cwd) {

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
		if (! ok) {
			fail(_T("Cann't execute %s: %s"), cmdName, lastErrorMsg());
		}
		SetPriorityClass(processInfo.hProcess, SysInfo::getProcessPriorityClass());

		if (ResumeThread(processInfo.hThread) == static_cast<DWORD>(- 1)) {
			abend(boost::_tformat(_T("Cann't execute %1%: %2%")) % cmdName % lastErrorMsg(), 1);
		}

		if (pWait && processInfo.hProcess != nullptr) {
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
