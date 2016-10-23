/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#pragma once

#include "common.h"
#include <windows.h>

namespace pApps {

	class CtrlHandlerManager {

			//************************************
			// Variable:  bool CtrlHandlerManager::DebugBeep_
			// Brief:     Has to be static, because is referenced by static method.
			//            If true pressing CtrlC should beep
			//************************************
			static bool DebugBeep_;

			//************************************
			// Field: Variable:  BOOL CtrlHandlerManager::Active_
			// Brief: Mirror of CtrlHandlerManager state
			//************************************
			BOOL Active_;

			//************************************
			// Method:    vSetConsoleCtrlHandler
			// FullName:  pApps::CtrlHandlerManager::vSetConsoleCtrlHandler
			// Access:    virtual private
			// Returns:   BOOL WINAPI
			// Qualifier:
			// Parameter: _In_opt_ PHANDLER_ROUTINE HandlerRoutine
			// Parameter: _In_ BOOL Add
			// Brief:     mockable, falls to SetConsoleCtrlHandler
			//************************************
			virtual BOOL WINAPI vSetConsoleCtrlHandler ( _In_opt_ PHANDLER_ROUTINE HandlerRoutine, _In_ BOOL Add ) {
				return SetConsoleCtrlHandler ( HandlerRoutine, Add );
			}

			//************************************
			// Method:    myCtrlHandler
			// FullName:  pApps::CtrlHandlerManager::myCtrlHandler
			// Access:    private static
			// Returns:   BOOL WINAPI
			// Qualifier:
			// Parameter: _In_ DWORD ctrlType
			// Brief:     Handler used by OS
			//************************************
			static BOOL WINAPI myCtrlHandler ( _In_ DWORD ctrlType ) {
				if ( DebugBeep_ ) {
					Beep ( 750, 300 );
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
			CtrlHandlerManager() : Active_ ( FALSE ) {
				DebugBeep_ = false,
				activate ( TRUE );
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
				DebugBeep_ = false;
				activate ( FALSE );
			}

			//************************************
			// Method:    setDebugBeep
			// FullName:  pApps::CtrlHandlerManager::setDebugBeep
			// Access:    private static
			// Returns:   void
			// Qualifier:
			// Parameter: _In_ bool value
			// Brief:     activate/deactivate Ctrl beep
			//************************************
			static void setDebugBeep ( _In_ bool value = true ) {
				DebugBeep_ = value;
			}

			//************************************
			// Method:    activate
			// FullName:  pApps::CtrlHandlerManager::activate
			// Access:    private
			// Returns:   void
			// Qualifier:
			// Parameter: _In_ BOOL newState
			// Brief:     activate/deactivate Ctrl handler
			//************************************
			void activate ( _In_ BOOL newState ) {
				if ( Active_ != newState ) {
					if ( vSetConsoleCtrlHandler ( myCtrlHandler, newState ) ) {
						Active_ = newState;
					}
				}
			}
	};
}
