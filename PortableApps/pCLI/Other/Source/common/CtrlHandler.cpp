/*******************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * part of PortableApps Launcher project
 * http://sourceforge.net/projects/tccleportable/
 * Release 1.0.0
 *
 *******************************************************/

#include "stdafx.h"
#include "./CtrlHandler.h"

/******************************************************************************
 *
 * singleton pApps::CtrlHandler allows handling:
 *   SetConsoleCtrlHandler
 *   SetWindowLong
 *   in a common way
 *
 * singleton should be manipulated using pApps::CtrlHandler::getInstance().
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
 * addWinMessage
 *   adds windows message to be handled
 *   use sth like WM_CLOSE
 *   ! WARNING: in /CONSOLE apps console window belogs to csrss.exe
 *     http://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/65457519-bccd-49d2-bcb1-03d74a1898a4/c-and-windows-api-in-console-application-can-do-a-messsage-loop
 *     so it cann't be handled
 *
 * activate
 *   activates SetConsoleCtrlHandler
 *
 * setDebugBeep
 *   to help debugging
 *
 *****************************************************************************/
namespace pApps
{

	std::set<DWORD> CtrlHandler::_ctrlEvents;
	std::set<UINT>  CtrlHandler::_ctrlMessages;
	WNDPROC CtrlHandler::_prevWndProc = nullptr;
	bool CtrlHandler::_debugBeep = false;
	HWND CtrlHandler::_consoleWindow = nullptr;
	BOOL CtrlHandler::_ctrlHandlerActive = FALSE;

	/******************************************************************************
	 *
	 * CtrlHandler
	 *   To be installed for console preograms using SetConsoleCtrlHandler
	 *
	 *****************************************************************************/
	BOOL CtrlHandler::myCtrlHandler( DWORD ctrlType ) {
		if ( _ctrlEvents.end() != _ctrlEvents.find( ctrlType ) || _ctrlEvents.end() != _ctrlEvents.find( CTRL_ALL_EVENTS ) ) {
			if ( _debugBeep )
				Beep( 750, 300 );
			return TRUE;
		}
		return ( FALSE );
	}

	LRESULT CALLBACK CtrlHandler::myWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
		if ( _ctrlMessages.end() != _ctrlMessages.find( message ) ) {
			if ( _debugBeep )
				Beep( 2000, 300 );
			return 0;
		}
		return CallWindowProc( _prevWndProc, hWnd, message, wParam, lParam );
	}

	void CtrlHandler::setDebugBeep( const bool value ) {
		_debugBeep = value;
	}
	void CtrlHandler::addWinMessage( const UINT message ) {
		_ctrlMessages.insert( message );
	}

	void CtrlHandler::addEvent( const DWORD ctrlType ) {
		_ctrlEvents.insert( ctrlType );
	}

	void CtrlHandler::activate() {
		if ( !_ctrlHandlerActive && _ctrlEvents.size() ) {
			_ctrlHandlerActive = SetConsoleCtrlHandler( (PHANDLER_ROUTINE) myCtrlHandler, TRUE );
		}
		if ( !_prevWndProc && _consoleWindow && _ctrlMessages.size() ) {
			_prevWndProc = (WNDPROC) SetWindowLong( _consoleWindow, GWL_WNDPROC_6432, (LONG) myWndProc );
		}
	}

	void CtrlHandler::clear() {
		if ( _ctrlHandlerActive ) {
			SetConsoleCtrlHandler( (PHANDLER_ROUTINE) myCtrlHandler, FALSE );
			_ctrlHandlerActive = FALSE;
		}
		if ( _prevWndProc && _consoleWindow ) {
			(WNDPROC) SetWindowLong( _consoleWindow, GWL_WNDPROC_6432, (LONG) _prevWndProc );
			_prevWndProc = nullptr;
		}
		_ctrlEvents.clear();
		_ctrlMessages.clear();
		_debugBeep = false;
		_consoleWindow = GetConsoleWindow();
	}

	CtrlHandler::CtrlHandler() {
		clear();
	}

	CtrlHandler::~CtrlHandler() {
		clear();
	}

	CtrlHandler& CtrlHandler::getInstance() {
		static CtrlHandler _instance;
		return _instance;
	}

	void initCtrlHandler() {

		/*******************************************************
		*	Disable Ctrl-C
		*******************************************************/
		pApps::CtrlHandler::getInstance().addEvent( pApps::CtrlHandler::CTRL_ALL_EVENTS );
#ifndef _CONSOLE
		pApps::CtrlHandler::getInstance().addWinMessage( WM_CLOSE );
#endif
		pApps::CtrlHandler::getInstance().activate();		
	};

	static int _initCtrlHandler = ( initCtrlHandler(), 0 );
}
