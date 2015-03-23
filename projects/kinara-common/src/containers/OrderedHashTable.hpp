// OrderedHashTable.hpp ---
//
// Filename: OrderedHashTable.hpp
// Author: Abhishek Udupa
// Created: Mon Mar 23 14:53:07 2015 (-0400)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_ORDERED_HASH_TABLE_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_ORDERED_HASH_TABLE_HPP_

#include "HashTable.hpp"
#include "Vector.hpp"

namespace kinara {
namespace containers {
namespace ordered_hash_table_detail_ {

namespace kc = kinara::containers;

template <typename T, typename HashFunction, typename EqualsFunction,
          typename CompareFunction,
          template <typename, typename, typename> HashTableImpl>
class OrderedHashTable : protected HashTableImpl<T, HashFunction, EqualsFunction>
{
private:
    typedef HashTableImpl<T, HashFunction, EqualsFunction> HashTableImplType;
    typedef HashTableImplType::EntryType HashTableImplEntryType;
    typedef HashTableImplType::Iterator HashTableImplIterator;

    class EntryCompareFunction : private CompareFunction
    {
    public:
        inline bool operator () (const HashTableImplEntryType* entry1,
                                 const HashTableImplEntryType* entry2) const
        {
            return CompareFunction::operator()(entry1->get_value_ref(), entry2->get_value_ref());
        }
    };

    vector<HashTableImplEntryType*> m_sorted_entries;

    inline void rebuild_sorted_entries()
    {
        m_sorted_entries.clear();
        for (auto it = HashTableImplType::begin(), last = HashTableImplType::end();
             it != last; ++it) {
            m_sorted_entries.push_back(it.get_current());
        }

        std::sort(m_sorted_entries.begin(), m_sorted_entries.end(), EntryCompareFunction());
    }

public:
    OrderedHashTable()
        : HashTableImplType()
    {
        // Nothing here
    }

    OrderedHashTable(const T& deleted_value, const T& nonused_value)
        : HashTableImplType(deleted_value, nonused_value)
    {
        // Nothing here
    }

    OrderedHashTable(u64 initial_capacity)
        : HashTableImplType(initial_capacity)
    {
        // Nothing here
    }

    OrderedHashTable(u64 initial_capacity, const T& deleted_value,
                     const T& nonused_value)
        : HashTableImplType(initial_capacity, deleted_value, nonused_value)
    {
        // Nothing here
    }

    OrderedHashTable(const OrderedHashTable& other)
        : HashTableImplType(other)
    {
        rebuild_sorted_entries();
    }

    OrderedHashTable(OrderedHashTable&& other)
        : HashTableImplType(std::move(other))
    {
        std::swap(m_sorted_entries, other.m_sorted_entries);
    }
};

} /* end namespace ordered_hash_table_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_ORDERED_HASH_TABLE_HPP_ */

//
// OrderedHashTable.hpp ends here
