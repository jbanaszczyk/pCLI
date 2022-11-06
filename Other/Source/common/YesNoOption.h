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
#include "pAppsUtils.h"

namespace p_apps {

	class YesNoOption {

		std::map<std::tstring, bool, CaseInsensitiveMap<std::tstring>::Comparator> keyValues;

		const bool defaultValue = true;

		[[nodiscard]] bool getOption(const std::tstring& key) const {
			const auto itValues = keyValues.find(key);
			return itValues == keyValues.end()
				? defaultValue
				: itValues->second;
		}

	public:
		YesNoOption()
			: keyValues({
				{_T("yes"), true},
				{_T("true"), true},
				{_T("1"), true},
				{_T("no"), false},
				{_T("false"), false},
				{_T("0"), false}
				})
		{
		}

		~YesNoOption() = default;

		YesNoOption(const YesNoOption& other) = delete;

		YesNoOption(YesNoOption&& other) noexcept = delete;

		YesNoOption& operator=(const YesNoOption& other) = delete;

		YesNoOption& operator=(YesNoOption&& other) noexcept = delete;

		bool operator()(const std::tstring& key) const {
			return getOption(key);
		}

		bool operator()(const boost::optional<std::tstring>& key) const {
			return key
				? getOption(key.get())
				: defaultValue;
		}

		bool operator()(const std::optional<std::tstring>& key) const {
			return key
				? getOption(key.value())
				: defaultValue;
		}

	};
}
