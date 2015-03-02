// SListTests.cpp ---
//
// Filename: SListTests.cpp
// Author: Abhishek Udupa
// Created: Thu Feb 26 16:01:51 2015 (-0500)
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

#include "../../projects/kinara-common/src/containers/SList.hpp"
#include <vector>
#include <random>
#include <cstdlib>
#include <algorithm>

#include "../../thirdparty/gtest/include/gtest/gtest.h"

using kinara::u32;
using kinara::u64;
using kinara::containers::SList;
using kinara::containers::u32SList;
using kinara::containers::PtrSList;
using kinara::containers::PoolPtrSList;

using kinara::containers::PoolSList;
using kinara::containers::u32PoolSList;
using kinara::memory::RefCountable;

using testing::Types;

template <typename u32SListType>
class u32SListTest : public ::testing::Test
{
protected:
    u32SListTest() {}
    virtual ~u32SListTest() {}
};

class RCClass : public RefCountable
{
private:
    int m_data;

public:
    RCClass()
        : RefCountable(), m_data(0)
    {
        // nothing here
    }

    RCClass(int data)
        : RefCountable(), m_data(data)
    {
        // nothing here
    }

    RCClass(const RCClass& Other)
        : RefCountable(), m_data(Other.m_data)
    {
        // nothing here
    }

    inline operator int () const
    {
        return m_data;
    }

    virtual ~RCClass()
    {
        // nothing here
    }
};

template <typename RCSListType>
class RCListTest : public ::testing::Test
{
protected:
    RCListTest() {}
    virtual ~RCListTest() {}
};

TYPED_TEST_CASE_P(u32SListTest);
TYPED_TEST_CASE_P(RCListTest);

TYPED_TEST_P(u32SListTest, Constructor)
{
    typedef TypeParam u32ListType;
    u32ListType u32_list1;
    EXPECT_EQ((u64)0, u32_list1.size());

    u32ListType u32_list2((u64)10, (u32)42);
    EXPECT_EQ((u64)10, u32_list2.size());

    u32 i = 0;
    for (auto it = u32_list2.begin(), last = u32_list2.end(); it != last; ++it) {
        EXPECT_EQ((u32)42, *it);
        ++i;
    }

    EXPECT_EQ((u32)10, i);

    u32ListType u32_list3(u32_list2.begin(), u32_list2.end());
    EXPECT_EQ((u64)10, u32_list3.size());

    i = 0;
    for (auto it = u32_list3.begin(), last = u32_list3.end(); it != last; ++it) {
        EXPECT_EQ((u32)42, *it);
        ++i;
    }

    EXPECT_EQ((u32)10, i);

    u32ListType u32_list4({1, 2, 3, 4, 5});
    EXPECT_EQ((u64)5, u32_list4.size());

    i = 0;
    for (auto it = u32_list4.begin(), last = u32_list4.end(); it != last; ++it) {
        EXPECT_EQ(++i, *it);
    }

    EXPECT_EQ((u32)5, i);

    u32ListType u32_list5(u32_list4);
    i = 0;
    for (auto it = u32_list5.begin(), last = u32_list5.end(); it != last; ++it) {
        EXPECT_EQ(++i, *it);
    }

    EXPECT_EQ((u32)5, i);

    u32ListType u32_list6(std::move(u32_list5));
    i = 0;
    for (auto it = u32_list6.begin(), last = u32_list6.end(); it != last; ++it) {
        EXPECT_EQ(++i, *it);
    }

    EXPECT_EQ((u32)5, i);

    EXPECT_EQ((u64)0, u32_list5.size());
}

TYPED_TEST_P(u32SListTest, Assignment)
{
    typedef TypeParam u32ListType;
    u32ListType list1;
    for (u32 i = 0; i < 1024; ++i) {
        list1.push_back(i);
    }

    EXPECT_EQ((u64)1024, list1.size());
    auto list2 = list1;
    EXPECT_EQ((u64)1024, list1.size());

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

TYPED_TEST_P(u32SListTest, Insertions)
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

    list1.erase_after(list1.begin());

    EXPECT_EQ((u64)1, list1.size());
    EXPECT_EQ((u64)1, list1.front());
    EXPECT_EQ((u64)1, list1.back());

    list1.erase((list1.begin()));

    EXPECT_EQ((u64)0, list1.size());
    EXPECT_TRUE(list1.begin() == list1.end());

    // insertions
    list1.clear();
    list1 = { 1, 2, 3, 4, 5 };
    auto position = list1.begin();
    ++position;
    ++position;
    auto ins_pos = list1.insert_after(position, 42);

    EXPECT_EQ((u32)42, *ins_pos);
    EXPECT_EQ((u64)6, list1.size());
    EXPECT_EQ((u32)5, list1.back());

    ++position;
    ++position;
    ++position;
    ins_pos = list1.insert_after(position, 84);

    EXPECT_EQ((u32)84, *ins_pos);
    EXPECT_EQ((u64)7, list1.size());
    EXPECT_EQ((u32)84, list1.back());

    list1.clear();
    list1 = { 1, 2, 3, 4, 5 };
    position = list1.begin();
    ++position;
    ++position;
    ins_pos = list1.insert(position, 42);

    EXPECT_EQ((u32)42, *ins_pos);
    EXPECT_EQ((u64)6, list1.size());
    EXPECT_EQ((u32)5, list1.back());

    ++position;
    ++position;
    ins_pos = list1.insert(position, 84);

    EXPECT_EQ((u32)84, *ins_pos);
    EXPECT_EQ((u32)5, *position);
    EXPECT_EQ((u64)7, list1.size());
    EXPECT_EQ((u32)5, list1.back());

}

