// SList.hpp ---
// Filename: SList.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 16 02:08:41 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_SLIST_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_SLIST_HPP_

#include <initializer_list>
#include <iterator>

#include "../basetypes/KinaraBase.hpp"
#include "../basetypes/KinaraTypes.hpp"
#include "../allocators/MemoryManager.hpp"
#include "../allocators/PoolAllocator.hpp"

#include "ContainersBase.hpp"

namespace kinara {
namespace containers {

namespace ka = kinara::allocators;
namespace kc = kinara::containers;

namespace slist_detail_ {

template <typename T, typename ConstructFunc,
          typename DestructFunc>
struct SListNode
{
    T m_value;
    SListNode* m_next;

    inline SListNode() = delete;
    inline SListNode(const SListNode& other) = delete;
    inline SListNode(SListNode&& other) = delete;
    inline SListNode& operator = (const SListNode& other) = delete;
    inline SListNode& operator = (SListNode&& other) = delete;

    inline ~SListNode()
    {
        DestructFunc the_destructor;
        the_destructor(m_value);
    }

    template <typename... ArgTypes>
    static inline SListNode* construct(void* mem_ptr, ArgTypes&&... args)
    {
        SListNode* node_ptr = static_cast<SListNode*>(mem_ptr);
        ConstructFunc the_constructor;
        the_constructor(&(node_ptr->m_value), std::forward<ArgTypes>(args)...);
    }
};


template <typename T, bool ISCONST, typename ConstructFunc, typename DestructFunc>
class Iterator : public std::iterator<std::forward_iterator_tag, T, u64,
                                      typename std::conditional<ISCONST, const T*, T*>::type,
                                      typename std::conditional<ISCONST, const T&, T&>::type>
{
private:
    typedef typename
    std::conditional<ISCONST,
                     const SListNode<T, ConstructFunc, DestructFunc>*,
                     SListNode<T, ConstructFunc, DestructFunc>*>::type NodeType;

    typedef typename
    std::conditional<ISCONST, const T&, T&>::type ValRefType;

    typedef typename
    std::conditional<ISCONST, const T*, T*>::type ValPtrType;

    NodeType* m_list_node;


public:
    inline Iterator()
        : m_list_node(nullptr)
    {
        // Nothing here
    }

    inline Iterator(NodeType* list_node)
        : m_list_node(list_node)
    {
        // Nothing here
    }

    template <bool OTHERISCONST>
    inline Iterator(const kc::slist_detail_::Iterator<T, OTHERISCONST,
                                                      ConstructFunc, DestructFunc>& other)
        : m_list_node(other.m_list_node)
    {
        static_assert(!OTHERISCONST || ISCONST,
                      "Cannot construct non-const iterator from const iterator");
    }

    inline ~Iterator()
    {
        // Nothing here
    }

    template <bool OTHERISCONST>
    inline Iterator&
    operator = (const kc::slist_detail_::Iterator<T, OTHERISCONST,
                                                  ConstructFunc, DestructFunc>& other)
    {
        static_assert(!OTHERISCONST || ISCONST,
                      "Cannot assign const iterator to non-const iterator");

        if (&other == this) {
            return *this;
        }
        m_list_node = other.m_list_node;
        return *this;
    }

    template <bool OTHERISCONST>
    inline bool
    operator == (const kc::slist_detail_::Iterator<T, OTHERISCONST,
                                                   ConstructFunc, DestructFunc>& other) const
    {
        return (m_list_node == other.m_list_node);
    }

    template <bool OTHERISCONST>
    inline bool
    operator != (const kc::slist_detail_::Iterator<T, OTHERISCONST,
                                                   ConstructFunc, DestructFunc>& other) const
    {
        return (m_list_node != other.m_list_node);
    }

    inline Iterator& operator ++ ()
    {
        m_list_node = m_list_node->get_next();
    }

    inline Iterator operator ++ (int unused)
    {
        auto retval = *this;
        m_list_node = m_list_node->get_next();
        return retval;
    }

    inline ValRefType operator * () const
    {
        return m_list_node->get_value();
    }

    inline ValPtrType operator -> () const
    {
        return &(m_list_node->get_value());
    }
};

} /* end namespace slist_detail_ */

template <typename T, typename ConstructFunc,
          typename DestructFunc, bool USEPOOLS>
class SListBase
{
public:
    typedef T ValueType;
    typedef T* PtrType;
    typedef const T* ConstPtrType;
    typedef T& RefType;
    typedef const T& ConstRefType;

