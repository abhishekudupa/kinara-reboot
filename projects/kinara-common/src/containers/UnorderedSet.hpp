// UnorderedSet.hpp ---
// Filename: UnorderedSet.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 02:09:13 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_UNORDERED_SET_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_UNORDERED_SET_HPP_

#include "../hashfuncs/Hash.hpp"
#include "../basetypes/Comparators.hpp"

#include "HashTable.hpp"

namespace kinara {
namespace containers {

namespace kc = kinara::containers;
namespace ku = kinara::utils;

namespace unordered_set_detail_ {

template <typename T, typename HashFunction, typename EqualsFunction,
          template <typename, typename, typename> class HashTableTemplateType>
class UnorderedSetBase : private HashTableTemplateType<T, HashFunction, EqualsFunction>
{
private:
    typedef HashTableTemplateType<T, HashFunction, EqualsFunction> HashTableType;

public:
    typedef T ValueType;
    typedef ValueType value_type;
    typedef T* PtrType;
    typedef T* ptr_type;
    typedef T& RefType;
    typedef T& ref_type;
    typedef const T* ConstPtrType;
    typedef const T* const_ptr_type;
    typedef const T& ConstRefType;
    typedef const T& const_ref_type;

    class Iterator : private HashTableType::Iterator,
                     public std::iterator<std::bidirectional_iterator_tag,
                                          T, i64, const T*, const T&>
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

        inline const T& operator * () const
        {
            return BaseType::operator*();
        }

        inline const T* operator -> () const
        {
            return BaseType::operator->();
        }
    };

    typedef Iterator iterator;
    typedef Iterator ConstIterator;
    typedef ConstIterator const_iterator;

    using HashTableType::empty;
    using HashTableType::size;
    using HashTableType::max_size;
    using HashTableType::clear;
    using HashTableType::count;
    using HashTableType::rehash;
    using HashTableType::reserve;
    using HashTableType::get_nonused_value;
    using HashTableType::set_nonused_value;
    using HashTableType::get_deleted_value;
    using HashTableType::set_deleted_value;

    inline UnorderedSetBase()
        : HashTableType()
    {
        // Nothing here
    }

    inline UnorderedSetBase(const T& deleted_value, const T& nonused_value)
        : HashTableType(deleted_value, nonused_value)
    {
        // Nothing here
    }

    inline UnorderedSetBase(const UnorderedSetBase& other)
        : HashTableType(other)
    {
        // Nothing here
    }

    inline UnorderedSetBase(UnorderedSetBase&& other)
        : HashTableType(std::move(other))
    {
        // Nothing here
    }

    inline UnorderedSetBase(u64 initial_capacity)
        : HashTableType(initial_capacity)
    {
        // Nothing here
    }

    inline UnorderedSetBase(u64 initial_capacity, const T& deleted_value, const T& nonused_value)
        : HashTableType(initial_capacity, deleted_value, nonused_value)
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline UnorderedSetBase(const InputIterator& first, const InputIterator& last)
        : HashTableType(first, last)
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline UnorderedSetBase(const InputIterator& first, const InputIterator& last,
                            const T& deleted_value, const T& nonused_value)
        : HashTableType(first, last, deleted_value, nonused_value)
    {
        // Nothing here
    }

    inline UnorderedSetBase(std::initializer_list<T> init_list)
        : HashTableType(init_list)
    {
        // Nothing here
    }

    inline UnorderedSetBase(std::initializer_list<T> init_list,
                            const T& deleted_value, const T& nonused_value)
        : HashTableType(init_list, deleted_value, nonused_value)
    {
        // Nothing here
    }

    inline ~UnorderedSetBase()
    {
        // Nothing here
    }

    inline UnorderedSetBase& operator = (const UnorderedSetBase& other)
    {
        if (&other == this) {
            return *this;
        }
        HashTableType::assign(other);
        return *this;
    }

    inline UnorderedSetBase& operator = (UnorderedSetBase&& other)
    {
        if (&other == this) {
            return *this;
        }
        HashTableType::assign(std::move(other));
        return *this;
    }

    inline UnorderedSetBase& operator = (std::initializer_list<T> init_list)
    {
        HashTableType::assign(init_list);
        return *this;
    }

    inline Iterator begin() const
    {
        return Iterator(HashTableType::begin());
    }

    inline Iterator end() const
    {
        return Iterator(HashTableType::end());
    }

    inline Iterator cbegin() const
    {
        return begin();
    }

    inline Iterator cend() const
    {
        return end();
    }

    template <typename U>
    inline Iterator find(const U& value) const
    {
        return Iterator(HashTableType::find(value));
    }

    template <typename... ArgTypes>
    inline std::pair<Iterator, bool> emplace(ArgTypes&&... args)
    {
        bool already_present;
        auto it = HashTableType::emplace(std::forward<ArgTypes>(args)...);
        return std::make_pair(Iterator(it), already_present);
    }

    inline std::pair<Iterator, bool> insert(const T& value)
    {
        bool already_present;
        auto it = HashTableType::insert(value, already_present);
        return std::make_pair(Iterator(it), already_present);
    }

    inline std::pair<Iterator, bool> insert(T&& value)
    {
        bool already_present;
        auto it = HashTableType::insert(std::move(value));
        return std::make_pair(Iterator(it), already_present);
    }

    template <typename InputIterator>
    inline void insert(const InputIterator& first, const InputIterator& last)
    {
        HashTableType::insert(first, last);
    }

    inline void insert(const std::initializer_list<T>& init_list)
    {
        HashTableType::insert(init_list);
    }

    inline void erase(const Iterator& position)
    {
        HashTableType::erase(position);
    }

