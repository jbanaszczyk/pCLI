/******************************************************************************
 *
 * Copyright 2011 jacek.banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

/******************************************************************************
*
* class pApps::YesNoOption converts strings to boolan
*
* Supported strings: "Yes", "No", "true", "false", "1", "0"
*
* addString(tstring, bool)
*   adds/modifies more string/bool conversions
*
* functor ()
*   translates tstring or boost::optional<tstring> to bool
*
*****************************************************************************/
#include "./common.h"

namespace p_apps {

	class YesNoOption {
		private:
			template <typename T>
			struct fuLess: std::binary_function<T, T, bool> {
				auto operator()(const T& s1, const T& s2) const -> bool {
					return boost::ilexicographical_compare(s1, s2);
				}
			};

			std::map<std::tstring, bool, fuLess<std::tstring>> _values;

			auto getOption(const std::tstring& key, const bool defValue) const -> bool {
				auto itValues = _values.find(key);
				if (_values.end() == itValues){
					return defValue;
				}
				return itValues->second;
			};

		public:
			YesNoOption()
				: _values({
					{_T("yes"), true},
					{_T("true"), true},
					{_T("1"), true},
					{_T("no"), false},
					{_T("false"), false},
					{_T("0"), false}
				}) {
			}

			~YesNoOption() {
			};

			auto addString(const std::tstring& key, const bool value) -> void {
				_values[ key ] = value;
			}

			auto operator()(const std::tstring& key, const bool defValue = true) -> bool {
				return getOption(key, defValue);
			};

			auto operator()(const boost::optional<std::tstring>& key, const bool defValue = true) -> bool {
				if (!key){
					return defValue;
				}
				return getOption(key.get(), defValue);
			};

	};
}
