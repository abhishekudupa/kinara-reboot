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

#include <initializer_list>
#include <iterator>

#include "../basetypes/KinaraTypes.hpp"
#include "../allocators/MemoryManager.hpp"
#include "../primeutils/PrimeGenerator.hpp"

#include "BitSet.hpp"

namespace kinara {
namespace containers {
namespace hash_table_detail_ {

namespace ka = kinara::allocators;
namespace ku = kinara::utils;

class HashTableBase
{
public:
    // the silver ratio, because the golden one is
    // too small!
    static constexpr float sc_resize_factor = 2.414214f;
    static constexpr float sc_max_load_factor = 0.717f;
    static constexpr float sc_min_load_factor = 0.1f;
    static constexpr float sc_deleted_nonused_rehash_ratio = 0.5f;
    static constexpr u64 sc_initial_table_size = 19;
};

// All hash tables derive privately from this class
// i.e., all hash tables are implemented in terms of this
// class. This base class handles all the details of
// maintaining the hash table, but depends on the implementation
// to figure out when an entry is considered nonused, deleted
// or in use. It also needs the type of the entries
// to be declared in the implementation.
// Finally, the implementation needs to declare the iterators
// as friends.
// To sum up the implementation must make the following available:
// 1.  EntryType, indicating the type of entries to use
// 2.  get_value_ref(EntryType*) to get the value reference from an entry
// 3.  get_value_ptr(EntryType*) to get the value pointer from an entry
// 4.  is_entry_nonused(EntryType*)
// 5.  is_entry_deleted(EntryType*)
// 6.  is_entry_used(EntryType*)
// 7.  mark_entry_used(EntryType*)
// 8.  mark_entry_deleted(EntryType*)
// 9.  mark_entry_nonused(EntryType*)
// 10. The implementation must inherit privately from HashTableImplBase
// 11. The implementation must mark HashTableImplBase as a friend
// 12. EntryType must support being assigned to a value of type T

// the following are callbacks on resize of tables
// 13. begin_resize(u64 new_table_size)
// 14. end_resize(u64 new_table_size)
// 15. is_new_entry_used(EntryType* new_table, u64 new_table_size, EntryType* entry)
// 16. is_new_entry_nonused(EntryType* new_table, u64 new_table_size, EntryType* entry)
// 17. mark_new_entry_used(EntryType* new_table, u64 new_table_size, EntryType* entry)
// 18. mark_new_entry_nonused(EntryType* new_table, u64 new_table_size, EntryType* entry)
// 19. initialize_new_table(EntryType* new_table, u64 new_table_size)
// 20. on_clear() for clearing all data structures
// 21. set_size(u64) which sets the size to a particular value

template <typename T, typename HashFunction, typename EqualsFunction,
          template <typename, typename, typename> class HashTableImpl,
          typename EntryType>
class HashTableImplBase : private HashTableBase
{
protected:
    typedef T ValueType;
    typedef HashTableImpl<T, HashFunction, EqualsFunction> ImplType;

    static constexpr u64 sc_fnv_prime = 0x100000001b3ul;

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

    inline void get_hashes(const T& value, u64 table_size, u64& h1, u64& h2) const
    {
        HashFunction hash_fun;

        h1 = hash_fun(value) % table_size;
        h2 = 1 + ((h1 * sc_fnv_prime) % (table_size - 1));
    }

    inline ImplType* this_as_impl()
    {
        return static_cast<ImplType*>(this);
    }

    inline const ImplType* this_as_impl() const
    {
        return static_cast<const ImplType*>(this);
    }

    // expands to accommodate at least one more element
    inline void expand_table()
    {
        expand_table(m_table_used + 1);
    }

    // moves one entry into the new table
    inline u64 move_into_table(EntryType* entry, EntryType* new_table, u64 new_capacity)
    {
        auto as_impl = this_as_impl();

        auto& value_ref = as_impl->get_value_ref(entry);

        u64 h1, h2;
        get_hashes(value_ref, new_capacity, h1, h2);

        auto index = h1;
        auto cur_entry = &(new_table[index]);

        auto is_nonused = as_impl->is_new_entry_nonused(new_table, new_capacity, cur_entry);

        while (!is_nonused) {
            index = (index + h2) % new_capacity;
            cur_entry = &(new_table[index]);

            is_nonused = as_impl->is_new_entry_nonused(new_table, new_capacity, cur_entry);
        }

        cur_entry->~EntryType();
        new (cur_entry) EntryType(std::move(value_ref));

        as_impl->mark_new_entry_used(new_table, new_capacity, cur_entry);

        return index;
    }

    // rebuilds the table into new_table, destroys old table
    // and deallocates its memory
    inline void rebuild_table(EntryType* new_table, u64 new_capacity)
    {
        auto as_impl = this_as_impl();
        as_impl->begin_resize(new_capacity);
        as_impl->initialize_new_table(new_table, new_capacity);

        m_first_used_index = new_capacity;

        for (auto cur_entry = m_table, last_entry = m_table + m_table_size;
             cur_entry != last_entry; ++cur_entry) {

            if (as_impl->is_entry_used(cur_entry)) {
                auto index = move_into_table(cur_entry, new_table, new_capacity);
                if (index < m_first_used_index) {
                    m_first_used_index = index;
                }
            }
        }

        // destroy the old table and update the books
        if (m_table != nullptr) {
            ka::deallocate_array_raw(m_table, m_table_size);
        } else {
            m_first_used_index = new_capacity;
        }

        m_table_size = new_capacity;
        m_table = new_table;
        m_table_deleted = 0;

        as_impl->end_resize(new_capacity);
    }

