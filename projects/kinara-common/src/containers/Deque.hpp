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

template <typename T>
class DequeBase final :
        protected deque_detail_::DequeInternal<T>
{
public:
    typedef T ValueType;
    typedef T value_type;
    typedef T* PtrType;
    typedef T& RefType;
    typedef const T* ConstPtrType;
    typedef const T& ConstRefType;
    typedef deque_detail_::DequeInternal<T> BaseType;

    // iterators
    typedef deque_detail_::IteratorBase<T, false> Iterator;
    typedef Iterator iterator;
    typedef deque_detail_::IteratorBase<T, true> ConstIterator;
    typedef ConstIterator const_iterator;
    typedef std::reverse_iterator<iterator> ReverseIterator;
    typedef ReverseIterator reverse_iterator;
    typedef std::reverse_iterator<const_iterator> ConstReverseIterator;
    typedef ConstReverseIterator const_reverse_iterator;

private:
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
        std::copy(first, last, begin());
    }

    template <typename InputIterator>
    inline void assign_range(const InputIterator& first,
                             const InputIterator& last,
                             std::input_iterator_tag unused)
    {
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
        auto actual_position = this->make_hole_at_position(position, num_elems);
        std::copy(first, last, actual_position);
        return actual_position;
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
        // Nothing here
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
        // Nothing here
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
            auto position = begin() + (new_size - 1);
            this->shrink_after(position);
        } else {
            auto num_elems_to_add = new_size - orig_size;
            this->expand_towards_back(num_elems_to_add);
            std::fill_n(begin() + orig_size, new_size - orig_size, value);
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
        *(this->m_finish - 1) = value;
    }

    void push_back(ValueType&& value)
    {
        this->expand_towards_back();
        *(this->m_finish - 1) = std::move(value);
    }

    void push_front(const ValueType& value)
    {
        this->expand_towards_front();
        *(this->m_start) = value;
    }

    void push_front(ValueType&& value)
    {
        this->expand_towards_front();
        *(this->m_start) = std::move(value);
    }

    void pop_front()
    {
        ++(this->m_start);
        this->compact();
    }

    void pop_back()
    {
        --(this->m_finish);
        this->compact();
    }

    Iterator insert(const ConstIterator& position, const ValueType& value)
    {
        auto hole_position = this->make_hole_at_position(position, 1);
        *hole_position = value;
        return hole_position;
    }

    Iterator insert(const ConstIterator& position, u64 n, const ValueType& value)
    {
        auto hole_position = this->make_hole_at_position(position, n);
        std::fill_n(hole_position, n, value);
        return hole_position;
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
        if (position == this->m_finish) {
            return Iterator(position.m_current, position.m_block_array_ptr);
        }
        return erase(position, position + 1);
    }

    Iterator erase(const ConstIterator& first, const ConstIterator& last)
    {
        if (first == last) {
            return Iterator(first.m_current, first.m_block_array_ptr);
        }

        auto num_elems = last - first;
        auto orig_size = size();

        KINARA_ASSERT(num_elems > 0);

        if ((u64)num_elems == orig_size) {
            clear();
            return this->m_finish;
        }

        auto elems_before_first = first - this->m_start;
        auto elems_after_last = this->m_finish - last;

        if (elems_after_last == 0) {
            this->shrink_after(first - 1);
            return this->m_finish;
        }

        if (elems_before_first < elems_after_last) {
            this->move_objects(this->m_start + num_elems, this->m_start, elems_before_first);
            this->m_start += num_elems;
        } else {
            this->move_objects(this->m_start + elems_before_first,
                               last, elems_after_last);

            this->m_finish -= num_elems;
        }
        return (Iterator(last.m_current, last.m_block_array_ptr));
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
        new (&(*hole_position)) T(std::forward<ArgTypes>(args)...);
        return hole_position;
    }

    template <typename... ArgTypes>
    inline void emplace_front(ArgTypes&&... args)
    {
        this->expand_towards_front();
        new (&(*(this->m_start))) T(std::forward<ArgTypes>(args)...);
    }

    template <typename... ArgTypes>
    inline void emplace_back(ArgTypes&&... args)
    {
        this->expand_towards_back();
        new (&(*(this->m_finish - 1))) T(std::forward<ArgTypes>(args)...);
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
template <typename T, typename BinaryPredicate>
inline i32 compare(const DequeBase<T>& deque1,
                   const DequeBase<T>& deque2,
                   BinaryPredicate predicate)
{
    auto diff = deque1.size() - deque2.size();
    if (diff != 0) {
        return diff;
    }

    auto it1 = deque1.begin();
    auto it2 = deque2.begin();
    auto end1 = deque1.end();
    auto end2 = deque2.end();
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

template <typename T>
inline i32 compare(const DequeBase<T>& deque1,
                   const DequeBase<T>& deque2)
{
    return compare(deque1, deque2, std::less<T>());
}

} /* end namespace deque_detail_ */

template <typename T>
static inline bool operator == (const DequeBase<T>& deque1,
                                const DequeBase<T>& deque2)
{
    return (deque_detail_::compare(deque1, deque2) == 0);
}

template <typename T>
static inline bool operator != (const DequeBase<T>& deque1,
                                const DequeBase<T>& deque2)
{
    return (deque_detail_::compare(deque1, deque2) != 0);
}

template <typename T>
static inline bool operator < (const DequeBase<T>& deque1,
                               const DequeBase<T>& deque2)
{
    return (deque_detail_::compare(deque1, deque2) < 0);
}
template <typename T>
static inline bool operator > (const DequeBase<T>& deque1,
                               const DequeBase<T>& deque2)
{
    return (deque_detail_::compare(deque1, deque2) > 0);
}
template <typename T>
static inline bool operator <= (const DequeBase<T>& deque1,
                                const DequeBase<T>& deque2)
{
    return (deque_detail_::compare(deque1, deque2) <= 0);
}
template <typename T>
static inline bool operator >= (const DequeBase<T>& deque1,
                                const DequeBase<T>& deque2)
{
    return (deque_detail_::compare(deque1, deque2) >= 0);
}


// Some useful typedefs
template <typename T>
using Deque = DequeBase<T>;

template <typename T>
using PtrDeque = DequeBase<T*>;

template <typename T>
using ConstPtrDeque = DequeBase<const T*>;

template <typename T>
using MPtrDeque =
    DequeBase<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                        memory::ManagedPointer<T>, T*>::type>;

template <typename T>
using ConstMPtrDeque =
    DequeBase<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                        memory::ManagedConstPointer<T>,
                                        const T*>::type>;

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
