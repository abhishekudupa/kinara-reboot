// Hash.hpp ---
// Filename: Hash.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 18:49:12 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_HASHFUNCS_HASH_HPP_
#define KINARA_KINARA_COMMON_HASHFUNCS_HASH_HPP_

// this file contains the definition of builtin hash
// functions for common types, as well as facilities
// to combine/mix hash values in a nice fashion

#include <functional>
#include <type_traits>

#include "../basetypes/KinaraBase.hpp"
#include "../basetypes/KinaraTypes.hpp"

#include "../memory/ManagedPointer.hpp"

#include "HashFunctions.hpp"

namespace kinara {
namespace utils {

namespace km = kinara::memory;

template <typename T>
class Hasher
{
private:
    inline u64 compute_hash(const T& object,
                            const std::true_type& is_hashable_object) const
    {
        return object.hash();
    }

    inline u64 compute_hash(const T& object,
                            const std::false_type& is_hashable_object) const
    {
        std::hash<T> hasher;
        return hasher(object);
    }

public:
    inline u64 operator () (const T& object) const
    {
        typename std::is_base_of<HashableEBC, T>::type is_hashable_object;
        return compute_hash(object, is_hashable_object);
    }
};

// specialization for pointers
template <typename T>
class Hasher<T*>
{
private:
    inline u64 compute_hash(const T* ptr,
                            const std::true_type& is_hashable_object) const
    {
        return ptr->hash();
    }

    inline u64 compute_hash(const T* ptr,
                            const std::false_type& is_hashable_object) const
    {
        std::hash<T*> hasher;
        return hasher(ptr);
    }

public:
    inline u64 operator () (const T* ptr) const
    {
        typename std::is_base_of<HashableEBC, T>::type is_hashable_object;
        return compute_hash(ptr, is_hashable_object);
    }
};

template <typename T>
class Hasher<const T*>
{
private:
    inline u64 compute_hash(const T* ptr,
                            const std::true_type& is_hashable_object) const
    {
        return ptr->hash();
    }

    inline u64 compute_hash(const T* ptr,
                            const std::false_type& is_hashable_object) const
    {
        std::hash<T*> hasher;
        return hasher(ptr);
    }

public:
    inline u64 operator () (const T* ptr) const
    {
        typename std::is_base_of<HashableEBC, T>::type is_hashable_object;
        return compute_hash(ptr, is_hashable_object);
    }
};

template <typename T>
class Hasher<km::ManagedPointer<T> >
{
private:
    inline u64 compute_hash(const T* ptr,
                            const std::true_type& is_hashable_object) const
    {
        return ptr->hash();
    }

    inline u64 compute_hash(const T* ptr,
                            const std::false_type& is_hashable_object) const
    {
        std::hash<T*> hasher;
        return hasher(ptr);
    }

public:
    inline u64 operator () (const km::ManagedPointer<T>& managed_ptr) const
    {
        typename std::is_base_of<HashableEBC, T>::type is_hashable_object;
        return compute_hash((const T*)managed_ptr, is_hashable_object);
    }
};

template <typename T>
class Hasher<km::ManagedConstPointer<T> >
{
private:
    inline u64 compute_hash(const T* ptr,
                            const std::true_type& is_hashable_object) const
    {
        return ptr->hash();
    }

    inline u64 compute_hash(const T* ptr,
                            const std::false_type& is_hashable_object) const
    {
        std::hash<T*> hasher;
        return hasher(ptr);
    }

public:
    inline u64 operator () (const km::ManagedConstPointer<T>& managed_ptr) const
    {
        typename std::is_base_of<HashableEBC, T>::type is_hashable_object;
        return compute_hash((const T*)managed_ptr, is_hashable_object);
    }
};

// Specialization for pairs
template <typename T1, typename T2>
class Hasher<std::pair<T1, T2>>
{
public:
    inline u64 operator () (const std::pair<T1, T2>& the_pair) const
    {
        Hasher<T1> t1_hasher;
        Hasher<T2> t2_hasher;
        auto h1 = t1_hasher(the_pair.first);
        auto h2 = t2_hasher(the_pair.second);

        return ((h1 * 0x100000001b3UL) ^ h2);
    }
};

// Specialization for tuples
template <typename... ArgTypes>
class Hasher<std::tuple<ArgTypes...>>
{
private:
    template <u64 INDEX, typename... TupleTypes>
    inline typename std::enable_if<INDEX == sizeof...(TupleTypes), u64>::type
    compute_hash(const std::tuple<TupleTypes...>& the_tuple,
                 u64 accumulated_hash) const
    {
        return accumulated_hash;
    }

