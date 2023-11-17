#include "stdafx.h"
#include "IniFile.h"
#include "Logger.h"

namespace p_apps {
	IniFile::IniValue::IniValue(const std::wstring& value, const bool setDefault, const bool setInterim) {
		setValue(value, setDefault, setInterim);
	}

	void IniFile::IniValue::propagate() {
		// update valueInterim to valueCurrent. To be executed after writeIniFile
		valueInterim = valueCurrent;
	}

	std::wstring IniFile::IniValue::getDefault() const {
		return valueDefault;
	}

	std::wstring IniFile::IniValue::getValue() const {
		return valueCurrent;
	}

	bool IniFile::IniValue::isModified() const {
		return !boost::equals(valueInterim, valueCurrent);
	}

	void IniFile::iniNames(const wchar_t* const sectionName, const std::filesystem::path& fName, std::vector<std::wstring>& sections) {
		if (std::error_code errCode; !exists(fName, errCode)) {
			return;
		}

		std::unique_ptr<wchar_t[]> bufPtr;

		DWORD bufSize = 1024;
		while (true) {
			bufPtr.reset(new(std::nothrow) wchar_t[bufSize]);
			if (!bufPtr) {
				break;
			}

			const auto len = GetPrivateProfileString(sectionName, nullptr, nullptr, bufPtr.get(), bufSize, fName.c_str());
			errno = 0;
			if (ENOENT == errno) {
				errno = 0;
				return;
			}
			if (bufSize - 2 > len) {
				break;
			}

#ifdef _WIN64
			constexpr DWORD maxBufSize = UINT_MAX / sizeof(wchar_t);
#else
			constexpr DWORD maxBufSize = heapMaxReqReal / sizeof (wchar_t);
#endif
			if (maxBufSize == bufSize) {
				break;
			}
			bufSize = maxBufSize / 2 >= bufSize
			          ? 2 * bufSize
			          : maxBufSize;
		}
		if (!bufPtr) {
			return;
		}
		auto ref = bufPtr.get();
		while (L'\0' != *ref) {
			const auto len = wcslen(ref);
			sections.emplace_back(ref);
			ref += len + 1;
		}
	}

	IniFile::IniFile()
		: iniName_(L"") {
	}

	IniFile::~IniFile() = default;

	void IniFile::setDefaults(const IniDefaults* defaults, const size_t nElements) {
		for (size_t idx = 0; nElements > idx; ++idx, ++defaults) {
			setValue(*defaults, true);
		}
	}

	void IniFile::readIniFile(const std::filesystem::path& iniName) {
		if (std::error_code errCode; !exists(iniName, errCode) || errCode) {
			logger::trace(L"[%s] trying  INI file: %s", _T(__FUNCTION__), iniName.c_str());
			return;
		}
		logger::trace(L"[%s] reading INI file: %s", _T(__FUNCTION__), iniName.c_str());

		iniName_ = iniName;
		std::vector<std::wstring> sectionNames;
		iniNames(nullptr, iniName, sectionNames);
		for (auto itSections = begin(sectionNames); end(sectionNames) != itSections; ++itSections) {
			std::vector<std::wstring> nameNames;
			iniNames(itSections->c_str(), iniName, nameNames);
			for (auto itNames = begin(nameNames); end(nameNames) != itNames; ++itNames) {
				std::wstring defaultValue;
				IniKey thisKey(itSections->c_str(), itNames->c_str());
				auto ref = values.find(thisKey);
				if (values.end() != ref) {
					defaultValue = ref->second.getDefault();
				}
				// ReSharper disable once CppTooWideScope
				auto valueStr = iniRead(iniName, *itSections, *itNames, defaultValue);
				if (valueStr) {
					if (values.end() == ref) {
						values.insert(std::pair<IniKey, IniValue>(thisKey, IniValue(valueStr.value(), false, true)));
					} else {
						ref->second.setValue(valueStr.value(), false, true);
					}
				}
			}
		}
	}

