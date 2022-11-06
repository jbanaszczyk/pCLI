// *************************************************************
// * Copyright 2011 jacek.banaszczyk@gmail.com                 *
// * Part of pCli project: https://github.com/jbanaszczyk/pCli *
// *************************************************************

#pragma once

#include "common.h"

namespace p_apps {
	class CtrlHandlerManager {

		/**
		 * \brief If true pressing CtrlC should beep
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
		virtual BOOL WINAPI vSetConsoleCtrlHandler(const PHANDLER_ROUTINE handlerRoutine, const BOOL add) {
			// NOLINT(misc-misplaced-const)
			return SetConsoleCtrlHandler(handlerRoutine, add);
		}

		/**
		 * \brief Handler used by OS
		 * \return always TRUE
		 */
		static BOOL WINAPI myCtrlHandler(DWORD /*ctrlType*/) {
			if (debugBeep) {
				Beep(500, 200);
			}
			return TRUE;
		}

	public:
		CtrlHandlerManager(const CtrlHandlerManager&) = delete;
		CtrlHandlerManager(const CtrlHandlerManager&&) = delete;
		CtrlHandlerManager& operator=(const CtrlHandlerManager&) = delete;
		CtrlHandlerManager&& operator=(const CtrlHandlerManager&&) = delete;
		/**
		 * \brief ctor - activate Ctrl handler
		 */
		CtrlHandlerManager() : isActive(FALSE) {
			debugBeep = C_DEBUG;
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
		 * \brief activate/deactivate Ctrl handler
		 * \param newState desired value of isActive
		 */
		void activate(const BOOL newState) {
			if (isActive != newState) {
				if (vSetConsoleCtrlHandler(myCtrlHandler, newState)) {
					isActive = newState;
				}
			}
		}
	};
}
