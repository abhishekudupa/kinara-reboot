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

namespace kinara {
namespace containers {
namespace hashtable_detail_ {

namespace ka = kinara::allocators;
namespace kc = kinara::containers;
namespace ku = kinara::utils;

template <typename T>
class HashEntry
{
private:
    static constexpr u32 sc_nonused_marker = 0x0;
    static constexpr u32 sc_deleted_marker = 0x1;
    static constexpr u32 sc_in_use_marker = 0x2;

public:
    T m_value;
    u32 m_marker;

    HashEntry()
        : m_value(), m_marker(sc_nonused_marker)
    {
        // Nothing here
    }

    HashEntry(const HashEntry& other)
        : m_value(other.m_value), m_marker(other.m_marker)
    {
        // Nothing here
    }

    HashEntry(HashEntry&& other)
        : HashEntry()
    {
        std::swap(m_value, other.m_value);
        std::swap(m_marker, other.m_marker);
    }

    ~HashEntry()
    {
        // Nothing here
    }

    inline HashEntry& operator = (const HashEntry& other)
    {
        if (&other == this) {
            return *this;
        }
        m_value = other.m_value;
        m_marker = other.m_marker;
    }

    inline HashEntry& operator = (HashEntry&& other)
    {
        if (&other == this) {
            return *this;
        }
        std::swap(m_value, other.m_value);
        std::swap(m_marker, other.m_marker);
    }

    inline bool is_in_use() const
    {
        return (m_marker == sc_in_use_marker);
    }

    inline bool is_nonused() const
    {
        return (m_marker == sc_nonused_marker);
    }

    inline bool is_deleted() const
    {
        return (m_marker == sc_deleted_marker);
    }

    inline void mark_in_use()
    {
        m_marker = sc_in_use_marker;
    }

    inline void mark_deleted()
    {
        m_marker = sc_deleted_marker;
    }

    inline void mark_nonused()
    {
        m_marker = sc_nonused_marker;
    }

    const T& get_value() const
    {
        return m_value;
    }

    const T* get_ptr() const
    {
        return &m_value;
    }
};

// specialization for pointer types
template <typename T>
class HashEntry<T*>
{
private:
    static constexpr u64 sc_nonused_marker = 0x0;
    static constexpr u64 sc_deleted_marker = 0x1;
    static constexpr u64 sc_in_use_marker = 0x2;

public:
    T* m_value;

    HashEntry()
        : m_value((T*)sc_nonused_marker)
    {
        // Nothing here
    }

    HashEntry(const HashEntry& other)
        : m_value(other.m_value)
    {
        // Nothing here
    }

    HashEntry(HashEntry&& other)
        : HashEntry()
    {
        std::swap(m_value, other.m_value);
    }

    ~HashEntry()
    {
        // Nothing here
    }

    inline HashEntry& operator = (const HashEntry& other)
    {
        if (&other == this) {
            return *this;
        }
        m_value = other.m_value;
    }

    inline HashEntry& operator = (HashEntry&& other)
    {
        if (&other == this) {
            return *this;
        }
        std::swap(m_value, other.m_value);
    }

    inline bool is_in_use() const
    {
        return (m_value == (T*)sc_in_use_marker);
    }

    inline bool is_nonused() const
    {
        return (m_value == (T*)sc_nonused_marker);
    }

    inline bool is_deleted() const
    {
        return (m_value == (T*)sc_deleted_marker);
    }

    inline void mark_in_use()
    {
        m_value = (T*)sc_in_use_marker;
    }

    inline void mark_deleted()
    {
        m_value = (T*)sc_deleted_marker;
    }

    inline void mark_nonused()
    {
        m_value = (T*)sc_nonused_marker;
    }

    T* get_value() const
    {
        return m_value;
    }

    T* const* get_ptr() const
    {
        return &m_value;
    }
};

template <typename T>
class HashEntry<const T*>
{
private:
    static constexpr u64 sc_nonused_marker = 0x0;
    static constexpr u64 sc_deleted_marker = 0x1;
    static constexpr u64 sc_in_use_marker = 0x2;

public:
    const T* m_value;

    HashEntry()
        : m_value((const T*)sc_nonused_marker)
    {
        // Nothing here
    }

    HashEntry(const HashEntry& other)
        : m_value(other.m_value)
    {
        // Nothing here
    }

    HashEntry(HashEntry&& other)
        : HashEntry()
    {
        std::swap(m_value, other.m_value);
    }

