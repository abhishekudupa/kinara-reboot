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

template <typename T,bool USEPOOLS>
class DListBase final
{
public:
    typedef T ValueType;
    typedef T value_type;
    typedef T* PtrType;
    typedef const T* ConstPtrType;
    typedef T& RefType;
    typedef const T& ConstRefType;

    typedef dlist_detail_::Iterator<T> Iterator;
    typedef Iterator iterator;
    typedef dlist_detail_::ConstIterator<T> ConstIterator;
    typedef ConstIterator const_iterator;
    typedef std::reverse_iterator<iterator> ReverseIterator;
    typedef ReverseIterator reverse_iterator;
    typedef std::reverse_iterator<const_iterator> ConstReverseIterator;
    typedef ConstReverseIterator const_reverse_iterator;

private:
    typedef dlist_detail_::DListNodeBase NodeBaseType;
    typedef dlist_detail_::DListNode<T> NodeType;

public:
    static const u64 sc_node_size;

private:
    ka::PoolAllocator* m_pool_allocator;
    u64 m_size;
    NodeBaseType m_root;
    bool m_pool_owned;

    template <typename... ArgTypes>
    inline NodeType* allocate_block(ArgTypes&&... args)
    {
        if (USEPOOLS) {
            if (m_pool_allocator == nullptr) {
                m_pool_allocator = ka::allocate_object_raw<ka::PoolAllocator>(sizeof(NodeType));
            }
            auto ptr = m_pool_allocator->allocate();
            return NodeType::construct(ptr, std::forward<ArgTypes>(args)...);
        } else {
            auto ptr = ka::allocate_raw(sizeof(NodeType));
            return NodeType::construct(ptr, std::forward<ArgTypes>(args)...);
        }
    }

    inline void deallocate_block(NodeType* node)
    {
        if (USEPOOLS) {
            ka::deallocate(*(m_pool_allocator), node);
        } else {
            ka::deallocate_object_raw(node, sizeof(NodeType));
        }
    }

    inline void increment_size()
    {
        m_size++;
    }

    inline void decrement_size()
    {
        m_size--;
    }

    inline void add_to_size(u64 addend)
    {
        m_size += addend;
    }

    inline void sub_from_size(u64 subend)
    {
        m_size -= subend;
    }

    inline u64 get_size() const
    {
        if (m_root.m_next == &m_root) {
            return 0;
        }
        return m_size;
    }

    inline void set_size(u64 new_size)
    {
        m_size = new_size;
    }

    inline Iterator construct_fill(NodeBaseType* position, u64 n, const ValueType& value)
    {
        KINARA_ASSERT(n > 0);

        auto prev_of_position = position->m_prev;
        auto last_inserted = prev_of_position;

        for (u64 i = 0; i < n; ++i) {
            auto new_node = allocate_block(value);
            last_inserted->m_next = new_node;
            new_node->m_prev = last_inserted;
            last_inserted = new_node;
        }

        position->m_prev = last_inserted;
        last_inserted->m_next = position;

        add_to_size(n);
        return Iterator(prev_of_position->m_next);
    }

    template <typename InputIterator>
    inline Iterator construct_range(NodeBaseType* position,
                                    const InputIterator& first,
                                    const InputIterator& last)
    {
        KINARA_ASSERT(first != last);

        auto prev_of_position = position->m_prev;
        auto last_inserted = prev_of_position;

        for (auto it = first; it != last; ++it) {
            auto new_node = allocate_block(*it);
            last_inserted->m_next = new_node;
            new_node->m_prev = last_inserted;
            last_inserted = new_node;
            increment_size();
        }

        position->m_prev = last_inserted;
        last_inserted->m_next = position;

        return Iterator(prev_of_position->m_next);
    }

    template <typename... ArgTypes>
    inline Iterator construct_at(NodeBaseType* position, ArgTypes&&... args)
    {
        auto node = allocate_block(std::forward<ArgTypes>(args)...);

        position->m_prev->m_next = node;
        node->m_prev = position->m_prev;

        node->m_next = position;
        position->m_prev = node;

        increment_size();

        return Iterator(node);
    }

public:
    explicit DListBase()
        : m_pool_allocator(nullptr), m_size(0),
          m_root(&(this->m_root), &(this->m_root)), m_pool_owned(true)
    {
        // Nothing here
    }

