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

#include "HashFunctions.hpp"

namespace kinara {
namespace utils {

template <typename T>
class Hasher
{
private:
    inline u64 get_hash(const Hashable& hashable_ref) const
    {
        return hashable_ref.hash();
    }

    inline u64 get_hash(const Hashable* hashable_ptr) const
    {
        return hashable_ptr->hash();
    }

    inline u64 get_hash(const Stringifiable& stringifiable_ref) const
    {
        return
    }

public:
    inline u64 operator () (const T& object) const
    {
    }
};

// Specializations for basic types

} /* end namespace utils */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_HASHFUNCS_HASH_HPP_ */

//
// Hash.hpp ends here