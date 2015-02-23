// StringBuffer.hpp ---
// Filename: StringBuffer.hpp
// Author: Abhishek Udupa
// Created: Mon Feb 23 00:55:12 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_STRING_BUFFER_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_STRING_BUFFER_HPP_

#include "../basetypes/KinaraBase.hpp"
#include "../basetypes/KinaraTypes.hpp"
#include <istream>

namespace kinara {
namespace containers {

class StringBuffer
{
private:
    char* m_buffer;
    u64 m_length;
    u64 m_capacity;

public:
    StringBuffer();
    StringBuffer(const StringBuffer& other) = delete;
    StringBuffer(StringBuffer&& other) = delete;
    inline StringBuffer& operator = (const StringBuffer& other) = delete;
    inline StringBuffer& operator = (StringBuffer&& other) = delete;

    ~StringBuffer();
    char* data();
    const char* data() const;
    void clear();

    void append(char c);
    void append(const char* contents);
};

static inline StringBuffer& operator << (StringBuffer& string_buffer,
                                         const char* contents)
{
    string_buffer.append(contents);
    return string_buffer;
}

static inline StringBuffer& operator << (StringBuffer& string_buffer,
                                         String& contents)
{
    string_buffer.append(contents.c_str());
    return string_buffer;
}

static inline std::istream& operator >> (std::istream& input_stream,
                                         StringBuffer& string_buffer)
{
    char local_buffer[256];
    u32 used_buffer = 0;
    do {
        input_stream.get(local_buffer[used_buffer++]);
        if (used_buffer == 255) {
            local_buffer[used_buffer] = (char)0;
            string_buffer.append(local_buffer);
            used_buffer = 0;
        }
    } while (local_buffer[used_buffer - 1] != ' ' &&
             local_buffer[used_buffer - 1] != '\n' &&
             local_buffer[used_buffer - 1] != '\t');

    --used_buffer;

    if (used_buffer == 0) {
        return input_stream;
    } else {
        local_buffer[used_buffer] = (char)0;
        string_buffer.append(local_buffer);
    }
}


} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_STRING_BUFFER_HPP_ */

//
// StringBuffer.hpp ends here
