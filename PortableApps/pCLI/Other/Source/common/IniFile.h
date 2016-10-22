/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#ifndef _B2C0002B_EEA7_4BD5_BC8F_67A4C303052E_INCLUDED
#define _B2C0002B_EEA7_4BD5_BC8F_67A4C303052E_INCLUDED

#ifdef _MSC_VER
	#pragma once
#endif

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

#include "./common.h"

#include <tchar.h>
#include <set>
#include <map>
#include <windows.h>

#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace pApps {
	class iniFile {
	  public:
		static boost::optional<std::tstring> iniRead(const boost::filesystem::path fName, const std::tstring section, const std::tstring name, const std::tstring defValue = _T(""));
		static void iniNames(const TCHAR *const sectionName, const boost::filesystem::path fName, std::vector<std::tstring>& sections);

		struct iniDefaults {
			std::tstring aSection;
			std::tstring aName;
			std::tstring aValue;
		};

	  private:
		class iniValue {
		  private:
			std::tstring _valueDefault;  // default
			std::tstring _valueInterim;  // read from file
			std::tstring _valueCurrent;  // effective
		  public:
			iniValue() {};
			iniValue(const std::tstring value, const bool setDefault, const bool setInterim);
			void setValue(const std::tstring value, const bool setDefault, const bool setInterim);
			void propagate();  // update _valueInterim value to _valueCurrent. To be executed after writeIniFile
			std::tstring getDefault() const;
			std::tstring getValue() const;
			bool isModified() const;
		};

		template<typename T>
		struct fuPairLess : std::binary_function < T, T, bool > {
			bool operator() (const std::pair<T, T>& s1, const std::pair<T, T>& s2) const {
				return  boost::ilexicographical_compare(s1.first, s2.first) || (boost::iequals(s1.first, s2.first) && boost::ilexicographical_compare(s1.second, s2.second));
			}
		};

		typedef std::pair<std::tstring, std::tstring> iniKey;
		std::map<iniKey, iniValue, fuPairLess<std::tstring> > _values;

		boost::filesystem::path _iniName;
		bool _saveOnExit;

	  public:
		template<typename T>
		struct fuLess : std::binary_function < T, T, bool > {
			bool operator() (const T& s1, const T& s2) const {
				return boost::ilexicographical_compare(s1, s2);
			}
		};

		typedef std::set<std::tstring, fuLess<std::tstring> > namesSet;

	  public:
		iniFile() : _iniName(_T("")), _saveOnExit(false) {};
		~iniFile();
		void setDefaults(const iniDefaults *defaults, const size_t nElements = 1);
		void setDefaults(const std::tstring aSection, const std::tstring aName, const std::tstring aDefault);
		void readIniFile(const boost::filesystem::path iniName, const bool saveOnExit );
		size_t writeIniFile();
		boost::optional<std::tstring> getValue(const std::tstring section, const std::tstring name) const;
		boost::optional<std::tstring> getDefault(const std::tstring section, const std::tstring name) const;
		std::tstring getValueNonEmpty(const std::tstring section, const std::tstring name) const;
		void enumSections(namesSet& result) const;
		void enumNames(const std::tstring section, namesSet& result) const;
		void setValue(const iniDefaults& defaults, const bool isDefault = false);
		void setValue(const std::tstring section, const std::tstring name, const std::tstring value, const bool isDefault = false);
	};
}
#endif  // _B2C0002B_EEA7_4BD5_BC8F_67A4C303052E_INCLUDED
