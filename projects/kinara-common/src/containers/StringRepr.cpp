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

#include "StringRepr.hpp"

namespace kinara {
namespace containers {

namespace string_detail_ {

StringRepr::StringRepr()
    : m_fixed_repr(true), m_repr(nullptr)
{
    // Nothing here
}

StringRepr::StringRepr(const char* contents)
{
    auto length = strlen(contents);
    m_fixed_repr.m_short_repr = (length < s_max_compact);
    m_repr.initialize(contents, length);
}

StringRepr::StringRepr(const char* contents, u64 length)
{
    m_fixed_repr.m_short_repr = (length < s_max_compact);
    m_repr.initialize(contents, length);
}

i32 StringRepr::compare(const char* other, u64 length)
{
    if (m_fixed_repr.m_short_repr) {
        auto length_mine = strlen(m_repr.m_short_repr);
        auto diff = length_mine - length;
        if (diff != 0) {
            return diff;
        }
        return memcmp(m_repr.m_short_repr, other, length);
    } else {
        auto length_mine = m_repr.m_long_repr.m_length;
        auto diff = length_mine - length;
        if (diff != 0) {
            return diff;
        }
        return memcmp(m_repr.m_long_repr.m_data, other, length);
    }
}

i32 StringRepr::compare(const StringRepr& other) const
{
    if (m_fixed_repr.m_short_repr && !(other.m_fixed_repr.m_short_repr)) {
        return -1;
    } else if (!(m_fixed_repr.m_short_repr) && other.m_fixed_repr.m_short_repr) {
        return 1;
    }
    // both are either short or long representations
    if (m_fixed_repr.m_short_repr) {
        auto length_mine = strlen(m_repr.m_short_repr.data());
        auto length_other = strlen(other.m_repr.m_short_repr.data());
        auto diff = length_mine - length_other;

        if (diff != 0) {
            return diff;
        }
        // lengths are equal
        return memcmp(m_repr.m_short_repr.data(),
                      other.m_repr.m_short_repr.data(),
                      length_mine);
    } else {
        auto diff = m_repr.m_long_repr.m_length - other.m_repr.m_long_repr.m_length;
        if (diff != 0) {
            return diff;
        }
        return memcmp(m_repr.m_long_repr.m_data, other.m_repr.m_long_repr.m_data,
                      m_repr.m_long_repr.m_length);
    }
}

StringRepr::~StringRepr()
{
    if (m_fixed_repr.m_short_repr) {
        return;
    } else {
        ka::deallocate_raw(m_repr.m_long_repr.m_data, m_repr.m_long_repr.m_length + 1);
    }
}

const char* StringRepr::c_str() const
{
    if (m_fixed_repr.m_short_repr) {
        return m_repr.m_short_repr.data();
    } else {
        return m_repr.m_long_repr.m_data;
    }
}

bool StringRepr::operator == (const StringRepr& other) const
{
    if (hash() != other.hash()) {
        return false;
    }
    return (compare(other) == 0);
}

bool StringRepr::operator != (const StringRepr& other) const
{
    if (hash() == other.hash()) {
        return (compare(other) != 0);
    }
    return true;
}

bool StringRepr::operator < (const StringRepr& other) const
{
    return (compare(other) < 0);
}

bool StringRepr::operator <= (const StringRepr& other) const
{
    return (compare(other) <= 0);
}

bool StringRepr::operator > (const StringRepr& other) const
{
    return (compare(other) > 0);
}

bool StringRepr::operator >= (const StringRepr& other) const
{
    return (compare(other) >= 0);
}

bool StringRepr::equals(const char* other, u64 length) const
{
    // include the nul character in the hash computation
    if (hash() != kinara::utils::default_hash_function(other, length + 1)) {
        return false;
    }
    return (compare(other, length) == 0);
}

bool StringRepr::nequals(const char *other, u64 length) const
{
    if (hash() == kinara::utils::default_hash_function(other, length + 1)) {
        return (compare(other, length) != 0);
    }
    return true;
}

bool StringRepr::lt(const char *other, u64 length) const
{
    return (compare(other, length) < 0)
}

bool StringRepr::le(const char *other, u64 length) const
{
    return (compare(other, length) <= 0)
}

bool StringRepr::gt(const char *other, u64 length) const
{
    return (compare(other, length) > 0)
}

bool StringRepr::ge(const char *other, u64 length) const
{
    return (compare(other, length) >= 0)
}

u64 StringRepr::hash() const
{
    if (m_fixed_repr.m_short_repr) {
        return kinara::utils::default_hash_function(m_repr.m_short_repr,
                                                    strlen(m_repr.m_short_repr) + 1);
    } else {
        return m_repr.m_long_repr.m_hashcode;
    }
}

void StringRepr::inc_ref() const
{
    m_fixed_repr.m_ref_count++;
}

void StringRepr::dec_ref() const
{
    m_fixed_repr.m_ref_count--;
}

i64 StringRepr::get_ref_count() const
{
    return m_fixed_repr.m_ref_count;
}

u64 StringRepr::size() const
{
    if (m_fixed_repr.m_short_repr) {
        return strlen(m_repr.m_short_repr);
    } else {
        return m_repr.m_long_repr.m_length;
    }
}

u64 StringRepr::length() const
{
    return size();
}

} /* end namespace string_detail_ */

} /* end namespace containers */
} /* end namespace kinara */

//
// String.cpp ends here
