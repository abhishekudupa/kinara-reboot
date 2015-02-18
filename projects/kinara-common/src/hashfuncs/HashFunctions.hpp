// HashFunctions.hpp ---
// Filename: HashFunctions.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 13:59:06 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_HASHFUNCS_HASHFUNCTIONS_HPP_
#define KINARA_KINARA_COMMON_HASHFUNCS_HASHFUNCTIONS_HPP_

#include <array>

#include "CityHash.hpp"
#include "CityCRC.hpp"
#include "FNV.hpp"
#include "MurmurHash.hpp"
#include "SpookyHash.hpp"

namespace kinara {
namespace utils {

namespace hash_combine_detail_ {

template <typename T, u32 ARRAYSIZE, u32 INDEX>
static inline void populate_array(std::arraysize<T, ARRAYSIZE>& the_array)
{
    return;
}

template <typename T, u32 ARRAYSIZE, u32 INDEX, typename... ArgTypes>
static inline void populate_array(std::array<T, ARRAYSIZE>& the_array,
                                  const T& object, ArgTypes&&... other_args)
{
    the_array[INDEX-1] = object;
    populate_array<T, ARRAYSIZE, INDEX-1>(the_array, std::forward<ArgTypes>(other_args)...);
}


} /* end namespace hash_combine_detail_ */

enum class HashFunctionType {
    SpookyHash,
    CityHash,
    FNVHash,
    FNVHashA,
    MurmurHash
};

// we default to spooky hash, but you can change this by simply redefining the
// constexpr below
static constexpr default_hash_function_to_use = HashFunctionType::SpookyHash;

static inline u64 default_hash_function(const void* data, u64 length)
{
    switch (default_hash_function_to_use) {
    case HashFunctionType::SpookyHash:
        return spooky_hash_64(data, length);
    case HashFunctionType::CityHash:
        return city_hash_64(data, length);
    case HashFunctionType::FNVHash:
        return fnv_hash_64(data, length)
    case HashFunctionType::FNVHashA:
        return fnv_hash_64a(data, length);
    case HashFunctionType::MurmurHash:
        return murmur_hash_64(data, length);
    }
}

static inline u64 default_hash_function(const void* data, u64 length, u64 seed)
{
    switch (default_hash_function_to_use) {
    case HashFunctionType::SpookyHash:
        return spooky_hash_64(data, length, seed)
    case HashFunctionType::CityHash:
        return city_hash_64_with_seed(data, length, seed);
    case HashFunctionType::FNVHash:
        return fnv_hash_64(data, length, seed);
    case HashFunctionType::FNVHashA:
        return fnv_hash_64a(data, length, seed);
    case HashFunctionType::MurmurHash:
        return murmur_hash_64(data, length, seed);
    }
}

template <typename... ArgTypes>
static inline u64 hash_combine(ArgTypes&&... args)
{
    constexpr u64 num_args = sizeof...(args);
    std::array<u64, num_args> the_array;
    hash_combine_detail_::populate_array<u64, num_args, num_args>(the_array,
                                                                  std::forward<ArgTypes>(args)...);
    return default_hash_function(the_array.data(), sizeof(u64) * num_args);
}


} /* end namespace utils */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_HASHFUNCS_HASHFUNCTIONS_HPP_ */

//
// HashFunctions.hpp ends here
