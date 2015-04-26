// OrderedSet.hpp ---
// Filename: OrderedSet.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 02:09:26 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_ORDERED_SET_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_ORDERED_SET_HPP_

#include "UnorderedSet.hpp"
#include "DList.hpp"

namespace kinara {
namespace containers {
namespace ordered_set_detail_ {

namespace kc = kinara::containers;
namespace ka = kinara::allocators;

template <typename T, typename HashFunction, typename EqualsFunction, typename LessFunction>
class OrderedSetBase
{
private:
    typedef PoolDList<T> ListType;
    typedef typename ListType::Iterator ListIterator;
    typedef ListIterator HashTableValueType;

    class HashTableEqualsFunction : private EqualsFunction
    {
    public:
        inline bool operator () (const HashTableValueType& value1,
                                 const HashTableValueType& value2) const
        {
            return EqualsFunction::operator()(*value1, *value2);
        }

        inline bool operator () (const HashTableValueType& value1, const T& value2) const
        {
            return EqualsFunction::operator()(*value1, value2);
        }

        inline bool operator () (const T& value1, const HashTableValueType& value2) const
        {
            return EqualsFunction::operator()(value1, value2);
        }
    };

    class HashTableHashFunction : private HashFunction
    {
    public:
        inline u64 operator () (const HashTableValueType& value) const
        {
            return HashFunction::operator()(*value);
        }

        inline u64 operator () (const T& value) const
        {
            return HashFunction::operator()(value);
        }
    };

    typedef
    RestrictedUnorderedSet<HashTableValueType, HashTableHashFunction, HashTableEqualsFunction>
    HashTableType;

    ka::PoolAllocator* m_pool_allocator;
    mutable ListType m_sorted_list;
    mutable ListType m_insertion_list;
    HashTableType m_hash_table;

public:
    typedef T ValueType;
    typedef T& RefType;
    typedef T* PtrType;
    typedef const T& ConstRefType;
    typedef const T* ConstPtrType;

