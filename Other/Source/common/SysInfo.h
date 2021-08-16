/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

 /******************************************************************************
  * 
  * Methods:
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
  *   ownsConsole()
  *     returns true,
  *       if current process has allocated console
  *       and console was created by current process
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
