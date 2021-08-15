/******************************************************************************
*
* Copyright 2011 jacek.banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#include "stdafx.h"

#include <functional>
#include <string>
#include <functional>

#include "../common/Environment.h"

using namespace testing;

TEST(Environment, ctor) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	const p_apps::Environment env(environment);

	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, exists) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	const p_apps::Environment env(environment);

	ASSERT_FALSE(env.exists(_T("")));
	ASSERT_TRUE(env.exists(_T("arg1")));
	ASSERT_TRUE(env.exists(_T("arg2")));
	ASSERT_FALSE(env.exists(_T("dummy")));
}

TEST(Environment, get) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	ASSERT_EQ(env.size(), 2);

	ASSERT_EQ(env.get(_T("")), _T(""));
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg2")), _T("val2"));
	ASSERT_EQ(env.get(_T("dummy")), _T(""));

	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, setupErr01) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T(""), _T("arg2=val2"), _T("")};
	const p_apps::Environment env(environment);

	ASSERT_EQ(env.size(), 1);
}

TEST(Environment, setupErr02) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("dummy="), _T("arg2=val2"), _T("")};
	const p_apps::Environment env(environment);

	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, setupErr03) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("=dummy"), _T("arg2=val2"), _T("")};
	const p_apps::Environment env(environment);

	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, erase01) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	ASSERT_EQ(env.size(), 2);
	env.erase(_T("arg1"));
	ASSERT_EQ(env.size(), 1);
	env.erase(_T("arg2"));
	ASSERT_EQ(env.size(), 0);
}

TEST(Environment, erase02) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	ASSERT_EQ(env.size(), 2);
	env.erase(_T("arg2"));
	ASSERT_EQ(env.size(), 1);
}

TEST(Environment, erase03) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	ASSERT_EQ(env.size(), 2);
	env.erase(_T("arg3"));
	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, erase04) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	ASSERT_EQ(env.size(), 2);
	env.erase(_T(""));
	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, set01) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	env.set(_T("arg1"), _T("val1"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg2")), _T("val2"));
}

TEST(Environment, set02) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	env.set(_T("arg2"), _T("val2"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg2")), _T("val2"));
}

TEST(Environment, set03) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	env.set(_T("arg3"), _T("val3"));
	ASSERT_EQ(env.size(), 3);
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg2")), _T("val2"));
	ASSERT_EQ(env.get(_T("arg3")), _T("val3"));
}

TEST(Environment, set04) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	env.set(_T("arg1"), _T(""));
	ASSERT_EQ(env.size(), 1);
	ASSERT_FALSE(env.exists(_T("arg1")));
}

TEST(Environment, set05) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	env.set(_T(""), _T("val1"));
	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, setBak01) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	env.set(_T("arg1"), _T("val1"), _T("arg1.bak"));
	ASSERT_EQ(env.size(), 3);
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg1.bak")), _T("val1"));
}

TEST(Environment, setBak02) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	env.set(_T("arg1"), _T("val1"), _T("arg2"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg2")), _T("val1"));
}

TEST(Environment, setBak03) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	env.set(_T("arg3"), _T("val3"), _T("arg2"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("arg3")), _T("val3"));
	ASSERT_EQ(env.get(_T("arg2")), _T(""));
}

TEST(Environment, setBak04) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	env.set(_T(""), _T("val"), _T("arg1.bak"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("arg1.bak")), _T(""));
}

TEST(Environment, case01) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	const p_apps::Environment env(environment);

	ASSERT_TRUE(env.exists(_T("ArG1")));
	ASSERT_TRUE(env.exists(_T("aRg2")));
}

TEST(Environment, case02) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	ASSERT_EQ(env.get(_T("ArG1")), _T("val1"));
	ASSERT_EQ(env.get(_T("aRg2")), _T("val2"));
}

TEST(Environment, case03) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	ASSERT_EQ(env.size(), 2);
	env.erase(_T("ArG1"));
	ASSERT_EQ(env.size(), 1);
	env.erase(_T("aRg2"));
	ASSERT_EQ(env.size(), 0);
}

TEST(Environment, case04) {
	const TCHAR* const environment[ ] = {_T("arg1=val1"), _T("arg2=val2"), _T("")};
	p_apps::Environment env(environment);

	env.set(_T("ArG1"), _T("VaL1"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("aRg1")), _T("VaL1"));
}

//////////////////////////

static std::function<DWORD(LPCWSTR lpSrc, LPWSTR lpDst, DWORD nSize)> forwardExpandEnvironmentStrings;

auto WINAPI mockExpandEnvironmentStrings(LPCWSTR lpSrc, LPWSTR lpDst, DWORD nSize) -> DWORD {
	return forwardExpandEnvironmentStrings(lpSrc, lpDst, nSize);
}

class MockExpandEnvironmentStrings {
	public:
		MockExpandEnvironmentStrings() {
			forwardExpandEnvironmentStrings = [this](LPCWSTR lpSrc, LPWSTR lpDst, DWORD nSize) {
				return mockExpandEnvironmentStrings(lpSrc, lpDst, nSize);
			};
		}

		MOCK_CONST_METHOD3_WITH_CALLTYPE(WINAPI, mockExpandEnvironmentStrings, DWORD(LPCTSTR, LPWSTR, DWORD));

		~MockExpandEnvironmentStrings() {
			forwardExpandEnvironmentStrings = {};
		}
};

ACTION_P(cpy2arg1, value) {
	wcsncpy_s(arg1, arg2 / sizeof(TCHAR), value, arg2);
	return arg2;
}

TEST(Environment, expandEnv) {
	const MockExpandEnvironmentStrings mock;

	WCHAR argument[ ]{_T("_%TEMP%_")};
	WCHAR expanded[ ]{_T("_C:\\TEMP_")};

	ON_CALL(mock, mockExpandEnvironmentStrings(_, _, _))
		.WillByDefault(Return(static_cast<DWORD>(sizeof expanded)));
	EXPECT_CALL(mock, mockExpandEnvironmentStrings(_, nullptr, 0));
	EXPECT_CALL(mock, mockExpandEnvironmentStrings(_, _, static_cast<DWORD>(sizeof expanded)))
		.WillOnce(cpy2arg1(expanded));

	const auto result = p_apps::Environment::expandEnv(std::tstring{argument}, mockExpandEnvironmentStrings);
	EXPECT_EQ(result, expanded);
}

TEST(Environment, expandEnvFail) {
	const MockExpandEnvironmentStrings mock;

	WCHAR argument[ ]{_T("_%TEMP%_")};

	ON_CALL(mock, mockExpandEnvironmentStrings(_, _, _))
		.WillByDefault(Return(static_cast<DWORD>(0)));
	EXPECT_CALL(mock, mockExpandEnvironmentStrings(_, nullptr, 0));

	const auto result = p_apps::Environment::expandEnv(std::tstring{argument}, mockExpandEnvironmentStrings);
	EXPECT_EQ(result, argument);
}
