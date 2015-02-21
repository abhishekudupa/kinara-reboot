// KinaraBase.hpp ---
//
// Filename: KinaraBase.hpp
// Author: Abhishek Udupa
// Created: Wed Feb 11 16:48:21 2015 (-0500)
//
//
// Copyright (c) 2015, Abhishek Udupa, University of Pennsylvania
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

#if !defined KINARA_KINARA_COMMON_BASETYPES_KINARA_BASE_HPP_
#define KINARA_KINARA_COMMON_BASETYPES_KINARA_BASE_HPP_

// check that we're on a 64 bit machine
#if (__SIZEOF_POINTER__ < 8)
#error "KINARA currently only supports 64-bit architectures"
#endif /* __SIZEOF_POINTER__ < 8 */

#if !defined __linux__
#error "KINARA can currently only be built on linux based platforms"
#endif

#include <cstdint>
#include <exception>
#include <utility>

namespace kinara {

typedef std::uint8_t  u08;
typedef std::uint16_t u16;
typedef std::uint32_t u32;
typedef std::uint64_t u64;

typedef std::pair<u64, u64> u128;

typedef std::int8_t   i08;
typedef std::int16_t  i16;
typedef std::int32_t  i32;
typedef std::int64_t  i64;

} /* end namespace kinara */


#endif /* KINARA_KINARA_COMMON_BASETYPES_KINARA_BASE_HPP_ */

//
// KinaraBase.hpp ends here
