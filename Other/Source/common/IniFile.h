/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

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
 *   cache all settings.
 * writeIniFile
 *   put changed settings back to *.INI,
 *   performed in destructor.
 *   writeIniFile changes defaults: current keyValues become default ones.
 * setValue
 *   modify settings
 * getValue
 *   as expected
 * getDefault
 *   as expected
 *   warning: default keyValues are modified using
 *     setDefaults (as expected) and writeIniFile
 * getValueNonEmpty
 *   returns getValue, but if it empty, return default value.
 *   be sure, that defaults are well-defined
 * enumSections *   returns set of known [sections]
 *     SetInsensitiveTChar: simple, case insensitive
 * enumNames
 *   returns set of known names within a section
 *     SetInsensitiveTChar: simple, case insensitive
 *
 *****************************************************************************/

#include "./common.h"

namespace p_apps {
	class IniFile {

	private:
		class IniValue {
		private:
			std::tstring valueDefault; // default
			std::tstring valueInterim; // was read from file
			std::tstring valueCurrent; // effective
		public:
			IniValue(const std::tstring& value, const bool setDefault, const bool setInterim) {
				setValue(value, setDefault, setInterim);
			}

			void setValue(const std::tstring& value, bool setDefault, bool setInterim);

			void propagate() {
				// update valueInterim to valueCurrent. To be executed after writeIniFile
				valueInterim = valueCurrent;
			}

			std::tstring getDefault() const {
				return valueDefault;
			}

			std::tstring getValue() const {
				return valueCurrent;
			}

			bool isModified() const {
				return !boost::equals(valueInterim, valueCurrent);
			}
		};

		using iniKey = std::pair<std::tstring, std::tstring>;

		std::map<iniKey, IniValue, CaseInsensitivePairFirst<std::tstring>::Comparator> values;

		std::filesystem::path iniName_;
		bool saveOnExit_;

	public:
		struct iniDefaults {
			std::tstring aSection;
			std::tstring aName;
			std::tstring aValue;
		};

		static boost::optional<std::tstring> iniRead(const std::filesystem::path& fName, const std::tstring& section, const std::tstring& name, const std::tstring& defValue = _T(""));

		static void iniNames(const TCHAR* const sectionName, const std::filesystem::path& fName, std::vector<std::tstring>& sections) {
			std::error_code errCode;
			if (!exists(fName, errCode)) {
				return;
			}
				
			std::unique_ptr<TCHAR[]> bufPtr;

			DWORD bufSize = 1024;
			while (true) {
				bufPtr.reset(new(std::nothrow) TCHAR[bufSize]);
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
					const DWORD maxBufSize = UINT_MAX / sizeof TCHAR;
#else
				const DWORD maxBufSize = heapMaxReqReal / sizeof TCHAR;
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
			while (_T('\0') != *ref) {
				const auto len = wcslen(ref);
				sections.push_back(ref);
				ref += len + 1;
			}
		}


	public:
		IniFile()
			: iniName_(_T(""))
		  , saveOnExit_(false) {
		}

		~IniFile() {
			writeIniFile();
		}

		void setDefaults(const std::tstring& aSection, const std::tstring& aName, const std::tstring& aDefault);


		void setDefaults(const iniDefaults* defaults, const size_t nElements = 1) {
			for (size_t idx = 0; nElements > idx; ++idx, ++defaults) {
				setValue(*defaults, true);
			}
		}

		void readIniFile(const std::filesystem::path& iniName, const bool saveOnExit) {
			std::error_code errCode;
			if (!exists(iniName, errCode)) {
				return;
			}
			logger::trace(_T("[%s] reading INI file: %s"), _T(__FUNCTION__), iniName);

			iniName_ = iniName;
			saveOnExit_ = saveOnExit;
			std::vector<std::tstring> sectionNames;
			iniNames(nullptr, iniName, sectionNames);
			for (auto itSections = begin(sectionNames); end(sectionNames) != itSections; ++itSections) {
				std::vector<std::tstring> nameNames;
				iniNames(itSections->c_str(), iniName, nameNames);
				for (auto itNames = begin(nameNames); end(nameNames) != itNames; ++itNames) {
					std::tstring defaultValue(_T(""));
					iniKey thisKey(itSections->c_str(), itNames->c_str());
					auto ref = values.find(thisKey);
					if (values.end() != ref) {
						defaultValue = ref->second.getDefault();
					}
					auto valueStr = iniRead(iniName, itSections->c_str(), itNames->c_str(), defaultValue);
					if (valueStr) {
						if (values.end() == ref) {
							values.insert(std::pair<iniKey, IniValue>(thisKey, IniValue(valueStr.get(), false, true)));
						} else {
							ref->second.setValue(valueStr.get(), false, true);
						}
					}
				}
			}
		}

		size_t writeIniFile() {
			if (!saveOnExit_) {
				return 0;
			}
			size_t result = 0;
			for (auto it = begin(values); end(values) != it; ++it) {
				if (it->second.isModified()) {
					if (WritePrivateProfileString(it->first.first.c_str(), it->first.second.c_str(), !it->second.getValue().empty()
						                                                                                 ? it->second.getValue().c_str()
						                                                                                 : nullptr, iniName_.c_str())) {
						it->second.propagate();
						++result;
					}
				}
			}
			return result;
		}

		boost::optional<std::tstring> getValue(const std::tstring& section, const std::tstring& name) const;

		boost::optional<std::tstring> getDefault(const std::tstring& section, const std::tstring& name) const;

		std::tstring getValueNonEmpty(const std::tstring& section, const std::tstring& name) const {
			const iniKey thisKey(section, name);
			const auto ref = values.find(thisKey);
			if (values.end() == ref) {
				return _T("");
			}
			auto result = ref->second.getValue();
			if (result.empty()) {
				result = ref->second.getDefault();
			}
			return result;
		}

		void enumSections(SetInsensitiveTChar& result) const {
			for (auto it = begin(values); end(values) != it; ++it) {
				result.insert(it->first.first);
			}
		}

		void enumNames(const std::tstring& section, SetInsensitiveTChar& result) const {
			for (auto it = begin(values); end(values) != it; ++it) {
				if (boost::iequals(section, it->first.first)) {
					result.insert(it->first.second);
				}
			}
		}

		void setValue(const std::tstring& section, const std::tstring& name, const std::tstring& value, bool isDefault = false);

		void setValue(const iniDefaults& defaults, const bool isDefault = false) {
			setValue(defaults.aSection, defaults.aName, defaults.aValue, isDefault);
		}

	};

