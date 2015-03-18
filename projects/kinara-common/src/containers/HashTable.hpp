// HashTable.hpp ---
// Filename: HashTable.hpp
// Author: Abhishek Udupa
// Created: Tue Mar 17 22:24:35 2015 (-0400)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_HASH_TABLE_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_HASH_TABLE_HPP_

#include "../basetypes/KinaraBase.hpp"
#include "../allocators/MemoryManager.hpp"
#include "../primeutils/PrimeGenerator.hpp"

namespace kinara {
namespace containers {
namespace hash_table_detail_ {

namespace ka = kinara::allocators;
namespace ku = kinara::utils;

class HashTableBase
{
public:
    static constexpr float sc_resize_factor = 1.618f;
    static constexpr float sc_max_load_factor = 0.717f;
    static constexpr float sc_min_load_factor = 0.1f;
    static constexpr float sc_deleted_nonused_rehash_ratio = 0.5f;
    static constexpr u64 sc_initial_table_size = 19;
};

template <typename T>
class UnifiedHashTableEntry
{
private:
    static constexpr u08 sc_nonused_marker = 0x0;
    static constexpr u08 sc_deleted_marker = 0x1;
    static constexpr u08 sc_is_used_marker = 0x2;

    u08 m_status_marker;
    T m_value;

public:
    inline UnifiedHashTableEntry()
        : m_status_marker(0), m_value()
    {
        // Nothing here
    }

    inline UnifiedHashTableEntry(const T& value)
        : m_status_marker(0), m_value(value)
    {
        // Nothing here
    }

    inline UnifiedHashTableEntry(T&& value)
        : m_status_marker(0), m_value(std::move(value))
    {
        // Nothing here
    }

    template <typename... ArgTypes>
    inline UnifiedHashTableEntry(ArgTypes&&... args)
        : m_status_marker(0), m_value(std::forward<ArgTypes>(args)...)
    {
        // Nothing here
    }

    inline UnifiedHashTableEntry(const UnifiedHashTableEntry& other)
        : m_status_marker(other.m_status_marker), m_value(other.m_value)
    {
        // Nothing here
    }

    inline UnifiedHashTableEntry(UnifiedHashTableEntry&& other)
        : m_status_marker(other.m_status_marker), m_value(std::move(other.m_value))
    {
        // Nothing here
    }

    inline ~UnifiedHashTableEntry()
    {
        // Nothing here
    }

    inline UnifiedHashTableEntry& operator = (const UnifiedHashTableEntry& other)
    {
        if (&other == this) {
            return *this;
        }
        m_status_marker = other.m_status_marker;
        m_value = other.m_value;
        return *this;
    }

    inline UnifiedHashTableEntry& operator = (UnifiedHashTableEntry&& other)
    {
        if (&other == this) {
            return *this;
        }
        std::swap(m_status_marker, other.m_status_marker);
        std::swap(m_value, other.m_value);
        return *this;
    }

    inline UnifiedHashTableEntry& operator = (const T& value)
    {
        m_value = value;
        return *this;
    }

    inline UnifiedHashTableEntry& operator = (T&& value)
    {
        m_value = std::move(value);
        return *this;
    }

    inline bool is_nonused() const
    {
        return (m_status_marker == sc_nonused_marker);
    }

    inline bool is_deleted() const
    {
        return (m_status_marker == sc_deleted_marker);
    }

    inline bool is_used() const
    {
        return (m_status_marker == sc_is_used_marker);
    }

    inline void mark_nonused()
    {
        m_status_marker = sc_nonused_marker;
    }

    inline void mark_deleted()
    {
        m_status_marker = sc_deleted_marker;
    }

    inline void mark_used()
    {
        m_status_marker = sc_is_used_marker;
    }

    inline T& get_value_ref()
    {
        return m_value;
    }

    inline const T& get_value_ref() const
    {
        return m_value;
    }

    inline T* get_value_ptr()
    {
        return &m_value;
    }

    inline T* get_value_ptr() const
    {
        return &m_value;
    }
};

// A hash table of unified hash entries
template <typename T, typename HashFunction, typename EqualsFunction>
class UnifiedHashTable final : private HashTableBase
{
    friend class Iterator;

private:
    typedef UnifiedHashTableEntry<T> EntryType;

    EntryType* m_table;
    u64 m_table_size;
    u64 m_table_used;
    u64 m_table_deleted;
    u64 m_first_used_index;

    inline void deallocate_table()
    {
        if (m_table != nullptr) {
            ka::deallocate_array_raw(m_table, m_table_size);
            m_table = nullptr;
            m_table_size = 0;
            m_table_used = 0;
            m_table_deleted = 0;
            m_first_used_index = 0;
        }
    }

    // expands to accommodate at least one more element
    inline void expand_table()
    {
        expand_table(m_table_used + 1);
    }

