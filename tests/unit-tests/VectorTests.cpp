// VectorTests.cpp ---
// Filename: VectorTests.cpp
// Author: Abhishek Udupa
// Created: Mon Feb 23 19:22:47 2015 (-0500)
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

#include "../../projects/kinara-common/src/containers/Vector.hpp"
#include <vector>
#include <random>
#include <cstdlib>
#include <algorithm>

#include "../../thirdparty/gtest/include/gtest/gtest.h"

using kinara::containers::u32Vector;
using kinara::containers::u32CompactVector;
using kinara::containers::PtrVector;
using kinara::memory::RefCountable;

using kinara::u32;
using kinara::u64;

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

TEST(Vector, EmptyIntVector)
{
    u32Vector test_vector;
    EXPECT_EQ((u64)0, test_vector.size());
    EXPECT_EQ(nullptr, test_vector.data());
}

TEST(Vector, ShortIntVector)
{
    u32Vector test_vector;
    test_vector = {1, 2, 3};

    EXPECT_EQ((u32)3, test_vector.size());
    EXPECT_EQ((u32)1, test_vector[0]);
    EXPECT_EQ((u32)2, test_vector[1]);
    EXPECT_EQ((u32)3, test_vector[2]);

    u32Vector test_vector2;
    test_vector2 = test_vector;
    test_vector.push_back(2);

    EXPECT_EQ((u32)3, test_vector2.size());
    EXPECT_EQ((u32)1, test_vector2[0]);
    EXPECT_EQ((u32)2, test_vector2[1]);
    EXPECT_EQ((u32)3, test_vector2[2]);

    EXPECT_EQ((u32)4, test_vector.size());
    EXPECT_EQ((u32)1, test_vector[0]);
    EXPECT_EQ((u32)2, test_vector[1]);
    EXPECT_EQ((u32)3, test_vector[2]);
    EXPECT_EQ((u32)2, test_vector[3]);
}

TEST(Vector, LongIntVector)
{
    u32Vector vector1;

    for (int i = 0; i < (1 << 16); ++i) {
        vector1.push_back(i);
    }

    u32Vector vector2 = vector1;

    for (int i = 0; i < (1 << 16); ++i) {
        EXPECT_EQ((u32)i, vector1[i]);
        EXPECT_EQ((u32)i, vector2[i]);
    }

    EXPECT_TRUE(vector1 == vector2);

    vector1.clear();

    EXPECT_EQ((u32)0, vector1.size());
    EXPECT_EQ((u32)(1 << 16), vector2.size());

    for (int i = 0; i < 10; ++i) {
        vector1.push_back(i);
    }
    vector2.insert(vector2.begin() + 10, vector1.begin(), vector1.end());
    EXPECT_EQ((u64)((1<<16) + 10), vector2.size());

    EXPECT_TRUE(vector1 < vector2);
    EXPECT_TRUE(vector2 > vector1);

    for (int i = 0; i < (1 << 16) + 10; ++i) {
        if (i < 10) {
            EXPECT_EQ((u32)i, vector2[i]);
        }
        else {
            EXPECT_EQ((u32)(i - 10), vector2[i]);
        }
    }
}

TEST(Vector, RefCountableObjects)
{
    PtrVector<RCClass> vector1;

    for (int i = 0; i < (1 << 16); ++i) {
        vector1.push_back(new RCClass(i));
    }

    PtrVector<RCClass> vector2 = vector1;

    for (int i = 0; i < (1 << 16); ++i) {
        EXPECT_EQ(i, (int)(*(vector1[i])));
        EXPECT_EQ(i, (int)(*(vector2[i])));
    }

    vector1.clear();

    EXPECT_EQ((u32)0, vector1.size());
    EXPECT_EQ((u32)(1 << 16), vector2.size());

    for (int i = 0; i < 10; ++i) {
        vector1.push_back(new RCClass(i));
    }

    vector2.insert(vector2.begin() + 10, vector1.begin(), vector1.end());

    EXPECT_EQ((u64)((1<<16) + 10), vector2.size());

    for (int i = 0; i < (1 << 16) + 10; ++i) {
        if (i < 10) {
            EXPECT_EQ(i, (int)(*(vector2[i])));
        }
        else {
            EXPECT_EQ(i - 10, (int)(*(vector2[i])));
        }
    }
}

// test compatibility of iterators
// with the rest of stl
TEST(Vector, IteratorCompat)
{
    u32Vector test_vector = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    std::sort(test_vector.begin(), test_vector.end());
    for (u64 i = 0, last = test_vector.size(); i < last; ++i) {
        for (u64 j = i + 1; j < last; ++j) {
            EXPECT_LE(test_vector[i], test_vector[j]);
        }
    }

    u32 i = 0;
    for (auto elem : test_vector) {
        EXPECT_EQ(i, elem);
        ++i;
    }
}

// TODO: Add tests for relational operators

//
// VectorTests.cpp ends here
