/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#include "stdafx.h"
#include "./SysInfo.h"

 /******************************************************************************
  *
  * singleton pApps::SysPidInfo allows queries about processes usid PID
  *
  * singleton should be manipulated using pApps::_sysPidInfo.
  *
  * All process related functions are accessed via LoadLibrary/GetProcAddress()
  * Some methods are overloaded.
  *   If there is nothing about PID - it means "current process"
  * Some methods (getDllName, getCommandLine)
  *   fails accessing 64-bit process from 32-bit application
  * From the same reason getExeName in some circumstances is not able to resolve
  *   \Device\Partition notation
  *
  * Toolhelp32Snapshot is taken during constructor.
  *   Refresh snapshoot to get current state.
  *
  * Methods:
  *   getMyPID()
  *     return current process ID
  *   getParentPID()
  *     returns PID of parent process
  *   getName()
  *     returns short name of the process (for example: cmd.exe)
  *   getExeName()
  *     returns full path of the process executable
  *     for example: c:\windows\system32\cmd.exe
  *     getExeName() tries to resolve 64-bit processes queried from 32-bit app
  *     but in some cases it returns \Device\Partition notation
  *   SysPidInfo::getDllName ( dllName [, PID] )
  *     returns full path of module/dll loaded by process
  *       getDllName( "kernel32.dll" ) returns c:\windows\system32\kernel32.dll
  *     method is provided to check if particular dll is loaded by given process
  *       especially ConEmu (conEmuHk.dll)
  *     method fails if called from 32-bit app to query 64-bit process
  *   bool isWow64()
  *     check if process is running on Wow64
  *     (!) 64-bit processes running on 64-bit platform ar not using Wow64
  *   isWow64(): without params
  *     returns true ifdef _WIN64
  *     for legacy isWow64() call isWow64( getInstance().getMyPID() );
  *   bool getCommandLine
  *     retrieves whole command line of the given process
  *     not guaranted.
  *     method fails if called from 32-bit app to query 64-bit process
  *
  *****************************************************************************/
namespace p_apps {

	auto SysInfo::findProcessInfo(PROCESSENTRY32& procEntry, const DWORD aPid) const -> bool {
		HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snapShot == INVALID_HANDLE_VALUE) {
			return false;
		}
		auto result = false;
		procEntry.dwSize = sizeof PROCESSENTRY32;
		if (Process32First(snapShot, &procEntry)) {
			do {
				if (aPid == procEntry.th32ProcessID) {
					result = true;
					break;
				}
			} while (Process32Next(snapShot, &procEntry));
		}
		CloseHandle(snapShot);
		return result;
	}

	 auto SysInfo::getExeName()  -> boost::optional<boost::filesystem::path> {
		TCHAR moduleName[MAX_PATH];
		auto isValid = false;
		auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
		if (hProcess == nullptr) {
			return boost::none;
		}
		HMODULE hMod;
		DWORD cbNeeded = 0;
		auto Ok = EnumProcessModules(hProcess, &hMod, sizeof hMod, &cbNeeded);
		isValid = false;
		if (!Ok && ERROR_PARTIAL_COPY == GetLastError()) {
			// http://winprogger.com/getmodulefilenameex-enumprocessmodulesex-failures-in-wow64
			if (GetProcessImageFileName(hProcess, moduleName, _countof(moduleName))) {

				// convert something like "\Device\HarddiskVolume2"	to "C:"
				TCHAR devPath[MAX_PATH];
				TCHAR devName[MAX_PATH] = _T("_:");
				for (auto aDrive = _T('A'); _T('Z') > aDrive; ++aDrive) {
					devName[0] = aDrive;
					if (QueryDosDevice(devName, devPath, _countof(devPath))) {
						auto len = _tcslen(devPath);
						if (MAX_PATH - 2 > len) {
							devPath[len] = _T('\\');
							devPath[len + 1] = _T('\0');
							if (boost::istarts_with(moduleName, devPath)) {
								wcscat_s(devName, moduleName + len);
								wcscpy_s(moduleName, devName);
								break;
							}
						}
					}
				}

				isValid = true;
			}
		}
		if (Ok) {
			if (GetModuleFileNameEx(hProcess, hMod, moduleName, _countof(moduleName))) {
				isValid = true;
			}
		}
		CloseHandle(hProcess);
		return isValid
			? boost::optional<boost::filesystem::path>(moduleName)
			: boost::none;
	}

	auto SysInfo::getDllName(const std::tstring dllName)  -> boost::optional<boost::filesystem::path> {
		auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
		if (hProcess == nullptr) {
			return boost::none;
		}
		boost::optional<boost::filesystem::path> result = boost::none;
		DWORD cbNeeded = 0;
		if (EnumProcessModules(hProcess, nullptr, 0, &cbNeeded)) {
			DWORD nEntries = cbNeeded / sizeof(HMODULE);
			std::unique_ptr<HMODULE[]> hMod(new(std::nothrow) HMODULE[nEntries]);

			if (hMod) {
				if (EnumProcessModules(hProcess, hMod.get(), nEntries * sizeof(hMod[0]), &cbNeeded)) {
					// probably nEntries * sizeof( *hMod ) == cbNeeded, but ...
					for (DWORD idx = 0; nEntries > idx; ++idx) {
						TCHAR processName[MAX_PATH];
						if (GetModuleFileNameEx(hProcess, hMod[idx], processName, _countof(processName))) {
							boost::filesystem::path dll(processName);
							if (boost::iequals(dll.filename().c_str(), dllName)) {
								result = dll;
								break;
							}
						}
					}
				}
			}
		}

		CloseHandle(hProcess);
		return result;
	}

	auto SysInfo::isWow64()  -> bool {
#ifdef _WIN64
		return true;
#else
		auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
		if (hProcess == nullptr) {
			return false;
		}
		auto Wow64Process = FALSE;
		if (!IsWow64Process(hProcess, &Wow64Process)) {
			Wow64Process = FALSE;
		}
		CloseHandle(hProcess);
		return Wow64Process == TRUE;
#endif
	}

	auto SysInfo::GetProcessPriorityClass()  -> DWORD {
		auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
		if (hProcess == nullptr) {
			return NORMAL_PRIORITY_CLASS;
		}
		auto result = GetPriorityClass(hProcess);
		CloseHandle(hProcess);
		if (!result) {
			result = NORMAL_PRIORITY_CLASS;
		}
		return result;
	}

	auto SysInfo::ownsConsole()  -> bool {
		auto consoleWnd = GetConsoleWindow();
		DWORD dwProcessId;
		GetWindowThreadProcessId(consoleWnd, &dwProcessId);
		if (GetCurrentProcessId() != dwProcessId) {
			return false;
		}

		if (!_isatty(_fileno(stdout)) || !_isatty(_fileno(stdin)) || !_isatty(_fileno(stderr))) {
			return false;
		}
		return true;
	}


}
