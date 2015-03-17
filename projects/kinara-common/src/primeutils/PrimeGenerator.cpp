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

#include "PrecomputedPrimeList.hpp"
#include "PrimeGenerator.hpp"

namespace kinara {
namespace utils {

namespace kc = kinara::containers;
namespace ka = kinara::allocators;

inline kc::u64Vector*& PrimeGenerator::get_prime_table()
{
    // TODO: fix up with a precomputed list of initial primes
    static kc::u64Vector* s_prime_table = nullptr;

    if (s_prime_table == nullptr) {
        s_prime_table = ka::allocate_object_raw<kc::u64Vector>();
        for(u64 i = 0; i < sc_init_pull_size; ++i) {
            s_prime_table->push_back(precomputed_primes::precomputed_prime_list[i]);
        }
    }

    return s_prime_table;
}

inline void PrimeGenerator::process_next_k(u64 k)
{
    auto& prime_table = *(get_prime_table());
    auto init_table_size = prime_table.size();

    while (k > 0 && init_table_size < precomputed_primes::num_precomputed_primes) {
        prime_table.push_back(precomputed_primes::precomputed_prime_list[init_table_size]);
        ++init_table_size;
        --k;
    }

    kc::u64Vector work_list(k/2 + 1);
    auto first = prime_table.back() + 2;
    auto last = first + k;
    u64 tmp_index = 0;
    for (u64 i = first; i < last; i += 2) {
        work_list[tmp_index++] = i;
    }
    work_list.resize(tmp_index);

    if (prime_table.size() > sc_max_prime_list_size) {
        throw KinaraException("Exceeded maximum size for prime generator");
    }

    u64 next_prime_index = 1;
    while (work_list.size() > 0) {
        u64 nuprime_table = prime_table.size();
        for (; next_prime_index < nuprime_table; ++next_prime_index) {
            auto cur_prime = prime_table[next_prime_index];
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
                    prime_table.push_back(work_list[i]);
                }
                return;
            }
        }

        auto cur_prime = prime_table.back();
        cur_prime = cur_prime * cur_prime;
        u64 work_list_index;
        for (work_list_index = 0; work_list[work_list_index] <= cur_prime; ++work_list_index) {
            prime_table.push_back(work_list[work_list_index]);
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
    auto& prime_table = *(get_prime_table());
    u64 low = 0;
    u64 high = prime_table.size() - 1;

    while (high - low > 1) {
        auto mid = low + (high - low) / 2;
        if (prime_table[mid] < lower_bound) {
            low = mid + 1;
        } else if (prime_table[mid] > lower_bound) {
            high = mid;
        } else {
            return lower_bound;
        }
    }
    // high - low <= 1
    if (prime_table[low] >= lower_bound) {
        return prime_table[low];
    } else {
        return prime_table[high];
    }
}

inline bool PrimeGenerator::is_prime(u64 candidate)
{
    auto& prime_table = *(get_prime_table());
    // go through the list of primes in the prime_table
    // first anyway.
    for (auto const& prime : prime_table) {
        if (candidate % prime == 0) {
            return false;
        }
        if (prime * prime > candidate) {
            return true;
        }
    }

    for (u64 i = prime_table.back() + 2; i * i <= candidate; i += 2) {
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
    for (auto candidate = lower_bound; candidate < UINT64_MAX; candidate += 2) {
        if (is_prime(candidate)) {
            return candidate;
        }
    }
    throw KinaraException("Exceeded maximum size for prime generator");
}

u64 PrimeGenerator::get_next_prime(u64 lower_bound, bool stateless)
{
    ++lower_bound;
    if (stateless) {
        return find_next_prime(lower_bound);
    }

    auto& prime_table = *(get_prime_table());

    if (prime_table.back() >= lower_bound) {
        return find_smallest_prime(lower_bound);
    } else {

        // revert to being stateless if we've reached
        // the maximum list size
        if (prime_table.size() >= sc_max_prime_list_size) {
            return find_next_prime(lower_bound);
        }

        process_next_k(1024);

        if (prime_table.size() >= sc_max_prime_list_size) {
            prime_table.shrink_to_fit();
            return find_next_prime(lower_bound);
        }

        if (prime_table.back() >= lower_bound) {
            return find_smallest_prime(lower_bound);
        } else {
            while (prime_table.back() < lower_bound) {
                process_next_k(16384);
                if (prime_table.size() >= sc_max_prime_list_size) {
                    prime_table.shrink_to_fit();
                    return find_next_prime(lower_bound);
                }
            }
            return find_smallest_prime(lower_bound);
        }
    }
}

void PrimeGenerator::trim_table()
{
    auto& prime_table = *(get_prime_table());
    prime_table.clear();
    prime_table.push_back(2);
    prime_table.push_back(3);
}

void PrimeGenerator::finalize()
{
    auto& prime_table_ptr = get_prime_table();
    ka::deallocate_object_raw(prime_table_ptr, sizeof(kc::u64Vector));
    prime_table_ptr = nullptr;
}

__attribute__ ((destructor)) void finalize_prime_generator()
{
    PrimeGenerator::finalize();
}

} /* end namespace utils */
} /* end namespace kinara */

//
// PrimeGenerator.cpp ends here
