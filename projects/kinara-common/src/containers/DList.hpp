// DList.hpp ---
// Filename: DList.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 02:08:46 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_DLIST_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_DLIST_HPP_

#include <initializer_list>

#include "../basetypes/KinaraTypes.hpp"
#include "../allocators/MemoryManager.hpp"
#include "../allocators/PoolAllocator.hpp"

#include "DListTypes.hpp"

namespace kinara {
namespace containers {

namespace ka = kinara::allocators;
namespace kc = kinara::containers;

template <typename T, typename ConstructFunc,
          typename DestructFunc, bool USEPOOLS>
class DListBase final
{
public:
    typedef T ValueType;
    typedef T* PtrType;
    typedef const T* ConstPtrType;
    typedef T& RefType;
    typedef const T& ConstRefType;

    typedef slist_detail_::Iterator<T, ConstructFunc, DestructFunc> Iterator;
    typedef Iterator iterator;
    typedef slist_detail_::ConstIterator<T, ConstructFunc, DestructFunc> ConstIterator;
    typedef ConstIterator const_iterator;
    typedef std::reverse_iterator<iterator> ReverseIterator;
    typedef ReverseIterator reverse_iterator;
    typedef std::reverse_iterator<const_iterator> ConstReverseIterator;
    typedef ConstReverseIterator const_reverse_iterator;

private:
    typedef dlist_detail_::DListNode NodeType;

    union PoolSizeUnionType
    {
        ka::PoolAllocator* m_pool_allocator;
        u64 m_size;

        PoolSizeUnionType()
            : m_size(0)
        {
            // Nothing here
        }

        ~PoolSizeUnionType()
        {
            // Nothing here
        }
    };

    PoolSizeUnionType m_pool_or_size;
    NodeType* m_head;
    NodeType* m_tail;

};

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_DLIST_HPP_ */

//
// DList.hpp ends here
