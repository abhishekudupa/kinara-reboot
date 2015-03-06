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
            ka::deallocate_object_raw(node, sizeof(NodeType));
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

    inline void set_size(u64 new_size)
    {
        if (!USEPOOLS) {
            m_pool_or_size.m_size = new_size;
        }
    }

    // constructs n objects AFTER the current position
    inline Iterator construct_fill(NodeBaseType* position, u64 n, const ValueType& value)
    {
        KINARA_ASSERT(n > 0);

        auto last_inserted = position;
        auto next_of_position = position->m_next;
        for (u64 i = 0; i < n; ++i) {
            auto node = allocate_block(value);
            last_inserted->m_next = node;
            last_inserted = node;
        }

        last_inserted->m_next = next_of_position;

        if (m_tail == nullptr || position == m_tail) {
            m_tail = static_cast<NodeType*>(last_inserted);
        }

        add_to_size(n);
        return Iterator(position->m_next);
    }

    // constructs the objects in the range AFTER the position
    template <typename InputIterator>
    inline Iterator construct_core(NodeBaseType* position,
                                   const InputIterator& first,
                                   const InputIterator& last)
    {
        KINARA_ASSERT (first != last);

        auto last_inserted = position;
        auto next_of_position = position->m_next;
        for (auto it = first; it != last; ++it) {
            auto node = allocate_block(*it);
            last_inserted->m_next = node;
            last_inserted = node;
            increment_size();
        }

        last_inserted->m_next = next_of_position;

        if (m_tail == nullptr || position == m_tail) {
            m_tail = static_cast<NodeType*>(last_inserted);
        }

        return Iterator(position->m_next);
    }

    template <typename... ArgTypes>
    inline Iterator construct_after(NodeBaseType* position, ArgTypes&&... args)
    {
        auto node = allocate_block(std::forward<ArgTypes>(args)...);
        node->m_next = position->m_next;
        position->m_next = node;
        increment_size();
        if (m_tail == nullptr || position == m_tail) {
            m_tail = node;
        }
        return Iterator(node);
    }

    inline NodeBaseType* find_node_before(NodeBaseType* position) const
    {
        for (auto cur_node = &m_node_before_head;
             cur_node != nullptr;
             cur_node = cur_node->m_next) {

            if (cur_node->m_next == position) {
                return const_cast<NodeBaseType*>(cur_node);
            }
        }
        return nullptr;
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
    SListBase(const InputIterator& first, const InputIterator& last)
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

    template <bool OUSEPOOLS>
    SListBase(const kc::SListBase<T, ConstructFunc, DestructFunc, OUSEPOOLS>& other)
        : SListBase(std::move(other))
    {
        // Nothing here
    }

    template <bool OUSEPOOLS>
    SListBase(kc::SListBase<T, ConstructFunc, DestructFunc, OUSEPOOLS>&& other)
        : SListBase()
    {
        if (other.size() == 0) {
            return;
        }
        construct_core(&m_node_before_head, other.begin(), other.end());
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
            m_pool_or_size.m_pool_allocator = nullptr;
        }
        m_node_before_head.m_next = nullptr;
        m_tail = nullptr;
        set_size(0);
    }

    ~SListBase()
    {
        reset();
    }

    template <typename InputIterator>
    inline void assign(const InputIterator& first, const InputIterator& last)
    {
        reset();
        if (last == first) {
            return;
        }
        construct_core(&m_node_before_head, first, last);
    }

    template <bool OUSEPOOLS>
    inline void assign(const kc::SListBase<T, ConstructFunc, DestructFunc, OUSEPOOLS>& other)
    {
        assign(other.begin(), other.end());
    }

    void assign(u64 n, const ValueType& value)
    {
        reset();
        if (n == 0) {
            return;
        }
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
        return *this;
    }

    Iterator before_begin() noexcept
    {
        return Iterator(&m_node_before_head);
    }

    ConstIterator before_begin() const noexcept
    {
        return ConstIterator(const_cast<NodeBaseType*>(&m_node_before_head));
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
        return (m_tail == nullptr);
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
        auto new_node = allocate_block(std::forward<ArgTypes>(args)...);
        new_node->m_next = m_node_before_head.m_next;
        m_node_before_head.m_next = new_node;
        if (m_tail == nullptr) {
            m_tail = new_node;
        }
        increment_size();
    }

    template <typename... ArgTypes>
    void emplace_back(ArgTypes&&... args)
    {
        auto new_node = allocate_block(std::forward<ArgTypes>(args)...);
        if (m_tail != nullptr) {
            m_tail->m_next = new_node;
        } else {
            m_node_before_head.m_next = new_node;
        }
        m_tail = new_node;
        increment_size();
    }

    void push_front(const ValueType& value)
    {
        auto new_node = allocate_block(value);
        new_node->m_next = m_node_before_head.m_next;
        m_node_before_head.m_next = new_node;
        if (m_tail == nullptr) {
            m_tail = new_node;
        }
        increment_size();
    }

    void push_front(ValueType&& value)
    {
        auto new_node = allocate_block(std::move(value));
        new_node->m_next = m_node_before_head.m_next;
        m_node_before_head.m_next = new_node;
        if (m_tail == nullptr) {
            m_tail = new_node;
        }
        increment_size();
    }

    void push_back(const ValueType& value)
    {
        auto new_node = allocate_block(value);
        if (m_tail != nullptr) {
            m_tail->m_next = new_node;
        } else {
            m_node_before_head.m_next = new_node;
        }
        m_tail = new_node;
        increment_size();
    }

    void push_back(ValueType&& value)
    {
        auto new_node = allocate_block(std::move(value));
        if (m_tail != nullptr) {
            m_tail->m_next = new_node;
        } else {
            m_node_before_head.m_next = new_node;
        }
        m_tail = new_node;
        increment_size();
    }

    void pop_front()
    {
        auto node = m_node_before_head.m_next;
        if (node == nullptr) {
            return;
        }
        m_node_before_head.m_next = node->m_next;
        deallocate_block(static_cast<NodeType*>(node));
        if (m_node_before_head.m_next == nullptr) {
            m_tail = nullptr;
        }
        decrement_size();
    }

    // This is linear in the length of the list!
    void pop_back()
    {
        if (m_tail == nullptr) {
            return;
        }
        auto node_before_last = find_node_before(m_tail);
        deallocate_block(static_cast<NodeType*>(m_tail));
        node_before_last->m_next = nullptr;
        if (node_before_last == &m_node_before_head) {
            m_tail = nullptr;
        } else {
            m_tail = static_cast<NodeType*>(node_before_last);
        }
        decrement_size();
    }

    template <typename... ArgTypes>
    Iterator emplace_after(const ConstIterator& position, ArgTypes&&... args)
    {
        return construct_after(position.get_node(), std::forward<ArgTypes>(args)...);
    }

    // linear time operation!
    template <typename... ArgTypes>
    Iterator emplace(const ConstIterator& position, ArgTypes&&... args)
    {
        if (position == cbefore_begin()) {
            throw KinaraException("Cannot emplace before the beginning of an SList");
        }
        auto node_before = find_node_before(position);
        return construct_after(node_before, std::forward<ArgTypes>(args)...);
    }

    Iterator insert_after(const ConstIterator& position, const ValueType& value)
    {
        return construct_after(position.get_node(), value);
    }

    Iterator insert_after(const ConstIterator& position, ValueType&& value)
    {
        return construct_after(position.get_node(), std::move(value));
    }

    Iterator insert_after(const ConstIterator& position, u64 n, const ValueType& value)
    {
        return construct_fill(position.get_node(), n, value);
    }

    template <typename InputIterator>
    Iterator insert_after(const ConstIterator& position,
                          const InputIterator& first,
                          const InputIterator& last)
    {
        return construct_core(position.get_node(), first, last);
    }

    Iterator insert_after(const ConstIterator& position,
                          const std::initializer_list<ValueType>& init_list)
    {
        return insert_after(position, init_list.begin(), init_list.end());
    }

    Iterator insert(const ConstIterator& position, const ValueType& value)
    {
        if (position == cbefore_begin()) {
            throw KinaraException("Cannot insert before the beginning of an SList");
        }
        auto node_before = find_node_before(position.get_node());
        return construct_after(node_before, value);
    }

    Iterator insert(const ConstIterator& position, ValueType&& value)
    {
        if (position == cbefore_begin()) {
            throw KinaraException("Cannot insert before the beginning of an SList");
        }

        auto node_before = find_node_before(position.get_node());
        return construct_after(node_before, std::move(value));
    }

    Iterator insert(const ConstIterator& position, u64 n, const ValueType& value)
    {
        if (position == cbefore_begin()) {
            throw KinaraException("Cannot insert before the beginning of an SList");
        }

        auto node_before = find_node_before(position.get_node());
        return construct_fill(node_before, n, value);
    }

    template <typename InputIterator>
    Iterator insert(const ConstIterator& position,
                    const InputIterator& first,
                    const InputIterator& last)
    {
        if (position == cbefore_begin()) {
            throw KinaraException("Cannot insert before the beginning of an SList");
        }

        auto node_before = find_node_before(position.get_node());
        return construct_core(node_before, first, last);
    }

    Iterator insert(const ConstIterator& position,
                    const std::initializer_list<ValueType>& init_list)
    {
        if (position == cbefore_begin()) {
            throw KinaraException("Cannot insert before the beginning of an SList");
        }

        return insert(position, init_list.begin(), init_list.end());
    }

    Iterator erase_after(const ConstIterator& position)
    {
        auto node = position.get_node();
        auto node_to_erase = node->m_next;
        if (node_to_erase == nullptr) {
            return end();
        }
        node->m_next = node_to_erase->m_next;
        deallocate_block(static_cast<NodeType*>(node_to_erase));
        decrement_size();

        if (m_node_before_head.m_next == nullptr) {
            m_tail = nullptr;
        } else if (node_to_erase == m_tail) {
            m_tail = static_cast<NodeType*>(node);
        }
        return Iterator(node->m_next);
    }

    Iterator erase_after(const ConstIterator& first, const ConstIterator& last)
    {
        auto node_before_first = first.get_node();
        auto last_node = last.get_node();
        NodeBaseType* new_tail = nullptr;

        for (auto node = node_before_first->m_next; node != last_node; ) {
            auto node_to_erase = node;
            if (node_to_erase == nullptr) {
                break;
            }
            node = node->m_next;
            deallocate_block(static_cast<NodeType*>(node_to_erase));
            decrement_size();

            if (node_to_erase == m_tail) {
                new_tail = node;
            }
        }

        node_before_first->m_next = last_node;

        if (m_node_before_head.m_next == nullptr) {
            m_tail = nullptr;
        } else if (new_tail != nullptr) {
            m_tail = static_cast<NodeType*>(new_tail);
        }
        return Iterator(last.get_node());
    }

    Iterator erase(const ConstIterator& position)
    {
        if (position == cbefore_begin()) {
            throw KinaraException("Cannot erase before the beginning of an SList");
        }

        auto node_before = find_node_before(position.get_node());
        return erase_after(ConstIterator(node_before));
    }

    Iterator erase(const ConstIterator& first, const ConstIterator& last)
    {
        if (first == cbefore_begin()) {
            throw KinaraException("Cannot erase before the beginning of an SList");
        }

        auto node_before = find_node_before(first.get_node());
        return erase_after(ConstIterator(node_before), last);
    }

    void swap(SListBase& other)
    {
        if (USEPOOLS) {
            std::swap(m_pool_or_size.m_pool_allocator, other.m_pool_or_size.m_pool_allocator);
        } else {
            std::swap(m_pool_or_size.m_size, other.m_pool_or_size.m_size);
        }
        std::swap(m_node_before_head.m_next, other.m_node_before_head.m_next);
        std::swap(m_tail, other.m_tail);
    }

    void resize(u64 n, const ValueType& value = ValueType())
    {
        auto orig_size = get_size();
        if (n == orig_size) {
            return;
        }
        if (n > orig_size) {
            construct_fill(m_tail, n - orig_size, value);
            return;
        }
        if (n == 0) {
            reset();
            return;
        }
        // advance to the nth element
        auto first_node_to_erase = m_node_before_head.m_next;
        auto node_before_first_node = &m_node_before_head;
        for (u64 i = 0; i < n; ++i) {
            node_before_first_node = first_node_to_erase;
            first_node_to_erase = first_node_to_erase->m_next;
        }
        while(first_node_to_erase != nullptr) {
            auto node = first_node_to_erase;
            if (node == nullptr) {
                break;
            }
            first_node_to_erase = node->m_next;
            deallocate_block(static_cast<NodeType*>(node));
        }
        node_before_first_node->m_next = nullptr;
        m_tail = static_cast<NodeType*>(node_before_first_node);
        set_size(n);
    }

    u64 size() const
    {
        return get_size();
    }

    void clear()
    {
        reset();
    }

    // Splices in the other list
    // into this list AFTER position
    void splice_after(const ConstIterator& position, SListBase& other)
    {
        if (&other == this) {
            throw KinaraException("Cannot splice an SListBase onto itself");
        }

        splice_after(position, std::move(other));
    }

    // Splices in the other list
    // into this list AFTER position
    void splice_after(const ConstIterator& position, SListBase&& other)
    {
        if (&other == this) {
            throw KinaraException("Cannot splice an SListBase onto itself");
        }
        if (other.size() == 0) {
            return;
        }

        auto osize = other.size();
        auto node_to_splice_after = position.get_node();
        other.m_tail->m_next = node_to_splice_after->m_next;
        node_to_splice_after->m_next = other.m_node_before_head.m_next;
        if (node_to_splice_after == m_tail) {
            m_tail = other.m_tail;
        }

        // merge the pool allocator if we're using pools
        if (USEPOOLS) {
            m_pool_or_size.m_pool_allocator->merge(other.m_pool_or_size.m_pool_allocator);
        } else {
            add_to_size(osize);
        }

        other.m_node_before_head.m_next = nullptr;
        other.m_tail = nullptr;
        other.reset();
    }

    // Splices in the element AFTER other_position
    // into this list AFTER position
    void splice_after(const ConstIterator& position, SListBase& other,
                      const ConstIterator& other_position)
    {
        splice_after(position, std::move(other), other_position);
    }

    // Splices in element AT other_position
    // into this list AFTER position
    void splice_element_after(const ConstIterator& position, SListBase& other,
                              const ConstIterator& other_position)
    {
        splice_element_after(position, std::move(other), other_position);
    }

    // Splices in the element AFTER other_position
    // into this list AFTER position
    void splice_after(const ConstIterator& position, SListBase&& other,
                      const ConstIterator& other_position)
    {
        insert_after(position, *(std::next(other_position)));
        other.erase_after(other_position);
    }

    // Splices in the element AT other_position
    // into this list AFTER position
    void splice_element_after(const ConstIterator& position, SListBase&& other,
                              const ConstIterator& other_position)
    {
        if (other_position == other.cbefore_begin()) {
            throw KinaraException("Cannot splice from before begin of SListBase");
        }

        insert_after(position, *other_position);
        other.erase(other_position);
    }

    // Splices in the range (first, last)
    // into this list AFTER position
    void splice_after(const ConstIterator& position, SListBase& other,
                      const ConstIterator& first, const ConstIterator& last)
    {
        splice_after(position, std::move(other), first, last);
    }

    // Splices in the range (first, last)
    // into this last AFTER position
    void splice_after(const ConstIterator& position, SListBase&& other,
                      const ConstIterator& first, const ConstIterator& last)
    {
        if (first == other.cbefore_begin() && last == other.end()) {
            splice_after(position, other);
            return;
        }
        insert_after(position, std::next(first), last);
        other.erase_after(first, last);
    }

    // Splices in the range [first, last)
    // into this list AFTER position
    void splice_range_after(const ConstIterator& position, SListBase& other,
                            const ConstIterator& first, const ConstIterator& last)
    {
        splice_range_after(position, std::move(other), first, last);
    }

    // Splices in the range [first, last)
    // into this list AFTER position
    void splice_range_after(const ConstIterator& position, SListBase&& other,
                            const ConstIterator& first, const ConstIterator& last)
    {
        if (first == other.cbefore_begin()) {
            throw KinaraException("Cannot splice from before begin of SListBase");
        }
        if (first == other.cbegin() && last == other.cend()) {
            splice_after(position, other);
            return;
        }
        insert_after(position, first, last);
        other.erase(first, last);
    }

    // Splice functions that splice AT the position rather than after

    // Splices the other list into
    // this list AT position
    void splice(const ConstIterator& position, SListBase& other)
    {
        splice(position, std::move(other));
    }

    // Splices other list
    // into this list AT position
    void splice(const ConstIterator& position, SListBase&& other)
    {
        if (&other == this) {
            throw KinaraException("Cannot splice an SListBase onto itself");
        }

        if (position == cbefore_begin()) {
            throw KinaraException("Cannot splice before beginning of list");
        }
        if (other.size() == 0) {
            return;
        }

        auto osize = other.size();
        auto node_to_splice_after = find_node_before(position.get_node());
        other.m_tail->m_next = node_to_splice_after->m_next;
        node_to_splice_after->m_next = other.m_node_before_head.m_next;
        if (node_to_splice_after == m_tail) {
            m_tail = other.m_tail;
        }
        if (USEPOOLS) {
            m_pool_or_size.m_pool_allocator->merge(other.m_pool_or_size.m_pool_allocator);
        } else {
            add_to_size(osize);
        }
        other.m_node_before_head.m_next = nullptr;
        other.m_tail = nullptr;
        other.reset();
    }

    // Splices in the element AFTER other_position
    // into this list AT position
    void splice(const ConstIterator& position, SListBase& other,
                const ConstIterator& other_position)
    {
        splice(position, std::move(other), other_position);
    }

    // Splices in element AT other_position
    // into this list AT position
    void splice_element(const ConstIterator& position, SListBase& other,
                        const ConstIterator& other_position)
    {
        splice_element(position, std::move(other), other_position);
    }

    // Splices in element AFTER other_position
    // into this list AT position
    void splice(const ConstIterator& position, SListBase&& other,
                const ConstIterator& other_position)
    {
        if (position == cbefore_begin()) {
            throw KinaraException("Cannot splice before beginning of list");
        }
        if (other_position == other.cbefore_begin()) {
            throw KinaraException("Cannot splice from before beginning of list");
        }

        insert(position, *(std::next(other_position)));
        other.erase_after(other_position);
    }

    // Splices in element AT other_position
    // into this list AT position
    void splice_element(const ConstIterator& position, SListBase&& other,
                        const ConstIterator& other_position)
    {
        if (position == cbefore_begin()) {
            throw KinaraException("Cannot splice before beginning of list");
        }
        if (other_position == other.cbefore_begin()) {
            throw KinaraException("Cannot splice from before beginning of list");
        }

        insert(position, *other_position);
        other.erase(other_position);
    }

    // Splices in the range (first, last)
    // into this list AT position
    void splice(const ConstIterator& position, SListBase& other,
                const ConstIterator& first, const ConstIterator& last)
    {
        splice(position, std::move(other), first, last);
    }

    // Splices in the range (first, last)
    // into this list AT position
    void splice(const ConstIterator& position, SListBase&& other,
                const ConstIterator& first, const ConstIterator& last)
    {
        if (position == cbefore_begin()) {
            throw KinaraException("Cannot splice before beginning of list");
        }

        if (first == other.cbefore_begin() && last == other.end()) {
            splice_after(position, other);
            return;
        }
        insert(position, std::next(first), last);
        other.erase_after(first, last);
    }

    // Splices in the range [first, last)
    // into this list AT position
    void splice_range(const ConstIterator& position, SListBase& other,
                      const ConstIterator& first, const ConstIterator& last)
    {
        splice_range(position, std::move(other), first, last);
    }

    // Splices in the range [first, last)
    // into this list AT position
    void splice_range(const ConstIterator& position, SListBase&& other,
                      const ConstIterator& first, const ConstIterator& last)
    {
        if (position == cbefore_begin()) {
            throw KinaraException("Cannot splice before beginning of list");
        }
        if (first == other.cbefore_begin()) {
            throw KinaraException("Cannot splice from before beginning of list");
        }

        if (first == other.cbegin() && last == other.cend()) {
            splice(position, other);
            return;
        }
        insert(position, first, last);
        other.erase(first, last);
    }

    void remove(const ValueType& value)
    {
        auto cur_node = m_node_before_head.m_next;
        auto prev_node = &m_node_before_head;

        while(cur_node != nullptr) {
            auto next_node = cur_node->m_next;
            if (static_cast<NodeType*>(cur_node)->m_value == value) {
                prev_node->m_next = cur_node->m_next;
                if (m_node_before_head.m_next == nullptr) {
                    m_tail = nullptr;
                } else if (cur_node == m_tail) {
                    m_tail = static_cast<NodeType*>(prev_node);
                }
                deallocate_block(static_cast<NodeType*>(cur_node));
                decrement_size();
            } else {
                prev_node = cur_node;
            }
            cur_node = next_node;
        }

        return;
    }

    template <typename Predicate>
    void remove_if(Predicate pred)
    {
        auto cur_node = m_node_before_head.m_next;
        auto prev_node = &m_node_before_head;

        while(cur_node != nullptr) {
            auto next_node = cur_node->m_next;
            if (pred(static_cast<NodeType*>(cur_node)->m_value)) {
                prev_node->m_next = cur_node->m_next;
                if (m_node_before_head.m_next == nullptr) {
                    m_tail = nullptr;
                } else if (cur_node == m_tail) {
                    m_tail = prev_node;
                }
                deallocate_block(cur_node);
                decrement_size();
            } else {
                prev_node = cur_node;
            }
            cur_node = next_node;
        }
        // not found
        return;
    }

    void unique()
    {
        auto first = begin();
        auto last = end();
        if (first == last) {
            return;
        }

        auto next = first;
        while (++next != last) {
            if (*first == *next) {
                erase_after(first);
                next = first;
            } else {
                first = next;
            }
        }
    }

    template <typename BinaryPredicate>
    void unique(BinaryPredicate predicate)
    {
        auto first = begin();
        auto last = end();
        if (first == last) {
            return;
        }

        auto next = first;
        while (++next != last) {
            if (predicate(*first, *next)) {
                erase_after(first);
                next = first;
            } else {
                first = next;
            }
        }
    }

    void merge(SListBase& other)
    {
        merge(std::move(other));
    }

    void merge(SListBase&& other)
    {
        merge(other, std::less<ValueType>());
    }

    template <typename Comparator>
    void merge(SListBase& other, Comparator comparator)
    {
        merge(std::move(other), comparator);
    }

    template <typename Comparator>
    void merge(SListBase&& other, Comparator comparator)
    {
        auto my_node = &m_node_before_head;
        auto& other_before_head = other.m_node_before_head;
        if (size() == 0 && other.size() == 0) {
            return;
        }

        while (my_node->m_next != nullptr && other_before_head.m_next != nullptr) {
            if (comparator(static_cast<NodeType*>(other_before_head.m_next)->m_value,
                           static_cast<NodeType*>(my_node->m_next)->m_value)) {
                auto node_to_insert = other_before_head.m_next;
                other_before_head.m_next = node_to_insert->m_next;
                node_to_insert->m_next = my_node->m_next;
                my_node->m_next = node_to_insert;
            }
            my_node = my_node->m_next;
        }

        // whatever remains in the other list goes to the end of my list
        if (other_before_head.m_next != nullptr) {
            my_node->m_next = other_before_head.m_next;
            other_before_head.m_next = nullptr;
            m_tail = other.m_tail;
        } else {
            m_tail = static_cast<NodeType*>(my_node);
        }
        add_to_size(other.get_size());
        // merge the pools
        if (USEPOOLS) {
            m_pool_or_size.m_pool_allocator->merge(other.m_pool_or_size.m_pool_allocator);
        }
        other.m_node_before_head.m_next = nullptr;
        other.m_tail = nullptr;
        other.reset();
    }

    void sort()
    {
        sort(std::less<ValueType>());
    }

    // an inplace merge sort
    // inspired from libstc++ forward_list::sort
    // credits go there!
    template <typename Comparator>
    void sort(Comparator comparator)
    {
        auto sorted_list = static_cast<NodeType*>(m_node_before_head.m_next);
        if (sorted_list == nullptr) {
            return;
        }

        u64 current_size = 1;
        while (true) {
            auto list_p = sorted_list;
            sorted_list = nullptr;
            NodeType* sorted_list_tail = nullptr;

            u64 num_merges_done = 0;

            while (list_p != nullptr) {
                ++num_merges_done;
                auto list_q = list_p;
                u64 p_size = 0;
                for (u64 i = 0; i < current_size; ++i) {
                    ++p_size;
                    list_q = static_cast<NodeType*>(list_q->m_next);
                    if (list_q == nullptr) {
                        break;
                    }
                }

                u64 q_size = current_size;

                while(p_size > 0 || (q_size > 0 && list_q != nullptr)) {
                    NodeType* node_to_merge = nullptr;
                    if (p_size == 0) {
                        node_to_merge = list_q;
                        list_q = static_cast<NodeType*>(list_q->m_next);
                        --q_size;
                    } else if (q_size == 0 || list_q == nullptr) {
                        node_to_merge = list_p;
                        list_p = static_cast<NodeType*>(list_p->m_next);
                        --p_size;
                    } else if (comparator(list_p->m_value, list_q->m_value)) {
                        node_to_merge = list_p;
                        list_p = static_cast<NodeType*>(list_p->m_next);
                        --p_size;
                    } else {
                        node_to_merge = list_q;
                        list_q = static_cast<NodeType*>(list_q->m_next);
                        --q_size;
                    }

                    if (sorted_list_tail != nullptr) {
                        sorted_list_tail->m_next = node_to_merge;
                    } else {
                        sorted_list = node_to_merge;
                    }
                    sorted_list_tail = node_to_merge;
                }

                list_p = list_q;
            }

            sorted_list_tail->m_next = nullptr;

            if (num_merges_done <= 1) {
                this->m_node_before_head.m_next = sorted_list;
                this->m_tail = sorted_list_tail;
                return;
            }

            current_size *= 2;
        }
    }

    void reverse()
    {
        auto cur_node = m_node_before_head.m_next->m_next;
        if (cur_node == nullptr) {
            return;
        }

        // this will be the new tail
        auto prev_node = m_node_before_head.m_next;
        prev_node->m_next = nullptr;
        auto old_tail = m_tail;
        m_tail = static_cast<NodeType*>(prev_node);

        while (cur_node != nullptr) {
            auto next_node = cur_node->m_next;
            cur_node->m_next = prev_node;
            prev_node = cur_node;
            cur_node = next_node;
        }

        m_node_before_head.m_next = old_tail;
    }

    // Functions/methods not part of stl
    Iterator find(const ValueType& value)
    {
        for (auto it = begin(), last = end(); it != last; ++it) {
            if (*it == value) {
                return it;
            }
        }
        return end();
    }

    ConstIterator find(const ValueType& value) const
    {
        for (auto it = begin(), last = end(); it != last; ++it) {
            if (*it == value) {
                return it;
            }
        }
        return end();
    }
};