    inline void erase(const T& value)
    {
        HashTableType::erase(value);
    }

    inline void erase(const Iterator& first, const Iterator& last)
    {
        HashTableType::erase(first, last);
    }

    inline bool operator == (const UnorderedSetBase& other) const
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
            if (find(*it2) == end1 || other.find(*it1) == end2) {
                return false;
            }
        }
        return true;
    }

    inline bool operator != (const UnorderedSetBase& other) const
    {
        return (!((*this) == other));
    }
};

} /* end namespace unordered_set_detail_ */


// Some useful typedefs
template <typename T, typename HashFunction = ku::Hasher<T>,
          typename EqualsFunction = ku::Equal<T> >
using UnifiedUnorderedSet =
    unordered_set_detail_::UnorderedSetBase<T, HashFunction, EqualsFunction,
                                            hash_table_detail_::UnifiedHashTable>;

template <typename T, typename HashFunction = ku::Hasher<T>,
          typename EqualsFunction = ku::Equal<T> >
using SegregatedUnorderedSet =
    unordered_set_detail_::UnorderedSetBase<T, HashFunction, EqualsFunction,
                                            hash_table_detail_::SegregatedHashTable>;

template <typename T, typename HashFunction = ku::Hasher<T>,
          typename EqualsFunction = ku::Equal<T> >
using RestrictedUnorderedSet =
    unordered_set_detail_::UnorderedSetBase<T, HashFunction, EqualsFunction,
                                            hash_table_detail_::RestrictedHashTable>;

template <typename T, typename HashFunction = ku::Hasher<T*>,
          typename EqualsFunction = ku::Equal<T*>>
    using PtrUnifiedUnorderedSet = UnifiedUnorderedSet<T*, HashFunction, EqualsFunction>;

template <typename T, typename HashFunction = ku::Hasher<const T*>,
          typename EqualsFunction = ku::Equal<const T*> >
using CPtrUnifiedUnorderedSet = UnifiedUnorderedSet<const T*>;

template <typename T, typename HashFunction = ku::Hasher<T*>,
          typename EqualsFunction = ku::Equal<T*>>
using PtrSegregatedUnorderedSet = SegregatedUnorderedSet<T*>;

template <typename T, typename HashFunction = ku::Hasher<const T*>,
          typename EqualsFunction = ku::Equal<const T*>>
using CPtrSegregatedUnorderedSet = SegregatedUnorderedSet<const T*>;

template <typename T, typename HashFunction = ku::Hasher<T*>,
          typename EqualsFunction = ku::Equal<T*>>
using PtrRestrictedUnorderedSet = RestrictedUnorderedSet<T*>;

template <typename T, typename HashFunction = ku::Hasher<const T*>,
          typename EqualsFunction = ku::Equal<const T*>>
using CPtrRestrictedUnorderedSet = RestrictedUnorderedSet<const T*>;

template <typename T,
          typename HashFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Hasher<memory::ManagedPointer<T> >,
                                    ku::Hasher<T*> >::type,
          typename EqualsFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Equal<memory::ManagedPointer<T> >,
                                    ku::Equal<T*> >::type>
using MPtrUnifiedUnorderedSet =
    UnifiedUnorderedSet<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                  memory::ManagedPointer<T>, T*>::type,
                        HashFunction, EqualsFunction>;

template <typename T,
          typename HashFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Hasher<memory::ManagedConstPointer<T> >,
                                    ku::Hasher<const T*> >::type,
          typename EqualsFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Equal<memory::ManagedConstPointer<T> >,
                                    ku::Equal<const T*> >::type>
using CMPtrUnifiedUnorderedSet =
    UnifiedUnorderedSet<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                  memory::ManagedConstPointer<T>,
                                                  const T*>::type,
                        HashFunction, EqualsFunction>;

template <typename T,
          typename HashFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Hasher<memory::ManagedPointer<T> >,
                                    ku::Hasher<T*> >::type,
          typename EqualsFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Equal<memory::ManagedPointer<T> >,
                                    ku::Equal<T*> >::type>
using MPtrSegregatedUnorderedSet =
    SegregatedUnorderedSet<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                     memory::ManagedPointer<T>, T*>::type,
                           HashFunction, EqualsFunction>;

template <typename T,
          typename HashFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Hasher<memory::ManagedConstPointer<T> >,
                                    ku::Hasher<const T*> >::type,
          typename EqualsFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Equal<memory::ManagedConstPointer<T> >,
                                    ku::Equal<const T*> >::type>
using CMPtrSegregatedUnorderedSet =
    SegregatedUnorderedSet<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                     memory::ManagedConstPointer<T>,
                                                     const T*>::type,
                           HashFunction, EqualsFunction>;

template <typename T,
          typename HashFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Hasher<memory::ManagedPointer<T> >,
                                    ku::Hasher<T*> >::type,
          typename EqualsFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Equal<memory::ManagedPointer<T> >,
                                    ku::Equal<T*> >::type>
using MPtrRestrictedUnorderedSet =
    RestrictedUnorderedSet<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                     memory::ManagedPointer<T>, T*>::type,
                           HashFunction, EqualsFunction>;

template <typename T,
          typename HashFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Hasher<memory::ManagedConstPointer<T> >,
                                    ku::Hasher<const T*> >::type,
          typename EqualsFunction =
          typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    ku::Equal<memory::ManagedConstPointer<T> >,
                                    ku::Equal<const T*> >::type>
using CMPtrRestrictedUnorderedSet =
    RestrictedUnorderedSet<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                                     memory::ManagedConstPointer<T>,
                                                     const T*>::type,
                           HashFunction, EqualsFunction>;

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_UNORDERED_SET_HPP_ */

//
// UnorderedSet.hpp ends here
