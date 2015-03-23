// UnorderedMap.hpp ---
// Filename: UnorderedMap.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 02:09:19 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_UNORDERED_MAP_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_UNORDERED_MAP_HPP_

#include <stdexcept>

#include "../hashfuncs/Hash.hpp"
#include "../basetypes/Comparators.hpp"

#include "HashTable.hpp"

namespace kinara {
namespace containers {

namespace kc = kinara::containers;
namespace ku = kinara::utils;

namespace unordered_map_detail_ {

template <typename KeyType, typename ValueType, typename HashFunction>
class KeyValuePairHasher : private HashFunction
{
public:
    inline u64 operator () (const std::pair<const KeyType, ValueType>& pair_object) const
    {
        return HashFunction::operator()(pair_object.first);
    }
};

template <typename KeyType, typename ValueType, typename EqualsFunction>
class KeyValuePairEquals : private EqualsFunction
{
public:
    inline bool operator () (const std::pair<const KeyType, ValueType>& pair_object1,
                             const std::pair<const KeyType, ValueType>& pair_object2) const
    {
        return EqualsFunction::operator()(pair_object1.first, pair_object2.first);
    }
};

template <typename MappedKeyType, typename MappedValueType,
          typename HashFunction, typename EqualsFunction,
          template <typename, typename, typename> class HashTableTemplateType>
class UnorderedMapBase : private HashTableTemplateType<std::pair<const MappedKeyType,
                                                                 MappedValueType>,
                                                       KeyValuePairHasher<MappedKeyType,
                                                                          MappedValueType,
                                                                          HashFunction>,
                                                       KeyValuePairEquals<MappedKeyType,
                                                                          MappedValueType,
                                                                          EqualsFunction>>
{
private:
    typedef HashTableTemplateType<std::pair<const MappedKeyType,
                                            MappedValueType>,
                                  KeyValuePairHasher<MappedKeyType,
                                                     MappedValueType,
                                                     HashFunction>,
                                  KeyValuePairEquals<MappedKeyType,
                                                     MappedValueType,
                                                     EqualsFunction>> HashTableType;
public:
    typedef std::pair<const MappedKeyType, MappedValueType> ValueType;
    typedef ValueType value_type;

    class Iterator : private HashTableType::Iterator,
                     public std::iterator<std::bidirectional_iterator_tag, ValueType,
                                          i64, ValueType*, ValueType&>
    {
    private:
        typedef typename HashTableType::Iterator BaseType;

    public:
        using BaseType::BaseType;
        inline Iterator(const BaseType& other)
            : BaseType(other)
        {
            // Nothing here
        }

        using BaseType::operator=;
        using BaseType::operator++;
        using BaseType::operator--;

        inline bool operator == (const Iterator& other) const
        {
            return BaseType::operator==(other);
        }

        inline bool operator != (const Iterator& other) const
        {
            return BaseType::operator!=(other);
        }

        inline ValueType& operator * () const
        {
            return BaseType::operator*();
        }

        inline ValueType* operator -> () const
        {
            return BaseType::operator->();
        }
    };

    class ConstIterator : private HashTableType::Iterator,
                          public std::iterator<std::bidirectional_iterator_tag, ValueType,
                                               i64, const ValueType*, const ValueType&>
    {
    private:
        typedef typename HashTableType::Iterator BaseType;

    public:
        using BaseType::BaseType;
        inline ConstIterator(const BaseType& other)
            : BaseType(other)
        {
            // Nothing here
        }

        // A regular iterator can be converted into a const iterator
        inline ConstIterator(const Iterator& other)
            : BaseType(other)
        {
            // Nothing here
        }

        using BaseType::operator=;

        // A regular iterator can be assigned to a const iterator
        inline ConstIterator& operator = (const Iterator& other)
        {
            return BaseType::operator=(other);
        }

        using BaseType::operator++;
        using BaseType::operator--;

        inline bool operator == (const ConstIterator& other) const
        {
            return BaseType::operator==(other);
        }

        inline bool operator != (const ConstIterator& other) const
        {
            return BaseType::operator!=(other);
        }

        inline const ValueType& operator * () const
        {
            return BaseType::operator*();
        }

        inline const ValueType* operator -> () const
        {
            return BaseType::operator->();
        }
    };


    typedef Iterator iterator;
    typedef ConstIterator const_iterator;

    using HashTableType::empty;
    using HashTableType::size;
    using HashTableType::max_size;
    using HashTableType::clear;
    using HashTableType::count;
    using HashTableType::rehash;
    using HashTableType::reserve;

    inline void set_special_values_(const std::true_type& is_pointer_type)
    {
        set_deleted_value(std::make_pair((MappedKeyType)0x1, MappedValueType()));
        set_nonused_value(std::make_pair((MappedKeyType)0x0, MappedValueType()));
    }

    inline void set_special_values_(const std::false_type& is_pointer_type)
    {
        return;
    }

    inline void set_special_values_()
    {
        typename std::is_pointer<MappedKeyType>::type is_pointer_type;
        set_special_values_(is_pointer_type);
    }

    inline UnorderedMapBase()
        : HashTableType()
    {
        set_special_values_();
    }

    inline UnorderedMapBase(const MappedKeyType& deleted_value,
                            const MappedKeyType& nonused_value)
        : HashTableType(std::pair<const MappedKeyType,
                        MappedValueType>(deleted_value, MappedValueType()),
                        std::pair<const MappedValueType,
                        MappedValueType>(nonused_value, MappedValueType()))
    {
        // Nothing here
    }

    inline UnorderedMapBase(const UnorderedMapBase& other)
        : HashTableType(other)
    {
        // Nothing here
    }

    inline UnorderedMapBase(UnorderedMapBase&& other)
        : HashTableType(std::move(other))
    {
        // Nothing here
    }

    inline UnorderedMapBase(u64 initial_capacity)
        : HashTableType()
    {
        set_special_values_();
        HashTableType::expand_table(initial_capacity);
    }

    inline UnorderedMapBase(u64 initial_capacity,
                            const MappedKeyType& deleted_value,
                            const MappedKeyType& nonused_value)
        : HashTableType(initial_capacity,
                        std::make_pair(deleted_value, MappedValueType()),
                        std::make_pair(nonused_value, MappedValueType()))
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline UnorderedMapBase(const InputIterator& first, const InputIterator& last)
        : HashTableType()
    {
        set_special_values_();
        HashTableType::insert_range(first, last);
    }

    template <typename InputIterator>
    inline UnorderedMapBase(const InputIterator& first,
                            const InputIterator& last,
                            const MappedKeyType& deleted_value,
                            const MappedKeyType& nonused_value)
        : HashTableType(first, last,
                        std::make_pair(deleted_value, MappedValueType()),
                        std::make_pair(nonused_value, MappedValueType()))
    {
        // Nothing here
    }

    inline UnorderedMapBase(std::initializer_list<ValueType> init_list)
        : HashTableType()
    {
        set_special_values_();
        HashTableType::assign(init_list);
    }

    inline UnorderedMapBase(std::initializer_list<ValueType> init_list,
                            const MappedKeyType& deleted_value,
                            const MappedKeyType& nonused_value)
        : HashTableType(init_list,
                        std::make_pair(deleted_value, MappedValueType()),
                        std::make_pair(nonused_value, MappedValueType()))
    {
        // Nothing here
    }

    inline ~UnorderedMapBase()
    {
        // Nothing here
    }

    inline UnorderedMapBase& operator = (const UnorderedMapBase& other)
    {
        if (&other == this) {
            return *this;
        }

        HashTableType::assign(other);
        return *this;
    }

    inline UnorderedMapBase& operator = (UnorderedMapBase&& other)
    {
        if (&other == this) {
            return *this;
        }

        HashTableType::assign(std::move(other));
        return *this;
    }

    inline UnorderedMapBase& operator = (std::initializer_list<ValueType> init_list)
    {
        HashTableType::assign(init_list);
        return *this;
    }

    inline void set_deleted_value(const MappedKeyType& deleted_value)
    {
        HashTableType::set_deleted_value(std::make_pair(deleted_value, MappedValueType()));
    }

    inline void set_nonused_value(const MappedKeyType& nonused_value)
    {
        HashTableType::set_nonused_value(std::make_pair(nonused_value, MappedValueType()));
    }

    inline const MappedKeyType& get_deleted_value() const
    {
        return HashTableType::get_deleted_value().first;
    }

    inline const MappedKeyType& get_nonused_value() const
    {
        return HashTableType::get_nonused_value().first;
    }

    inline Iterator begin()
    {
        return Iterator(HashTableType::begin());
    }

    inline Iterator end()
    {
        return Iterator(HashTableType::end());
    }

    inline ConstIterator begin() const
    {
        return ConstIterator(HashTableType::begin());
    }

    inline ConstIterator end() const
    {
        return ConstIterator(HashTableType::end());
    }

    inline ConstIterator cbegin() const
    {
        return begin();
    }

    inline ConstIterator cend() const
    {
        return end();
    }

    inline MappedValueType& operator [] (const MappedKeyType& key)
    {
        auto it = find(key);
        bool already_present;
        if (it == end()) {
            it = HashTableType::insert(std::pair<const MappedKeyType,
                                       MappedValueType>(key, MappedValueType()), already_present);
            return it->second;
        }
        return it->second;
    }

    inline MappedValueType& operator [] (MappedKeyType&& key)
    {
        auto it = find(key);
        bool already_present;
        if (it == end()) {
            it = HashTableType::insert(std::pair<const MappedKeyType,
                                       MappedValueType>(std::move(key), MappedValueType()),
                                       already_present);
            return it->second;
        }
        return it->second;
    }

    inline MappedValueType& at (const MappedKeyType& key)
    {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("Key not found in Kinara::UnorderedMap::at()");
        }
        return it->second;
    }