// free functions for relational operators
namespace slist_detail_ {

template <typename T, typename CF1, typename DF1, bool UP1,
          typename CF2, typename DF2, bool UP2>
static inline i64 compare(const SListBase<T, CF1, DF1, UP1>& list_a,
                          const SListBase<T, CF2, DF2, UP2>& list_b)
{
    auto diff = list_a.size() - list_b.size();
    if (diff != 0) {
        return diff;
    }
    auto it1 = list_a.begin();
    auto it2 = list_b.begin();

    auto end1 = list_a.end();
    auto end2 = list_b.end();

    std::less<T> less_func;
    while (it1 != end1 && it2 != end2) {
        if (less_func(*it1, *it2)) {
            return -1;
        } else if (less_func(*it2, *it1)) {
            return 1;
        }
        ++it1;
        ++it2;
    }
    return 0;
}

} /* end namespace slist_detail_ */

template <typename T, typename CF1, typename DF1, bool UP1,
          typename CF2, typename DF2, bool UP2>
static inline bool operator == (const SListBase<T, CF1, DF1, UP1>& list_a,
                                const SListBase<T, CF2, DF2, UP2>& list_b)
{
    return (slist_detail_::compare(list_a, list_b) == 0);
}

template <typename T, typename CF1, typename DF1, bool UP1,
          typename CF2, typename DF2, bool UP2>
