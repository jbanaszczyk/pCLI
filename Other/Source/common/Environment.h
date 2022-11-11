/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

 /******************************************************************************
  *
  * expandEnv
  *   Expand environment variable strings
  *
  * class pApps::Environment represents set of environment variables
  *   pApps::Environment is a simple, case insensitive map of tstrings
  *   pApps::Environment should be initialized from envp[] array
  *     manipulated using get/set/erase
  *     then dumped to another envp[] array and passed to launched program
  *
  *****************************************************************************/

#include "common.h"
#include "pAppsUtils.h"
#include "Logger.h"

namespace p_apps {
	class Environment {

		std::map<std::tstring, std::tstring, CaseInsensitiveMap<std::tstring>::Comparator> mEnv;

	public:
		Environment() = default;

		explicit Environment(const TCHAR* const envp[]) {
			setUp(envp);
		}

		Environment(const Environment&) = delete;
		Environment(const Environment&&) = delete;
		auto operator=(const Environment&)->Environment & = delete;
		auto operator=(const Environment&&)->Environment && = delete;
		~Environment() = default;

		typedef DWORD(WINAPI* ExpandEnvironmentStrings_t)(LPCTSTR, LPWSTR, DWORD);

		static std::tstring expandEnv(const std::tstring& variableName, ExpandEnvironmentStrings_t ExpandEnvironmentStrings_f = ExpandEnvironmentStrings) {
			const auto bufferSize = ExpandEnvironmentStrings_f(variableName.c_str(), nullptr, 0);

			const std::unique_ptr<TCHAR[]> buffer(new(std::nothrow) TCHAR[bufferSize]);
			if (bufferSize == 0 || buffer == nullptr) {
				return variableName;
			}
			if (ExpandEnvironmentStrings_f(variableName.c_str(), buffer.get(), bufferSize) != bufferSize) {
				return variableName;
			}
			return buffer.get();
		}

		auto setUp(const TCHAR* const envp[]) -> void {
			if (envp == nullptr) {
				return;
			}
			for (auto* ref = envp; (*ref != nullptr) && (**ref != _T('\0')); ++ref) {
				auto* sep = _tcschr(*ref, _T('='));
				if (sep == nullptr) {
					logger::warning(_T("Environment: garbage, missing '=' in %s"), *ref);
					break;
				}
				const size_t len = sep - *ref;
				if (len == 0) {
					logger::warning(_T("Environment: malformed, missing variable name in %s"), *ref);
					continue;
				}
				++sep;
				if (*sep == _T('\0')) {
					logger::warning(_T("Environment: malformed, missing variable value in %s"), *ref);
					continue;
				}
				mEnv.insert(std::pair(std::tstring{ *ref, len }, std::tstring{ sep }));
			}
		}

		[[nodiscard]] size_t size() const {
			return mEnv.size();
		}

		[[nodiscard]] bool exists(const std::tstring& name) const {
			if (name.empty()) {
				return false;
			}
			return mEnv.end() != mEnv.find(name);
		}

		void erase(const std::tstring& name) {
			mEnv.erase(name);
		}

		std::tstring get(const std::tstring& name) {
			if (!exists(name)) {
				return std::tstring{};
			}
			return mEnv[name];
		}

		void set(const std::tstring& name, const std::tstring& value, const std::tstring& nameBackup = std::tstring{}) {
			if (!nameBackup.empty()) {
				set(nameBackup, get(name));
			}
			if (!name.empty()) {
				if (value.empty()) {
					erase(name);
				}
				else {
					mEnv[name] = value;
				}
			}
		}

		void dump(std::unique_ptr<TCHAR[]>& result) const {
			auto len = 2 * mEnv.size() + 1; // equal signs, zeros after every string, extra zero
			for (const auto& it : mEnv) {
				len += it.first.length() + it.second.length();
			}
			result.reset(new(std::nothrow) TCHAR[len]);

			if (!result) {
				return;
			}

			auto ref = result.get();

			for (const auto& it : mEnv) {
				auto thisLen = it.first.length();
				wcsncpy_s(ref, len, it.first.c_str(), thisLen);
				ref += thisLen;
				len -= thisLen;
				*(ref++) = _T('=');
				--len;
				thisLen = it.second.length();
				wcsncpy_s(ref, len, it.second.c_str(), thisLen);
				ref += thisLen;
				len -= thisLen;
				*(ref++) = _T('\0');
				--len;
			}
			*ref = _T('\0');
		}
	};
}