    inline const MappedValueType& at (const MappedKeyType& key) const
    {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("Key not found in Kinara::UnorderedMap::at()");
        }
        return it->second;
    }

    inline Iterator find(const MappedKeyType& key)
    {
        return Iterator(HashTableType::find(std::make_pair(key, MappedValueType())));
    }

    inline ConstIterator find(const MappedKeyType& key) const
    {
        return ConstIterator(HashTableType::find(std::make_pair(key, MappedValueType())));
    }

    template <typename... ArgTypes>
    inline std::pair<Iterator, bool> emplace(ArgTypes&&... args)
    {
        bool already_present;
        auto it = HashTableType::emplace(std::forward<ArgTypes>(args)...);
        return std::make_pair(Iterator(it), already_present);
    }

    inline std::pair<Iterator, bool> insert(const ValueType& value)
    {
        bool already_present;
        auto it = HashTableType::insert(value, already_present);
        return std::make_pair(Iterator(it), already_present);
    }

    template <typename P,
              typename Unused =
              typename std::enable_if<std::is_constructible<ValueType, P&&>::value>::type>
    inline std::pair<Iterator, bool> insert(P&& value)
    {
        bool already_present;
        auto it = HashTableType::insert(std::forward<P>(value));
        return std::make_pair(Iterator(it), already_present);
    }

