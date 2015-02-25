// StringTests.cpp ---
// Filename: StringTests.cpp
// Author: Abhishek Udupa
// Created: Mon Feb 23 22:14:43 2015 (-0500)
//
// Copyright (c) 2013, Abhishek Udupa, University of Pennsylvania
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by The University of Pennsylvania
// 4. Neither the name of the University of Pennsylvania nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//

// Code:

#include "../../projects/kinara-common/src/containers/String.hpp"
#include <string>
#include <random>
#include <cstdlib>
#include "../../thirdparty/gtest/include/gtest/gtest.h"

using kinara::containers::String;
using kinara::u32;
using kinara::u64;

// Test short strings
TEST(String, ShortStrings) {
    String empty_string;
    EXPECT_EQ(empty_string.length(), (u64)0);

    // short string test
    empty_string += "hello";
    EXPECT_EQ(empty_string, "hello");

    String world_string("world");
    String hello_world_string = empty_string + " " + world_string;
    EXPECT_EQ(hello_world_string, "hello world");
    EXPECT_EQ(hello_world_string.length(), (u64)11);
}

TEST(String, LongStrings) {
    String short_string1("This is a short string");
    String short_string2("This is a short string as well");
    EXPECT_EQ(short_string1.length(), (u64)22);
    EXPECT_EQ(short_string2.length(), (u64)30);
    String long_string = short_string1 + short_string2;
    EXPECT_EQ(long_string.length(), (u64)52);
    String long_string2 = (short_string1 += short_string2);
    EXPECT_EQ((void*)(long_string2.c_str()), (void*)(long_string.c_str()));

    String another_long_string(std::move(long_string));
    EXPECT_EQ((void*)(long_string2.c_str()), (void*)(another_long_string.c_str()));

    EXPECT_EQ(another_long_string.find("This"), (u64)0);
    EXPECT_EQ(another_long_string.find("short"), (u64)10);

    EXPECT_EQ(another_long_string.to_lower(), "this is a short stringthis is a short string as well");
}

// Fuzz test with random strings
TEST(String, RandomStrings) {
    std::random_device rd;
    const int max_string_len = 256;
    for (int i = 0; i < 1024; ++i) {
        auto len1 = rd() % max_string_len;
        auto len2 = rd() % max_string_len;
        auto len3 = rd() % max_string_len;

        String str1, str2, str3;
        std::string stdstr1, stdstr2, stdstr3;

        for (size_t j = 0; j < len1; ++j) {
            char c = ((rd() % 2 == 0 ? 'a' : 'A') + (rd() % 26));
            str1 += c;
            stdstr1 += c;
        }
        EXPECT_EQ(str1, stdstr1.c_str());

        for (size_t j = 0; j < len2; ++j) {
            char c = ((rd() % 2 == 0 ? 'a' : 'A') + (rd() % 26));
            str2 += c;
            stdstr2 += c;
        }
        EXPECT_EQ(str2, stdstr2.c_str());

        for (size_t j = 0; j < len3; ++j) {
            char c = ((rd() % 2 == 0 ? 'a' : 'A') + (rd() % 26));
            str3 += c;
            stdstr3 += c;
        }
        EXPECT_EQ(str3, stdstr3.c_str());

        // some concat tests
        str1 += str2;
        stdstr1 += stdstr2;
        EXPECT_EQ(str1, stdstr1.c_str());

        EXPECT_EQ(str1 + str3, (stdstr1 + stdstr3).c_str());
    }
}

// Perf tests
TEST(String, PerfRandomStrings) {
    std::random_device rd;
    const int max_string_len = 256;
    for (int i = 0; i < 1024; ++i) {
        auto len1 = rd() % max_string_len;
        auto len2 = rd() % max_string_len;
        auto len3 = rd() % max_string_len;

        String str1, str2, str3;

        for (size_t j = 0; j < len1; ++j) {
            char c = ((rd() % 2 == 0 ? 'a' : 'A') + (rd() % 26));
            str1 += c;
        }

        for (size_t j = 0; j < len2; ++j) {
            char c = ((rd() % 2 == 0 ? 'a' : 'A') + (rd() % 26));
            str2 += c;
        }

        for (size_t j = 0; j < len3; ++j) {
            char c = ((rd() % 2 == 0 ? 'a' : 'A') + (rd() % 26));
            str3 += c;
        }

        // some concat tests
        str1 += str2;
        EXPECT_NE(nullptr, str1.c_str());
        EXPECT_NE(nullptr, (str1 + str3).c_str());
    }
}

TEST(String, PerfRandomStringsStd) {
    std::random_device rd;
    const int max_string_len = 256;
    for (int i = 0; i < 1024; ++i) {
        auto len1 = rd() % max_string_len;
        auto len2 = rd() % max_string_len;
        auto len3 = rd() % max_string_len;

        std::string str1, str2, str3;

        for (size_t j = 0; j < len1; ++j) {
            char c = ((rd() % 2 == 0 ? 'a' : 'A') + (rd() % 26));
            str1 += c;
        }

        for (size_t j = 0; j < len2; ++j) {
            char c = ((rd() % 2 == 0 ? 'a' : 'A') + (rd() % 26));
            str2 += c;
        }

        for (size_t j = 0; j < len3; ++j) {
            char c = ((rd() % 2 == 0 ? 'a' : 'A') + (rd() % 26));
            str3 += c;
        }

        // some concat tests
        str1 += str2;
        EXPECT_NE(nullptr, str1.c_str());
        EXPECT_NE(nullptr, (str1 + str3).c_str());
    }
}

// fixture for a random string
class PerfTestFixture : public ::testing::Test
{
protected:
    static constexpr int sc_test_size = (1 << 16) - 1;
    static constexpr int sc_num_strings = (1 << 8);

    char* m_buffer;

    std::string std_strings[sc_num_strings];
    String kinara_strings[sc_num_strings];

    virtual void SetUp()
    {
        std::mt19937 rd;
        m_buffer = (char*)calloc(1, sc_test_size + 1);
        for (int i = 0; i < sc_test_size; ++i) {
            m_buffer[i] = ((rd() % 2 == 0 ? 'a' : 'A') + (rd() % 26));
        }

        for (int i = 0; i < sc_num_strings; ++i) {
            std_strings[i] = std::string(m_buffer);
            kinara_strings[i] = String(m_buffer);
        }
    }

    virtual void TearDown()
    {
        free(m_buffer);
    }
};

// Perf tests on comparing pairwise equality of std strings
TEST_F(PerfTestFixture, StdString) {
    for (int i = 0; i < sc_num_strings; ++i) {
        for (int j = 0; j < sc_num_strings; ++j) {
            EXPECT_EQ(std_strings[i], std_strings[j]);
        }
    }
}

// Perf tests on comparing pairwise equality of kinara strings
TEST_F(PerfTestFixture, KinaraString) {
    for (int i = 0; i < sc_num_strings; ++i) {
        for (int j = 0; j < sc_num_strings; ++j) {
            EXPECT_EQ(kinara_strings[i], kinara_strings[j]);
        }
    }
}

//
// StringTests.cpp ends here
