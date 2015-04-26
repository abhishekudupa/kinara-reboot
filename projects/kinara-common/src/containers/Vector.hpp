// Vector.hpp ---
// Filename: Vector.hpp
// Author: Abhishek Udupa
// Created: Sat Feb 14 00:57:31 2015 (-0500)
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

// Custom vector class

#if !defined KINARA_KINARA_COMMON_CONTAINERS_VECTOR_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_VECTOR_HPP_

#include <cstring>
#include <initializer_list>
#include <algorithm>
#include <functional>

#include "../basetypes/KinaraBase.hpp"
#include "../basetypes/KinaraTypes.hpp"
#include "../allocators/MemoryManager.hpp"

#include "ContainersBase.hpp"

namespace kinara {
namespace containers {

// custom vector class. API compatible with
// the C++ stl vector, except for the allocators
// bit, which we tailor to use our allocator

namespace ka = kinara::allocators;
namespace kc = kinara::containers;

template <typename T>
class VectorBase final
{
public:
    typedef T ValueType;
    typedef T value_type;
    typedef T* PtrType;
    typedef const T* ConstPtrType;
    typedef T& RefType;
    typedef const T& ConstRefType;

    typedef T* Iterator;
    typedef T* iterator;
    typedef const T* ConstIterator;
    typedef const T* const_iterator;
    typedef std::reverse_iterator<iterator> ReverseIterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> ConstReverseIterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

private:
    T* m_data;

    static constexpr u64 sc_array_overhead = (sizeof(u64) * 2);
    static constexpr u64 sc_max_size = (UINT64_MAX - sc_array_overhead) / sizeof(ValueType);

    inline u64 get_size() const
    {
        if (m_data == nullptr) {
            return 0;
        }
        return *((static_cast<u64*>(static_cast<void*>(m_data))) - 1);
    }

    inline void set_size(u64 size)
    {
        if (m_data == nullptr) {
            return;
        }
        *((static_cast<u64*>(static_cast<void*>(m_data))) - 1) = size;
    }

    inline void increment_size()
    {
        if (m_data == nullptr) {
            return;
        }
        ++(*((static_cast<u64*>(static_cast<void*>(m_data))) - 1));
    }

    inline void decrement_size()
    {
        if (m_data == nullptr) {
            return;
        }
        --(*((static_cast<u64*>(static_cast<void*>(m_data))) - 1));
    }

    inline u64 get_capacity() const
    {
        if (m_data == nullptr) {
            return 0;
        }
        return *((static_cast<u64*>(static_cast<void*>(m_data))) - 2);
    }

    inline void set_capacity(u64 capacity)
    {
        if (m_data == nullptr) {
            return;
        }
        *((static_cast<u64*>(static_cast<void*>(m_data))) - 2) = capacity;
    }

    inline u64 get_array_size() const
    {
        if (m_data == nullptr) {
            return 0;
        }
        return ((*((static_cast<u64*>(static_cast<void*>(m_data))) - 2) *
                 sizeof(T)) + sc_array_overhead);
    }

    inline T* allocate_data(u64 num_elements, std::true_type is_trivial_value)
    {
        auto retval =
            ka::casted_allocate_raw_cleared<T>(sizeof(T) * num_elements + sc_array_overhead);
        auto retval_as_ptr_to_u64 = static_cast<u64*>(static_cast<void*>(retval));
        return static_cast<T*>(static_cast<void*>(retval_as_ptr_to_u64 + 2));;
    }

    inline T* allocate_data(u64 num_elements, std::false_type is_trivial_value)
    {
        auto buffer = ka::casted_allocate_raw<T>(sizeof(T) * num_elements + sc_array_overhead);
        auto buffer_as_ptr_to_u64 = static_cast<u64*>(static_cast<void*>(buffer));
        auto retval = static_cast<T*>(static_cast<void*>(buffer_as_ptr_to_u64 + 2));
        for (auto cur_ptr = retval, last = retval + num_elements; cur_ptr != last; ++cur_ptr) {
            new (cur_ptr) T();
        }
        return retval;
    }

    inline T* allocate_data(u64 num_elements)
    {
        typename std::is_trivially_default_constructible<T>::type is_trivial_value;
        return allocate_data(num_elements, is_trivial_value);
    }

    inline void deallocate_data()
    {
        if (m_data == nullptr) {
            return;
        }
        ka::deallocate_raw(get_array_ptr(), get_array_size());
        m_data = nullptr;
    }

