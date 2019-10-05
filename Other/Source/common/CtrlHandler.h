/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#pragma once

#include "common.h"

namespace p_apps {

	class CtrlHandlerManager {

		CtrlHandlerManager(const CtrlHandlerManager&) = delete;
		CtrlHandlerManager(const CtrlHandlerManager&&) = delete;
		CtrlHandlerManager& CtrlHandlerManager:: operator= (const CtrlHandlerManager&) = delete;
		CtrlHandlerManager&& CtrlHandlerManager:: operator= (const CtrlHandlerManager&&) = delete;

	    /**
		 * \brief Has to be static, because is referenced by static method.
		 *        If true pressing CtrlC should beep
		 */
		static bool debugBeep;

		//************************************
		// Field: Variable:  BOOL CtrlHandlerManager::Active
		// Brief: Mirror of CtrlHandlerManager state
		//************************************
		BOOL isActive;

		//************************************
		// Method:    vSetConsoleCtrlHandler
		// FullName:  pApps::CtrlHandlerManager::vSetConsoleCtrlHandler
		// Access:    virtual private
		// Returns:   BOOL WINAPI
		// Qualifier:
		// Parameter: PHANDLER_ROUTINE HandlerRoutine
		// Parameter: BOOL Add
		// Brief:     mockable, falls to SetConsoleCtrlHandler
		//************************************
		virtual BOOL WINAPI vSetConsoleCtrlHandler(PHANDLER_ROUTINE HandlerRoutine, BOOL Add) {
			return SetConsoleCtrlHandler(HandlerRoutine, Add);
		}

		//************************************
		// Method:    myCtrlHandler
		// FullName:  pApps::CtrlHandlerManager::myCtrlHandler
		// Access:    private static
		// Returns:   BOOL WINAPI
		// Qualifier:
		// Parameter: DWORD ctrlType
		// Brief:     Handler used by OS
		//************************************
		static BOOL WINAPI myCtrlHandler(DWORD ctrlType) {
			if (debugBeep) {
				Beep(750, 300);
			}
			return TRUE;
		}

	public:
		//************************************
		// Method:    CtrlHandlerManager
		// FullName:  pApps::CtrlHandlerManager::CtrlHandlerManager
		// Access:    public
		// Returns:
		// Qualifier: : Active_( FALSE )
		// Brief:     ctor - activate Ctrl handler
		//************************************
		CtrlHandlerManager() : isActive(FALSE) {
			debugBeep = false,
#if defined _DEBUG
				debugBeep = true,
#endif
				activate(TRUE);
		}

		//************************************
		// Method:    ~CtrlHandlerManager
		// FullName:  pApps::CtrlHandlerManager::~CtrlHandlerManager
		// Access:    public
		// Returns:
		// Qualifier:
		// Brief:     dtor - deactivate Ctrl handler
		//************************************
		virtual ~CtrlHandlerManager() {
			debugBeep = false;
			activate(FALSE);
		}

		//************************************
		// Method:    setDebugBeep
		// FullName:  pApps::CtrlHandlerManager::setDebugBeep
		// Access:    private static
		// Returns:   void
		// Qualifier:
		// Parameter: bool value
		// Brief:     activate/deactivate Ctrl beep
		//************************************
		static void setDebugBeep(const bool value = true) {
			debugBeep = value;
		}

		//************************************
		// Method:    activate
		// FullName:  pApps::CtrlHandlerManager::activate
		// Access:    private
		// Returns:   void
		// Qualifier:
		// Parameter: BOOL newState
		// Brief:     activate/deactivate Ctrl handler
		//************************************
		void activate(BOOL newState) {
			if (isActive != newState) {
				if (vSetConsoleCtrlHandler(myCtrlHandler, newState)) {
					isActive = newState;
				}
			}
		}
	};
}
