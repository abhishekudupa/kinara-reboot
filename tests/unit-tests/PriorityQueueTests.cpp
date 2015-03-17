// PriorityQueueTests.cpp ---
// Filename: PriorityQueueTests.cpp
// Author: Abhishek Udupa
// Created: Sat Mar 14 13:39:32 2015 (-0400)
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

#include "../../projects/kinara-common/src/containers/PriorityQueue.hpp"
#include "../../projects/kinara-common/src/containers/Vector.hpp"
#include "../../projects/kinara-common/src/containers/MultiWayHeap.hpp"

#include <vector>
#include <utility>
#include <random>
#include <cstdlib>
#include <algorithm>
#include <queue>

#include "RCClass.hpp"

#include "../../thirdparty/gtest/include/gtest/gtest.h"

#define MAX_TEST_SIZE (1 << 12)
#define NUM_TEST_ITERATIONS (1 << 5)
#define NUM_INSERT_DELETE_ITERATIONS (1 << 5);

using kinara::u32;
using kinara::u64;
using kinara::i32;
using kinara::i64;

using kinara::containers::PriorityQueue;
using kinara::containers::i64PriorityQueue;
using kinara::containers::BinaryHeap;
using kinara::containers::TernaryHeap;
using kinara::containers::QuaternaryHeap;
using kinara::containers::MultiWayHeap;
using kinara::containers::Vector;

using testing::Types;

template <typename PrioQueueType>
class PrioQueueTest : public testing::Test
{
protected:
    PrioQueueTest() {}
    virtual ~PrioQueueTest() {}
};

#define PERF_TEST_TEST_SIZE ((u64)(1 << 20))
#define PERF_TEST_ITERATIONS ((u64)(1 << 6))

template <typename PrioQueueType>
class PrioQueuePerfTest : public testing::Test
{
protected:
    Vector<i64> m_random_data;

    PrioQueuePerfTest()
        : m_random_data((size_t)PERF_TEST_TEST_SIZE, 0)
    {
        std::default_random_engine generator;
        std::uniform_int_distribution<i64> distribution(0, 1 << 30);

        for (u64 i = 0; i < PERF_TEST_TEST_SIZE; ++i) {
            auto elem = distribution(generator);
            m_random_data[i] = elem;
        }
    }

    virtual ~PrioQueuePerfTest()
    {
        // Nothing here
    }
};

class i64i64PairCompare
{
public:
    inline bool operator () (const std::pair<i64, i64>& pair1,
                             const std::pair<i64, i64>& pair2) const
    {
        return (pair1.first > pair2.first);
    }
};

TYPED_TEST_CASE_P(PrioQueueTest);
TYPED_TEST_CASE_P(PrioQueuePerfTest);

TYPED_TEST_P(PrioQueueTest, Constructor)
{
    typedef TypeParam PrioQueueT;
    PrioQueueT prio_queue;

    EXPECT_EQ(0ul, prio_queue.size());
}


