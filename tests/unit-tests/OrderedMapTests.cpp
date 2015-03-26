// OrderedMapTests.cpp ---
//
// Filename: OrderedMapTests.cpp
// Author: Abhishek Udupa
// Created: Thu Mar 26 14:05:21 2015 (-0400)
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

#include "../../projects/kinara-common/src/containers/OrderedMap.hpp"

#include <utility>
#include <random>
#include <algorithm>
#include <map>

#include "RCClass.hpp"

#include "../../thirdparty/gtest/include/gtest/gtest.h"

using kinara::u32;
using kinara::u64;
using kinara::i32;
using kinara::i64;

const u64 max_insertion_value = (1 << 16);
const u64 max_test_iterations = (1 << 4);

using kinara::containers::OrderedMap;

typedef OrderedMap<u64, u64> u64u64OrderedMap;

static inline bool test_equal(const u64u64OrderedMap& kinara_map, const std::map<u64, u64>& std_map)
{
    if (kinara_map.size() != std_map.size()) {
        return false;
    }

    auto it1 = kinara_map.begin();
    auto it2 = std_map.begin();
    auto end1 = kinara_map.end();
    auto end2 = std_map.end();

    while (it1 != end1 && it2 != end2) {
        if (it1->first != it2->first || it1->second != it2->second) {
            return false;
        }
        ++it1;
        ++it2;
    }
    return true;
}

TEST(OrderedMapTest, Constructor)
{
    typedef u64u64OrderedMap MapType;

    MapType map1(
        {
            {1, 43}, {2, 44}, {3, 45}, {4, 46}, {5, 47},
            {6, 48}, {7, 49}, {8, 50}, {9, 51}, {10, 52}
        });

    EXPECT_EQ(10ull, map1.size());

    auto it1 = map1.begin();
    for (u64 i = 0; i < 10; ++i) {
        EXPECT_EQ(i+1, it1->first);
        EXPECT_EQ(i+1+42, it1->second);
        ++it1;
    }

    EXPECT_TRUE(it1 == map1.end());

    MapType map2 = map1;
    EXPECT_EQ(10ull, map2.size());

    it1 = map1.begin();
    auto it2 = map2.begin();
    for (u64 i = 0; i < 10; ++i) {
        EXPECT_EQ(i+1, it1->first);
        EXPECT_EQ(i+1+42, it1->second);
        EXPECT_EQ(i+1, it2->first);
        EXPECT_EQ(i+1+42, it2->second);
        ++it1;
        ++it2;
    }
    EXPECT_TRUE(it1 == map1.end());
    EXPECT_TRUE(it2 == map2.end());

    MapType map3 = std::move(map2);
    EXPECT_EQ(0ull, map2.size());
    EXPECT_EQ(10ull, map3.size());

    auto it3 = map3.begin();
    for (u64 i = 0; i < 10; ++i) {
        EXPECT_EQ(i+1, it3->first);
        EXPECT_EQ(i+1+42, it3->second);
        ++it3;
    }
    EXPECT_TRUE(it3 == map3.end());
}

TEST(OrderedMapTest, Assignment)
{
    typedef u64u64OrderedMap MapType;

    MapType map1;
    map1 = { {1, 43}, {2, 44}, {3, 45}, {4, 46}, {5, 47},
             {6, 48}, {7, 49}, {8, 50}, {9, 51}, {10, 52} };

    EXPECT_EQ(10ull, map1.size());
    auto it1 = map1.begin();
    for (u64 i = 0; i < 10; ++i) {
        EXPECT_EQ(i+1, it1->first);
        EXPECT_EQ(i+1+42, it1->second);
        ++it1;
    }

    EXPECT_TRUE(it1 == map1.end());

    MapType map2 = map1;
    EXPECT_EQ(10ull, map2.size());

    it1 = map1.begin();
    auto it2 = map2.begin();
    for (u64 i = 0; i < 10; ++i) {
        EXPECT_EQ(i+1, it1->first);
        EXPECT_EQ(i+1+42, it1->second);
        EXPECT_EQ(i+1, it2->first);
        EXPECT_EQ(i+1+42, it2->second);
        ++it1;
        ++it2;
    }
    EXPECT_TRUE(it1 == map1.end());
    EXPECT_TRUE(it2 == map2.end());

    MapType map3 = std::move(map2);
    EXPECT_EQ(0ull, map2.size());
    EXPECT_EQ(10ull, map3.size());

    auto it3 = map3.begin();
    for (u64 i = 0; i < 10; ++i) {
        EXPECT_EQ(i+1, it3->first);
        EXPECT_EQ(i+1+42, it3->second);
        ++it3;
    }
    EXPECT_TRUE(it3 == map3.end());

}

