// OrderedSetTests.cpp ---
//
// Filename: OrderedSetTests.cpp
// Author: Abhishek Udupa
// Created: Wed Mar 25 15:17:38 2015 (-0400)
//
//
// Copyright (c) 2015, Abhishek Udupa, University of Pennsylvania
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

#include "../../projects/kinara-common/src/containers/OrderedSet.hpp"

#include <utility>
#include <random>
#include <algorithm>
#include <set>

#include "RCClass.hpp"

#include "../../thirdparty/gtest/include/gtest/gtest.h"

using kinara::u32;
using kinara::u64;
using kinara::i32;
using kinara::i64;

const u64 max_insertion_value = (1 << 6);
const u64 max_test_iterations = (1 << 4);

using kinara::containers::u64OrderedSet;

static inline bool test_equal(const u64OrderedSet& set1, std::set<u64>& set2)
{
    if (set1.size() != set2.size()) {
        return false;
    }

    auto it1 = set1.begin();
    auto it2 = set2.begin();

    auto end1 = set1.end();
    auto end2 = set2.end();

    while (it1 != end1 && it2 != end2) {
        if (*it1 != *it2) {
            return false;
        }
        ++it1;
        ++it2;
    }
    return true;
}

TEST(OrderedSetTest, Constructor)
{
    typedef u64OrderedSet SetType;

    SetType set1;

    EXPECT_EQ(0ul, set1.size());

    SetType set2(set1);
    EXPECT_EQ(0ull, set1.size());
    EXPECT_EQ(0ull, set2.size());

    EXPECT_TRUE(set1.begin() == set1.end());
    EXPECT_TRUE(set2.begin() == set2.end());

    SetType set3({1ull, 2ull, 3ull, 4ull, 5ull});
    EXPECT_EQ(5ul, set3.size());

    auto it3 = set3.begin();
    for (u64 i = 0; i < 5; ++i) {
        EXPECT_EQ(i+1, *it3);
        ++it3;
    }
    EXPECT_EQ(set3.end(), it3);

    SetType set4(std::move(set3));
    EXPECT_EQ(5ul, set4.size());
    EXPECT_EQ(0ul, set3.size());

    auto it4 = set4.begin();
    for (u64 i = 0; i < 5; ++i) {
        EXPECT_EQ(i+1, *it4);
        ++it4;
    }
    EXPECT_EQ(set4.end(), it4);
}

TEST(OrderedSetTest, Assignment)
{
    typedef u64OrderedSet SetType;
    SetType set1;

    set1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    EXPECT_EQ(10ull, set1.size());
    auto it1 = set1.begin();
    for (u64 i = 0; i < 10; ++i) {
        EXPECT_EQ(i+1, *it1);
        ++it1;
    }
    EXPECT_TRUE(it1 == set1.end());

    SetType set2 = set1;
    EXPECT_EQ(10ull, set2.size());

    it1 = set1.begin();
    auto it2 = set2.begin();
    for (u64 i = 0; i < 10; ++i) {
        EXPECT_EQ(i+1, *it1);
        EXPECT_EQ(i+1, *it2);
        ++it1;
        ++it2;
    }
    EXPECT_TRUE(it1 == set1.end());
    EXPECT_TRUE(it2 == set2.end());

    SetType set3 = std::move(set2);
    EXPECT_EQ(0ull, set2.size());
    EXPECT_EQ(10ull, set3.size());

    auto it3 = set3.begin();
    for (u64 i = 0; i < 10; ++i) {
        EXPECT_EQ(i+1, *it3);
        ++it3;
    }
    EXPECT_TRUE(it3 == set3.end());
}

TEST(OrderedSetTest, Functional)
{
    typedef u64OrderedSet SetType;


    SetType kinara_set;
    std::set<u64> std_set;

    std::default_random_engine generator;
    std::uniform_int_distribution<u64> distribution(0, 1);

    for (u64 i = 0; i < max_test_iterations; ++i) {
        std_set.clear();
        kinara_set.clear();

        for (u64 j = 0; j < max_insertion_value; ++j) {
            auto flip = (distribution(generator) == 1);
            if (flip) {
                std_set.insert(j);
                kinara_set.insert(j);

                EXPECT_EQ(std_set.size(), kinara_set.size());
            }
        }

        EXPECT_TRUE(test_equal(kinara_set, std_set));

        // erase some random elements
        for (u64 j = 0; j < max_insertion_value; ++j) {
            auto flip = (distribution(generator) == 1);
            if (flip) {
                std_set.erase(j);
                kinara_set.erase(j);

                EXPECT_EQ(std_set.size(), kinara_set.size());
            }
        }

        EXPECT_TRUE(test_equal(kinara_set, std_set));

        // repeat the above two steps
        for (u64 j = 0; j < max_insertion_value; ++j) {
            auto flip = (distribution(generator) == 1);
            if (flip) {
                std_set.insert(j);
                kinara_set.insert(j);

                EXPECT_EQ(std_set.size(), kinara_set.size());
            }
        }

        EXPECT_TRUE(test_equal(kinara_set, std_set));

        // erase some random elements
        for (u64 j = 0; j < max_insertion_value; ++j) {
            auto flip = (distribution(generator) == 1);
            if (flip) {
                std_set.erase(j);
                kinara_set.erase(j);

                EXPECT_EQ(std_set.size(), kinara_set.size());
            }
        }

        EXPECT_TRUE(test_equal(kinara_set, std_set));
    }
}

//
// OrderedSetTests.cpp ends here
