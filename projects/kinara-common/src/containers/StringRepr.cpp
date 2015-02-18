// String.cpp ---
// Filename: String.cpp
// Author: Abhishek Udupa
// Created: Tue Feb 17 18:11:56 2015 (-0500)
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

#include <cmath>

#include "../hashfuncs/HashFunctions.hpp"
#include "../allocators/MemoryManager.hpp"

#include "String.hpp"

namespace kinara {
namespace containers {

namespace string_detail_ {

StringRepr* StringRepr::s_repr_hash_table = nullptr;
u64 StringRepr::s_repr_hash_table_size = (u64)0;
u64 StringRepr::s_repr_hash_table_used = (u64)0;
kinara::utils::PrimeGenerator StringRepr::s_prime_generator(true);

namespace ka = kinara::allocators;

void StringRepr::expand_hash_table()
{
    if (s_repr_hash_table_size > 0) {
        if (((float)s_repr_hash_table_size /
             (float)s_repr_hash_table_used) <
            s_hash_table_max_load_factor) {
            return;
        }

        // try a garbage collection first
        garbage_collect();
        if (((float)s_repr_hash_table_size /
             (float)s_repr_hash_table_used) <
            s_hash_table_max_load_factor) {
            return;
        }
    }

    // we need to resize the table
    u32 new_table_size = (u32)ceil(s_repr_hash_table_size * s_hash_table_resize_factor);
    new_table_size =
    auto new_hash_table = ka::casted_allocate_raw_cleared<StringRepr*>(new_table_size *
                                                                       sizeof(StringRepr*));
    for (u32 i = 0; i < s_repr_hash_table_size; ++i) {
        if (s_repr_hash_table[i] != s_hash_table_empty_value &&
            s_repr_hash_table[i] != s_hash_table_deleted_value) {

        }
    }
}

} /* end namespace string_detail_ */

} /* end namespace containers */
} /* end namespace kinara */

//
// String.cpp ends here
