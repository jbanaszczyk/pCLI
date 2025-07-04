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

namespace p_apps {
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

	std::vector<std::wstring> tokenize(const std::wstring& str) {
		std::vector<std::wstring> result;
		boost::escaped_list_separator<wchar_t> separator(L'^', L' ', L'\"');
		boost::tokenizer<boost::escaped_list_separator<wchar_t>, std::wstring::const_iterator, std::wstring> tok(str, separator);
		for (auto it = tok.begin(); it != tok.end(); ++it) {
			result.push_back(*it);
		}

		return result;
	}

	static bool needsQuotation(const std::wstring& str) {
		return str.empty() || str.find(L' ') != std::string::npos;
	}

	/****************************************************************************
	 * \brief  remove quotation marks
	 *
	 * \param  str string to be unquoted
	 * \return unquoted string
	 * \detail escape character is '^' - tcc escape char
	 ***************************************************************************/
	std::wstring unquote(const std::wstring& str) {
		std::wstring result;
		std::wistringstream ss(str);
		ss >> std::quoted(result, L'\"', L'^');
		return result;
	}

	std::wstring quote(const std::optional<std::wstring>& str) {
		if (str) {
			return quote(str.value());
		}
		return L"";
	}

	/****************************************************************************
	 * \brief  quote string if it contains spaces
	 *
	 * \param  str string to be quoted
	 * \return string surrounded with "" if it is required else the same string
	 ***************************************************************************/
	std::wstring quote(const std::wstring& str) {
		auto unquoted = unquote(str);
		if (!needsQuotation(str)) {
			return str;
		}
		std::wostringstream oss;
		oss << std::quoted(str, L'\"', L'^');
		return oss.str();
	}

	/****************************************************************************
	 * \brief  quote filename if it contains spaces
	 *
	 * \param  fileName string to be quoted
	 * \return string surrounded with "" if it is required else the same string
	 * \detail please note, that `"` in filename is illegal using windows
	 ***************************************************************************/
	std::wstring quote(const std::filesystem::path& fileName) {
		return quote(fileName.wstring());
	}

	/******************************************************************************
	 *
	 * getComputerName
	 *   retrieve NetBIOS computer name
	 *
	 *****************************************************************************/
	std::wstring getComputerName() {
		DWORD bufSize = 0;
		GetComputerName(nullptr, &bufSize);

		const std::unique_ptr<wchar_t[]> buf(new(std::nothrow) wchar_t[bufSize]);
		if (buf) {
			if (GetComputerName(buf.get(), &bufSize)) {
				return buf.get();
			}
		}
		return L"localhost";
	}

	/******************************************************************************
	 *
	 * getUserName
	 *   retrieve SamCompatible user name. Something like Engineering\JSmith
	 *
	 *****************************************************************************/
	std::wstring getUserName() {
		DWORD bufSize = 0;
		GetUserNameEx(NameSamCompatible, nullptr, &bufSize);

		const std::unique_ptr<wchar_t[]> buf(new(std::nothrow) wchar_t[bufSize]);
		if (buf) {
			if (GetUserNameEx(NameSamCompatible, buf.get(), &bufSize)) {
				std::wstring result = buf.get();

				// FIXME review boost iterators to make it prettier

				if ('\\' == result.back()) {
					result.pop_back();
				}

				if (boost::find_first(result, L"\\")) {
					return result;
				}
			}
		}
		return getComputerName() + L"\\user";
	}

	/******************************************************************************
	*
	* getDomainName
	*   uses getUserName
	*
	*****************************************************************************/
	std::wstring getDomainName() {
		auto userName = getUserName();
		return userName.substr(0, userName.find(L'\\'));
	}

	/******************************************************************************
	*
	* pathToUnc
	*   Convert path to UNC
	*
	*****************************************************************************/

	std::wstring pathToUnc(const std::filesystem::path& netPath) {
		DWORD bufSize = 0;
		UNIVERSAL_NAME_INFO* nothing = nullptr;
		if (ERROR_MORE_DATA == WNetGetUniversalName(netPath.wstring().c_str(), UNIVERSAL_NAME_INFO_LEVEL, &nothing, &bufSize)) {
			const std::unique_ptr<BYTE[]> buf(new(std::nothrow) BYTE[bufSize]);
			if (buf) {
				if (WNetGetUniversalName(netPath.wstring().c_str(), UNIVERSAL_NAME_INFO_LEVEL, buf.get(), &bufSize) == NO_ERROR) {
					const auto pUni = reinterpret_cast<UNIVERSAL_NAME_INFO*>(buf.get());
					return pUni->lpUniversalName;
				}
			}
		}
		return L"";
	}

