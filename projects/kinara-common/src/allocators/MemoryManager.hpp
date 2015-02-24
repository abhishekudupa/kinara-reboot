// MemoryManager.hpp ---
// Filename: MemoryManager.hpp
// Author: Abhishek Udupa
// Created: Fri Feb 13 01:47:38 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_ALLOCATORS_MEMORY_MANAGER_HPP_
#define KINARA_KINARA_COMMON_ALLOCATORS_MEMORY_MANAGER_HPP_

#include <exception>
#include <utility>

#include "../basetypes/KinaraBase.hpp"

namespace kinara {
namespace allocators {

class OutOfMemoryError : public std::exception
{
public:
    OutOfMemoryError() noexcept;
    virtual ~OutOfMemoryError() noexcept;
    const char* what() const noexcept override;
};


class MemoryManager final
{
private:
    static u64 s_total_bytes_allocated;
    static u64 s_memory_allocation_limit;
    static u64 s_warn_watermark;
    static u64 s_peak_bytes_allocated;
    static constexpr u64 sc_block_header_size = sizeof(u64);

public:
    static void* allocate(u64 size);
    static void* allocate_cleared(u64 size);

    static void* allocate_raw(u64 size);
    static void* allocate_raw_cleared(u64 size);

    static void deallocate(const void* block_ptr);
    static void deallocate_raw(const void* block_ptr, u64 size);

    static void set_allocation_limit(u64 allocation_limit);
    static void set_warn_watermark(u64 warn_watermark);
    static u64 get_bytes_allocated();
    static u64 get_peak_bytes_allocated();
    static u64 get_allocation_limit();
    static u64 get_warn_watermark();

    bool is_warn_watermark_reached();
    bool is_out_of_memory();
};

static inline void* allocate(u64 size)
{
    return MemoryManager::allocate(size);
}

static inline void* allocate_cleared(u64 size)
{
    return MemoryManager::allocate_cleared(size);
}

static inline void* allocate_raw(u64 size)
{
    return MemoryManager::allocate_raw(size);
}

static inline void* allocate_raw_cleared(u64 size)
{
    return MemoryManager::allocate_raw_cleared(size);
}

template <typename T>
static inline T* casted_allocate(u64 size)
{
    return static_cast<T*>(MemoryManager::allocate(size));
}

template <typename T>
static inline T* casted_allocate_raw(u64 size)
{
    return static_cast<T*>(MemoryManager::allocate_raw(size));
}

template <typename T>
static inline T* casted_allocate_cleared(u64 size)
{
    return static_cast<T*>(MemoryManager::allocate_cleared(size));
}

template <typename T>
static inline T* casted_allocate_raw_cleared(u64 size)
{
    return static_cast<T*>(MemoryManager::allocate_raw_cleared(size));
}

template <typename T>
static inline void deallocate(const T* block_ptr)
{
    MemoryManager::deallocate(block_ptr);
}

template <typename T>
static inline void deallocate_raw(const T* block_ptr, u64 size)
{
    MemoryManager::deallocate_raw(block_ptr, size);
}

template <typename T, typename... ArgTypes>
static inline T* allocate_array(u64 num_elements, ArgTypes&&... args)
{
    auto retval = casted_allocate<T>(sizeof(T) * num_elements);
    for (u64 i = 0; i < num_elements; ++i) {
        new (retval + i) T(std::forward<ArgTypes>(args)...);
    }
    return retval;
}

template <typename T>
static inline void deallocate_array(const T* array_ptr)
{
    if (array_ptr == nullptr) {
        return;
    }
    const void* array_ptr_as_void = static_cast<const void*>(array_ptr);
    const u64* array_ptr_as_u64 = static_cast<const u64*>(array_ptr_as_void) - 1;
    // we need to compute the actual size of this array
    u64 actual_num_elements = *array_ptr_as_u64;
    actual_num_elements = actual_num_elements / sizeof(T*);

    auto cur_ptr = array_ptr;
    for (u64 i = 0; i < actual_num_elements; ++i) {
        cur_ptr->~T();
        ++cur_ptr;
    }
    MemoryManager::deallocate(array_ptr_as_void);
}

template <typename T, typename... ArgTypes>
static inline T* allocate_array_raw(u64 num_elements, ArgTypes&&... args)
{
    auto retval = casted_allocate_raw<T>(sizeof(T) * num_elements);
    for (u64 i = 0; i < num_elements; ++i) {
        new (retval + i) T(std::forward<ArgTypes>(args)...);
    }
    return retval;
}

template <typename T>
static inline void deallocate_array_raw(T* array_ptr, u64 num_elements)
{
    auto cur_ptr = array_ptr;
    for (u64 i = 0; i < num_elements; ++i) {
        cur_ptr->~T();
        ++cur_ptr;
    }
    MemoryManager::deallocate_raw(array_ptr, num_elements * sizeof(T));
}

// allocates an uninitialized array
template <typename T>
static inline T* allocate_uarray(u64 num_elements)
{
    return casted_allocate<T>(sizeof(T) * num_elements);
}

template <typename T>
static inline T* allocate_uarray_raw(u64 num_elements)
{
    return casted_allocate_raw<T>(sizeof(T) * num_elements);
}

// deallocates array without calling destructors
template <typename T>
static inline void deallocate_uarray(T* array_ptr)
{
    MemoryManager::deallocate(array_ptr);
}

template <typename T>
static inline void deallocate_uarray_raw(T* array_ptr, u64 num_elements)
{
    MemoryManager::deallocate_raw(array_ptr, sizeof(T) * num_elements);
}

template <typename T, typename... ArgTypes>
static inline T* allocate_object(ArgTypes&&... args)
{
    auto retval = allocate(sizeof(T));
    new (retval) T(std::forward<ArgTypes>(args)...);
    return static_cast<T*>(retval);
}

template <typename T, typename... ArgTypes>
static inline T* allocate_object_raw(ArgTypes&&... args)
{
    auto retval = allocate_raw(sizeof(T));
    new (retval) T(std::forward<ArgTypes>(args)...);
    return static_cast<T*>(retval);
}

template <typename T>
static inline void deallocate_object(const T* object_ptr,
                                     const std::true_type& is_polymorphic_value)
{
    deallocate(dynamic_cast<void*>(const_cast<T*>(object_ptr)));
}

template <typename T>
static inline void deallocate_object(const T* object_ptr,
                                     const std::false_type& is_polymorphic_value)
{
    deallocate(object_ptr);
}

template <typename T>
static inline void deallocate_object(const T* object_ptr)
{
    typename std::is_polymorphic<T>::type is_polymorphic_value;
    object_ptr->~T();
    deallocate_object(object_ptr, is_polymorphic_value);
}

template <typename T>
static inline void deallocate_object_raw(const T* object_ptr,
                                         u64 size,
                                         const std::true_type& is_polymorphic_value)
{
    deallocate_raw(dynamic_cast<void*>(const_cast<T*>(object_ptr)), size);
}

template <typename T>
static inline void deallocate_object_raw(const T* object_ptr,
                                         u64 size,
                                         const std::false_type& is_polymorphic_value)
{
    deallocate_raw(object_ptr, size);
}

template <typename T>
static inline void deallocate_object_raw(const T* object_ptr, u64 size)
{
    typename std::is_polymorphic<T>::type is_polymorphic_value;
    object_ptr->~T();
    deallocate_object_raw(object_ptr, size, is_polymorphic_value);
}


} /* end namespace allocators */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_ALLOCATORS_MEMORY_MANAGER_HPP_ */

//
// MemoryManager.hpp ends here
