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
 *   pApps::Environment is a simple, case insensitive map of wstrings
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

		std::map<std::wstring, std::wstring, CaseInsensitiveMap<std::wstring>::Comparator> mEnv;
		std::wstring backupPrefix = std::wstring{};

	public:
		Environment() = default;

		explicit Environment(const wchar_t* const envp[]) {
			setUp(envp);
		}

		Environment(const Environment&) = delete;
		Environment(const Environment&&) = delete;
		Environment& operator=(const Environment&)= delete;
		Environment&& operator=(const Environment&&)= delete;
		~Environment() = default;

		typedef DWORD (WINAPI* ExpandEnvironmentStrings_t)(LPCTSTR, LPWSTR, DWORD);

		static std::wstring expandEnv(const std::wstring& variableName, ExpandEnvironmentStrings_t ExpandEnvironmentStrings_f = ExpandEnvironmentStrings) {
			const auto bufferSize = ExpandEnvironmentStrings_f(variableName.c_str(), nullptr, 0);

			const std::unique_ptr<wchar_t[]> buffer(new(std::nothrow) wchar_t[bufferSize]);
			if (bufferSize == 0 || buffer == nullptr) {
				return variableName;
			}
			if (ExpandEnvironmentStrings_f(variableName.c_str(), buffer.get(), bufferSize) != bufferSize) {
				return variableName;
			}
			return buffer.get();
		}

		void setUp(const wchar_t* const envp[]) {
			if (envp == nullptr) {
				logger::warning(L"%s: null constructor argument", _T(__FUNCTION__));
				return;
			}
			for (auto* ref = envp; (*ref != nullptr) && (**ref != L'\0'); ++ref) {
				auto* sep = _tcschr(*ref, L'=');
				if (sep == nullptr) {
					logger::warning(L"Environment: garbage, missing '=' in %s", *ref);
					break;
				}
				const size_t len = sep - *ref;
				if (len == 0) {
					logger::warning(L"Environment: malformed, missing variable name in %s", *ref);
					continue;
				}
				++sep;
				if (*sep == L'\0') {
					logger::warning(L"Environment: malformed, missing variable value in %s", *ref);
					continue;
				}
				mEnv.insert(std::pair(std::wstring{*ref, len}, std::wstring{sep}));
			}

			logger::trace(L"[%s] %d variables", _T(__FUNCTION__), size());
		}

		void setNameBackup(const std::wstring& prefix) {
			backupPrefix = prefix;
		}

		[[nodiscard]] size_t size() const {
			return mEnv.size();
		}

		[[nodiscard]] bool exists(const std::wstring& name) const {
			if (name.empty()) {
				return false;
			}
			return mEnv.end() != mEnv.find(name);
		}

		void erase(const std::wstring& name) {
			mEnv.erase(name);
		}

		std::wstring get(const std::wstring& name) {
			if (!exists(name)) {
				return std::wstring{};
			}
			return mEnv[name];
		}

		void set(const std::wstring& name, const std::wstring& value, bool withBackup = false) {
			if (withBackup) {
				set(backupPrefix + name, get(name));
			}
			if (!name.empty()) {
				if (value.empty()) {
					erase(name);
				} else {
					mEnv[name] = value;
				}
			}
		}

		void dump(std::unique_ptr<wchar_t[]>& result) const {
			auto len = 2 * mEnv.size() + 1; // equal signs, zeros after every string, extra zero
			for (const auto& it : mEnv) {
				len += it.first.length() + it.second.length();
			}
			result.reset(new(std::nothrow) wchar_t[len]);

			if (!result) {
				return;
			}

			auto ref = result.get();

			for (const auto& it : mEnv) {
				auto thisLen = it.first.length();
				wcsncpy_s(ref, len, it.first.c_str(), thisLen);
				ref += thisLen;
				len -= thisLen;
				*(ref++) = L'=';
				--len;
				thisLen = it.second.length();
				wcsncpy_s(ref, len, it.second.c_str(), thisLen);
				ref += thisLen;
				len -= thisLen;
				*(ref++) = L'\0';
				--len;
			}
			*ref = L'\0';
		}
	};
}
