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

#include "../basetypes/KinaraTypes.hpp"
#include "../allocators/MemoryManager.hpp"
#include "../allocators/PoolAllocator.hpp"

#include "SListTypes.hpp"

namespace kinara {
namespace containers {

namespace ka = kinara::allocators;
namespace kc = kinara::containers;

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
    typedef slist_detail_::SListNodeBase NodeBaseType;

    union PoolSizeUnionType {
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

    // member variables
    PoolSizeUnionType m_pool_or_size;
    NodeBaseType m_node_before_head;
    NodeType* m_tail;

    // helper functions
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
        if (m_tail == nullptr) {
            return 0;
        }
        if (USEPOOLS) {
            return m_pool_or_size.m_pool_allocator->get_objects_allocated();
        } else {
            return m_pool_or_size.m_size;
        }
    }

    // constructs n objects AFTER the current position
    inline Iterator construct_fill(NodeBaseType* position, u64 n, const ValueType& value)
    {
        KINARA_ASSERT(n > 0);

        auto last_inserted = position;
        for (u64 i = 0; i < n; ++i) {
            auto node = allocate_block(value);
            last_inserted->m_next = node;
            last_inserted = node;
        }
        if (m_tail == nullptr || position == m_tail) {
            m_tail = static_cast<NodeType*>(last_inserted);
        }

        return Iterator(position->m_next);
    }

    // constructs the objects in the range AFTER the position
    template <typename InputIterator>
    inline Iterator construct_core(NodeBaseType* position, InputIterator first, InputIterator last)
    {
        KINARA_ASSERT (first != last);

        auto last_inserted = position;
        for (auto it = first; it != last; ++it) {
            auto node = allocate_block(*it);
            last_inserted->m_next = node;
            last_inserted = node;
            increment_size();
        }
        if (m_tail == nullptr || position == m_tail) {
            m_tail = static_cast<NodeType*>(last_inserted);
        }

        return Iterator(position->m_next);
    }

 public:
    explicit SListBase()
        : m_pool_or_size(), m_node_before_head(nullptr), m_tail(nullptr)
    {
        // Nothing here
    }

    explicit SListBase(u64 n)
        : SListBase(n, ValueType())
    {
        // Nothing here
    }


    explicit SListBase(u64 n, const ValueType& value)
        : SListBase()
    {
        if (n == 0) {
            return;
        }
        construct_fill(&m_node_before_head, n, value);
    }

    template <typename InputIterator>
    SListBase(InputIterator first, InputIterator last)
        : SListBase()
    {
        if (first == last) {
            return;
        }
        construct_core(&m_node_before_head, first, last);
    }

    SListBase(const SListBase& other)
        : SListBase(other.begin(), other.end())
    {
        // Nothing here
    }

    SListBase(SListBase&& other)
        : SListBase()
    {
        if (USEPOOLS) {
            m_pool_or_size.m_pool_allocator = other.m_pool_or_size.m_pool_allocator;
            other.m_pool_or_size.m_pool_allocator = nullptr;
        } else {
            m_pool_or_size.m_size = other.m_pool_or_size.m_size;
            other.m_pool_or_size.m_size = 0;
        }
        std::swap(m_node_before_head, other.m_node_before_head);
        std::swap(m_tail, other.m_tail);
    }

    SListBase(std::initializer_list<ValueType> init_list)
        : SListBase()
    {
        if (init_list.size() == 0) {
            return;
        }
        construct_core(&m_node_before_head, init_list.begin(), init_list.end());
    }

    inline void reset()
    {
        for (auto cur_node = m_node_before_head.m_next; cur_node != nullptr; ) {
            auto next_node = cur_node->m_next;
            deallocate_block(static_cast<NodeType*>(cur_node));
            cur_node = next_node;
        }
        if (USEPOOLS && m_pool_or_size.m_pool_allocator != nullptr) {
            ka::deallocate_object_raw(m_pool_or_size.m_pool_allocator, sizeof(ka::PoolAllocator));
        }
        m_node_before_head.m_next = nullptr;
        m_tail = nullptr;
    }

    ~SListBase()
    {
        reset();
    }

    template <typename InputIterator>
    inline void assign(InputIterator first, InputIterator last)
    {
        reset();
        construct_core(m_node_before_head, first, last);
    }

    template <bool OUSEPOOLS>
    inline void assign(const kc::SListBase<T, ConstructFunc, DestructFunc, OUSEPOOLS>& other)
    {
        assign(other.begin(), other.end());
    }

    void assign(u64 n, const ValueType& value)
    {
        reset();
        construct_fill(m_node_before_head, n, value);
    }

    void assign(std::initializer_list<ValueType> init_list)
    {
        assign(init_list.begin(), init_list.end());
    }

    inline SListBase& operator = (const SListBase& other)
    {
        if (&other == this) {
            return *this;
        }
        assign(other);
        return *this;
    }

    template <bool OUSEPOOLS>
    inline SListBase& operator = (const kc::SListBase<T, ConstructFunc,
                                                      DestructFunc, OUSEPOOLS>& other)
    {
        if (&other == this) {
            return *this;
        }
        assign(other);
        return *this;
    }

    inline SListBase& operator = (SListBase&& other)
    {
        if (&other == this) {
            return *this;
        }
        reset();
        if (USEPOOLS) {
            m_pool_or_size.m_pool_allocator = other.m_pool_or_size.m_pool_allocator;
            other.m_pool_or_size.m_pool_allocator = nullptr;
        } else {
            m_pool_or_size.m_size = other.m_pool_or_size.m_size;
            other.m_pool_or_size.m_size = 0;
        }
        std::swap(m_node_before_head, other.m_node_before_head);
        std::swap(m_tail, other.m_tail);
        return *this;
    }

