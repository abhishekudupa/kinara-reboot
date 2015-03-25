// PoolAllocator.cpp ---
// Filename: PoolAllocator.cpp
// Author: Abhishek Udupa
// Created: Sun Feb 22 23:03:52 2015 (-0500)
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

#include <functional>

#include "../basetypes/KinaraTypes.hpp"

#include "MemoryManager.hpp"
#include "PoolAllocator.hpp"

namespace kinara {
namespace allocators {

PoolAllocator::PoolAllocator(u32 object_size, u32 num_objects)
    : m_num_objects(num_objects), m_object_size(object_size),
      m_page_size(0), m_free_list(nullptr), m_chunk_list(nullptr),
      m_bytes_claimed(0), m_bytes_allocated(0)
{
    KINARA_ASSERT(object_size > 0);

    const u32 round_multiple = 1 << sc_alignment;
    const u32 remainder = m_object_size % round_multiple;
    if (remainder != 0) {
        m_object_size = m_object_size + (round_multiple - remainder);
    }

    m_page_size = m_object_size * m_num_objects + sc_chunk_overhead;
}

PoolAllocator::~PoolAllocator()
{
    reset();
}

inline u08* PoolAllocator::get_data_ptr_from_chunk_ptr(Chunk* chunk_ptr) const
{
    return
        (static_cast<u08*>(static_cast<void*>(chunk_ptr)) + sc_chunk_overhead);
}

void* PoolAllocator::allocate()
{
    std::less_equal<void*> less_func;
    if (m_free_list != nullptr) {
        auto block_ptr = m_free_list;
        void* retval = static_cast<void*>(block_ptr);
        m_free_list = block_ptr->m_next_block;
        m_bytes_allocated += m_object_size;
        return retval;
    }
    // no free blocks
    auto head_chunk = m_chunk_list;
    if (head_chunk != nullptr) {
        auto cur_chunk_ptr = head_chunk->get_cur_ptr();
        auto const has_space =
            less_func(cur_chunk_ptr + m_object_size,
                      head_chunk->get_end_ptr(m_page_size));

        if (has_space) {
            void* retval = static_cast<void*>(cur_chunk_ptr);
            head_chunk->m_current_ptr += m_object_size;
            m_bytes_allocated += m_object_size;
            return retval;
        }
    }

    // no free chunks either, allocate one
    auto new_chunk = new (allocate_raw(m_page_size)) Chunk(m_page_size, sc_chunk_overhead);
    m_bytes_claimed += m_page_size;
    new_chunk->m_next_chunk = m_chunk_list;
    m_chunk_list = new_chunk;

    void* retval = static_cast<void*>(new_chunk->m_current_ptr);
    new_chunk->m_current_ptr += m_object_size;
    m_bytes_allocated += m_object_size;
    return retval;
}

inline void PoolAllocator::check_duplicates(Block *block_ptr)
{
    for (auto block1 = block_ptr; block1 != nullptr; block1 = block1->m_next_block) {
        for (auto block2 = block1->m_next_block; block2 != nullptr; block2 = block2->m_next_block) {
            if (block1 == block2) {
                fprintf(stderr, "Error! Duplicate block found in free list!\n");
            }
        }
    }
}

void PoolAllocator::deallocate(void *void_ptr)
{
    if (void_ptr == nullptr) {
        return;
    }

    auto block_ptr = static_cast<Block*>(void_ptr);

    block_ptr->m_next_block = m_free_list;
    m_free_list = block_ptr;

    m_bytes_allocated -= m_object_size;
}

void PoolAllocator::reset()
{
    for (auto chunk = m_chunk_list; chunk != nullptr;) {
        auto next_chunk = chunk->m_next_chunk;
        deallocate_raw(chunk, m_page_size);
        chunk = next_chunk;
    }
    m_free_list = nullptr;
    m_chunk_list = nullptr;
    m_bytes_allocated = 0;
    m_bytes_claimed = 0;
}

void PoolAllocator::merge(PoolAllocator* other, bool collect_garbage)
{
    // we need the other pool to have the same
    // object size
    if (other->m_object_size != m_object_size) {
        throw KinaraException("Object sizes must match for pools to be merged");
    }

    // empty out the first chunk
    auto first_chunk_ptr = other->m_chunk_list->get_cur_ptr();
    auto first_chunk_end = other->m_chunk_list->get_end_ptr(other->m_page_size);
    while (first_chunk_ptr + m_object_size < first_chunk_end) {
        auto block_ptr = static_cast<Block*>(static_cast<void*>(first_chunk_ptr));
        block_ptr->m_next_block = m_free_list;
        m_free_list = block_ptr;
        first_chunk_ptr += m_object_size;
    }
    // Push these chunks after all of my chunks
    if (m_chunk_list == nullptr) {
        m_chunk_list = other->m_chunk_list;
    } else {
        auto my_last_chunk = m_chunk_list;
        auto my_cur_chunk = m_chunk_list;
        while (my_cur_chunk != nullptr) {
            my_last_chunk = my_cur_chunk;
            my_cur_chunk = my_cur_chunk->m_next_chunk;
        }
        my_last_chunk->m_next_chunk = other->m_chunk_list;
    }

    // merge all the free blocks
    if (m_free_list == nullptr) {
        m_free_list = other->m_free_list;
    } else {
        auto my_last_block = m_free_list;
        auto my_cur_block = m_free_list;
        while (my_cur_block != nullptr) {
            my_last_block = my_cur_block;
            my_cur_block = my_cur_block->m_next_block;
        }
        my_last_block->m_next_block = other->m_free_list;
    }

    // adjust sizes and such
    m_bytes_claimed += other->m_bytes_claimed;
    m_bytes_allocated += other->m_bytes_allocated;

    other->m_free_list = nullptr;
    other->m_chunk_list = nullptr;

    other->reset();

    if (collect_garbage) {
        garbage_collect();
    }
}

inline u64 PoolAllocator::count_blocks_in_range(void* range_low, void* range_high) const
{
    std::less<void*> less_func;
    std::less_equal<void*> less_equal_func;

    u64 retval = 0;
    for (auto block = m_free_list; block != nullptr; block = block->m_next_block) {
        if (less_equal_func(range_low, block) && less_func(block, range_high)) {
            ++retval;
        }
    }
    return retval;
}

inline void PoolAllocator::remove_blocks_in_range(void* range_low, void* range_high)
{
    std::less<void*> less_func;
    std::less_equal<void*> less_equal_func;

    auto block_ptr = m_free_list;
    auto prev_block_ptr = m_free_list;

    for (; block_ptr != nullptr; block_ptr = block_ptr->m_next_block) {
        if (less_equal_func(range_low, block_ptr) &&
            less_func(block_ptr, range_high)) {
            if (block_ptr == m_free_list) {
                m_free_list = block_ptr->m_next_block;
            } else {
                prev_block_ptr->m_next_block = block_ptr->m_next_block;
            }
            continue;
        } else {
            prev_block_ptr = block_ptr;
        }
    }
    return;
}

// releases pages which are completely free.
// NOT cheap!
void PoolAllocator::garbage_collect()
{
    auto chunk_ptr = m_chunk_list;
    auto prev_chunk_ptr = m_chunk_list;
    auto next_chunk_ptr = m_chunk_list;

    for (; chunk_ptr != nullptr; chunk_ptr = next_chunk_ptr) {
        next_chunk_ptr = chunk_ptr->m_next_chunk;

        auto chunk_data_low = get_data_ptr_from_chunk_ptr(chunk_ptr);
        auto chunk_data_high = chunk_data_low + (m_num_objects * m_object_size);
        auto free_blocks_in_chunk = count_blocks_in_range(chunk_data_low, chunk_data_high);

        if (free_blocks_in_chunk == m_num_objects) {
            remove_blocks_in_range(chunk_data_low, chunk_data_high);

            if (chunk_ptr == m_chunk_list) {
                m_chunk_list = next_chunk_ptr;
            } else {
                prev_chunk_ptr->m_next_chunk = next_chunk_ptr;
            }

            deallocate_raw(chunk_ptr, m_page_size);
            m_bytes_claimed -= m_page_size;

            continue;
        } else {
            prev_chunk_ptr = chunk_ptr;
        }
    }
    return;
}

u64 PoolAllocator::get_bytes_allocated() const
{
    return m_bytes_allocated;
}

u64 PoolAllocator::get_objects_allocated() const
{
    return (m_bytes_allocated / m_object_size);
}

u64 PoolAllocator::get_bytes_claimed() const
{
    return m_bytes_claimed;
}

u64 PoolAllocator::get_block_size() const
{
    return m_object_size;
}

u64 PoolAllocator::get_num_objects_at_once() const
{
    return m_num_objects;
}

} /* end namespace allocators */
} /* end namespace kinara */

//
// PoolAllocator.cpp ends here