    // the pool must not go away as long as the list is alive
    explicit DListBase(ka::PoolAllocator* pool_allocator)
        : DListBase()
    {
        static_assert(USEPOOLS,
                      "Cannot construct non-pooled DList with a "
                      "user provided pool allocator");
        m_pool_allocator = pool_allocator;
        m_pool_owned = false;
    }

    explicit DListBase(u64 n)
        : DListBase(n, ValueType())
    {
        // Nothing here
    }

    inline DListBase(u64 n, ka::PoolAllocator* pool_allocator)
        : DListBase()
    {
        static_assert(USEPOOLS,
                      "Cannot construct non-pooled DList with a "
                      "user provided pool allocator");
        m_pool_allocator = pool_allocator;
        m_pool_owned = false;
        construct_fill(&m_root, n, ValueType());
    }

    DListBase(u64 n, const ValueType& value)
        : DListBase()
    {
        if (n == 0) {
            return;
        }
        construct_fill(&m_root, n, value);
    }

    inline DListBase(u64 n, const ValueType& value, ka::PoolAllocator* pool_allocator)
        : DListBase()
    {
        static_assert(USEPOOLS,
                      "Cannot construct non-pooled DList with a "
                      "user provided pool allocator");
        m_pool_allocator = pool_allocator;
        m_pool_owned = false;
        construct_fill(&m_root, n, value);
    }

    template <typename InputIterator>
    DListBase(const InputIterator& first, const InputIterator& last)
        : DListBase()
    {
        if (first == last) {
            return;
        }
        construct_range(&m_root, first, last);
    }

    template <typename InputIterator>
    DListBase(const InputIterator& first, const InputIterator& last,
              ka::PoolAllocator* pool_allocator)
        : DListBase()
    {
        static_assert(USEPOOLS,
                      "Cannot construct non-pooled DList with a "
                      "user provided pool allocator");
        m_pool_allocator = pool_allocator;
        m_pool_owned = false;
        if (first == last) {
            return;
        }
        construct_range(&m_root, first, last);
    }

    DListBase(const DListBase& other)
        : DListBase(other.begin(), other.end())
    {
        // Nothing here
    }

    DListBase(DListBase&& other)
        : DListBase()
    {
        std::swap(m_pool_allocator, other.m_pool_allocator);
        std::swap(m_size, other.m_size);
        std::swap(m_root, other.m_root);
        std::swap(m_pool_owned, other.m_pool_owned);

        m_root.m_next->m_prev = &m_root;
        m_root.m_prev->m_next = &m_root;

        other.m_root.m_prev->m_next = &(other.m_root);
        other.m_root.m_next->m_prev = &(other.m_root);
    }

    template <bool OUSEPOOLS>
    DListBase(const kc::DListBase<T, OUSEPOOLS>& other)
        : DListBase(std::move(other))
    {
        // Nothing here
    }

    template <bool OUSEPOOLS>
    DListBase(kc::DListBase<T, OUSEPOOLS>&& other)
        : DListBase(other.begin(), other.end())
    {
        // Nothing here
    }

    DListBase(std::initializer_list<ValueType> init_list)
        : DListBase(init_list.begin(), init_list.end())
    {
        // Nothing here
    }

    DListBase(std::initializer_list<ValueType> init_list,
              ka::PoolAllocator* pool_allocator)
        : DListBase(init_list.begin(), init_list.end(), pool_allocator)
    {
        // Nothing here
    }

    inline void reset()
    {
        for (auto node = m_root.m_next; node != &m_root; ) {
            auto next_node = node->m_next;
            deallocate_block(static_cast<NodeType*>(node));
            node = next_node;
        }

        if (USEPOOLS && m_pool_allocator != nullptr) {
            if (m_pool_owned) {
                ka::deallocate_object_raw(m_pool_allocator,
                                          sizeof(ka::PoolAllocator));
                m_pool_allocator = nullptr;
            }
        }
        m_root.m_next = &m_root;
        m_root.m_prev = &m_root;
        set_size(0);
    }

    ~DListBase()
    {
        reset();
    }

    template <typename InputIterator>
    inline void assign(const InputIterator& first, const InputIterator& last)
    {
        reset();
        if (first == last) {
            return;
        }
        construct_range(&m_root, first, last);
    }