    class Iterator : public std::iterator<std::bidirectional_iterator_tag,
                                          T, i64, const T*, const T&>
    {
        friend class OrderedSetBase;

    private:
        OrderedSetBase* m_ordered_set;
        ListIterator m_list_iterator;

    public:
        inline Iterator()
            : m_ordered_set(nullptr), m_list_iterator()
        {
            // Nothing here
        }

        inline Iterator(OrderedSetBase* ordered_set, const ListIterator& list_iterator)
            : m_ordered_set(ordered_set), m_list_iterator(list_iterator)
        {
            // Nothing here
        }

        inline Iterator(const Iterator& other)
            : m_ordered_set(other.m_ordered_set), m_list_iterator(other.m_list_iterator)
        {
            // Nothing here
        }

        inline Iterator& operator = (const Iterator& other)
        {
            if (&other == this) {
                return *this;
            }
            m_ordered_set = other.m_ordered_set;
            m_list_iterator = other.m_list_iterator;
            return *this;
        }

        inline bool operator == (const Iterator& other) const
        {
            return (m_list_iterator == other.m_list_iterator);
        }

        inline bool operator != (const Iterator& other) const
        {
            return (m_list_iterator != other.m_list_iterator);
        }

        inline Iterator& operator ++ ()
        {
            m_ordered_set->merge_newly_inserted_elements();
            ++m_list_iterator;
            return *this;
        }

        inline Iterator& operator -- ()
        {
            m_ordered_set->merge_newly_inserted_elements();
            --m_list_iterator;
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

        inline const T& operator * () const
        {
            return *m_list_iterator;
        }

        inline const T* operator -> () const
        {
            return (&m_list_iterator)->operator->();
        }
    };

    typedef Iterator iterator;
    typedef Iterator ConstIterator;
    typedef ConstIterator const_iterator;
    typedef std::reverse_iterator<Iterator> ReverseIterator;
    typedef ReverseIterator reverse_iterator;
    typedef std::reverse_iterator<ConstIterator> ConstReverseIterator;
    typedef ConstReverseIterator const_reverse_iterator;

    OrderedSetBase()
        : m_pool_allocator(ka::allocate_object_raw<ka::PoolAllocator>(ListType::sc_node_size)),
          m_sorted_list(m_pool_allocator), m_insertion_list(m_pool_allocator),
          m_hash_table(m_sorted_list.end(), HashTableValueType())
    {
        // Nothing here
    }

    template <typename InputIterator>
    OrderedSetBase(const InputIterator& first, const InputIterator& last)
        : OrderedSetBase()
    {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    OrderedSetBase(std::initializer_list<T> init_list)
        : OrderedSetBase(init_list.begin(), init_list.end())
    {
        // Nothing here
    }

    OrderedSetBase(const OrderedSetBase& other)
        : OrderedSetBase(other.begin(), other.end())
    {
        // Nothing here
    }

    OrderedSetBase(OrderedSetBase&& other)
        : m_pool_allocator(nullptr), m_sorted_list(), m_insertion_list(),
          m_hash_table(m_sorted_list.end(), HashTableValueType())
    {
        std::swap(m_pool_allocator, other.m_pool_allocator);
        std::swap(m_sorted_list, other.m_sorted_list);
        std::swap(m_insertion_list, other.m_insertion_list);
        std::swap(m_hash_table, other.m_hash_table);
    }

    ~OrderedSetBase()
    {
        m_sorted_list.clear();
        m_insertion_list.clear();
        if (m_pool_allocator != nullptr) {
            ka::deallocate_object_raw(m_pool_allocator, sizeof(ka::PoolAllocator));
        }
        m_hash_table.clear();
    }

    inline void clear()
    {
        m_sorted_list.clear();
        m_insertion_list.clear();
        m_pool_allocator->garbage_collect();
        m_hash_table.clear();
    }

    inline OrderedSetBase& operator = (const OrderedSetBase& other)
    {
        if (&other == this) {
            return *this;
        }
        clear();
        insert(other.begin(), other.end());
        return *this;
    }

    inline OrderedSetBase& operator = (std::initializer_list<T> init_list)
    {
        clear();
        insert(init_list);
        return *this;
    }

    inline OrderedSetBase& operator = (OrderedSetBase&& other)
    {
        if (&other == this) {
            return *this;
        }

        clear();
        std::swap(m_sorted_list, other.m_sorted_list);
        std::swap(m_insertion_list, other.m_insertion_list);
        std::swap(m_hash_table, other.m_hash_table);
        std::swap(m_pool_allocator, other.m_pool_allocator);
        return *this;
    }

    inline Iterator begin() const
    {
        merge_newly_inserted_elements();
        return Iterator(const_cast<OrderedSetBase*>(this), m_sorted_list.begin());
    }

    inline Iterator end() const
    {
        return Iterator(const_cast<OrderedSetBase*>(this), m_sorted_list.end());
    }

    inline Iterator cbegin() const
    {
        return begin();
    }

    inline Iterator cend() const
    {
        return end();
    }

    inline ReverseIterator rbegin() const
    {
        return ReverseIterator(end());
    }

    inline ReverseIterator rend() const
    {
        return ReverseIterator(begin());
    }

    inline ReverseIterator crbegin() const
    {
        return rbegin();
    }

    inline ReverseIterator crend() const
    {
        return rend();
    }

    inline bool empty() const
    {
        return ((m_sorted_list.size() + m_insertion_list.size()) == 0);
    }

    inline u64 size() const
    {
        return (m_sorted_list.size() + m_insertion_list.size());
    }

    inline u64 max_size() const
    {
        return UINT64_MAX;
    }

    inline void merge_newly_inserted_elements() const
    {
        LessFunction less_fun;
        m_insertion_list.sort(less_fun);
        m_sorted_list.merge(m_insertion_list, less_fun);
    }

    inline Iterator find(const ValueType& value) const
    {
        auto hash_iter = m_hash_table.find(value);
        if (hash_iter == m_hash_table.end()) {
            return end();
        } else {
            return Iterator(const_cast<OrderedSetBase*>(this), *hash_iter);
        }
    }

    inline std::pair<Iterator, bool> insert(const ValueType& value)
    {
        auto it = find(value);
        if (it != end()) {
            return make_pair(it, false);
        } else {
            auto list_iter = m_insertion_list.insert(m_insertion_list.end(), value);
            m_hash_table.insert(list_iter);
            return make_pair(Iterator(this, list_iter), true);
        }
    }

    inline std::pair<Iterator, bool> insert(ValueType&& value)
    {
        auto it = find(value);
        if (it != end()) {
            return make_pair(it, false);
        } else {
            auto list_iter = m_insertion_list.insert(m_insertion_list.end(), std::move(value));
            m_hash_table.insert(list_iter);
            return make_pair(Iterator(this, list_iter), true);
        }
    }

    template <typename InputIterator>
    inline void insert(const InputIterator& first, const InputIterator& last)
    {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    void insert(std::initializer_list<T> init_list)
    {
        insert(init_list.begin(), init_list.end());
    }

    Iterator erase(const Iterator& position)
    {
        merge_newly_inserted_elements();
        m_hash_table.erase(position.m_list_iterator);
        return Iterator(this, m_sorted_list.erase(position.m_list_iterator));
    }

    u64 erase(const ValueType& value)
    {
        auto it = find(value);
        if (it == end()) {
            return size();
        }

        erase(it);
        return size();
    }

    Iterator erase(const Iterator& first, const Iterator& last)
    {
        Iterator retval;
        for (auto it = first; it != last; ++it) {
            retval = erase(it);
        }
        return retval;
    }

    inline void swap(OrderedSetBase& other)
    {
        std::swap(*this, other);
    }

    template <typename... ArgTypes>
    inline std::pair<Iterator, bool> emplace(ArgTypes&&... args)
    {
        ValueType copy(std::forward<ArgTypes>(args)...);
        return insert(std::move(copy));
    }

    inline u64 count(const ValueType& value)
    {
        return (find(value) != end() ? 1 : 0);
    }

    // not part of stl
    inline void shrink_to_fit()
    {
        merge_newly_inserted_elements();
        m_hash_table.shrink_to_fit();
        m_pool_allocator->garbage_collect();
    }

    inline Iterator lower_bound(const ValueType& value) const
    {
        LessFunction less_func;
        merge_newly_inserted_elements();
        for (auto it = m_sorted_list.begin(), last = m_sorted_list.end(); it != last; ++it) {
            if (!less_func(*it, value)) {
                return Iterator(this, it);
            }
        }
        return end();
    }

    inline Iterator upper_bound(const ValueType& value) const
    {
        LessFunction less_func;
        merge_newly_inserted_elements();
        for (auto it = m_sorted_list.begin(), last = m_sorted_list.end(); it != last; ++it) {
            if (less_func(value, it)) {
                return Iterator(this, it);
            }
        }
        return end();
    }

    inline i64 compare(const OrderedSetBase& other) const
    {
        auto sz = size();
        auto diff = sz - other.size();
        if (diff != 0) {
            return diff;
        }
        auto it1 = begin();
        auto it2 = other.begin();
        auto end1 = end();
        auto end2 = other.end();

        LessFunction less_func;
        while (it1 != end1 && it2 != end2) {
            if (less_func(*it1, *it2)) {
                return -1;
            } else if (less_func(*it2, *it1)) {
                return 1;
            }
        }
        return 0;
    }

    inline bool operator == (const OrderedSetBase& other) const
    {
        return (compare(other) == 0);
    }

    inline bool operator != (const OrderedSetBase& other) const
    {
        return (compare(other) != 0);
    }

    inline bool operator < (const OrderedSetBase& other) const
    {
        return (compare(other) < 0);
    }

    inline bool operator <= (const OrderedSetBase& other) const
    {
        return (compare(other) <= 0);
    }

    inline bool operator > (const OrderedSetBase& other) const
    {
        return (compare(other) > 0);
    }

    inline bool operator >= (const OrderedSetBase& other) const
    {
        return (compare(other) >= 0);
    }
};

} /* end namespace ordered_set_detail_ */

// Some useful typedefs
template <typename T, typename HashFunction = ku::Hasher<T>,
          typename EqualsFunction = ku::Equal<T>, typename LessFunction = ku::Less<T> >
using OrderedSet = ordered_set_detail_::OrderedSetBase<T, HashFunction,
                                                       EqualsFunction, LessFunction>;

typedef OrderedSet<u08> u08OrderedSet;
typedef OrderedSet<u16> u16OrderedSet;
typedef OrderedSet<u32> u32OrderedSet;
typedef OrderedSet<u64> u64OrderedSet;

typedef OrderedSet<i08> i08OrderedSet;
typedef OrderedSet<i16> i16OrderedSet;
typedef OrderedSet<i32> i32OrderedSet;
typedef OrderedSet<i64> i64OrderedSet;

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_ORDERED_SET_HPP_ */

//
// OrderedSet.hpp ends here
