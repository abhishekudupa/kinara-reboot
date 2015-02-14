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

public:
    static void* allocate(u64 size);
    static void* allocate_cleared(u64 size);

    static void* allocate_raw(u64 size);
    static void* allocate_raw_cleared(u64 size);

    static void deallocate(void* block_ptr);
    static void deallocate_raw(void* block_ptr, u64 size);

    static void set_allocation_limit(u64 allocation_limit);
    static void set_warn_watermark(u64 warn_watermark);
    static u64 get_bytes_allocated();
    static u64 get_peak_bytes_allocated();
    static u64 get_allocation_limit();
    static u64 get_warn_watermark();

    bool is_warn_watermark_reached();
    bool is_out_of_memory();
};

template <typename T>
static inline T* casted_alloc(u64 size)
{
    return reinterpret_cast<T*>(MemoryManager::allocate(size));
}

template <typename T>
static inline T* casted_alloc_raw(u64 size)
{
    return reinterpret_cast<T*>(MemoryManager::allocate_raw(size));
}

template <typename T>
static inline T* casted_alloc_cleared(u64 size)
{
    return reinterpret_cast<T*>(MemoryManager::allocate_cleared(size));
}

template <typename T>
static inline T* casted_alloc_raw_cleared(u64 size)
{
    return reinterpret_cast<T*>(MemoryManager::allocate_raw_cleared(size));
}

template <typename T, typename... ArgTypes>
static inline T* allocate_array(u64 num_elements, ArgTypes&&... args)
{
    auto retval = casted_alloc<T>(sizeof(T) * num_elements);
    for (u64 i = 0; i < num_elements; ++i) {
        new (retval[i]) T(std::forward<ArgTypes>(args)...);
    }
    return retval;
}

template <typename T>
static inline void deallocate_array(T* array_ptr)
{
    if (array_ptr == nullptr) {
        return;
    }
    // we need to compute the actual size of this array
    u64 actual_num_elements = (reinterpret_cast<u64*>(array_ptr))[-1];
    actual_num_elements = actual_num_elements / sizeof(T*);

    auto cur_ptr = array_ptr;
    for (i64 i = 0; i < actual_num_elements; ++i) {
        cur_ptr->~T();
        ++cur_ptr;
    }
    MemoryManager::deallocate(array_ptr);
}

template <typename T, typename... ArgTypes>
static inline T* allocate_raw_array(u64 num_elements, ArgTypes&&... args)
{
    auto retval = casted_raw_alloc<T>(sizeof(T) * num_elements);
    for (u64 i = 0; i < num_elements; ++i) {
        new (retval[i]) T(std::forward<ArgTypes>(args)...);
    }
    return retval;
}

template <typename T>
static inline void deallocate_raw_array(T* array_ptr, u64 num_elements)
{
    MemoryManager::deallocate_raw(array_ptr, num_elements * sizeof(T));
}

// allocates an uninitialized array
template <typename T>
static inline T* allocate_uarray(u64 num_elements)
{
    return casted_alloc<T>(sizeof(T) * num_elements);
}

template <typename T>
static inline T* allocate_raw_uarray(u64 num_elements)
{
    return casted_alloc_raw<T>(sizeof(T) * num_elements);
}

// deallocates array without calling destructors
template <typename T>
static inline void deallocate_uarray(T* array_ptr)
{
    MemoryManager::deallocate(array_ptr);
}

template <typename T>
static inline void deallocate_raw_uarray(T* array_ptr, u64 num_elements)
{
    MemoryManager::deallocate_raw(array_ptr, sizeof(T) * num_elements);
}

} /* end namespace allocators */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_ALLOCATORS_MEMORY_MANAGER_HPP_ */

//
// MemoryManager.hpp ends here
