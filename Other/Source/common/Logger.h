#pragma once

#include "common.h"
#include "SysInfo.h"

enum LoggerSeverityLevel {
	Trace,
	Debug,
	Info,
	Warning,
	Error,
	Fatal,
	Fail
};

namespace logger {
	namespace internal {
		extern LoggerSeverityLevel currentSeverityLevel;

		extern std::wostream* logStream;

		std::wstring formatElapsedTime();

		std::wstring formatSeverityLevel(LoggerSeverityLevel severityLevel);

		inline std::wstring logHelper(const boost::wformat& tf) {
			return str(tf);
		}

		template <class T, class... Arguments>
		std::wstring logHelper(boost::wformat& tf, T&& t, Arguments&&... args) {
			return logHelper(tf % std::forward<T>(t), std::forward<Arguments>(args)...);
		}
	}

	void init();

	void init(LoggerSeverityLevel initialSeverityLevel);

	void setSeverityLevel(LoggerSeverityLevel newSeverityLevel);

	template <typename... Arguments>
	std::optional<std::wstring> log(const LoggerSeverityLevel severityLevel, const wchar_t* fmt, Arguments&&... args) {
		if (severityLevel >= internal::currentSeverityLevel) {
			boost::wformat tf{fmt};
			auto message = internal::logHelper(tf, std::forward<Arguments>(args)...);
			*internal::logStream << boost::wformat(L"%s %s %s") % internal::formatElapsedTime() % internal::formatSeverityLevel(severityLevel) % message << std::endl;
			return message;
		}
		return std::nullopt;
	}

	template <typename... Arguments>
	void trace(const wchar_t* fmt, Arguments&&... args) {
		log(Trace, fmt, std::forward<Arguments>(args)...);
	}

	template <typename... Arguments>
	void debug(const wchar_t* fmt, Arguments&&... args) {
		log(Debug, fmt, std::forward<Arguments>(args)...);
	}

	template <typename... Arguments>
	void info(const wchar_t* fmt, Arguments&&... args) {
		log(Info, fmt, std::forward<Arguments>(args)...);
	}

	template <typename... Arguments>
	void warning(const wchar_t* fmt, Arguments&&... args) {
		log(Warning, fmt, std::forward<Arguments>(args)...);
	}

	template <typename... Arguments>
	void error(const wchar_t* fmt, Arguments&&... args) {
		log(Error, fmt, std::forward<Arguments>(args)...);
	}

	template <typename... Arguments>
	void fatal(const wchar_t* fmt, Arguments&&... args) {
		log(Fatal, fmt, std::forward<Arguments>(args)...);
	}

	inline bool isTrace() {
		return Trace >= internal::currentSeverityLevel;
	}

	inline bool isDebug() {
		return Debug >= internal::currentSeverityLevel;
	}

	inline bool isInfo() {
		return Info >= internal::currentSeverityLevel;
	}

	inline bool isWarning() {
		return Warning >= internal::currentSeverityLevel;
	}

	inline bool isError() {
		return Error >= internal::currentSeverityLevel;
	}

	inline bool isFatal() {
		return Fatal >= internal::currentSeverityLevel;
	}
}

template <typename... Arguments>
[[noreturn]] void fail(const wchar_t* fmt, Arguments&&... args) {
	// ReSharper disable once CppTooWideScopeInitStatement
	auto message = logger::log(Fail, fmt, std::forward<Arguments>(args)...);
	if (message.has_value()) {
		if (sys_info::ownsConsole()) {
			*logger::internal::logStream << L"Press [ENTER] to exit." << std::endl;
			std::wcin.get();
		} else {
			MessageBox(nullptr, message.value().c_str(), boost::wformat(L"Can't continue.").str().c_str(), MB_OK | MB_ICONERROR);
		}
	}
	exit(1);
}
