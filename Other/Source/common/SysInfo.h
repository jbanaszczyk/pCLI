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

	class SysInfo {
		auto findProcessInfo(PROCESSENTRY32& procEntry, DWORD aPid) const -> bool;
	public:
		static auto getExeName()->boost::optional<boost::filesystem::path>;
		static auto getDllName(std::tstring dllName)->boost::optional<boost::filesystem::path>;
		static auto isWow64() -> bool;
		static auto GetProcessPriorityClass()->DWORD;
		static auto ownsConsole() -> bool;

	};
}