TYPED_TEST_P(PrioQueueTest, Functional)
{
    typedef TypeParam PrioQueueT;
    PrioQueueT prio_queue;
    std::priority_queue<std::pair<i64, i64>,
                        std::vector<std::pair<i64, i64> >,
                        i64i64PairCompare> std_prio_queue;

    const u32 max_test_size = MAX_TEST_SIZE;
    const u32 num_iterations = NUM_TEST_ITERATIONS;
    const u32 num_insert_delete_iterations = NUM_INSERT_DELETE_ITERATIONS;

    std::default_random_engine generator;
    std::uniform_int_distribution<u32> distribution(0, 1 << 30);

    for (u32 i = 0; i < num_iterations; ++i) {
        const u32 test_size = 1 + distribution(generator) % max_test_size;
        for (u32 j = 0; j < num_insert_delete_iterations; ++j) {
            u64 current_size = prio_queue.size();

            EXPECT_EQ(std_prio_queue.size(), current_size);

            u32 num_to_insert = distribution(generator) % (test_size - current_size);

            for (u32 k = 0; k < num_to_insert; ++k) {
                auto key = distribution(generator);
                prio_queue.push(std::make_pair((i64)key, (i64)key));
                std_prio_queue.push(std::make_pair(key, key));

                auto const& prio_top = prio_queue.top();
                auto const& std_top = std_prio_queue.top();

                EXPECT_EQ(std_prio_queue.size(), prio_queue.size());
                EXPECT_EQ(std_top, prio_top);
            }

            current_size = prio_queue.size();
            u32 num_to_delete = distribution(generator) % current_size;

            for (u32 k = 0; k < num_to_delete; ++k) {

                auto const& prio_top = prio_queue.top();
                auto const& std_top = std_prio_queue.top();

                EXPECT_EQ(std_prio_queue.size(), prio_queue.size());
                EXPECT_EQ(std_top, prio_top);

                if (std_top != prio_top) {
                    printf("Blown!\n");
                }

                prio_queue.pop();
                std_prio_queue.pop();
            }
        }

        // Pop whatever is left in the priority queue
        while (!prio_queue.empty()) {

            auto const& prio_top = prio_queue.top();
            auto const& std_top = std_prio_queue.top();

            if (std_top != prio_top) {
                printf("Blown!\n");
            }

            EXPECT_EQ(std_prio_queue.size(), prio_queue.size());
            EXPECT_EQ(std_top, prio_top);

            prio_queue.pop();
            std_prio_queue.pop();
        }
    }
}

TYPED_TEST_P(PrioQueuePerfTest, PerfTest)
{
    typedef TypeParam PrioQueueT;

    PrioQueueT prio_queue;

    for (u64 j = 0; j < PERF_TEST_ITERATIONS; ++j) {

        for (u64 i = 0; i < PERF_TEST_TEST_SIZE; ++i) {
            prio_queue.push(i);
        }

        EXPECT_EQ(PERF_TEST_TEST_SIZE, prio_queue.size());

        for (u64 i = 0; i < PERF_TEST_TEST_SIZE; ++i) {
            prio_queue.pop();
        }

        EXPECT_EQ(0ul, prio_queue.size());
    }
}

REGISTER_TYPED_TEST_CASE_P(PrioQueueTest,
                           Constructor,
                           Functional);

REGISTER_TYPED_TEST_CASE_P(PrioQueuePerfTest,
                           PerfTest);

typedef Types<PriorityQueue<std::pair<i64, i64>,
                            i64i64PairCompare,
                            BinaryHeap<std::pair<i64, i64> > >,
              PriorityQueue<std::pair<i64, i64>,
                            i64i64PairCompare,
                            TernaryHeap<std::pair<i64, i64> > >,
              PriorityQueue<std::pair<i64, i64>,
                            i64i64PairCompare,
                            QuaternaryHeap<std::pair<i64, i64> > > >
PriorityQueueImplementations;

typedef Types<PriorityQueue<i64, std::less<i64>, BinaryHeap<i64> >,
              PriorityQueue<i64, std::less<i64>, TernaryHeap<i64> >,
              PriorityQueue<i64, std::less<i64>, QuaternaryHeap<i64> >,
              PriorityQueue<i64, std::less<i64>, MultiWayHeap<i64, std::less<i64>, 5> >,
              PriorityQueue<i64, std::less<i64>, MultiWayHeap<i64, std::less<i64>, 6> >,
              PriorityQueue<i64, std::less<i64>, MultiWayHeap<i64, std::less<i64>, 7> >,
              PriorityQueue<i64, std::less<i64>, MultiWayHeap<i64, std::less<i64>, 8> >,
              std::priority_queue<i64, std::vector<i64>, std::greater<i64> > >
PriorityQueuePerfImplementations;

INSTANTIATE_TYPED_TEST_CASE_P(PrioQueueTemplateTests,
                              PrioQueueTest, PriorityQueueImplementations);

INSTANTIATE_TYPED_TEST_CASE_P(PrioQueuePerfTests,
                              PrioQueuePerfTest, PriorityQueuePerfImplementations);

//
// PriorityQueueTests.cpp ends here