    template <bool OUSEPOOLS>
    inline void assign(const kc::DListBase<T, OUSEPOOLS>& other)
    {
        assign(other.begin(), other.end());
    }

    void assign(u64 n, const ValueType& value)
    {
        reset();
        if (n == 0) {
            return;
        }
        construct_fill(&m_root, n, value);
    }

    void assign(std::initializer_list<ValueType> init_list)
    {
        assign(init_list.begin(), init_list.end());
    }

    inline DListBase& operator = (const DListBase& other)
    {
        if (&other == this) {
            return *this;
        }
        assign(other);
        return *this;
    }

    template <bool OUSEPOOLS>
    inline DListBase&
    operator = (const kc::DListBase<T, OUSEPOOLS>& other)
    {
        if (&other == this) {
            return *this;
        }
        assign(other);
        return *this;
    }

    inline DListBase& operator = (DListBase&& other)
    {
        if (&other == this) {
            return *this;
        }

        std::swap(m_pool_allocator, other.m_pool_allocator);
        std::swap(m_size, other.m_size);

        std::swap(m_root, other.m_root);
        std::swap(m_pool_owned, other.m_pool_owned);

        m_root.m_next->m_prev = &m_root;
        m_root.m_prev->m_next = &m_root;

        other.m_root.m_prev->m_next = &(other.m_root);
        other.m_root.m_next->m_prev = &(other.m_root);

        return *this;
    }

    inline DListBase& operator = (std::initializer_list<ValueType> init_list)
    {
        assign(init_list);
        return *this;
    }

    Iterator begin() noexcept
    {
        return Iterator(m_root.m_next);
    }

    ConstIterator begin() const noexcept
    {
        return ConstIterator(m_root.m_next);
    }

    Iterator end() noexcept
    {
        return Iterator(&m_root);
    }

    ConstIterator end() const noexcept
    {
        return ConstIterator(const_cast<NodeBaseType*>(&m_root));
    }

    ConstIterator cbegin() const noexcept
    {
        return begin();
    }

    ConstIterator cend() const noexcept
    {
        return end();
    }

    ReverseIterator rbegin() noexcept
    {
        return ReverseIterator(end());
    }

    ReverseIterator rend() noexcept
    {
        return ReverseIterator(begin());
    }

    ConstReverseIterator rbegin() const noexcept
    {
        return ReverseIterator(begin());
    }

    ConstReverseIterator rend() const noexcept
    {
        return ReverseIterator(end());
    }

    ConstReverseIterator crbegin() const noexcept
    {
        return rbegin();
    }

    ConstReverseIterator crend() const noexcept
    {
        return rend();
    }

    bool empty() const noexcept
    {
        return (m_root.m_next == &m_root);
    }

    u64 size() const noexcept
    {
        return get_size();
    }

    u64 max_size() const noexcept
    {
        return UINT64_MAX;
    }

    RefType front()
    {
        return ((static_cast<NodeType*>(m_root.m_next))->m_value);
    }

    ConstRefType front() const
    {
        return ((static_cast<NodeType*>(m_root.m_next))->m_value);
    }

    RefType back()
    {
        return ((static_cast<NodeType*>(m_root.m_prev))->m_value);
    }

    ConstRefType back() const
    {
        return ((static_cast<NodeType*>(m_root.m_prev))->m_value);
    }

    template <typename... ArgTypes>
    void emplace_front(ArgTypes&&... args)
    {
        construct_at(m_root.m_next, std::forward<ArgTypes>(args)...);
    }

    void push_front(const ValueType& value)
    {
        construct_at(m_root.m_next, value);
    }

    void push_front(ValueType&& value)
    {
        construct_at(m_root.m_next, std::move(value));
    }

    void pop_front()
    {
        auto node_to_delete = m_root.m_next;
        if (node_to_delete == &m_root) {
            return;
        }

        m_root.m_next = m_root.m_next->m_next;
        m_root.m_next->m_prev = &m_root;
        decrement_size();
        deallocate_block(static_cast<NodeType*>(node_to_delete));
        return;
    }

    template <typename... ArgTypes>
    void emplace_back(ArgTypes&&... args)
    {
        construct_at(&m_root, std::forward<ArgTypes>(args)...);
    }

    void push_back(const ValueType& value)
    {
        construct_at(&m_root, value);
    }