    inline void destroy_range(T* first, T* last, std::true_type is_trivial_value)
    {
        return;
    }

    inline void destroy_range(T* first, T* last, std::false_type is_trivial_value)
    {
        for (auto cur_ptr = first; cur_ptr != last; ++cur_ptr) {
            cur_ptr->~ValueType();
        }
    }

    inline void destroy_range(T* first, T* last)
    {
        typename std::is_trivially_destructible<T>::type is_trivial_value;
        destroy_range(first, last, is_trivial_value);
    }


    inline void destroy_range()
    {
        if (m_data == nullptr) {
            return;
        }
        destroy_range(m_data, m_data + get_size());
    }

    inline void* get_array_ptr() const
    {
        return static_cast<void*>(static_cast<u64*>(static_cast<void*>(m_data)) - 2);
    }

    // expand to accommodate one more element
    inline void expand()
    {
        auto old_capacity = get_capacity();
        auto old_size = get_size();

        if (old_capacity > old_size) {
            return;
        }

        // need to resize
        auto new_capacity = std::max((old_capacity * 3) / 2, 8ul);
        expand(new_capacity);
    }

    inline void expand(u64 new_capacity)
    {
        auto old_capacity = get_capacity();
        auto old_size = get_size();

        if (old_capacity >= new_capacity) {
            return;
        }

        // need to reallocate
        auto new_data = allocate_data(new_capacity);

        if (m_data != nullptr) {
            std::move(m_data, m_data + old_size, new_data);
            destroy_range();
            deallocate_data();
        }

        m_data = new_data;
        set_size(old_size);
        set_capacity(new_capacity);
    }

    inline T* expand_with_hole(u64 hole_size,
                               T* hole_position)
    {
        auto old_capacity = get_capacity();
        auto old_size = get_size();

        auto new_capacity = old_size + hole_size;
        Iterator retval = hole_position;

        if (old_capacity >= new_capacity) {
            // punch a hole and return
            std::move_backward(hole_position, m_data + old_size, hole_position + hole_size);
            return retval;
        }

        // reallocate
        auto new_data = allocate_data(new_capacity);
        auto offset_from_begin = hole_position - m_data;
        retval = new_data + offset_from_begin;

        if (m_data != nullptr) {
            std::move(m_data, m_data + offset_from_begin, new_data);
            std::move(m_data + offset_from_begin, m_data + old_size,
                      new_data + offset_from_begin + hole_size);
            destroy_range();
            deallocate_data();
        }

        m_data = new_data;
        set_capacity(new_capacity);
        set_size(old_size);

        return retval;
    }

    inline void compact(bool strict = false)
    {
        auto orig_capacity = get_capacity();
        auto orig_size = get_size();

        if (!strict && orig_capacity <= std::max((4 * orig_size) / 3, 8ul)) {
            return;
        }

        // reallocate the buffer
        auto new_data = allocate_data(orig_size);

        std::move(m_data, m_data + orig_size, new_data);
        destroy_range();
        deallocate_data();

        m_data = new_data;
        set_size(orig_size);
        set_capacity(orig_size);
    }

    template <typename ForwardIterator>
    inline void insert_range(const ConstIterator& position,
                             const ForwardIterator& first,
                             const ForwardIterator& last,
                             std::forward_iterator_tag unused)
    {
        auto num_elements = std::distance(first, last);
        auto actual_pos = expand_with_hole(num_elements, const_cast<T*>(position));
        std::copy(first, last, actual_pos);
        set_size(get_size() + num_elements);
        return;
    }

    template <typename InputIterator>
    inline void insert_range(const ConstIterator& position,
                             const InputIterator& first,
                             const InputIterator& last,
                             std::input_iterator_tag unused)
    {
        Iterator actual_pos = position;
        for (auto it = first; it != last; ++it) {
            actual_pos = this->insert(actual_pos, *it);
            ++actual_pos;
        }
    }

    template <typename ForwardIterator>
    inline void assign_range(const ForwardIterator& first, const ForwardIterator& last,
                             std::forward_iterator_tag unused)
    {
        destroy_range();

        auto new_size = (u64)std::distance(first, last);
        auto old_capacity = get_capacity();

        if (new_size == 0) {
            deallocate_data();
            return;
        }

        if (new_size > old_capacity ||
            old_capacity > std::max((4 * new_size) / 3, 8ul)) {
            deallocate_data();
            m_data = allocate_data(new_size);
            set_size(new_size);
            set_capacity(new_size);
        } else {
            set_size(new_size);
        }

        std::copy(first, last, m_data);
    }

