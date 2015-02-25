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

// forward declaration of list class
template <typename T, typename ConstructFunc, typename DestructFunc, bool USEPOOLS>
class SListBase;

namespace slist_detail_ {

// Just a pointer to the next node
struct SListNodeBase
{
    SListNodeBase* m_next;

    inline SListNodeBase()
        : m_next(nullptr)
    {
        // Nothing here
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

template <typename T, typename ConstructFunc,
          typename DestructFunc>
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

template <typename T, typename ConstructFunc, typename DestructFunc, bool ISCONST>
class IteratorBase :
        public std::iterator<std::forward_iterator_tag, T, u64,
                             typename std::conditional<ISCONST, const T*, T*>::type,
                             typename std::conditional<ISCONST, const T&, T&>::type>
{
    friend class SListBase<T, ConstructFunc, DestructFunc, true>;
    friend class SListBase<T, ConstructFunc, DestructFunc, false>;

private:
    typedef SListNode<T, ConstructFunc, DestructFunc> NodeType;
    typedef typename std::conditional<ISCONST, const T*, T*>::type ValPtrType;
    typedef typename std::conditional<ISCONST, const T&, T&>::type ValRefType;

    // helper to get the node
    inline NodeType* get_node() const
    {
        return m_node;
    }

    inline operator NodeType () const
    {
        return m_node;
    }

protected:
    NodeType* m_node;

public:
    inline IteratorBase()
        : m_node(nullptr)
    {
        // Nothing here
    }

    inline IteratorBase(NodeType* node)
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
    IteratorBase(const kc::slist_detail_::IteratorBase<T, ConstructFunc, DestructFunc, OISCONST>&
                 other)
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
    operator = (const kc::slist_detail_::IteratorBase<T, ConstructFunc, DestructFunc, OISCONST>&
                other)
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
    }

    inline IteratorBase operator ++ (int unused)
    {
        auto retval = *this;
        m_node = m_node->m_next;
        return retval;
    }

    inline ValRefType operator * ()
    {
        return m_node->m_value;
    }

    inline ValRefType operator -> ()
    {
        return m_node->m_value;
    }

    inline bool operator == (const IteratorBase& other) const
    {
        return (m_node == other.m_node);
    }

    template <bool OISCONST>
    inline bool
    operator == (const kc::slist_detail_::IteratorBase<T, ConstructFunc, DestructFunc, OISCONST>&
                 other) const
    {
        return (m_node == other.m_node);
    }


    inline bool operator != (const IteratorBase& other) const
    {
        return (m_node != other.m_node);
    }

    template <bool OISCONST>
    inline bool
    operator != (const kc::slist_detail_::IteratorBase<T, ConstructFunc, DestructFunc, OISCONST>&
                 other) const
    {
        return (m_node != other.m_node);
    }
};

template <typename T, typename ConstructFunc, typename DestructFunc>
using Iterator = IteratorBase<T, ConstructFunc, DestructFunc, false>;

template <typename T, typename ConstructFunc, typename DestructFunc>
using ConstIterator = IteratorBase<T, ConstructFunc, DestructFunc, true>;

} /* end namespace slist_detail_ */

/*
   A singly-linked list:
   - push_back, push_front are constant time operations
   - pop_front is also constant time
   - pop_back is linear time
   - inserts are constant time
   - searches are linear time
   - overhead = (3 words) + (n * 1 word)
*/

template <typename T, typename ConstructFunc,
          typename DestructFunc, bool USEPOOLS>
class SListBase final
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

    inline void add_to_size(u64 addend)
    {
        if (!USEPOOLS) {
            m_pool_or_size.m_size += addend;
        }
    }

    inline void sub_from_size(u64 subend)
    {
        if (!USEPOOLS) {
            m_pool_or_size.m_size -= subend;
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

    // constructs n objects AFTER the current position
    inline Iterator construct_core(Iterator position, u64 n, const ValueType& value)
    {

    }

    // constructs the objects in the range AFTER the position
    template <typename InputIterator>
    inline void construct_core(Iterator position, InputIterator first, InputIterator last)
    {

    }

public:
    explicit SListBase()
        : m_pool_or_size(), m_list_head(nullptr), m_list_tail(nullptr)
    {

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
