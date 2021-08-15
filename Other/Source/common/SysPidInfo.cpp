/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#include "stdafx.h"
#include "./SysPidInfo.h"

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
	auto SysPidInfo::tGetProcAddress(const HMODULE hModule, const LPCSTR lpProcName) const -> FARPROC {
		auto result = GetProcAddress(hModule, lpProcName);
		if (result == nullptr){
			auto len = strlen(lpProcName);
			std::unique_ptr<char[ ]> tName(new(std::nothrow) char[ len + 2 ]);
			if (tName){
				strcpy_s(tName.get(), len + 2, lpProcName);
#ifdef _UNICODE
				tName[ len ] = _T('W');
#else
			tName[len] = _T ( 'A' );
#endif
				tName[ len + 1 ] = _T('\0');
				result = GetProcAddress(hModule, tName.get());
			}
		}
		return result;
	}

	auto SysPidInfo::clear(void) -> void {
		_hSnapShot = INVALID_HANDLE_VALUE;
		_fnCreateProcess = nullptr;
		_fnCreateToolhelp32Snapshot = nullptr;
		_fnEnumProcessModules = nullptr;
		_fnGetConsoleWindow = nullptr;
		_fnGetCurrentProcessId = nullptr;
		_fnGetModuleFileNameEx = nullptr;
		_fnGetPriorityClass = nullptr;
		_fnGetProcessImageFileName = nullptr;
		_fnGetStartupInfo = nullptr;
		_fnGetWindowThreadProcessId = nullptr;
		_fnIsWow64Process = nullptr;
		_fnNtQueryInformationProcess = nullptr;
		_fnOpenProcess = nullptr;
		_fnProcess32First = nullptr;
		_fnProcess32Next = nullptr;
		_fnQueryDosDevice = nullptr;
		_fnReadProcessMemory = nullptr;
		_fnResumeThread = nullptr;
		_fnSetPriorityClass = nullptr;
	};

	SysPidInfo::SysPidInfo()
		: _hInstDllKernel(nullptr),
		  _hInstDllPsApi(nullptr),
		  _hInstDllNtdll(nullptr),
		  _hInstDllUser(nullptr) {
		clear();

		auto _hInstDllKernel = LoadLibrary(_T("Kernel32.DLL"));
		// Get procedure addresses.
		// We are linking to these functions of Kernel32 explicitly, because otherwise a module using
		// this code would fail to load under Windows NT, which does not have the Toolhelp32 functions in the Kernel 32.
		if (_hInstDllKernel){
			_fnCreateProcess = (__CREATE_PROCESS)tGetProcAddress(_hInstDllKernel, "CreateProcess");
			_fnCreateToolhelp32Snapshot = (__CREATE_TOOLHELP32_SNAPSHOT)tGetProcAddress(_hInstDllKernel, "CreateToolhelp32Snapshot");
			_fnGetConsoleWindow = (__GET_CONSOLE_WINDOW)tGetProcAddress(_hInstDllKernel, "GetConsoleWindow");
			_fnGetCurrentProcessId = (__GETCURRENT_PROCESS_ID)tGetProcAddress(_hInstDllKernel, "GetCurrentProcessId");
			_fnGetPriorityClass = (__GET_PRIORITY_CLASS)tGetProcAddress(_hInstDllKernel, "GetPriorityClass");
			_fnGetStartupInfo = (__GET_STARTUP_INFO)tGetProcAddress(_hInstDllKernel, "GetStartupInfo");
			_fnIsWow64Process = (__IS_WOW64_PROCESS)tGetProcAddress(_hInstDllKernel, "IsWow64Process");
			_fnOpenProcess = (__OPEN_PROCESS)tGetProcAddress(_hInstDllKernel, "OpenProcess");
			_fnProcess32First = (__PROCESS32_FIRST)tGetProcAddress(_hInstDllKernel, "Process32First");
			_fnProcess32Next = (__PROCESS32_NEXT)tGetProcAddress(_hInstDllKernel, "Process32Next");
			_fnQueryDosDevice = (__QUERY_DOS_DEVICE)tGetProcAddress(_hInstDllKernel, "QueryDosDevice");
			_fnReadProcessMemory = (__READ_PROCESS_MEMORY)tGetProcAddress(_hInstDllKernel, "ReadProcessMemory");
			_fnResumeThread = (__RESUME_THREAD)tGetProcAddress(_hInstDllKernel, "ResumeThread");
			_fnSetPriorityClass = (__SET_PRIORITY_CLASS)tGetProcAddress(_hInstDllKernel, "SetPriorityClass");
		}
		_hInstDllPsApi = LoadLibrary(_T("PsAPI.DLL"));
		if (_hInstDllPsApi){
			_fnEnumProcessModules = (__ENUM_PROCESS_MODULES)tGetProcAddress(_hInstDllPsApi, "EnumProcessModules");
			_fnGetModuleFileNameEx = (__GETMODULE_FILE_NAME_EX)tGetProcAddress(_hInstDllPsApi, "GetModuleFileNameEx");
			_fnGetProcessImageFileName = (__GETPROCESS_IMAGE_FILE_NAME)tGetProcAddress(_hInstDllPsApi, "GetProcessImageFileName");
		}
		_hInstDllNtdll = LoadLibrary(_T("Ntdll.dll"));
		if (_hInstDllNtdll){
			_fnNtQueryInformationProcess = (__NT_QUERY_INFORMATION_PROCESS)tGetProcAddress(_hInstDllNtdll, "NtQueryInformationProcess");
		}
		_hInstDllUser = LoadLibrary(_T("User32.dll"));
		if (_hInstDllUser){
			_fnGetWindowThreadProcessId = (__GET_WINDOW_THREAD_PROCESS_ID)tGetProcAddress(_hInstDllUser, "GetWindowThreadProcessId");
		}
		// Get a handle to a Toolhelp snapshot of the systems processes.
		if (_fnCreateToolhelp32Snapshot && _fnProcess32First && _fnProcess32Next){
			_hSnapShot = _fnCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		}
	}

	SysPidInfo::~SysPidInfo() {
		clear();
		if (INVALID_HANDLE_VALUE != _hSnapShot){
			CloseHandle(_hSnapShot);
			_hSnapShot = INVALID_HANDLE_VALUE;
		}
		if (_hInstDllKernel){
			FreeLibrary(_hInstDllKernel);
			_hInstDllKernel = nullptr;
		}
		if (_hInstDllPsApi){
			FreeLibrary(_hInstDllPsApi);
			_hInstDllKernel = nullptr;
		}
		if (_hInstDllNtdll){
			FreeLibrary(_hInstDllNtdll);
			_hInstDllNtdll = nullptr;
		}
		if (_hInstDllUser){
			FreeLibrary(_hInstDllUser);
			_hInstDllUser = nullptr;
		}
	}

	auto SysPidInfo::getMyPID() const -> DWORD {
		if (_fnGetCurrentProcessId){
			return _fnGetCurrentProcessId();
		}
		return 0;
	}

	auto SysPidInfo::findProcessInfo(PROCESSENTRY32& procEntry, const DWORD aPid) const -> bool {  // NOLINT
		if (INVALID_HANDLE_VALUE == _hSnapShot){
			return false;
		}
		procEntry.dwSize = sizeof PROCESSENTRY32;
		if (_fnProcess32First(_hSnapShot, &procEntry)){
			do{
				if (aPid == procEntry.th32ProcessID){
					return true;
				}
			} while (_fnProcess32Next(_hSnapShot, &procEntry));
		}
		return false;
	}

	auto SysPidInfo::getParentPID() const -> boost::optional<DWORD> {
		return getParentPID(getMyPID());
	};

	auto SysPidInfo::getParentPID(const DWORD aPid) const -> boost::optional<DWORD> {
		PROCESSENTRY32 procEntry;
		if (!findProcessInfo(procEntry, aPid)){
			return boost::none;
		}
		return procEntry.th32ParentProcessID;
	}

	auto SysPidInfo::getName() const -> boost::optional<std::tstring> {
		return getName(getMyPID());
	}

	auto SysPidInfo::getName(const DWORD aPid) const -> boost::optional<std::tstring> {
		PROCESSENTRY32 procEntry;
		if (!findProcessInfo(procEntry, aPid)){
			return boost::none;
		}
		return procEntry.szExeFile;
	}

	auto SysPidInfo::getExeName() const -> boost::optional<boost::filesystem::path> {
		return getExeName(getMyPID());
	}

	auto SysPidInfo::getExeName(const DWORD aPid) const -> boost::optional<boost::filesystem::path> {
		TCHAR moduleName[ MAX_PATH ];
		auto isValid = false;
		if (!_fnOpenProcess || !_fnEnumProcessModules || !_fnGetModuleFileNameEx){
			return boost::none;
		}
		auto hProcess = _fnOpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aPid);
		if (hProcess == nullptr){
			return boost::none;
		}
		HMODULE hMod;
		DWORD cbNeeded = 0;
		auto Ok = _fnEnumProcessModules(hProcess, &hMod, sizeof hMod, &cbNeeded);
		isValid = true;
		if (!Ok && ERROR_PARTIAL_COPY == GetLastError()){
			// http://winprogger.com/getmodulefilenameex-enumprocessmodulesex-failures-in-wow64
			if (_fnGetProcessImageFileName(hProcess, moduleName, _countof(moduleName))){
				if (_fnQueryDosDevice){
					// convert something like "\Device\HarddiskVolume2"	to "C:"
					TCHAR devPath[ MAX_PATH ];
					TCHAR devName[ MAX_PATH ] = _T("_:");
					for (auto aDrive = _T('A'); _T('Z') > aDrive; ++aDrive){
						devName[ 0 ] = aDrive;
						if (_fnQueryDosDevice(devName, devPath, _countof(devPath))){
							auto len = _tcslen(devPath);
							if (MAX_PATH - 2 > len){
								devPath[ len ] = _T('\\');
								devPath[ len + 1 ] = _T('\0');
								if (boost::istarts_with(moduleName, devPath)){
									wcscat_s(devName, moduleName + len);
									wcscpy_s(moduleName, devName);
									break;
								}
							}
						}
					}
				}
				isValid = true;
			}
		}
		if (Ok){
			if (_fnGetModuleFileNameEx(hProcess, hMod, moduleName, _countof(moduleName))){
				isValid = true;
			}
		}
		CloseHandle(hProcess);
		return isValid
			       ? boost::optional<boost::filesystem::path>(moduleName)
			       : boost::none;
	}

	auto SysPidInfo::getDllName(const std::tstring dllName) const -> boost::optional<boost::filesystem::path> {
		return getDllName(dllName, getMyPID());
	}

	auto SysPidInfo::getDllName(const std::tstring dllName, const DWORD aPid) const -> boost::optional<boost::filesystem::path> {
		if (!_fnOpenProcess || !_fnEnumProcessModules || !_fnGetModuleFileNameEx){
			return boost::none;
		}
		auto hProcess = _fnOpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aPid);
		if (hProcess == nullptr){
			return boost::none;
		}
		boost::optional<boost::filesystem::path> result = boost::none;
		DWORD cbNeeded = 0;
		if (_fnEnumProcessModules(hProcess, nullptr, 0, &cbNeeded)){
			DWORD nEntries = cbNeeded / sizeof(HMODULE);
			std::unique_ptr<HMODULE[ ]> hMod(new(std::nothrow) HMODULE[ nEntries ]);

			if (hMod){
				if (_fnEnumProcessModules(hProcess, hMod.get(), nEntries * sizeof(hMod[ 0 ]), &cbNeeded)){ // probably nEntries * sizeof( *hMod ) == cbNeeded, but ...
					for (DWORD idx = 0; nEntries > idx; ++idx){
						TCHAR processName[ MAX_PATH ];
						if (_fnGetModuleFileNameEx(hProcess, hMod[ idx ], processName, _countof(processName))){
							boost::filesystem::path dll(processName);
							if (boost::iequals(dll.filename().c_str(), dllName)){
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

	auto SysPidInfo::isWow64() const -> bool {
#ifdef _WIN64
		return true;
#else
	return isWow64(getMyPID());
#endif
	}

	auto SysPidInfo::isWow64(const DWORD aPid) const -> bool {
		if (!_fnIsWow64Process || !_fnOpenProcess){
			return false;
		}
		auto hProcess = _fnOpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aPid);
		if (hProcess == nullptr){
			return false;
		}
		auto isWow64 = FALSE;
		if (!_fnIsWow64Process(hProcess, &isWow64)){
			isWow64 = FALSE;
		}
		CloseHandle(hProcess);
		return (isWow64
			        ? true
			        : false);
	}

	auto SysPidInfo::GetPriorityClass() const -> DWORD {
		return GetPriorityClass(getMyPID());
	}

	auto SysPidInfo::GetPriorityClass(const DWORD aPid) const -> DWORD {
		if (!_fnGetPriorityClass || !_fnOpenProcess){
			return NORMAL_PRIORITY_CLASS;
		}
		auto hProcess = _fnOpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aPid);
		if (hProcess == nullptr){
			return NORMAL_PRIORITY_CLASS;
		}
		auto result = _fnGetPriorityClass(hProcess);
		CloseHandle(hProcess);
		if (!result){
			result = NORMAL_PRIORITY_CLASS;
		}
		return result;
	}

	auto SysPidInfo::SetPriorityClass(HANDLE hProcess, DWORD dwPriorityClass) const -> void {
		if (_fnSetPriorityClass){
			_fnSetPriorityClass(hProcess, dwPriorityClass);
		}
	}

	auto SysPidInfo::ResumeThread(HANDLE hThread) const -> bool {
		if (!_fnResumeThread){
			SetLastError(ERROR_INVALID_FUNCTION);
			return FALSE;
		}
		auto result = _fnResumeThread(hThread);
		return static_cast<DWORD>(-1) != result;
	};

	auto SysPidInfo::hasConsole() const -> HWND {
		if (!_fnGetConsoleWindow){
			return nullptr;
		}
		return _fnGetConsoleWindow();
	}

	auto SysPidInfo::ownsConsole() const -> bool {
		if (_fnGetWindowThreadProcessId && _fnGetCurrentProcessId && _fnGetConsoleWindow){
			auto consoleWnd = _fnGetConsoleWindow();
			DWORD dwProcessId;
			_fnGetWindowThreadProcessId(consoleWnd, &dwProcessId);
			if (GetCurrentProcessId() != dwProcessId){
				return false;
			}
		}
		if (!_isatty(_fileno(stdout)) || !_isatty(_fileno(stdin)) || !_isatty(_fileno(stderr))){
			return false;
		}
		return true;
	}

	auto SysPidInfo::getCommandLine(const DWORD aPid, std::vector<std::tstring>& args) const -> bool {
		if (!_fnNtQueryInformationProcess || !_fnReadProcessMemory || !_fnOpenProcess){
			return false;
		}
		auto hProcess = _fnOpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aPid);
		if (hProcess == nullptr){
			return false;
		}
		auto result = false;
		PROCESS_BASIC_INFORMATION ProcessInfo;
		auto status = _fnNtQueryInformationProcess(hProcess, ProcessBasicInformation, &ProcessInfo, sizeof ProcessInfo, nullptr);
		if (STATUS_SUCCESS == status){
			PEB Peb;
			if (_fnReadProcessMemory(hProcess, ProcessInfo.PebBaseAddress, &Peb, sizeof Peb, nullptr)){
				RTL_USER_PROCESS_PARAMETERS ProcParam;
				if (_fnReadProcessMemory(hProcess, Peb.ProcessParameters, &ProcParam, sizeof ProcParam, nullptr)){
					LPVOID lpAddress = ProcParam.CommandLine.Buffer;
					auto len = ProcParam.CommandLine.MaximumLength;
					std::unique_ptr<TCHAR[ ]> buf(new(std::nothrow) TCHAR[ len + 1 ]);
					if (buf && _fnReadProcessMemory(hProcess, lpAddress, buf.get(), (len + 1) * sizeof buf[ 0 ], nullptr)){
						LPWSTR* szArglist;
						int nArgs;
						szArglist = CommandLineToArgvW(buf.get(), &nArgs);
						if (szArglist){
							result = true;
							for (auto idx = 0; nArgs > idx; idx++){
								args.push_back(szArglist[ idx ]);
							}
						}
						LocalFree(szArglist);
					}
				}
			}
		}
		CloseHandle(hProcess);
		return result;
	}

	auto SysPidInfo::GetStartupInfo(LPSTARTUPINFO lpStartupInfo) const -> BOOL {
		if (!_fnGetStartupInfo){
			return FALSE;
		}
		try{
			_fnGetStartupInfo(lpStartupInfo);
		} catch (...){
			lpStartupInfo = nullptr;
			return FALSE;
		}
		return TRUE;
	}

	auto SysPidInfo::CreateProcess(LPCTSTR lpApplicationName, LPTSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
	                               LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) const -> BOOL {
		if (!_fnCreateProcess){
			SetLastError(ERROR_INVALID_FUNCTION);
			return FALSE;
		}
		return _fnCreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}

	SysPidInfo _sysPidInfo;
}
