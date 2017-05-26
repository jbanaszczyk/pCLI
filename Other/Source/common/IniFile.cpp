/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#include "stdafx.h"

#include "./IniFile.h"

/******************************************************************************
 *
 * IniFile : manipulation *.INI
 *
 * Class is a simple wrapper for GetPrivateProfileString
 *
 * *.ini file is cached and not refreshed.
 * IniFile is not thread-aware.
 * IniFile is UTF-8 aware
 *
 * There are two static methods
 *
 * iniRead
 *   directly read one setting from any *.INI
 *
 * iniNames
 *   enumerate all sections or names within section, directly in *.INI
 *
 * IniFile operations:
 *
 * void setDefaults
 *   provide array of iniDefaults structs.
 *     { _T("Section"), _T("Name"),    _T("Default") }, ... }
 * void readIniFile
 *   cache all setings.
 * writeIniFile
 *   put changed settings back to *.INI,
 *   performed in destructor.
 *   writeIniFile changes defaults: current values become default ones.
 * setValue
 *   modify settings
 * getValue
 *   as expected
 * getDefault
 *   as expected
 *   warning: default values are modified using
 *     setDefaults (as expected) and writeIniFile
 * getValueNonEmpty
 *   returns getValue, but if it empty, return default value.
 *   be sure, that defaults are well-defined
 * enumSections *   returns set of known [sections]
 *     namesSet: simple, case insensitive
 * enumNames
 *   returns set of known names within a section
 *     namesSet: simple, case insensitive
 *
 *****************************************************************************/
namespace pApps {
	boost::optional<std::tstring> iniFile::iniRead(const boost::filesystem::path fName, const std::tstring section, const std::tstring name, const std::tstring defValue) {
		boost::system::error_code errCode;
		if (!boost::filesystem::exists(fName, errCode)) {
			return boost::none;
		}
		std::unique_ptr< TCHAR[] > buf;
		DWORD bufSize = 1024;
		while (true) {
			buf.reset(new (std::nothrow) TCHAR[bufSize]);

			if (buf == nullptr ) {
				break;
			}
			errno = 0;
			DWORD len = GetPrivateProfileString(section.c_str(), name.c_str(), defValue.c_str(), buf.get(), bufSize, fName.c_str());
			if (ENOENT == errno) {
				errno = 0;
				return boost::none;
			}
			if (bufSize - 1 > len) {
				break;
			}
#ifdef _WIN64
			const DWORD maxBufSize = UINT_MAX / sizeof buf[0];
#else
			const DWORD maxBufSize = heapMaxReqReal / sizeof TCHAR;
#endif
			if (maxBufSize == bufSize) {
				break;
			}
			bufSize = maxBufSize / 2 >= bufSize ? 2 * bufSize : maxBufSize;
		}
		if (buf == nullptr ) {
			return boost::none;
		}
		if (_T('\0') == buf.get()) {
			return boost::none;
		}
		return buf.get();
	}

	void iniFile::iniNames(const TCHAR *const sectionName, const boost::filesystem::path fName, std::vector<std::tstring>& sections) {
		boost::system::error_code errCode;
		if (!boost::filesystem::exists(fName, errCode)) {
			return;
		}
		std::unique_ptr<TCHAR[]> bufPtr;

		DWORD bufSize = 1024;
		while (true) {
			bufPtr.reset(new (std::nothrow) TCHAR[bufSize]);
			if (!bufPtr) {
				break;
			}

			DWORD len = GetPrivateProfileString(sectionName, nullptr, nullptr, bufPtr.get(), bufSize, fName.c_str());
			errno = 0;
			if (ENOENT == errno) {
				errno = 0;
				return;
			}
			if (bufSize - 2 > len) {
				break;
			}

#ifdef _WIN64
			const DWORD maxBufSize = UINT_MAX / sizeof TCHAR;
#else
			const DWORD maxBufSize = heapMaxReqReal / sizeof TCHAR;
#endif
			if (maxBufSize == bufSize) {
				break;
			}
			bufSize = maxBufSize / 2 >= bufSize ? 2 * bufSize : maxBufSize;
		}
		if (!bufPtr) {
			return;
		}
		TCHAR *ref = bufPtr.get();
		while (_T('\0') != *ref) {
			size_t len = wcslen(ref);
			sections.push_back(ref);
			ref += len + 1;
		}
		return;
	}

	/*****************************************************/

	iniFile::iniValue::iniValue(const std::tstring value, const bool setDefault, const bool setInterim) : _valueDefault(_T("")), _valueInterim(_T("")), _valueCurrent(_T("")) {
		setValue(value, setDefault, setInterim);
	}

	void iniFile::iniValue::setValue(const std::tstring value, const bool setDefault, const bool setInterim) {
		if (setDefault) {
			_valueDefault = value;
		}
		if (setInterim) {
			_valueInterim = value;
		}
		_valueCurrent = value;
	};

