// *************************************************************
// * Copyright 2011 jacek.banaszczyk@gmail.com                 *
// * Part of pCli project: https://github.com/jbanaszczyk/pCli *
// *************************************************************

#include "stdafx.h"
#include "./SysInfo.h"

namespace SysInfo {

	class SmartHandle {
		HANDLE handle;
	public:
		explicit SmartHandle(std::nullptr_t = nullptr)
			: handle(nullptr) {
		}

		SmartHandle(const HANDLE value)
			: handle(value == INVALID_HANDLE_VALUE
				         ? nullptr
				         : value) {
		}

		explicit operator bool() const {
			return handle != nullptr;
		}

		operator HANDLE() const {
			return handle;
		}

		friend bool operator ==(const SmartHandle lhs, const SmartHandle rhs) {
			return lhs.handle == rhs.handle;
		}

		friend bool operator !=(const SmartHandle lhs, const SmartHandle rhs) {
			return !(lhs == rhs);
		}

		struct Deleter {
			using pointer = SmartHandle;

			void operator()(const SmartHandle closingHandle) const {
				CloseHandle(closingHandle);
			}
		};
	};

	inline bool operator ==(const HANDLE lha, const SmartHandle rha) {
		return SmartHandle(lha) == rha;
	}

	inline bool operator !=(const HANDLE lha, const SmartHandle rha) {
		return !(lha == rha);
	}

	inline bool operator ==(SmartHandle lha, HANDLE rha) {
		return lha == SmartHandle(rha);
	}

	inline bool operator !=(SmartHandle lha, HANDLE rha) {
		return !(lha == rha);
	}

	using SmartHandlePtr = std::unique_ptr<SmartHandle, SmartHandle::Deleter>;

	template <typename T>
	class ArrayDeleter {
		void operator ()(const T* p) {
			delete[] p;
		}
	};

	static auto OpenMyProcessForQuery() {
		return OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	}

	std::optional<std::filesystem::path> getExeName() {
		constexpr int moduleNameSize = MAX_PATH;
		const std::unique_ptr<wchar_t[]> moduleName(new(std::nothrow) wchar_t[moduleNameSize]);
		if (!moduleName) {
			return std::nullopt;
		}

		const SmartHandlePtr hProcess(OpenMyProcessForQuery());
		if (!hProcess) {
			return std::nullopt;
		}

		HMODULE hMod;
		DWORD cbNeeded = 0;
		if (EnumProcessModules(hProcess.get(), &hMod, sizeof (hMod), &cbNeeded)) {
			if (GetModuleFileNameEx(hProcess.get(), hMod, moduleName.get(), moduleNameSize)) {
				return std::optional<std::filesystem::path>{moduleName.get()};
			}
		}

		return std::nullopt;
	}

	std::optional<std::filesystem::path> getDllName(const std::wstring& dllName) {
		constexpr int processNameSize = MAX_PATH;
		const std::unique_ptr<wchar_t[]> processName(new(std::nothrow) wchar_t[processNameSize]);
		if (!processName) {
			return std::nullopt;
		}

		const SmartHandlePtr hProcess(OpenMyProcessForQuery());
		if (!hProcess) {
			return std::nullopt;
		}
		std::optional<std::filesystem::path> result = std::nullopt;
		DWORD cbNeeded = 0;
		if (EnumProcessModules(hProcess.get(), nullptr, 0, &cbNeeded)) {
			DWORD nEntries = cbNeeded / sizeof(HMODULE);
			const std::unique_ptr<HMODULE[]> hMod(new(std::nothrow) HMODULE[nEntries]);
			if (!hMod) {
				return std::nullopt;
			}
			if (EnumProcessModules(hProcess.get(), hMod.get(), nEntries * sizeof(hMod[0]), &cbNeeded)) {
				// probably nEntries * sizeof( *hMod ) == cbNeeded, but ...
				for (decltype(nEntries) idx = 0; idx < nEntries; ++idx) {
					if (GetModuleFileNameEx(hProcess.get(), hMod[idx], processName.get(), processNameSize)) {
						std::filesystem::path dll(processName.get());
						if (boost::iequals(dll.filename().c_str(), dllName)) {
							return std::optional{dll};
						}
					}
				}
			}
		}
		return std::nullopt;
	}

	bool isWow64() {
#ifdef _WIN64
		return true;
#else
		const SmartHandlePtr hProcess(OpenMyProcessForQuery());
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

	DWORD getProcessPriorityClass() {
		const SmartHandlePtr hProcess(OpenMyProcessForQuery());
		if (!hProcess) {
			return NORMAL_PRIORITY_CLASS;
		}
		auto result = GetPriorityClass(hProcess.get());
		if (!result) {
			result = NORMAL_PRIORITY_CLASS;
		}
		return result;
	}

	bool ownsConsole() {
		const auto consoleWindow = GetConsoleWindow();
		DWORD dwProcessId;
		GetWindowThreadProcessId(consoleWindow, &dwProcessId);
		if (GetCurrentProcessId() != dwProcessId) {
			return false;
		}

		if (!_isatty(_fileno(stdout)) || !_isatty(_fileno(stdin)) || !_isatty(_fileno(stderr))) {
			return false;
		}
		return true;
	}
}
