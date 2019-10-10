/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#include "stdafx.h"

#include "../common/YesNoOption.h"
#include <boost/optional/optional.hpp>

using namespace testing;

TEST(YesNoOption, yesLowerCase) {
    p_apps::YesNoOption yesNoOption;
    ASSERT_EQ(1, yesNoOption(_T("yes")));
    ASSERT_EQ(1, yesNoOption(_T("true")));
    ASSERT_EQ(1, yesNoOption(_T("1")));
}

TEST(YesNoOption, yesMixedCase) {
    p_apps::YesNoOption yesNoOption;
    ASSERT_EQ(1, yesNoOption(_T("yEs")));
    ASSERT_EQ(1, yesNoOption(_T("TRUE")));
}

TEST(YesNoOption, noLowerCase) {
    p_apps::YesNoOption yesNoOption;
    ASSERT_EQ(0, yesNoOption(_T("no")));
    ASSERT_EQ(0, yesNoOption(_T("false")));
    ASSERT_EQ(0, yesNoOption(_T("0")));
}

TEST(YesNoOption, noMixedCase) {
    p_apps::YesNoOption yesNoOption;
    ASSERT_EQ(0, yesNoOption(_T("nO")));
    ASSERT_EQ(0, yesNoOption(_T("FALSE")));
}

TEST(YesNoOption, yesDefault) {
    p_apps::YesNoOption yesNoOption;
    ASSERT_EQ(1, yesNoOption(_T("yyy"), 1));
    ASSERT_EQ(1, yesNoOption(_T(""), 1));
}

TEST(YesNoOption, noDefault) {
    p_apps::YesNoOption yesNoOption;
    ASSERT_EQ(0, yesNoOption(_T("yyy"), 0));
    ASSERT_EQ(0, yesNoOption(_T(""), 0));
}


TEST(YesNoOption, yesOptional) {
    p_apps::YesNoOption yesNoOption;
    ASSERT_EQ(1, yesNoOption(boost::optional<std::tstring>(_T("yes"))));
    ASSERT_EQ(1, yesNoOption(boost::optional<std::tstring>(_T("yes"), 1)));
    ASSERT_EQ(1, yesNoOption(boost::optional<std::tstring>(_T("yes"), 0)));
    ASSERT_EQ(1, yesNoOption(boost::none, 1));
}

TEST(YesNoOption, noOptional) {
    p_apps::YesNoOption yesNoOption;
    ASSERT_EQ(0, yesNoOption(boost::optional<std::tstring>(_T("no"))));
    ASSERT_EQ(0, yesNoOption(boost::optional<std::tstring>(_T("no"), 1)));
    ASSERT_EQ(0, yesNoOption(boost::optional<std::tstring>(_T("no"), 0)));
    ASSERT_EQ(0, yesNoOption(boost::none, 0));
}
