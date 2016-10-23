/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#include "stdafx.h"
#include "CtrlHandler.h"

namespace pApps {

	//************************************
	// Variable:  bool CtrlHandlerManager::DebugBeep_
	// Brief:     Has to be static, because is referenced by static method
	//************************************
	bool CtrlHandlerManager::DebugBeep_ = false;

	//************************************
	// Variable:  CtrlHandlerManager::DebugBeep_
	// Brief:     Has to be static, because is referenced by static method
	//************************************
	CtrlHandlerManager CtrlHandler_;
}