    template <typename InputIterator>
    inline void assign_range(const InputIterator& first, const InputIterator& last,
                             std::input_iterator_tag unused)
    {
        destroy_range();

        if (first == last) {
            deallocate_data();
            return;
        }

        for (auto it = first; it != last; ++it) {
            this->push_back(*it);
        }

        return;
    }

public:
    // we define the assign functions first
    template <typename InputIterator>
    void assign(const InputIterator& first, const InputIterator& last)
    {
        typedef typename std::iterator_traits<InputIterator>::iterator_category IterCategory;
        assign_range(first, last, IterCategory());
    }

    void assign(u64 n, const ValueType& value)
    {
        auto old_capacity = get_capacity();
        destroy_range();

        if (n > old_capacity ||
            old_capacity > std::max((4 * n) / 3, 8ul)) {
            deallocate_data();
            m_data = allocate_data(n);
            set_size(n);
            set_capacity(n);
        } else {
            set_size(n);
        }

        std::fill_n(m_data, n, value);
    }

    void assign(std::initializer_list<ValueType> init_list)
    {
        assign(init_list.begin(), init_list.end());
    }

    explicit VectorBase()
        : m_data(nullptr)
    {
        // Nothing here
    }

    explicit VectorBase(u64 size)
        : VectorBase(size, ValueType())
    {
        // Nothing here
    }

    VectorBase(u64 size, const ValueType& value)
        : VectorBase()
    {
        if (size == 0) {
            return;
        }
        assign(size, value);
    }

    template <typename InputIterator>
    VectorBase(const InputIterator& first, const InputIterator& last)
        : m_data(nullptr)
    {
        assign(first, last);
    }

    // overload default copy and move constructors
    VectorBase(const VectorBase& other)
        : m_data(nullptr)
    {
        auto size = other.size();
        if (size == 0) {
            return;
        }
        assign(other.begin(), other.end());
    }

    VectorBase(const VectorBase&& other)
        : m_data(nullptr)
    {
        std::swap(m_data, other.m_data);
    }

    VectorBase(std::initializer_list<ValueType> init_list)
        : m_data(nullptr)
    {
        auto size = init_list.size();
        if (size == 0) {
            return;
        }
        assign(init_list.begin(), init_list.end());
    }

    ~VectorBase()
    {
        if (m_data == nullptr) {
            return;
        }
        destroy_range();
        deallocate_data();
    }

    // The default assignment operator
    VectorBase& operator = (const VectorBase& other)
    {
        if (&other == this) {
            return *this;
        }
        assign(other.begin(), other.end());
        return *this;
    }

    // move assignment
    VectorBase& operator = (VectorBase&& other)
    {
        if (&other == this) {
            return *this;
        }

        std::swap(other.m_data, m_data);
        return *this;
    }

    VectorBase& operator = (std::initializer_list<ValueType> init_list)
    {
        assign(init_list.begin(), init_list.end());
        return *this;
    }

    Iterator begin() noexcept
    {
        return m_data;
    }

    ConstIterator begin() const noexcept
    {
        return m_data;
    }

    Iterator end() noexcept
    {
        return (m_data + get_size());
    }

    ConstIterator end() const noexcept
    {
        return (m_data + get_size());
    }

    ReverseIterator rbegin() noexcept
    {
        return ReverseIterator(m_data + get_size());
    }

    ConstReverseIterator rbegin() const noexcept
    {
        return ConstReverseIterator(m_data + get_size());
    }

    ReverseIterator rend() noexcept
    {
        return ReverseIterator(m_data);
    }

    ConstReverseIterator rend() const noexcept
    {
        return ConstReverseIterator(m_data);
    }

    ConstIterator cbegin() const noexcept
    {
        return m_data;
    }

    ConstIterator cend() const noexcept
    {
        return (m_data + get_size());
    }

    ConstReverseIterator crbegin() const noexcept
    {
        return ConstReverseIterator(m_data + get_size());
    }

    ConstReverseIterator crend() const noexcept
    {
        return ConstReverseIterator(m_data);
    }

    u64 size() const
    {
        return get_size();
    }

    u64 max_size() const
    {
        return sc_max_size;
    }

