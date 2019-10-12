/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

/******************************************************************************
 *
 * unique pointers and deletors
 *
 *****************************************************************************/
#include "./common.h"

namespace p_apps {

    namespace details {
        struct HandleDeleter {
            void operator()(const HANDLE handle) const {
                if (handle != INVALID_HANDLE_VALUE)
                    CloseHandle(handle);
            }
        };

    }

    using HandleUniquePtr = std::unique_ptr<void, details::HandleDeleter>;

    inline HandleUniquePtr handleUniquePtr(const HANDLE handle) {
        if (handle == INVALID_HANDLE_VALUE || handle == nullptr) {
            return nullptr;
        }
        return HandleUniquePtr(handle);
    }
}
