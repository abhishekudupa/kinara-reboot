//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// audupa: adapted from public domain source for use in kinara

#if !defined KINARA_KINARA_COMMON_HASHFUNCS_MURMUR_HASH_HPP_
#define KINARA_KINARA_COMMON_HASHFUNCS_MURMUR_HASH_HPP_

#include <cstdint>

#include "../basetypes/KinaraBase.hpp"

namespace kinara {
namespace utils {
namespace murmur_hash_detail_ {

extern void MurmurHash3_x86_32  (const void * key, u64 len, u32 seed, void* out);
extern void MurmurHash3_x86_128 (const void * key, u64 len, u32 seed, void* out);
extern void MurmurHash3_x64_128 (const void * key, u64 len, u32 seed, void* out);

} /* end namespace murmur_hash_detail_ */

static inline u32 murmur_hash_32(const void* key, u64 length, u32 seed)
{
    u32 retval = 0;
    murmur_hash_detail_::MurmurHash3_x86_32(key, length, seed, &retval);
    return retval;
}

static inline u32 murmur_hash_32(const void* key, u64 length)
{
    // a large prime
    constexpr u32 seed = 2147480009;
    return murmur_hash_32(key, length, seed);
}

static inline u128 murmur_hash_128(const void* key, u64 length, u32 seed)
{
    u64 retval[2];
    murmur_hash_detail_::MurmurHash3_x64_128(key, length, seed, &retval);
    return std::make_pair(retval[0], retval[1]);
}

static inline u64 murmur_hash_64(const void* key, u64 length, u32 seed)
{
    auto&& retval = murmur_hash_128(key, length, seed);
    return (retval.first ^ retval.second);
}

static inline u64 murmur_hash_64(const void* key, u64 length)
{
    constexpr u32 seed = 2147480837;
    return murmur_hash_64(key, length, seed);
}

} /* end namespace utils */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_HASHFUNCS_MURMUR_HASH_HPP_ */
