// HashTableTypes.hpp ---
//
// Filename: HashTableTypes.hpp
// Author: Abhishek Udupa
// Created: Tue Mar 17 11:45:08 2015 (-0400)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_HASH_TABLE_TYPES_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_HASH_TABLE_TYPES_HPP_

namespace kinara {
namespace containers {
namespace hash_table_detail_ {

namespace ka = kinara::allocators;
namespace kc = kinara::containers;
namespace ku = kinara::utils;

template <typename T>
class HashEntry
{
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
template <typename T, typename HashFunction,
          typename EqualsFunction>
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

} /* end namespace hash_table_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_HASH_TABLE_TYPES_HPP_ */

//
// HashTableTypes.hpp ends here
