/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

/******************************************************************************
 *
 * There a static method
 *
 * expandEnv
 *   Expand environment variable strings
 *
 * class pApps::Environment represents set of environment variables
 * Environment is a simple, case insensitive map of tstrings
 * Environment should be initialized from envp[] array
 * manipulated using get/set/erase
 * then dumped to another envp[] array and passed to launched program
 *
 * Probably there will exist a single object of class Environment
 *
 *****************************************************************************/

#include "./common.h"

#include <tchar.h>
#include <map>

#include <boost/algorithm/string.hpp>

namespace pApps {
	class Environment {
	  public:
		static std::tstring expandEnv(const std::tstring str);

	  private:
		template<typename T>
		struct fuLess : std::binary_function < T, T, bool > {
			bool operator() (const T& s1, const T& s2) const {
				return boost::ilexicographical_compare(s1, s2);
			}
		};
		typedef std::map<std::tstring, std::tstring, fuLess<std::tstring>> IMap;
		IMap _mEnv;
	  public:
		Environment(const TCHAR *const envp[]);
		void setUp(const TCHAR *const envp[]);
		bool exists(const std::tstring name) const;
		std::tstring get(const std::tstring name);
		void set(const std::tstring name, const std::tstring value, const std::tstring keyBackup = _T(""));
		void erase(const std::tstring name);
		void dump(std::vector<std::tstring>& result) const;
		void dump(std::unique_ptr<TCHAR[]>& result) const;
	};
}
