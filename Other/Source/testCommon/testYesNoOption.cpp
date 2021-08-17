// *************************************************************
// * Copyright 2011 jacek.banaszczyk@gmail.com                 *
// * Part of pCli project: https://github.com/jbanaszczyk/pCli *
// *************************************************************

#include "stdafx.h"

#include <functional>
#include <string>

#include "../common/YesNoOption.h"

using namespace testing;

class YesNo: public testing::TestWithParam<std::tuple<bool, std::tstring>> {
};

TEST_P(YesNo, Insensitive) {
	const auto expectation = std::get<0>(GetParam());
	const auto& argument = std::get<1>(GetParam());
	const p_apps::YesNoOption yesNoOption;
	ASSERT_EQ(yesNoOption(argument), expectation);
}

INSTANTIATE_TEST_SUITE_P(
	YesNoTchar,
	YesNo,
	::testing::Values(
		std::make_tuple(true, _T("Yes")),
		std::make_tuple(true, _T("yes")),
		std::make_tuple(true, _T("yEs")),
		std::make_tuple(true, _T("true")),
		std::make_tuple(true, _T("TRUE")),
		std::make_tuple(true, _T("1")),
		std::make_tuple(false, _T("No")),
		std::make_tuple(false, _T("no")),
		std::make_tuple(false, _T("false")),
		std::make_tuple(false, _T("FALSE")),
		std::make_tuple(false, _T("0")),
		std::make_tuple(true, _T("What"))
	));


TEST(YesNoOptional, optionalTrue) {
	const p_apps::YesNoOption yesNoOption;
	ASSERT_EQ(true, yesNoOption(boost::optional<std::tstring>(_T("True"))));
}

TEST(YesNoOptional, optionalFalse) {
	const p_apps::YesNoOption yesNoOption;
	ASSERT_EQ(false, yesNoOption(boost::optional<std::tstring>(_T("False"))));
}

TEST(YesNoOptional, optionalSomething) {
	const p_apps::YesNoOption yesNoOption;
	ASSERT_EQ(true, yesNoOption(boost::optional<std::tstring>(_T("What"))));
}

TEST(YesNoOptional, optionalNone) {
	const p_apps::YesNoOption yesNoOption;
	ASSERT_EQ(true, yesNoOption(boost::none));
}

TEST(YesNoOptional, optionalTrueWithDefaultTrue) {
	const p_apps::YesNoOption yesNoOption(true);
	ASSERT_EQ(true, yesNoOption(boost::optional<std::tstring>(_T("True"))));
}

TEST(YesNoOptional, optionalFalseWithDefaultTrue) {
	const p_apps::YesNoOption yesNoOption(true);
	ASSERT_EQ(false, yesNoOption(boost::optional<std::tstring>(_T("False"))));
}

TEST(YesNoOptional, optionalSomethingWithDefaultTrue) {
	const p_apps::YesNoOption yesNoOption(true);
	ASSERT_EQ(true, yesNoOption(boost::optional<std::tstring>(_T("What"))));
}

TEST(YesNoOptional, optionalNoneWithDefaultTrue) {
	const p_apps::YesNoOption yesNoOption(true);
	ASSERT_EQ(true, yesNoOption(boost::none));
}

TEST(YesNoOptional, optionalTrueWithDefaultFalse) {
	const p_apps::YesNoOption yesNoOption(false);
	ASSERT_EQ(true, yesNoOption(boost::optional<std::tstring>(_T("True"))));
}

TEST(YesNoOptional, optionalFalseWithDefaultFalse) {
	const p_apps::YesNoOption yesNoOption(false);
	ASSERT_EQ(false, yesNoOption(boost::optional<std::tstring>(_T("False"))));
}

TEST(YesNoOptional, optionalSomethingWithDefaultFalse) {
	const p_apps::YesNoOption yesNoOption(false);
	ASSERT_EQ(false, yesNoOption(boost::optional<std::tstring>(_T("What"))));
}

TEST(YesNoOptional, optionalNoneWithDefaultFalse) {
	const p_apps::YesNoOption yesNoOption(false);
	ASSERT_EQ(false, yesNoOption(boost::none));
}
