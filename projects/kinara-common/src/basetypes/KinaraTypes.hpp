// KinaraTypes.hpp ---
// Filename: KinaraTypes.hpp
// Author: Abhishek Udupa
// Created: Fri Feb 13 19:00:04 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_BASETYPES_KINARA_TYPES_HPP_
#define KINARA_KINARA_COMMON_BASETYPES_KINARA_TYPES_HPP_

#include <ostream>
#include <type_traits>
#include <string>

#include "../memory/RefCountable.hpp"
#include "../memory/ManagedPointer.hpp"

namespace kinara {

// A base class for objects
class KinaraObject
{
    static void* operator new(std::size_t sz);
    static void* operator new[](std::size_t count);
    static void operator delete(void* ptr, std::size_t sz);
    static void operator delete[](void* ptr, std::size_t sz);
    virtual ~KinaraObject();
};

class StringifiableEBC
{
    // Nothing here
};

// a base class for stringifiable objects
template <typename DerivedClass>
class Stringifiable : public StringifiableEBC
{
public:
    inline std::string to_string(u32 verbosity) const
    {
        return static_cast<const DerivedClass*>(this)->to_string(verbosity);
    }

    inline std::string to_string() const
    {
        return static_cast<const DerivedClass*>(this)->to_string();
    }
};

class HashableEBC
{
    // Nothing here
};

// A base class for Hashable objects
template <typename DerivedClass>
class Hashable : public HashableEBC
{
private:
    class HashValue
    {
    private:
        mutable bool m_hash_valid : 1;
        mutable u64 m_hash_value : 63;

    public:
        inline HashValue()
            : m_hash_valid(false), m_hash_value(0)
        {
            // Nothing here
        }

        inline HashValue(const HashValue& other)
            : m_hash_valid(other.m_hash_valid),
              m_hash_value(other.m_hash_value)
        {
            // Nothing here
        }

        inline HashValue& operator = (const HashValue& other)
        {
            if (&other == this) {
                return *this;
            }
            m_hash_valid = other.m_hash_valid;
            m_hash_value = other.m_hash_value;
            return *this;
        }

        inline HashValue(u64 hash_value)
            : m_hash_valid(true), m_hash_value(hash_value)
        {
            // Nothing here
        }

        inline bool is_hash_valid() const
        {
            return m_hash_valid;
        }

        inline u64 get_hash_value() const
        {
            return m_hash_value;
        }

        inline void set_hash_value(u64 hash_value) const
        {
            m_hash_valid = true;
            m_hash_value = false;
        }

        inline void clear_hash_value() const
        {
            m_hash_valid = false;
            m_hash_value = (u64)0;
        }
    };

    HashValue m_hash_value;

public:
    inline Hashable()
        : m_hash_value()
    {
        // Nothing here
    }

    inline Hashable(const Hashable& other)
        : m_hash_value(other.m_hash_value)
    {
        // Nothing here
    }

    inline Hashable& operator = (const Hashable& other)
    {
        if (&other == this) {
            return *this;
        }
        m_hash_value = other.m_hash_value;
        return *this;
    }

    inline u64 hash() const
    {
        if (m_hash_value.is_valid()) {
            return m_hash_value.get_hash_value();
        } else {
            auto this_as_derived = static_cast<const DerivedClass*>(this);
            m_hash_value.set_hash_value(this_as_derived->compute_hash_value());
            return m_hash_value.get_hash_value();
        }
    }

    inline void invalidate_hash_value() const
    {
        m_hash_value.clear_hash_value();
    }
};

class ComparableEBC
{
    // Nothing here
};

// Base class for comparable objects
template <typename DerivedClass>
class Comparable : public ComparableEBC
{
public:
    i64 compare(const Comparable& other) const
    {
        return static_cast<const DerivedClass*>(this)->compare_with(other);
    }

    bool equals(const Comparable& other)
    {
        auto other_as_derived = dynamic_cast<const DerivedClass*>(&other);
        if (other_as_derived == nullptr) {
            return false;
        }
        return static_cast<const DerivedClass*>(this)->equal_to(*other_as_derived);
    }

    inline bool operator == (const Comparable& other) const
    {
        return (equals(other));
    }

    inline bool operator != (const Comparable& other) const
    {
        return (!equals(other));
    }

    inline bool operator < (const Comparable& other) const
    {
        return (compare(other) < 0);
    }

    inline bool operator <= (const Comparable& other) const
    {
        return (compare(other) <= 0);
    }

    inline bool operator > (const Comparable& other) const
    {
        return (compare(other) > 0);
    }

    inline bool operator >= (const Comparable& other) const
    {
        return (compare(other) >= 0);
    }
};

class Interruptible
{
protected:
    mutable volatile bool m_interrupted;

public:
    inline Interruptible()
        : m_interrupted(false)
    {
        // Nothing here
    }

    inline void interrupt() const
    {
        m_interrupted = true;
    }

    inline void reset_interrupted() const
    {
        m_interrupted = false;
    }
};

// Base class for "Constructible" objects
// these are essentially objects which can
// generate code that constructs a "semantically
// "equivalent" copy of themselves
class Constructible
{

};

class KinaraException : public std::exception
{
private:
    std::string m_exception_info;

public:
    KinaraException()
        : m_exception_info("No information about exception")
    {
        // Nothing here
    }

    KinaraException(const std::string& exception_info) noexcept
        : m_exception_info(exception_info)
    {
        // Nothing here
    }

    virtual ~KinaraException() noexcept
    {
        // Nothing here
    }

    inline KinaraException& operator = (const KinaraException& other) noexcept
    {
        if (&other == this) {
            return *this;
        }
        m_exception_info = other.m_exception_info;
        return *this;
    }

    virtual const char* what() const noexcept override
    {
        return m_exception_info.c_str();
    }
};

namespace stringification_detail_ {

template <typename T>
inline std::string to_string_(const T& object,
                              std::true_type is_stringifiable)
{
    return object.to_string();
}

template <typename T>
inline std::string to_string_(const T& object,
                              std::false_type is_stringifiable)
{
    return std::to_string(object);
}

template <typename T>
inline std::string to_string_(const T& object)
{
    typename std::is_base_of<StringifiableEBC, T>::value is_stringifiable;
    return to_string_(object, is_stringifiable);
}


} /* end namespace stringification_detail_ */

template <typename T>
inline std::string to_string(const T& object)
{
    return stringification_detail_::to_string_(object);
}

} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_BASETYPES_KINARA_TYPES_HPP_ */

//
// KinaraTypes.hpp ends here