    template <u64 INDEX, typename... TupleTypes>
    inline typename std::enable_if<INDEX < sizeof...(TupleTypes), u64>::type
    compute_hash(const std::tuple<TupleTypes...>& the_tuple,
                 u64 accumulated_hash) const
    {
        Hasher<typename std::tuple_element<INDEX, typename std::tuple<TupleTypes...> >::type> hasher;
        auto h1 = hasher(std::get<INDEX>(the_tuple));
        auto new_accumulated_hash = (accumulated_hash * 0x100000001b3UL) ^ h1;
        return compute_hash<INDEX+1>(the_tuple, new_accumulated_hash);
    }

public:
    inline u64 operator () (const std::tuple<ArgTypes...>& the_tuple) const
    {
        return compute_hash<0>(the_tuple, 0xcbf29ce484222325UL);
    }
};

// or perhaps you want the raw hash?
template <typename T>
class RawHasher
{
public:
    inline u64 operator () (const T& object) const
    {
        std::hash<T> hasher;
        return hasher(object);
    }
};

// Specialization of raw hash for pairs
template <typename T1, typename T2>
class RawHasher<std::pair<T1, T2>>
{
public:
    inline u64 operator () (const std::pair<T1, T2>& the_pair) const
    {
        RawHasher<T1> t1_hasher;
        RawHasher<T2> t2_hasher;
        return (t1_hasher(the_pair.first) ^ t2_hasher(the_pair.second));
    }
};

// specialization of raw hash for tuples
template <typename... ArgTypes>
class RawHasher<std::tuple<ArgTypes...>>
{
private:
    template <u64 INDEX, typename... TupleTypes>
    inline typename std::enable_if<INDEX == sizeof...(TupleTypes), u64>::type
    compute_hash(const std::tuple<TupleTypes...>& the_tuple,
                 u64 accumulated_hash) const
    {
        return accumulated_hash;
    }

    template <u64 INDEX, typename... TupleTypes>
    inline typename std::enable_if<INDEX < sizeof...(TupleTypes), u64>::type
    compute_hash(const std::tuple<TupleTypes...>& the_tuple,
                 u64 accumulated_hash) const
    {
        RawHasher<typename std::tuple_element<INDEX,
                                              typename std::tuple<TupleTypes...> >::type> hasher;
        auto h1 = hasher(std::get<INDEX>(the_tuple));
        auto new_accumulated_hash = (accumulated_hash * 0x100000001b3UL) ^ h1;
        compute_hash<INDEX+1>(the_tuple, new_accumulated_hash);
    }

public:
    inline u64 operator () (const std::tuple<ArgTypes...>& the_tuple) const
    {
        return compute_hash<0>(the_tuple, 0xcbf29ce484222325UL);
    }
};

// hashes for iterables
template <typename T, typename ElemHasher = Hasher<typename T::value_type>>
class IterableHasher
{
public:
    inline u64 operator () (const T& iterable) const
    {
        ElemHasher elem_hasher;
        u64 retval = 0xcbf29ce484222325UL;
        for (auto it = iterable.begin(), last = iterable.end(); it != last; ++it) {
            auto cur_hash = elem_hasher(*it);
            retval = (retval * 0x100000001b3UL) ^ cur_hash;
        }
        return retval;
    }
};


} /* end namespace utils */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_HASHFUNCS_HASH_HPP_ */

//
// Hash.hpp ends here
