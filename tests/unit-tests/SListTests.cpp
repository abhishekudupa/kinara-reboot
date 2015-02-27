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

TEST(SList, Constructor)
{
    u32SList u32_list1;
    EXPECT_EQ((u64)0, u32_list1.size());

    u32SList u32_list2((u64)10, (u32)42);
    EXPECT_EQ((u64)10, u32_list2.size());

    u32SList u32_list3(u32_list2.begin(), u32_list2.end());
    EXPECT_EQ((u64)10, u32_list3.size());

    // TODO: Add tests for rest of constructors
}

// TODO
// 1. Add tests for assignment operators
// 2. Add tests for insertions and emplacements and push_*
// 3. Add tests for pop_* and erase methods
// 4. Add test for resize
// 5. Add tests for splice_* methods
// 6. Add tests for remove_*
// 7. Add tests for unique
// 8. Add tests for merge and sort
// 9. Add tests for reverse
// 10. Add tests for relational operators
// 11. Add tests for RefCountable objects

//
// SListTests.cpp ends here
