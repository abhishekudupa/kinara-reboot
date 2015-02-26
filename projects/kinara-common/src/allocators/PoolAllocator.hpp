// PoolAllocator.hpp ---
// Filename: PoolAllocator.hpp
// Author: Abhishek Udupa
// Created: Sun Feb 22 22:45:54 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_ALLOCATORS_POOL_ALLOCATOR_HPP_
#define KINARA_KINARA_COMMON_ALLOCATORS_POOL_ALLOCATOR_HPP_

#include <new>

#include "../basetypes/KinaraBase.hpp"
#include "../basetypes/KinaraErrors.hpp"

namespace kinara {
namespace allocators {


// Like a small block allocator, but only one fixed size
// which is rounded up to the alignment factor
class PoolAllocator
{
private:
    static constexpr u32 sc_default_num_objects = 32;
    static constexpr u32 sc_alignment = 3;
    static constexpr u32 sc_chunk_overhead = (2 * sizeof(void*));

    // number of objects in a page of allocation
    u32 m_num_objects;
    u32 m_object_size;
    u32 m_page_size;

    struct Block
    {
        Block* m_next_block;

        inline Block()
            : m_next_block(nullptr)
        {
            // Nothing here
        }
    };

    struct Chunk
    {
        Chunk* m_next_chunk;
        u08* m_current_ptr;

        inline Chunk(u32 chunk_size, u32 chunk_overhead)
            : m_next_chunk(nullptr),
              m_current_ptr(static_cast<u08*>(static_cast<void*>(this)) + chunk_overhead)
        {
            // Nothing here
        }

        inline u08* get_cur_ptr() const
        {
            return m_current_ptr;
        }

        inline u08* get_end_ptr(u32 page_size) const
        {
            return (static_cast<u08*>(static_cast<void*>(const_cast<Chunk*>(this))) + page_size);
        }
    };

    Block* m_free_list;
    Chunk* m_chunk_list;
    u64 m_bytes_claimed;
    u64 m_bytes_allocated;

    inline u08* get_data_ptr_from_chunk_ptr(Chunk* chunk_ptr) const;
    inline u64 count_blocks_in_range(void* range_low, void* range_high) const;
    inline void remove_blocks_in_range(void* range_low, void* range_high);

public:
    PoolAllocator(u32 object_size, u32 num_objects = sc_default_num_objects);
    ~PoolAllocator();

    void* allocate();
    void deallocate(void* block_ptr);

    void reset();
    void garbage_collect();

    // merges the other pool allocator's
    // blocks, i.e., takes ownership
    // of the other pool allocator's memory
    // The other allocator is left as though
    // only just constructed
    void merge(PoolAllocator* other, bool collect_garbage = false);

    u64 get_bytes_allocated() const;
    u64 get_objects_allocated() const;
    u64 get_bytes_claimed() const;
    u64 get_block_size() const;
    u64 get_num_objects_at_once() const;
};

template <typename T, typename... ArgTypes>
static inline T* allocate(PoolAllocator& pool_allocator, ArgTypes&&... args)
{
    KINARA_ASSERT((sizeof(T) <= pool_allocator.get_block_size()));
    return new (pool_allocator.allocate()) T(std::forward<ArgTypes>(args)...);
}

template <typename T>
static inline void deallocate(PoolAllocator& pool_allocator,
                              const T* object_ptr)
{
    KINARA_ASSERT((sizeof(T) <= pool_allocator.get_block_size() &&
                   sizeof(T) + 8 > pool_allocator.get_block_size()));
    object_ptr->~T();
    pool_allocator.deallocate(const_cast<T*>(object_ptr));
}

} /* end namespace allocators */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_ALLOCATORS_POOL_ALLOCATOR_HPP_ */

//
// PoolAllocator.hpp ends here
