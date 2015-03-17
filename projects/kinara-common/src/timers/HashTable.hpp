// HashTable.hpp ---
//
// Filename: HashTable.hpp
// Author: Abhishek Udupa
// Created: Mon Mar 16 10:56:19 2015 (-0400)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_HASH_TABLE_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_HASH_TABLE_HPP_

#include "../basetypes/KinaraBase.hpp"
#include "../basetypes/KinaraErrors.hpp"
#include "../primeutils/PrimeGenerator.hpp"
#include "../allocators/MemoryManager.hpp"
#include "../hashfuncs/Hash.hpp"

#include "HashTableTypes.hpp"

namespace kinara {
namespace containers {
namespace hashtable_detail_ {

namespace ka = kinara::allocators;
namespace kc = kinara::containers;
namespace ku = kinara::utils;

// Maintains a simple open-addressed hash table
template <typename T, typename HashFunction = ku::Hasher<T>,
          typename EqualsFunction = std::equal_to<T> >
class HashTable
{
private:
    static constexpr float sc_resize_factor = 1.618f;
    static constexpr float sc_max_load_factor = 0.7f;
    static constexpr float sc_min_load_factor = 0.05f;
    static constexpr float sc_allocator_utilization_low = 0.1f;
    static constexpr float sc_deleted_nonused_rehash_ratio = 0.5f;
    static constexpr u64 sc_initial_table_size = 19;

    typedef HashEntry<T> TableEntry;

    TableEntry* m_table;
    u64 m_table_size;
    u64 m_size;
    u64 m_num_deleted_slots;

    inline TableEntry* allocate_table(u64 table_size)
    {
        // we reserve the first and last element of the table
        // as sentinels
        auto actual_table_size = ku::PrimeGenerator::get_next_prime(table_size + 2);
        m_table = ka::allocate_array<TableEntry>(actual_table_size);

        // mark the sentinels
        m_table[0].mark_in_use();
        m_table[actual_table_size - 1].mark_in_use();

        m_table_size = actual_table_size - 2;
        return m_table;
    }

    inline void deallocate_table()
    {
        if (m_table != nullptr) {
            ka::deallocate_array_raw(m_table, m_table_size + 2);
            m_table = nullptr;
        }
    }

    inline void expand_table(u64 required_capacity)
    {
        u64 current_capacity = m_table_size;
        float new_utilization;
        if (current_capacity > 0) {
            new_utilization = (float)required_capacity / (float)current_capacity;
        } else {
            new_utilization = 1.0f;
        }
        if (new_utilization < sc_max_load_factor) {
            return;
        }

        // rebuild the table
        auto new_table_size = (u64)ceil((float)required_capacity * sc_resize_factor);
        new_table_size = std::max(new_table_size, required_capacity + 3);
        new_table_size = ku::PrimeGenerator::get_next_prime(new_table_size);

        auto new_table = ka::allocate_array_raw<TableEntry>(new_table_size + 2);
        new_table[0].mark_in_use();
        new_table[new_table_size + 1].mark_in_use();

        rebuild_table(new_table, new_table_size);
    }

    // precondition: true
    // ensures: the_hash_table() contains at least 2 empty slots
    // ensures min_utilization <= utilization factor < max_utilization
    inline void expand_table()
    {
        expand_table(m_size + 1);
    }

    inline void rebuild_table(TableEntry* new_table, u64 new_table_size);
    {
        auto old_table = m_table;
        auto old_table_size = m_table_size;

        for (u64 i = 1; i < old_table_size + 1; ++i) {
            auto& entry = old_table[i];
            if (entry.is_nonused() || entry.is_deleted()) {
                continue;
            }
            move_into_table(std::move(entry), new_table, new_table_size);
        }

        ka::deallocate_array_raw(old_table, old_table_size);
        m_table_size = new_table_size;
        m_num_deleted_slots = 0;
    }

public:
    inline HashTable()
        : m_table(nullptr), m_table_size(0), m_size(0)
    {
        // Nothing here
    }

    inline HashTable(u64 table_size)
        : m_table(allocate_table(table_size)), m_size(0)
    {
        // Nothing here
    }

    inline ~HashTable()
    {
        deallocate_table();
    }

    inline Iterator before_begin() const
    {
        if (m_table == nullptr) {
            return Iterator();
        }
        return Iterator(m_table);
    }

    inline Iterator begin() const
    {
        if (m_table == nullptr) {
            return Iterator();
        }
        Iterator retval(m_table);
        return (++retval);
    }

    inline Iterator end() const
    {
        if (m_table == nullptr) {
            return Iterator();
        }
        return Iterator(m_table + m_table_size + 1);
    }

    inline Iterator find(const T& value)
    {
        if (m_table == nullptr) {
            return Iterator();
        }

        HashFunction hash_function;
        EqualsFunction equals_function;

        u64 h1 = 1 + (hash_function(value) % m_table_size);
        u64 h2 = 1 + (((h1 << 17) ^ h1) % (m_table_size - 1));

        u64 num_probes = 0;
        u64 index = h1;
        TableEntry* cur_entry = &(m_table[index]);

        while (!(cur_entry->is_nonused()) && num_probes < m_table_size - 2) {
            ++num_probes;
            if (!(cur_entry->is_deleted()) && equals_function(cur_entry->get_value(), value)) {
                return Iterator(cur_entry);
            }

            // rehash
            index = 1 + ((index + h2) % m_table_size);
            cur_entry = &(m_table[index]);
        }

        return end();
    }

    inline Iterator insert(const T& value, bool& already_present)
    {
        auto it = find(value);
        if (it != end()) {
            already_present = true;
            return it;
        }

        already_present = false;
        expand_table();

        HashFunction hash_function;
        EqualsFunction equals_function;

        u64 h1 = 1 + (hash_function(value) % m_table_size);
        u64 h2 = 1 + (((h1 << 17) ^ h1) % (m_table_size - 1));

        u64 index = h1;
        TableEntry* cur_entry = &(m_table[index]);

        while (!(cur_entry->is_nonused()) && !(cur_entry->is_deleted())) {
            index = (index + h2) % m_table_size
        }
    }

};

} /* end namespace hashtable_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_HASH_TABLE_HPP_ */

//
// HashTable.hpp ends here
