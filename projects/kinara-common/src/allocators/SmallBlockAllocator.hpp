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
#include "MemoryManager.hpp"

namespace kinara {
namespace allocators {

class SmallBlockAllocator
{
private:
    // preconfigured constants
    static constexpr u32 PageSize = 8192;
    static constexpr u32 ChunkSize = PageSize - (2 * sizeof(void*));
    static constexpr u32 MaxSmallBlockSize = 256;
    static constexpr u32 Alignment = 3;
    static constexpr u32 NumBuckets = (MaxSmallBlockSize >> Alignment);

    struct Chunk
    {
        Chunk* m_next_chunk;
        u08* m_current_ptr;
        u08 m_data[ChunkSize];

        Chunk()
            : m_current_ptr(m_data)
        {
            // Nothing here
        }
    };

    Chunk* m_chunks[NumBuckets];
    u08* m_free_lists[NumBuckets];
    u64 m_bytes_allocated;

public:
    SmallBlockAllocator();
    ~SmallBlockAllocator();

    void reset();
    void* allocate(u64 size);
    void deallocate(void* block_ptr, u64 block_size);
    u64 get_bytes_allocated() const;
    u64 get_bytes_claimed() const;
    void consolidate();
};

static inline void* operator new (size_t size, SmallBlockAllocator& allocator)
{
    return allocator.allocate(size)
}

static inline void* operator new[] (size_t size, SmallBlockAllocator& allocator)
{
    return allocator.allocate(size);
}

static inline void operator delete(void* ptr, SmallBlockAllocator& allocator)
{
    __builtin_unreachable();
}

static inline void operator delete[](void* ptr, SmallBlockAllocator& allocator)
{
    __builtin_unreachable();
}

} /* end namespace allocators */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_ALLOCATORS_SMALL_BLOCK_ALLOCATOR_HPP_ */

//
// SmallBlockAllocator.hpp ends here
