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

#include "../basetypes/KinaraBase.hpp"
#include "../basetypes/KinaraTypes.hpp"

namespace kinara {
namespace utils {
namespace hash_detail_ {

template <typename T>
static inline u64 get_hash_value_ref(const T& object_ref,
                                     const std::true_type& is_convertible_to_ref)
{
    const Hashable& hashable_ref = object_ref;
    return hashable_ref.hash();
}

template <typename T>
static inline u64 get_hash_value_ref(const T& object_ref,
                                     const std::false_type& is_convertible_to_ref)
{
    // Not convertible to a hashable ref, just try to return std::hash
    std::hash<T> the_hasher;
    return the_hasher(object_ref);
}

// can convert to a hashable pointer
template <typename T>
static inline u64 get_hash_value(const T& object_ref,
                                 const std::true_type& is_convertible_to_ptr)
{
    const Hashable* hashable_ptr = object_ref;
    return hashable_ptr->hash();
}

// cannot convert to a hashable pointer
// but can we convert to a hashable ref?
template <typename T>
static inline u64 get_hash_value(const T& object_ref,
                                 const std::false_type& is_convertible_to_ptr)
{
    typename std::is_convertible<T, const Hashable&>::type is_convertible_to_ref;
    get_hash_value_ref(object_ref, is_convertible_to_ref);
}

template <typename T>
static inline u64 get_hash_value(const T& object)
{
    typename std::is_convertible<T, const Hashable*>::type is_pointer_value;
    hash_detail_::get_hash_value(object, is_pointer_value);
}

} /* end namespace hash_detail_ */

template <typename T, typename... RestOfTypes>
class Hasher
{

};

template<typename T>
class Hasher<T>
{

};

template <typename T1, typename T2>
class Hasher<std::pair<T1, T2>>
{

};

template <typename... ArgTypes>
class Hasher<std::tuple<ArgTypes>>
{

};


} /* end namespace utils */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_HASHFUNCS_HASH_HPP_ */

//
// Hash.hpp ends here
