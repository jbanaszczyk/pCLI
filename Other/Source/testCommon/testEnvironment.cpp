/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#include "stdafx.h"

#include "../common/Environment.h"

/*
#include <functional>

BOOST_AUTO_TEST_SUITE(Environment)

BOOST_AUTO_TEST_CASE(ctor) {
	pApps::Environment env;
	env.set(_T("key"), _T("val"));
}

DWORD WINAPI Mock_ExpandEnvironmentStrings__________(LPCTSTR lpSrc, LPTSTR lpDst, DWORD nSize) {
	std::tstring src(lpSrc);
	boost::to_upper(src);

	std::tstring retVal(lpSrc);

	if (retVal.front() == _T('%') && retVal.back() == _T('%') && retVal.length() >= 2) {
		retVal.erase(retVal.end() - 1);
		retVal.erase(retVal.begin());
	}
	return 0;
}

DWORD WINAPI Mock_ExpandEnvironmentStrings(LPCTSTR lpSrc, LPTSTR lpDst, DWORD nSize) {
	std::cout << "Mocking" << std::endl;
	return 0;
}

class MockedEnvironment {
public:
	DWORD WINAPI cMock_ExpandEnvironmentStrings(LPCTSTR lpSrc, LPTSTR lpDst, DWORD nSize) {
		return 0;
	}
};

MOCK_CLASS(mock_class) {};

//	std::tstring tmp;
//	tmp =    pApps::Environment::expandEnv( _T( "%TMP%" ) );
//	tmp =    pApps::Environment::expandEnv( _T( "%windir%" ) );
//	tmp =    pApps::Environment::expandEnv( _T( "%WINDIR%" ) );
//	BOOST   _CHECK( tmp == _T("C:\\TEMP" ));
//	tmp =     pApps::Environment::expandEnv( _T( "%TEMP%" ), Mock_ExpandEnvironmentStrings );

// auto fp = std::bind( &MockedEnvironment::Mock_ExpandEnvironmentStrings, mock, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );
// fp( _T( "%TEMP%" ), retVal, 0 );

//	std::function<void( int, int )>

//std::tstring tmp = pApps::Environment::expandEnv( _T( "%TEMP%" ), fp);

//		boost::function1( fp );

BOOST_AUTO_TEST_SUITE_END()

*/
