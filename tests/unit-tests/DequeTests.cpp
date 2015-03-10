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

#include "../../projects/kinara-common/src/containers/Deque.hpp"
#include <algorithm>
#include <deque>

#include "RCClass.hpp"

#include "../../thirdparty/gtest/include/gtest/gtest.h"

using kinara::u32;
using kinara::u64;
using kinara::containers::Deque;
using kinara::containers::u32Deque;
using kinara::containers::PtrDeque;

TEST(u32DequeTest, Constructor)
{
    u32Deque deque1;
    EXPECT_EQ(0ull, deque1.size());

    u32Deque deque2(10);
    EXPECT_EQ(10ull, deque2.size());

    u32Deque deque3({1, 2, 3, 4, 5 });
    EXPECT_EQ(5ull, deque3.size());

    u32Deque deque4(deque3);
    EXPECT_EQ(5ull, deque3.size());
    EXPECT_EQ(5ull, deque4.size());

    u32 i = 0;
    for (auto it = deque3.begin(); it != deque3.end(); ++it) {
        EXPECT_EQ(++i, *it);
    }
    EXPECT_EQ(5u, i);

    i = 0;
    for (auto it = deque4.begin(); it != deque4.end(); ++it) {
        EXPECT_EQ(++i, *it);
    }
    EXPECT_EQ(5u, i);

    u32Deque deque5(u32Deque({1, 2, 3, 4, 5}));
    EXPECT_EQ(5ull, deque5.size());
    i = 0;
    for (auto it = deque5.begin(); it != deque5.end(); ++it) {
        EXPECT_EQ(++i, *it);
    }
    EXPECT_EQ(5u, i);

    u32Deque deque6((u64)10, 42);
    EXPECT_EQ(10ull, deque6.size());
    i = 0;
    for (auto it = deque6.begin(); it != deque6.end(); ++it) {
        EXPECT_EQ(42u, *it);
        ++i;
    }
    EXPECT_EQ(10u, i);
}

TEST(u32DequeTest, Assignment)
{
    u32Deque deque1;
    deque1.push_back(1);
    deque1.push_back(2);
    deque1.push_back(3);

    u32Deque deque2 = deque1;

    EXPECT_EQ(3ull, deque1.size());
    EXPECT_EQ(3ull, deque2.size());

    u32 i = 0;
    for (auto num : deque1) {
        EXPECT_EQ(++i, num);
    }
    EXPECT_EQ(3u, i);

    i = 0;
    for (auto num : deque2) {
        EXPECT_EQ(++i, num);
    }
    EXPECT_EQ(3u, i);

    deque1 = u32Deque({1, 2, 3, 4, 5, 6 });
    EXPECT_EQ(6ull, deque1.size());

    i = 0;
    for (auto num : deque1) {
        EXPECT_EQ(++i, num);
    }
    EXPECT_EQ(6u, i);

    deque1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    i = 0;
    for (auto num : deque1) {
        EXPECT_EQ(++i, num);
    }
    EXPECT_EQ(10u, i);

    // some tests on larger deques
    const u32 test_size = 65536;
    deque1.clear();

    for (u32 i = 0; i < test_size; ++i) {
        deque1.push_back(i);
    }

    deque2 = deque1;

    auto it1 = deque1.cbegin();
    auto it2 = deque2.cbegin();

    for (i = 0; i < test_size; ++i) {
        EXPECT_EQ(i, deque1[i]);
        EXPECT_EQ(i, deque2[i]);
        EXPECT_EQ(i, *it1);
        EXPECT_EQ(i, *it2);
        ++it1;
        ++it2;
    }

    EXPECT_EQ(i, test_size);
    EXPECT_EQ(it1, deque1.end());
    EXPECT_EQ(it2, deque2.end());
    EXPECT_NE(it2, deque1.end());
    EXPECT_NE(it1, deque2.end());
}

