// PrimeGeneratorTest.cpp ---
//
// Filename: PrimeGeneratorTest.cpp
// Author: Abhishek Udupa
// Created: Mon Mar 16 18:14:03 2015 (-0400)
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

#include "../../projects/kinara-common/src/primeutils/PrimeGenerator.hpp"
#include "../../thirdparty/gtest/include/gtest/gtest.h"
#include <random>

using kinara::utils::PrimeGenerator;

using kinara::u32;
using kinara::u64;

#define MAX_TEST_SIZE ((u64)(1 << 10))

TEST(PrimeGenerator, Functional)
{
    u64 cur_prime = 2;
    while (cur_prime < MAX_TEST_SIZE) {
        auto prime_from_stateless = PrimeGenerator::get_next_prime(cur_prime, true);
        auto prime_from_stateful = PrimeGenerator::get_next_prime(cur_prime);
        EXPECT_EQ(prime_from_stateless, prime_from_stateful);
        cur_prime = prime_from_stateless;
    }

    // generate random primes now
    PrimeGenerator::trim_table();
    std::default_random_engine generator;
    std::uniform_int_distribution<u32> distribution(0, 1 << 24);

    for (u64 i = 0; i < MAX_TEST_SIZE; ++i) {
        auto lower_bound = distribution(generator);
        auto prime_from_stateless = PrimeGenerator::get_next_prime(lower_bound, true);
        auto prime_from_stateful = PrimeGenerator::get_next_prime(lower_bound);
        EXPECT_EQ(prime_from_stateless, prime_from_stateful);
    }
}

//
// PrimeGeneratorTest.cpp ends here
