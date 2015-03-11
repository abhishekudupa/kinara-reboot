// DListTests.cpp ---
// Filename: DListTests.cpp
// Author: Abhishek Udupa
// Created: Mon Mar  2 22:42:15 2015 (-0500)
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

#include "../../projects/kinara-common/src/containers/DList.hpp"
#include <vector>
#include <random>
#include <cstdlib>
#include <algorithm>

#include "RCClass.hpp"

#include "../../thirdparty/gtest/include/gtest/gtest.h"

using kinara::u32;
using kinara::u64;
using kinara::containers::DList;
using kinara::containers::u32DList;
using kinara::containers::MPtrDList;
using kinara::containers::PoolMPtrDList;

using kinara::containers::PoolDList;
using kinara::containers::u32PoolDList;

using testing::Types;

template <typename u32DListType>
class u32DListTest : public testing::Test
{
protected:
    u32DListTest() {}
    virtual ~u32DListTest() {}
};

template <typename RCSListType>
class RCDListTest : public ::testing::Test
{
protected:
    RCDListTest() {}
    virtual ~RCDListTest() {}
};

TYPED_TEST_CASE_P(u32DListTest);
TYPED_TEST_CASE_P(RCDListTest);

TYPED_TEST_P(u32DListTest, Constructor)
{
    typedef TypeParam u32ListType;

    u32ListType list1;
    EXPECT_EQ(0ull, list1.size());
    EXPECT_TRUE(list1.begin() == list1.end());

    u32ListType list2(10);
    EXPECT_EQ(10ull, list2.size());

    u32 i = 0;
    for (auto num : list2) {
        EXPECT_EQ(0u, num);
        ++i;
    }
    EXPECT_EQ(10u, i);

    u32ListType list3(10, 42u);
    i = 0;
    for (auto num : list3) {
        EXPECT_EQ(42u, num);
        ++i;
    }
    EXPECT_EQ(10u, i);

    u32ListType list4(list3.begin(), list3.end());
    i = 0;
    for (auto num : list4) {
        EXPECT_EQ(42u, num);
        ++i;
    }
    EXPECT_EQ(10u, i);

    u32ListType list5(list4);
    i = 0;
    for (auto num : list5) {
        EXPECT_EQ(42u, num);
        ++i;
    }
    EXPECT_EQ(10u, i);

    u32ListType list6({1, 2, 3, 4, 5});
    EXPECT_EQ(5ull, list6.size());
    i = 0;
    for (auto num : list6) {
        EXPECT_EQ(++i, num);
    }
    EXPECT_EQ(5u, i);
}

TYPED_TEST_P(u32DListTest, Assignment)
{
    typedef TypeParam u32ListType;
    u32ListType list1;
    for (u32 i = 0; i < 1024; ++i) {
        list1.push_back(i);
    }

    EXPECT_EQ(1024ull, list1.size());
    auto list2 = list1;

    EXPECT_EQ((u64)1024, list1.size());
    EXPECT_EQ((u64)1024, list2.size());

    u32 i = 0;
    for (auto const& num : list1) {
        EXPECT_EQ(i++, num);
    }
    EXPECT_EQ((u32)1024, i);

    i = 0;
    for (auto const& num : list2) {
        EXPECT_EQ(i++, num);
    }
    EXPECT_EQ((u32)1024, i);

    auto list3 = std::move(list1);
    i = 0;
    for (auto const& num : list3) {
        EXPECT_EQ(i++, num);
    }

    EXPECT_EQ((u32)1024, i);
    EXPECT_EQ((u64)0, list1.size());
}

TYPED_TEST_P(u32DListTest, Insertions)
{
    typedef TypeParam u32ListType;

    u32ListType list1;
    list1.push_back(2);
    list1.push_front(1);

    EXPECT_EQ((u64)2, list1.size());

    auto it = list1.begin();
    EXPECT_EQ((u32)1, *it);
    ++it;
    EXPECT_EQ((u32)2, *it);

    list1.emplace_back(2);
    list1.emplace_front(1);

    EXPECT_EQ((u64)4, list1.size());

    it = list1.begin();
    EXPECT_EQ((u32)1, *it);
    ++it;
    EXPECT_EQ((u32)1, *it);
    ++it;
    EXPECT_EQ((u32)2, *it);
    ++it;
    EXPECT_EQ((u32)2, *it);

    list1.pop_back();
    list1.pop_front();

    EXPECT_EQ((u64)2, list1.size());

    it = list1.begin();
    EXPECT_EQ((u32)1, *it);
    ++it;
    EXPECT_EQ((u32)2, *it);

    list1.clear();

    EXPECT_EQ((u64)0, list1.size());
    EXPECT_TRUE(list1.begin() == list1.end());

    list1.push_front(1);
    list1.push_back(2);

    list1.erase(list1.begin());

    EXPECT_EQ((u64)1, list1.size());
    EXPECT_EQ((u64)2, list1.front());
    EXPECT_EQ((u64)2, list1.back());

    list1.erase((list1.begin()));

    EXPECT_EQ((u64)0, list1.size());
    EXPECT_TRUE(list1.begin() == list1.end());

    // insertions
    list1.clear();
    list1 = { 1, 2, 3, 4, 5 };
    auto position = list1.begin();
    ++position;
    ++position;
    auto ins_pos = list1.insert(position, 42);

    EXPECT_EQ((u32)42, *ins_pos);
    EXPECT_EQ((u64)6, list1.size());
    EXPECT_EQ((u32)5, list1.back());

    ++position;
    ++position;
    ++position;
    ins_pos = list1.insert(position, 84);

    EXPECT_EQ((u32)84, *ins_pos);
    EXPECT_EQ((u64)7, list1.size());
    EXPECT_EQ((u32)84, list1.back());
}

