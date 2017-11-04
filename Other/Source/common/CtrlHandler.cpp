/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#include "stdafx.h"
#include "CtrlHandler.h"

namespace p_apps {

	//************************************
	// Variable:  bool CtrlHandlerManager::DebugBeep_
	// Brief:     Has to be static, because is referenced by static method
	//************************************
	bool CtrlHandlerManager::debugBeep = false;

	//************************************
	// Variable:  CtrlHandlerManager::DebugBeep_
	// Brief:     Has to be static, because is referenced by static method
	//************************************
	CtrlHandlerManager ctrlCHandler;
}
