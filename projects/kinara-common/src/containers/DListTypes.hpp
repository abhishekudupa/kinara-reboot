// DListTypes.hpp ---
//
// Filename: DListTypes.hpp
// Author: Abhishek Udupa
// Created: Mon Mar  2 17:47:59 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_DLIST_TYPES_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_DLIST_TYPES_HPP_

#include <iterator>

#include "../basetypes/KinaraBase.hpp"
#include "ContainersBase.hpp"

namespace kinara {
namespace containers {

template <typename T, typename ConstructFunc, typename DestructFunc, bool USEPOOLS>
class DListBase;

namespace dlist_detail_ {

namespace kc = kinara::containers;

struct DListNodeBase
{
    DListNodeBase* m_next;
    DListNodeBase* m_prev;

    inline DListNodeBase()
        : m_next(nullptr), m_prev(nullptr)
    {
        // Nothing here
    }

    inline DListNodeBase(DListNodeBase* next, DListNodeBase* prev)
        : m_next(next), m_prev(prev)
    {
        // Nothing here
    }

    inline ~DListNodeBase()
    {
        // Nothing here
    }
};

template <typename T, typename ConstructFunc, typename DestructFunc>
struct DListNode : public DListNodeBase
{
    T m_value;

    inline DListNode() = delete;
    inline DListNode(const DListNode& other) = delete;
    inline DListNode(DListNode&& other) = delete;
    inline DListNode& operator = (const DListNode& other) = delete;
    inline DListNode& operator = (DListNode&& other) = delete;

    inline ~DListNode()
    {
        DestructFunc the_destructor;
        the_destructor(m_value);
    }

    template <typename... ArgTypes>
    static inline DListNode* construct(void* mem_ptr, ArgTypes&&... args)
    {
        DListNode* node_ptr = static_cast<DListNode*>(mem_ptr);
        node_ptr->m_next = nullptr;
        node_ptr->m_prev = nullptr;
        ConstructFunc the_constructor;
        the_constructor(&(node_ptr->m_value), std::forward<ArgTypes>(args)...);
        return node_ptr;
    }
};

template <typename T, typename ConstructFunc, typename DestructFunc, bool ISCONST>
class IteratorBase
    : public std::iterator<std::bidirectional_iterator_tag, T, i64,
                           typename std::conditional<ISCONST, const T*, T*>::type,
                           typename std::conditional<ISCONST, const T&, T&>::type>
{
    friend class DListBase<T, ConstructFunc, DestructFunc, true>;
    friend class DListBase<T, ConstructFunc, DestructFunc, false>;
    friend class kc::dlist_detail_::IteratorBase<T, ConstructFunc, DestructFunc, true>;
    friend class kc::dlist_detail_::IteratorBase<T, ConstructFunc, DestructFunc, false>;

private:
    typedef DListNode<T, ConstructFunc, DestructFunc> NodeType;
    typedef DListNodeBase NodeBaseType;
    typedef typename std::conditional<ISCONST, const T&, T&>::type ValRefType;
    typedef typename std::conditional<ISCONST, const T*, T*>::type ValPtrType;

    inline NodeBaseType* get_node() const
    {
        return m_node;
    }

    inline operator NodeBaseType () const
    {
        return m_node;
    }

    NodeBaseType* m_node;

public:
    inline IteratorBase()
        : m_node(nullptr)
    {
        // Nothing here
    }

    inline IteratorBase(NodeBaseType* node)
        : m_node(node)
    {
        // Nothing here
    }

    inline IteratorBase(const IteratorBase& other)
        : m_node(other.m_node)
    {
        // Nothing here
    }

    template <bool OISCONST>
    inline IteratorBase(const kc::dlist_detail_::IteratorBase<T, ConstructFunc,
                                                              DestructFunc, OISCONST>& other)
        : m_node(other.m_node)
    {
        static_assert(((!OISCONST) || ISCONST),
                      "Cannot construct const iterator "
                      "from non-const iterator");
    }

    inline ~IteratorBase()
    {
        // Nothing here
    }

    inline IteratorBase& operator = (const IteratorBase& other)
    {
        if (&other == this) {
            return *this;
        }
        m_node = other.m_node;
        return *this;
    }

    template <bool OISCONST>
    inline IteratorBase&
    operator = (const kc::dlist_detail_::IteratorBase<T, ConstructFunc,
                                                      DestructFunc, OISCONST>& other)
    {
        if (&other == this) {
            return *this;
        }

        m_node = other.m_node;
        return *this;
    }

    inline IteratorBase& operator ++ ()
    {
        m_node = m_node->m_next;
        return *this;
    }

    inline IteratorBase operator ++ (int unused)
    {
        auto retval = *this;
        m_node = m_node->m_next;
        return retval;
    }

    inline IteratorBase& operator -- ()
    {
        m_node = m_node->m_prev;
        return *this;
    }

    inline IteratorBase operator -- (int unused)
    {
        auto retval = *this;
        m_node = m_node->m_prev;
        return retval;
    }

    inline ValRefType operator * () const
    {
        return static_cast<NodeType*>(m_node)->m_value;
    }

    inline ValPtrType operator -> () const
    {
        return (&(static_cast<NodeType*>(m_node)->m_value));
    }

    inline bool operator == (const IteratorBase& other) const
    {
        return (m_node == other.m_node);
    }

    inline bool operator != (const IteratorBase& other) const
    {
        return (m_node != other.m_node);
    }

    template <bool OISCONST>
    inline bool
    operator == (const kc::dlist_detail_::IteratorBase<T, ConstructFunc,
                                                       DestructFunc, OISCONST>& other)
    {
        return (m_node == other.m_node);
    }

    template <bool OISCONST>
    inline bool
    operator != (const kc::dlist_detail_::IteratorBase<T, ConstructFunc,
                                                       DestructFunc, OISCONST>& other)
    {
        return (m_node != other.m_node);
    }
};

template <typename T, typename ConstructFunc, typename DestructFunc>
using Iterator = IteratorBase<T, ConstructFunc, DestructFunc, false>;

template <typename T, typename ConstructFunc, typename DestructFunc>
using ConstIterator = IteratorBase<T, ConstructFunc, DestructFunc, true>;

} /* end namespace dlist_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_DLIST_TYPES_HPP_ */

//
// DListTypes.hpp ends here
