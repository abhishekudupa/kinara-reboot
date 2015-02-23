// StringBuffer.cpp ---
// Filename: StringBuffer.cpp
// Author: Abhishek Udupa
// Created: Mon Feb 23 01:04:47 2015 (-0500)
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

#include <string.h>

#include "../allocators/MemoryManager.hpp"

#include "StringBuffer.hpp"

namespace kinara {
namespace containers {

namespace ka = kinara::allocators;

StringBuffer::StringBuffer()
    : m_buffer(ka::casted_allocate_raw_cleared<char>(32)), m_length(0),
      m_capacity(31)
{
    // Nothing here
}

StringBuffer::~StringBuffer()
{
    ka::deallocate_raw(m_buffer, m_capacity + 1);
}

char* StringBuffer::data()
{
    return m_buffer;
}

const char* StringBuffer::data() const
{
    return m_buffer;
}

void StringBuffer::clear()
{
    ka::deallocate_raw(m_buffer, m_capacity + 1);
    m_buffer = ka::casted_allocate_raw_cleared<char>(32);
    m_length = 0;
    m_capacity = 31;
}

void StringBuffer::append(char c)
{
    if (m_length < m_capacity) {
        m_buffer[m_length++] = c;
        return;
    } else {
        auto new_capacity = ((m_capacity + 1) * 3) / 2;
        auto new_buffer = ka::casted_allocate_raw_cleared<char>(new_capacity);
        memcpy(new_buffer, m_buffer, m_length + 1);
        ka::deallocate_raw(m_buffer, m_capacity + 1);
        m_capacity = new_capacity - 1;
        m_buffer[m_length++] = c;
        return;
    }
}

void StringBuffer::append(const char *contents)
{
    auto length = strlen(contents);
    if (m_length + length < m_capacity) {
        memcpy(m_buffer + m_length, contents, length + 1);
        m_length += length;
        return;
    } else {
        auto new_capacity = m_length + length;
        new_capacity = (new_capacity * 3) / 2;
        auto new_buffer = ka::casted_allocate_raw_cleared<char>(new_capacity);
        memcpy(new_buffer, m_buffer, m_length + 1);
        ka::deallocate_raw(m_buffer, m_capacity + 1);
        m_capacity = new_capacity - 1;
        memcpy(m_buffer + m_length, contents, length + 1);
        m_length += length;
        return;
    }
}

} /* end namespace containers */
} /* end namespace kinara */

//
// StringBuffer.cpp ends here
