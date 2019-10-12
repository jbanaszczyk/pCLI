/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

/******************************************************************************
 *
 * global pApps::SysPidInfo sysPidInfo allows queries about processes using PID
 *
 * Methods:
 *   getExeName()
 *     returns full path of the process executable
 *     for example: c:\windows\system32\cmd.exe
 *     getExeName() tries to resolve 64-bit processes queried from 32-bit app
 *     but in some cases it returns \Device\Partition notation
 *   SysPidInfo::hasInjectedDll ( dllName [, PID] )
 *     check if dll loaded by process
 *     method is provided to check if particular dll is loaded by given process
 *       especially ConEmu (conEmuHk.dll)
 *   bool isWow64()
 *     check if process is running on Wow64
 *     (!) 64-bit processes running on 64-bit platform are not using Wow64
 *   hasConsole()
 *     returns true, if current process has allocated console
 *   ownsConsole()
 *     returns true,
 *       if current process has allocated console
 *       and console was created by current process
 *
 *****************************************************************************/
#include "./common.h"
#include "UniquePointers.h"

namespace p_apps {

    class ISysPidInfo {
    public:
        virtual ~ISysPidInfo() = default;
    private:
        virtual boost::filesystem::path getExeName(const TCHAR *argv0) = 0;
        virtual bool                    hasInjectedDll(const std::tstring &wantedDllName) = 0;
        virtual bool                    isWow64() = 0;
        virtual DWORD                   getPriorityClass() = 0;
        virtual void                    setPriorityClass(HANDLE hProcess, DWORD dwPriorityClass) = 0;  // NOLINT(misc-misplaced-const)
        virtual bool                    resumeThread(HANDLE hThread) = 0;  // NOLINT(misc-misplaced-const)
        virtual bool                    hasConsole() = 0;
        virtual bool                    ownsConsole() = 0;
        virtual bool                    getStartupInfo(LPSTARTUPINFO lpStartupInfo) = 0;
        virtual bool                    createProcess(LPCTSTR lpApplicationName, LPTSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) = 0;  // NOLINT(misc-misplaced-const)
    };

    class SysPidInfo : public ISysPidInfo {
    public:

        boost::filesystem::path getExeName(const TCHAR *argv0) override {

            const DWORD                    MAX_NAME_SIZE = MAX_PATH;
            const std::unique_ptr<TCHAR[]> moduleName(new(std::nothrow) TCHAR[MAX_NAME_SIZE]);
            if (moduleName == nullptr) {
                return boost::filesystem::path(argv0);
            }

            const auto hProcess = handleUniquePtr(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId()));
            if (hProcess != nullptr) {
                HMODULE hModule;
                DWORD   cbNeeded = 0;
                if (EnumProcessModules(hProcess.get(), &hModule, sizeof hModule, &cbNeeded)) {
                    if (GetModuleFileNameEx(hProcess.get(), hModule, moduleName.get(), MAX_NAME_SIZE)) {
                        return boost::filesystem::path(moduleName.get());
                    }
                }
            }
            return boost::filesystem::path(argv0);
        }

        bool hasInjectedDll(const std::tstring &wantedDllName) override {
            const auto hProcess = handleUniquePtr(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId()));
            if (hProcess == nullptr) {
                return false;
            }

            DWORD cbNeeded = 0;
            if (!EnumProcessModules(hProcess.get(), nullptr, 0, &cbNeeded)) {
                return false;
            }
            const auto                       modulesCount = cbNeeded / sizeof(HMODULE);
            const std::unique_ptr<HMODULE[]> modules(new(std::nothrow) HMODULE[modulesCount]);
            if (modules == nullptr) {
                return false;
            }
            if (!EnumProcessModules(hProcess.get(), modules.get(), modulesCount * sizeof(HMODULE), &cbNeeded)) {
                return false;
            }

            const DWORD                    MAX_NAME_SIZE = MAX_PATH;
            const std::unique_ptr<TCHAR[]> processNameBuffer(new(std::nothrow) TCHAR[MAX_NAME_SIZE]);
            if (processNameBuffer == nullptr) {
                return false;
            }

            for (DWORD idx = 0; modulesCount > idx; ++idx) {
                if (GetModuleFileNameEx(hProcess.get(), modules[idx], processNameBuffer.get(), MAX_NAME_SIZE)) {
                    boost::filesystem::path dll(processNameBuffer.get());
                    if (boost::equals(dll.filename().c_str(), wantedDllName)) {
                        return true;
                    }
                }
            }
            return false;
        }

        bool isWow64() override {
#ifdef _WIN64
			return true;
#else
            const auto hProcess = handleUniquePtr(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId()));
            if (hProcess == nullptr) {
                return false;
            }
            auto isWow64 = FALSE;
            if (!IsWow64Process(hProcess.get(), &isWow64)) {
                return false;
            }

            return isWow64 == TRUE;
#endif
        }

        DWORD getPriorityClass() override {
            const auto hProcess = handleUniquePtr(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId()));
            if (hProcess == nullptr) {
                return NORMAL_PRIORITY_CLASS;
            }
            const auto priority = GetPriorityClass(hProcess.get());
            return priority != 0
                       ? priority
                       : NORMAL_PRIORITY_CLASS;
        }

        void setPriorityClass(const HANDLE hProcess, const DWORD dwPriorityClass) override {  // NOLINT(misc-misplaced-const)
            SetPriorityClass(hProcess, dwPriorityClass);
        }

        bool resumeThread(const HANDLE hThread) override {  // NOLINT(misc-misplaced-const)
            const auto result = ResumeThread(hThread);
            return static_cast<DWORD>(-1) != result;
        }

        bool hasConsole() override {
            return GetConsoleWindow() != nullptr;
        }

        bool ownsConsole() override {
            const auto currentConsoleWindow = GetConsoleWindow();
            if (currentConsoleWindow != nullptr) {
                DWORD dwProcessId;
                GetWindowThreadProcessId(currentConsoleWindow, &dwProcessId);
                if (dwProcessId != GetCurrentProcessId()) {
                    return false;
                }
            }
            return _isatty(_fileno(stdout)) && _isatty(_fileno(stdin)) && _isatty(_fileno(stderr));
        }
        
        bool getStartupInfo(const LPSTARTUPINFO lpStartupInfo) override {
#ifdef UNICODE
            GetStartupInfoW(lpStartupInfo);
            return true;
#else
			try {
				GetStartupInfoA(lpStartupInfo);
			}
			catch (...) {
				lpStartupInfo = nullptr;
				return false;
			}
			return true;
#endif
        }

        bool createProcess(const LPCTSTR lpApplicationName, const LPTSTR lpCommandLine, const LPSECURITY_ATTRIBUTES lpProcessAttributes, const LPSECURITY_ATTRIBUTES lpThreadAttributes, const BOOL bInheritHandles, const DWORD dwCreationFlags, const LPVOID lpEnvironment, const LPCTSTR lpCurrentDirectory, const LPSTARTUPINFO lpStartupInfo, const LPPROCESS_INFORMATION lpProcessInformation) override {  // NOLINT(misc-misplaced-const)
            return 0 != CreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
        }
    };

    extern SysPidInfo sysPidInfo;
}
