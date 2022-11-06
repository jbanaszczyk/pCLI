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

namespace p_apps {
	class Environment {

		std::map<std::tstring, std::tstring, CaseInsensitiveMap<std::tstring>::Comparator> mEnv;

	public:
		Environment() {};
		Environment(const TCHAR* const envp[]) {
			setUp(envp);
		}

		Environment(const Environment&) = delete;
		Environment(const Environment&&) = delete;
		auto Environment::operator=(const Environment&)->Environment & = delete;
		auto Environment::operator=(const Environment&&)->Environment && = delete;


		using ExpandEnvironmentStrings_t = DWORD(WINAPI*)(LPCTSTR, LPWSTR, DWORD);

		static auto expandEnv(const std::tstring& str,
			ExpandEnvironmentStrings_t ExpandEnvironmentStrings_f = ExpandEnvironmentStrings) -> std::tstring {
			auto bufSize = ExpandEnvironmentStrings_f(str.c_str(), nullptr, 0);

			// FIXME: Error: unique_ptr releases buf, but get() holds its value
			// TODO: review other uses of unique_ptr
			
			std::unique_ptr<TCHAR[]> buf(new(std::nothrow) TCHAR[bufSize]); 
			if (bufSize == 0 || buf.get() == nullptr) {
				return str;
			}
			if (ExpandEnvironmentStrings_f(str.c_str(), buf.get(), bufSize) != bufSize) {
				return str;
			}
			return buf.get();
		}

		auto setUp(const TCHAR* const envp[]) -> void {
			if (envp == nullptr) {
				return;
			}
			for (auto* ref = envp; (*ref != nullptr) && (**ref != _T('\0')); ++ref) {
				auto* sep = _tcschr(*ref, _T('='));
				if (sep == nullptr) {
					break; // garbage
				}
				const size_t len = sep - *ref;
				if (len == 0) {
					continue; // malformed
				}
				++sep;
				if (*sep == _T('\0')) {
					continue; // malformed
				}
				mEnv.insert(std::pair<std::tstring, std::tstring>(std::tstring{ *ref, len }, std::tstring{ sep }));
			}
		}

		auto size(void) const -> size_t {
			return mEnv.size();
		}

		auto exists(const std::tstring& name) const -> bool {
			if (name.empty()) {
				return false;
			}
			return mEnv.end() != mEnv.find(name);
		}

		auto erase(const std::tstring& name) -> void {
			mEnv.erase(name);
		}

		auto get(const std::tstring& name) -> std::tstring {
			if (!exists(name)) {
				return std::tstring{};
			}
			return mEnv[name];
		}

		auto set(const std::tstring& name, const std::tstring& value, const std::tstring& nameBackup = std::tstring{}) -> void {
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

		auto dump(std::unique_ptr<TCHAR[]>& result) const -> void {
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
			--len;
			assert(!len);
		}
	};
}

/*
PAL:_IgnoreLanguage=false
PortableApps.comApps=strPortableAppsPath
PortableApps.comDisableSplash=true
PortableApps.comDocuments=X:\\Documents
PortableApps.comHighContrast=false
PortableApps.comLanguageCode=pl
PortableApps.comLanguageCode2=pl
PortableApps.comLanguageCode2_INTERNAL=pl
PortableApps.comLanguageCode3=pol
PortableApps.comLanguageCode3_INTERNAL=pol
PortableApps.comLanguageCode_INTERNAL=pl
PortableApps.comLanguageGlibc=pl
PortableApps.comLanguageGlibc_INTERNAL=pl
PortableApps.comLanguageLCID=1045
PortableApps.comLanguageLCID_INTERNAL=1045
PortableApps.comLanguageName=Polish
PortableApps.comLanguageName_INTERNAL=Polish
PortableApps.comLanguageNSIS=LANG_POLISH
PortableApps.comLanguageNSIS_INTERNAL=LANG_POLISH
PortableApps.comLocaleCode2=pl
PortableApps.comLocaleCode2_INTERNAL=pl
PortableApps.comLocaleCode3=pol
PortableApps.comLocaleCode3_INTERNAL=pol
PortableApps.comLocaleglibc=pl
PortableApps.comLocaleglibc_INTERNAL=pl
PortableApps.comLocaleID=1045
PortableApps.comLocaleID_INTERNAL=1045
PortableApps.comLocaleWinName=LANG_POLISH
PortableApps.comLocaleWinName_INTERNAL=LANG_POLISH
PortableApps.comMusic=X:\\Documents\Music
PortableApps.comPictures=X:\\Documents\Pictures
PortableApps.comPlatformVersion=21.0.0.0
PortableApps.comRoot=strPortableAppsRootPath
PortableApps.comVideos=X:\\Documents\Videos
 */