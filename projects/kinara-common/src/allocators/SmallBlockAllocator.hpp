// SmallBlockAllocator.hpp ---
// Filename: SmallBlockAllocator.hpp
// Author: Abhishek Udupa
// Created: Fri Feb 13 20:41:01 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_ALLOCATORS_SMALL_BLOCK_ALLOCATOR_HPP_
#define KINARA_KINARA_COMMON_ALLOCATORS_SMALL_BLOCK_ALLOCATOR_HPP_

#include "../basetypes/KinaraBase.hpp"
#include "../basetypes/KinaraErrors.hpp"

namespace kinara {
namespace allocators {

class SmallBlockAllocator
{
private:
    // preconfigured constants
    static constexpr u32 sc_page_size = 16384;
    static constexpr u32 sc_chunk_size = sc_page_size - (2 * sizeof(void*));
    static constexpr u32 sc_max_small_block_size = 256;
    // power of two to align blocks at
    static constexpr u32 sc_alignment = 3;
    static constexpr u32 sc_num_buckets = (sc_page_size >> sc_alignment);

    struct Chunk
    {
        Chunk* m_next_chunk;
        u08* m_current_ptr;
        u08 m_data[sc_chunk_size];

        inline Chunk()
            : m_next_chunk(nullptr), m_current_ptr(m_data)
        {
            // Nothing here
        }
    };

    struct BlockList
    {
        BlockList* m_next;
    };

    Chunk* m_chunks[sc_num_buckets];
    BlockList* m_free_lists[sc_num_buckets];
    u64 m_bytes_allocated;
    u64 m_bytes_claimed;

    inline void release_memory();
    inline u64 get_slot_index_for_size(u64 size) const;
    inline u64 count_blocks_in_range(u64 slot_index,
                                     void* range_low,
                                     void* range_high) const;
    inline void remove_blocks_in_range(u64 slot_index,
                                       void* range_low,
                                       void* range_high);

public:
    SmallBlockAllocator();
    ~SmallBlockAllocator();

    void reset();
    void* allocate(u64 size);
    void deallocate(void* block_ptr, u64 block_size);
    u64 get_bytes_allocated() const;
    u64 get_bytes_claimed() const;
    void garbage_collect();
};

} /* end namespace allocators */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_ALLOCATORS_SMALL_BLOCK_ALLOCATOR_HPP_ */

//
// SmallBlockAllocator.hpp ends here
