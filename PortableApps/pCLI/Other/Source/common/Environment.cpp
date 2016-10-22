/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#include "stdafx.h"

#include "./Environment.h"

/******************************************************************************
 *
 * There is a static method
 *
 * expandEnv
 *   Expand environment variable strings
 *
 * class pApps::Environment represents set of environment variables
 * Environment is a simple, case insensitive map of tstrings
 * Environment should be initialized from envp[] array
 * manipulated using get/set/erase
 * then dumped to another envp[] array and passed to launched program
 *
 *****************************************************************************/

namespace pApps {
	std::tstring Environment::expandEnv(const std::tstring str) {
		DWORD bufSize = ExpandEnvironmentStrings(str.c_str(), nullptr, 0);

		std::unique_ptr< TCHAR[] > buf(new (std::nothrow) TCHAR[bufSize]);
		if (nullptr == buf) {
			return str;
		}
		bufSize = ExpandEnvironmentStrings(str.c_str(), buf.get(), bufSize);
		if (0 == bufSize) {
			return str;
		}
		return buf.get();
	}

	Environment::Environment(const TCHAR *const envp[]) {
		setUp(envp);
	}

	void Environment::setUp(const TCHAR *const envp[]) {
		if (nullptr == envp) {
			return;
		}
		for (auto *ref = envp; nullptr != *ref; ++ref) {
			auto *sep = _tcschr(*ref, _T('='));
			if (nullptr == sep) {
				continue;
			}
			if (sep == *ref) {
				continue;
			}
			size_t len = sep - *ref;
			sep++;
			if (_T('\0') == *sep) {
				continue;
			}
			std::tstring name(*ref, len);
			_mEnv.insert(std::pair<std::tstring, std::tstring>(name, sep));
		}
	}

	bool Environment::exists(const std::tstring name) const {
		if (0 == name.length()) {
			return false;
		}
		return _mEnv.end() != _mEnv.find(name);
	}

	std::tstring Environment::get(const std::tstring name) {
		if (0 == name.length()) {
			return _T("");
		}
		return _mEnv[name];
	}

	void Environment::set(const std::tstring name, const std::tstring value, const std::tstring keyBackup) {
		if (keyBackup.length() != 0) {
			_mEnv[keyBackup] = _mEnv[name];
		}
		_mEnv[name] = value;
	}

	void Environment::erase(const std::tstring name) {
		_mEnv.erase(name);
	}

	void Environment::dump(std::vector<std::tstring>& result) const {
		for (const auto& it : _mEnv) {
			std::tstring val = it.first + _T("=") + it.second;
			result.push_back(val);
		}
	}

	void Environment::dump(std::unique_ptr<TCHAR[]>& result) const {
		size_t len = 2 * _mEnv.size() + 1; // equal signs, zeros after every string, extra zero
		for (const auto& it : _mEnv) {
			len += it.first.length() + it.second.length();
		}
		result.reset(new (std::nothrow) TCHAR[len]);

		if (!result) {
			return;
		}

		TCHAR *ref = result.get();

		for (const auto& it : _mEnv) {
			TCHAR *xyz = ref;
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
		*(ref++) = _T('\0');
		--len;
		assert(!len);
	}
}