    ~HashEntry()
    {
        // Nothing here
    }

    inline HashEntry& operator = (const HashEntry& other)
    {
        if (&other == this) {
            return *this;
        }
        m_value = other.m_value;
    }

    inline HashEntry& operator = (HashEntry&& other)
    {
        if (&other == this) {
            return *this;
        }
        std::swap(m_value, other.m_value);
    }

    inline bool is_in_use() const
    {
        return (m_value == (const T*)sc_in_use_marker);
    }

    inline bool is_nonused() const
    {
        return (m_value == (const T*)sc_nonused_marker);
    }

    inline bool is_deleted() const
    {
        return (m_value == (const T*)sc_deleted_marker);
    }

    inline void mark_in_use()
    {
        m_value = (const T*)sc_in_use_marker;
    }

    inline void mark_deleted()
    {
        m_value = (const T*)sc_deleted_marker;
    }

    inline void mark_nonused()
    {
        m_value = (const T*)sc_nonused_marker;
    }

    T const* const* get_value() const
    {
        return &m_value;
    }
};

// iterator class, note that hash tables support only
// const iterators!
template <typename T>
class Iterator : public std::iterator<std::bidirectional_iterator_tag,
                                      T, i64, const T*, const T&>
{
private:
    typedef HashEntry<T> TableEntry;
    TableEntry* m_entry;

public:
    Iterator()
        : m_entry(nullptr)
    {
        // Nothing here
    }

    Iterator(TableEntry* entry)
        : m_entry(entry)
    {
        // Nothing here
    }

    Iterator(const Iterator& other)
        : m_entry(other.m_entry)
    {
        // Nothing here
    }

    ~Iterator()
    {
        // Nothing here
    }

    inline Iterator& operator = (const Iterator& other)
    {
        if (&other == this) {
            return *this;
        }
        m_entry = other.m_entry;
    }

    inline Iterator& operator ++ ()
    {
        do {
            ++m_entry;
        } while (!m_entry->is_in_use());
        return *this;
    }

    inline Iterator operator ++ (int usused)
    {
        auto retval = *this;
        (*this)++;
        return retval;
    }

    inline Iterator& operator -- ()
    {
        do {
            --m_entry;
        } while (!m_entry->is_in_use());
        return *this;
    }

    inline Iterator operator -- (int unused)
    {
        auto retval = *this;
        (*this)--;
        return retval;
    }

    inline const T& operator * () const
    {
        return m_entry->get_value();
    }

    inline const T* operator -> () const
    {
        return m_entry->get_ptr();
    }
};

// Maintains a simple open-addressed hash table
template <typename T, typename HashFunction = ku::Hasher<T>,
          typename EqualsFunction = std::equal_to<T> >
class HashTable
{
private:
    static constexpr float sc_default_resize_factor = 1.618f;
    static constexpr float sc_default_max_load_factor = 0.7f;
    static constexpr float sc_default_min_load_factor = 0.05f;
    static constexpr float sc_allocator_utilization_low = 0.1f;
    static constexpr float sc_deleted_nonused_rehash_ratio = 0.5f;
    static constexpr u64 sc_initial_table_size = 19;

    typedef HashEntry<T> TableEntry;

    TableEntry* m_table;
    u64 m_table_size;
    u64 m_size;

    inline TableEntry* allocate_table(u64 table_size)
    {
        // we reserve the first and last element of the table
        // as sentinels
        ku::PrimeGenerator prime_generator(true);
        auto actual_table_size = prime_generator.get_next_prime(table_size + 2);
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

        u64 h1 = hash_function(value) % m_table_size;
        u64 h2 = 1 + (((h1 << 17) ^ h1) % (m_table_size - 1));
        u64 num_probes = 0;
        u64 index = h1;
        TableEntry* cur_entry = m_table[index];

        while (!(cur_entry->is_nonused()) && num_probes < m_table_size) {
            ++num_probes;
            if (!(cur_entry->is_deleted()) && equals_function(cur_entry->get_value(), value)) {
                return Iterator(cur_entry);
            }

            // rehash
            index = (index + h2) % m_table_size;
            entry = m_table[index];
        }

        return end();
    }

    inline Iterator insert(const T& value)
    {
        if (m_table == nullptr) {
            allocate_table(sc_initial_table_size);
        }
    }

};

} /* end namespace hashtable_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_HASH_TABLE_HPP_ */

//
// HashTable.hpp ends here
