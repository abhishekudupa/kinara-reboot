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
#include <algorithm>

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

    // iterators
    typedef deque_detail_::IteratorBase<T, ConstructFunc, DestructFunc, true> Iterator;
    typedef Iterator iterator;
    typedef deque_detail_::IteratorBase<T, ConstructFunc, DestructFunc, false> ConstIterator;
    typedef ConstIterator const_iterator;
    typedef std::reverse_iterator<iterator> ReverseIterator;
    typedef ReverseIterator reverse_iterator;
    typedef std::reverse_iterator<const_iterator> ConstReverseIterator;
    typedef ConstReverseIterator const_reverse_iterator;

private:
    inline void call_destructors()
    {
        DestructFunc the_destructor;
        for (auto it = this->m_start; it != this->m_finish; ++it) {
            the_destructor(*it);
        }
    }

    inline void destroy_range(Iterator first, Iterator last)
    {
        DestructFunc the_destructor;
        for (auto it = first; it != last; ++it) {
            the_destructor(*it);
        }
    }

    // assumes that the object isn't constructed yet
    template <typename InputIterator>
    inline void construct_range(const InputIterator& first,
                                const InputIterator& last,
                                std::input_iterator_tag unused)
    {
        this->initialize(0);
        for (auto it = first; it != last; ++it) {
            push_back(*it);
        }
    }

    template <typename ForwardIterator>
    inline void construct_range(const ForwardIterator& first,
                                const ForwardIterator& last,
                                std::forward_iterator_tag unused)
    {
        this->initialize(std::distance(first, last));
        u64 i = 0;
        ConstructFunc the_construct_func;
        for (auto it = first; it != last; ++it, ++i) {
            the_construct_func(&((*this)[i]), *it);
        }
    }

    template <typename InputIterator>
    inline void assign_range(const InputIterator& first,
                             const InputIterator& last,
                             std::input_iterator_tag unused)
    {
        call_destructors();
        this->reset();

        for (auto it = first; it != last; ++it) {
            this->push_back(*it);
        }
    }

    template <typename ForwardIterator>
    inline void assign_range(const ForwardIterator& first,
                             const ForwardIterator& last,
                             std::forward_iterator_tag unused)
    {
        auto const num_elems = std::distance(first, last);
        resize(num_elems);

        u64 i = 0;
        for (auto it = first; it != last; ++it, ++i) {
            (*this)[i] = *it;
        }
    }

    template <typename InputIterator>
    inline Iterator insert_range(const ConstIterator& position,
                                 const InputIterator& first,
                                 const InputIterator& last,
                                 std::input_iterator_tag unused)
    {
        Iterator actual_position = position;
        u64 num_elems_added = 0;
        for (auto it = first; it != last; ++it) {
            actual_position = this->insert(actual_position, it);
            ++actual_position;
            ++num_elems_added;
        }
        return (actual_position - num_elems_added);
    }

    template <typename ForwardIterator>
    inline Iterator insert_range(const ConstIterator& position,
                                 const ForwardIterator& first,
                                 const ForwardIterator& last,
                                 std::forward_iterator_tag unused)
    {
        auto num_elems = std::distance(first, last);
        auto actual_position = this->make_hole_at_position(position);
        ConstructFunc the_constructor;
        for (auto it = first; it != last; ++it) {
            the_constructor(&(*actual_position), *it);
            ++actual_position;
        }
        return (actual_position - num_elems);
    }

