// FNV.cpp ---
// Filename: FNV.cpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 16:40:25 2015 (-0500)
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

#include "FNV.hpp"

namespace kinara {
namespace utils {
namespace fnv_hash_detail_ {

Fnv32_t
fnv_32_buf(const void *buf, size_t len, Fnv32_t hval)
{
    auto bp = static_cast<const u08*>(buf);
    auto be = bp + len;

    while (bp < be) {
        hval *= FNV1_32_PRIME;
        hval ^= (Fnv32_t)*bp++;
    }

    return hval;
}

Fnv32_t
fnv_32_str(const char *str, Fnv32_t hval)
{
    auto s = static_cast<const u08*>(static_cast<const void*>(str));

    while (*s) {
        hval *= FNV1_32_PRIME;
        hval ^= (Fnv32_t)*s++;
    }

    return hval;
}

Fnv32_t
fnv_32a_buf(const void *buf, size_t len, Fnv32_t hval)
{
    auto bp = static_cast<const u08*>(buf);
    auto be = bp + len;

    while (bp < be) {
        hval ^= (Fnv32_t)*bp++;
        hval *= FNV1_32_PRIME;
    }

    return hval;
}

Fnv32_t
fnv_32a_str(const char *str, Fnv32_t hval)
{
    auto s = static_cast<const u08*>(static_cast<const void*>(str));

    while (*s) {
        hval ^= (Fnv32_t)*s++;
        hval *= FNV1_32_PRIME;
    }

    /* return our new hash value */
    return hval;
}

Fnv64_t
fnv_64_buf(const void* buf, size_t len, Fnv64_t hval)
{
    auto bp = static_cast<const u08*>(buf);
    auto be = bp + len;

    while (bp < be) {
        hval *= FNV1_64_PRIME;
        hval ^= (Fnv64_t)*bp++;
    }
    return hval;
}


Fnv64_t
fnv_64_str(const char* str, Fnv64_t hval)
{
    auto s = static_cast<const u08*>(static_cast<const void*>(str));
    while (*s) {
        hval *= FNV1_64_PRIME;
        hval ^= (Fnv64_t)*s++;
    }
    return hval;
}

Fnv64_t
fnv_64a_buf(const void* buf, size_t len, Fnv64_t hval)
{
    auto bp = static_cast<const u08*>(buf);
    auto be = bp + len;

    while (bp < be) {
        hval ^= (Fnv64_t)*bp++;
        hval *= FNV1_64_PRIME;
    }
    return hval;
}


Fnv64_t
fnv_64a_str(const char *str, Fnv64_t hval)
{
    auto s = static_cast<const u08*>(static_cast<const void*>(str));
    while (*s) {
        hval ^= (Fnv64_t)*s++;
        hval *= FNV1_64_PRIME;
    }
    return hval;
}

} /* end namespace fnv_hash_detail_ */
} /* end namespace utils */
} /* end namespace kinara */

//
// FNV.cpp ends here
