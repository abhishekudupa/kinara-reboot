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

template <typename T, typename IncrementFunc,
          u32 MAXSLACK, typename ConstructFunc,
          typename DestructFunc>
class VectorBase final
{
public:
    typedef T ValueType;
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

    inline void call_destructors() const
    {
        DestructFunc the_destruct_func;
        for (u64 i = 0, size = get_size(); i < size; ++i) {
            the_destruct_func(m_data[i]);
        }
    }

    inline T* allocate_data(u64 num_elements)
    {
        auto retval = ka::casted_allocate_raw<T>(sizeof(T) * num_elements + sc_array_overhead);
        auto retval_as_ptr_to_u64 = static_cast<u64*>(static_cast<void*>(retval));
        return static_cast<T*>(static_cast<void*>(retval_as_ptr_to_u64 + 2));
    }

    inline void deallocate_data()
    {
        if (m_data == nullptr) {
            return;
        }
        ka::deallocate_raw(get_array_ptr(), get_array_size());
    }

    inline void copy_core(T* dst_ptr, const T* src_start, const T* src_end)
    {
        auto num_elems = std::distance(src_start, src_end);
        memcpy(dst_ptr, src_start, sizeof(T) * num_elems);
    }

    template <typename InputIterator>
    inline void construct_core(T* dst_ptr, InputIterator first, InputIterator last)
    {
        ConstructFunc the_construct_func;
        InputIterator it = first;
        for (auto cur_ptr = dst_ptr; it != last; ++cur_ptr, ++it) {
            the_construct_func(cur_ptr, *it);
        }
    }

    inline void construct_core(T* dst_ptr, u64 n, const ValueType& value)
    {
        ConstructFunc the_construct_func;
        u64 i = 0;
        for (auto cur_ptr = dst_ptr; i < n; ++i, ++cur_ptr) {
            the_construct_func(cur_ptr, value);
        }
    }

    inline void* get_array_ptr() const
    {
        return static_cast<void*>(static_cast<u64*>(static_cast<void*>(m_data)) - 2);
    }

    // expand to accommodate one more element
    inline void expand()
    {
        auto capacity = get_capacity();
        auto size = get_size();

        if (capacity > size) {
            return;
        }

        IncrementFunc increment_fun;
        auto new_capacity = increment_fun(capacity);

        auto array_size = get_array_size();
        auto new_data = allocate_data(new_capacity);

        if (m_data != nullptr) {
            memcpy(new_data, m_data, size * sizeof(T));
            ka::deallocate_raw(get_array_ptr(), array_size);
        }

        m_data = new_data;
        set_size(size);
        set_capacity(new_capacity);
    }

    inline void expand(u64 new_capacity)
    {
        auto capacity = get_capacity();
        auto size = get_size();

        if (capacity >= new_capacity) {
            return;
        }

        auto array_size = get_array_size();

        // need to reallocate
        auto new_data = allocate_data(new_capacity);

        if (m_data != nullptr) {
            memcpy(new_data, m_data, size * sizeof(T));
            ka::deallocate_raw(get_array_ptr(), array_size);
        }

        m_data = new_data;
        set_size(size);
        set_capacity(new_capacity);
    }

    inline Iterator expand_with_hole(u64 hole_size,
                                     ConstIterator hole_position)
    {
        auto capacity = get_capacity();
        auto array_size = get_array_size();
        auto size = get_size();
        auto new_capacity = size + hole_size;
        Iterator retval = const_cast<T*>(hole_position);

        if (capacity >= new_capacity) {
            // punch a hole and return
            memmove(const_cast<T*>(hole_position) + hole_size, hole_position,
                    std::distance(hole_position, cend()));
            return retval;
        }
        // reallocate
        auto new_data = allocate_data(new_capacity);
        auto offset_from_begin = std::distance(cbegin(), hole_position);
        retval = new_data + offset_from_begin;

        if (m_data != nullptr) {
            memcpy(new_data, m_data, sizeof(T) * offset_from_begin);
            memcpy(new_data + offset_from_begin + hole_size,
                   m_data + offset_from_begin,
                   sizeof(T) * (size - offset_from_begin));
            ka::deallocate_raw(get_array_ptr(), array_size);
        }

        m_data = new_data;

        set_capacity(new_capacity);
        set_size(size);
        return retval;
    }

    inline void compact(u64 allowed_slack = MAXSLACK)
    {
        auto array_size = get_array_size();
        auto capacity = get_capacity();
        auto size = get_size();
        if (capacity <= size * allowed_slack) {
            return;
        }

        // reallocate the buffer
        auto new_data = allocate_data(size);
        memcpy(new_data, m_data, sizeof(T) * size);
        ka::deallocate_raw(get_array_ptr(), array_size);

        m_data = new_data;
        set_size(size);
        set_capacity(size);
    }

public:
    // we define the assign functions first
    template <typename InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        call_destructors();
        auto new_size = (u64)std::distance(first, last);

        if (new_size == 0) {
            deallocate_data();
            return;
        }

