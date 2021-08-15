/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

/******************************************************************************
 *
 * This file is included by stdafx.h (so, it is included in all *.cpp sources)
 *
 * There are some global symbol redefinitions (MAX_PATH, _HEAP_MAXREQ)
 *   so ./common.h, should be included in all *.h files too
 *
 * put there all common includes, #define's etc.
 *
 *****************************************************************************/

/** Compiler and environment ************************************************************/

#ifndef _MSC_VER
#error "COMPILER IS NOT SUPPORTED"
#endif

#ifndef _WIN32
#error "OS IS NOT SUPPORTED"
#endif

#define WINVER         0x0601  // _WIN32_WINNT_WIN7 
#define _WIN32_WINDOWS 0x0601  // _WIN32_WINNT_WIN7 
#define _WIN32_WINNT   0x0601  // _WIN32_WINNT_WIN7 
#include <SDKDDKVer.h>

#define SECURITY_WIN32

// prevent definition of min and max
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
#endif

#pragma comment(lib,"Secur32.lib")	// GetUserNameExW
#pragma comment(lib,"mpr.lib")	    // WNetGetUniversalNameW

/** Mandatory #include *****************************************************************/

//#include "LauncherVersion.h"

/** Common headers *********************************************************************/

#include <assert.h>
#include <windows.h>
#include <tchar.h>
#include <set>
#include <map>
#include <vector>
#include <tlhelp32.h>
#include <psapi.h>
#include <functional>
#include <utility>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <Winternl.h>
#include <Shellapi.h>
#include <Security.h>
#include <process.h>
#include <Winnetwk.h>
#include <tchar.h>

#include <boost/optional.hpp>
#include <boost/locale.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

using namespace std::string_literals;

#ifdef _INC_TCHAR
namespace std {
	using tstring = std::basic_string<TCHAR>;
	using tifstream = std::basic_ifstream<TCHAR>;
}

#ifndef _UNICODE
		#define _tcout cout
		#define _tcerr cerr
		#define _tcin  cin
		#define _tformat format			// to be used with boost:: format
		#define _tstring string			// to be used with boost:: filesystem
		#define _texecve _execve
		#define _tspawnve _spawnve
#else
#define _tcout wcout
#define _tcerr wcerr
#define _tcin  wcin
#define _tformat wformat		// to be usedx with boost:: format
#define _tstring wstring		// to be used with boost:: filesystem
#define _texecve _wexecve
#define _tspawnve _wspawnve
#endif
#endif

#ifdef MAX_PATH
#undef MAX_PATH
#define MAX_PATH 32767
#endif

#ifdef _HEAP_MAXREQ
// real maximum allocation size is a bit les, than _HEAP_MAXREQ
#ifdef _DEBUG
#ifdef _WIN64
			const size_t heapMaxReqReal = _HEAP_MAXREQ - 52;
#else
			const size_t heapMaxReqReal = _HEAP_MAXREQ - 36;
#endif
#else
const size_t heapMaxReqReal = _HEAP_MAXREQ;
#endif

#ifdef _WIN64
#define GWL_WNDPROC_6432 GWLP_WNDPROC
#else
		#define GWL_WNDPROC_6432 GWL_WNDPROC
#endif

#endif

#ifndef LOCALE_SNAME
	#define LOCALE_SNAME                  0x0000005c   // since WINVER >= 0x0600 : (use with care) locale name (ie: en-us)
#endif

#ifndef LOCALE_SISO639LANGNAME2
	#define LOCALE_SISO639LANGNAME2       0x00000067   // since WINVER >= 0x0600 : (use with care) 3 character ISO abbreviated language name, eg "eng"
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS	((NTSTATUS)0x00000000L)  // from of NtStatus.h
#endif

/***************************************************************************************/