    // moves one entry into the new table
    inline u64 move_into_table(EntryType* entry, EntryType* new_table, u64 new_capacity)
    {
        HashFunction hash_fun;
        auto h1 = hash_fun(entry->get_value_ref()) % new_capacity;
        auto h2 = 1 + (h1 ^ (h1 << 23) ^ (h1 >> 19)) % (new_capacity - 1);

        auto index = h1;
        auto cur_entry = &(new_table[index]);

        while (!(cur_entry->is_nonused()) && !(cur_entry->is_deleted())) {
            index = (index + h2) % new_capacity;
            cur_entry = &(new_table[index]);
        }

        *cur_entry = std::move(entry->get_value_ref());
        cur_entry->mark_used();

        return index;
    }

    // rebuilds the table into new_table, destroys old table
    // and deallocates its memory
    inline void rebuild_table(EntryType* new_table, u64 new_capacity)
    {
        bool first = true;
        for (auto cur_entry = m_table, last_entry = m_table + m_table_size;
             cur_entry != last_entry; ++cur_entry) {
            if (cur_entry->is_used()) {
                auto index = move_into_table(cur_entry, new_table, new_capacity);
                if (first) {
                    m_first_used_index = index;
                    first = false;
                }
            }
        }

        // destroy the old table and update the books
        ka::deallocate_array_raw(m_table, m_table_size);
        m_table_size = new_capacity;
        m_table = new_table;
        m_table_deleted = 0;
    }

    // expands to accommodate at least new_capacity elements
    inline void expand_table(u64 new_capacity)
    {
        auto required_capacity = (u64)((float)new_capacity / sc_max_load_factor);
        if (required_capacity < m_table_size) {
            return;
        }
        required_capacity = std::max(required_capacity, sc_initial_table_size);
        required_capacity = ku::PrimeGenerator::get_next_prime(required_capacity);

        auto new_table = ka::allocate_array_raw<EntryType>(required_capacity);
        rebuild_table(new_table, required_capacity);
    }

    inline void rehash_table()
    {
        u64 new_table_size = m_table_size;
        if (((float)m_table_used / (float)m_table_size) < sc_min_load_factor) {
            new_table_size = m_table_used * 2;
            new_table_size = std::max(new_table_size, sc_initial_table_size);
            new_table_size = ku::PrimeGenerator::get_next_prime(new_table_size);
        }

        auto new_table = ka::allocate_array_raw<EntryType>(new_table_size);
        rebuild_table(new_table, new_table_size);
    }

public:
    // This iterator is mutable
    // so that classes which actually implement
    // hash tables can give sensible semantics
    // even if a hash map is implemented
    class Iterator : public std::iterator<std::bidirectional_iterator_tag,
                                          T, i64, T*, T&>
    {
        friend class UnifiedHashTable<T, HashFunction, EqualsFunction>;

    private:
        UnifiedHashTable* m_hash_table;
        EntryType* m_current;

    public:
        inline Iterator()
            : m_hash_table(nullptr), m_current(nullptr)
        {
            // Nothing here
        }

        inline Iterator(UnifiedHashTable* hash_table, EntryType* current)
            : m_hash_table(hash_table), m_current(current)
        {
            // Nothing here
        }

        inline Iterator(const Iterator& other)
            : m_hash_table(other.hash_table), m_current(other.current)
        {
            // Nothing here
        }

        inline Iterator& operator = (const Iterator& other)
        {
            if (&other == this) {
                return *this;
            }
            m_hash_table = other.m_hash_table;
            m_current = other.m_current;
            return *this;
        }

        inline bool operator == (const Iterator& other) const
        {
            return (m_current == other.m_current);
        }

        inline bool operator != (const Iterator& other) const
        {
            return (m_current != other.m_current);
        }

        inline Iterator& operator ++ ()
        {
            auto last = m_hash_table->m_table + m_hash_table->m_table_size;
            if (m_current == last) {
                return *this;
            }
            do {
                ++m_current;
            } while (m_current != last && !(m_current->is_used()));
            return *this;
        }

        inline Iterator& operator -- ()
        {
            auto first = m_hash_table->m_table + m_first_used_index;
            if (m_current == first) {
                return *this;
            }
            do {
                --m_current;
            } while (m_current != first && !(m_current->is_used()));
            return *this;
        }

        inline Iterator operator ++ (int unused)
        {
            auto retval = *this;
            ++(*this);
            return retval;
        }

        inline Iterator operator -- (int unused)

        {
            auto retval = *this;
            --(*this);
            return retval;
        }

        inline T& operator * () const
        {
            return m_current->get_value_ref();
        }

        inline T* operator -> () const
        {
            return m_current->get_value_ptr();
        }
    };

    inline UnifiedHashTable()
        : m_table(nullptr), m_table_size(0), m_table_used(0),
          m_table_deleted(0), m_first_used_index(0)
    {
        // Nothing here
    }

    inline UnifiedHashTable(u64 initial_capacity)
        : UnifiedHashTable()
    {
        auto actual_capacity = std::max(initial_capacity, sc_initial_table_size);
        actual_capacity = ku::PrimeGenerator::get_next_prime(actual_capacity);
        m_table = ka::allocate_array_raw<EntryType>(actual_capacity);
        m_table_size = actual_capacity;
        m_first_used_index = m_table_size;
    }