    void push_back(ValueType&& value)
    {
        construct_at(&m_root, std::move(value));
    }

    void pop_back()
    {
        auto node_to_delete = m_root.m_prev;
        if (node_to_delete == &m_root) {
            return;
        }
        m_root.m_prev = m_root.m_prev->m_prev;
        m_root.m_prev->m_next = &m_root;
        decrement_size();
        deallocate_block(static_cast<NodeType*>(node_to_delete));
        return;
    }

    template <typename... ArgTypes>
    Iterator emplace(const ConstIterator& position, ArgTypes&&... args)
    {
        return construct_at(position.get_node(), std::forward<ArgTypes>(args)...);
    }

    Iterator insert(const ConstIterator& position, const ValueType& value)
    {
        return construct_at(position.get_node(), value);
    }

    Iterator insert(const ConstIterator& position, u64 n, const ValueType& value)
    {
        if (n == 0) {
            return Iterator(position.get_node());
        }
        return construct_fill(position.get_node(), n, value);
    }

    template <typename InputIterator>
    Iterator insert(const ConstIterator& position,
                    const InputIterator& first,
                    const InputIterator& last)
    {
        if (first == last) {
            return Iterator(position.get_node());
        }
        return construct_range(position.get_node(), first, last);
    }

    Iterator insert(const ConstIterator& position, ValueType&& value)
    {
        return construct_at(position.get_node(), std::move(value));
    }

    Iterator insert(const ConstIterator& position,
                    std::initializer_list<ValueType> init_list)
    {
        if (init_list.size() == 0) {
            return Iterator(position.get_node());
        }
        return construct_range(position.get_node(), init_list.begin(), init_list.end());
    }

    Iterator erase(const ConstIterator& position)
    {
        if (empty()) {
            return begin();
        }
        auto pos_node = position.get_node();
        pos_node->m_prev->m_next = pos_node->m_next;
        pos_node->m_next->m_prev = pos_node->m_prev;
        auto retval = pos_node->m_next;
        deallocate_block(static_cast<NodeType*>(pos_node));
        decrement_size();
        return Iterator(retval);
    }

    Iterator erase(const ConstIterator& first, const ConstIterator& last)
    {
        if (empty()) {
            return begin();
        }
        auto first_node = first.get_node();
        auto before_first = first_node->m_prev;
        auto last_node = last.get_node();

        for (auto node = first_node; node != last_node; ) {
            auto next_node = node->m_next;
            deallocate_block(static_cast<NodeType*>(node));
            decrement_size();
            node = next_node;
        }
        before_first->m_next = last_node;
        last_node->m_prev = before_first;
        return Iterator(last_node);
    }

    void swap(DListBase& other)
    {
        auto const other_empty = other.empty();
        auto const this_empty = empty();

        std::swap(m_pool_allocator, other.m_pool_allocator);
        std::swap(m_size, other.m_size);
        std::swap(m_pool_owned, other.m_pool_owned);
        std::swap(m_root, other.m_root);
        std::swap(m_root, other.m_root);
    }

    void resize(u64 n)
    {
        resize(n, ValueType());
    }

    void resize(u64 n, const ValueType& value)
    {
        auto orig_size = get_size();
        if (n == orig_size) {
            return;
        }
        if (n == 0) {
            reset();
            return;
        }
        if (n < orig_size) {
            auto it = begin();
            for (u64 i = 0; i < n; ++i) {
                ++it;
            }
            erase(it, end());
        } else {
            auto num_to_add = n - orig_size;
            construct_fill(&m_root, num_to_add, value);
        }
    }

    void clear()
    {
        reset();
    }

    void splice(const ConstIterator& position, DListBase& other)
    {
        splice(position, std::move(other));
    }

    void splice(const ConstIterator& position, DListBase&& other)
    {
        if (other.empty()) {
            return;
        }

        auto position_node = position.get_node();
        auto before_position_node = position_node->m_prev;
        auto last_inserted = before_position_node;

        for (auto node = other.m_root.m_next; node != &(other.m_root); ) {

            last_inserted->m_next = node;
            node->m_prev = last_inserted;

            last_inserted = node;
            node = node->m_next;
        }

        last_inserted->m_next = position_node;
        position_node->m_prev = last_inserted;

        add_to_size(other.size());

        if (USEPOOLS &&
            m_pool_allocator != other.m_pool_allocator) {
            m_pool_allocator->merge(other.m_pool_allocator);
        }

        other.m_root.m_next = &(other.m_root);
        other.m_root.m_prev = &(other.m_root);
        other.reset();
    }