    template <typename InputIterator>
    inline void insert(const InputIterator& first, const InputIterator& last)
    {
        HashTableType::insert(first, last);
    }

    inline void insert(std::initializer_list<ValueType> init_list)
    {
        HashTableType::insert(init_list);
    }

    inline void erase(const ConstIterator& position)
    {
        HashTableType::erase(position);
    }

    inline void erase(const MappedKeyType& value)
    {
        HashTableType::erase(std::make_pair(value, MappedValueType()));
    }

    inline void erase(const ConstIterator& first, const ConstIterator& last)
    {
        HashTableType::erase(first, last);
    }

    inline bool operator == (const UnorderedMapBase& other) const
    {
        auto sz = size();

        if (sz != other.size()) {
            return false;
        }

        auto it1 = begin();
        auto it2 = other.begin();

        auto end1 = end();
        auto end2 = other.end();

        for (u64 i = 0; i < sz; ++i) {
            auto find_other_in_mine = find(it2->first);
            auto find_mine_in_other = other.find(it1->first);

            if (find_other_in_mine == end1 || find_mine_in_other == end2) {
                return false;
            }
            if ((find_mine_in_other->value != it1->value) ||
                (find_other_in_mine->value != it2->value)) {
                return false;
            }
        }

        return true;
    }

