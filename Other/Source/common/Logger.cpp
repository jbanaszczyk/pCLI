#include "stdafx.h"

#include "Logger.h"

namespace logger {
	namespace internal {
		LoggerSeverityLevel currentSeverityLevel = Trace;

		std::wostream* logStream = nullptr;

		DWORD startTime = timeGetTime();

		std::wstring formatElapsedTime() {
			const auto elapsedTime = timeGetTime() - startTime;
			return str(boost::wformat(L"[%10lu]") % elapsedTime);
		}

		static const wchar_t* severityLevelNames[] = {
			L"[trace]  ",
			L"[debug]  ",
			L"[info]   ",
			L"[warning]",
			L"[error]  ",
			L"[fatal]  ",
			L"[fail]   "
		};

		std::wstring formatSeverityLevel(const LoggerSeverityLevel severityLevel) {
			return severityLevelNames[severityLevel];
		}
	}

	void setSeverityLevel(const LoggerSeverityLevel newSeverityLevel) {
		internal::currentSeverityLevel = newSeverityLevel;
	}

	void init() {
		// ReSharper disable once CppUnreachableCode
		init(C_DEBUG
		     ? Trace
		     : Warning
		);
	}

	void init(const LoggerSeverityLevel initialSeverityLevel) {
		internal::logStream = &std::wclog;
		setSeverityLevel(initialSeverityLevel);
	}
}
