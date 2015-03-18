// BitSet.hpp ---
// Filename: BitSet.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 02:10:18 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_BIT_SET_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_BIT_SET_HPP_

#include "../basetypes/KinaraTypes.hpp"

namespace kinara {
namespace containers {

class BitSet final
{
private:
    u64 m_num_bits;
    u08* m_bit_array;

    inline i32 compare(const BitSet& other) const;

public:
    class BitRef
    {
    private:
        BitSet* m_bit_set;
        u64 m_bit_num;

    public:
        BitRef(BitSet* bit_set, u64 bit_num);
        BitRef(const BitRef& other);
        ~BitRef();

        BitRef& operator = (const BitRef& other);
        BitRef& operator = (bool value);

        bool operator == (const BitRef& other) const;
        bool operator != (const BitRef& other) const;

        bool operator == (bool value) const;
        bool operator != (bool value) const;

        operator bool () const;
        bool operator ! () const;
    };

    BitSet();
    BitSet(u64 size);
    BitSet(u64 size, bool initial_value);
    BitSet(const BitSet& other);
    BitSet(BitSet&& other);

    ~BitSet();

    BitSet& operator = (const BitSet& other);
    BitSet& operator = (BitSet&& other);

    bool operator == (const BitSet& other) const;
    bool operator < (const BitSet& other) const;
    bool operator > (const BitSet& other) const;
    bool operator <= (const BitSet& other) const;
    bool operator >= (const BitSet& other) const;
    bool operator != (const BitSet& other) const;

    void set(u64 bit_num);
    bool test(u64 bit_num) const;
    void clear(u64 bit_num);
    // returns the value of bit before flip
    bool flip(u64 bit_num);

    // Gang set, clear and flip
    void set();
    void clear();
    void flip();

    // resets to a zero size bit set
    void reset();

    bool operator [] (u64 bit_num) const;
    BitRef operator [] (u64 bit_num);

    u64 size() const;
    void resize_and_clear(u64 new_num_bits);
    std::string to_string() const;
};

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_BIT_SET_HPP_ */

//
// BitSet.hpp ends here