    void splice(const ConstIterator& position, DListBase& other,
                const ConstIterator& element)
    {
        splice(position, std::move(other), element);
    }

    void splice(const ConstIterator& position, DListBase&& other,
                const ConstIterator& element)
    {
        insert(position, *element);
        other.erase(element);
    }

    void splice(const ConstIterator& position, DListBase& other,
                const ConstIterator& first, const ConstIterator& last)
    {
        splice(position, std::move(other), first, last);
    }

    void splice(const ConstIterator& position, DListBase&& other,
                const ConstIterator& first, const ConstIterator& last)
    {
        if (first == last || other.empty()) {
            return;
        }
        if (first == other.begin() && last == other.end()) {
            splice(position, std::move(other));
            return;
        }
        insert(position, first, last);
        other.erase(first, last);
    }

    void remove(const ValueType& value)
    {
        auto cur_node = m_root.m_next;
        auto prev_node = &m_root;

        while (cur_node != &m_root) {
            auto next_node = cur_node->m_next;
            if (static_cast<NodeType*>(cur_node)->m_value == value) {
                prev_node->m_next = next_node;
                next_node->m_prev = prev_node;
                deallocate_block(static_cast<NodeType*>(cur_node));
                decrement_size();
            } else {
                prev_node = cur_node;
            }
            cur_node = next_node;
        }
        return;
    }

    template <typename UnaryPredicate>
    void remove_if(UnaryPredicate predicate)
    {
        auto cur_node = m_root.m_next;
        auto prev_node = &m_root;

        while (cur_node != &m_root) {
            auto next_node = cur_node->m_next;
            if (predicate(static_cast<NodeType*>(cur_node)->m_value)) {
                prev_node->m_next = next_node;
                next_node->m_prev = prev_node;
                deallocate_block(static_cast<NodeType*>(cur_node));
                decrement_size();
            } else {
                prev_node = cur_node;
            }
            cur_node = next_node;
        }
        return;
    }

