#include "stdafx.h"

#include "Logger.h"

namespace logger {
	namespace internal {

		loggerSeverityLevel currentSeverityLevel = loggerSeverityLevel::trace;

		std::wostream* logStream = nullptr;


		DWORD startTime = timeGetTime();

		const std::wstring formatElapsedTime() {
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

		const std::wstring formatSeverityLevel(loggerSeverityLevel severityLevel) {
			return severityLevelNames[severityLevel];
		}
	}

	void setSeverityLevel(loggerSeverityLevel newSeverityLevel) {
		internal::currentSeverityLevel = newSeverityLevel;
	}

	void init() {
		// ReSharper disable once CppUnreachableCode
		init(C_DEBUG
			     ? loggerSeverityLevel::trace
			     : loggerSeverityLevel::warning
		);
	}

	void init(const loggerSeverityLevel initialSeverityLevel) {
		internal::logStream = &std::wclog;
		setSeverityLevel(initialSeverityLevel);
	}

}