    typedef slist_detail_::Iterator<T, false, ConstructFunc, DestructFunc> Iterator;
    typedef Iterator iterator;
    typedef slist_detail_::Iterator<T, true, ConstructFunc, DestructFunc> ConstIterator;
    typedef ConstIterator const_iterator;

    // No reverse iteration is possible!

private:
    typedef slist_detail_::SListNode<T, ConstructFunc, DestructFunc> NodeType;

    union PoolSizeUnionType {
        ka::PoolAllocator* m_pool_allocator;
        u64 m_size;

        PoolSizeUnionType()
            : m_pool_allocator(nullptr)
        {
            // Nothing here
        }

        ~PoolSizeUnionType()
        {
            // Nothing here
        }
    };

    PoolSizeUnionType m_pool_or_size;
    NodeType* m_list_head;
    NodeType* m_list_tail;

    template <typename... ArgTypes>
    inline NodeType* allocate_block(ArgTypes&&... args)
    {
        if (USEPOOLS) {
            if (m_pool_or_size.m_pool_allocator == nullptr) {
                m_pool_or_size.m_pool_allocator =
                    ka::allocate_object_raw<ka::PoolAllocator>(sizeof(NodeType));
            }
            auto ptr = m_pool_or_size.m_pool_allocator->allocate();
            return NodeType::construct(ptr, std::forward<ArgTypes>(args)...);
        } else {
            auto ptr = ka::allocate_raw(sizeof(NodeType));
            return NodeType::construct(ptr, std::forward<ArgTypes>(args)...);
        }
    }

    inline void deallocate_block(NodeType* node)
    {
        if (USEPOOLS) {
            ka::deallocate(*(m_pool_or_size.m_pool_allocator), node);
        } else {
            ka::deallocate_raw(node, sizeof(NodeType));
        }
    }

    inline void increment_size()
    {
        if (!USEPOOLS) {
            m_pool_or_size.m_size++;
        }
    }

    inline void decrement_size()
    {
        if (!USEPOOLS) {
            m_pool_or_size.m_size--;
        }
    }

    inline u64 get_size() const
    {
        if (m_list_head == nullptr) {
            return 0;
        }
        if (USEPOOLS) {
            return m_pool_or_size.m_pool_allocator->get_objects_allocated();
        } else {
            return m_pool_or_size.m_size;
        }
    }

public:
    explicit SListBase()
        : m_pool_or_size(), m_list_head(nullptr)
    {
        // Nothing here
    }

    explicit SListBase(u64 n)
        : SListBase(n, ValueType())
    {
        // Nothing here
    }


    explicit SListBase(u64 n, const ValueType& value)
        : m_pool_or_size(), m_list_head(nullptr)
    {
        // allocate n blocks all initialized to value
        for (u64 i = 0; i < n; ++i) {
            auto node = allocate_block(value);
            node->m_next = nullptr;
            if (m_list_head == nullptr) {
                m_list_head = node;
                m_list_tail = node;
            } else {
                m_list_tail->m_next = node;
                m_list_tail = node;
            }
            increment_size();
        }
    }

    template <typename InputIterator>
    SListBase(const InputIterator& first, const InputIterator& last)
        : m_pool_or_size(), m_list_head(nullptr), m_list_tail(nullptr)
    {
        for (auto it = first; it != last; ++it) {
            auto node = allocate_block(*it);
            if (m_list_head == nullptr) {
                m_list_head = node;
                m_list_tail = node;
            } else {
                m_list_tail->m_next = node;
                m_list_tail = node;
            }
            increment_size();
        }
    }

    SListBase(const SListBase& other)
        : m_pool_or_size(), m_list_head(nullptr), m_list_tail(nullptr)
    {
        for (auto it = other.begin(), last = other.end(); it != last; ++it) {
            auto node = allocate_block(*it);
            if (m_list_head == nullptr) {
                m_list_head = node;
                m_list_tail = node;
            } else {
                m_list_tail->m_next = node;
                m_list_tail = node;
            }
            increment_size();
        }
    }

};

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_SLIST_HPP_ */

//
// SList.hpp ends here
