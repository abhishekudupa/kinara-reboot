// Deque.hpp ---
// Filename: Deque.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 02:09:37 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_DEQUE_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_DEQUE_HPP_

#include <initializer_list>

#include "../basetypes/KinaraTypes.hpp"

#include "DequeTypes.hpp"

namespace kinara {
namespace containers {

namespace ka = kinara::allocators;
namespace kc = kinara::containers;

template <typename T, typename ConstructFunc, typename DestructFunc>
class DequeBase final :
        protected deque_detail_::DequeInternal<T, ConstructFunc, DestructFunc>
{
public:
    typedef T ValueType;
    typedef T* PtrType;
    typedef T& RefType;
    typedef const T* ConstPtrType;
    typedef const T& ConstRefType;
    typedef deque_detail_::DequeInternal<T, ConstructFunc, DestructFunc> BaseType;

private:

public:
    explicit DequeBase()
        : BaseType()
    {
        // Nothing here
    }

};

// Some useful typedefs
template <typename T,
          typename ConstructFunc = DefaultConstructFunc<T>,
          typename DestructFunc = DefaultDestructFunc<T>>
using Deque = DequeBase<T, ConstructFunc, DestructFunc>;

template <typename T,
          typename ConstructFunc = DefaultConstructFunc<T*>,
          typename DestructFunc = DefaultConstructFunc<T*>>
using PtrDeque = DequeBase<T*, ConstructFunc, DestructFunc>;

typedef Deque<u08> u08Deque;
typedef Deque<u16> u16Deque;
typedef Deque<u32> u32Deque;
typedef Deque<u64> u64Deque;
typedef Deque<i08> i08Deque;
typedef Deque<i16> i16Deque;
typedef Deque<i32> i32Deque;
typedef Deque<i64> i64Deque;

// forward declaration
class String;

typedef Deque<String> StringDeque;

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_DEQUE_HPP_ */

//
// Deque.hpp ends here