    void resize(u64 new_size, const ValueType& value = ValueType())
    {
        auto old_size = get_size();
        if (new_size < old_size) {
            set_size(new_size);
            compact();
        } else if (new_size > old_size) {
            if (new_size > get_capacity()) {
                expand(new_size);
            }
            std::fill_n(begin() + old_size, (new_size - old_size), value);
        }
        return;
    }

    u64 capacity() const
    {
        return get_capacity();
    }

    bool empty() const
    {
        return (m_data == nullptr);
    }

    void reserve(u64 new_capacity)
    {
        auto old_capacity = get_capacity();
        if (new_capacity <= old_capacity) {
            return;
        } else {
            expand(new_capacity);
            return;
        }
    }

    void shrink_to_fit()
    {
        compact(true);
    }

    RefType operator [] (u64 index)
    {
        return m_data[index];
    }

    ConstRefType operator [] (u64 index) const
    {
        return m_data[index];
    }

    RefType at(u64 index)
    {
        return m_data[index];
    }

    ConstRefType at(u64 index) const
    {
        return m_data[index];
    }

    RefType front()
    {
        return m_data[0];
    }

    ConstRefType front() const
    {
        return m_data[0];
    }

    RefType back()
    {
        return m_data[get_size() - 1];
    }

    ConstRefType back() const
    {
        return m_data[get_size() - 1];
    }

    PtrType data()
    {
        return m_data;
    }

    ConstPtrType data() const
    {
        return m_data;
    }

    void push_back(const ValueType& value)
    {
        expand();
        m_data[get_size()] = value;
        increment_size();
    }

    void push_back(ValueType&& value)
    {
        expand();
        m_data[get_size()] = std::move(value);
        increment_size();
    }

    void push_front(const ValueType& value)
    {
        insert(begin(), value);
    }

    void push_front(ValueType&& value)
    {
        insert(begin(), std::move(value));
    }

    void pop_back()
    {
        decrement_size();
    }

    void pop_front()
    {
        erase(begin());
    }

    Iterator insert(const ConstIterator& position, const ValueType& value)
    {
        auto actual_pos = expand_with_hole(1, position);
        *actual_pos = value;
        increment_size();
        return actual_pos;
    }

    Iterator insert(const ConstIterator& position, u64 n, const ValueType& value)
    {
        auto actual_pos = expand_with_hole(n, position);
        std::fill_n(actual_pos, n, value);
        set_size(get_size() + n);
        return actual_pos;
    }

    template <typename InputIterator>
    Iterator insert(const ConstIterator& position,
                    const InputIterator& first,
                    const InputIterator& last)
    {
        typedef typename std::iterator_traits<InputIterator>::iterator_category IterCategory;
        auto offset_from_begin = position - begin();
        insert_range(position, first, last, IterCategory());
        return (begin() + offset_from_begin);
    }

    Iterator insert(const ConstIterator& position, ValueType&& value)
    {
        auto actual_pos = expand_with_hole(1, position);
        *actual_pos = std::move(value);
        increment_size();
        return actual_pos;
    }

    Iterator insert(const ConstIterator& position, std::initializer_list<ValueType> il)
    {
        auto num_elements = il.size();
        auto actual_pos = expand_with_hole(num_elements, position);
        std::copy(il.begin(), il.end(), actual_pos);
        set_size(get_size() + num_elements);
        return actual_pos;
    }

    Iterator erase(const ConstIterator& position)
    {
        if (position == cend()) {
            return const_cast<T*>(position);
        }
        return erase(position, position + 1);
    }

    Iterator erase(const ConstIterator& first, const ConstIterator& last)
    {
        if (first == last) {
            return const_cast<T*>(first);
        }
        auto num_to_delete = std::distance(first, last);
        auto orig_size = get_size();
        auto orig_capacity = get_capacity();
        auto new_size = orig_size - num_to_delete;
        auto offset_from_begin = first - begin();

        if (orig_capacity <= std::max((4 * orig_size) / 3, 8ul)) {
            std::move(const_cast<T*>(last),
                      m_data + orig_size,
                      m_data + offset_from_begin);

            std::fill(m_data + new_size, m_data + orig_size, ValueType());
        } else {
            auto new_data = allocate_data(new_size);
            std::move(m_data, m_data + offset_from_begin, new_data);
            std::move(const_cast<T*>(last),
                      m_data + orig_size,
                      new_data + offset_from_begin);

            destroy_range();
            deallocate_data();

            m_data = new_data;
            set_capacity(new_size);
        }
        set_size(new_size);
        return (m_data + offset_from_begin);
    }