    void unique()
    {
        std::equal_to<ValueType> equals_func;
        unique(equals_func);
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
                erase(next);
                next = first;
            } else {
                first = next;
            }
        }
    }

    void merge(DListBase& other)
    {
        merge(std::move(other));
    }

    void merge(DListBase&& other)
    {
        std::less<ValueType> less_func;
        merge(std::move(other), less_func);
    }

    template <typename Comparator>
    void merge(DListBase& other, Comparator comparator)
    {
        merge(std::move(other), comparator);
    }

    template <typename Comparator>
    void merge(DListBase&& other, Comparator comparator)
    {
        if (other.empty()) {
            return;
        }
        if (empty()) {
            (*this) = std::move(other);
        }

        auto other_size = other.size();
        auto cur_node = m_root.m_next;
        auto node_to_merge = other.m_root.m_next;

        while (cur_node != &m_root && node_to_merge != &(other.m_root)) {
            auto next_of_node_to_merge = node_to_merge->m_next;
            auto next_of_cur_node = cur_node->m_next;

            if (comparator(static_cast<NodeType*>(node_to_merge)->m_value,
                           static_cast<NodeType*>(cur_node)->m_value)) {

                cur_node->m_prev->m_next = node_to_merge;
                node_to_merge->m_prev = cur_node->m_prev;
                cur_node->m_prev = node_to_merge;
                node_to_merge->m_next = cur_node;
                node_to_merge = next_of_node_to_merge;
            } else {
                cur_node = next_of_cur_node;
            }
        }

        // whatever remains in the other list goes to the end of this
        if (node_to_merge != &(other.m_root)) {
            m_root.m_prev->m_next = node_to_merge;
            node_to_merge->m_prev = m_root.m_prev;

            m_root.m_prev = other.m_root.m_prev;
            other.m_root.m_prev->m_next = &m_root;
        }

        add_to_size(other_size);

        // merge the pools
        if (USEPOOLS &&
            m_pool_allocator != other.m_pool_allocator) {
            m_pool_allocator->merge(other.m_pool_allocator);
        }

        other.m_root.m_next = &(other.m_root);
        other.m_root.m_prev = &(other.m_root);
        other.reset();
    }

    void sort()
    {
        std::less<ValueType> less_func;
        sort(less_func);
    }

    // an in place merge sort
    template <typename Comparator>
    void sort(Comparator comparator)
    {
        auto my_size = get_size();
        if (my_size <= 1) {
            return;
        }

        NodeBaseType sorted_list;

        // move the list to be rooted at sorted_list
        sorted_list.m_next = m_root.m_next;
        sorted_list.m_prev = m_root.m_prev;
        m_root.m_next->m_prev = &sorted_list;
        m_root.m_prev->m_next = &sorted_list;

        u64 current_size = 1;
        while (true) {
            auto list_p = sorted_list.m_next;

            sorted_list.m_next = &sorted_list;
            sorted_list.m_prev = &sorted_list;

            u64 num_merges_done = 0;
            while (list_p != &sorted_list) {
                ++num_merges_done;
                auto list_q = list_p;
                u64 p_size = 0;
                for (u64 i = 0; i < current_size; ++i) {
                    ++p_size;
                    list_q = list_q->m_next;
                    if (list_q == &sorted_list) {
                        break;
                    }
                }

                u64 q_size = current_size;
                while (p_size > 0 || (q_size > 0 && list_q != &sorted_list)) {
                    NodeType* node_to_merge = nullptr;
                    if (p_size == 0) {
                        node_to_merge = static_cast<NodeType*>(list_q);
                        list_q = list_q->m_next;
                        --q_size;
                    } else if (q_size == 0 || list_q == &sorted_list) {
                        node_to_merge = static_cast<NodeType*>(list_p);
                        list_p = list_p->m_next;
                        --p_size;
                    } else if (comparator(static_cast<NodeType*>(list_p)->m_value,
                                          static_cast<NodeType*>(list_q)->m_value)) {
                        node_to_merge = static_cast<NodeType*>(list_p);
                        list_p = list_p->m_next;
                        --p_size;
                    } else {
                        node_to_merge = static_cast<NodeType*>(list_q);
                        list_q = list_q->m_next;
                        --q_size;
                    }

                    if (sorted_list.m_next == &sorted_list) {
                        // empty sorted list
                        sorted_list.m_next = node_to_merge;
                        sorted_list.m_prev = node_to_merge;
                        node_to_merge->m_next = &sorted_list;
                        node_to_merge->m_prev = &sorted_list;
                    } else {
                        // non-empty sorted list, add to end
                        sorted_list.m_prev->m_next = node_to_merge;
                        node_to_merge->m_prev = sorted_list.m_prev;
                        node_to_merge->m_next = &sorted_list;
                        sorted_list.m_prev = node_to_merge;
                    }
                }

                list_p = list_q;
            }

            if (num_merges_done <= 1) {
                m_root.m_next = sorted_list.m_next;
                m_root.m_prev = sorted_list.m_prev;
                m_root.m_next->m_prev = &m_root;
                m_root.m_prev->m_next = &m_root;
                return;
            }

            current_size *= 2;
        }
    }

    void reverse() noexcept
    {
        auto my_size = get_size();
        if (my_size <= 1) {
            return;
        }

        auto cur_node = m_root.m_next;

        while (cur_node != &m_root) {
            auto next_node = cur_node->m_next;
            std::swap(cur_node->m_next, cur_node->m_prev);
            cur_node = next_node;
        }

        std::swap(m_root.m_next, m_root.m_prev);
    }

    // functions not part of stl
    inline void garbage_collect()
    {
        if (!USEPOOLS) {
            return;
        }
        m_pool_allocator->garbage_collect();
    }

    // Use with caution. Mucking about with
    // the pool can cause list corruption or worse!
    inline ka::PoolAllocator* get_pool() const
    {
        if (!USEPOOLS) {
            return nullptr;
        }
        return m_pool_allocator;
    }

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
        for (auto it = cbegin(), last = cend(); it != last; ++it) {
            if (*it == value) {
                return it;
            }
        }
        return cend();
    }

    template <typename UnaryPredicate>
    Iterator find(UnaryPredicate predicate)
    {
        for (auto it = begin(), last = end(); it != last; ++it) {
            if (predicate(*it)) {
                return it;
            }
        }
        return end();
    }

    template <typename UnaryPredicate>
    ConstIterator find(UnaryPredicate predicate) const
    {
        for (auto it = begin(), last = end(); it != last; ++it) {
            if (predicate(*it)) {
                return it;
            }
        }
        return end();
    }

    template <bool OUSEPOOLS>
    inline i64 compare(const DListBase<T, OUSEPOOLS>& other) const
    {
        return compare(other, std::less<T>());
    }

    template <bool OUSEPOOLS, typename BinaryPredicate>
    inline i64 compare(const DListBase<T, OUSEPOOLS>& other,
                       BinaryPredicate predicate) const
    {
        auto diff = size() - other.size();
        if (diff != 0) {
            return diff;
        }

        auto it1 = begin();
        auto it2 = other.begin();

        auto end1 = end();
        auto end2 = other.end();

        while (it1 != end1 && it2 != end2) {
            if (predicate(*it1, *it2)) {
                return -1;
            } else if (predicate(*it2, *it1)) {
                return 1;
            }

            ++it1;
            ++it2;
        }

        return 0;
    }
};

