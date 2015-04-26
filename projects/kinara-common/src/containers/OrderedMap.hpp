// OrderedMap.hpp ---
// Filename: OrderedMap.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 02:09:31 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_ORDERED_MAP_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_ORDERED_MAP_HPP_

#include "OrderedSet.hpp"

namespace kinara {
namespace containers {
namespace ordered_map_detail_ {

namespace kc = kinara::containers;
namespace ka = kinara::allocators;

// forward declaration for use in iterator
template <typename KeyType, typename MappedType, typename KeyHash,
          typename KeyEquals, typename KeyLess>
class OrderedMapBase;

template <typename ValueType, typename OrderedMapType,
          typename ListIteratorType, bool ISCONST>
class OMIterator : public std::iterator<std::bidirectional_iterator_tag, ValueType, i64,
                                        typename std::conditional<ISCONST,
                                                                  const ValueType*,
                                                                  ValueType*>::type,
                                        typename std::conditional<ISCONST,
                                                                  const ValueType&,
                                                                  ValueType&>::type>
{
    friend OrderedMapType;
    friend class kc::ordered_map_detail_::OMIterator<ValueType, OrderedMapType,
                                                     ListIteratorType, false>;
    friend class kc::ordered_map_detail_::OMIterator<ValueType, OrderedMapType,
                                                     ListIteratorType, true>;

private:
    OrderedMapType* m_ordered_map;
    ListIteratorType m_list_iterator;

    typedef typename std::conditional<ISCONST, const ValueType*, ValueType*>::type PtrType;
    typedef typename std::conditional<ISCONST, const ValueType&, ValueType&>::type RefType;

public:
    inline OMIterator()
        : m_ordered_map(nullptr), m_list_iterator()
    {
        // Nothing here
    }

    inline OMIterator(OrderedMapType* ordered_map, const ListIteratorType& list_iterator)
        : m_ordered_map(ordered_map), m_list_iterator(list_iterator)
    {
        // Nothing here
    }

    inline OMIterator(const OMIterator& other)
        : m_ordered_map(other.m_ordered_map), m_list_iterator(other.m_list_iterator)
    {
        // Nothing here
    }

    template <bool OISCONST>
    inline OMIterator(const kc::ordered_map_detail_::OMIterator<ValueType, OrderedMapType,
                                                                ListIteratorType,
                                                                OISCONST>& other)
        : m_ordered_map(other.m_ordered_map), m_list_iterator(other.m_list_iterator)
    {
        static_assert(!OISCONST || ISCONST,
                      "Cannot construct non-const iterator from const iterator");
    }

    inline OMIterator& operator = (const OMIterator& other) const
    {
        if (&other == this) {
            return *this;
        }
        m_ordered_map = other.m_ordered_map;
        m_list_iterator = other.m_list_iterator;
        return *this;
    }

    template <bool OISCONST>
    inline OMIterator&
    operator = (const kc::ordered_map_detail_::OMIterator<ValueType, OrderedMapType,
                                                          ListIteratorType, OISCONST>& other)
    {
        static_assert(!OISCONST || ISCONST,
                      "Cannot assign const iterator to non-const iterator");
        m_ordered_map = other.m_ordered_map;
        m_list_iterator = other.m_list_iterator;
        return *this;
    }

    template <bool OISCONST>
    inline bool
    operator == (const kc::ordered_map_detail_::OMIterator<ValueType, OrderedMapType,
                                                           ListIteratorType, OISCONST>& other)
        const
    {
        return (m_list_iterator == other.m_list_iterator);
    }

    template <bool OISCONST>
    inline bool
    operator != (const kc::ordered_map_detail_::OMIterator<ValueType, OrderedMapType,
                                                           ListIteratorType, OISCONST>& other)
        const
    {
        return (m_list_iterator != other.m_list_iterator);
    }

    inline OMIterator& operator ++ ()
    {
        m_ordered_map->merge_newly_inserted_elements();
        ++m_list_iterator;
        return *this;
    }

    inline OMIterator& operator -- ()
    {
        m_ordered_map->merge_newly_inserted_elements();
        --m_list_iterator;
        return *this;
    }

    inline OMIterator& operator ++ (int unused)
    {
        auto retval = *this;
        ++(*this);
        return retval;
    }

    inline OMIterator& operator -- (int unused)
    {
        auto retval = *this;
        --(*this);
        return retval;
    }

    inline RefType operator * () const
    {
        return *m_list_iterator;
    }

    inline PtrType operator -> () const
    {
        return (&m_list_iterator)->operator->();
    }
};

template <typename KeyType, typename MappedType, typename KeyHash,
          typename KeyEquals, typename KeyLess>
class OrderedMapBase
{
public:
    typedef std::pair<const KeyType, MappedType> ValueType;
    typedef ValueType& RefType;
    typedef ValueType* PtrType;
    typedef const ValueType& ConstRefType;
    typedef const ValueType* ConstPtrType;

private:
    typedef PoolDList<ValueType> ListType;
    typedef typename ListType::Iterator ListIterator;
    typedef ListIterator HashTableValueType;

