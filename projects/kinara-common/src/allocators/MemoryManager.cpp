// MemoryManager.cpp ---
// Filename: MemoryManager.cpp
// Author: Abhishek Udupa
// Created: Fri Feb 13 19:29:04 2015 (-0500)
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

#include <cstdlib>

#include "MemoryManager.hpp"

namespace kinara {
namespace allocators {

u64 MemoryManager::s_total_bytes_allocated = (u64)0;
u64 MemoryManager::s_memory_allocation_limit = UINT64_MAX;
u64 MemoryManager::s_warn_watermark = UINT64_MAX;
u64 MemoryManager::s_peak_bytes_allocated = (u64)0;

OutOfMemoryError::OutOfMemoryError() noexcept
{
    // Nothing here
}

OutOfMemoryError::~OutOfMemoryError() noexcept
{
    // Nothing here
}

const char* OutOfMemoryError::what() const noexcept
{
    return "Memory limit exceeded or out of memory";
}

void* MemoryManager::allocate(u64 size)
{
    auto actual_size = size + (sizeof(u64));

    if (s_total_bytes_allocated + actual_size > s_memory_allocation_limit) {
        throw OutOfMemoryError();
    }
    s_total_bytes_allocated += actual_size;
    s_peak_bytes_allocated = (s_total_bytes_allocated > s_peak_bytes_allocated ?
                              s_total_bytes_allocated : s_peak_bytes_allocated);

    u64* block_ptr = reinterpret_cast<u64*>(malloc(size));
    *block_ptr = size;
    return (block_ptr + 1);
}

void* MemoryManager::allocate_cleared(u64 size)
{
    auto actual_size = size + (sizeof(u64));

    if (s_total_bytes_allocated + actual_size > s_memory_allocation_limit) {
        throw OutOfMemoryError();
    }

    s_total_bytes_allocated += actual_size;
    s_peak_bytes_allocated = (s_total_bytes_allocated > s_peak_bytes_allocated ?
                              s_total_bytes_allocated : s_peak_bytes_allocated);

    u64* block_ptr = reinterpret_cast<u64*>(calloc(size, 1));
    *block_ptr = size;
    return (block_ptr + 1);
}

// Raw allocation, we are guaranteed that the
// caller will deallocate_raw on this block.
// So we don't need to remember the size!
void* MemoryManager::allocate_raw(u64 size)
{
    if (s_total_bytes_allocated + size > s_memory_allocation_limit) {
        throw OutOfMemoryError();
    }

    s_total_bytes_allocated += size;
    s_peak_bytes_allocated = (s_total_bytes_allocated > s_peak_bytes_allocated ?
                              s_total_bytes_allocated : s_peak_bytes_allocated);
    return (malloc(size));
}

void* MemoryManager::allocate_raw_cleared(u64 size)
{
    s_total_bytes_allocated += size;
    s_peak_bytes_allocated = (s_total_bytes_allocated > s_peak_bytes_allocated ?
                              s_total_bytes_allocated : s_peak_bytes_allocated);
    return (calloc(size, 1));
}

void MemoryManager::deallocate(void* block_ptr)
{
    if (block_ptr == nullptr) {
        return;
    }
    u64* actual_block_ptr = (reinterpret_cast<u64*>(block_ptr) - 1);
    s_total_bytes_allocated -= *actual_block_ptr;
    free(actual_block_ptr);
}

void MemoryManager::set_allocation_limit(u64 allocation_limit)
{
    s_memory_allocation_limit = allocation_limit;
}

void MemoryManager::set_warn_watermark(u64 warn_watermark)
{
    s_warn_watermark = warn_watermark;
}

u64 MemoryManager::get_bytes_allocated()
{
    return s_total_bytes_allocated;
}

u64 MemoryManager::get_peak_bytes_allocated()
{
    return s_peak_bytes_allocated;
}

u64 MemoryManager::get_allocation_limit()
{
    return s_memory_allocation_limit;
}

u64 MemoryManager::get_warn_watermark()
{
    return s_warn_watermark;
}

bool MemoryManager::is_warn_watermark_reached()
{
    return (s_total_bytes_allocated >= s_warn_watermark);
}

bool MemoryManager::is_out_of_memory()
{
    return (s_total_bytes_allocated >= s_memory_allocation_limit);
}


} /* end namespace allocators */
} /* end namespace kinara */

//
// MemoryManager.cpp ends here
