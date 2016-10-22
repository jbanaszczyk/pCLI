/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#pragma once

/******************************************************************************
*
* Procere used by SetConsoleCtrlHandler( myCtrlHandler, ... has to be static,
* so there is a lot of static things
*
* singleton pApps::CtrlHandler allows handling:
*   SetConsoleCtrlHandler
*   in a common way
*
* singleton should be manipulated using pApps::CtrlHandler::getInstance().
*
* There is defined static object of class currentCtrlHandler - it initializes CtrlHandler
*
* Methods:
*
* clear
*   dactivates all handlers, clears all data
*
* addEvent
*   adds console event to be handled (disabled) by handler procedure
*   use any of CTRL_C_EVENT, CTRL_BREAK_EVENT, and so on
*   or pApps::CtrlHandler::CTRL_ALL_EVENTS to deactivate all
*
* activate
*   activates SetConsoleCtrlHandler
*
* setDebugBeep
*   to help debugging
*
*****************************************************************************/
#include "./common.h"

#include <windows.h>
#include <tchar.h>
#include <set>

namespace pApps {
	class CtrlHandler : private boost::noncopyable {
	private:
		CtrlHandler() {
			clear();
		}

		~CtrlHandler() {
			clear();
		}

	public:
		static CtrlHandler& getInstance() {
			static CtrlHandler _instance;
			return _instance;
		}

		static const DWORD CTRL_ALL_EVENTS = -1;

		void clear() {
			if ( ctrlHandlerActive_ ) {
				SetConsoleCtrlHandler( myCtrlHandler, FALSE );
				ctrlHandlerActive_ = FALSE;
			}
			ctrlEvents_.clear();
			debugBeep_ = false;
			consoleWindow_ = GetConsoleWindow();
		}

		void activate() {
			if ( !ctrlHandlerActive_ && ctrlEvents_.size() ) {
				ctrlHandlerActive_ = SetConsoleCtrlHandler( myCtrlHandler, TRUE );
			}
		}

		void setDebugBeep( const bool value = true ) {
			debugBeep_ = value;
		}

		void addEvent( const DWORD ctrlType ) {
			ctrlEvents_.insert( ctrlType );
		}

	private:
		static std::set<DWORD> ctrlEvents_;
		static HWND consoleWindow_;
		static BOOL ctrlHandlerActive_;
		static bool debugBeep_;

		static BOOL WINAPI myCtrlHandler( _In_ DWORD ctrlType ) {
			if ( ctrlEvents_.end() != ctrlEvents_.find( ctrlType ) || ctrlEvents_.end() != ctrlEvents_.find( CTRL_ALL_EVENTS ) ) {
				if ( debugBeep_ ) {
					Beep( 750, 300 );
				}
				return TRUE;
			}
			return ( FALSE );
		}
	};
}