    class HashTableEqualsFunction : private KeyEquals
    {
    public:
        inline bool operator () (const HashTableValueType& value1,
                                 const HashTableValueType& value2) const
        {
            return KeyEquals::operator() (value1->first, value2->first);
        }

        inline bool operator () (const HashTableValueType& value1, const KeyType& value2) const
        {
            return KeyEquals::operator() (value1->first, value2);
        }

        inline bool operator () (const KeyType& value1, const HashTableValueType& value2) const
        {
            return KeyEquals::operator() (value1, value2->first);
        }

        inline bool operator () (const KeyType& value1, const KeyType& value2) const
        {
            return KeyEquals::operator() (value1, value2);
        }
    };

    class HashTableHashFunction : private KeyHash
    {
    public:
        inline u64 operator () (const HashTableValueType& value) const
        {
            return KeyHash::operator() (value->first);
        }

        inline u64 operator () (const KeyType& value) const
        {
            return KeyHash::operator() (value);
        }
    };

    class PairLessFunction : private KeyLess
    {
    public:
        inline bool operator () (const ValueType& value1, const ValueType& value2)
        {
            return KeyLess::operator() (value1.first, value2.first);
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
    typedef OMIterator<ValueType, OrderedMapBase, ListIterator, false> Iterator;
    typedef OMIterator<ValueType, OrderedMapBase, ListIterator, true> ConstIterator;
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    typedef std::reverse_iterator<Iterator> ReverseIterator;
    typedef ReverseIterator reverse_iterator;
    typedef std::reverse_iterator<ConstIterator> ConstReverseIterator;
    typedef ConstReverseIterator const_reverse_iterator;

    OrderedMapBase()
        : m_pool_allocator(ka::allocate_object_raw<ka::PoolAllocator>(ListType::sc_node_size)),
          m_sorted_list(m_pool_allocator), m_insertion_list(m_pool_allocator),
          m_hash_table(m_sorted_list.end(), HashTableValueType())
    {
        // Nothing here
    }

    template <typename InputIterator>
    OrderedMapBase(const InputIterator& first, const InputIterator& last)
        : OrderedMapBase()
    {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    OrderedMapBase(std::initializer_list<ValueType> init_list)
        : OrderedMapBase(init_list.begin(), init_list.end())
    {
        // Nothing here
    }

    OrderedMapBase(const OrderedMapBase& other)
        : OrderedMapBase(other.begin(), other.end())
    {
        // Nothing here
    }

    OrderedMapBase(OrderedMapBase&& other)
        : m_pool_allocator(nullptr), m_sorted_list(), m_insertion_list(),
          m_hash_table(m_sorted_list.end(), HashTableValueType())
    {
        std::swap(m_pool_allocator, other.m_pool_allocator);
        std::swap(m_sorted_list, other.m_sorted_list);
        std::swap(m_insertion_list, other.m_insertion_list);
        std::swap(m_hash_table, other.m_hash_table);
    }

    ~OrderedMapBase()
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

    inline OrderedMapBase& operator = (const OrderedMapBase& other)
    {
        if (&other == this) {
            return *this;
        }
        clear();
        insert(other.begin(), other.end());
        return *this;
    }

    inline OrderedMapBase& operator = (OrderedMapBase&& other)
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

    inline OrderedMapBase& operator = (std::initializer_list<ValueType> init_list)
    {
        clear();
        insert(init_list.begin(), init_list.end());
        return *this;
    }

    inline ConstIterator begin() const
    {
        merge_newly_inserted_elements();
        return ConstIterator(const_cast<OrderedMapBase*>(this), m_sorted_list.begin());
    }

    inline ConstIterator end() const
    {
        return ConstIterator(const_cast<OrderedMapBase*>(this), m_sorted_list.end());
    }

    inline Iterator begin()
    {
        merge_newly_inserted_elements();
        return Iterator(this, m_sorted_list.begin());
    }

    inline Iterator end()
    {
        return Iterator(this, m_sorted_list.end());
    }

    inline ConstIterator cbegin() const
    {
        return begin();
    }

    inline ConstIterator cend() const
    {
        return end();
    }

    inline ReverseIterator rbegin()
    {
        return ReverseIterator(end());
    }

    inline ReverseIterator rend()
    {
        return ReverseIterator(begin());
    }

    inline ConstReverseIterator rbegin() const
    {
        return ConstReverseIterator(end());
    }

    inline ConstReverseIterator rend() const
    {
        return ConstReverseIterator(begin());
    }

    inline ConstReverseIterator crbegin() const
    {
        return rbegin();
    }

    inline ConstReverseIterator crend() const
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
        PairLessFunction less_fun;
        m_insertion_list.sort(less_fun);
        m_sorted_list.merge(m_insertion_list, less_fun);
    }

    inline MappedType& operator [] (const KeyType& key)
    {
        auto it = find(key);
        if (it == end()) {
            auto&& itpair = insert(std::pair<const KeyType, MappedType>(key, MappedType()));
            return itpair.first->second;
        } else {
            return it->second;
        }
    }

    inline MappedType& operator [] (KeyType&& key)
    {
        auto it = find(key);
        if (it == end()) {
            auto&& itpair = insert(std::pair<const KeyType, MappedType>(std::move(key),
                                                                        MappedType()));
            return itpair.first->second;
        } else {
            return it->second;
        }
    }

    inline MappedType& at(const KeyType& key)
    {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("Key not found in Kinara::UnorderedMap::at()");
        }
        return it->second;
    }

