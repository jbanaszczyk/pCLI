/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

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
 *   hasConsole()
 *     returns true, if current process has allocated console
 *   ownsConsole()
 *     returns true,
 *       if current process has allocated console
 *       and console was created by current process
 *   bool getCommandLine
 *     retrieves whole command line of the given process
 *     not guaranted.
 *     method fails if called from 32-bit app to query 64-bit process
 *
 *   GetStartupInfo
 *   CreateProcess
 *     Wrappers for WinAPI, but uses dynamic function address
 *
 *****************************************************************************/
#include "./common.h"

namespace p_apps {

	class SysPidInfo {
		private:
			using __CREATE_PROCESS = BOOL( WINAPI *)(LPCTSTR, LPTSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCTSTR, LPSTARTUPINFO, LPPROCESS_INFORMATION);
			using __ENUM_PROCESS_MODULES = BOOL( WINAPI *)(HANDLE, HMODULE*, DWORD, LPDWORD);
			using __IS_WOW64_PROCESS = BOOL( WINAPI *)(HANDLE, PBOOL);
			using __PROCESS32_FIRST = BOOL( WINAPI *)(HANDLE, LPPROCESSENTRY32);
			using __PROCESS32_NEXT = BOOL( WINAPI *)(HANDLE, LPPROCESSENTRY32);
			using __READ_PROCESS_MEMORY = BOOL( WINAPI *)(HANDLE, LPCVOID, LPVOID, SIZE_T, SIZE_T*);
			using __SET_PRIORITY_CLASS = BOOL( WINAPI *)(HANDLE, DWORD);
			using __GETCURRENT_PROCESS_ID = DWORD( WINAPI *)(VOID);
			using __GETMODULE_FILE_NAME_EX = DWORD( WINAPI *)(HANDLE, HMODULE, LPTSTR, DWORD);
			using __GETPROCESS_IMAGE_FILE_NAME = DWORD( WINAPI *)(HANDLE, LPTSTR, DWORD);
			using __GET_PRIORITY_CLASS = DWORD( WINAPI *)(HANDLE);
			using __GET_WINDOW_THREAD_PROCESS_ID = DWORD( WINAPI *)(HWND, LPDWORD);
			using __QUERY_DOS_DEVICE = DWORD( WINAPI *)(LPCTSTR, LPTSTR, DWORD);
			using __RESUME_THREAD = DWORD( WINAPI *)(HANDLE);
			using __CREATE_TOOLHELP32_SNAPSHOT = HANDLE( WINAPI *)(DWORD, DWORD);
			using __OPEN_PROCESS = HANDLE( WINAPI *)(DWORD, BOOL, DWORD);
			using __GET_CONSOLE_WINDOW = HWND( WINAPI *)(VOID);
			using __NT_QUERY_INFORMATION_PROCESS = NTSTATUS( WINAPI *)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
			using __GET_STARTUP_INFO = VOID( WINAPI *)(LPSTARTUPINFO);

			__CREATE_PROCESS _fnCreateProcess;
			__CREATE_TOOLHELP32_SNAPSHOT _fnCreateToolhelp32Snapshot;
			__ENUM_PROCESS_MODULES _fnEnumProcessModules;
			__GETCURRENT_PROCESS_ID _fnGetCurrentProcessId;
			__GETMODULE_FILE_NAME_EX _fnGetModuleFileNameEx;
			__GETPROCESS_IMAGE_FILE_NAME _fnGetProcessImageFileName;
			__GET_CONSOLE_WINDOW _fnGetConsoleWindow;
			__GET_PRIORITY_CLASS _fnGetPriorityClass;
			__GET_STARTUP_INFO _fnGetStartupInfo;
			__GET_WINDOW_THREAD_PROCESS_ID _fnGetWindowThreadProcessId;
			__IS_WOW64_PROCESS _fnIsWow64Process;
			__NT_QUERY_INFORMATION_PROCESS _fnNtQueryInformationProcess;
			__OPEN_PROCESS _fnOpenProcess;
			__PROCESS32_FIRST _fnProcess32First;
			__PROCESS32_NEXT _fnProcess32Next;
			__QUERY_DOS_DEVICE _fnQueryDosDevice;
			__READ_PROCESS_MEMORY _fnReadProcessMemory;
			__RESUME_THREAD _fnResumeThread;
			__SET_PRIORITY_CLASS _fnSetPriorityClass;

			HINSTANCE _hInstDllKernel;
			HINSTANCE _hInstDllPsApi;
			HINSTANCE _hInstDllNtdll;
			HINSTANCE _hInstDllUser;

			HANDLE _hSnapShot;
		public:
			SysPidInfo();
			~SysPidInfo();
		public:
			auto getMyPID() const -> DWORD;
			auto getParentPID() const -> boost::optional<DWORD>;
			auto getParentPID(DWORD aPid) const -> boost::optional<DWORD>;
			auto getName() const -> boost::optional<std::tstring>;
			auto getName(DWORD aPid) const -> boost::optional<std::tstring>;
			auto getExeName() const -> boost::optional<boost::filesystem::path>;
			auto getExeName(DWORD aPid) const -> boost::optional<boost::filesystem::path>;
			auto getDllName(std::tstring dllName) const -> boost::optional<boost::filesystem::path>;
			auto getDllName(std::tstring dllName, DWORD aPid) const -> boost::optional<boost::filesystem::path>;
			auto isWow64() const -> bool;
			auto isWow64(DWORD aPid) const -> bool;

			auto GetPriorityClass() const -> DWORD;
			auto GetPriorityClass(DWORD aPid) const -> DWORD;

			auto SetPriorityClass(HANDLE hProcess, DWORD dwPriorityClass) const -> void;
			auto ResumeThread(HANDLE hThread) const -> bool;

			auto hasConsole() const -> HWND;
			auto ownsConsole() const -> bool;
			auto getCommandLine(DWORD aPid, std::vector<std::tstring>& args) const -> bool;

			auto GetStartupInfo(LPSTARTUPINFO lpStartupInfo) const -> BOOL;
			auto CreateProcess(LPCTSTR lpApplicationName, LPTSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
			                   LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) const -> BOOL;
		private:
			auto clear(void) -> void;
			auto findProcessInfo(PROCESSENTRY32& procEntry, DWORD aPid) const -> bool;
			auto SysPidInfo::tGetProcAddress(HMODULE hModule, LPCSTR lpProcName) const -> FARPROC;
	};

	extern SysPidInfo _sysPidInfo;
}
