// *************************************************************
// * Copyright 2011 jacek.banaszczyk@gmail.com                 *
// * Part of pCli project: https://github.com/jbanaszczyk/pCli *
// *************************************************************

#pragma once
#include "common.h"

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

	using SetInsensitiveTChar = std::set<std::wstring, CaseInsensitiveMap<std::wstring>::Comparator>; // FIXME move somewhere near usage

	/******************************************************************************
	*	Merge two STL containers
	*	used to merge vectors
	*****************************************************************************/
	template <typename T, typename U>
	void appendContainer(T& destination, const U& source) {
		// FIXME try to simplify
		if (!source.empty()) {
			destination.reserve(destination.size() + source.size());
			destination.insert(destination.end(), source.begin(), source.end());
		}
	}


}