        if (new_size > get_capacity()) {
            deallocate_data();
            m_data = allocate_data(new_size);
            set_size(new_size);
            set_capacity(new_size);
        } else {
            set_size(new_size);
        }

        construct_core(m_data, std::move(first), std::move(last));
        compact();
    }

    void assign(u64 n, const ValueType& value)
    {
        call_destructors();
        if (n > get_capacity()) {
            deallocate_data();
            m_data = allocate_data(n);
            set_size(n);
            set_capacity(n);
        } else {
            set_size(n);
        }
        construct_core(m_data, n, value);
        compact();
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
        : VectorBase(size)
    {
        if (size == 0) {
            return;
        }
        assign(size, value);
    }

    template <typename InputIterator>
    VectorBase(InputIterator first, InputIterator last)
        : m_data(nullptr)
    {
        auto size = distance(first, last);
        if (size == 0) {
            return;
        }
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
        call_destructors();
        deallocate_data();
        m_data = nullptr;
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
        call_destructors();
        m_data = nullptr;
        std::swap(other.m_data, m_data);
        return *this;
    }

    VectorBase& operator = (std::initializer_list<ValueType> init_list)
    {
        assign(std::move(init_list));
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
            DestructFunc the_destruct_func;
            for (u64 i = new_size; i < old_size; ++i) {
                the_destruct_func(m_data[i]);
            }
            set_size(new_size);
            compact();
        } else if (new_size > old_size) {
            if (new_size > get_capacity()) {
                expand(new_size);
            }
            construct_core(begin() + old_size, (new_size - old_size), value);
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
        compact(1);
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
        ConstructFunc the_construct_func;
        the_construct_func(&(m_data[get_size()]), value);
        increment_size();
    }

    void push_back(ValueType&& value)
    {
        expand();
        ConstructFunc the_construct_func;
        the_construct_func(&(m_data[get_size()]), std::move(value));
        increment_size();
    }

    void pop_back()
    {
        DestructFunc the_destruct_func;
        the_destruct_func(m_data[get_size() - 1]);
        set_size(get_size() - 1);
    }

    Iterator insert(ConstIterator position, const ValueType& value)
    {
        auto actual_pos = expand_with_hole(1, position);
        ConstructFunc the_construct_func;
        the_construct_func(actual_pos, value);
        increment_size();
        return actual_pos;
    }

    Iterator insert(ConstIterator position, u64 n, const ValueType& value)
    {
        auto actual_pos = expand_with_hole(n, position);
        ConstructFunc the_construct_func;
        construct_core(actual_pos, n, value);
        set_size(get_size() + n);
        return actual_pos;
    }

    template <typename InputIterator>
    Iterator insert(ConstIterator position, InputIterator first, InputIterator last)
    {
        auto num_elements = std::distance(first, last);
        auto actual_pos = expand_with_hole(num_elements, position);
        construct_core(actual_pos, first, last);
        set_size(get_size() + num_elements);
        return actual_pos;
    }

    Iterator insert(ConstIterator position, ValueType&& value)
    {
        auto actual_pos = expand_with_hole(1, position);
        ConstructFunc the_construct_func;
        the_construct_func(actual_pos, std::move(value));
        set_size(get_size() + 1);
        return actual_pos;
    }

    Iterator insert(ConstIterator position, std::initializer_list<ValueType> il)
    {
        auto num_elements = il.size();
        auto actual_pos = expand_with_hole(num_elements, position);
        construct_core(actual_pos, il.begin(), il.end());
        set_size(get_size() + num_elements);
        return actual_pos;
    }

    Iterator erase(ConstIterator position)
    {
        DestructFunc the_destruct_func;
        the_destruct_func(*position);

        memmove(position, position + 1, sizeof(T) * std::distance(position + 1, cend()));

        set_size(get_size() - 1);
        compact();
        return (const_cast<T*>(position));
    }

    Iterator erase(ConstIterator first, ConstIterator last)
    {
        auto num_deleted_elements = std::distance(first, last);
        DestructFunc the_destruct_func;
        for (auto it = first; it != last; ++it) {
            the_destruct_func(*it);
        }
        memmove(first, first + num_deleted_elements,
                sizeof(T) * std::distance(first + num_deleted_elements, cend()));
        set_size(get_size() - num_deleted_elements);
        compact();
        return (const_cast<T*>(first));
    }

    void swap(const VectorBase& other)
    {
        std::swap(m_data, other.m_data);
    }

    void clear()
    {
        call_destructors();
        reset();
    }

    // clear, but without destructors
    void reset()
    {
        deallocate_data();
        m_data = nullptr;
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
};

// overload of relational operators for vector

namespace detail {
template <typename T, typename Inc1, typename Inc2,
          u32 MS1, u32 MS2, typename CF1, typename CF2,
          typename DF1, typename DF2>
static inline i64 compare(const VectorBase<T, Inc1, MS1, CF1, DF1>& vector_1,
                          const VectorBase<T, Inc2, MS2, CF2, DF2>& vector_2)
{
    auto diff = vector_1.size() - vector_2.size();
    if (diff != 0) {
        return diff;
    }
    auto size = vector_1.size();
    for (u64 i = 0; i < size; ++i) {
        if (vector_1[i] < vector_2[i]) {
            return -1;
        } else if (vector_1[i] > vector_2[i]) {
            return 1;
        }
    }
    return 0;
}

} /* end namespace detail */

template <typename T, typename Inc1, typename Inc2,
          u32 MS1, u32 MS2, typename CF1, typename CF2,
          typename DF1, typename DF2>
static inline bool operator == (const VectorBase<T, Inc1, MS1, CF1, DF1>& vector_1,
                                const VectorBase<T, Inc2, MS2, CF2, DF2>& vector_2)
{
    return (detail::compare(vector_1, vector_2) == 0);
}

template <typename T, typename Inc1, typename Inc2,
          u32 MS1, u32 MS2, typename CF1, typename CF2,
          typename DF1, typename DF2>
static inline bool operator != (const VectorBase<T, Inc1, MS1, CF1, DF1>& vector_1,
                                const VectorBase<T, Inc2, MS2, CF2, DF2>& vector_2)
{
    return (detail::compare(vector_1, vector_2) != 0);
}

template <typename T, typename Inc1, typename Inc2,
          u32 MS1, u32 MS2, typename CF1, typename CF2,
          typename DF1, typename DF2>
static inline bool operator < (const VectorBase<T, Inc1, MS1, CF1, DF1>& vector_1,
                                const VectorBase<T, Inc2, MS2, CF2, DF2>& vector_2)
{
    return (detail::compare(vector_1, vector_2) < 0);
}

template <typename T, typename Inc1, typename Inc2,
          u32 MS1, u32 MS2, typename CF1, typename CF2,
          typename DF1, typename DF2>
static inline bool operator <= (const VectorBase<T, Inc1, MS1, CF1, DF1>& vector_1,
                                const VectorBase<T, Inc2, MS2, CF2, DF2>& vector_2)
{
    return (detail::compare(vector_1, vector_2) <= 0);
}

template <typename T, typename Inc1, typename Inc2,
          u32 MS1, u32 MS2, typename CF1, typename CF2,
          typename DF1, typename DF2>
static inline bool operator > (const VectorBase<T, Inc1, MS1, CF1, DF1>& vector_1,
                                const VectorBase<T, Inc2, MS2, CF2, DF2>& vector_2)
{
    return (detail::compare(vector_1, vector_2) > 0);
}

template <typename T, typename Inc1, typename Inc2,
          u32 MS1, u32 MS2, typename CF1, typename CF2,
          typename DF1, typename DF2>
static inline bool operator >= (const VectorBase<T, Inc1, MS1, CF1, DF1>& vector_1,
                                const VectorBase<T, Inc2, MS2, CF2, DF2>& vector_2)
{
    return (detail::compare(vector_1, vector_2) >= 0);
}

// some useful typedefs
template <typename T,
          typename ConstructFunc = DefaultConstructFunc<T>,
          typename DestructFunc = DefaultDestructFunc<T>>
using CompactVector = VectorBase<T, detail::AdditiveIncrementer<16>,
                                 1, ConstructFunc, DestructFunc>;

template <typename T,
          typename ConstructFunc = DefaultConstructFunc<T>,
          typename DestructFunc = DefaultDestructFunc<T>>
using Vector = VectorBase<T, detail::MultiplicativeIncrementer<3, 2, 8>,
                          2, ConstructFunc, DestructFunc>;

template <typename T>
using PtrVector = VectorBase<T*, detail::MultiplicativeIncrementer<3, 2, 8>,
                             2, DefaultConstructFunc<T*>,
                             DefaultDestructFunc<T*>>;

template <typename T>
using CompactPtrVector = VectorBase<T*, detail::AdditiveIncrementer<16>, 1,
                                    DefaultConstructFunc<T*>,
                                    DefaultDestructFunc<T*>>;

class String;

typedef Vector<u08> u08Vector;
typedef Vector<i08> i08Vector;
typedef Vector<u16> u16Vector;
typedef Vector<i16> i16Vector;
typedef Vector<u32> u32Vector;
typedef Vector<i32> i32Vector;
typedef Vector<u64> u64Vector;
typedef Vector<i64> i64Vector;

typedef CompactVector<u08> u08CompactVector;
typedef CompactVector<i08> i08CompactVector;
typedef CompactVector<u16> u16CompactVector;
typedef CompactVector<i16> i16CompactVector;
typedef CompactVector<u32> u32CompactVector;
typedef CompactVector<i32> i32CompactVector;
typedef CompactVector<u64> u64CompactVector;
typedef CompactVector<i64> i64CompactVector;

typedef Vector<String> StringVector;
typedef CompactVector<String> StringCompactVector;

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_VECTOR_HPP_ */

//
// Vector.hpp ends here
