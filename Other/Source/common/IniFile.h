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
 *   cache all setings.
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
	class iniFile {

		private:
			class iniValue {
				private:
					std::tstring valueDefault;  // default
					std::tstring valueInterim;  // was read from file
					std::tstring valueCurrent;  // effective
				public:
					iniValue(const std::tstring value, const bool setDefault, const bool setInterim)
						: valueDefault{},
						  valueInterim{},
						  valueCurrent{} {
						setValue(value, setDefault, setInterim);
					}

					auto setValue(const std::tstring value, const bool setDefault, const bool setInterim) -> void {
						if (setDefault){
							valueDefault = value;
						}
						if (setInterim){
							valueInterim = value;
						}
						valueCurrent = value;
					};

					auto propagate() -> void {  // update valueInterim to valueCurrent. To be executed after writeIniFile
						valueInterim = valueCurrent;
					};

					auto getDefault() const -> std::tstring {
						return valueDefault;
					};

					auto getValue() const -> std::tstring {
						return valueCurrent;
					};

					auto isModified() const -> bool {
						return !boost::equals(valueInterim, valueCurrent);
					};
			};

			using iniKey = std::pair<std::tstring, std::tstring>;

			std::map<iniKey, iniValue, CaseInsensitivePairFirst<std::tstring>::Comparator> values;

			boost::filesystem::path iniName_;
			bool saveOnExit_;

		public:
			struct iniDefaults {
				std::tstring aSection;
				std::tstring aName;
				std::tstring aValue;
			};

			static auto iniRead(const boost::filesystem::path fName, const std::tstring section, const std::tstring name, const std::tstring defValue = _T("")) -> boost::optional<std::tstring> {
				boost::system::error_code errCode;
				if (!exists(fName, errCode)){
					return boost::none;
				}
				std::unique_ptr<TCHAR[ ]> buf;
				DWORD bufSize = 1024;
				while (true){
					buf.reset(new(std::nothrow) TCHAR[ bufSize ]);

					if (buf == nullptr){
						break;
					}
					errno = 0;
					auto len = GetPrivateProfileString(section.c_str(), name.c_str(), defValue.c_str(), buf.get(), bufSize, fName.c_str());
					if (ENOENT == errno){
						errno = 0;
						return boost::none;
					}
					if (bufSize - 1 > len){
						break;
					}
#ifdef _WIN64
					const DWORD maxBufSize = UINT_MAX / sizeof buf[ 0 ];
#else
                const DWORD maxBufSize = heapMaxReqReal / sizeof TCHAR;
#endif
					if (maxBufSize == bufSize){
						break;
					}
					bufSize = maxBufSize / 2 >= bufSize
						          ? 2 * bufSize
						          : maxBufSize;
				}
				if (buf == nullptr){
					return boost::none;
				}
				if (nullptr == buf.get()){
					return boost::none;
				}
				return buf.get();
			}

			static auto iniNames(const TCHAR* const sectionName, const boost::filesystem::path fName, std::vector<std::tstring>& sections) -> void {
				boost::system::error_code errCode;
				if (!exists(fName, errCode)){
					return;
				}
				std::unique_ptr<TCHAR[ ]> bufPtr;

				DWORD bufSize = 1024;
				while (true){
					bufPtr.reset(new(std::nothrow) TCHAR[ bufSize ]);
					if (!bufPtr){
						break;
					}

					auto len = GetPrivateProfileString(sectionName, nullptr, nullptr, bufPtr.get(), bufSize, fName.c_str());
					errno = 0;
					if (ENOENT == errno){
						errno = 0;
						return;
					}
					if (bufSize - 2 > len){
						break;
					}

#ifdef _WIN64
					const DWORD maxBufSize = UINT_MAX / sizeof TCHAR;
#else
                const DWORD maxBufSize = heapMaxReqReal / sizeof TCHAR;
#endif
					if (maxBufSize == bufSize){
						break;
					}
					bufSize = maxBufSize / 2 >= bufSize
						          ? 2 * bufSize
						          : maxBufSize;
				}
				if (!bufPtr){
					return;
				}
				auto ref = bufPtr.get();
				while (_T('\0') != *ref){
					auto len = wcslen(ref);
					sections.push_back(ref);
					ref += len + 1;
				}
			}



		public:
			iniFile()
				: iniName_(_T("")),
				  saveOnExit_(false) {
			};

			~iniFile() {
				writeIniFile();
			}

			auto setDefaults(const std::tstring aSection, const std::tstring aName, const std::tstring aDefault) -> void {
				setValue(aSection, aName, aDefault, true);
			}


			auto setDefaults(const iniDefaults* defaults, const size_t nElements = 1) -> void {
				for (size_t idx = 0; nElements > idx; ++idx, ++defaults){
					setValue(*defaults, true);
				}
			};

			auto readIniFile(const boost::filesystem::path iniName, const bool saveOnExit) -> void {
				iniName_ = iniName;
				saveOnExit_ = saveOnExit;
				std::vector<std::tstring> sectionNames;
				iniNames(nullptr, iniName, sectionNames);
				for (auto itSections = begin(sectionNames); end(sectionNames) != itSections; ++itSections){
					std::vector<std::tstring> nameNames;
					iniNames(itSections->c_str(), iniName, nameNames);
					for (auto itNames = begin(nameNames); end(nameNames) != itNames; ++itNames){
						std::tstring defaultValue(_T(""));
						iniKey thisKey(itSections->c_str(), itNames->c_str());
						auto ref = values.find(thisKey);
						if (values.end() != ref){
							defaultValue = ref->second.getDefault();
						}
						auto valueStr = iniRead(iniName, itSections->c_str(), itNames->c_str(), defaultValue);
						if (valueStr){
							if (values.end() == ref){
								values.insert(std::pair<iniKey, iniValue>(thisKey, iniValue(valueStr.get(), false, true)));
							} else{
								ref->second.setValue(valueStr.get(), false, true);
							}
						}
					}
				}
			};

			auto writeIniFile() -> size_t {
				if (!saveOnExit_){
					return 0;
				}
				size_t result = 0;
				for (auto it = begin(values); end(values) != it; ++it){
					if (it->second.isModified()){
						if (WritePrivateProfileString(it->first.first.c_str(), it->first.second.c_str(), !it->second.getValue().empty()
							                                                                                 ? it->second.getValue().c_str()
							                                                                                 : nullptr, iniName_.c_str())){
							it->second.propagate();
							++result;
						}
					}
				}
				return result;
			}

			auto getValue(const std::tstring section, const std::tstring name) const -> boost::optional<std::tstring> {
				iniKey thisKey(section, name);
				auto ref = values.find(thisKey);
				if (values.end() == ref){
					return boost::none;
				}
				auto result = ref->second.getValue();
				if (result.empty()){
					return boost::none;
				}
				return result;
			}

			auto getDefault(const std::tstring section, const std::tstring name) const -> boost::optional<std::tstring> {
				iniKey thisKey(section, name);
				auto ref = values.find(thisKey);
				if (values.end() == ref){
					return boost::none;
				}
				auto result = ref->second.getDefault();
				if (result.empty()){
					return boost::none;
				}
				return result;
			}

			auto getValueNonEmpty(const std::tstring section, const std::tstring name) const -> std::tstring {
				iniKey thisKey(section, name);
				auto ref = values.find(thisKey);
				if (values.end() == ref){
					return _T("");
				}
				auto result = ref->second.getValue();
				if (result.empty()){
					result = ref->second.getDefault();
				}
				return result;
			}

			auto enumSections(SetInsensitiveTChar& result) const -> void {
				for (auto it = begin(values); end(values) != it; ++it){
					result.insert(it->first.first);
				}
			}

			auto enumNames(const std::tstring section, SetInsensitiveTChar& result) const -> void {
				for (auto it = begin(values); end(values) != it; ++it){
					if (boost::iequals(section, it->first.first)){
						result.insert(it->first.second);
					}
				}
			}

			auto setValue(const std::tstring section, const std::tstring name, const std::tstring value, const bool isDefault = false) -> void {
				iniKey thisKey(section, name);
				auto ref = values.find(thisKey);
				if (values.end() == ref){
					values.insert(std::pair<iniKey, iniValue>(thisKey, iniValue(value, isDefault, false)));
				} else{
					ref->second.setValue(value, isDefault, false);
				}
			}

			auto setValue(const iniDefaults& defaults, const bool isDefault = false) -> void {
				setValue(defaults.aSection, defaults.aName, defaults.aValue, isDefault);
			}

	};
}