TEST(OrderedMapTest, Functional)
{
    typedef u64u64OrderedMap MapType;

    MapType kinara_map;
    std::map<u64, u64> std_map;

    std::default_random_engine generator;
    std::uniform_int_distribution<u64> distribution(0, 1);

    for (u64 i = 0; i < max_test_iterations; ++i) {
        std_map.clear();
        kinara_map.clear();

        for (u64 j = 0; j < max_insertion_value; ++j) {
            auto flip = (distribution(generator) == 1);
            if (flip) {
                std_map[j] = j + 42;
                kinara_map[j] = j + 42;

                EXPECT_EQ(std_map.size(), kinara_map.size());
            }
        }

        EXPECT_TRUE(test_equal(kinara_map, std_map));

        // erase some random elements
        for (u64 j = 0; j < max_insertion_value; ++j) {
            auto flip = (distribution(generator) == 1);
            if (flip) {
                std_map.erase(j);
                kinara_map.erase(j);

                EXPECT_EQ(std_map.size(), kinara_map.size());
            }
        }

        EXPECT_TRUE(test_equal(kinara_map, std_map));

        // repeat the above two steps
        for (u64 j = 0; j < max_insertion_value; ++j) {
            auto flip = (distribution(generator) == 1);
            if (flip) {
                std_map[j] = j + 42;
                kinara_map[j] = j + 42;

                EXPECT_EQ(std_map.size(), kinara_map.size());
            }
        }

        EXPECT_TRUE(test_equal(kinara_map, std_map));

        // erase some random elements
        for (u64 j = 0; j < max_insertion_value; ++j) {
            auto flip = (distribution(generator) == 1);
            if (flip) {
                std_map.erase(j);
                kinara_map.erase(j);

                EXPECT_EQ(std_map.size(), kinara_map.size());
            }
        }

        EXPECT_TRUE(test_equal(kinara_map, std_map));
    }
}

TEST(OrderedMapTest, Performance)
{
    typedef u64u64OrderedMap MapType;

    MapType kinara_map;

    std::default_random_engine generator;
    std::uniform_int_distribution<u64> distribution(0, 1);

    for (u64 j = 0; j < (1 << 4); ++j) {
        kinara_map.clear();

        for (u64 i = 0; i < 64 * max_insertion_value; ++i) {
            kinara_map[i] = i + 42;
        }

        for (u64 i = 0; i < 64 * max_insertion_value; ++i) {
            if (distribution(generator) == 1) {
                kinara_map.erase(i);
            }
        }
    }
}

TEST(StdVsOrderedMapTest, Performance)
{
    std::map<u64, u64> std_map;

    std::default_random_engine generator;
    std::uniform_int_distribution<u64> distribution(0, 1);

    for (u64 j = 0; j < (1 << 4); ++j) {
        std_map.clear();

        for (u64 i = 0; i < 64 * max_insertion_value; ++i) {
            std_map[i] = i + 42;
        }

        for (u64 i = 0; i < 64 * max_insertion_value; ++i) {
            if (distribution(generator) == 1) {
                std_map.erase(i);
            }
        }
    }
}


//
// OrderedMapTests.cpp ends here
