// MultiWayHeap.hpp ---
//
// Filename: MultiWayHeap.hpp
// Author: Abhishek Udupa
// Created: Fri Mar 13 16:45:59 2015 (-0400)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_MULTI_WAY_HEAP_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_MULTI_WAY_HEAP_HPP_

#include "Vector.hpp"

namespace kinara {
namespace containers {
namespace multiway_heap_detail_ {

template <typename Key, typename Value, u32 ARITY>
MultiWayHeap
{
private:

    struct HeapEntry
    {
        Key m_key;
        Value m_value;

        HeapEntry()
            : m_key(), m_value()
        {
            // Nothing here
        }

        HeapEntry(const HeapEntry& other)
            : m_key(other.m_key), m_value(other.m_value)
        {
            // Nothing here
        }

        HeapEntry(HeapEntry&& other)
            : HeapEntry()
        {
            std::swap(m_key, other.m_key);
            std::swap(m_value, other.m_value);
        }

        ~HeapEntry()
        {
            // Nothing here
        }

        inline HeapEntry& operator = (const HeapEntry& other)
        {
            if (&other == this) {
                return *this;
            }
            m_key = other.m_key;
            m_value = other.m_value;
            return *this;
        }

        inline HeapEntry& operator = (HeapEntry&& other)
        {
            if (&other == this) {
                return *this;
            }
            std::swap(m_key, other.m_key);
            std::swap(m_value, other.m_value);
        }
    };

    Vector<HeapEntry> m_data;
    const Key m_infimum;
    const Key m_supremum;

public:
    inline MultiWayHeap(const Key& infimum, const Key& supremum)
        : m_data(), m_infimum(infimum), m_supremum(supremum)
    {
        // Nothing here
    }

    inline MultiWayHeap()
        : m_data(), m_infimum(get_heap_infimum<Key>()),
          m_supremum(get_heap_supremum<Key>())
    {
        // Nothing here
    }

    inline ~MultiWayHeap()
    {
        // Nothing here
    }

    inline u64 get_size() const
    {
        return (m_data.size())
    }

    inline void get_min(Key& key, Value& value) const
    {
        key = m_data[0].key;
        value = m_data[0].value;
    }

    inline void delete_min()
    {

    }

    inline void insert(const Key& key, const Value& value)
    {
    }
};

} /* end namespace multiway_heap_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_MULTI_WAY_HEAP_HPP_ */

//
// MultiWayHeap.hpp ends here