	std::wstring generateRandomAlphanumericString(const std::size_t len) {
		// ReSharper disable StringLiteralTypo
		std::wstring str(L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
		// ReSharper restore StringLiteralTypo
		std::random_device rd;
		std::mt19937 generator(rd());
		std::shuffle(str.begin(), str.end(), generator);
		return str.substr(0, len);
	}

	// ReSharper disable once CppInconsistentNaming
	std::wstring string2wstring(const std::string& str) {
		size_t cchRequired = 0;
		[[maybe_unused]] auto ignored1 = mbstowcs_s(&cchRequired, nullptr, 0, str.c_str(), 0); // w/ null terminator
		std::wstring result(cchRequired, L'\0');
		const auto cchActual = cchRequired;
		[[maybe_unused]] auto ignored2 = mbstowcs_s(&cchRequired, result.data(), cchActual, str.c_str(), cchActual);
		return result;
	}

	FILE* createEmptyFile(const std::filesystem::path& fileName) {
		FILE* stream;
		_wfopen_s(&stream, fileName.c_str(), L"wx");
		if (stream) {
			[[maybe_unused]] auto ignored = fclose(stream);
		}
		return stream;
	}

	/****************************************************************************
	 * \brief  create directory if does not exist, check if is writeable
	 *
	 * \param  directory to be created
	 ***************************************************************************/
	void makeDirWriteable(const std::filesystem::path& directory) {
		std::error_code errCode;

		create_directories(directory, errCode);
		if (errCode) {
			fail(L"[%s] Cannot create directory: %s\nReason: %s", _T(__FUNCTION__), directory.wstring(), string2wstring(errCode.message()));
		}

		constexpr auto maxTry = 8;
		for (auto counter = 0; counter < maxTry; counter++) {
			auto testPath = directory / generateRandomAlphanumericString(8);

			// ReSharper disable once CppTooWideScope
			const auto stream = createEmptyFile(testPath);
			if (stream) {
				std::filesystem::remove(testPath);
				return;
			}
		}

		fail(L"[%s] Cannot create files in directory: %s", _T(__FUNCTION__), directory.wstring());
	}

	/******************************************************************************
	 *
	 * errnoMsg
	 *   wrapper for _tcserror_s
	 *   clears errno
	 *
	 *****************************************************************************/
	std::wstring errnoMsg() {
		constexpr size_t msgSize = 128;
		wchar_t buf[msgSize];
		_tcserror_s(buf, errno);
		buf[msgSize - 1] = L'\0';
		return buf;
	}

	/******************************************************************************
	 *
	 * lastErrorMsg
	 *   LastError as a std::wstring
	 *   clears LastError
	 *
	 *****************************************************************************/
	std::wstring lastErrorMsg() {
		const auto err = GetLastError();
		SetLastError(ERROR_SUCCESS);
		if (err) {
			LPCTSTR lpMsgBuf = nullptr;
			// ReSharper disable once CppTooWideScope
			const auto bufLen = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, err, 0,
			                                  reinterpret_cast<LPTSTR>(&lpMsgBuf),
			                                  0, nullptr);
			if (bufLen) {
				std::wstring result(lpMsgBuf, lpMsgBuf + bufLen);
				// ReSharper disable once CppFunctionalStyleCast
				LocalFree(HLOCAL(lpMsgBuf));
				return result;
			}
		}
		return L"";
	}

	void execute(const bool pWait, const std::wstring& cmdName, const std::vector<std::wstring>& cmdLine, const Environment& cmdEnvironment, const std::filesystem::path& cwd) {
		//-------------------------------------------- lpApplicationName
		//-------------------------------------------- lpCommandLine

		const auto argv = boost::algorithm::join(cmdLine, L" ");
		const std::unique_ptr<wchar_t[]> commandLine(new(std::nothrow) wchar_t[argv.length() + 1]);
		if (commandLine) {
			wcscpy_s(commandLine.get(), argv.length() + 1, argv.c_str());
		}
		//-------------------------------------------- lpProcessAttributes
		//-------------------------------------------- lpThreadAttributes
		//-------------------------------------------- bInheritHandles
		//-------------------------------------------- dwCreationFlags
		constexpr DWORD creationFlags = CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT;

		//-------------------------------------------- lpEnvironment
		std::unique_ptr<wchar_t[]> env;
		cmdEnvironment.dump(env);
		//-------------------------------------------- lpCurrentDirectory
		//-------------------------------------------- lpStartupInfo
		STARTUPINFO startupInfo;
		ZeroMemory(&startupInfo, sizeof startupInfo);
		STARTUPINFO myStartupInfoPtr;
		GetStartupInfo(&myStartupInfoPtr);
		CopyMemory(&startupInfo, &myStartupInfoPtr, sizeof startupInfo);

		startupInfo.lpTitle = nullptr;

		startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		startupInfo.dwFlags |= STARTF_USESTDHANDLES;
		startupInfo.dwFlags &= ~STARTF_USESHOWWINDOW;

		startupInfo.cb = sizeof startupInfo;
		//-------------------------------------------- lpProcessInformation
		PROCESS_INFORMATION processInfo;
		ZeroMemory(&processInfo, sizeof processInfo);
		processInfo.hProcess = nullptr;
		processInfo.hThread = nullptr;
		//-------------------------------------------- go

		// ReSharper disable once CppTooWideScopeInitStatement
		const auto ok = CreateProcess(
			cmdName.c_str(),
			commandLine.get(),
			nullptr,
			nullptr,
			TRUE, creationFlags,
			env.get(),
			cwd.c_str(),
			&startupInfo,
			&processInfo);

		if (!ok) {
			fail(L"Cannot execute %s: %s", cmdName, lastErrorMsg());
		}
		SetPriorityClass(processInfo.hProcess, sys_info::getProcessPriorityClass());

		if (ResumeThread(processInfo.hThread) == static_cast<DWORD>(-1)) {
			fail(L"Cannot execute %s: %s", cmdName, lastErrorMsg());
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
	}

	// ReSharper disable once CppInconsistentNaming
	void imbueIO() {
		std::cout.imbue(std::locale());
		std::cerr.imbue(std::locale());
		std::cin.imbue(std::locale());

		(void) _setmode(_fileno(stdout), _O_WTEXT);
		(void) _setmode(_fileno(stdin), _O_WTEXT);
		(void) _setmode(_fileno(stderr), _O_WTEXT);
	}
}
