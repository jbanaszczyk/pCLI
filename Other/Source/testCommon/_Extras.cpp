/******************************************************************************
*
* Copyright 2013 Jacek.Banaszczyk@gmail.com
* Part of pCli project: https://github.com/jbanaszczyk/pCli
*
*****************************************************************************/


#include "stdafx.h"
/*
#include <string>
#include <utility>

void* Resource_Reserve (size_t size);
void Resource_Free (void* resource);

namespace Decoder {
    std::pair<std::string, int> decode (const std::string& msg);
}

namespace Decoder {

    namespace details {
        int decode (const char* msg, size_t length, char* output) {
            if (length == 0 || msg[0] != '#')
                return 1;

            output[0] = '!';

            for (size_t i = 1; i < length; ++i) {
                output[i] = msg[i] + 1;
            }

            return 0;
        }
    }

    std::pair<std::string, int> decode (const std::string& msg) {
        size_t size = msg.size () + 1;
        union {
            void* void_ptr;
            char* char_ptr;
        } p = {Resource_Reserve (size)};

        if (!p.void_ptr)
            return std::make_pair (std::string (), -1);

        int errorCode = 0;

        if ((errorCode = details::decode (msg.data (), size, p.char_ptr)) != 0) {
            Resource_Free (p.void_ptr);
            return std::make_pair (std::string (), errorCode);
        }

        std::string result (p.char_ptr, size - 1);

        Resource_Free (p.void_ptr);

        return {result, errorCode};
    }
}

///////////////////////

#include <gmock/gmock.h>


class ResourceSystemMock {
public:
    ResourceSystemMock (void);
    MOCK_CONST_METHOD1 (Resource_Reserve, void*(size_t));
    MOCK_CONST_METHOD1 (Resource_Free, void (void*));
    ~ResourceSystemMock (void);
};

#include <functional>

static std::function<void* (size_t)> _reserve;
static std::function<void (void*)> _free;

ResourceSystemMock::ResourceSystemMock () {
    assert (!_reserve && !_free);
    _reserve = [this](size_t s) { return Resource_Reserve (s); };
    _free = [this](void * p) { Resource_Free (p); };
}

ResourceSystemMock::~ResourceSystemMock () {
    _reserve = {};
    _free = {};
}

void* Resource_Reserve (size_t size) {
    return _reserve (size);
}

void Resource_Free (void* resource) {
    _free (resource);
}

/////////////////////

using namespace ::testing;

struct DecoderTestsFixture : Test {
    ResourceSystemMock rs;
    using ValueWithError = std::pair < std::string, int >;
};

TEST_F (DecoderTestsFixture, failed_reserve_resources) {
    EXPECT_CALL (rs, Resource_Reserve (_)).WillRepeatedly (Return (nullptr));

    ASSERT_THAT (Decoder::decode ("message which needs too much resources"), Eq (ValueWithError ("", -1)));
}

TEST_F (DecoderTestsFixture, unsuccessful_decode) {
    const char msg[] = "wrong message";
    char out[sizeof (msg)];

    EXPECT_CALL (rs, Resource_Reserve (sizeof (msg))).WillOnce (Return (out));
    EXPECT_CALL (rs, Resource_Free (out));

    ASSERT_THAT (Decoder::decode (msg), Eq (ValueWithError ("", 1)));
}

TEST_F (DecoderTestsFixture, successful_decode) {
    const char msg[] = "#abc";
    char out[sizeof (msg)];

    EXPECT_CALL (rs, Resource_Reserve (sizeof (msg))).WillOnce (Return (out));
    EXPECT_CALL (rs, Resource_Free (out));

    ASSERT_THAT (Decoder::decode (msg), Eq (ValueWithError ("!bcd", 0)));
}
*/