public:
    inline void assign(u64 n, const ValueType& value)
    {
        resize(n);
        for (auto it = this->m_start; it != this->m_finish; ++it) {
            *it = value;
        }
    }

    template <typename InputIterator>
    inline void assign(InputIterator first, InputIterator last)
    {
        typedef typename std::iterator_traits<InputIterator>::iterator_category IterCategory;
        assign_range(first, last, IterCategory());
    }

    inline void assign(std::initializer_list<ValueType> init_list)
    {
        assign(init_list.begin(), init_list.end());
    }

    explicit DequeBase()
        : BaseType()
    {
        // Nothing here
    }

    explicit DequeBase(u64 n)
        : DequeBase(n, ValueType())
    {
    }

    explicit DequeBase(u64 n, const ValueType& value)
        : BaseType(n)
    {
        for (u64 i = 0; i < n; ++i) {
            (*this)[i] = value;
        }
    }

    template <typename InputIterator>
    DequeBase(InputIterator first, InputIterator last)
        : BaseType(false)
    {
        typedef typename std::iterator_traits<InputIterator>::iterator_category IterCategory;
        construct_range(first, last, IterCategory());
    }

    DequeBase(const DequeBase& other)
        : DequeBase(other.begin(), other.end())
    {
        // Nothing here
    }

    DequeBase(DequeBase&& other)
        : BaseType(std::move(other))
    {
        // Nothing here
    }

    DequeBase(std::initializer_list<ValueType> init_list)
        : DequeBase(init_list.begin(), init_list.end())
    {
        // Nothing here
    }

    ~DequeBase()
    {
        call_destructors();
    }

    inline DequeBase& operator = (const DequeBase& other)
    {
        if (&other == this) {
            return *this;
        }
        assign(other.begin(), other.end());
        return *this;
    }

    inline DequeBase& operator = (DequeBase&& other)
    {
        if (&other == this) {
            return *this;
        }
        BaseType::assign(std::move(other));
        return *this;
    }

    inline DequeBase& operator = (std::initializer_list<ValueType> init_list)
    {
        assign(init_list.begin(), init_list.end());
        return *this;
    }

    inline Iterator begin() noexcept
    {
        return this->m_start;
    }

    inline Iterator end() noexcept
    {
        return this->m_finish;
    }

    inline ConstIterator begin() const noexcept
    {
        return (ConstIterator(this->m_start));
    }

    inline ConstIterator end() const noexcept
    {
        return (ConstIterator(this->m_finish));
    }

    inline ConstIterator cbegin() const noexcept
    {
        return begin();
    }

    inline ConstIterator cend() const noexcept
    {
        return end();
    }

    inline ReverseIterator rbegin() noexcept
    {
        return ReverseIterator(end());
    }

    inline ReverseIterator rend() noexcept
    {
        return ReverseIterator(begin());
    }

    inline ConstReverseIterator rbegin() const noexcept
    {
        return ReverseIterator(begin());
    }

    inline ConstReverseIterator rend() const noexcept
    {
        return ReverseIterator(end());
    }

    inline ConstReverseIterator crbegin() const noexcept
    {
        return rbegin();
    }

    inline ConstReverseIterator crend() const noexcept
    {
        return rend();
    }

    inline u64 size() const noexcept
    {
        return this->get_size();
    }

    inline u64 max_size() const noexcept
    {
        return UINT64_MAX;
    }

    inline void resize(u64 new_size, const ValueType& value)
    {
        auto const orig_size = size();

        if (new_size == orig_size) {
            return;
        }

        if (new_size < orig_size) {
            auto position = cbegin() + (new_size - 1);
            destroy_range(position + 1, cend());
            this->shrink_after(position);
        } else {
            auto num_elems_to_add = new_size - orig_size;
            this->expand_towards_back(num_elems_to_add);
            // construct
            ConstructFunc the_constructor;
            for (u64 i = 0; i < num_elems_to_add; ++i) {
                the_constructor(&((*this)[orig_size + i]), value);
            }
            this->m_finish += num_elems_to_add;
        }
    }

    inline void resize(u64 new_size)
    {
        resize(new_size, ValueType());
    }

    inline bool empty() const noexcept
    {
        return (this->m_start == this->m_finish);
    }

    inline void shrink_to_fit()
    {
        shrink_after(end(), true);
    }

    inline RefType operator [] (u64 index)
    {
        return ((this->m_start)[index]);
    }

    inline ConstRefType operator [] (u64 index) const
    {
        return ((this->m_start)[index]);
    }

    inline RefType at(u64 index)
    {
        return ((this->m_start)[index]);
    }

    inline ConstRefType at(u64 index) const
    {
        return ((this->m_start)[index]);
    }

    inline RefType front()
    {
        return (*(this->m_start));
    }

    inline ConstRefType front() const
    {
        return (*(this->m_start));
    }

    inline RefType back()
    {
        return (*(this->m_finish - 1));
    }

    inline ConstRefType back() const
    {
        return (*(this->m_finish - 1));
    }

    void push_back(const ValueType& value)
    {
        this->expand_towards_back();
        ConstructFunc the_constructor;
        the_constructor(&(*(this->m_finish)), value);
        this->m_finish++;
    }

    void push_back(ValueType&& value)
    {
        this->expand_towards_back();
        ConstructFunc the_constructor;
        the_constructor(&(*(this->m_finish)), std::move(value));
        this->m_finish++;
    }

    void push_front(const ValueType& value)
    {
        this->expand_towards_front();
        ConstructFunc the_constructor;
        --(this->m_start);
        the_constructor(&(*(this->m_start)), value);
    }

    void push_front(ValueType&& value)
    {
        this->expand_towards_front();
        ConstructFunc the_constructor;
        --(this->m_start);
        the_constructor(&(*(this->m_start)), std::move(value));
    }

    void pop_front()
    {
        DestructFunc the_destructor;
        the_destructor(*(this->m_start));
        ++(this->m_start);
        this->compact();
    }

    void pop_back()
    {
        DestructFunc the_destructor;
        --(this->m_finish);
        the_destructor(*(this->m_finish));
        this->compact();
    }

    Iterator insert(const ConstIterator& position, const ValueType& value)
    {
        auto hole_position = this->make_hole_at_position(position, 1);
        ConstructFunc the_constructor;
        the_constructor(&(*hole_position), value);
        return hole_position;
    }

    Iterator insert(const ConstIterator& position, u64 n, const ValueType& value)
    {
        auto hole_position = this->make_hole_at_position(position, n);
        ConstructFunc the_constructor;
        for (u64 i = 0; i < n; ++i) {
            the_constructor(&(*hole_position), value);
            ++hole_position;
        }
        return (hole_position - n);
    }

    template <typename InputIterator>
    Iterator insert(const ConstIterator& position, const InputIterator& first,
                    const InputIterator& last)
    {
        typedef typename std::iterator_traits<InputIterator>::iterator_category IterCategory;
        return insert_range(position, first, last, IterCategory());
    }

    Iterator insert(const ConstIterator& position, ValueType&& value)
    {
        return insert(position, std::move(value));
    }

    Iterator insert(const ConstIterator& position,
                    const std::initializer_list<ValueType>& init_list)
    {
        return insert(position, init_list.begin(), init_list.end());
    }

    Iterator erase(const ConstIterator& position)
    {
        DestructFunc the_destructor;
        the_destructor(*position);
        auto offset_from_start = position - this->m_start;
        auto offset_from_finish = this->m_finish - position;
        if (offset_from_start < offset_from_finish) {
            this->move_objects(this->m_start + 1, this->m_start, offset_from_start);
            ++(this->m_start);
        } else {
            this->move_objects(this->m_finish - 1, this->m_finish, offset_from_finish - 1);
            --(this->m_finish);
        }
        return (this->m_start + offset_from_start);
    }

    Iterator erase(const ConstIterator& first, const ConstIterator& last)
    {
        if (first == last) {
            return;
        }
        destroy_range(first, last);
        auto num_elems = last - first;
        auto offset_from_start = first - this->m_start;
        auto offset_from_finish = last - this->m_finish;
        if (offset_from_start < offset_from_finish) {
            this->move_objects(this->m_start + num_elems, this->m_start, offset_from_start);
            this->m_start += num_elems;
        } else {
            this->move_objects(this->m_finish - num_elems, this->m_finish,
                               offset_from_finish - num_elems);
            this->m_finish -= num_elems;
        }
        return (this->m_start + offset_from_start);
    }

    void swap(DequeBase& other)
    {
        BaseType::swap(other);
    }

    void clear()
    {
        this->reset();
    }

    template <typename... ArgTypes>
    inline void emplace(const ConstIterator& position, ArgTypes&&... args)
    {
        auto hole_position = this->make_hole_at_position(position, 1);
        ConstructFunc the_constructor;
        the_constructor(&(*hole_position), std::forward<ArgTypes>(args)...);
        return hole_position;
    }

    template <typename... ArgTypes>
    inline void emplace_front(ArgTypes&&... args)
    {
        this->expand_towards_front();
        ConstructFunc the_constructor;
        --(this->m_start);
        the_constructor(&(*(this->m_start)), std::forward<ArgTypes>(args)...);
    }

    template <typename... ArgTypes>
    inline void emplace_back(ArgTypes&&... args)
    {
        this->expand_towards_back();
        ConstructFunc the_constructor;
        the_constructor(&(*(this->m_finish)), std::forward<ArgTypes>(args)...);
        ++(this->m_finish);
    }

    // functions not part of standard stl
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

    void sort()
    {
        sort(std::less<T>());
    }

    template <typename Comparator>
    void sort(Comparator comparator)
    {
        std::sort(begin(), end(), comparator);
    }

    void reverse()
    {
        std::reverse(begin(), end());
    }
};