    inline const MappedType& at(const KeyType& key) const
    {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("Key not found in Kinara::UnorderedMap::at()");
        }
        return it->second;
    }

    template <typename U>
    inline Iterator find(const U& value)
    {
        auto hash_iter = m_hash_table.find(value);
        if (hash_iter == m_hash_table.end()) {
            return end();
        } else {
            return Iterator(this, *hash_iter);
        }
    }

    template <typename U>
    inline ConstIterator find(const U& value) const
    {
        auto hash_iter = m_hash_table.find(value);
        if (hash_iter == m_hash_table.end()) {
            return end();
        } else {
            return ConstIterator(const_cast<OrderedMapBase*>(this), *hash_iter);
        }
    }

    template <typename... ArgTypes>
    inline std::pair<Iterator, bool> emplace(ArgTypes&&... args)
    {
        ValueType value_copy(std::forward<ArgTypes>(args)...);
        auto it = find(value_copy.first);
        if (it == end()) {
            auto list_iter = m_insertion_list.insert(m_insertion_list.end(), std::move(value_copy));
            m_hash_table.insert(list_iter);
            return std::make_pair(Iterator(this, list_iter), true);
        } else {
            return std::make_pair(it, false);
        }
    }

    inline std::pair<Iterator, bool> insert(const ValueType& value)
    {
        auto it = find(value.first);
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
        auto it = find(value.first);
        if (it != end()) {
            return std::make_pair(it, false);
        } else {
            auto list_iter = m_insertion_list.insert(m_insertion_list.end(), std::move(value));
            m_hash_table.insert(list_iter);
            return std::make_pair(Iterator(this, list_iter), true);
        }
    }

    inline std::pair<Iterator, bool> insert(const std::pair<KeyType, MappedType>& value)
    {
        return insert(std::make_pair<const KeyType, MappedType>(value.first, value.second));
    }

    inline std::pair<Iterator, bool> insert(const KeyType& key, const MappedType& value)
    {
        return insert(std::pair<const KeyType, MappedType>(key, value));
    }

    template <typename P,
              typename Unused =
              typename std::enable_if<std::is_constructible<ValueType, P&&>::value>::type>
    inline std::pair<Iterator, bool> insert(P&& value)
    {
        ValueType value_copy(std::move(value));
        return insert(std::move(value_copy));
    }

