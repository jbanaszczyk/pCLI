/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
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

#include "./common.h"
#include <memory>

namespace p_apps {
	class Environment {
	private:
		template<typename T>
		struct fuLess : std::binary_function < T, T, bool > {
			bool operator() (const T& s1, const T& s2) const {
				return boost::ilexicographical_compare(s1, s2);
			}
		};
		typedef std::map<std::tstring, std::tstring, fuLess<std::tstring>> IMap;

		IMap mEnv;

	public:
		Environment() = delete;
		Environment(const Environment&) = delete;
		Environment(const Environment&&) = delete;
		Environment& Environment:: operator= (const Environment&) = delete;
		Environment&& Environment:: operator= (const Environment&&) = delete;

		Environment(const TCHAR *const envp[]) {
			setUp(envp);
		}

		typedef DWORD(WINAPI *ExpandEnvironmentStrings_t) (LPCTSTR, LPWSTR, DWORD);

		static std::tstring expandEnv(const std::tstring& str, ExpandEnvironmentStrings_t ExpandEnvironmentStrings_f = ExpandEnvironmentStrings) {
			auto bufSize = ExpandEnvironmentStrings_f(str.c_str(), nullptr, 0);

            std::unique_ptr< TCHAR[] > buf(new (std::nothrow) TCHAR[bufSize]);
			if (bufSize == 0 || buf.get() == nullptr) {
				return str;
			}
			if (ExpandEnvironmentStrings_f(str.c_str(), buf.get(), bufSize) != bufSize) {
				return str;
			}
			return buf.get();
		}

		void setUp(const TCHAR *const envp[]) {
			if (envp == nullptr) {
				return;
			}
			for (auto *ref = envp; (*ref != nullptr) && (**ref != _T('\0')); ++ref) {
				auto *sep = _tcschr(*ref, _T('='));
				if (sep == nullptr) {
					break; // garbage
				}
				size_t len = sep - *ref;
				if (len == 0) {
					continue; // mallformed
				}
				++sep;
				if (*sep == _T('\0')) {
					continue; // mallformed
				}
				mEnv.insert(std::pair<std::tstring, std::tstring>(std::tstring{ *ref, len }, std::tstring{ sep }));
			}
		}

		size_t size(void) {
			return mEnv.size();
		}

		bool exists(const std::tstring& name) const {
			if (name.empty() ) {
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
				} else {
					mEnv[name] = value;
				}
			}
		}

		void dump(std::unique_ptr<TCHAR[]>& result) const {
			size_t len = 2 * mEnv.size() + 1; // equal signs, zeros after every string, extra zero
			for (const auto& it : mEnv) {
				len += it.first.length() + it.second.length();
			}
			result.reset(new (std::nothrow) TCHAR[len]);

			if (!result) {
				return;
			}

			TCHAR *ref = result.get();

			for (const auto& it : mEnv) {
				size_t thisLen = it.first.length();
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
			--len;
			assert(!len);
		}
	};
}
