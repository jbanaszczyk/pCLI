/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#include "stdafx.h"
#include <boost/test/unit_test.hpp>

#include "../common/CtrlHandler.cpp"

#include  <boost/serialization/singleton.hpp>

struct A {
	std::string name_;
	int age_;
};

struct MyClass : public boost::serialization::singleton<MyClass>, public A {
};

BOOST_AUTO_TEST_CASE( CtrlHandler ) {

	pApps::CtrlHandler::getInstance().setDebugBeep( true );

	MyClass::get_mutable_instance().name_ = "Robin";
	MyClass::get_mutable_instance().age_ = 21;

	std::cin.get();

	/*
	// seven ways to detect and report the same error:
	BOOST_CHECK( add( 2, 2 ) == 4 );        // #1 continues on error

	BOOST_REQUIRE( add( 2, 2 ) == 4 );      // #2 throws on error

	if ( add( 2, 2 ) != 4 )
	    BOOST_ERROR( "Ouch..." );            // #3 continues on error

	if ( add( 2, 2 ) != 4 )
	    BOOST_FAIL( "Ouch..." );             // #4 throws on error

	if ( add( 2, 2 ) != 4 ) throw "Ouch..."; // #5 throws on error

	BOOST_CHECK_MESSAGE( add( 2, 2 ) == 4,  // #6 continues on error
	    "add(..) result: " << add( 2, 2 ) );

	BOOST_CHECK_EQUAL( add( 2, 2 ), 4 );      // #7 continues on error
	*/
}
