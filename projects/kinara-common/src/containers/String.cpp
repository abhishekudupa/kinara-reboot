// String.cpp ---
// Filename: String.cpp
// Author: Abhishek Udupa
// Created: Mon Feb 23 12:33:45 2015 (-0500)
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

#include <alloca.h>
#include <cctype>

#include "../allocators/MemoryManager.hpp"

#include "StringRepr.hpp"
#include "String.hpp"

namespace kinara {
namespace containers {

String::String()
    : m_the_repr(string_detail_::StringRepr::make_repr(nullptr))
{
    m_the_repr->inc_ref();
}

String::String(const char* contents)
    : m_the_repr(string_detail_::StringRepr::make_repr(contents))
{
    m_the_repr->inc_ref();
}

String::String(const char* contents, u64 len)
    : m_the_repr(string_detail_::StringRepr::make_repr(contents, len))
{
    m_the_repr->inc_ref();
}

String::String(const String& other)
    : m_the_repr(other.m_the_repr)
{
    m_the_repr->inc_ref();
}

String::String(const String& other, u64 pos, u64 len)
    : m_the_repr(string_detail_::StringRepr::make_repr(other.c_str() + pos,
                                                       std::min(other.length() - pos, len)))

{
    m_the_repr->inc_ref();
}

String::String(String&& other)
    : String()
{
    std::swap(m_the_repr, other.m_the_repr);
}

String::String(u64 n, char c)
{
    char* local_buffer;
    if (n <= sc_max_alloca) {
        local_buffer = static_cast<char*>(alloca(n));
        memset(local_buffer, c, n);
        m_the_repr = string_detail_::StringRepr::make_repr(local_buffer, n);
    } else {
        local_buffer = kinara::allocators::casted_allocate_raw<char>(n);
        memset(local_buffer, c, n);
        m_the_repr = string_detail_::StringRepr::make_repr(local_buffer, n);
        kinara::allocators::deallocate_raw(local_buffer, n);
    }
    m_the_repr->inc_ref();
}

String::~String()
{
    m_the_repr->dec_ref();
    m_the_repr = nullptr;
}

String& String::operator = (const String& other)
{
    return assign(other);
}

String& String::operator = (const char* contents)
{
    return assign(contents);
}

String& String::operator = (String&& other)
{
    return assign(std::move(other));
}

String& String::operator = (char c)
{
    return assign(1, c);
}

u64 String::size() const
{
    return m_the_repr->size();
}

u64 String::length() const
{
    return size();
}

u64 String::max_size() const
{
    return UINT64_MAX;
}

u64 String::capacity() const
{
    return size();
}

void String::clear() noexcept
{
    m_the_repr->dec_ref();

    m_the_repr = string_detail_::StringRepr::empty_string_repr();
    m_the_repr->inc_ref();
}

bool String::empty() const noexcept
{
    return (m_the_repr == string_detail_::StringRepr::empty_string_repr());
}

char String::operator [] (u64 index) const
{
    return ((m_the_repr->c_str())[index]);
}

char String::at(u64 index) const
{
    return (*this)[index];
}

char String::back() const
{
    return (*this)[length()];
}

char String::front() const
{
    return (*this)[0];
}

String& String::operator += (const String& other)
{
    return append(other);
}

String& String::operator += (const char* contents)
{
    return append(contents);
}

String& String::operator += (char c)
{
    return append(1, c);
}

String& String::append(const String& other)
{
    return append(other.c_str(), other.length());
}

String& String::append(const String& other, u64 pos, u64 len)
{
    return append(other.c_str() + pos, std::min(other.length() - pos, len));
}

String& String::append(const char* contents)
{
    return append(contents, strlen(contents));
}

String& String::append(const char* contents, u64 len)
{
    if (contents == nullptr || len == 0) {
        return *this;
    }

    auto old_repr = m_the_repr;
    auto old_length = length();
    auto other_length = len;
    auto new_length = old_length + other_length;
    char* local_buffer;
    bool using_alloca = false;

    if (new_length <= sc_max_alloca) {
        local_buffer = static_cast<char*>(alloca(new_length));
        using_alloca = true;
    } else {
        local_buffer = kinara::allocators::casted_allocate_raw<char>(new_length);
    }

    memcpy(local_buffer, c_str(), old_length);
    memcpy(local_buffer + old_length, contents, len);

    m_the_repr = string_detail_::StringRepr::make_repr(local_buffer, new_length);

    old_repr->dec_ref();
    m_the_repr->inc_ref();

    if (!using_alloca) {
        kinara::allocators::deallocate_raw(local_buffer, new_length);
    }
    return *this;
}

String& String::append(u64 n, char c)
{
    if (n == 0 || c == (char)0) {
        return *this;
    }

    auto old_repr = m_the_repr;
    auto old_length = length();
    auto other_length = n;
    auto new_length = old_length + other_length;
    char* local_buffer;
    bool using_alloca = false;

    if (new_length <= sc_max_alloca) {
        local_buffer = static_cast<char*>(alloca(new_length));
        using_alloca = true;
    } else {
        local_buffer = kinara::allocators::casted_allocate_raw<char>(new_length);
    }

    memcpy(local_buffer, c_str(), old_length);
    memset(local_buffer + old_length, c, other_length);

    m_the_repr = string_detail_::StringRepr::make_repr(local_buffer, new_length);

    old_repr->dec_ref();
    m_the_repr->inc_ref();

    if (!using_alloca) {
        kinara::allocators::deallocate_raw(local_buffer, new_length);
    }
    return *this;
}

String& String::assign(const String& other)
{
    if (&other == this) {
        return *this;
    }

    m_the_repr->dec_ref();
    m_the_repr = other.m_the_repr;
    m_the_repr->inc_ref();
    return *this;
}

String& String::assign(const String& other, u64 pos, u64 len)
{
    if (&other == this && pos == 0 && len >= length()) {
        return *this;
    }
    m_the_repr->dec_ref();
    m_the_repr = string_detail_::StringRepr::make_repr(other.c_str() + pos,
                                                       std::min(other.length() - pos, len));
    m_the_repr->inc_ref();
    return *this;
}

String& String::assign(const char* contents)
{
    m_the_repr->dec_ref();
    m_the_repr = string_detail_::StringRepr::make_repr(contents, strlen(contents));
    m_the_repr->inc_ref();
    return *this;
}

String& String::assign(const char* contents, u64 len)
{
    m_the_repr->dec_ref();
    m_the_repr = string_detail_::StringRepr::make_repr(contents, len);
    m_the_repr->inc_ref();
    return *this;
}

String& String::assign(u64 n, char c)
{
    char* local_buffer;
    m_the_repr->dec_ref();

    if (n <= sc_max_alloca) {
        local_buffer = static_cast<char*>(alloca(n));
    } else {
        local_buffer = kinara::allocators::casted_allocate_raw<char>(n);
    }
    memset(local_buffer, c, n);
    m_the_repr = string_detail_::StringRepr::make_repr(local_buffer, n);
    m_the_repr->inc_ref();
    if (n > sc_max_alloca) {
        kinara::allocators::deallocate_raw(local_buffer, n);
    }
    return *this;
}

String& String::assign(String&& other)
{
    std::swap(m_the_repr, other.m_the_repr);
    return *this;
}

const char* String::c_str() const
{
    return m_the_repr->c_str();
}

u64 String::find(const String& other) const
{
    return find(other.c_str());
}

inline bool String::match(u64 pos, const char* match, u64 len) const
{
    auto mystr = c_str();
    for (u64 i = 0; i < len; ++i) {
        if (mystr[pos+i] != match[i]) {
            return false;
        }
    }
    return true;
}

inline bool String::imatch(u64 pos, const char* match, u64 len) const
{
    auto mystr = c_str();
    for (u64 i = 0; i < len; ++i) {
        if (tolower(mystr[pos+i]) != tolower(match[i])) {
            return false;
        }
    }
    return true;
}

u64 String::find(const char* contents) const
{
    return find(contents, 0, strlen(contents));
}

u64 String::find(const char* contents, u64 pos, u64 len) const
{
    auto max_pos = length() - len;
    for (u64 i = 0; i < max_pos; ++i) {
        if (match(i, contents, len)) {
            return i;
        }
    }
    return UINT64_MAX;
}

String String::substr(u64 pos, u64 len) const
{
    if (pos >= length()) {
        return String("");
    }
    return String(c_str() + pos, std::min(length() - pos, len));
}

i32 String::compare(const String& other) const
{
    return (m_the_repr->compare(*(other.m_the_repr)));
}

i32 String::compare(const char *contents) const
{
    return (m_the_repr->compare(contents, strlen(contents)));
}

i32 String::compare(const char* contents, u64 len) const
{
    return (m_the_repr->compare(contents, len));
}

bool String::operator == (const String& other) const
{
    return (m_the_repr == other.m_the_repr);
}

bool String::operator == (const char* contents) const
{
    return (compare(contents) == 0);
}

bool String::operator != (const String& other) const
{
    return (m_the_repr != other.m_the_repr);
}

bool String::operator != (const char* contents) const
{
    return (compare(contents) != 0);
}

bool String::operator < (const String& other) const
{
    return (compare(other) < 0);
}

bool String::operator < (const char* contents) const
{
    return (compare(contents) < 0);
}

bool String::operator <= (const String& other) const
{
    return (compare(other) <= 0);
}

bool String::operator <= (const char* contents) const
{
    return (compare(contents) <= 0);
}

bool String::operator > (const String& other) const
{
    return (compare(other) > 0);
}

bool String::operator > (const char* contents) const
{
    return (compare(contents) > 0);
}

bool String::operator >= (const String& other) const
{
    return (compare(other) >= 0);
}

bool String::operator >= (const char* contents) const
{
    return (compare(contents) >= 0);
}

bool String::starts_with(const String& other) const
{
    auto other_length = other.length();
    if (other_length > length()) {
        return false;
    }
    return match(0, other.c_str(), other_length);
}

bool String::starts_with(const char* contents) const
{
    auto other_length = strlen(contents);
    if (other_length > length()) {
        return false;
    }
    return match(0, contents, other_length);
}

bool String::istarts_with(const String& other) const
{
    auto other_length = other.length();
    if (other_length > length()) {
        return false;
    }
    return imatch(0, other.c_str(), other_length);
}

bool String::istarts_with(const char* contents) const
{
    auto other_length = strlen(contents);
    if (other_length > length()) {
        return false;
    }
    return imatch(0, contents, strlen(contents));
}

bool String::ends_with(const String& other) const
{
    auto other_length = other.length();
    auto my_length = length();
    if (other_length > my_length) {
        return false;
    }
    return match(my_length - other_length, other.c_str(), other_length);
}

bool String::ends_with(const char* other) const
{
    auto other_length = strlen(other);
    auto my_length = length();
    if (other_length > my_length) {
        return false;
    }
    return match(my_length - other_length, other, other_length);
}

bool String::iends_with(const String& other) const
{
    auto other_length = other.length();
    auto my_length = length();
    if (other_length > my_length) {
        return false;
    }
    return imatch(my_length - other_length, other.c_str(), other_length);
}

bool String::iends_with(const char* other) const
{
    auto other_length = strlen(other);
    auto my_length = length();
    if (other_length > my_length) {
        return false;
    }
    return imatch(my_length - other_length, other, other_length);
}

String& String::to_lower()
{
    auto old_repr = m_the_repr;

    auto len = length();
    char* local_buffer;

    if (len <= sc_max_alloca) {
        local_buffer = static_cast<char*>(alloca(len));
    } else {
        local_buffer = kinara::allocators::casted_allocate_raw<char>(len);
    }

    memcpy(local_buffer, c_str(), len);
    for (u64 i = 0; i < len; ++i) {
        local_buffer[i] = tolower(local_buffer[i]);
    }
    m_the_repr = string_detail_::StringRepr::make_repr(local_buffer, len);
    old_repr->dec_ref();
    m_the_repr->inc_ref();

    if (len > sc_max_alloca) {
        kinara::allocators::deallocate_raw(local_buffer, len);
    }
    return *this;
}

String String::to_lower() const
{
    auto len = length();
    char* local_buffer;

    if (len <= sc_max_alloca) {
        local_buffer = static_cast<char*>(alloca(len));
    } else {
        local_buffer = kinara::allocators::casted_allocate_raw<char>(len);
    }

    memcpy(local_buffer, c_str(), len);
    for (u64 i = 0; i < len; ++i) {
        local_buffer[i] = tolower(local_buffer[i]);
    }

    auto retval = String(local_buffer, len);

    if (len > sc_max_alloca) {
        kinara::allocators::deallocate_raw(local_buffer, len);
    }
    return retval;
}

String& String::to_upper()
{
    auto old_repr = m_the_repr;

    auto len = length();
    char* local_buffer;

    if (len <= sc_max_alloca) {
        local_buffer = static_cast<char*>(alloca(len));
    } else {
        local_buffer = kinara::allocators::casted_allocate_raw<char>(len);
    }

    memcpy(local_buffer, c_str(), len);
    for (u64 i = 0; i < len; ++i) {
        local_buffer[i] = toupper(local_buffer[i]);
    }
    m_the_repr = string_detail_::StringRepr::make_repr(local_buffer, len);
    old_repr->dec_ref();
    m_the_repr->inc_ref();

    if (len > sc_max_alloca) {
        kinara::allocators::deallocate_raw(local_buffer, len);
    }
    return *this;
}

String String::to_upper() const
{
    auto len = length();
    char* local_buffer;

    if (len <= sc_max_alloca) {
        local_buffer = static_cast<char*>(alloca(len));
    } else {
        local_buffer = kinara::allocators::casted_allocate_raw<char>(len);
    }

    memcpy(local_buffer, c_str(), len);
    for (u64 i = 0; i < len; ++i) {
        local_buffer[i] = toupper(local_buffer[i]);
    }

    auto retval = String(local_buffer, len);

    if (len > sc_max_alloca) {
        kinara::allocators::deallocate_raw(local_buffer, len);
    }
    return retval;
}

String String::strip() const
{
    auto len = length();

    u64 begin_pos = 0;
    u64 end_pos = len;
    auto mystr = c_str();
    for (u64 i = 0; i < len; ++i) {
        if (mystr[i] == ' ' || mystr[i] == '\n' || mystr[i] == '\t') {
            begin_pos++;
        } else {
            break;
        }
    }

    for (i64 i = (i64)len; i >= 0; --i) {
        if (mystr[i] == ' ' || mystr[i] == '\n' || mystr[i] == '\t') {
            end_pos--;
        } else {
            break;
        }
    }

    return String(mystr + begin_pos, end_pos - begin_pos);
}

String& String::strip()
{
    auto len = length();

    u64 begin_pos = 0;
    u64 end_pos = len;
    auto mystr = c_str();
    for (u64 i = 0; i < len; ++i) {
        if (mystr[i] == ' ' || mystr[i] == '\n' || mystr[i] == '\t') {
            begin_pos++;
        } else {
            break;
        }
    }

    for (i64 i = (i64)len; i >= 0; --i) {
        if (mystr[i] == ' ' || mystr[i] == '\n' || mystr[i] == '\t') {
            end_pos--;
        } else {
            break;
        }
    }

    auto old_repr = m_the_repr;
    m_the_repr = string_detail_::StringRepr::make_repr(mystr + begin_pos, end_pos - begin_pos);
    old_repr->dec_ref();
    m_the_repr->inc_ref();

    return *this;
}

String add_strings_(const char* string1, const char* string2,
                    u64 length1, u64 length2)
{
    if (length1 == 0) {
        return String(string2, length2);
    } else if (length2 == 0) {
        return String(string1, length1);
    }
    u64 len = length1 + length2;
    char* local_buffer;
    if (len <= String::sc_max_alloca) {
        local_buffer = static_cast<char*>(alloca(len));
    } else {
        local_buffer = ka::casted_allocate_raw<char>(len);
    }
    memcpy(local_buffer, string1, length1);
    memcpy(local_buffer + length1, string2, length2);
    auto retval = String(local_buffer, len);
    if (len > String::sc_max_alloca) {
        ka::deallocate_raw(local_buffer, len);
    }
    return retval;
}

// overloaded friend functions
String operator + (const String& lhs, const String& rhs)
{
    return add_strings_(lhs.c_str(), rhs.c_str(), lhs.length(), rhs.length());
}

String operator + (const char* lhs, const String& rhs)
{
    return add_strings_(lhs, rhs.c_str(), strlen(lhs), rhs.length());
}

String operator + (const String& lhs, const char* rhs)
{
    return add_strings_(lhs.c_str(), rhs, lhs.length(), strlen(rhs));
}

String operator + (const String& lhs, char rhs)
{
    char buffer[1];
    buffer[0] = rhs;
    return add_strings_(lhs.c_str(), buffer, lhs.length(), 1);
}

String operator + (char lhs, const String& rhs)
{
    char buffer[1];
    buffer[0] = lhs;
    return add_strings_(buffer, rhs.c_str(), 1, rhs.length());
}

} /* end namespace containers */
} /* end namespace kinara */

//
// String.cpp ends here
