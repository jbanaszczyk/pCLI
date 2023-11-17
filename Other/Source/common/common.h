// *************************************************************
// * Copyright 2011 jacek.banaszczyk@gmail.com                 *
// * Part of pCli project: https://github.com/jbanaszczyk/pCli *
// *************************************************************

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
#define _WIN32_WINDOWS 0x0601  // _WIN32_WINNT_WIN7     // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)   
#define _WIN32_WINNT   0x0601  // _WIN32_WINNT_WIN7     // NOLINT(clang-diagnostic-reserved-macro-identifier)

#define SECURITY_WIN32

// prevent definition of min and max
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC    // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
#include <crtdbg.h>
#endif

#pragma comment(lib,"Secur32.lib")	// GetUserNameExW
#pragma comment(lib,"mpr.lib")	    // WNetGetUniversalNameW
#pragma comment( lib, "winmm.lib")  // timeGetTime 
#pragma comment( lib, "Userenv.lib")  // timeGetTime 

/** Common headers *********************************************************************/

// ReSharper disable CppUnusedIncludeDirective

#include <windows.h>
#include <tchar.h>
#include <set>
#include <map>
#include <vector>
#include <TlHelp32.h>
#include <psapi.h>
#include <io.h>
#include <iomanip>
#include <fcntl.h>
#include <cerrno>
#include <Shellapi.h>
#include <Security.h>
#include <process.h>
#include <Winnetwk.h>
#include <timeapi.h>
#include <locale>
#include <cstdlib>
#include <optional>
#include <filesystem>
#include <random>
#include <userenv.h>

#include <boost/locale.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>

// ReSharper restore CppUnusedIncludeDirective

#if defined _DEBUG
constexpr bool C_DEBUG = true;
#else
constexpr bool C_DEBUG = false;
#endif

#ifdef MAX_PATH
#undef MAX_PATH
#define MAX_PATH 32767
#endif

#ifdef _HEAP_MAXREQ
// real maximum allocation size is a bit less, than _HEAP_MAXREQ
#ifdef _DEBUG
#ifdef _WIN64
constexpr size_t heapMaxReqReal = _HEAP_MAXREQ - 52;
#else
constexpr size_t heapMaxReqReal = _HEAP_MAXREQ - 36;
#endif
#else
constexpr size_t heapMaxReqReal = _HEAP_MAXREQ;
#endif
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS	((NTSTATUS)0x00000000L)  // from of NtStatus.h
#endif

/***************************************************************************************/
