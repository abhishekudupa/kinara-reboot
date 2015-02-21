// SmallBlockAllocator.cpp ---
// Filename: SmallBlockAllocator.cpp
// Author: Abhishek Udupa
// Created: Fri Feb 13 20:41:12 2015 (-0500)
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

#include <cstring>
#include <new> // for placement new
#include <functional> // for less, etc

#include "MemoryManager.hpp"
#include "SmallBlockAllocator.hpp"

namespace kinara {
namespace allocators {

SmallBlockAllocator::SmallBlockAllocator()
{
    // we begin with an empty set of chunks and free lists
    for (u32 i = 0; i < sc_num_buckets; ++i) {
        m_chunks[i] = nullptr;
        m_free_lists[i] = nullptr;
    }
    m_bytes_allocated = 0;
    m_bytes_claimed = 0;
}

SmallBlockAllocator::~SmallBlockAllocator()
{
    release_memory();
}

inline void SmallBlockAllocator::release_memory()
{
    for (u32 i = 0; i < sc_num_buckets; ++i) {
        auto cur_chunk = m_chunks[i];
        while(cur_chunk != nullptr) {
            auto next_chunk = cur_chunk->m_next_chunk;
            deallocate_raw(cur_chunk, sc_page_size);
            cur_chunk = next_chunk;
        }
        m_chunks[i] = nullptr;
        m_free_lists[i] = nullptr;
    }
}

void SmallBlockAllocator::reset()
{
    release_memory();
}

inline u64 SmallBlockAllocator::get_slot_index_for_size(u64 size) const
{
    return ((size - 1) >> sc_alignment);
}

void* SmallBlockAllocator::allocate(u64 size)
{
    if (size == 0) {
        return nullptr;
    }

    if (size > sc_max_small_block_size) {
        // just delegate this directly to the
        // memory manager. We're not in charge
        // of this block. When it comes back to us
        // we're just going to pass it back to the
        // memory manager
        return allocate_raw(size);
    }

    m_bytes_allocated += size;

    auto slot_index = get_slot_index_for_size(size);
    KINARA_ASSERT((slot_index < sc_num_buckets));
    if (m_free_lists[slot_index] != nullptr) {
        auto retval = m_free_lists[slot_index];
        m_free_lists[slot_index] = retval->m_next;
        return retval;
    }
    // no free block, check if we have something in
    // the chunks
    auto first_chunk = m_chunks[slot_index];
    if (first_chunk != nullptr) {
        auto new_current_ptr = first_chunk->m_current_ptr + size;
        if (new_current_ptr <= first_chunk->m_data + sc_chunk_size) {
            void* retval = first_chunk->m_current_ptr;
            first_chunk->m_current_ptr = new_current_ptr;
            return retval;
        }
    }
    // we need to allocate a new chunk
    Chunk* new_chunk = new (allocate_raw(sc_page_size)) Chunk();
    new_chunk->m_next_chunk = first_chunk;
    m_chunks[slot_index] = new_chunk;
    m_bytes_claimed += sc_page_size;

    auto retval = new_chunk->m_current_ptr;
    new_chunk->m_current_ptr += size;
    return retval;
}

void SmallBlockAllocator::deallocate(void *block_ptr, u64 block_size)
{
    if (block_size == 0 || block_ptr == nullptr) {
        return;
    }

    if (block_size > sc_max_small_block_size) {
        // I'm not managing this block, palm off to the
        // memory manager
        return deallocate_raw(block_ptr, block_size);
    }
    m_bytes_allocated -= block_size;

    auto slot_index = get_slot_index_for_size(block_size);
    auto block_ptr_as_block_list = static_cast<BlockList*>(block_ptr);
    block_ptr_as_block_list->m_next = m_free_lists[slot_index];
    m_free_lists[slot_index] = block_ptr_as_block_list;
}

u64 SmallBlockAllocator::get_bytes_allocated() const
{
    return m_bytes_allocated;
}

u64 SmallBlockAllocator::get_bytes_claimed() const
{
    return m_bytes_claimed;
}

inline u64 SmallBlockAllocator::count_blocks_in_range(u64 slot_index,
                                                      void *range_low,
                                                      void *range_high) const
{
    u64 retval = 0;
    auto block_ptr = m_free_lists[slot_index];
    std::less<void*> less_func;
    std::less_equal<void*> less_equal_func;

    for (; block_ptr != nullptr; block_ptr = block_ptr->m_next) {
        if (less_equal_func(range_low, block_ptr) && less_func(block_ptr, range_high)) {
            ++retval;
        }
    }
    return retval;
}

inline void SmallBlockAllocator::remove_blocks_in_range(u64 slot_index,
                                                        void *range_low,
                                                        void *range_high)
{
    auto block_ptr = m_free_lists[slot_index];
    auto head_ptr = m_free_lists[slot_index];
    auto prev_block_ptr = block_ptr;
    std::less<void*> less_func;
    std::less_equal<void*> less_equal_func;

    for (; block_ptr != nullptr; block_ptr = block_ptr->m_next) {
        if (less_equal_func(range_low, block_ptr) && less_func(block_ptr, range_high)) {
            if (block_ptr == head_ptr) {
                m_free_lists[slot_index] = block_ptr->m_next;
                head_ptr = m_free_lists[slot_index];
            } else {
                prev_block_ptr->m_next = block_ptr->m_next;
            }
            continue;
        }
        prev_block_ptr = block_ptr;
    }
    return;
}

// Returns completely empty chunks back to the
// memory manager
void SmallBlockAllocator::garbage_collect()
{
    for (u64 i = 0; i < sc_num_buckets; ++i) {
        auto slot_size = (i + 1) << sc_alignment;
        auto head_ptr = m_chunks[i];
        auto chunk_ptr = m_chunks[i];
        auto prev_chunk_ptr = m_chunks[i];
        for (; chunk_ptr != nullptr; chunk_ptr = chunk_ptr->m_next_chunk) {
            auto num_blocks_in_chunk =
                (chunk_ptr->m_current_ptr - chunk_ptr->m_data) / slot_size;
            auto num_free_blocks_in_chunk =
                count_blocks_in_range(i, chunk_ptr->m_data, chunk_ptr->m_current_ptr);
            if (num_blocks_in_chunk == num_free_blocks_in_chunk) {
                remove_blocks_in_range(i, chunk_ptr->m_data, chunk_ptr->m_current_ptr);

                // return this chunk to the memory manager
                deallocate_raw(chunk_ptr, sc_page_size);
                m_bytes_claimed -= sc_page_size;

                if (chunk_ptr == head_ptr) {
                    m_chunks[i] = chunk_ptr->m_next_chunk;
                    head_ptr = chunk_ptr->m_next_chunk;
                } else {
                    prev_chunk_ptr->m_next_chunk = chunk_ptr->m_next_chunk;
                }
                continue;
            }
            prev_chunk_ptr = chunk_ptr;
        }
    }
}

} /* end namespace allocators */
} /* end namespace kinara */

//
// SmallBlockAllocator.cpp ends here