    template <typename InputIterator>
    inline void insert(const InputIterator& first, const InputIterator& last)
    {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    inline void insert(std::initializer_list<ValueType> init_list)
    {
        insert(init_list.begin(), init_list.end());
    }

    Iterator erase(const ConstIterator& position)
    {
        merge_newly_inserted_elements();
        m_hash_table.erase(position.m_list_iterator);
        return Iterator(this, m_sorted_list.erase(position.m_list_iterator));
    }

    u64 erase(const KeyType& key)
    {
        auto it = find(key);
        if (it == end()) {
            return size();
        }
        erase(it);
        return size();
    }

    inline Iterator erase(const ConstIterator& first, const ConstIterator& last)
    {
        Iterator retval;
        for (auto it = first; it != last; ++it) {
            retval = erase(it);
        }
        return retval;
    }

    inline void swap(OrderedMapBase& other)
    {
        std::swap(*this, other);
    }

    inline u64 count(const KeyType& key)
    {
        return (find(key) == end() ? 0 : 1);
    }

    // not part of stl
    inline void shrink_to_fit()
    {
        merge_newly_inserted_elements();
        m_hash_table.shrink_to_fit();
        m_pool_allocator->garbage_collect();
    }

    inline Iterator lower_bound(const KeyType& key)
    {
        KeyLess less_func;
        merge_newly_inserted_elements();
        for (auto it = m_sorted_list.begin(), last = m_sorted_list.end(); it != last; ++it) {
            if (!less_func(*it, key)) {
                return Iterator(this, it);
            }
        }
        return end();
    }

    inline ConstIterator lower_bound(const KeyType& key) const
    {
        KeyLess less_func;
        merge_newly_inserted_elements();
        for (auto it = m_sorted_list.begin(), last = m_sorted_list.end(); it != last; ++it) {
            if (!less_func(*it, key)) {
                return ConstIterator(const_cast<OrderedMapBase*>(this), it);
            }
        }
        return end();
    }

    inline Iterator upper_bound(const KeyType& key)
    {
        KeyLess less_func;
        merge_newly_inserted_elements();
        for (auto it = m_sorted_list.begin(), last = m_sorted_list.end(); it != last; ++it) {
            if (less_func(key, *it)) {
                return Iterator(this, it);
            }
        }
        return end();
    }

    inline ConstIterator upper_bound(const KeyType& key) const
    {
        KeyLess less_func;
        merge_newly_inserted_elements();
        for (auto it = m_sorted_list.begin(), last = m_sorted_list.end(); it != last; ++it) {
            if (less_func(key, *it)) {
                return ConstIterator(const_cast<OrderedMapBase*>(this), it);
            }
        }
        return end();
    }

    inline i64 compare(const OrderedMapBase& other) const
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

        KeyLess less_func;
        std::less<MappedType> std_less;
        while (it1 != end1 && it2 != end2) {
            if (less_func(it1->first, it2->first)) {
                return -1;
            } else if (less_func(it2->first, it1->first)) {
                return 1;
            } else if (std_less(it1->second, it2->second)) {
                return -1;
            } else if (std_less(it2->second, it1->second)) {
                return 1;
            }
        }
        return 0;
    }

    inline bool operator == (const OrderedMapBase& other) const
    {
        return (compare(other) == 0);
    }

    inline bool operator != (const OrderedMapBase& other) const
    {
        return (compare(other) != 0);
    }

    inline bool operator < (const OrderedMapBase& other) const
    {
        return (compare(other) < 0);
    }

    inline bool operator <= (const OrderedMapBase& other) const
    {
        return (compare(other) <= 0);
    }

    inline bool operator > (const OrderedMapBase& other) const
    {
        return (compare(other) > 0);
    }

    inline bool operator >= (const OrderedMapBase& other) const
    {
        return (compare(other) >= 0);
    }
};

} /* end namespace ordered_map_detail_ */

// some useful typedefs
template <typename KeyType, typename MappedType,
          typename KeyHash = ku::Hasher<KeyType>,
          typename KeyEquals = ku::Equal<KeyType>,
          typename KeyLess = ku::Less<KeyType> >
using OrderedMap = ordered_map_detail_::OrderedMapBase<KeyType, MappedType,
                                                       KeyHash, KeyEquals, KeyLess>;

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_ORDERED_MAP_HPP_ */

//
// OrderedMap.hpp ends here
