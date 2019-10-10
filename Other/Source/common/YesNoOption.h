/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

 /******************************************************************************
 *
 * class pApps::YesNoOption converts strings to boolean
 *
 * Supported strings (case insensitive):
 *   "Yes", "No", "true", "false", "1", "0"
 *
 * functor ()
 *   translates tstring or boost::optional<tstring> to bool
 *
 *****************************************************************************/
#include "./common.h"
#include <boost/optional/optional.hpp>

namespace p_apps {

	class YesNoOption {
		template <typename T>
		struct fuLess : std::binary_function<T, T, bool> {
			bool operator()(const T& s1, const T& s2) const {
				return boost::ilexicographical_compare(s1, s2);
			}
		};

		std::map<std::tstring, bool, fuLess<std::tstring>> allowedValues;

		bool getOption(const std::tstring& key, const bool defValue) const {
			const auto itValues = allowedValues.find(key);
			return
				allowedValues.end() != itValues
				? itValues->second
				: defValue;
		}

	public:
		YesNoOption() : allowedValues({
			{_T("yes"), true}, {_T("true"), true}, {_T("1"), true},
			{_T("no"), false}, {_T("false"), false}, {_T("0"), false} })
		{}

		~YesNoOption() = default;

		bool operator()(const std::tstring& key, const bool defValue = true) const {
			return getOption(key, defValue);
		}

		bool operator()(const boost::optional<std::tstring>& key, const bool defValue = true) const {
			return
				key
				? getOption(key.get(), defValue)
				: defValue;
		}
	};
}
