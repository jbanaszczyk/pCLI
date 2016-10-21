/*****************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * part of PortableApps Launcher project
 * http://sourceforge.net/projects/tccleportable/
 * Release 1.0.0
 *
 *****************************************************************************/

#ifndef _8ECE158B_EBCD_484E_9712_3988D0C2C5DE_INCLUDED
#define _8ECE158B_EBCD_484E_9712_3988D0C2C5DE_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif

/******************************************************************************
 *
 * singleton pApps::CtrlHandler allows handling:
 *   SetConsoleCtrlHandler
 *   SetWindowLong
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
#include "./common.h"

#include <windows.h>
#include <tchar.h>
#include <set>

namespace pApps
{
	class CtrlHandler : private boost::noncopyable 	{
	private:
		static std::set<DWORD> _ctrlEvents;
		static std::set<UINT>  _ctrlMessages;

		static LRESULT CALLBACK myWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		static BOOL myCtrlHandler( DWORD ctrlType );

		static HWND _consoleWindow;
		static BOOL _ctrlHandlerActive;
		static WNDPROC _prevWndProc;
		static bool _debugBeep;

	private:
		CtrlHandler();
		~CtrlHandler();
	public:
		static CtrlHandler& getInstance();
		static const DWORD CTRL_ALL_EVENTS = -1;
		void clear();
		void activate();
		void setDebugBeep( const bool value = true );
		void addEvent( const DWORD ctrlType );
		void addWinMessage( const UINT message );
	};
}

#endif  // _8ECE158B_EBCD_484E_9712_3988D0C2C5DE_INCLUDED