    inline SListBase& operator = (std::initializer_list<ValueType> init_list)
    {
        assign(std::move(init_list));
    }

    Iterator before_begin() noexcept
    {
        return Iterator(&m_node_before_head);
    }

    ConstIterator before_begin() const noexcept
    {
        return ConstIterator(&m_node_before_head);
    }

    Iterator begin() noexcept
    {
        return Iterator(m_node_before_head.m_next);
    }

    ConstIterator begin() const noexcept
    {
        return ConstIterator(m_node_before_head.m_next);
    }

    Iterator end() noexcept
    {
        return Iterator(nullptr);
    }

    ConstIterator end() const noexcept
    {
        return ConstIterator(nullptr);
    }

    ConstIterator cbefore_begin() const noexcept
    {
        return before_begin();
    }

    ConstIterator cbegin() const noexcept
    {
        return begin();
    }

    ConstIterator cend() const noexcept
    {
        return end();
    }

    bool empty() const noexcept
    {
        return (m_node_before_head.m_next == nullptr);
    }

    u64 max_size() const noexcept
    {
        return UINT64_MAX;
    }

    RefType front()
    {
        return static_cast<NodeType*>(m_node_before_head.m_next)->m_value;
    }

    ConstRefType front() const
    {
        return static_cast<NodeType*>(m_node_before_head.m_next)->m_value;
    }

    RefType back()
    {
        return m_tail->m_value;
    }

    ConstRefType back() const
    {
        return m_tail->m_value;
    }

    template <typename... ArgTypes>
    void emplace_front(ArgTypes&&... args)
    {
        auto new_node = NodeType::construct(std::forward<ArgTypes>(args)...);
        new_node->m_next = m_node_before_head.m_next;
        m_node_before_head.m_next = new_node;
        increment_size();
    }

    template <typename... ArgTypes>
    void emplace_back(ArgTypes&&... args)
    {
        auto new_node = NodeType::construct(std::forward<ArgTypes>(args)...);
        m_tail->m_next = new_node;
        m_tail = new_node;
        increment_size();
    }

    void push_front(const ValueType& value)
    {
        auto new_node = NodeType::construct(value);
        new_node->m_next = m_node_before_head.m_next;
        m_node_before_head.m_next = new_node;
        increment_size();
    }

    void push_front(ValueType&& value)
    {
        auto new_node = NodeType::construct(std::move(value));
        new_node->m_next = m_node_before_head.m_next;
        m_node_before_head.m_next = new_node;
        increment_size();
    }

    void push_back(const ValueType& value)
    {
        auto new_node = NodeType::construct(value);
        m_tail->m_next = new_node;
        m_tail = new_node;
        increment_size();
    }

    void push_back(ValueType&& value)
    {
        auto new_node = NodeType::construct(std::move(value));
        m_tail->m_next = new_node;
        m_tail = new_node;
        increment_size();
    }

    void pop_front()
    {
        auto node = m_node_before_head.m_next;
        m_node_before_head.m_next = node->m_next;
        deallocate_block(static_cast<NodeType*>(node));
        decrement_size();
    }

    // This is linear in the length of the list!
    void pop_back()
    {
        auto cur_ptr = m_node_before_head.m_next;
        if (cur_ptr == nullptr) {
            return;
        }

        auto prev_ptr = cur_ptr;
        while (cur_ptr != m_tail) {
            cur_ptr = cur_ptr->m_next;
            prev_ptr = cur_ptr;
        }

        deallocate_block(cur_ptr);
        m_tail = prev_ptr;
        decrement_size();
    }

};

template <typename T,
          typename ConstructFunc = DefaultConstructFunc<T>,
          typename DestructFunc = DefaultDestructFunc<T>>
using PoolSList = SListBase<T, ConstructFunc, DestructFunc, true>;

template <typename T,
          typename ConstructFunc = DefaultConstructFunc<T>,
          typename DestructFunc = DefaultDestructFunc<T>>
using SList = SListBase<T, ConstructFunc, DestructFunc, false>;

template <typename T,
          typename ConstructFunc = DefaultConstructFunc<T*>,
          typename DestructFunc = DefaultDestructFunc<T*>>
using PoolPtrSList = SListBase<T*, ConstructFunc, DestructFunc, true>;

template <typename T,
          typename ConstructFunc = DefaultConstructFunc<T*>,
          typename DestructFunc = DefaultDestructFunc<T*>>
using PtrSList = SListBase<T*, ConstructFunc, DestructFunc, false>;

typedef PoolSList<u08> u08PoolSList;
typedef PoolSList<u16> u16PoolSList;
typedef PoolSList<u32> u32PoolSList;
typedef PoolSList<u64> u64PoolSList;
typedef PoolSList<i08> i08PoolSList;
typedef PoolSList<i16> i16PoolSList;
typedef PoolSList<i32> i32PoolSList;
typedef PoolSList<i64> i64PoolSList;

typedef SList<u08> u08SList;
typedef SList<u16> u16SList;
typedef SList<u32> u32SList;
typedef SList<u64> u64SList;
typedef SList<i08> i08SList;
typedef SList<i16> i16SList;
typedef SList<i32> i32SList;
typedef SList<i64> i64SList;

// forward declaration
class String;

typedef SList<String> StringSList;
typedef PoolSList<String> StringPoolSList;

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_SLIST_HPP_ */

//
// SList.hpp ends here
