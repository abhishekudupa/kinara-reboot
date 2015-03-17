// BitSet.cpp ---
//
// Filename: BitSet.cpp
// Author: Abhishek Udupa
// Created: Tue Mar 17 14:32:16 2015 (-0400)
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

#include <sstream>
#include <cstring>
#include <utility>
#include <string>

#include "../allocators/MemoryManager.hpp"

#include "BitSet.hpp"

namespace kinara {
namespace containers {

namespace ka = kinara::allocators;

// Implementation of BitRef
BitSet::BitRef::BitRef(BitSet* bit_set, u64 bit_num)
    : m_bit_set(bit_set), m_bit_num(bit_num)
{
    // Nothing here
}

BitSet::BitRef::BitRef(const BitRef& other)
    : BitRef(other.m_bit_set, other.m_bit_num)
{
    // Nothing here
}

BitSet::BitRef::~BitRef()
{
    m_bit_set = nullptr;
    m_bit_num = 0;
}

BitSet::BitRef& BitSet::BitRef::operator = (const BitRef& other)
{
    if (&other == this) {
        return *this;
    }
    m_bit_set = other.m_bit_set;
    m_bit_num = other.m_bit_num;
    return *this;
}

BitSet::BitRef& BitSet::BitRef::operator = (bool value)
{
    if (value) {
        m_bit_set->set(m_bit_num);
    } else {
        m_bit_set->clear(m_bit_num);
    }
    return *this;
}

bool BitSet::BitRef::operator == (const BitRef& other) const
{
    return ((bool)(*this) == (bool)(other));
}

bool BitSet::BitRef::operator != (const BitRef& other) const
{
    return ((bool)(*this) != (bool)(other));
}

bool BitSet::BitRef::operator == (bool value) const
{
    return ((bool)(*this) == value);
}

bool BitSet::BitRef::operator != (bool value) const
{
    return ((bool)(*this) != value);
}

BitSet::BitRef::operator bool() const
{
    if (m_bit_set == nullptr) {
        return false;
    }
    return m_bit_set->test(m_bit_num);
}

bool BitSet::BitRef::operator ! () const
{
    return (!((bool)(*this)));
}

// implementation of BitSet
BitSet::BitSet()
    : m_num_bits(0), m_bit_array(nullptr)
{
    // Nothing here
}

BitSet::BitSet(u64 size)
    : m_num_bits(size)
{
    u64 num_bytes = (m_num_bits + 7) / 8;
    m_bit_array = ka::casted_allocate_raw_cleared<u08>(sizeof(u08) * num_bytes);
}

BitSet::BitSet(u64 size, bool initial_value)
    : BitSet(size)
{
    if (initial_value) {
        memset(m_bit_array, 0xFF, size / 8);
        // take care of the last few bits.
        if (m_num_bits % 8 != 0) {
            u08 mask = 0;
            for (u32 i = 0; i < m_num_bits % 8; ++i) {
                mask >>= 1;
                mask |= (u08)0x80;
            }
            m_bit_array[m_num_bits / 8] |= mask;
        }
    }
}

BitSet::BitSet(const BitSet& other)
    : m_num_bits(other.m_num_bits)
{
    u64 num_bytes = (m_num_bits + 7) / 8;
    m_bit_array = ka::casted_allocate_raw_cleared<u08>(sizeof(u08) * num_bytes);
    memcpy(m_bit_array, other.m_bit_array, num_bytes);
}

BitSet::BitSet(BitSet&& other)
    : m_num_bits(0), m_bit_array(nullptr)
{
    std::swap(m_num_bits, other.m_num_bits);
    std::swap(m_bit_array, other.m_bit_array);
}

BitSet::~BitSet()
{
    u64 num_bytes = (m_num_bits + 7) / 8;
    ka::deallocate_raw(m_bit_array, num_bytes);
}

BitSet& BitSet::operator = (const BitSet& other)
{
    if (&other == this) {
        return *this;
    }

    if (m_bit_array != nullptr) {
        u64 old_num_bytes = (m_num_bits + 7) / 8;
        ka::deallocate_raw(m_bit_array, old_num_bytes);
    }

    m_num_bits = other.m_num_bits;
    u64 num_bytes = (m_num_bits + 7) / 8;
    m_bit_array = ka::casted_allocate_raw<u08>(sizeof(u08) * num_bytes);
    memcpy(m_bit_array, other.m_bit_array, num_bytes);
    return *this;
}

BitSet& BitSet::operator = (BitSet&& other)
{
    std::swap(m_num_bits, other.m_num_bits);
    std::swap(m_bit_array, other.m_bit_array);
    return *this;
}

i32 BitSet::compare(const BitSet& other) const
{
    i32 diff = m_num_bits - other.m_num_bits;
    if (diff != 0) {
        return diff;
    }
    return memcmp(m_bit_array, other.m_bit_array, (m_num_bits + 7) / 8);
}

bool BitSet::operator == (const BitSet& other) const
{
    return (compare(other) == 0);
}

bool BitSet::operator < (const BitSet& other) const
{
    return (compare(other) < 0);
}

bool BitSet::operator > (const BitSet& other) const
{
    return (compare(other) > 0);
}

bool BitSet::operator <= (const BitSet& other) const
{
    return (compare(other) <= 0);
}

bool BitSet::operator >= (const BitSet& other) const
{
    return (compare(other) >= 0);
}

bool BitSet::operator != (const BitSet& other) const
{
    return (compare(other) != 0);
}

void BitSet::set(u64 bit_num)
{
    u64 offset = bit_num / 8;
    const u64 bit_num_mod_8 = bit_num % 8;
    u08 mask = 0x80;
    if (bit_num_mod_8 != 0) {
        mask >>= (bit_num_mod_8);
    }
    m_bit_array[offset] |= mask;
}

void BitSet::clear(u64 bit_num)
{
    u64 offset = bit_num / 8;
    const u32 bit_num_mod_8 = bit_num % 8;
    u08 mask = 0x80;
    if (bit_num_mod_8 != 0) {
        mask >>= (bit_num_mod_8);
    }
    m_bit_array[offset] &= (~mask);
}

bool BitSet::test(u64 bit_num) const
{
    u64 offset = bit_num / 8;
    const u32 bit_num_mod_8 = bit_num % 8;
    u08 mask = 0x80;
    if (bit_num_mod_8 != 0) {
        mask >>= (bit_num_mod_8);
    }
    return (((m_bit_array[offset] & mask) == 0) ? false : true);
}

bool BitSet::flip(u64 bit_num)
{
    u32 offset = bit_num / 8;
    const u32 bit_num_mod_8 = bit_num % 8;
    u08 mask = 0x80;
    if (bit_num_mod_8 != 0) {
        mask >>= (bit_num_mod_8);
    }
    bool retval = (((m_bit_array[offset] & mask) == 0) ? false : true);
    if (retval) {
        m_bit_array[offset] &= (~mask);
    } else {
        m_bit_array[offset] |= mask;
    }
    return retval;
}

void BitSet::set()
{
    memset(m_bit_array, 0xFF, m_num_bits / 8);
    // take care of the last few bits.
    if (m_num_bits % 8 != 0) {
        u32 mask = 0;
        for (u32 i = 0; i < m_num_bits % 8; ++i) {
            mask >>= 1;
            mask |= (u08)0x80;
        }
        m_bit_array[m_num_bits / 8] |= mask;
    }
}

void BitSet::clear()
{
    memset(m_bit_array, 0, (m_num_bits + 7) / 8);
}

void BitSet::flip()
{
    for (u64 i = 0; i < m_num_bits / 8; ++i) {
        m_bit_array[i] = (~(m_bit_array[i]));
    }
    // take care of the last few bits with
    // explicit calls
    for (u32 i = 0; i < m_num_bits % 8; ++i) {
        flip(((m_num_bits / 8) * 8) + i);
    }
}

bool BitSet::operator [] (u64 bit_num) const
{
    return test(bit_num);
}

BitSet::BitRef BitSet::operator [] (u64 bit_num)
{
    return BitRef(this, bit_num);
}

u64 BitSet::size() const
{
    return m_num_bits;
}

std::string BitSet::to_string() const
{
    std::ostringstream sstr;
    sstr << "{\n";
    for (u64 i = 0; i < m_num_bits; ++i) {
        if (test(i)) {
            sstr << " " << i << " -> 1";
        } else {
            sstr << " " << i << " -> 0";
        }
    }
    sstr << " }";
    return sstr.str();
}

void BitSet::resize_and_clear(u64 new_num_bits)
{
    if (new_num_bits == m_num_bits) {
        clear();
    }
    if (new_num_bits == 0) {
        ka::deallocate_raw(m_bit_array, (m_num_bits + 7 / 8));
        m_bit_array = nullptr;
        m_num_bits = 0;
    }

    auto new_num_bytes = (new_num_bits + 7) / 8;
    auto new_bit_array = ka::casted_allocate_raw_cleared<u08>(new_num_bytes * sizeof(u08));
    ka::deallocate_raw(m_bit_array, (m_num_bits + 7) / 8);
    m_num_bits = new_num_bits;
    m_bit_array = new_bit_array;
}

} /* end namespace containers */
} /* end namespace kinara */

//
// BitSet.cpp ends here
