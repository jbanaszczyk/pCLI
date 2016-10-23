/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#include "stdafx.h"
#include <boost/test/unit_test.hpp>

#include "../common/CtrlHandler.cpp"

static volatile int ctrlCCounter = 0;

static BOOL WINAPI mockCtrlHandler ( _In_ DWORD ctrlType ) {
	++ctrlCCounter;
	return TRUE;
}

class mockCtrlHandlerManager : public pApps::CtrlHandlerManager {
	public:
		BOOL WINAPI vSetConsoleCtrlHandler ( _In_opt_ PHANDLER_ROUTINE HandlerRoutine, _In_ BOOL Add ) override {
			if ( Add < 2 ) {
				return SetConsoleCtrlHandler ( HandlerRoutine, Add );
			}

			Add -= 2;
			ctrlCCounter = 0;
			return SetConsoleCtrlHandler ( mockCtrlHandler, Add );
		}
};

BOOST_AUTO_TEST_CASE ( CtrlHandler, *boost::unit_test::disabled() ) {

	// A bit difficult
	// shut down installed CtrlHandler
	pApps::CtrlHandler_.activate ( false );

	// mocked ctor invokes activate again
	mockCtrlHandlerManager mockCtrlHandler_;
	// so deactivate again
	mockCtrlHandler_.activate ( FALSE );

	// install mockCtrlHandler as CtrlHandler
	mockCtrlHandler_.activate ( TRUE + 2 );

	BOOST_CHECK ( ctrlCCounter == 0 );
	GenerateConsoleCtrlEvent ( 0, 0 ); // generate CtrlC
	Sleep ( 1 );                       // yield to allow processing
	BOOST_CHECK ( ctrlCCounter == 1 );
	GenerateConsoleCtrlEvent ( 1, 0 ); // generate CtrlBreak
	Sleep ( 1 );                      // yield to allow processing
	BOOST_CHECK ( ctrlCCounter == 2 );

	// deactivate mock
	mockCtrlHandler_.activate ( FALSE );
	// reactivate CtrlHandler
	pApps::CtrlHandler_.activate ( true );
}
