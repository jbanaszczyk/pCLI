#include "stdafx.h"

#include "Logger.h"

namespace logger {
	namespace internal {

		loggerSeverityLevel currentSeverityLevel = loggerSeverityLevel::trace;

		std::wostream* logStream = nullptr;


		DWORD startTime = timeGetTime();

		boost::basic_format<wchar_t> formatElapsedTime() {
			const auto elapsedTime = timeGetTime() - startTime;
			return  boost::_tformat(_T("[%10lu]")) % elapsedTime;
		}

		static const TCHAR* severityLevelNames[] = {
			_T("[trace]  "),
			_T("[debug]  "),
			_T("[info]   "),
			_T("[warning]"),
			_T("[error]  "),
			_T("[fatal]  "),
			_T("[fail]   ")
		};

		const TCHAR* formatSeverityLevel(loggerSeverityLevel severityLevel) {
			return severityLevelNames[severityLevel];
		}
	}

	void setSeverityLevel(loggerSeverityLevel newSeverityLevel) {
		internal::currentSeverityLevel = newSeverityLevel;
	}

	void init() {
		internal::logStream = &std::_tclog;
		setSeverityLevel(loggerSeverityLevel::trace);
	}

	void init(const loggerSeverityLevel initialSeverityLevel) {
		setSeverityLevel(initialSeverityLevel);
	}

}