static inline bool operator != (const SListBase<T, CF1, DF1, UP1>& list_a,
                                const SListBase<T, CF2, DF2, UP2>& list_b)
{
    return (slist_detail_::compare(list_a, list_b) != 0);
}

template <typename T, typename CF1, typename DF1, bool UP1,
          typename CF2, typename DF2, bool UP2>
static inline bool operator < (const SListBase<T, CF1, DF1, UP1>& list_a,
                               const SListBase<T, CF2, DF2, UP2>& list_b)
{
    return (slist_detail_::compare(list_a, list_b) < 0);
}

template <typename T, typename CF1, typename DF1, bool UP1,
          typename CF2, typename DF2, bool UP2>
static inline bool operator <= (const SListBase<T, CF1, DF1, UP1>& list_a,
                                const SListBase<T, CF2, DF2, UP2>& list_b)
{
    return (slist_detail_::compare(list_a, list_b) <= 0);
}

template <typename T, typename CF1, typename DF1, bool UP1,
          typename CF2, typename DF2, bool UP2>
static inline bool operator >= (const SListBase<T, CF1, DF1, UP1>& list_a,
                                const SListBase<T, CF2, DF2, UP2>& list_b)
{
    return (slist_detail_::compare(list_a, list_b) >= 0);
}

template <typename T, typename CF1, typename DF1, bool UP1,
          typename CF2, typename DF2, bool UP2>
static inline bool operator > (const SListBase<T, CF1, DF1, UP1>& list_a,
                               const SListBase<T, CF2, DF2, UP2>& list_b)
{
    return (slist_detail_::compare(list_a, list_b) > 0);
}

// Some useful typedefs
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
