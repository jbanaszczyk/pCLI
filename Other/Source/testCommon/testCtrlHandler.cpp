/******************************************************************************
*
* Copyright 2011 jacek.banaszczyk@gmail.com
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

static auto WINAPI mockCtrlHandler(DWORD /*ctrlType*/) -> BOOL {
	++ctrlCCounter;
	return TRUE;
}

class MockCtrlHandlerManager: public p_apps::CtrlHandlerManager {
	public:
		auto WINAPI vSetConsoleCtrlHandler(const PHANDLER_ROUTINE handlerRoutine, BOOL add) -> BOOL override {
			if (add < 2){
				return SetConsoleCtrlHandler(handlerRoutine, add);
			}

			add -= 2;
			ctrlCCounter = 0;
			return SetConsoleCtrlHandler(mockCtrlHandler, add);
		}
};

class CtrlHandlers: public testing::Test {
	protected:
		auto SetUp() -> void override {
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

		auto TearDown() -> void override {
			// deactivate mock
			mockCtrlHandler.activate(FALSE);
			// reactivate CtrlHandler
			p_apps::ctrlCHandler.activate(true);
		}

		MockCtrlHandlerManager mockCtrlHandler;
};

TEST_F(CtrlHandlers, CtrlCHandler) {
	EXPECT_EQ(0, ctrlCCounter);
	GenerateConsoleCtrlEvent(0, 0); // generate CtrlC
	Sleep(sleepTime); // yield to allow processing
	ASSERT_EQ(1, ctrlCCounter);
}

TEST_F(CtrlHandlers, CtrlBreakHandler) {
	EXPECT_EQ(0, ctrlCCounter);
	GenerateConsoleCtrlEvent(1, 0); // generate CtrlBreak
	Sleep(sleepTime); // yield to allow processing
	ASSERT_EQ(1, ctrlCCounter);
}
