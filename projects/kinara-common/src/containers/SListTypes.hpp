// SListTypes.hpp ---
//
// Filename: SListTypes.hpp
// Author: Abhishek Udupa
// Created: Thu Feb 26 15:51:00 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_SLIST_TYPES_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_SLIST_TYPES_HPP_

#include <iterator>

#include "../basetypes/KinaraBase.hpp"
#include "ContainersBase.hpp"

namespace kinara {
namespace containers {

// forward declaration of list class
template <typename T, bool USEPOOLS> class SListBase;

namespace slist_detail_ {

namespace kc = kinara::containers;

// Just a pointer to the next node
struct SListNodeBase
{
    SListNodeBase* m_next;

    inline SListNodeBase()
        : m_next(nullptr)
    {
        // Nothing here
    }

    inline SListNodeBase(const SListNodeBase& other)
        : m_next(other.m_next)
    {
        // Nothing here
    }

    inline SListNodeBase(SListNodeBase&& other)
        : SListNodeBase()
    {
        std::swap(m_next, other.m_next);
    }

    inline SListNodeBase& operator = (const SListNodeBase& other)
    {
        if (&other == this) {
            return *this;
        }
        m_next = other.m_next;
        return *this;
    }

    inline SListNodeBase& operator = (SListNodeBase&& other)
    {
        if (&other == this) {
            return *this;
        }
        m_next = nullptr;
        std::swap(m_next, other.m_next);
        return *this;
    }

    inline SListNodeBase(SListNodeBase* next)
        : m_next(next)
    {
        // Nothing here
    }

    inline ~SListNodeBase()
    {
        // Nothing here
    }
};

template <typename T>
struct SListNode : public SListNodeBase
{
    T m_value;

    inline SListNode() = delete;
    inline SListNode(const SListNode& other) = delete;
    inline SListNode(SListNode&& other) = delete;
    inline SListNode& operator = (const SListNode& other) = delete;
    inline SListNode& operator = (SListNode&& other) = delete;

    inline ~SListNode()
    {
        // Nothing here
    }

    template <typename... ArgTypes>
    static inline SListNode* construct(void* mem_ptr, ArgTypes&&... args)
    {
        SListNode* node_ptr = static_cast<SListNode*>(mem_ptr);
        node_ptr->m_next = nullptr;
        new (&(node_ptr->m_value)) T(std::forward<ArgTypes>(args)...);
        return node_ptr;
    }
};

template <typename T, bool ISCONST>
class IteratorBase :
        public std::iterator<std::forward_iterator_tag, T, i64,
                             typename std::conditional<ISCONST, const T*, T*>::type,
                             typename std::conditional<ISCONST, const T&, T&>::type>
{
    friend class SListBase<T, true>;
    friend class SListBase<T, false>;
    friend class kc::slist_detail_::IteratorBase<T, true>;
    friend class kc::slist_detail_::IteratorBase<T, false>;

private:
    typedef SListNode<T> NodeType;
    typedef SListNodeBase NodeBaseType;
    typedef typename std::conditional<ISCONST, const T*, T*>::type ValPtrType;
    typedef typename std::conditional<ISCONST, const T&, T&>::type ValRefType;

    // helper to get the node
    inline NodeBaseType* get_node() const
    {
        return m_node;
    }

    inline operator NodeBaseType () const
    {
        return m_node;
    }

    // The only member
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
    inline
    IteratorBase(const kc::slist_detail_::IteratorBase<T, OISCONST>& other)
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
    operator = (const kc::slist_detail_::IteratorBase<T, OISCONST>& other)
    {
        static_assert(((!OISCONST) || ISCONST),
                      "Cannot assign const iterator to non-const iterator");

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

    inline ValRefType operator * () const
    {
        return (static_cast<NodeType*>(m_node))->m_value;
    }

    inline ValPtrType operator -> () const
    {
        return (&((static_cast<NodeType*>(m_node))->m_value));
    }

    template <bool OISCONST>
    inline bool
    operator == (const kc::slist_detail_::IteratorBase<T, OISCONST>& other) const
    {
        return (m_node == other.m_node);
    }

    template <bool OISCONST>
    inline bool
    operator != (const kc::slist_detail_::IteratorBase<T, OISCONST>& other) const
    {
        return (m_node != other.m_node);
    }
};

template <typename T>
using Iterator = IteratorBase<T, false>;

template <typename T>
using ConstIterator = IteratorBase<T, true>;

} /* end namespace slist_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_SLIST_TYPES_HPP_ */

//
// SListTypes.hpp ends here