	void iniFile::iniValue::propagate() {
		_valueInterim = _valueCurrent;
	};

	std::tstring iniFile::iniValue::getDefault() const {
		return _valueDefault;
	};

	std::tstring iniFile::iniValue::getValue() const {
		return _valueCurrent;
	};

	bool iniFile::iniValue::isModified() const {
		return !boost::equals(_valueInterim, _valueCurrent);
	};

	/*****************************************************/

	iniFile::~iniFile() {
		writeIniFile();
	}

	void iniFile::setValue(const std::tstring section, const std::tstring name, const std::tstring value, const bool isDefault) {
		iniKey thisKey(section, name);
		auto ref = _values.find(thisKey);
		if (_values.end() == ref) {
			_values.insert(std::pair<iniKey, iniValue>(thisKey, iniValue(value, isDefault, false)));
		} else {
			ref->second.setValue(value, isDefault, false);
		}
	}

	void iniFile::setValue(const iniDefaults& defaults, const bool isDefault) {
		setValue(defaults.aSection, defaults.aName, defaults.aValue, isDefault);
	}

	void iniFile::setDefaults(const std::tstring aSection, const std::tstring aName, const std::tstring aDefault) {
		setValue(aSection, aName, aDefault, true);
	}

	void iniFile::setDefaults(const iniDefaults *defaults, const size_t nElements) {
		for (size_t idx = 0; nElements > idx; ++idx, ++defaults) {
			setValue(*defaults, true);
		}
	};

	void iniFile::readIniFile(const boost::filesystem::path iniName, const bool saveOnExit) {
		_iniName = iniName;
		_saveOnExit = saveOnExit;
		std::vector<std::tstring> sectionNames;
		iniNames(nullptr, iniName, sectionNames);
		for (auto itSections = begin(sectionNames); end(sectionNames) != itSections; ++itSections) {
			std::vector<std::tstring> nameNames;
			iniNames(itSections->c_str(), iniName, nameNames);
			for (auto itNames = begin(nameNames); end(nameNames) != itNames; ++itNames) {
				std::tstring defaultValue(_T(""));
				iniKey thisKey(itSections->c_str(), itNames->c_str());
				auto ref = _values.find(thisKey);
				if (_values.end() != ref) {
					defaultValue = ref->second.getDefault();
				}
				boost::optional<std::tstring> valueStr = iniFile::iniRead(iniName, itSections->c_str(), itNames->c_str(), defaultValue);
				if (valueStr) {
					if (_values.end() == ref) {
						_values.insert(std::pair<iniKey, iniValue>(thisKey, iniValue(valueStr.get(), false, true)));
					} else {
						ref->second.setValue(valueStr.get(), false, true);
					}
				}
			}
		}
	};

	size_t iniFile::writeIniFile() {
		if (!_saveOnExit) {
			return 0;
		}
		size_t retVal = 0;
		for (auto it = begin(_values); end(_values) != it; ++it) {
			if (it->second.isModified()) {
				if (WritePrivateProfileString(it->first.first.c_str(), it->first.second.c_str(), ! it->second.getValue().empty() ? it->second.getValue().c_str() : nullptr, _iniName.c_str())) {
					it->second.propagate();
					++retVal;
				}
			}
		}
		return retVal;
	}

	boost::optional<std::tstring> iniFile::getValue(const std::tstring section, const std::tstring name) const {
		iniKey thisKey(section, name);
		auto ref = _values.find(thisKey);
		if (_values.end() == ref) {
			return boost::none;
		}
		std::tstring retVal = ref->second.getValue();
		if (retVal.empty()) {
			return boost::none;
		}
		return retVal;
	}

	boost::optional<std::tstring> iniFile::getDefault(const std::tstring section, const std::tstring name) const {
		iniKey thisKey(section, name);
		auto ref = _values.find(thisKey);
		if (_values.end() == ref) {
			return boost::none;
		}
		std::tstring retVal = ref->second.getDefault();
		if (retVal.empty()) {
			return boost::none;
		}
		return retVal;
	}

	std::tstring iniFile::getValueNonEmpty(const std::tstring section, const std::tstring name) const {
		iniKey thisKey(section, name);
		auto ref = _values.find(thisKey);
		if (_values.end() == ref) {
			return _T("");
		}
		std::tstring retVal = ref->second.getValue();
		if (retVal.empty()) {
			retVal = ref->second.getDefault();
		}
		return retVal;
	}

	void iniFile::enumSections(namesSet& result) const {
		for (auto it = begin(_values); end(_values) != it; ++it) {
			result.insert(it->first.first);
		}
	}

	void iniFile::enumNames(const std::tstring section, namesSet& result) const {
		for (auto it = begin(_values); end(_values) != it; ++it) {
			if (boost::iequals(section, it->first.first)) {
				result.insert(it->first.second);
			}
		}
	}
}