    void swap(const VectorBase& other)
    {
        std::swap(m_data, other.m_data);
    }

    void clear()
    {
        destroy_range();
        reset();
    }

    // clear, but without destructors
    void reset()
    {
        deallocate_data();
    }

    template <typename... ArgTypes>
    iterator emplace(const ConstIterator& position, ArgTypes&&... args)
    {
        return insert(position, T(std::forward<ArgTypes>(args)...));
    }

    template <typename... ArgTypes>
    void emplace_back(ArgTypes&&... args)
    {
        push_back(T(std::forward<ArgTypes>(args)...));
    }

    // functions that are not part of std::vector
    // find the first occurence of value
    // requires values to have the == operator defined
    Iterator find(const ValueType& value)
    {
        for (auto it = begin(), last = end(); it != last; ++it) {
            if ((*it) == value) {
                return it;
            }
        }
        return end();
    }

    ConstIterator find(const ValueType& value) const
    {
        for (auto it = begin(), last = end(); it != last; ++it) {
            if ((*it) == value) {
                return it;
            }
        }
        return end();
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

    void sort()
    {
        std::sort(begin(), end());
    }

    void sort(const std::function<bool(const T&, const T&)>& compare_fun)
    {
        std::sort(begin(), end(), compare_fun);
    }

    void stable_sort()
    {
        std::stable_sort(begin(), end());
    }

    void stable_sort(const std::function<bool(const T&, const T&)>& compare_fun)
    {
        std::stable_sort(begin(), end(), compare_fun);
    }

    // in place reverse
    void reverse()
    {
        auto right_mover = begin();
        auto left_mover = end() - 1;

        std::less<T*> less_func;

        while (less_func(right_mover, left_mover)) {
            std::swap(*right_mover, *left_mover);
            ++right_mover;
            --left_mover;
        }
    }

    template <typename BinaryPredicate>
    inline i64 compare(const VectorBase& other,
                       BinaryPredicate predicate) const
    {
        auto diff = size() - other.size();
        if (diff != 0) {
            return diff;
        }
        for (u64 i = 0, last = size(); i < last; ++i) {
            if (predicate((*this)[i], other[i])) {
                return -1;
            } else if (predicate(other[i], (*this)[i])) {
                return 1;
            }
        }
        return 0;
    }

    inline i64 compare(const VectorBase& other) const
    {
        return compare(other, std::less<T>());
    }
};

template <typename T>
static inline bool operator == (const VectorBase<T>& vector_1,
                                const VectorBase<T>& vector_2)
{
    return (vector_1.compare(vector_2) == 0);
}

template <typename T>
static inline bool operator != (const VectorBase<T>& vector_1,
                                const VectorBase<T>& vector_2)
{
    return (vector_1.compare(vector_2) != 0);
}

template <typename T>
static inline bool operator < (const VectorBase<T>& vector_1,
                               const VectorBase<T>& vector_2)
{
    return (vector_1.compare(vector_2) < 0);
}

template <typename T>
static inline bool operator > (const VectorBase<T>& vector_1,
                               const VectorBase<T>& vector_2)
{
    return (vector_1.compare(vector_2) > 0);
}

template <typename T>
static inline bool operator <= (const VectorBase<T>& vector_1,
                                const VectorBase<T>& vector_2)
{
    return (vector_1.compare(vector_2) <= 0);
}

template <typename T>
static inline bool operator >= (const VectorBase<T>& vector_1,
                                const VectorBase<T>& vector_2)
{
    return (vector_1.compare(vector_2) >= 0);
}

// Some useful typedefs
template <typename T>
using Vector = VectorBase<T>;

template <typename T>
using PtrVector = VectorBase<T*>;

template <typename T>
using ConstPtrVector = VectorBase<const T*>;

template <typename T>
using MPtrVector =
    VectorBase<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                         memory::ManagedPointer<T>, T*>::type>;

template <typename T>
using ConstMPtrVector =
    VectorBase<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                         memory::ManagedConstPointer<T>,
                                         const T*>::type>;

typedef Vector<u08> u08Vector;
typedef Vector<i08> i08Vector;
typedef Vector<u16> u16Vector;
typedef Vector<i16> i16Vector;
typedef Vector<u32> u32Vector;
typedef Vector<i32> i32Vector;
typedef Vector<u64> u64Vector;
typedef Vector<i64> i64Vector;

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_VECTOR_HPP_ */

//
// Vector.hpp ends here