TYPED_TEST_P(u32DListTest, Resize)
{
    typedef TypeParam u32ListType;

    u32ListType list1;
    list1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    list1.resize(5);

    u32 i = 0;
    for (auto num : list1) {
        EXPECT_EQ((u32)(++i), num);
    }
    EXPECT_EQ((u32)5, i);

    list1.resize(10);
    EXPECT_EQ((u64)10, list1.size());

    i = 0;
    for (auto num : list1) {
        if (i < 5) {
            EXPECT_EQ((u32)(++i), num);
        } else {
            EXPECT_EQ((u32)0, num);
            ++i;
        }
    }

    EXPECT_EQ((u32)10, i);
    list1.resize(0);
    EXPECT_EQ((u64)0, list1.size());
}

TYPED_TEST_P(u32DListTest, Splice)
{
    typedef TypeParam u32ListType;

    u32ListType list1, list2;
    list1 = { 1, 2, 3, 9, 10 };
    list2 = { 4, 5, 6, 7, 8 };

    auto pos = list1.begin();
    ++pos;
    ++pos;
    ++pos;

    list1.splice(pos, list2);

    EXPECT_EQ((u64)10, list1.size());
    EXPECT_EQ((u64)0, list2.size());
    EXPECT_TRUE(list2.begin() == list2.end());

    u32 i = 0;
    for (auto num : list1) {
        EXPECT_EQ((u32)(++i), num);
    }

    EXPECT_EQ((u32)10, i);

    list1.clear();
    list2.clear();

    list1 = { 1, 2, 4, 5 };
    list2 = { 6, 7, 3, 8, 9, 10 };

    auto opos = list2.begin();
    ++opos;
    ++opos;

    pos = list1.begin();
    ++pos;
    ++pos;

    list1.splice(pos, list2, opos);
    EXPECT_EQ((u64)5, list1.size());
    EXPECT_EQ((u64)5, list2.size());

    i = 0;
    for (auto num : list1) {
        EXPECT_EQ((u32)(++i), num);
    }
    EXPECT_EQ((u64)5, i);
    for (auto num : list2) {
        EXPECT_EQ((u32)(++i), num);
    }
    EXPECT_EQ((u64)10, i);

    list1.clear();
    list2.clear();

    list1 = { 1, 2, 5 };
    list2 = { 3, 4, 6, 7, 8, 9, 10 };

    pos = list1.begin();
    ++pos;
    ++pos;

    auto opos_begin = list2.begin();
    auto opos_end = list2.begin();
    ++opos_end;
    ++opos_end;

    list1.splice(pos, list2, opos_begin, opos_end);
    EXPECT_EQ((u64)5, list1.size());
    EXPECT_EQ((u64)5, list2.size());

    i = 0;
    for (auto num : list1) {
        EXPECT_EQ((u32)(++i), num);
    }

    EXPECT_EQ((u64)5, i);
    for (auto num : list2) {
        EXPECT_EQ((u32)(++i), num);
    }
    EXPECT_EQ((u64)10, i);

    list1.clear();
    list2.clear();

    list1 = { 1, 2 };
    list2 = { 3, 4, 5, 6, 7, 8, 9, 10 };

    pos = list1.end();
    opos_begin = list2.begin();
    opos_end = list2.begin();
    ++opos_end;
    ++opos_end;
    ++opos_end;

    list1.splice(pos, list2, opos_begin, opos_end);
    EXPECT_EQ((u64)5, list1.size());
    EXPECT_EQ((u64)5, list2.size());

    i = 0;
    for (auto num : list1) {
        EXPECT_EQ((u32)(++i), num);
    }

    EXPECT_EQ((u64)5, i);
    for (auto num : list2) {
        EXPECT_EQ((u32)(++i), num);
    }
    EXPECT_EQ((u64)10, i);

    list1.clear();
    list2.clear();

    list1 = { 2, 3, 4, 5 };
    list2 = { 6, 7, 8, 9, 10, 1 };

    pos = list1.begin();
    opos = list2.begin();
    ++opos;
    ++opos;
    ++opos;
    ++opos;
    ++opos;

    list1.splice(pos, list2, opos);

    EXPECT_EQ((u64)5, list1.size());
    EXPECT_EQ((u64)5, list2.size());

    i = 0;
    for (auto num : list1) {
        EXPECT_EQ((u32)(++i), num);
    }

    EXPECT_EQ((u64)5, i);
    for (auto num : list2) {
        EXPECT_EQ((u32)(++i), num);
    }
    EXPECT_EQ((u64)10, i);
}