TYPED_TEST_P(u32SListTest, Resize)
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

TYPED_TEST_P(u32SListTest, Splice)
{
    typedef TypeParam u32ListType;

    u32ListType list1, list2;
    list1 = { 1, 2, 3, 9, 10 };
    list2 = { 4, 5, 6, 7, 8 };

    auto pos = list1.begin();
    ++pos;
    ++pos;

    list1.splice_after(pos, list2);

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

    list1 = { 1, 2, 3, 9, 10 };
    list2 = { 4, 5, 6, 7, 8 };

    pos = list1.begin();
    ++pos;
    ++pos;
    ++pos;

    list1.splice(pos, list2);
    EXPECT_EQ((u64)10, list1.size());
    EXPECT_EQ((u64)0, list2.size());
    EXPECT_TRUE(list2.begin() == list2.end());

    i = 0;
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

    pos = list1.begin();
    ++pos;

    list1.splice_after(pos, list2, opos);
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

    list1 = { 1, 2, 4, 5 };
    list2 = { 6, 7, 3, 8, 9, 10 };

    opos = list2.begin();
    ++opos;
    ++opos;

    pos = list1.begin();
    ++pos;
    ++pos;

    list1.splice_element(pos, list2, opos);
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

    auto opos_begin = list2.before_begin();
    auto opos_end = list2.begin();
    ++opos_end;
    ++opos_end;

    list1.splice_after(pos, list2, opos_begin, opos_end);
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

    list1 = { 1, 2, };
    list2 = { 3, 4, 5, 6, 7, 8, 9, 10 };

    pos = list1.begin();
    ++pos;

    opos_begin = list2.before_begin();
    opos_end = list2.begin();
    ++opos_end;
    ++opos_end;
    ++opos_end;

    list1.splice_after(pos, list2, opos_begin, opos_end);
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

    list1 = { 1, 2, };
    list2 = { 3, 4, 5, 6, 7, 8, 9, 10 };

    pos = list1.end();
    opos_begin = list2.before_begin();
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

    pos = list1.before_begin();
    opos = list2.begin();
    ++opos;
    ++opos;
    ++opos;
    ++opos;
    ++opos;

    list1.splice_element_after(pos, list2, opos);

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

    list1 = { 2, 3, 4, 5 };
    list2 = { 6, 7, 8, 9, 10, 1 };

    pos = list1.begin();
    opos = list2.begin();
    ++opos;
    ++opos;
    ++opos;
    ++opos;
    ++opos;

    list1.splice_element(pos, list2, opos);

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

TYPED_TEST_P(u32SListTest, Remove)
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

TYPED_TEST_P(u32SListTest, Unique)
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

TYPED_TEST_P(u32SListTest, SortMerge)
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

TYPED_TEST_P(u32SListTest, Reverse)
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

TYPED_TEST_P(u32SListTest, Relational)
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

TYPED_TEST_P(RCListTest, RefCountableTests)
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

REGISTER_TYPED_TEST_CASE_P(u32SListTest,
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

REGISTER_TYPED_TEST_CASE_P(RCListTest, RefCountableTests);

typedef Types<u32SList, u32PoolSList> u32SListImplementations;
typedef Types<PtrSList<RCClass>, PoolPtrSList<RCClass>> RCListImplementations;

INSTANTIATE_TYPED_TEST_CASE_P(NonPoolAndPool,
                              u32SListTest, u32SListImplementations);
INSTANTIATE_TYPED_TEST_CASE_P(NonPoolAndPoolRC,
                              RCListTest, RCListImplementations);

//
// SListTests.cpp ends here
