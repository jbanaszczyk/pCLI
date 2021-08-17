// *************************************************************
// * Copyright 2011 jacek.banaszczyk@gmail.com                 *
// * Part of pCli project: https://github.com/jbanaszczyk/pCli *
// *************************************************************

#pragma once

namespace p_apps {

	template <typename T>
	struct CaseInsensitiveMap {
		struct Comparator {
			bool operator()(const T& lhs, const T& rhs) const {
				return boost::ilexicographical_compare(lhs, rhs);
			}
		};
	};

	template <typename T>
	struct CaseInsensitivePairFirst {
		struct Comparator {
			bool operator()(const std::pair<T, T>& lhs, const std::pair<T, T>& rhs) const {
				return boost::ilexicographical_compare(lhs.first, rhs.first) || (boost::iequals(lhs.first, rhs.first) && boost::ilexicographical_compare(lhs.second, rhs.second));
			}
		};
	};

	using SetInsensitiveTchar = std::set<std::tstring, CaseInsensitiveMap<std::tstring>::Comparator>; // FIXME move somewhere near usage
}
