// PrimeGenerator.hpp ---
// Filename: PrimeGenerator.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 13:59:20 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_PRIMEUTILS_PRIME_GENERATOR_HPP_
#define KINARA_KINARA_COMMON_PRIMEUTILS_PRIME_GENERATOR_HPP_

#include "../basetypes/KinaraBase.hpp"
#include "../containers/Vector.hpp"

namespace kinara {
namespace utils {

namespace kc = kinara::containers;

class PrimeGenerator
{
private:
    static constexpr u64 sc_max_prime_list_size = ((u64)1 << 26);

    kc::u64Vector m_primes;
    bool m_is_stateless;

    inline void process_next_k(u64 k);
    inline u64 find_smallest_prime(u64 lower_bound);
    inline u64 find_next_prime(u64 lower_bound);
    bool is_prime(u64 candidate);

public:
    PrimeGenerator(bool stateless);
    ~PrimeGenerator();

    // gets the next prime greater than or equal to lower_bound
    u64 get_next_prime(u64 lower_bound);
};

} /* end namespace utils */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_PRIMEUTILS_PRIME_GENERATOR_HPP_ */

//
// PrimeGenerator.hpp ends here
