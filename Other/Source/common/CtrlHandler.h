/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/

#pragma once

#include "common.h"

namespace p_apps
{
    class CtrlHandlerManager
    {
        CtrlHandlerManager(const CtrlHandlerManager&) = delete;
        CtrlHandlerManager(const CtrlHandlerManager&&) = delete;
        CtrlHandlerManager& CtrlHandlerManager::operator=(const CtrlHandlerManager&) = delete;
        CtrlHandlerManager&& CtrlHandlerManager::operator=(const CtrlHandlerManager&&) = delete;

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
         * \brief mockable, falls to SetConsoleCtrlHandler
         * \param handlerRoutine required by SetConsoleCtrlHandler
         * \param add required by SetConsoleCtrlHandler
         * \return passes return value from SetConsoleCtrlHandler
         */
        virtual BOOL WINAPI vSetConsoleCtrlHandler(const PHANDLER_ROUTINE handlerRoutine, const BOOL add)
        {
            return SetConsoleCtrlHandler(handlerRoutine, add);
        }

        /**
         * \brief Handler used by OS
         * \return always TRUE
         */
        static BOOL WINAPI myCtrlHandler(DWORD /*ctrlType*/)
        {
            if (debugBeep)
            {
                Beep(750, 300);
            }
            return TRUE;
        }

    public:
        /**
         * \brief ctor - activate Ctrl handler
         */
        CtrlHandlerManager() : isActive(FALSE)
        {
            debugBeep = false,
#if defined _DEBUG
                debugBeep = true,
#endif
                activate(TRUE);
        }

        /**
         * \brief dtor - deactivate Ctrl handler
         */
        virtual ~CtrlHandlerManager()
        {
            debugBeep = false;
            activate(FALSE);
        }

        /**
         * \brief activate/deactivate Ctrl beep
         * \param value desired value of debugBeep
         */
        static void setDebugBeep(const bool value = true)
        {
            debugBeep = value;
        }

        /**
         * \brief activate/deactivate Ctrl handler
         * \param newState desired value of isActive
         */
        void activate(const BOOL newState)
        {
            if (isActive != newState)
            {
                if (vSetConsoleCtrlHandler(myCtrlHandler, newState))
                {
                    isActive = newState;
                }
            }
        }
    };
}