    // expands to accommodate at least new_size elements
    inline void expand_table(u64 new_size)
    {
        if (m_table_size > 0) {
            auto new_usage = (float)new_size / (float)m_table_size;

            if (new_usage < sc_max_load_factor) {
                return;
            }
        }
        // need to reallocate
        auto required_capacity = (u64)(new_size * sc_resize_factor);
        auto initial_table_size = sc_initial_table_size;
        required_capacity = std::max(required_capacity, initial_table_size);
        required_capacity = ku::PrimeGenerator::get_next_prime(required_capacity);

        auto new_table = ka::allocate_array_raw<EntryType>(required_capacity);
        rebuild_table(new_table, required_capacity);
    }

    inline void rehash_table()
    {
        u64 new_table_size = m_table_size;
        auto initial_table_size = sc_initial_table_size;

        if (((float)m_table_used / (float)m_table_size) < sc_min_load_factor) {
            new_table_size = (u64)(m_table_used * sc_resize_factor);
            new_table_size = std::max(new_table_size, initial_table_size);
            new_table_size = ku::PrimeGenerator::get_next_prime(new_table_size);
        }

        auto new_table = ka::allocate_array_raw<EntryType>(new_table_size);
        rebuild_table(new_table, new_table_size);
    }

protected:

    // This iterator is mutable
    // so that classes which actually implement
    // hash tables can give sensible semantics
    // even if a hash map is implemented
    class Iterator
    {
    public:
        typedef i64 difference_type;
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef EntryType value_type;
        typedef EntryType& reference;
        typedef EntryType* pointer;

    private:
        HashTableImplBase* m_hash_table;
        EntryType* m_current;

        inline ImplType* hash_table_as_impl() const
        {
            return static_cast<ImplType*>(m_hash_table);
        }

    public:
        inline Iterator()
            : m_hash_table(nullptr), m_current(nullptr)
        {
            // Nothing here
        }

        inline Iterator(HashTableImplBase* hash_table, EntryType* current)
            : m_hash_table(hash_table), m_current(current)
        {
            // Nothing here
        }

        inline Iterator(const Iterator& other)
            : m_hash_table(other.m_hash_table), m_current(other.m_current)
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
            auto as_impl = hash_table_as_impl();
            auto last = m_hash_table->m_table + m_hash_table->m_table_size;
            if (m_current == last) {
                return *this;
            }
            do {
                ++m_current;
            } while (m_current != last && !(as_impl->is_entry_used(m_current)));
            return *this;
        }

        inline Iterator& operator -- ()
        {
            auto first = m_hash_table->m_table + m_first_used_index;
            auto as_impl = hash_table_as_impl();

            if (m_current == first) {
                return *this;
            }
            do {
                --m_current;
            } while (m_current != first && !(as_impl->is_entry_used(m_current)));
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
            return (hash_table_as_impl()->get_value_ref(m_current));
        }

        inline T* operator -> () const
        {
            return (hash_table_as_impl()->get_value_ptr(m_current));
        }

        inline EntryType* get_current() const
        {
            return m_current;
        }

