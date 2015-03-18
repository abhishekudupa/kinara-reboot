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
        using BaseType::operator=;
        using BaseType::operator==;
        using BaseType::operator!=;
        using BaseType::operator++;
        using BaseType::operator--;

        inline const T& operator * () const
        {
            return BaseType::operator*();
        }

        inline const T* operator -> () const
        {
            return BaseType::operator->();
        }
    };

    using HashTableType::HashTableType;
    using HashTableType::insert;
    using HashTableType::find;
    using HashTableType::erase;
    using HashTableType::size;
    using HashTableType::begin;
    using HashTableType::end;
    using HashTableType::clear;
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

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_UNORDERED_SET_HPP_ */

//
// UnorderedSet.hpp ends here
