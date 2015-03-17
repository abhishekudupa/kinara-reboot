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

inline u64& MemoryManager::total_bytes_allocated()
{
    static u64 s_total_bytes_allocated = (u64)0;
    return s_total_bytes_allocated;
}

inline u64& MemoryManager::memory_allocation_limit()
{
    static u64 s_memory_allocation_limit = (u64)UINT64_MAX;
    return s_memory_allocation_limit;
}

inline u64& MemoryManager::warn_watermark()
{
    static u64 s_warn_watermark = (u64)UINT64_MAX;
    return s_warn_watermark;
}

inline u64& MemoryManager::peak_bytes_allocated()
{
    static u64 s_peak_bytes_allocated = (u64)0;
    return s_peak_bytes_allocated;
}

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
    if (size == 0) {
        return nullptr;
    }
    auto actual_size = size + sc_block_header_size;

    auto& total = total_bytes_allocated();
    total += actual_size;
    auto& peak = peak_bytes_allocated();
    auto& limit = memory_allocation_limit();

    if (total + actual_size > limit) {
        throw OutOfMemoryError();
    }

    peak = (total > peak ? total : peak);


    u64* block_ptr = static_cast<u64*>(malloc(actual_size));
    if (block_ptr == nullptr) {
        throw OutOfMemoryError();
    }
    *block_ptr = size;
    return (block_ptr + 1);
}

void* MemoryManager::allocate_cleared(u64 size)
{
    if (size == 0) {
        return nullptr;
    }
    auto actual_size = size + sc_block_header_size;
    auto& total = total_bytes_allocated();
    auto& peak = peak_bytes_allocated();
    auto limit = memory_allocation_limit();

    if (total + actual_size > limit) {
        throw OutOfMemoryError();
    }

    total += actual_size;
    peak = (total > peak ? total : peak);

    u64* block_ptr = static_cast<u64*>(calloc(actual_size, 1));
    if (block_ptr == nullptr) {
        throw OutOfMemoryError();
    }
    *block_ptr = size;
    return (block_ptr + 1);
}

// Raw allocation, we are guaranteed that the
// caller will deallocate_raw on this block.
// So we don't need to remember the size!
void* MemoryManager::allocate_raw(u64 size)
{
    if (size == 0) {
        return nullptr;
    }

    auto& total = total_bytes_allocated();
    auto& peak = peak_bytes_allocated();
    auto& limit = memory_allocation_limit();

    if (total + size > limit) {
        throw OutOfMemoryError();
    }

    total += size;
    peak = (total > peak ? total : peak);
    auto retval = malloc(size);
    if (retval == nullptr) {
        throw OutOfMemoryError();
    }
    return retval;
}

void* MemoryManager::allocate_raw_cleared(u64 size)
{
    if (size == 0) {
        return nullptr;
    }


    auto& total = total_bytes_allocated();
    auto& peak = peak_bytes_allocated();
    auto& limit = memory_allocation_limit();

    if (total + size > limit) {
        throw OutOfMemoryError();
    }

    total += size;
    peak = (total > peak ? total : peak);

    auto retval = calloc(size, 1);
    if (retval == nullptr) {
        throw OutOfMemoryError();
    }
    return retval;
}

void MemoryManager::deallocate(const void* block_ptr)
{
    if (block_ptr == nullptr) {
        return;
    }

    auto actual_block_ptr = (static_cast<const u64*>(block_ptr) - 1);
    total_bytes_allocated() -= *actual_block_ptr;
    free(const_cast<u64*>(actual_block_ptr));
}

void MemoryManager::deallocate_raw(const void* block_ptr, u64 size)
{
    if (block_ptr == nullptr) {
        return;
    }
    total_bytes_allocated() -= size;
    free(const_cast<void*>(block_ptr));
}

void MemoryManager::set_allocation_limit(u64 allocation_limit)
{
    memory_allocation_limit() = allocation_limit;
}

void MemoryManager::set_warn_watermark(u64 new_warn_watermark)
{
    warn_watermark() = new_warn_watermark;
}

u64 MemoryManager::get_bytes_allocated()
{
    return total_bytes_allocated();
}

u64 MemoryManager::get_peak_bytes_allocated()
{
    return peak_bytes_allocated();
}

u64 MemoryManager::get_allocation_limit()
{
    return memory_allocation_limit();
}

u64 MemoryManager::get_warn_watermark()
{
    return warn_watermark();
}

bool MemoryManager::is_warn_watermark_reached()
{
    return (total_bytes_allocated() >= warn_watermark());
}

bool MemoryManager::is_out_of_memory()
{
    return (total_bytes_allocated() >= memory_allocation_limit());
}

} /* end namespace allocators */
} /* end namespace kinara */

// override the new and delete operators
// to use kinara managed memory

void* operator new(std::size_t count)
{
    return kinara::allocators::allocate(count);
}

void operator delete(void* block_ptr) noexcept
{
    kinara::allocators::deallocate(block_ptr);
}

//
// MemoryManager.cpp ends here
