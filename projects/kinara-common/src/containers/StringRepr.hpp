// StringRepr.hpp ---
// Filename: StringRepr.hpp
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_STRING_REPR_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_STRING_REPR_HPP_

#include <array>
#include <cstring>

#include "../basetypes/KinaraBase.hpp"
#include "../primeutils/PrimeGenerator.hpp"
#include "../hashfuncs/HashFunctions.hpp"

namespace kinara {
namespace containers {

namespace string_detail_ {

namespace ka = kinara::allocators;

// sizeof(StringRepr) = 40 bytes on all 64 bit platforms
class StringRepr
{
private:
    static constexpr u32 sc_max_compact = 32;

    struct FixedRepr {
        bool m_short_repr :  1;
        mutable i64 m_ref_count   : 63;

        inline FixedRepr(bool short_repr)
            : m_short_repr(short_repr), m_ref_count(0)
        {
            // Nothing here
        }

        inline FixedRepr()
            : m_short_repr(false), m_ref_count(0)
        {
            // Nothing here
        }
    };

    FixedRepr m_fixed_repr;

    struct LongRepr {
        char* m_data;
        // hash code INCLUDES the nul character
        u64 m_hashcode;
        // length WITHOUT the null character
        u64 m_length;
    };

    typedef std::array<char, sc_max_compact> ShortRepr;

    union ReprUnion {
        LongRepr m_long_repr;
        ShortRepr m_short_repr;

        inline ReprUnion()
        {
            // Nothing here
        }

        inline void initialize(const char* contents, u64 length)
        {
            // for the terminating nul character
            ++length;

            if (contents == nullptr || length == 0) {
                memset(m_short_repr.data(), 0, sc_max_compact);
                return;
            }

            if (length > sc_max_compact) {
                auto data = ka::casted_allocate_raw_cleared<char>(length);
                memcpy(data, contents, length);
                auto hashcode = kinara::utils::default_hash_function(data, length);
                m_long_repr.m_data = data;
                m_long_repr.m_hashcode = hashcode;
                m_long_repr.m_length = length - 1;
            } else {
                memset(m_short_repr.data(), 0, sc_max_compact);
                memcpy(m_short_repr.data(), contents, length);
            }
        }

        inline ReprUnion(const char* contents, u64 length)
        {
            initialize(contents, length);
        }

        inline ~ReprUnion()
        {
            // do nothing, it's not for me to decide.
        }
    };

    ReprUnion m_repr;

    StringRepr(const StringRepr& other) = delete;
    StringRepr(StringRepr&& other) = delete;

    StringRepr& operator = (const StringRepr& other) = delete;
    StringRepr& operator = (StringRepr&& other) = delete;
    i32 compare(const StringRepr& other) const;
    i32 compare(const char* other, u64 length) const;

public:
    StringRepr();
    StringRepr(const char* contents);
    StringRepr(const char* contents, u64 length);

    ~StringRepr();
    const char* c_str() const;
    bool operator == (const StringRepr& other) const;
    bool operator != (const StringRepr& other) const;
    bool operator < (const StringRepr& other) const;
    bool operator <= (const StringRepr& other) const;
    bool operator > (const StringRepr& other) const;
    bool operator >= (const StringRepr& other) const;

    // all lengths here are WITHOUT the nul character
    bool equals(const char* other, u64 length) const;
    bool nequals(const char* other, u64 length) const;
    bool lt(const char* other, u64 length) const;
    bool gt(const char* other, u64 length) const;
    bool le(const char* other, u64 length) const;
    bool ge(const char* other, u64 length) const;

    u64 hash() const;
    void inc_ref() const;
    void dec_ref() const;
    i64 get_ref_count() const;
    u64 size() const;
    u64 length() const;
    static StringRepr* make_repr(const char* contents);
    static StringRepr* make_repr(const char* contents, u64 length);
};

} /* end namespace string_detail_ */


} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_STRING_REPR_HPP_ */

//
// StringRepr.hpp ends here
