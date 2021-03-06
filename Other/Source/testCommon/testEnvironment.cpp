/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#include "stdafx.h"

#include <string>
#include <functional>

#include "../common/Environment.h"

using namespace testing;

TEST(Environment, ctor) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, withEmpty) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), nullptr, _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.size(), 1);
}

TEST(Environment, exists) {
	const TCHAR* const        envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	const p_apps::Environment env(envp);

	ASSERT_FALSE(env.exists(_T("")));
	ASSERT_TRUE(env.exists(_T("arg1")));
	ASSERT_TRUE(env.exists(_T("arg2")));
	ASSERT_FALSE(env.exists(_T("dummy")));
}

TEST(Environment, get) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.size(), 2);

	ASSERT_EQ(env.get(_T("")), _T(""));
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg2")), _T("val2"));
	ASSERT_EQ(env.get(_T("dummy")), _T(""));

	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, setupErr01) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T(""), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.size(), 1);
}

TEST(Environment, setupErr02) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("dummy="), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, setupErr03) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("=dummy"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, erase01) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.size(), 2);
	env.erase(_T("arg1"));
	ASSERT_EQ(env.size(), 1);
	env.erase(_T("arg2"));
	ASSERT_EQ(env.size(), 0);
}

TEST(Environment, erase02) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.size(), 2);
	env.erase(_T("arg2"));
	ASSERT_EQ(env.size(), 1);
}

TEST(Environment, erase03) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.size(), 2);
	env.erase(_T("arg3"));
	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, erase04) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.size(), 2);
	env.erase(_T(""));
	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, set01) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	env.set(_T("arg1"), _T("val1"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg2")), _T("val2"));
}

TEST(Environment, set02) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	env.set(_T("arg2"), _T("val2"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg2")), _T("val2"));
}

TEST(Environment, set03) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	env.set(_T("arg3"), _T("val3"));
	ASSERT_EQ(env.size(), 3);
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg2")), _T("val2"));
	ASSERT_EQ(env.get(_T("arg3")), _T("val3"));
}

TEST(Environment, set04) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	env.set(_T("arg1"), _T(""));
	ASSERT_EQ(env.size(), 1);
	ASSERT_FALSE(env.exists(_T("arg1")));
}

TEST(Environment, set05) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	env.set(_T(""), _T("val1"));
	ASSERT_EQ(env.size(), 2);
}

TEST(Environment, setBak01) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	env.set(_T("arg1"), _T("val1"), _T("arg1.bak"));
	ASSERT_EQ(env.size(), 3);
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg1.bak")), _T("val1"));
}

TEST(Environment, setBak02) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	env.set(_T("arg1"), _T("val1"), _T("arg2"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("arg1")), _T("val1"));
	ASSERT_EQ(env.get(_T("arg2")), _T("val1"));
}

TEST(Environment, setBak03) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	env.set(_T("arg3"), _T("val3"), _T("arg2"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("arg3")), _T("val3"));
	ASSERT_EQ(env.get(_T("arg2")), _T(""));
}

TEST(Environment, setBak04) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	env.set(_T(""), _T("val"), _T("arg1.bak"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("arg1.bak")), _T(""));
}

TEST(Environment, letterCase01) {
	const TCHAR* const        envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	const p_apps::Environment env(envp);

	ASSERT_TRUE(env.exists(_T("ArG1")));
	ASSERT_TRUE(env.exists(_T("aRg2")));
}

TEST(Environment, letterCase02) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.get(_T("ArG1")), _T("val1"));
	ASSERT_EQ(env.get(_T("aRg2")), _T("val2"));
}

TEST(Environment, letterCase03) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	ASSERT_EQ(env.size(), 2);
	env.erase(_T("ArG1"));
	ASSERT_EQ(env.size(), 1);
	env.erase(_T("aRg2"));
	ASSERT_EQ(env.size(), 0);
}

TEST(Environment, letterCase04) {
	const TCHAR* const  envp[] = { _T("arg1=val1"), _T("arg2=val2"), _T("") };
	p_apps::Environment env(envp);

	env.set(_T("ArG1"), _T("VaL1"));
	ASSERT_EQ(env.size(), 2);
	ASSERT_EQ(env.get(_T("aRg1")), _T("VaL1"));
}

//////////////////////////

static std::function<DWORD(LPCWSTR lpSrc, LPWSTR lpDst, DWORD nSize)> forwardExpandEnvironmentStrings;

DWORD WINAPI mockExpandEnvironmentStrings(const LPCWSTR lpSrc, const LPWSTR lpDst, const DWORD nSize) {
	return forwardExpandEnvironmentStrings(lpSrc, lpDst, nSize);
}

class MockExpandEnvironmentStrings {
public:
	MockExpandEnvironmentStrings() {
		forwardExpandEnvironmentStrings = [this](const LPCWSTR lpSrc, const LPWSTR lpDst, const DWORD nSize) {
			return mockExpandEnvironmentStrings(lpSrc, lpDst, nSize);
		};
	}

	MOCK_CONST_METHOD3_WITH_CALLTYPE(WINAPI, mockExpandEnvironmentStrings, DWORD(const LPCTSTR, LPWSTR, const DWORD));

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

	WCHAR argument[]{ _T("_%TEMP%_") };
	WCHAR expanded[]{ _T("_C:\\TEMP_") };

	ON_CALL(mock, mockExpandEnvironmentStrings(_, _, _))
		.WillByDefault(Return(static_cast<DWORD>(sizeof expanded)));
	EXPECT_CALL(mock, mockExpandEnvironmentStrings(_, nullptr, 0));
	EXPECT_CALL(mock, mockExpandEnvironmentStrings(_, _, static_cast<DWORD>(sizeof expanded)))
		.WillOnce(cpy2arg1(expanded));

	auto retVal = p_apps::Environment::expandEnv(std::tstring{ argument }, mockExpandEnvironmentStrings);
	EXPECT_EQ(retVal, expanded);
}

TEST(Environment, expandEnvFail) {
	const MockExpandEnvironmentStrings mock;

	WCHAR argument[]{ _T("_%TEMP%_") };

	ON_CALL(mock, mockExpandEnvironmentStrings(_, _, _))
		.WillByDefault(Return(static_cast<DWORD>(0)));
	EXPECT_CALL(mock, mockExpandEnvironmentStrings(_, nullptr, 0));

	auto retVal = p_apps::Environment::expandEnv(std::tstring{ argument }, mockExpandEnvironmentStrings);
	EXPECT_EQ(retVal, argument);
}

//////////////////////////

static void rebuildEnvpIndex(TCHAR* result[], const size_t envpSize, TCHAR* dumped) {	
	for (auto index = decltype(envpSize){0}; index < envpSize; ++index) {
		*result = dumped;
		dumped += wcslen(dumped);
		++result;
	}
}

TEST(Environment, dump) {
	const TCHAR* envp[] = { _T("arg1=val1"), _T("arg222=val333"), _T("") };       
	const size_t envpSize = sizeof envp / sizeof envp[0];
	const p_apps::Environment env(envp);

	TCHAR* dumped = env.dump().get();
	TCHAR* result[envpSize];
	rebuildEnvpIndex(result, envpSize, dumped);

	ASSERT_THAT(envp, ::testing::UnorderedElementsAreArray(envp));
}
