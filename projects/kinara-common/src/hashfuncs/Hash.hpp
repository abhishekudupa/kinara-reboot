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

#include "HashFunctions.hpp"

namespace kinara {
namespace utils {

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
        typename std::is_convertible<const T*, const Hashable*>::type is_hashable_object;
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
        typename std::is_convertible<const T*, const Hashable*>::type is_hashable_object;
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
        typename std::is_convertible<const T*, const Hashable*>::type is_hashable_object;
        return compute_hash(ptr, is_hashable_object);
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
        return (t1_hasher(the_pair.first) ^ t2_hasher(the_pair.second));
    }
};

// Specialization for tuples
template <typename... ArgTypes>
class Hasher<std::tuple<ArgTypes...>>
{
private:
    template <u64 INDEX, typename... TupleTypes>
    inline typename std::enable_if<INDEX == sizeof...(TupleTypes), void>::type
    compute_hash(const std::tuple<TupleTypes...>& the_tuple,
                 std::array<u64, sizeof...(TupleTypes)>& the_array) const
    {
     	// break recursion
    }

    template <u64 INDEX, typename... TupleTypes>
    inline typename std::enable_if<INDEX < sizeof...(TupleTypes), void>::type
    compute_hash(const std::tuple<TupleTypes...>& the_tuple,
                 std::array<u64, sizeof...(TupleTypes)>& the_array) const
    {
        Hasher<typename std::tuple_element<INDEX, typename std::tuple<TupleTypes...> >::type> hasher;
        the_array[INDEX] = hasher(std::get<INDEX>(the_tuple));
        compute_hash<INDEX+1>(the_tuple, the_array);
    }

public:
    inline u64 operator () (const std::tuple<ArgTypes...>& the_tuple) const
    {
        std::array<u64, sizeof...(ArgTypes)> the_array;
        memset(the_array.data(), 0, sizeof(u64) * sizeof...(ArgTypes));
        compute_hash<0>(the_tuple, the_array);
        return default_hash_function(the_array.data(), sizeof(u64) * sizeof...(ArgTypes));
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
    inline typename std::enable_if<INDEX == sizeof...(TupleTypes), void>::type
    compute_hash(const std::tuple<TupleTypes...>& the_tuple,
                 std::array<u64, sizeof...(TupleTypes)>& the_array) const
    {
     	// break recursion
    }

    template <u64 INDEX, typename... TupleTypes>
    inline typename std::enable_if<INDEX < sizeof...(TupleTypes), void>::type
    compute_hash(const std::tuple<TupleTypes...>& the_tuple,
                 std::array<u64, sizeof...(TupleTypes)>& the_array) const
    {
        RawHasher<typename std::tuple_element<INDEX,
                                              typename std::tuple<TupleTypes...> >::type> hasher;
        the_array[INDEX] = hasher(std::get<INDEX>(the_tuple));
        compute_hash<INDEX+1>(the_tuple, the_array);
    }

public:
    inline u64 operator () (const std::tuple<ArgTypes...>& the_tuple) const
    {
        std::array<u64, sizeof...(ArgTypes)> the_array;
        memset(the_array.data(), 0, sizeof(u64) * sizeof...(ArgTypes));
        compute_hash<0>(the_tuple, the_array);
        return default_hash_function(the_array.data(), sizeof(u64) * sizeof...(ArgTypes));
    }
};

// hashes for iterables
template <typename T, typename ElemHasher = Hasher<T::value_type>>
class IterableHasher
{
public:
    inline u64 operator () (const T& iterable) const
    {
        ElemHasher elem_hasher;
        u64 retval = 0;
        for (auto it = iterable.begin(), last = iterable.end(); it != last; ++it) {
            auto cur_hash = elem_hasher(*it);
            retval = retval ^ ((cur_hash << 23) ^ (cur_hash >> 37));
        }
        return retval;
    }
};


} /* end namespace utils */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_HASHFUNCS_HASH_HPP_ */

//
// Hash.hpp ends here
