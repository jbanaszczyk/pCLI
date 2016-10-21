/*****************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * part of PortableApps Launcher project
 * http://sourceforge.net/projects/tccleportable/
 * Release 1.0.0
 *
 *****************************************************************************/

#ifndef _FD07B0C1_AC69_4C9C_AE11_F56D9CC86B31_INCLUDED
#define _FD07B0C1_AC69_4C9C_AE11_F56D9CC86B31_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif

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

#include <tchar.h>
#include <map>

namespace pApps {

	class YesNoOption {
	private:
		template<typename T>
		struct fuLess : std::binary_function < T, T, bool > {
			bool operator() (const T& s1, const T& s2) const {
				return boost::ilexicographical_compare(s1, s2);
			}
		};
		std::map< std::tstring, bool, fuLess<std::tstring>> _values;
	
		bool getOption(const std::tstring& key, const bool defValue) const {
			auto itValues = _values.find(key);
			if (_values.end() == itValues)
				return defValue;
			return itValues->second;
		};

	public:
		YesNoOption() : _values({
			{ _T("yes"), true },
			{ _T("true"), true },
			{ _T("1"), true },
			{ _T("no"), false },
			{ _T("false"), false },
			{ _T("0"), false }
		}) { }

			~YesNoOption() {};

			void addString(const std::tstring& key, const bool value) {
				_values[key] = value;
			}

			bool operator()(const std::tstring& key, const bool defValue = true) {
				return getOption(key, defValue);
			};

			bool operator()(const boost::optional<std::tstring>& key, const bool defValue = true) {
				if (!key)
					return defValue;
				return getOption(key.get(), defValue);
			};

	};
}

#endif  // _FD07B0C1_AC69_4C9C_AE11_F56D9CC86B31_INCLUDED
