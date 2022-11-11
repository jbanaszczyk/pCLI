#pragma once

#include "common.h"

enum loggerSeverityLevel {
	trace,
	debug,
	info,
	warning,
	error,
	fatal,
	fail_
};

namespace logger {
	namespace internal {

		extern loggerSeverityLevel currentSeverityLevel;

		extern std::wostream* logStream;

		const std::wstring formatElapsedTime();
		const std::wstring formatSeverityLevel(loggerSeverityLevel severityLevel);

		inline std::wstring logHelper(const boost::_tformat& tf) {
			return boost::str(tf);
		}

		template<class T, class... Arguments>
		std::wstring logHelper(boost::_tformat& tf, T&& t, Arguments&&... args) {
			return logHelper(tf % std::forward<T>(t), std::forward<Arguments>(args)...);
		}
	}

	void init();
	void init(loggerSeverityLevel initialSeverityLevel);
	void setSeverityLevel(loggerSeverityLevel newSeverityLevel);
	
	template<typename... Arguments>
	std::optional<std::wstring> log(const loggerSeverityLevel severityLevel, const TCHAR* fmt, Arguments&&... args) {
		if (severityLevel >= internal::currentSeverityLevel) {
			boost::_tformat tf{ fmt };
			auto message = internal::logHelper(tf, std::forward<Arguments>(args)...);

			auto z1 = internal::formatElapsedTime();
			auto z2 = internal::formatSeverityLevel(severityLevel);
			auto z3 = str(boost::_tformat(_T("%s %s %s")) % internal::formatElapsedTime() % internal::formatSeverityLevel(severityLevel) % message);

			*internal::logStream << boost::_tformat(_T("%s %s %s")) % internal::formatElapsedTime() % internal::formatSeverityLevel(severityLevel) % message << std::endl;
			return message;
		}
		return std::nullopt;
	}

	template<typename... Arguments>
	void trace(const TCHAR* fmt, Arguments&&... args) {
		log(loggerSeverityLevel::trace, fmt, std::forward<Arguments>(args)...);
	}

	template<typename... Arguments>
	void debug(const TCHAR* fmt, Arguments&&... args) {
		log(loggerSeverityLevel::debug, fmt, std::forward<Arguments>(args)...);
	}

	template<typename... Arguments>
	void info(const TCHAR* fmt, Arguments&&... args) {
		log(loggerSeverityLevel::info, fmt, std::forward<Arguments>(args)...);
	}

	template<typename... Arguments>
	void warning(const TCHAR* fmt, Arguments&&... args) {
		log(loggerSeverityLevel::warning, fmt, std::forward<Arguments>(args)...);
	}

	template<typename... Arguments>
	void error(const TCHAR* fmt, Arguments&&... args) {
		log(loggerSeverityLevel::error, fmt, std::forward<Arguments>(args)...);
	}

	template<typename... Arguments>
	void fatal(const TCHAR* fmt, Arguments&&... args) {
		log(loggerSeverityLevel::fatal, fmt, std::forward<Arguments>(args)...);
	}

}

template<typename... Arguments>
void fail(const TCHAR* fmt, Arguments&&... args) {
	auto message = logger::log(loggerSeverityLevel::fail_, fmt, std::forward<Arguments>(args)...);
	if (message.has_value()) {
		if (SysInfo::ownsConsole()) {
			*logger::internal::logStream << _T("Press [ENTER] to exit.") << std::endl;
			std::_tcin.get();
		}
		else {
			MessageBox(nullptr, message.value().c_str(), (boost::_tformat(_T("Can't continue."))).str().c_str(), MB_OK | MB_ICONERROR);
		}
	}
	exit(1);
}
