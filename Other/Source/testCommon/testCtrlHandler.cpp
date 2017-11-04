/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

/******************************************************************************

  * W A R N I N G *

  Test presses Ctrl-C and Ctrl-Break keys.
  It can be handled as exception under debugger.
  Simply: disable this kind of exception

*****************************************************************************/

#include "stdafx.h"
#include "../common/CtrlHandler.cpp"

static volatile unsigned int ctrlCCounter = 0;
const DWORD sleepTime = 3;

static BOOL WINAPI mockCtrlHandler(DWORD ctrlType) {
	++ctrlCCounter;
	return TRUE;
}

class mockCtrlHandlerManager : public p_apps::CtrlHandlerManager {
public:
	BOOL WINAPI vSetConsoleCtrlHandler(PHANDLER_ROUTINE HandlerRoutine, BOOL Add) override {
		if (Add < 2) {
			return SetConsoleCtrlHandler(HandlerRoutine, Add);
		}

		Add -= 2;
		ctrlCCounter = 0;
		return SetConsoleCtrlHandler(mockCtrlHandler, Add);
	}
};

class CtrlHandlers : public ::testing::Test {
protected:
	virtual void SetUp() { 
		// A bit difficult

		ctrlCCounter = 0;

		// shut down installed CtrlHandler
		p_apps::ctrlCHandler.activate(false);

		// mocked ctor invokes activate again
		// so deactivate again
		mockCtrlHandler.activate(FALSE);

		// install mockCtrlHandler as CtrlHandler
		mockCtrlHandler.activate(TRUE + 2);
	}

	virtual void TearDown() {
		// deactivate mock
		mockCtrlHandler.activate(FALSE);
		// reactivate CtrlHandler
		p_apps::ctrlCHandler.activate(true);
	}

	mockCtrlHandlerManager mockCtrlHandler;
};

TEST_F(CtrlHandlers, CtrlCHandler) {
	EXPECT_EQ(0, ctrlCCounter);
	GenerateConsoleCtrlEvent(0, 0); // generate CtrlC
	Sleep(sleepTime);               // yield to allow processing
	ASSERT_EQ(1, ctrlCCounter);
}

TEST_F(CtrlHandlers, CtrlBreakHandler) {
	EXPECT_EQ(0, ctrlCCounter);
	GenerateConsoleCtrlEvent(1, 0); // generate CtrlBreak
	Sleep(sleepTime);               // yield to allow processing
	ASSERT_EQ(1, ctrlCCounter);
}