// We compare against std::deque for correctness
// caveat: assumes that std::deque is correct! :-)
TEST(u32DequeTest, PushPop)
{
    const u32 num_iterations = 65536;

    u32Deque deque1;
    std::deque<u32> std_deque;

    std::default_random_engine generator;
    std::uniform_int_distribution<u32> distribution(0, (1 << 30));

    for (u32 i = 0; i < num_iterations; ++i) {

        auto num_to_push_back = distribution(generator) % 256;
        for (u32 j = 0; j < num_to_push_back; ++j) {
            auto elem = distribution(generator) % (1 << 30);
            deque1.push_back(elem);
            std_deque.push_back(elem);
            EXPECT_EQ(std_deque.size(), deque1.size());
        }

        auto num_to_push_front = distribution(generator) % 256;
        for (u32 j = 0; j < num_to_push_front; ++j) {
            auto elem = distribution(generator) % (1 << 30);
            deque1.push_front(elem);
            std_deque.push_front(elem);
            EXPECT_EQ(std_deque.size(), deque1.size());
        }

        if (deque1.size() > 0) {
            auto num_to_pop_back = distribution(generator) % deque1.size();
            for (u32 j = 0; j < num_to_pop_back; ++j) {
                auto elem1 = deque1.back();
                auto std_elem = std_deque.back();
                EXPECT_EQ(std_elem, elem1);

                deque1.pop_back();
                std_deque.pop_back();
                EXPECT_EQ(std_deque.size(), deque1.size());
            }

            if (deque1.size() > 0) {
                auto num_to_pop_front = distribution(generator) % deque1.size();
                for (u32 j = 0; j < num_to_pop_front; ++j) {
                    auto elem1 = deque1.front();
                    auto std_elem = std_deque.front();
                    EXPECT_EQ(std_elem, elem1);

                    deque1.pop_front();
                    std_deque.pop_front();
                    EXPECT_EQ(std_deque.size(), deque1.size());
                }
            }
        }

        EXPECT_EQ(std_deque.size(), deque1.size());
        while (!deque1.empty()) {
            if ((distribution(generator) % 2) == 0) {
                auto elem = deque1.back();
                auto std_elem = std_deque.back();
                EXPECT_EQ(std_elem, elem);
                deque1.pop_back();
                std_deque.pop_back();
            } else {
                auto elem = deque1.front();
                auto std_elem = std_deque.front();
                EXPECT_EQ(std_elem, elem);
                deque1.pop_front();
                std_deque.pop_front();
            }
        }

        EXPECT_EQ(std_deque.size(), deque1.size());
        EXPECT_EQ((u64)0, std_deque.size());
    }
}

TEST(u32DequeTest, Insertions)
{
    u32Deque deque1;
    std::deque<u32> std_deque;
    std::vector<u32> insert_vector;

    // test short insertions
    deque1 = { 1, 2, 3, 9, 10 };
    insert_vector = { 4, 5, 6, 7, 8 };
    auto pos = deque1.begin();
    ++pos;
    ++pos;
    ++pos;
    deque1.insert(pos, insert_vector.begin(), insert_vector.end());
    EXPECT_EQ(10ull, deque1.size());

    u32 k = 0;
    for (auto num : deque1) {
        EXPECT_EQ(++k, num);
    }
    EXPECT_EQ(10u, k);

    deque1.clear();
    deque1 = { 1, 7, 8, 9, 10 };
    insert_vector = { 2, 3, 4, 5, 6 };
    pos = deque1.begin();
    ++pos;
    deque1.insert(pos, insert_vector.begin(), insert_vector.end());

    EXPECT_EQ(10ull, deque1.size());

    k = 0;
    for (auto num : deque1) {
        EXPECT_EQ(++k, num);
    }
    EXPECT_EQ(10u, k);


    const u32 num_iterations = 65536;
    const u32 max_test_size = 8192;

    std::default_random_engine generator;
    std::uniform_int_distribution<u32> distribution(0, (1 << 30));

    for (u32 j = 0; j < num_iterations; ++j) {
        u32 test_size = 1 + distribution(generator) % max_test_size;
        u32 insert_position = distribution(generator) % test_size;
        u32 hole_size = distribution(generator) % (test_size - insert_position);

        deque1.clear();
        std_deque.clear();
        insert_vector.clear();

        for (u32 i = 0; i < insert_position; ++i) {
            deque1.push_back(i);
            std_deque.push_back(i);
            EXPECT_EQ(std_deque.size(), deque1.size());
        }

        for (u32 i = insert_position; i < insert_position + hole_size; ++i) {
            insert_vector.push_back(i);
        }

        for (u32 i = insert_position + hole_size; i < test_size; ++i) {
            deque1.push_back(i);
            std_deque.push_back(i);
            EXPECT_EQ(std_deque.size(), deque1.size());
        }

        u32Deque::iterator insert_iterator = deque1.begin();
        insert_iterator = insert_iterator + insert_position;

        deque1.insert(insert_iterator, insert_vector.cbegin(), insert_vector.cend());
        std_deque.insert(std_deque.begin() + insert_position, insert_vector.cbegin(), insert_vector.cend());

        EXPECT_EQ(std_deque.size(), deque1.size());
        for (u32 i = 0; i < test_size; ++i) {
            EXPECT_EQ(i, std_deque[i]);
            EXPECT_EQ(i, deque1[i]);
        }
        EXPECT_EQ(test_size, std_deque.end() - std_deque.begin());
        EXPECT_EQ(test_size, deque1.end() - deque1.begin());
    }
}

//
// DListTests.cpp ends here
