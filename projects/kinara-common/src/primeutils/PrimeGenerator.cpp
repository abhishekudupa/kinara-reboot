// PrimeGenerator.cpp ---
// Filename: PrimeGenerator.cpp
// Author: Abhishek Udupa
// Created: Tue Feb 17 18:22:09 2015 (-0500)
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

#include "../basetypes/KinaraTypes.hpp"

#include "PrimeGenerator.hpp"

namespace kinara {
namespace utils {

namespace kc = kinara::containers;

PrimeGenerator::PrimeGenerator(bool stateless)
    : m_is_stateless(stateless)
{
    if (!m_is_stateless) {
        m_primes.push_back(2);
    }
}

inline void PrimeGenerator::process_next_k(u64 k)
{
    kc::u64Vector work_list(k/2);
    auto first = m_primes.back() + 2;
    auto last = first + k;
    for (u64 i = first; i < last; i += 2) {
        work_list.push_back(i);
    }

    if (m_primes.size() > MaxPrimeListSize) {
        throw KinaraException("Exceeded maximum size for prime generator");
    }

    u64 next_prime_index = 0;
    while (work_list.size() > 0) {
        u64 num_primes = m_primes.size();
        for (; next_prime_index < num_primes; ++next_prime_index) {
            auto cur_prime = m_primes[next_prime_index];
            u64 num_work_elems = work_list.size();
            u64 next_work_list_index = 0;

            for (u64 i = 0; i < num_work_elems; ++i) {
                if (work_list[i] % cur_prime == 0) {
                    continue;
                }
                work_list[next_work_list_index] = work_list[i];
                next_work_list_index++;
            }
            work_list.resize(next_work_list_index);
            if (next_work_list_index == 0) {
                return;
            }
            if (cur_prime * cur_prime >= work_list.back()) {
                for (u64 i = 0; i < next_work_list_index; ++i) {
                    m_primes.push_back(work_list[i]);
                }
                return;
            }
        }

        auto cur_prime = m_primes.back();
        cur_prime = cur_prime * cur_prime;
        u64 work_list_index;
        for (work_list_index = 0; work_list[work_list_index] <= cur_prime; ++work_list_index) {
            m_primes.push_back(work_list[work_list_index]);
        }
        u64 j = 0;
        for (u64 i = work_list_index, end = work_list.size(); i < end; ++i, ++j) {
            work_list[j] = work_list[i];
        }
        work_list.resize(j);
    }
}

inline u64 PrimeGenerator::find_smallest_prime(u64 lower_bound)
{
    u64 low = 0;
    u64 high = m_primes.size() - 1;

    while (high - low > 1) {
        auto mid = low + (high - low) / 2;
        if (m_primes[mid] < lower_bound) {
            low = mid + 1;
        } else if (m_primes[mid] > lower_bound) {
            high = mid;
        } else {
            return lower_bound;
        }
    }
    // high - low <= 1
    if (m_primes[low] >= lower_bound) {
        return m_primes[low];
    } else {
        return m_primes[high];
    }
}

inline bool PrimeGenerator::is_prime(u64 candidate)
{
    for (u64 i = 3; i * i <= candidate; ++i) {
        if (candidate % i == 0) {
            return false;
        }
    }
    return true;
}

inline u64 PrimeGenerator::find_next_prime(u64 lower_bound)
{
    if (lower_bound % 2 == 0) {
        ++lower_bound;
    }
    for (auto candidate = lower_bound; candidate < UINT64_MAX; ++candidate) {
        if (is_prime(candidate)) {
            return candidate;
        }
    }
    throw KinaraException("Exceeded maximum size for prime generator");
}

u64 PrimeGenerator::get_next_prime(u64 lower_bound)
{
    if (m_is_stateless) {
        return find_next_prime(lower_bound);
    }
    if (m_primes.back() >= lower_bound) {
        return find_smallest_prime(lower_bound);
    } else {
        process_next_k(1024);
        if (m_primes.back() >= lower_bound) {
            return find_smallest_prime(lower_bound);
        } else {
            while (m_primes.back() < lower_bound) {
                process_next_k(16384);
            }
            return find_smallest_prime(lower_bound);
        }
    }
}

} /* end namespace utils */
} /* end namespace kinara */

//
// PrimeGenerator.cpp ends here
