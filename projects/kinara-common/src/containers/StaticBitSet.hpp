// StaticBitSet.hpp ---
// Filename: StaticBitSet.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 02:10:14 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_STATIC_BIT_SET_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_STATIC_BIT_SET_HPP_

#include <cstring>
#include <utility>
#include <string>
#include <sstream>

#include "../basetypes/KinaraBase.hpp"
#include "../allocators/MemoryManager.hpp"

namespace kinara {
namespace containers {

namespace ka = kinara::allocators;

template <u64 SIZE>
class StaticBitSet final
{
private:
    static constexpr u64 sc_num_bits = SIZE;
    static constexpr u64 sc_num_bytes = (SIZE + 7) / 8;
    u08 m_bit_array[sc_num_bytes];

public:
    class BitRef
    {
    private:
        StaticBitSet* m_bit_set;
        u64 m_bit_num;

    public:
        inline BitRef(StaticBitSet* bit_set, u64 bit_num)
            : m_bit_set(bit_set), m_bit_num(bit_num)
        {
            // Nothing here
        }

        inline BitRef(const BitRef& other)
            : m_bit_set(other.m_bit_set), m_bit_num(other.m_bit_num)
        {
            // Nothing here
        }

        inline ~BitRef()
        {
            // Nothing here
        }

        inline BitRef& operator = (const BitRef& other)
        {
            if (&other == this) {
                return *this;
            }
            m_bit_set = other.m_bit_set;
            m_bit_num = other.m_bit_num;
            return *this;
        }

        inline BitRef& operator = (bool value)
        {
            if (value) {
                m_bit_set->set(m_bit_num);
            } else {
                m_bit_set->clear(m_bit_num);
            }
            return *this;
        }

        inline bool operator == (const BitRef& other) const
        {
            return ((bool)(*this) == (bool)(other));
        }

        inline bool operator != (const BitRef& other) const
        {
            return ((bool)(*this) == (bool)(other));
        }

        inline bool operator == (bool value) const
        {
            return ((bool)(*this) == value);
        }

        inline bool operator != (bool value) const
        {
            return ((bool)(*this) != value);
        }

        operator bool () const
        {
            if (m_bit_set == nullptr) {
                return false;
            }
            return m_bit_set->test(m_bit_num);
        }

        bool operator ! () const
        {
            return (!((bool)(*this)));
        }
    };

    inline StaticBitSet()
        : StaticBitSet(false)
    {
        // Nothing here
    }

    inline StaticBitSet(bool initial_value)
    {
        memset(m_bit_array, (initial_value ? 0xFF : 0), sc_num_bits / 8);
        if (initial_value) {
            if (sc_num_bits % 8 != 0) {
                u08 mask = 0;
                for (u32 i = 0; i < sc_num_bits % 8; ++i) {
                    mask >>= 1;
                    mask |= (u08)0x80;
                }
                m_bit_array[sc_num_bits / 8] |= mask;
            }
        }
    }

    inline StaticBitSet(const StaticBitSet& other)
    {
        memcpy(m_bit_array, other.m_bit_array, sc_num_bytes);
    }

    inline ~StaticBitSet()
    {
        // Nothing here
    }

    inline StaticBitSet& operator = (const StaticBitSet& other)
    {
        if (&other == this) {
            return *this;
        }
        memcpy(m_bit_array, other.m_bit_array, sc_num_bytes);
        return *this;
    }


    inline bool operator == (const StaticBitSet& other) const
    {
        return (memcmp(m_bit_array, other.m_bit_array, sc_num_bytes) == 0);
    }

    inline bool operator != (const StaticBitSet& other) const
    {
        return (memcmp(m_bit_array, other.m_bit_array, sc_num_bytes) != 0);
    }

    inline bool operator < (const StaticBitSet& other) const
    {
        return (memcmp(m_bit_array, other.m_bit_array, sc_num_bytes) < 0);
    }

    inline bool operator > (const StaticBitSet& other) const
    {
        return (memcmp(m_bit_array, other.m_bit_array, sc_num_bytes) > 0);
    }

    inline bool operator <= (const StaticBitSet& other) const
    {
        return (memcmp(m_bit_array, other.m_bit_array, sc_num_bytes) <= 0);
    }

    inline bool operator >= (const StaticBitSet& other) const
    {
        return (memcmp(m_bit_array, other.m_bit_array, sc_num_bytes) >= 0);
    }

    inline void set(u64 bit_num)
    {
        u64 offset = bit_num / 8;
        const u64 bit_num_mod_8 = bit_num % 8;
        u08 mask = 0x80;
        if (bit_num_mod_8 != 0) {
            mask >>= (bit_num_mod_8);
        }
        m_bit_array[offset] |= mask;
    }

    inline bool test(u64 bit_num) const
    {
        u64 offset = bit_num / 8;
        const u32 bit_num_mod_8 = bit_num % 8;
        u08 mask = 0x80;
        if (bit_num_mod_8 != 0) {
            mask >>= (bit_num_mod_8);
        }
        return (((m_bit_array[offset] & mask) == 0) ? false : true);
    }

    inline void clear(u64 bit_num)
    {
        u64 offset = bit_num / 8;
        const u32 bit_num_mod_8 = bit_num % 8;
        u08 mask = 0x80;
        if (bit_num_mod_8 != 0) {
            mask >>= (bit_num_mod_8);
        }
        m_bit_array[offset] &= (~mask);
    }

    // returns the value of bit before flip
    inline bool flip(u64 bit_num)
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

    // Gang set, clear and flip
    inline void set()
    {
        memset(m_bit_array, 0xFF, sc_num_bits / 8);
        // take care of the last few bits.
        if (sc_num_bits % 8 != 0) {
            u32 mask = 0;
            for (u32 i = 0; i < sc_num_bits % 8; ++i) {
                mask >>= 1;
                mask |= (u08)0x80;
            }
            m_bit_array[sc_num_bits / 8] |= mask;
        }
    }

    inline void clear()
    {
        memset(m_bit_array, 0, (sc_num_bits + 7) / 8);
    }

    inline void flip()
    {
        for (u64 i = 0; i < sc_num_bits / 8; ++i) {
            m_bit_array[i] = (~(m_bit_array[i]));
        }
        // take care of the last few bits with
        // explicit calls
        for (u32 i = 0; i < sc_num_bits % 8; ++i) {
            flip(((sc_num_bits / 8) * 8) + i);
        }
    }

    inline bool operator [] (u64 bit_num) const
    {
        return test(bit_num);
    }

    inline BitRef operator [] (u64 bit_num)
    {
        return BitRef(this, bit_num);
    }

    inline u64 size() const
    {
        return sc_num_bits;
    }

    inline std::string to_string() const
    {
        std::ostringstream sstr;
        sstr << "{\n";
        for (u64 i = 0; i < sc_num_bits; ++i) {
            if (test(i)) {
                sstr << " " << i << " -> 1";
            } else {
                sstr << " " << i << " -> 0";
            }
        }
        sstr << " }";
        return sstr.str();
    }
};

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_STATIC_BIT_SET_HPP_ */

//
// StaticBitSet.hpp ends here
