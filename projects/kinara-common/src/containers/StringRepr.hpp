// String.hpp ---
// Filename: String.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 02:08:35 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_STRING_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_STRING_HPP_

#include <array>

#include "../basetypes/KinaraBase.hpp"
#include "../primeutils/PrimeGenerator.hpp"

namespace kinara {
namespace containers {

namespace string_detail_ {

namespace ka = kinara::allocators;

// sizeof(StringRepr) = 40 bytes on all 64 bit platforms
class StringRepr
{
private:
    static constexpr u32 s_max_short = 24;
    static constexpr float s_hash_table_resize_factor = 1.618;
    static constexpr float s_hash_table_max_load_factor = 0.7;
    static constexpr StringRepr* s_hash_table_deleted_value =
        reinterpret_cast<StringRepr*>(0x1);
    static constexpr StringRepr* s_hash_table_empty_value =
        reinterpret_cast<StringRepr*>(nullptr);

    static StringRepr* s_repr_hash_table;
    static u64 s_repr_hash_table_size;
    static u64 s_repr_hash_table_used;
    static kinara::utils PrimeGenerator s_prime_generator;

    u64 m_size;
    i64 m_ref_count;
    union ReprUnion {
        std::array<char, m_max_compact> m_short_repr;
        char* m_long_repr;
    };
    ReprUnion m_repr;

    static inline void expand_hash_table();
    static inline void garbage_collect();

public:
    static StringRepr* make_repr(const char* contents);
};

} /* end namespace string_detail_ */

class String
{
private:
    StringRepr* m_the_repr;

public:
    String(const char* contents);
    ~String();

    const char* c_str() const;
};

inline operator String () (const char* char_data)
{
    return String(char_data);
}

inline std::ostream& operator << (std::ostream& out_stream,
                                  const String& the_string)
{
    out << the_string.c_str() << endl;
}

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_STRING_HPP_ */

//
// String.hpp ends here