    inline bool operator != (const UnorderedMapBase& other) const
    {
        return (!((*this) == other));
    }
};

} /* end namespace unordered_map_detail_ */

// Some useful typedefs

template <typename MappedKeyType, typename MappedValueType,
          typename HashFunction = ku::Hasher<MappedKeyType>,
          typename EqualsFunction = ku::Equal<MappedKeyType> >
using UnifiedUnorderedMap =
    unordered_map_detail_::UnorderedMapBase<MappedKeyType, MappedValueType,
                                            HashFunction, EqualsFunction,
                                            hash_table_detail_::UnifiedHashTable>;

template <typename MappedKeyType, typename MappedValueType,
          typename HashFunction = ku::Hasher<MappedKeyType>,
          typename EqualsFunction = ku::Equal<MappedKeyType> >
using SegregatedUnorderedMap =
    unordered_map_detail_::UnorderedMapBase<MappedKeyType, MappedValueType,
                                            HashFunction, EqualsFunction,
                                            hash_table_detail_::SegregatedHashTable>;

template <typename MappedKeyType, typename MappedValueType,
          typename HashFunction = ku::Hasher<MappedKeyType>,
          typename EqualsFunction = ku::Equal<MappedKeyType> >
using RestrictedUnorderedMap =
    unordered_map_detail_::UnorderedMapBase<MappedKeyType, MappedValueType,
                                            HashFunction, EqualsFunction,
                                            hash_table_detail_::RestrictedHashTable>;

template <typename MappedKeyType, typename MappedValueType>
using PtrUnifiedUnorderedMap = UnifiedUnorderedMap<MappedKeyType*, MappedValueType>;

template <typename MappedKeyType, typename MappedValueType>
using CPtrUnifiedUnorderedMap = UnifiedUnorderedMap<const MappedKeyType*, MappedValueType>;

template <typename MappedKeyType, typename MappedValueType>
using PtrSegregatedUnorderedMap = SegregatedUnorderedMap<MappedKeyType*, MappedValueType>;

template <typename MappedKeyType, typename MappedValueType>
using CPtrSegregatedUnorderedMap = SegregatedUnorderedMap<const MappedKeyType*, MappedValueType>;

template <typename MappedKeyType, typename MappedValueType>
using PtrRestrictedUnorderedMap = RestrictedUnorderedMap<MappedKeyType*, MappedValueType>;

template <typename MappedKeyType, typename MappedValueType>
using CPtrRestrictedUnorderedMap = RestrictedUnorderedMap<const MappedKeyType*, MappedValueType>;

template <typename T, typename MappedValueType>
using MPtrUnifiedUnorderedMap =
    UnifiedUnorderedMap<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                  memory::ManagedPointer<T>, T*>::type,
                        MappedValueType>;

template <typename T, typename MappedValueType>
using CMPtrUnifiedUnorderedMap =
    UnifiedUnorderedMap<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                  memory::ManagedConstPointer<T>, const T*>::type,
                        MappedValueType>;

template <typename T, typename MappedValueType>
using MPtrSegregatedUnorderedMap =
    SegregatedUnorderedMap<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                     memory::ManagedPointer<T>, T*>::type,
                           MappedValueType>;

template <typename T, typename MappedValueType>
using CMPtrSegregatedUnorderedMap =
    SegregatedUnorderedMap<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                     memory::ManagedConstPointer<T>, const T*>::type,
                           MappedValueType>;

template <typename T, typename MappedValueType>
using MPtrRestrictedUnorderedMap =
    RestrictedUnorderedMap<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                     memory::ManagedPointer<T>, T*>::type,
                           MappedValueType>;

template <typename T, typename MappedValueType>
using CMPtrRestrictedUnorderedMap =
    RestrictedUnorderedMap<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                     memory::ManagedConstPointer<T>, const T*>::type,
                           MappedValueType>;

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_UNORDERED_MAP_HPP_ */

//
// UnorderedMap.hpp ends here
