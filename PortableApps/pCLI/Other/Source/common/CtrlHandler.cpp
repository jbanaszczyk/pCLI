/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#include "stdafx.h"
#include "./CtrlHandler.h"

namespace pApps {
	std::set<DWORD> CtrlHandler::ctrlEvents_;
	bool CtrlHandler::debugBeep_ = false;
	HWND CtrlHandler::consoleWindow_ = nullptr;
	BOOL CtrlHandler::ctrlHandlerActive_ = FALSE;

	void initCtrlHandler() {
		/*******************************************************
		*   Disable Ctrl-C
		*******************************************************/
		pApps::CtrlHandler::getInstance().addEvent( pApps::CtrlHandler::CTRL_ALL_EVENTS );
		pApps::CtrlHandler::getInstance().activate();
	};

	static int _initCtrlHandler = ( initCtrlHandler(), 0 );
}