TYPED_TEST_P(u32DListTest, Remove)
{
    typedef TypeParam u32ListType;

    u32ListType list1;

    list1 = { 1, 2, 3, 4, 5 };
    list1.remove(3);
    EXPECT_EQ((u64)4, list1.size());
    auto it = list1.begin();
    EXPECT_EQ(1u, *it);
    ++it;
    EXPECT_EQ(2u, *it);
    ++it;
    EXPECT_EQ(4u, *it);
    ++it;
    EXPECT_EQ(5u, *it);
    ++it;
    EXPECT_EQ(list1.end(), it);
}

TYPED_TEST_P(u32DListTest, Unique)
{
    typedef TypeParam u32ListType;
    u32ListType list = { 1, 2, 2, 3, 4, 4 };
    list.unique();

    EXPECT_EQ(4ull, list.size());
    auto it = list.begin();

    EXPECT_EQ(1u, *it);
    ++it;
    EXPECT_EQ(2u, *it);
    ++it;
    EXPECT_EQ(3u, *it);
    ++it;
    EXPECT_EQ(4u, *it);
    ++it;
    EXPECT_EQ(list.end(), it);
}

TYPED_TEST_P(u32DListTest, SortMerge)
{
    typedef TypeParam u32ListType;
    u32ListType list1 = { 5, 10, 9, 1, 2 };
    u32ListType list2 = { 4, 6, 8, 7, 3 };
    list1.sort();
    list2.sort();

    EXPECT_EQ(5u, list1.size());
    EXPECT_EQ(5u, list2.size());

    auto it = list1.begin();
    EXPECT_EQ(1u, *it);
    ++it;
    EXPECT_EQ(2u, *it);
    ++it;
    EXPECT_EQ(5u, *it);
    ++it;
    EXPECT_EQ(9u, *it);
    ++it;
    EXPECT_EQ(10u, *it);
    ++it;

    EXPECT_EQ(list1.end(), it);

    it = list2.begin();
    EXPECT_EQ(3u, *it);
    ++it;
    EXPECT_EQ(4u, *it);
    ++it;
    EXPECT_EQ(6u, *it);
    ++it;
    EXPECT_EQ(7u, *it);
    ++it;
    EXPECT_EQ(8u, *it);
    ++it;

    EXPECT_EQ(list2.end(), it);

    // test merge
    list1.merge(list2);
    EXPECT_EQ(10ull, list1.size());
    EXPECT_EQ(0ull, list2.size());

    u32 i = 0;
    for (auto num : list1) {
        EXPECT_EQ(++i, num);
    }

    EXPECT_EQ(10u, i);
}

TYPED_TEST_P(u32DListTest, Reverse)
{
    typedef TypeParam u32ListType;
    u32ListType list;
    list = { 5, 4, 3, 2, 1 };
    list.reverse();
    EXPECT_EQ(5ull, list.size());
    u32 i = 0;
    for (auto num : list) {
        EXPECT_EQ(++i, num);
    }
    EXPECT_EQ(5u, i);
}

TYPED_TEST_P(u32DListTest, Relational)
{
    typedef TypeParam u32ListType;
    u32ListType list1, list2, list3, list4, list5;
    list1 = { 1, 2, 3, 4, 5 };
    list5 = { 1, 2, 3, 4, 5 };
    list2 = { 9, 10 };
    list3 = { 1, 2, 3, 4, 6 };
    list4 = { 1, 2, 3, 4 };


    EXPECT_LT(list2, list1);
    EXPECT_GT(list1, list2);
    EXPECT_LT(list1, list3);
    EXPECT_EQ(list1, list5);
    EXPECT_LT(list4, list3);
    EXPECT_GT(list3, list4);
}

TYPED_TEST_P(RCDListTest, RefCountableTests)
{
    typedef TypeParam ListType;

    ListType list1;
    for (u32 i = 0; i < 128; ++i) {
        list1.push_back(new RCClass(i));
    }

    list1.emplace_front(new RCClass(128));
    list1.emplace_back(new RCClass(129));

    list1.resize(10);
    list1.clear();
}

REGISTER_TYPED_TEST_CASE_P(u32DListTest,
                           Constructor,
                           Assignment,
                           Insertions,
                           Resize,
                           Splice,
                           Remove,
                           Unique,
                           SortMerge,
                           Reverse,
                           Relational);

REGISTER_TYPED_TEST_CASE_P(RCDListTest, RefCountableTests);

typedef Types<u32DList, u32PoolDList> u32DListImplementations;
typedef Types<MPtrDList<RCClass>, PoolMPtrDList<RCClass>> RCDListImplementations;

INSTANTIATE_TYPED_TEST_CASE_P(NonPoolAndPoolDList,
                              u32DListTest, u32DListImplementations);
INSTANTIATE_TYPED_TEST_CASE_P(NonPoolAndPoolDListRC,
                              RCDListTest, RCDListImplementations);

//
// DListTests.cpp ends here