	inline void IniFile::IniValue::setValue(const std::tstring& value, const bool setDefault, const bool setInterim) {
		if (setDefault) {
			valueDefault = value;
		}
		if (setInterim) {
			valueInterim = value;
		}
		valueCurrent = value;
	}

	inline boost::optional<std::tstring> IniFile::iniRead(const std::filesystem::path& fName, const std::tstring& section, const std::tstring& name, const std::tstring& defValue) {
		std::error_code errCode;
		if (!exists(fName, errCode)) {
			return boost::none;
		}
		std::unique_ptr<TCHAR[]> buf;
		DWORD bufSize = 1024;
		while (true) {
			buf.reset(new(std::nothrow) TCHAR[bufSize]);

			if (buf == nullptr) {
				break;
			}
			errno = 0;
			const auto len = GetPrivateProfileString(section.c_str(), name.c_str(), defValue.c_str(), buf.get(), bufSize, fName.c_str());
			if (ENOENT == errno) {
				errno = 0;
				return boost::none;
			}
			if (bufSize - 1 > len) {
				break;
			}
			auto z1 = heapMaxReqReal;
			auto z2 = UINT_MAX;
#ifdef _WIN64
					const DWORD maxBufSize = UINT_MAX / sizeof buf[ 0 ];
#else
			const DWORD maxBufSize = heapMaxReqReal / sizeof TCHAR;
#endif
			if (maxBufSize == bufSize) {
				break;
			}
			bufSize = maxBufSize / 2 >= bufSize
				          ? 2 * bufSize
				          : maxBufSize;
		}
		if (buf == nullptr) {
			return boost::none;
		}
		if (nullptr == buf) {
			return boost::none;
		}
		return buf.get();
	}

	inline void IniFile::setDefaults(const std::tstring& aSection, const std::tstring& aName, const std::tstring& aDefault) {
		setValue(aSection, aName, aDefault, true);
	}

	inline boost::optional<std::tstring> IniFile::getValue(const std::tstring& section, const std::tstring& name) const {
		const iniKey thisKey(section, name);
		const auto ref = values.find(thisKey);
		if (values.end() == ref) {
			return boost::none;
		}
		auto result = ref->second.getValue();
		if (result.empty()) {
			return boost::none;
		}
		return result;
	}

	inline boost::optional<std::tstring> IniFile::getDefault(const std::tstring& section, const std::tstring& name) const {
		const iniKey thisKey(section, name);
		const auto ref = values.find(thisKey);
		if (values.end() == ref) {
			return boost::none;
		}
		auto result = ref->second.getDefault();
		if (result.empty()) {
			return boost::none;
		}
		return result;
	}

	inline void IniFile::setValue(const std::tstring& section, const std::tstring& name, const std::tstring& value, const bool isDefault) {
		iniKey thisKey(section, name);
		const auto ref = values.find(thisKey);
		if (values.end() == ref) {
			values.insert(std::pair<iniKey, IniValue>(thisKey, IniValue(value, isDefault, false)));
		} else {
			ref->second.setValue(value, isDefault, false);
		}
	}
}
