/******************************************************************************
*
* Copyright 2011-2021 jacek.banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#pragma once

#include "common.h"

namespace p_apps {
	class CtrlHandlerManager {

		/**
		 * \brief Has to be static, because is referenced by static method.
		 *        If true pressing CtrlC should beep
		 */
		static bool debugBeep;

		/**
		 * \brief Mirror of CtrlHandlerManager state
		 */
		BOOL isActive;

		/**
		 * \brief mock-able, falls to SetConsoleCtrlHandler
		 * \param handlerRoutine required by SetConsoleCtrlHandler
		 * \param add required by SetConsoleCtrlHandler
		 * \return passes return value from SetConsoleCtrlHandler
		 */
		virtual auto WINAPI vSetConsoleCtrlHandler(const PHANDLER_ROUTINE handlerRoutine, const BOOL add) -> BOOL { // NOLINT(misc-misplaced-const)
			return SetConsoleCtrlHandler(handlerRoutine, add);
		}

		/**
		 * \brief Handler used by OS
		 * \return always TRUE
		 */
		static auto WINAPI myCtrlHandler(DWORD /*ctrlType*/) -> BOOL {
			if (debugBeep){
				Beep(750, 300);
			}
			return TRUE;
		}

		public:
			CtrlHandlerManager(const CtrlHandlerManager&) = delete;
			CtrlHandlerManager(const CtrlHandlerManager&&) = delete;
			auto operator=(const CtrlHandlerManager&) -> CtrlHandlerManager& = delete;
			auto operator=(const CtrlHandlerManager&&) -> CtrlHandlerManager&& = delete;
			/**
			 * \brief ctor - activate Ctrl handler
			 */
			CtrlHandlerManager()
				: isActive(FALSE) {
#if defined _DEBUG
				debugBeep = true,
#else
				debugBeep = false,
#endif
					activate(TRUE);
			}

			/**
			 * \brief destructor - deactivate Ctrl handler
			 */
			virtual ~CtrlHandlerManager() {
				debugBeep = false;
				activate(FALSE);
			}

			/**
			 * \brief activate/deactivate Ctrl beep
			 * \param value desired value of debugBeep
			 */
			static auto setDebugBeep(const bool value = true) -> void {
				debugBeep = value;
			}

			/**
			 * \brief activate/deactivate Ctrl handler
			 * \param newState desired value of isActive
			 */
			auto activate(const BOOL newState) -> void {
				if (isActive != newState){
					if (vSetConsoleCtrlHandler(myCtrlHandler, newState)){
						isActive = newState;
					}
				}
			}
	};
}
