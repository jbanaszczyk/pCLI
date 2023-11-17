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
 *     { L"Section", L"Name", L"Default" }, ... }
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
#include "pAppsUtils.h"

namespace p_apps {
	class IniFile {  // NOLINT(cppcoreguidelines-special-member-functions)

		class IniValue {
			std::wstring valueDefault; // default
			std::wstring valueInterim; // was read from file
			std::wstring valueCurrent; // effective
		public:
			IniValue(const std::wstring& value, bool setDefault, bool setInterim);

			void setValue(const std::wstring& value, bool setDefault, bool setInterim);

			void propagate();

			[[nodiscard]] std::wstring getDefault() const;

			[[nodiscard]] std::wstring getValue() const;

			[[nodiscard]] bool isModified() const;
		};

		using IniKey = std::pair<std::wstring, std::wstring>;

		std::map<IniKey, IniValue, CaseInsensitivePairFirst<std::wstring>::Comparator> values;

		// ReSharper disable once CppInconsistentNaming
		std::filesystem::path iniName_;

	public:
		struct IniDefaults {
			std::wstring aSection;
			std::wstring aName;
			std::wstring aValue;
		};

		static std::optional<std::wstring> iniRead(const std::filesystem::path& fName, const std::wstring& section, const std::wstring& name, const std::wstring& defValue = L"");

		static void iniNames(const wchar_t* sectionName, const std::filesystem::path& fName, std::vector<std::wstring>& sections);

		IniFile();

		~IniFile();

		void setDefaults(const std::wstring& aSection, const std::wstring& aName, const std::wstring& aDefault);

		void setDefaults(const IniDefaults* defaults, size_t nElements = 1);

		void readIniFile(const std::filesystem::path& iniName);

		[[nodiscard]] std::optional<std::wstring> getValue(const std::wstring& section, const std::wstring& name) const;

		[[nodiscard]] std::optional<std::wstring> getDefault(const std::wstring& section, const std::wstring& name) const;

		[[nodiscard]] std::wstring getValueNonEmpty(const std::wstring& section, const std::wstring& name) const;

		void enumSections(SetInsensitiveTChar& result) const;

		void enumNames(const std::wstring& section, SetInsensitiveTChar& result) const;

		void setValue(const std::wstring& section, const std::wstring& name, const std::wstring& value, bool isDefault = false);

		void setValue(const IniDefaults& defaults, bool isDefault = false);
	};
}
