// ContainersBase.hpp ---
//
// Filename: ContainersBase.hpp
// Author: Abhishek Udupa
// Created: Thu Feb 12 17:38:57 2015 (-0500)
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

#if !defined KINARA_CONTAINERS_CONTAINERS_BASE_HPP_
#define KINARA_CONTAINERS_CONTAINERS_BASE_HPP_

#include <cmath>
#include <ostream>

#include "../basetypes/KinaraBase.hpp"

namespace kinara {
namespace containers {


template <typename ForwardIterator>
static inline void print_iterable(std::ostream& out_stream,
                                  const ForwardIterator& begin_iterator,
                                  const ForwardIterator& end_iterator)
{
    for (auto it = begin_iterator; it != end_iterator; ++it) {
        out_stream << *it;
        if (next(it) != end_iterator) {
            std::operator<<(out_stream, ", ");
        }
    }
    return;
}

namespace detail {

template <u64 INCREMENT=8>
struct AdditiveIncrementer
{
    inline u64 operator () (u64 current_size) const
    {
        return current_size + INCREMENT;
    }
};

template <u32 NUMERATOR=3, u32 DENOMINATOR=2, u32 INITIALSIZE=8>
struct MultiplicativeIncrementer
{
    inline u64 operator () (u64 current_size) const
    {
        if (current_size == 0) {
            return INITIALSIZE;
        }
        float factor = (float)NUMERATOR / (float)DENOMINATOR;
        return (u64)(ceil((float)current_size * factor));
    }
};

} /* end namespace detail */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_CONTAINERS_CONTAINERS_BASE_HPP_ */

//
// ContainersBase.hpp ends here
