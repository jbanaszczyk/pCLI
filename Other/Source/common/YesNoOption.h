// *************************************************************
// * Copyright 2011 jacek.banaszczyk@gmail.com                 *
// * Part of pCli project: https://github.com/jbanaszczyk/pCli *
// *************************************************************

#pragma once

/******************************************************************************
*
* class pApps::YesNoOption converts strings to boolean
*
* Supported strings: "Yes", "No", "true", "false", "1", "0"
*
* functor ()
*   translates tstring or boost::optional<tstring> to bool
*
*****************************************************************************/
#include "./common.h"

namespace p_apps {

	class YesNoOption {
		template <typename T>
		struct fuLess : std::binary_function<T, T, bool> {
			bool operator()(const T& s1, const T& s2) const {
				return boost::ilexicographical_compare(s1, s2);
			}
		};

		std::map<std::tstring, bool, fuLess<std::tstring>> keyValues;
		bool defaultValue;

		bool getOption(const std::tstring& key) const {
			const auto itValues = keyValues.find(key);
			return itValues == keyValues.end()
				       ? defaultValue
				       : itValues->second;
		}

	public:
		YesNoOption(const bool defaultValue = true)
			: keyValues({
				{_T("yes"), true},
				{_T("true"), true},
				{_T("1"), true},
				{_T("no"), false},
				{_T("false"), false},
				{_T("0"), false}
			})
			, defaultValue(defaultValue) {
		}

		~YesNoOption() = default;

		bool operator()(const std::tstring& key) const {
			return getOption(key);
		}

		bool operator()(const boost::optional<std::tstring>& key) const {
			return key
				       ? getOption(key.get())
				       : defaultValue;
		}

	};
}
