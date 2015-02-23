// String.hpp ---
// Filename: String.hpp
// Author: Abhishek Udupa
// Created: Tue Feb 17 23:39:46 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_STRING_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_STRING_HPP_

#include <istream>

namespace kinara {
namespace containers {

namespace string_detail_ {

// forward declaration
class StringRepr;

} /* end namespace string_detail_ */

class String
{
private:
    string_detail_::StringRepr* m_the_repr;

public:
    typedef const char* Iterator;
    typedef const char* iterator;

    String();
    String(const char* contents);
    String(const String& other);
    String(const String& other, u64 pos, u64 len = UINT64_MAX);
    String(const char* contents, u64 length);
    String(String&& other);
    String(u64 n, char c);
    ~String();

    String& operator = (const String& other);
    String& operator = (const char* contents);
    String& operator = (char c);
    String& operator = (String&& other);

    u64 size() const;
    u64 length() const;
    u64 max_size() const;

    void resize(u64 n);
    void resize(u64 n, char c);

    u64 capacity() const;
    void reserve(u64 n) const;
    void clear() noexcept;
    bool empty() const noexcept;
    void shrink_to_fit();

    char& operator [] (u64 index);
    const char& operator [] (u64 index) const;

    char& at (u64 index);
    const char& at (u64 index) const;

    char& back();
    const char& back() const;

    char& front();
    const char& front() const;

    String& operator += (const String& other);
    String& operator += (const char* contents);
    String& operator += (char c);

    String& append(const String& other);
    String& append(const String& other, u64 pos, u64 len);
    String& append(const char* contents);
    String& append(const char* contents, u64 len);
    String& append(u64 n, char c);

    void assign(const String& other);
    void assign(const String& other, u64 pos, u64 len);
    void assign(const char* contents);
    void assign(const char* contents, u64 len);
    void assign(u64 n, char c);
    void assign(String&& other);

    const char* c_str() const;

    u64 find(const String& other) const;
    u64 find(const char* contents) const;
    String substr(u64 pos = 0, u64 len = UINT64_MAX) const;
    i32 compare(const String& other) const;
    i32 compare(const char* contents) const;

    bool operator == (const String& other) const;
    bool operator == (const char* contents) const;
    bool operator != (const String& other) const;
    bool operator != (const char* contents) const;
    bool operator < (const String& other) const;
    bool operator < (const char* contents) const;
    bool operator <= (const String& other) const;
    bool operator <= (const char* contents) const;
    bool operator > (const String& other) const;
    bool operator > (const char* contents) const;
    bool operator >= (const String& other) const;
    bool operator >= (const char* contents) const;

    // additional functions not part of std::string
    bool starts_with(const String& other) const;
    bool starts_with(const char* contents) const;
    bool istarts_with(const String& other) const;
    bool istarts_with(const char* contents) const;

    bool ends_with(const String& other) const;
    bool ends_with(const char* contents) const;
    bool iends_with(const String& other) const;
    bool iends_with(const char* contents) const;

    void strip() const;
};

static inline std::ostream& operator << (std::ostream& out_stream,
                                         const String& the_string)
{
    out_stream << the_string.c_str();
    return out_stream;
}

static inline std::istream& operator >> (std::istream& in_stream, String& str)
{
    // TODO: Implement me
    return in_stream;
}

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_STRING_HPP_ */

//
// String.hpp ends here