        inline HashTableImplBase* get_hash_table() const
        {
            return m_hash_table;
        }
    };

private:
    template <typename InputIterator>
    inline void insert_range(const InputIterator& first,
                                 const InputIterator& last,
                                 std::input_iterator_tag unused)
    {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    template <typename ForwardIterator>
    inline void insert_range(const ForwardIterator& first,
                             const ForwardIterator& last,
                             std::forward_iterator_tag unused)
    {
        auto num_elements = std::distance(first, last);
        expand_table(m_table_size + num_elements);
        bool dummy;
        for (auto it = first; it != last; ++it) {
            insert(*it, dummy);
        }
    }

    template <typename InputIterator>
    inline void insert_range(const InputIterator& first, const InputIterator& last)
    {
        typedef typename std::iterator_traits<InputIterator>::iterator_category IterCategory;
        insert_range(first, last, IterCategory());
    }

protected:
    inline HashTableImplBase()
        : m_table(nullptr), m_table_size(0), m_table_used(0),
          m_table_deleted(0), m_first_used_index(0)
    {
        // Nothing here
    }

    inline explicit HashTableImplBase(u64 initial_capacity)
        : HashTableImplBase()
    {
        auto initial_table_size = sc_initial_table_size;
        auto actual_capacity = std::max(initial_capacity, initial_table_size);
        actual_capacity = ku::PrimeGenerator::get_next_prime(actual_capacity);
        m_table = ka::allocate_array_raw<EntryType>(actual_capacity);
        m_table_size = actual_capacity;
        m_first_used_index = m_table_size;
        this_as_impl()->initialize_new_table(m_table, m_table_size);
    }

    inline HashTableImplBase(const HashTableImplBase& other)
        : HashTableImplBase()
    {
        assign(other);
    }

    inline HashTableImplBase(HashTableImplBase&& other)
        : HashTableImplBase()
    {
        assign(std::move(other));
    }

    template <typename InputIterator>
    inline HashTableImplBase(const InputIterator& first, const InputIterator& last)
        : HashTableImplBase()
    {
        assign(first, last);
    }

    inline HashTableImplBase(std::initializer_list<T> init_list)
        : HashTableImplBase()
    {
        assign(init_list);
    }

    inline ~HashTableImplBase()
    {
        deallocate_table();
    }

    template <typename InputIterator>
    inline void assign(const InputIterator& first, const InputIterator& last)
    {
        clear();

        if (first == last) {
            return;
        }

        typedef typename std::iterator_traits<InputIterator>::iterator_category IterCategory;
        insert_range(first, last, IterCategory());
    }

    inline void assign(std::initializer_list<T> init_list)
    {
        assign(init_list.begin(), init_list.end());
    }

    inline void assign(const HashTableImplBase& other)
    {
        clear();

        if (other.size() == 0) {
            return;
        }

        auto as_impl = this_as_impl();
        auto initial_table_size = sc_initial_table_size;
        u64 actual_capacity = (u64)ceil(other.m_table_used * sc_resize_factor);
        actual_capacity = std::max(actual_capacity, initial_table_size);
        actual_capacity = ku::PrimeGenerator::get_next_prime(actual_capacity);
        m_table = ka::allocate_array_raw<EntryType>(actual_capacity);
        m_table_size = actual_capacity;
        m_first_used_index = m_table_size;

        as_impl->initialize_new_table(m_table, m_table_size);
        as_impl->set_size(m_table_size);

        bool dummy;
        for (auto it = other.begin(), last = other.end(); it != last; ++it) {
            this->insert(*it, dummy);
        }
    }

    inline void assign(HashTableImplBase&& other)
    {
        if (other.size() == 0) {
            return;
        }

        std::swap(m_table, other.m_table);
        std::swap(m_table_used, other.m_table_used);
        std::swap(m_table_size, other.m_table_size);
        std::swap(m_table_deleted, other.m_table_deleted);
        std::swap(m_first_used_index, other.m_first_used_index);
    }

    inline HashTableImplBase& operator = (const HashTableImplBase& other) = delete;
    inline HashTableImplBase& operator = (HashTableImplBase&& other) = delete;

    inline bool empty() const
    {
        return (this->m_table_used == 0);
    }

    inline Iterator begin() const
    {
        return Iterator(const_cast<HashTableImplBase*>(this), m_table + m_first_used_index);
    }

    inline Iterator end() const
    {
        return Iterator(const_cast<HashTableImplBase*>(this), m_table + m_table_size);
    }

    inline u64 size() const
    {
        return m_table_used;
    }

    inline u64 max_size() const
    {
        return UINT64_MAX;
    }

    inline u64 count(const ValueType& value) const
    {
        return (find(value) == end() ? 0 : 1);
    }

    inline void reserve(u64 new_capacity)
    {
        auto initial_table_size = sc_initial_table_size;
        auto actual_capacity = std::max(initial_table_size, new_capacity);
        actual_capacity = ku::PrimeGenerator::get_next_prime(actual_capacity);
        expand_table(actual_capacity);
    }

    inline void rehash(u64 new_capacity)
    {
        if (new_capacity <= m_table_size) {
            rehash_table();
        } else {
            expand_table(new_capacity);
        }
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

        EqualsFunction equals_fun;
        auto as_impl = this_as_impl();

        u64 h1, h2;
        get_hashes(value, m_table_size, h1, h2);
        auto index = h1;

        auto cur_entry = &(m_table[index]);
        u64 num_probes = 0;

        auto is_nonused = as_impl->is_entry_nonused(cur_entry);
        while (!is_nonused && num_probes < m_table_size) {
            ++num_probes;
            auto const& value_ref = as_impl->get_value_ref(cur_entry);
            auto is_deleted = as_impl->is_entry_deleted(cur_entry);

            if (!is_deleted && (equals_fun(value_ref, value))) {
                return Iterator(const_cast<HashTableImplBase*>(this), cur_entry);
            }

            index = (index + h2) % m_table_size;
            cur_entry = &(m_table[index]);
            is_nonused = as_impl->is_entry_nonused(cur_entry);
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
        auto as_impl = this_as_impl();

        expand_table();

        u64 h1, h2;
        get_hashes(value, m_table_size, h1, h2);
        auto index = h1;

        auto cur_entry = &(m_table[index]);
        auto is_nonused = as_impl->is_entry_nonused(cur_entry);
        auto is_deleted = as_impl->is_entry_deleted(cur_entry);

        while (!is_nonused && !is_deleted) {
            index = (index + h2) % m_table_size;
            cur_entry = &(m_table[index]);

            is_nonused = as_impl->is_entry_nonused(cur_entry);
            is_deleted = as_impl->is_entry_deleted(cur_entry);
        }

        cur_entry->~EntryType();
        new (cur_entry) EntryType(std::move(value));

        if (index < m_first_used_index) {
            m_first_used_index = index;
        }

        as_impl->mark_entry_used(cur_entry);
        ++m_table_used;
        return Iterator(this, cur_entry);
    }

    template <typename InputIterator>
    inline void insert(const InputIterator& first, const InputIterator& last)
    {
        insert_range(first, last);
    }

    inline void insert(std::initializer_list<T> init_list)
    {
        insert_range(init_list.begin(), init_list.end());
    }

    template <typename... ArgTypes>
    inline Iterator emplace(bool& already_present, ArgTypes&&... args)
    {
        T constructed_value(std::forward<ArgTypes>(args)...);
        return insert(constructed_value, already_present);
    }

    inline void erase(const Iterator& position)
    {
        auto as_impl = this_as_impl();

        if (as_impl->is_entry_used(position.get_current())) {
            as_impl->mark_entry_deleted(position.get_current());
        } else {
            return;
        }

        ++m_table_deleted;
        --m_table_used;

        auto deleted_nonused_ratio = (float)m_table_deleted / (float)(m_table_size - m_table_used);
        if ((deleted_nonused_ratio >= sc_deleted_nonused_rehash_ratio) ||
            (((float)m_table_used / (float)m_table_size) < sc_min_load_factor)) {
            rehash_table();
        } else if ((u64)(position.get_current() - m_table) == m_first_used_index) {
            auto temp = position;
            ++temp;
            m_first_used_index = temp.get_current() - m_table;
        }
    }

    inline void erase(const ValueType& value)
    {
        auto position = find(value);
        if (position == end()) {
            return;
        }
        erase(position);
    }

    inline void erase(const Iterator& first, const Iterator& last)
    {
        for (auto it = first; it != last; ++it) {
            erase(it);
        }
    }

    inline void clear()
    {
        auto impl = this_as_impl();
        deallocate_table();
        impl->on_clear();
    }

    // Does not actually shrink to fit,
    // but shrinks to ~ size / sc_max_load_factor
    inline void shrink_to_fit()
    {
        auto new_size = (u64)(m_table_used / sc_max_load_factor);
        new_size += 3;
        auto initial_table_size = sc_initial_table_size;
        new_size = std::max(new_size, initial_table_size);
        new_size = ku::PrimeGenerator::get_next_prime(new_size);

        if (new_size == m_table_size) {
            return;
        }

        auto new_table = ka::allocate_array_raw<EntryType>(new_size);
        rebuild_table(new_table, new_size);
    }
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
class UnifiedHashTable
    : protected HashTableImplBase<T, HashFunction, EqualsFunction,
                                  kc::hash_table_detail_::UnifiedHashTable,
                                  UnifiedHashTableEntry<T> >
{
private:
    typedef HashTableImplBase<T, HashFunction, EqualsFunction,
                              kc::hash_table_detail_::UnifiedHashTable,
                              UnifiedHashTableEntry<T> > BaseType;
    friend BaseType;

private:
    typedef UnifiedHashTableEntry<T> EntryType;

    inline T& get_value_ref(EntryType* entry) const
    {
        return entry->get_value_ref();
    }

    inline T* get_value_ptr(EntryType* entry) const
    {
        return entry->get_value_ptr();
    }

    inline bool is_entry_nonused(EntryType* entry) const
    {
        return entry->is_nonused();
    }

    inline bool is_entry_deleted(EntryType* entry) const
    {
        return entry->is_deleted();
    }

    inline bool is_entry_used(EntryType* entry) const
    {
        return entry->is_used();
    }

    inline void mark_entry_used(EntryType* entry) const
    {
        entry->mark_used();
    }

    inline void mark_entry_deleted(EntryType* entry) const
    {
        entry->mark_deleted();
    }

    inline void mark_entry_nonused(EntryType* entry) const
    {
        entry->mark_nonused();
    }

    inline void begin_resize(u64 new_table_size) const
    {
        // Nothing here
    }

    inline void end_resize(u64 new_table_size) const
    {
        // Nothing here
    }

    inline bool is_new_entry_used(EntryType* new_table,
                                  u64 new_table_size,
                                  EntryType* entry) const
    {
        return entry->is_used();
    }

    inline bool is_new_entry_nonused(EntryType* new_table,
                                     u64 new_table_size,
                                     EntryType* entry) const
    {
        return entry->is_nonused();
    }

    inline void mark_new_entry_used(EntryType* new_table,
                                    u64 new_table_size,
                                    EntryType* entry) const
    {
        entry->mark_used();
    }

    inline void mark_new_entry_nonused(EntryType* new_table,
                                    u64 new_table_size,
                                    EntryType* entry) const
    {
        entry->mark_nonused();
    }

    inline void initialize_new_table(EntryType* new_table, u64 new_table_size) const
    {
        // Nothing needs to be done, entries are marked nonused by default
    }

    inline void on_clear() const
    {
        // Nothing needs to be done
    }

    inline void set_size(u64 new_size) const
    {
        // Nothing needs to be done
    }

public:
    typedef typename BaseType::Iterator Iterator;
    typedef Iterator iterator;

    // These accept deleted and nonused values for compatibility
    inline UnifiedHashTable()
        : BaseType()
    {
        // Nothing here
    }

    inline UnifiedHashTable(const T& deleted_value, const T& nonused_value)
        : BaseType()
    {
        // Nothing here
    }

    inline explicit UnifiedHashTable(u64 initial_capacity)
        : BaseType(initial_capacity)
    {
        // Nothing here
    }

    inline UnifiedHashTable(u64 initial_capacity,
                            const T& deleted_value,
                            const T& nonused_value)
        : BaseType(initial_capacity)
    {
        // Nothing here
    }

    inline UnifiedHashTable(const UnifiedHashTable& other)
        : BaseType(other)
    {
        // Nothing here
    }

    inline UnifiedHashTable(UnifiedHashTable&& other)
        : BaseType(std::move(other))
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline UnifiedHashTable(const InputIterator& first, const InputIterator& last)
        : BaseType(first, last)
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline UnifiedHashTable(const InputIterator& first, const InputIterator& last,
                            const T& deleted_value, const T& nonused_value)
        : BaseType(first, last)
    {
        // Nothing here
    }

    inline UnifiedHashTable(std::initializer_list<T> init_list)
        : BaseType(init_list)
    {
        // Nothing here
    }

    inline UnifiedHashTable(std::initializer_list<T> init_list,
                            const T& deleted_value,
                            const T& nonused_value)
        : BaseType(init_list)
    {
        // Nothing here
    }

    inline ~UnifiedHashTable()
    {
        // Nothing here
    }

    inline UnifiedHashTable& operator = (const UnifiedHashTable& other)
    {
        if (&other == this) {
            return *this;
        }
        BaseType::assign(other);
        return *this;
    }

    inline UnifiedHashTable& operator = (UnifiedHashTable&& other)
    {
        if (&other == this) {
            return *this;
        }
        BaseType::assign(std::move(other));
        return *this;
    }

    inline UnifiedHashTable& operator = (std::initializer_list<T> init_list)
    {
        BaseType::assign(init_list);
        return *this;
    }

    // for compatibility
    inline T get_deleted_value() const
    {
        return T();
    }

    inline T get_nonused_value() const
    {
        return T();
    }

    inline void set_deleted_value(const T& value) const
    {
        // nothing
    }

    inline void set_nonused_value(const T& value) const
    {
        // nothing
    }
};

template <typename T, typename HashFunction, typename EqualsFunction>
class SegregatedHashTable
    : protected HashTableImplBase<T, HashFunction, EqualsFunction,
                                  kc::hash_table_detail_::SegregatedHashTable, T>
{
private:
    typedef HashTableImplBase<T, HashFunction, EqualsFunction,
                              kc::hash_table_detail_::SegregatedHashTable, T> BaseType;
    friend BaseType;
    typedef T EntryType;

    // additional structures to keep track of nonused and deleted entries
    mutable BitSet m_deleted_entries;
    mutable BitSet m_nonused_entries;

    mutable BitSet m_new_deleted_entries;
    mutable BitSet m_new_nonused_entries;

    inline u64 get_index_for_entry(EntryType* entry) const
    {
        return (entry - this->m_table);
    }

    inline EntryType& get_value_ref(EntryType* entry) const
    {
        return *entry;
    }

    inline EntryType* get_value_ptr(EntryType* entry) const
    {
        return entry;
    }

    inline bool is_entry_nonused(EntryType* entry) const
    {
        auto index = get_index_for_entry(entry);
        return m_nonused_entries.test(index);
    }

    inline bool is_entry_deleted(EntryType* entry) const
    {
        auto index = get_index_for_entry(entry);
        return m_deleted_entries.test(index);
    }

    inline bool is_entry_used(EntryType* entry) const
    {
        auto index = get_index_for_entry(entry);
        return (!m_nonused_entries.test(index) && !m_deleted_entries.test(index));
    }

    inline void mark_entry_nonused(EntryType* entry) const
    {
        auto index = get_index_for_entry(entry);
        m_nonused_entries.set(index);
        m_deleted_entries.clear(index);
    }

    inline void mark_entry_deleted(EntryType* entry) const
    {
        auto index = get_index_for_entry(entry);
        m_nonused_entries.clear(index);
        m_deleted_entries.set(index);
    }

    inline void mark_entry_used(EntryType* entry) const
    {
        auto index = get_index_for_entry(entry);
        m_nonused_entries.clear(index);
        m_deleted_entries.clear(index);
    }

    inline void begin_resize(u64 new_table_size) const
    {
        m_new_nonused_entries.resize_and_clear(new_table_size);
        m_new_nonused_entries.set();
        m_new_deleted_entries.resize_and_clear(new_table_size);
    }

    inline void end_resize(u64 new_table_size) const
    {
        m_deleted_entries.reset();
        m_nonused_entries.reset();

        m_deleted_entries = std::move(m_new_deleted_entries);
        m_nonused_entries = std::move(m_new_nonused_entries);
    }

    inline bool is_new_entry_used(EntryType* new_table,
                                  u64 new_table_size,
                                  EntryType* entry) const
    {
        auto index = entry - new_table;
        return (!m_new_nonused_entries.test(index) && !m_new_deleted_entries.test(index));
    }

    inline bool is_new_entry_nonused(EntryType* new_table,
                                     u64 new_table_size,
                                     EntryType* entry) const
    {
        auto index = entry - new_table;
        return (m_new_nonused_entries.test(index));
    }

    inline void mark_new_entry_used(EntryType* new_table,
                                    u64 new_table_size,
                                    EntryType* entry) const
    {
        auto index = entry - new_table;
        m_new_nonused_entries.clear(index);
        m_new_deleted_entries.clear(index);
    }

    inline void mark_new_entry_nonused(EntryType* new_table,
                                       u64 new_table_size,
                                       EntryType* entry) const
    {
        auto index = entry - new_table;
        m_new_nonused_entries.set(index);
        m_new_deleted_entries.clear(index);
    }

    inline void initialize_new_table(EntryType* new_table, u64 new_table_size) const
    {
        // Nothing to be done
    }

    inline void on_clear() const
    {
        m_nonused_entries.reset();
        m_deleted_entries.reset();
        m_new_nonused_entries.reset();
        m_new_deleted_entries.reset();
    }

    inline void set_size(u64 new_size) const
    {
        m_deleted_entries.resize_and_clear(new_size);
        m_nonused_entries.resize_and_clear(new_size);
        m_nonused_entries.set();
    }

public:
    typedef typename BaseType::Iterator Iterator;
    typedef Iterator iterator;

    inline SegregatedHashTable()
        : BaseType(), m_deleted_entries(), m_nonused_entries(),
          m_new_deleted_entries(), m_new_nonused_entries()
    {
        // Nothing here
    }

    inline SegregatedHashTable(const T& deleted_value, const T& nonused_value)
        : BaseType(), m_deleted_entries(), m_nonused_entries(),
          m_new_deleted_entries(), m_new_nonused_entries()
    {
        // Nothing here
    }

    inline SegregatedHashTable(u64 initial_capacity)
        : BaseType(), m_deleted_entries(), m_nonused_entries(),
          m_new_deleted_entries(), m_new_nonused_entries()
    {
        this->expand_table(initial_capacity);
        set_size(this->m_table_size);
    }

    inline SegregatedHashTable(u64 initial_capacity, const T& deleted_value,
                               const T& nonused_value)
        : BaseType(), m_deleted_entries(), m_nonused_entries(),
          m_new_deleted_entries(), m_new_nonused_entries()
    {
        this->expand_table(initial_capacity);
        set_size(this->m_table_size);
    }

    inline SegregatedHashTable(const SegregatedHashTable& other)
        : BaseType(), m_deleted_entries(), m_nonused_entries(),
          m_new_deleted_entries(), m_new_nonused_entries()
    {
        BaseType::assign(other);
    }

    inline SegregatedHashTable(SegregatedHashTable&& other)
        : BaseType(), m_deleted_entries(),
          m_nonused_entries(), m_new_deleted_entries(),
          m_new_nonused_entries()
    {
        BaseType::assign(std::move(other));
        m_deleted_entries = std::move(other.m_deleted_entries);
        m_nonused_entries = std::move(other.m_nonused_entries);
    }

    template <typename InputIterator>
    inline SegregatedHashTable(const InputIterator& first, const InputIterator& last)
        : BaseType(), m_deleted_entries(),
          m_nonused_entries(), m_new_deleted_entries(),
          m_new_nonused_entries()
    {
        BaseType::assign(first, last);
    }

    template <typename InputIterator>
    inline SegregatedHashTable(const InputIterator& first, const InputIterator& last,
                               const T& deleted_value, const T& nonused_value)
        : BaseType(), m_deleted_entries(),
          m_nonused_entries(), m_new_deleted_entries(),
          m_new_nonused_entries()
    {
        BaseType::assign(first, last);
    }

    inline SegregatedHashTable(std::initializer_list<T> init_list)
        : BaseType(), m_deleted_entries(),
          m_nonused_entries(), m_new_deleted_entries(),
          m_new_nonused_entries()
    {
        BaseType::assign(init_list);
    }

    inline SegregatedHashTable(std::initializer_list<T> init_list,
                               const T& deleted_value,
                               const T& nonused_value)
        : BaseType(), m_deleted_entries(),
          m_nonused_entries(), m_new_deleted_entries(),
          m_new_nonused_entries()
    {
        BaseType::assign(init_list);
    }

    inline ~SegregatedHashTable()
    {
        // Nothing here
    }

    inline SegregatedHashTable& operator = (const SegregatedHashTable& other)
    {
        if (&other == this) {
            return *this;
        }
        BaseType::assign(other);
        return *this;
    }

    inline SegregatedHashTable& operator = (SegregatedHashTable&& other)
    {
        if (&other == this) {
            return *this;
        }
        BaseType::assign(std::move(other));
        m_deleted_entries = std::move(other.m_deleted_entries);
        m_nonused_entries = std::move(other.m_nonused_entries);
        return *this;
    }

    inline SegregatedHashTable& operator = (std::initializer_list<T> init_list)
    {
        BaseType::assign(init_list);
    }

    // for compatibility
    inline T get_deleted_value() const
    {
        return T();
    }

    inline T get_nonused_value() const
    {
        return T();
    }

    inline void set_deleted_value(const T& value) const
    {
        // nothing
    }

    inline void set_nonused_value(const T& value) const
    {
        // nothing
    }
};

template <typename T, typename HashFunction, typename EqualsFunction>
class RestrictedHashTableBase
    : protected HashTableImplBase<T, HashFunction, EqualsFunction,
                                  kc::hash_table_detail_::RestrictedHashTableBase, T>
{
protected:
    typedef T EntryType;
    typedef HashTableImplBase<T, HashFunction, EqualsFunction,
                              kc::hash_table_detail_::RestrictedHashTableBase, T> TableBaseType;
    friend TableBaseType;


    T m_deleted_value;
    T m_nonused_value;

    inline u64 get_index_for_entry(EntryType* entry) const
    {
        return (entry - this->m_table);
    }

    inline EntryType& get_value_ref(EntryType* entry) const
    {
        return *entry;
    }

    inline EntryType* get_value_ptr(EntryType* entry) const
    {
        return entry;
    }

    inline bool is_entry_nonused(EntryType* entry) const
    {
        return (*entry == m_nonused_value);
    }

    inline bool is_entry_deleted(EntryType* entry) const
    {
        return (*entry == m_deleted_value);
    }

    inline bool is_entry_used(EntryType* entry) const
    {
        return ((*entry != m_nonused_value) && (*entry != m_deleted_value));
    }

    inline void mark_entry_nonused(EntryType* entry) const
    {
        *entry = m_nonused_value;
    }

    inline void mark_entry_deleted(EntryType* entry) const
    {
        *entry = m_deleted_value;
    }

    inline void mark_entry_used(EntryType* entry) const
    {
        // do nothing
        return;
    }

    inline void begin_resize(u64 new_table_size) const
    {
        // do nothing
    }

    inline void end_resize(u64 new_table_size) const
    {
        // do nothing
    }

    inline bool is_new_entry_used(EntryType* new_table,
                                  u64 new_table_size,
                                  EntryType* entry) const
    {
        return is_entry_used(entry);
    }

    inline bool is_new_entry_nonused(EntryType* new_table,
                                     u64 new_table_size,
                                     EntryType* entry) const
    {
        return is_entry_nonused(entry);
    }

    inline void mark_new_entry_used(EntryType* new_table,
                                    u64 new_table_size,
                                    EntryType* entry) const
    {
        mark_entry_used(entry);
    }

    inline void mark_new_entry_nonused(EntryType* new_table,
                                       u64 new_table_size,
                                       EntryType* entry) const
    {
        mark_entry_nonused(entry);
    }

    inline void initialize_new_table(EntryType* new_table, u64 new_table_size) const
    {
        for (auto cur_ptr = new_table, last_ptr = new_table + new_table_size;
             cur_ptr != last_ptr; ++cur_ptr) {
            cur_ptr->~EntryType();
            new (cur_ptr) EntryType(m_nonused_value);
        }
    }

    inline void on_clear() const
    {
        // do nothing
    }

    inline void set_size(u64 size) const
    {
        // do nothing
    }

public:
    typedef typename TableBaseType::Iterator Iterator;
    typedef Iterator iterator;

    // We allow a default constructor, but the client
    // needs to set nonused value before any inserts
    // and deleted value before any erases
    inline RestrictedHashTableBase()
        : TableBaseType()
    {
        // Nothing here
    }

    inline RestrictedHashTableBase(const T& deleted_value, const T& nonused_value)
        : TableBaseType(), m_deleted_value(deleted_value), m_nonused_value(nonused_value)
    {
        // Nothing here
    }

    inline RestrictedHashTableBase(u64 initial_capacity)
        : TableBaseType()
    {
        TableBaseType::expand_table(initial_capacity);
    }

    inline RestrictedHashTableBase(u64 initial_capacity,
                                   const T& deleted_value,
                                   const T& nonused_value)
        : TableBaseType(), m_deleted_value(deleted_value), m_nonused_value(nonused_value)
    {
        TableBaseType::expand_table(initial_capacity);
    }

    inline RestrictedHashTableBase(const RestrictedHashTableBase& other)
        : TableBaseType(), m_deleted_value(other.m_deleted_value),
          m_nonused_value(other.m_nonused_value)
    {
        TableBaseType::assign(other);
    }

    inline RestrictedHashTableBase(RestrictedHashTableBase&& other)
        : TableBaseType(), m_deleted_value(other.m_deleted_value),
          m_nonused_value(other.m_nonused_value)
    {
        TableBaseType::assign(std::move(other));
    }

    template <typename InputIterator>
    inline RestrictedHashTableBase(const InputIterator& first,
                                   const InputIterator& last)
    {
        throw KinaraException("Restricted hash tables cannot be constructed merely from a range");
    }

    template <typename InputIterator>
    inline RestrictedHashTableBase(const InputIterator& first,
                                   const InputIterator& last,
                                   const T& deleted_value,
                                   const T& nonused_value)
        : TableBaseType(), m_deleted_value(deleted_value),
          m_nonused_value(nonused_value)
    {
        TableBaseType::assign(first, last);
    }

    inline RestrictedHashTableBase(std::initializer_list<T> init_list,
                                   const T& deleted_value,
                                   const T& nonused_value)
        : TableBaseType(), m_deleted_value(deleted_value),
          m_nonused_value(nonused_value)
    {
        TableBaseType::assign(init_list);
    }

    inline RestrictedHashTableBase(std::initializer_list<T> init_list)
    {
        throw KinaraException((std::string)"Restricted hash tables cannot be constructed " +
                              "merely from an initializer list");
    }

    inline ~RestrictedHashTableBase()
    {
        // Nothing here
    }

    inline RestrictedHashTableBase& operator = (const RestrictedHashTableBase& other)
    {
        if (&other == this) {
            return *this;
        }

        m_deleted_value = other.m_deleted_value;
        m_nonused_value = other.m_nonused_value;

        TableBaseType::assign(other);
        return *this;
    }

    inline RestrictedHashTableBase& operator = (RestrictedHashTableBase&& other)
    {
        if (&other == this) {
            return *this;
        }
        std::swap(m_deleted_value, other.m_deleted_value);
        std::swap(m_nonused_value, other.m_nonused_value);
        TableBaseType::assign(std::move(other));
        return *this;
    }

    inline RestrictedHashTableBase& operator = (std::initializer_list<T> init_list)
    {
        TableBaseType::assign(init_list);
        return *this;
    }

    inline const T& get_deleted_value() const
    {
        return m_deleted_value;
    }

    inline const T& get_nonused_value() const
    {
        return m_nonused_value;
    }

    inline void set_deleted_value(const T& deleted_value)
    {
        for (auto cur_ptr = this->m_table, last_ptr = this->m_table + this->m_table_size;
             cur_ptr != last_ptr; ++cur_ptr) {
            if (*cur_ptr == m_deleted_value) {
                *cur_ptr = deleted_value;
            }
        }
        m_deleted_value = deleted_value;
    }

    inline void set_nonused_value(const T& nonused_value)
    {
        for (auto cur_ptr = this->m_table, last_ptr = this->m_table + this->m_table_size;
             cur_ptr != last_ptr; ++cur_ptr) {
            if (*cur_ptr == m_nonused_value) {
                *cur_ptr = nonused_value;
            }
        }
        m_nonused_value = nonused_value;
    }
};

template <typename T, typename HashFunction, typename EqualsFunction>
class RestrictedHashTable : public RestrictedHashTableBase<T, HashFunction, EqualsFunction>
{
protected:
    typedef HashTableImplBase<T, HashFunction, EqualsFunction,
                              kc::hash_table_detail_::RestrictedHashTable, T> TableBaseType;
    typedef RestrictedHashTableBase<T, HashFunction, EqualsFunction> BaseType;
    friend TableBaseType;
    typedef T EntryType;

public:
    inline RestrictedHashTable()
        : BaseType()
    {
        // Nothing here
    }

    inline RestrictedHashTable(const T& deleted_value, const T& nonused_value)
        : BaseType(deleted_value, nonused_value)
    {
        // Nothing here
    }

    inline RestrictedHashTable(const RestrictedHashTable& other)
        : BaseType(other)
    {
        // Nothing here
    }

    inline RestrictedHashTable(RestrictedHashTable&& other)
        : BaseType(std::move(other))
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline RestrictedHashTable(const InputIterator& first,
                               const InputIterator& last)
        : BaseType(first, last)
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline RestrictedHashTable(const InputIterator& first,
                               const InputIterator& last,
                               const T& deleted_value,
                               const T& nonused_value)
        : BaseType(first, last, deleted_value, nonused_value)
    {
        // Nothing here
    }

    inline RestrictedHashTable(std::initializer_list<T> init_list)
        : BaseType(init_list)
    {
        // Nothing here
    }

    inline RestrictedHashTable(std::initializer_list<T> init_list,
                               const T& deleted_value,
                               const T& nonused_value)
        : BaseType(init_list, deleted_value, nonused_value)
    {
        // Nothing here
    }

    inline RestrictedHashTable& operator = (const RestrictedHashTable& other)
    {
        if (&other == this) {
            return *this;
        }
        BaseType::assign(other);
        return *this;
    }

    inline RestrictedHashTable& operator = (RestrictedHashTable&& other)
    {
        if (&other == this) {
            return *this;
        }
        BaseType::assign(std::move(other));
        return *this;
    }

    inline RestrictedHashTable& operator = (std::initializer_list<T> init_list)
    {
        BaseType::assign(init_list);
        return *this;
    }
};

template <typename T, typename HashFunction, typename EqualsFunction>
class RestrictedHashTable<T*, HashFunction, EqualsFunction>
    : public RestrictedHashTableBase<T*, HashFunction, EqualsFunction>
{
protected:
    typedef HashTableImplBase<T*, HashFunction, EqualsFunction,
                              kc::hash_table_detail_::RestrictedHashTable, T*> TableBaseType;
    typedef RestrictedHashTableBase<T*, HashFunction, EqualsFunction> BaseType;
    friend TableBaseType;
    typedef T* EntryType;

public:
    inline RestrictedHashTable()
        : BaseType((T*)(0x1), (T*)(0x0))
    {
        // Nothing here
    }

    inline RestrictedHashTable(T* deleted_value, T* nonused_value)
        : RestrictedHashTable()
    {
        // Nothing here
    }

    inline RestrictedHashTable(u64 initial_capacity)
        : BaseType(initial_capacity, (T*)(0x1), (T*)(0x0))
    {
        // Nothing here
    }

    inline RestrictedHashTable(u64 initial_capacity,
                               T* deleted_value,
                               T* nonused_value)
        : RestrictedHashTable(initial_capacity)
    {
        // Nothing here
    }

    inline RestrictedHashTable(const RestrictedHashTable& other)
        : BaseType(other)
    {
        // Nothing here
    }

    inline RestrictedHashTable(RestrictedHashTable&& other)
        : BaseType(std::move(other))
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline RestrictedHashTable(const InputIterator& first, const InputIterator& last)
        : BaseType(first, last, (T*)0x1, (T*)0x0)
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline RestrictedHashTable(const InputIterator& first, const InputIterator& last,
                               T* deleted_value, T* nonused_value)
        : RestrictedHashTable(first, last)
    {
        // Nothing here
    }

    inline RestrictedHashTable(std::initializer_list<T*> init_list)
        : RestrictedHashTable(init_list, (T*)0x1, (T*)0x0)
    {
        // Nothing here
    }

    inline RestrictedHashTable(std::initializer_list<T*> init_list,
                               T* deleted_value, T* nonused_value)
        : RestrictedHashTable(init_list)
    {
        // Nothing here
    }

    inline RestrictedHashTable& operator = (const RestrictedHashTable& other)
    {
        if (&other == this) {
            return *this;
        }
        BaseType::assign(other);
        return *this;
    }

    inline RestrictedHashTable& operator = (RestrictedHashTable&& other)
    {
        if (&other == this) {
            return *this;
        }
        BaseType::assign(std::move(other));
        return *this;
    }

    inline RestrictedHashTable& operator = (std::initializer_list<T*> init_list)
    {
        BaseType::assign(init_list);
        return *this;
    }
};


template <typename T, typename HashFunction, typename EqualsFunction>
class RestrictedHashTable<const T*, HashFunction, EqualsFunction>
    : public RestrictedHashTableBase<const T*, HashFunction, EqualsFunction>
{
protected:
    typedef HashTableImplBase<const T*, HashFunction, EqualsFunction,
                              kc::hash_table_detail_::RestrictedHashTable, const T*> TableBaseType;
    typedef RestrictedHashTableBase<const T*, HashFunction, EqualsFunction> BaseType;
    friend TableBaseType;
    typedef T* EntryType;

public:
    inline RestrictedHashTable()
        : BaseType((T*)(0x1), (T*)(0x0))
    {
        // Nothing here
    }

    inline RestrictedHashTable(const T* deleted_value, const T* nonused_value)
        : RestrictedHashTable()
    {
        // Nothing here
    }

    inline RestrictedHashTable(u64 initial_capacity)
        : BaseType(initial_capacity, (T*)(0x1), (T*)(0x0))
    {
        // Nothing here
    }

    inline RestrictedHashTable(u64 initial_capacity,
                               const T* deleted_value,
                               const T* nonused_value)
        : RestrictedHashTable(initial_capacity)
    {
        // Nothing here
    }

    inline RestrictedHashTable(const RestrictedHashTable& other)
        : BaseType(other)
    {
        // Nothing here
    }

    inline RestrictedHashTable(RestrictedHashTable&& other)
        : BaseType(std::move(other))
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline RestrictedHashTable(const InputIterator& first, const InputIterator& last)
        : BaseType(first, last, (T*)0x1, (T*)0x0)
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline RestrictedHashTable(const InputIterator& first, const InputIterator& last,
                               const T* deleted_value, const T* nonused_value)
        : RestrictedHashTable(first, last)
    {
        // Nothing here
    }

    inline RestrictedHashTable(std::initializer_list<const T*> init_list)
        : RestrictedHashTable(init_list, (T*)0x1, (T*)0x0)
    {
        // Nothing here
    }

    inline RestrictedHashTable(std::initializer_list<T*> init_list,
                               const T* deleted_value, const T* nonused_value)
        : RestrictedHashTable(init_list)
    {
        // Nothing here
    }

    inline RestrictedHashTable& operator = (const RestrictedHashTable& other)
    {
        if (&other == this) {
            return *this;
        }
        BaseType::assign(other);
        return *this;
    }

    inline RestrictedHashTable& operator = (RestrictedHashTable&& other)
    {
        if (&other == this) {
            return *this;
        }
        BaseType::assign(std::move(other));
        return *this;
    }

    inline RestrictedHashTable& operator = (std::initializer_list<const T*> init_list)
    {
        BaseType::assign(init_list);
        return *this;
    }
};

} /* end namespace hash_table_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_HASH_TABLE_HPP_ */

//
// HashTable.hpp ends here
