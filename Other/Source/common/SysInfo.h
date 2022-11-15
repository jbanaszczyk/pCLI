// *************************************************************
// * Copyright 2011 jacek.banaszczyk@gmail.com                 *
// * Part of pCli project: https://github.com/jbanaszczyk/pCli *
// *************************************************************

#pragma once

#include "./common.h"

namespace SysInfo {

	/******************************************************************************
	 *
	 *   getExeName()
	 *     returns full path of the current process executable
	 *     getExeName() tries to resolve 64-bit processes queried from 32-bit app
	 *     but in some cases it returns \Device\Partition notation
	 *
	 *****************************************************************************/
	std::optional<std::filesystem::path> getExeName();

	/******************************************************************************
	 *
	 *   getDllName( dllName )
	 *     returns full path of module/dll loaded by process
	 *       getDllName( "kernel32.dll" ) returns c:\windows\system32\kernel32.dll
	 *     method is provided to check if particular dll is loaded by given process
	 *       especially ConEmu (conEmuHk.dll)
	 *
	 *****************************************************************************/
	std::optional<std::filesystem::path> getDllName(const std::wstring& dllName);

	/******************************************************************************
	 *
	 *   bool isWow64()
	 *     check if process is running on Wow64
	 *     BTW: 64-bit processes running on 64-bit platform ar not using Wow64
	 *
	 *****************************************************************************/
	bool isWow64();

	/******************************************************************************
	 *
	 * Methods:
	 *   GetProcessPriorityClass()
	 *     Open current process and retrieves its PriorityClass
	 *
	 *****************************************************************************/
	DWORD getProcessPriorityClass();

	/******************************************************************************
	 *
	 *   ownsConsole()
	 *     returns true,
	 *       if current process has allocated console
	 *       and console was created by current process
	 *
	 *****************************************************************************/
	bool ownsConsole();
}