    inline UnifiedHashTable(const UnifiedHashTable& other)
        : UnifiedHashTable()
    {
        assign(other);
    }

    inline UnifiedHashTable(UnifiedHashTable&& other)
        : UnifiedHashTable()
    {
        assign(std::move(other));
    }

    inline void assign(const UnifiedHashTable& other)
    {
        deallocate_table();

        u64 actual_capacity = (u64)ceil(other.m_table_used * sc_resize_factor);
        actual_capacity = std::max(actual_capacity, sc_initial_table_size);
        actual_capacity = ku::PrimeGenerator::get_next_prime(actual_capacity);
        m_table = ka::allocate_array_raw<EntryType>(actual_capacity);
        m_table_size = actual_capacity;
        m_first_used_index = m_table_size;
    }

    inline void assign(UnifiedHashTable&& other)
    {
        std::swap(m_table, other.m_table);
        std::swap(m_table_used, other.m_table_used);
        std::swap(m_table_size, other.m_table_size);
        std::swap(m_table_deleted, other.m_table_deleted);
        std::swap(m_first_used_index, other.m_first_used_index);
    }

    inline UnifiedHashTable& operator = (const UnifiedHashTable& other) = delete;
    inline UnifiedHashTable& operator = (UnifiedHashTable&& other) = delete;

    inline Iterator begin() const
    {
        return Iterator(this, m_table + m_first_used_index);
    }

    inline Iterator end() const
    {
        return Iterator(this, m_table + m_table_size);
    }

    inline u64 size() const
    {
        return m_table_used;
    }

    inline u64 capacity() const
    {
        return m_table_size;
    }

    inline Iterator find(const T& value) const
    {
        if (m_table == nullptr || m_table_size == 0 || m_table_used == 0) {
            return end();
        }

        HashFunction hash_fun;
        EqualsFunction equals_fun;

        auto h1 = hash_fun(value) % m_table_size;
        auto h2 = 1 + (h1 ^ (h1 << 23) ^ (h1 >> 19)) % (m_table_size - 1);
        auto index = h1;

        auto cur_entry = &(m_table[index]);
        u64 num_probes = 0;

        while (!(cur_entry->is_nonused()) && num_probes < m_table_size) {
            ++num_probes;
            if (!(cur_entry->is_deleted()) && (equals_fun(cur_entry->get_value_ref(), value))) {
                return Iterator(this, cur_entry);
            }

            index = (index + h2) % m_table_size;
            cur_entry = &(m_table[index]);
        }

        return end();
    }

    inline Iterator insert(const T& value, bool& already_present)
    {
        T copied_value(value);
        return insert(std::move(copied_value), already_present);
    }

    inline Iterator insert(T&& value, bool& already_present)
    {
        auto it = find(value);
        if (it != end()) {
            already_present = true;
            return it;
        }

        already_present = false;

        expand_table();

        HashFunction hash_fun;
        auto h1 = hash_fun(value) % m_table_size;
        auto h2 = 1 + (h1 ^ (h1 << 23) ^ (h1 >> 19)) % (m_table_size - 1);
        auto index = h1;

        auto cur_entry = &(m_table[index]);
        while (!(cur_entry->is_nonused()) && !(cur_entry->is_deleted())) {
            index = (index + h2) % m_table_size;
            cur_entry = &(m_table[index]);
        }

        *cur_entry = std::move(value);
        if (index < m_first_used_index) {
            m_first_used_index = index;
        }

        cur_entry->mark_used();
        ++m_table_used;
        return Iterator(this, cur_entry);
    }

    template <typename... ArgTypes>
    inline Iterator emplace(bool& already_present, ArgTypes&&... args)
    {
        T constructed_value(std::forward<ArgTypes>(args)...);
        return insert(constructed_value, already_present);
    }

    inline void erase(const Iterator& position)
    {
        if (position.m_current->is_used()) {
            position.m_current->mark_deleted();
        } else {
            return;
        }

        ++m_table_deleted;
        --m_table_used;

        auto deleted_nonused_ratio = (float)m_table_deleted / (float)(m_table_size - m_table_used);
        if ((deleted_nonused_ratio >= sc_deleted_nonused_rehash_ratio) ||
            (((float)m_table_used / (float)m_table_size) < sc_min_load_factor)) {
            rehash_table();
        } else if ((position.m_current - m_table) == m_first_used_index) {
            auto temp = position;
            ++temp;
            m_first_used_index = temp.m_current - m_table;
        }
    }

    inline void clear()
    {
        deallocate_table();
    }
};

template <typename T, typename HashFunction, typename EqualsFunction>
class SegregatedHashTable final : private HashTableBase
{

};

template <typename T, typename HashFunction, typename EqualsFunction>
class RestrictedHashTable final : private HashTableBase
{

};

} /* end namespace hash_table_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_HASH_TABLE_HPP_ */

//
// HashTable.hpp ends here