	std::wstring IniFile::getValueNonEmpty(const std::wstring& section, const std::wstring& name) const {
		const IniKey thisKey(section, name);
		const auto ref = values.find(thisKey);
		if (values.end() == ref) {
			return L"";
		}
		auto result = ref->second.getValue();
		if (result.empty()) {
			result = ref->second.getDefault();
		}
		return result;
	}

	void IniFile::enumSections(SetInsensitiveTChar& result) const {
		for (auto it = begin(values); end(values) != it; ++it) {
			result.insert(it->first.first);
		}
	}

	void IniFile::enumNames(const std::wstring& section, SetInsensitiveTChar& result) const {
		for (auto it = begin(values); end(values) != it; ++it) {
			if (boost::iequals(section, it->first.first)) {
				result.insert(it->first.second);
			}
		}
	}

	void IniFile::setValue(const IniDefaults& defaults, const bool isDefault) {
		setValue(defaults.aSection, defaults.aName, defaults.aValue, isDefault);
	}

	void IniFile::IniValue::setValue(const std::wstring& value, const bool setDefault, const bool setInterim) {
		if (setDefault) {
			valueDefault = value;
		}
		if (setInterim) {
			valueInterim = value;
		}
		valueCurrent = value;
	}

	std::optional<std::wstring> IniFile::iniRead(const std::filesystem::path& fName, const std::wstring& section, const std::wstring& name, const std::wstring& defValue) {
		if (std::error_code errCode; !exists(fName, errCode)) {
			return std::nullopt;
		}
		std::unique_ptr<wchar_t[]> buf;
		DWORD bufSize = 1024;
		while (true) {
			buf.reset(new(std::nothrow) wchar_t[bufSize]);

			if (buf == nullptr) {
				break;
			}
			errno = 0;
			const auto len = GetPrivateProfileString(section.c_str(), name.c_str(), defValue.c_str(), buf.get(), bufSize, fName.c_str());
			if (ENOENT == errno) {
				errno = 0;
				return std::nullopt;
			}
			if (bufSize - 1 > len) {
				break;
			}
#ifdef _WIN64
			constexpr DWORD maxBufSize = UINT_MAX / sizeof buf[0];
#else
			constexpr DWORD maxBufSize = heapMaxReqReal / sizeof(wchar_t);
#endif
			if (maxBufSize == bufSize) {
				break;
			}
			bufSize = maxBufSize / 2 >= bufSize
			          ? 2 * bufSize
			          : maxBufSize;
		}
		if (buf == nullptr) {
			return std::nullopt;
		}
		if (nullptr == buf) {
			return std::nullopt;
		}
		return buf.get();
	}

	void IniFile::setDefaults(const std::wstring& aSection, const std::wstring& aName, const std::wstring& aDefault) {
		setValue(aSection, aName, aDefault, true);
	}

	std::optional<std::wstring> IniFile::getValue(const std::wstring& section, const std::wstring& name) const {
		const IniKey thisKey(section, name);
		const auto ref = values.find(thisKey);
		if (values.end() == ref) {
			return std::nullopt;
		}
		auto result = ref->second.getValue();
		if (result.empty()) {
			return std::nullopt;
		}
		return result;
	}

	std::optional<std::wstring> IniFile::getDefault(const std::wstring& section, const std::wstring& name) const {
		const IniKey thisKey(section, name);
		const auto ref = values.find(thisKey);
		if (values.end() == ref) {
			return std::nullopt;
		}
		auto result = ref->second.getDefault();
		if (result.empty()) {
			return std::nullopt;
		}
		return result;
	}

	void IniFile::setValue(const std::wstring& section, const std::wstring& name, const std::wstring& value, const bool isDefault) {
		IniKey thisKey(section, name);
		if (const auto ref = values.find(thisKey); values.end() == ref) {
			values.insert(std::pair(thisKey, IniValue(value, isDefault, false)));
		} else {
			ref->second.setValue(value, isDefault, false);
		}
	}
}
