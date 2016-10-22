/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#ifndef _C975DB66_8F15_4C20_8875_D254869E715B_INCLUDED
#define _C975DB66_8F15_4C20_8875_D254869E715B_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif

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

#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <Winternl.h>
#include <Shellapi.h>

#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace pApps {

	class SysPidInfo {
		private:
			typedef BOOL     ( WINAPI *__CREATE_PROCESS )               ( LPCTSTR, LPTSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCTSTR, LPSTARTUPINFO, LPPROCESS_INFORMATION );
			typedef BOOL     ( WINAPI *__ENUM_PROCESS_MODULES )         ( HANDLE, HMODULE *, DWORD, LPDWORD );
			typedef BOOL     ( WINAPI *__IS_WOW64_PROCESS )             ( HANDLE, PBOOL );
			typedef BOOL     ( WINAPI *__PROCESS32_FIRST )              ( HANDLE, LPPROCESSENTRY32 );
			typedef BOOL     ( WINAPI *__PROCESS32_NEXT )               ( HANDLE, LPPROCESSENTRY32 );
			typedef BOOL     ( WINAPI *__READ_PROCESS_MEMORY )          ( HANDLE, LPCVOID, LPVOID, SIZE_T, SIZE_T * );
			typedef BOOL     ( WINAPI *__SET_PRIORITY_CLASS )           ( HANDLE, DWORD );
			typedef DWORD    ( WINAPI *__GETCURRENT_PROCESS_ID )        ( VOID );
			typedef DWORD    ( WINAPI *__GETMODULE_FILE_NAME_EX )       ( HANDLE, HMODULE, LPTSTR, DWORD );
			typedef DWORD    ( WINAPI *__GETPROCESS_IMAGE_FILE_NAME )   ( HANDLE, LPTSTR, DWORD ) ;
			typedef DWORD    ( WINAPI *__GET_PRIORITY_CLASS )           ( HANDLE );
			typedef DWORD    ( WINAPI *__GET_WINDOW_THREAD_PROCESS_ID ) ( HWND, LPDWORD );
			typedef DWORD    ( WINAPI *__QUERY_DOS_DEVICE )             ( LPCTSTR, LPTSTR, DWORD );
			typedef DWORD    ( WINAPI *__RESUME_THREAD)                 ( HANDLE );
			typedef HANDLE   ( WINAPI *__CREATE_TOOLHELP32_SNAPSHOT )   ( DWORD, DWORD );
			typedef HANDLE   ( WINAPI *__OPEN_PROCESS )                 ( DWORD, BOOL, DWORD );
			typedef HWND     ( WINAPI *__GET_CONSOLE_WINDOW )           ( VOID );
			typedef NTSTATUS ( WINAPI *__NT_QUERY_INFORMATION_PROCESS ) ( HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG );
			typedef VOID     ( WINAPI *__GET_STARTUP_INFO )             ( LPSTARTUPINFO );

			__CREATE_PROCESS               _fnCreateProcess;
			__CREATE_TOOLHELP32_SNAPSHOT   _fnCreateToolhelp32Snapshot;
			__ENUM_PROCESS_MODULES         _fnEnumProcessModules;
			__GETCURRENT_PROCESS_ID        _fnGetCurrentProcessId;
			__GETMODULE_FILE_NAME_EX       _fnGetModuleFileNameEx;
			__GETPROCESS_IMAGE_FILE_NAME   _fnGetProcessImageFileName;
			__GET_CONSOLE_WINDOW           _fnGetConsoleWindow;
			__GET_PRIORITY_CLASS           _fnGetPriorityClass;
			__GET_STARTUP_INFO             _fnGetStartupInfo;
			__GET_WINDOW_THREAD_PROCESS_ID _fnGetWindowThreadProcessId;
			__IS_WOW64_PROCESS             _fnIsWow64Process;
			__NT_QUERY_INFORMATION_PROCESS _fnNtQueryInformationProcess;
			__OPEN_PROCESS                 _fnOpenProcess;
			__PROCESS32_FIRST              _fnProcess32First;
			__PROCESS32_NEXT               _fnProcess32Next;
			__QUERY_DOS_DEVICE             _fnQueryDosDevice;
			__READ_PROCESS_MEMORY          _fnReadProcessMemory;
			__RESUME_THREAD                _fnResumeThread;
			__SET_PRIORITY_CLASS           _fnSetPriorityClass;

			HINSTANCE _hInstDllKernel;
			HINSTANCE _hInstDllPsApi;
			HINSTANCE _hInstDllNtdll;
			HINSTANCE _hInstDllUser;

			HANDLE _hSnapShot;
		public:
			SysPidInfo();
			~SysPidInfo();
		public:
			DWORD getMyPID() const;
			boost::optional<DWORD> getParentPID () const;
			boost::optional<DWORD> getParentPID ( const DWORD aPid ) const;
			boost::optional<std::tstring> getName () const;
			boost::optional<std::tstring> getName ( const DWORD aPid ) const;
			boost::optional<boost::filesystem::path> getExeName () const;
			boost::optional<boost::filesystem::path> getExeName ( const DWORD aPid ) const;
			boost::optional<boost::filesystem::path> getDllName ( const std::tstring dllName ) const;
			boost::optional<boost::filesystem::path> getDllName ( const std::tstring dllName, const DWORD aPid ) const;
			bool isWow64() const;
			bool isWow64 ( const DWORD aPid ) const;

			DWORD GetPriorityClass() const;
			DWORD GetPriorityClass( const DWORD aPid ) const;

			void SetPriorityClass( HANDLE hProcess, DWORD dwPriorityClass ) const;
			bool ResumeThread ( HANDLE hThread ) const;

			HWND hasConsole() const;
			bool ownsConsole() const;
			bool getCommandLine ( const DWORD aPid, std::vector<std::tstring>& args ) const;

			BOOL GetStartupInfo( LPSTARTUPINFO lpStartupInfo ) const;
			BOOL CreateProcess( LPCTSTR lpApplicationName, LPTSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation ) const;
		private:
			void clear(void);
			bool findProcessInfo ( PROCESSENTRY32& procEntry, const DWORD aPid ) const;
			FARPROC SysPidInfo::tGetProcAddress ( const HMODULE hModule, const LPCSTR lpProcName ) const;
	};

	extern SysPidInfo _sysPidInfo;
}

#endif  // _C975DB66_8F15_4C20_8875_D254869E715B_INCLUDED
