/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#include "stdafx.h"
#include "./SysInfo.h"

namespace sys_info {

	class SmartHandle {
		HANDLE handle;
	public:
		SmartHandle(std::nullptr_t = nullptr) : handle(nullptr) {}
		SmartHandle(HANDLE value) : handle(value == INVALID_HANDLE_VALUE ? nullptr : value) {}

		explicit operator bool() const { return handle != nullptr; }
		operator HANDLE() const { return handle; }

		friend bool operator ==(SmartHandle l, SmartHandle r) { return l.handle == r.handle; }
		friend bool operator !=(SmartHandle l, SmartHandle r) { return !(l == r); }

		struct Deleter {
			typedef SmartHandle pointer;
			void operator()(SmartHandle handle) const {
				CloseHandle(handle);
			}
		};
	};

	inline bool operator ==(HANDLE l, SmartHandle r) { return SmartHandle(l) == r; }
	inline bool operator !=(HANDLE l, SmartHandle r) { return !(l == r); }
	inline bool operator ==(SmartHandle l, HANDLE r) { return l == SmartHandle(r); }
	inline bool operator !=(SmartHandle l, HANDLE r) { return !(l == r); }

	using SmartHandlePtr = std::unique_ptr<SmartHandle, SmartHandle::Deleter>;

	template< typename T >
	class ArrayDeleter {
		void operator ()(T const* p) {
			delete[] p;
		}
	};

	auto getExeName()  -> boost::optional<boost::filesystem::path> {
		const int moduleNameSize = MAX_PATH;
		std::unique_ptr<TCHAR[]> moduleName(new (std::nothrow) TCHAR[moduleNameSize]);
		if (!moduleName) {
			return boost::none;
		}

		SmartHandlePtr hProcess(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId()));
		if (!hProcess) {
			return boost::none;
		}

		HMODULE hMod;
		DWORD cbNeeded = 0;
		if (EnumProcessModules(hProcess.get(), &hMod, sizeof hMod, &cbNeeded)) {
			if (GetModuleFileNameEx(hProcess.get(), hMod, moduleName.get(), moduleNameSize)) {
				return boost::optional<boost::filesystem::path>(moduleName.get());
			}
		}

		return  boost::none;
	}

	auto getDllName(const std::tstring dllName)  -> boost::optional<boost::filesystem::path> {
		const int processNameSize = MAX_PATH;
		std::unique_ptr<TCHAR[]> processName(new (std::nothrow) TCHAR[processNameSize]);
		if (!processName) {
			return boost::none;
		}

		SmartHandlePtr hProcess(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId()));
		if (!hProcess) {
			return boost::none;
		}
		boost::optional<boost::filesystem::path> result = boost::none;
		DWORD cbNeeded = 0;
		if (EnumProcessModules(hProcess.get(), nullptr, 0, &cbNeeded)) {
			DWORD nEntries = cbNeeded / sizeof(HMODULE);
			std::unique_ptr<HMODULE[]> hMod(new(std::nothrow) HMODULE[nEntries]);
			if (!hMod) {
				return boost::none;
			}
			if (EnumProcessModules(hProcess.get(), hMod.get(), nEntries * sizeof(hMod[0]), &cbNeeded)) {
				// probably nEntries * sizeof( *hMod ) == cbNeeded, but ...
				for (decltype(nEntries) idx = 0; idx < nEntries; ++idx) {
					if (GetModuleFileNameEx(hProcess.get(), hMod[idx], processName.get(), processNameSize)) {
						boost::filesystem::path dll(processName.get());
						if (boost::iequals(dll.filename().c_str(), dllName)) {
							return boost::optional<boost::filesystem::path>(dll);
						}
					}
				}
			}
		}
		return boost::none;
	}

	auto isWow64()  -> bool {
#ifdef _WIN64
		return true;
#else
		SmartHandlePtr hProcess(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId()));
		if (!hProcess) {
			return false;
		}
		auto Wow64Process = FALSE;
		if (IsWow64Process(hProcess.get(), &Wow64Process) != 0) {
			return Wow64Process != FALSE;
		}

		return false;
#endif
	}

	auto GetProcessPriorityClass()  -> DWORD {
		SmartHandlePtr hProcess(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId()));
		if (!hProcess) {
			return NORMAL_PRIORITY_CLASS;
		}
		auto result = GetPriorityClass(hProcess.get());
		if (!result) {
			result = NORMAL_PRIORITY_CLASS;
		}
		return result;
	}

	auto ownsConsole()  -> bool {
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