namespace deque_detail_ {

// compare function for deques
template <typename T, typename CF1, typename DF1,
          typename CF2, typename DF2>
inline i32 compare(const DequeBase<T, CF1, DF1>& deque1,
                   const DequeBase<T, CF2, DF2>& deque2)
{
    auto diff = deque1.size() - deque2.size();
    if (diff != 0) {
        return diff;
    }
    auto it1 = deque1.begin();
    auto it2 = deque2.begin();
    auto end1 = deque1.end();
    auto end2 = deque2.end();
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

} /* end namespace deque_detail_ */

template <typename T, typename CF1, typename DF1,
          typename CF2, typename DF2>
static inline bool operator == (const DequeBase<T, CF1, DF1>& deque1,
                                const DequeBase<T, CF2, DF2>& deque2)
{
    return (compare(deque1, deque2) == 0);
}

template <typename T, typename CF1, typename DF1,
          typename CF2, typename DF2>
static inline bool operator != (const DequeBase<T, CF1, DF1>& deque1,
                                const DequeBase<T, CF2, DF2>& deque2)
{
    return (compare(deque1, deque2) != 0);
}

template <typename T, typename CF1, typename DF1,
          typename CF2, typename DF2>
static inline bool operator < (const DequeBase<T, CF1, DF1>& deque1,
                               const DequeBase<T, CF2, DF2>& deque2)
{
    return (compare(deque1, deque2) < 0);
}

template <typename T, typename CF1, typename DF1,
          typename CF2, typename DF2>
static inline bool operator > (const DequeBase<T, CF1, DF1>& deque1,
                               const DequeBase<T, CF2, DF2>& deque2)
{
    return (compare(deque1, deque2) > 0);
}

template <typename T, typename CF1, typename DF1,
          typename CF2, typename DF2>
static inline bool operator <= (const DequeBase<T, CF1, DF1>& deque1,
                                const DequeBase<T, CF2, DF2>& deque2)
{
    return (compare(deque1, deque2) <= 0);
}

template <typename T, typename CF1, typename DF1,
          typename CF2, typename DF2>
static inline bool operator >= (const DequeBase<T, CF1, DF1>& deque1,
                                const DequeBase<T, CF2, DF2>& deque2)
{
    return (compare(deque1, deque2) >= 0);
}

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
