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
using kinara::containers::PtrDList;
using kinara::containers::PoolPtrDList;

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
}

REGISTER_TYPED_TEST_CASE_P(u32DListTest,
                           Constructor);

typedef Types<u32DList, u32PoolDList> u32DListImplementations;

INSTANTIATE_TYPED_TEST_CASE_P(NonPoolAndPool,
                              u32DListTest, u32DListImplementations);

//
// DListTests.cpp ends here
