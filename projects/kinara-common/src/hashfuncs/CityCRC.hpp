// Copyright (c) 2011 Google, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// CityHash, by Geoff Pike and Jyrki Alakuijala
//
// This file declares the subset of the CityHash functions that require
// _mm_crc32_u64().  See the CityHash README for details.
//
// Functions in the CityHash family are not suitable for cryptography.

// audupa: adapted from the sources by google for use in kinara

#if !defined KINARA_KINARA_COMMON_HASHFUNCS_CITY_CRC_HPP_
#define KINARA_KINARA_COMMON_HASHFUNCS_CITY_CRC_HPP_

#include "CityHash.hpp"

// only defined if sse 4.2 available
#ifdef __SSE4_2__

namespace kinara {
namespace utils {
namespace cityhash_detail_ {

// Hash function for a byte array.
uint128 CityHashCrc128(const char *s, size_t len);

// Hash function for a byte array.  For convenience, a 128-bit seed is also
// hashed into the result.
uint128 CityHashCrc128WithSeed(const char *s, size_t len, uint128 seed);

// Hash function for a byte array.  Sets result[0] ... result[3].
void CityHashCrc256(const char *s, size_t len, uint64 *result);

} /* end namespace cityhash_detail_ */

static inline u128 city_hash_crc_128(const char* s, u64 len)
{
    return cityhash_detail_::CityHashCrc128(s, len);
}

static inline u128 city_hash_crc_128_with_seed(const char* data, u64 length,
                                               u64 seed1, u64 seed2)
{
    return cityhash_detail_::CityHashCrc128WithSeed(data, length, std::make_pair(seed1, seed2));
}

static inline void city_hash_crc_256(const char* data, u64 length, u64* result)
{
    return cityhash_detail_::CityHashCrc256(data, length, result);
}

} /* end namespace utils */
} /* end namespace kinara */

#endif /* __SSE4_2__ */
#endif /* KINARA_KINARA_COMMON_HASHFUNCS_CITY_CRC_HPP_ */