template <typename T, bool USEPOOLS>
const u64 DListBase<T, USEPOOLS>::sc_node_size = sizeof(DListBase<T, USEPOOLS>::NodeType);

// relational operators for dlist
template <typename T, bool UP1, bool UP2>
static inline bool operator == (const DListBase<T, UP1>& list1,
                                const DListBase<T, UP2>& list2)
{
    return (list1.compare(list2) == 0);
}

template <typename T, bool UP1, bool UP2>
static inline bool operator != (const DListBase<T, UP1>& list1,
                                const DListBase<T, UP2>& list2)
{
    return (list1.compare(list2) != 0);
}

template <typename T, bool UP1, bool UP2>
static inline bool operator < (const DListBase<T, UP1>& list1,
                               const DListBase<T, UP2>& list2)
{
    return (list1.compare(list2) < 0);
}

template <typename T, bool UP1, bool UP2>
static inline bool operator > (const DListBase<T, UP1>& list1,
                               const DListBase<T, UP2>& list2)
{
    return (list1.compare(list2) > 0);
}

template <typename T, bool UP1, bool UP2>
static inline bool operator <= (const DListBase<T, UP1>& list1,
                                const DListBase<T, UP2>& list2)
{
    return (list1.compare(list2) <= 0);
}

template <typename T, bool UP1, bool UP2>
static inline bool operator >= (const DListBase<T, UP1>& list1,
                                const DListBase<T, UP2>& list2)
{
    return (list1.compare(list2) >= 0);
}

// Some useful typedefs
template <typename T>
using PoolDList = DListBase<T, true>;

template <typename T>
using DList = DListBase<T, false>;

template <typename T>
using PoolPtrDList = DListBase<T*, true>;

template <typename T>
using PtrDList = DListBase<T*, false>;

template <typename T>
using ConstPtrDList = DListBase<const T*, false>;

template <typename T>
using PoolConstPtrDList = DListBase<const T*, true>;

template <typename T>
using MPtrDList =
    DListBase<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                        memory::ManagedPointer<T>, T*>::type, false>;

template <typename T>
using PoolMPtrDList =
    DListBase<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                        memory::ManagedPointer<T>, T*>::type, true>;

template <typename T>
using ConstMPtrDList =
    DListBase<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                        memory::ManagedConstPointer<T>,
                                        const T*>::type, false>;

template <typename T>
using PoolConstMPtrDList =
    DListBase<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                        memory::ManagedConstPointer<T>,
                                        const T*>::type, true>;

typedef PoolDList<u08> u08PoolDList;
typedef PoolDList<u16> u16PoolDList;
typedef PoolDList<u32> u32PoolDList;
typedef PoolDList<u64> u64PoolDList;
typedef PoolDList<i08> i08PoolDList;
typedef PoolDList<i16> i16PoolDList;
typedef PoolDList<i32> i32PoolDList;
typedef PoolDList<i64> i64PoolDList;

typedef DList<u08> u08DList;
typedef DList<u16> u16DList;
typedef DList<u32> u32DList;
typedef DList<u64> u64DList;
typedef DList<i08> i08DList;
typedef DList<i16> i16DList;
typedef DList<i32> i32DList;
typedef DList<i64> i64DList;

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_DLIST_HPP_ */

//
// DList.hpp ends here
