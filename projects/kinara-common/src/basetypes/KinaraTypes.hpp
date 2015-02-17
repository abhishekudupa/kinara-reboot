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

#if !defined KINARA_KINARA_COMMON_KINARA_TYPES_HPP_
#define KINARA_KINARA_COMMON_KINARA_TYPES_HPP_

#include <ostream>
#include <type_traits>

#include "../memory/RefCountable.hpp"

namespace kinara {

// a base class for stringifiable objects
class Stringifiable
{
public:
    Stringifiable();
    virtual ~Stringifiable();

    virtual std::string to_string(u32 verbosity) const = 0;
    inline std::string to_string() const
    {
        return to_string(0);
    }
};

// A base class for Hashable objects
class Hashable
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

        inline HashValue(u64 hash_value)
            : m_hash_valid(true), m_hash_value(hash_value)
        {
            // Nothing here
        }

        inline bool is_valid() const
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
    Hashable();
    virtual ~Hashable();

    u64 hash() const
    {
        if (m_hash_value.is_valid()) {
            return m_hash_value.get_hash_value();
        } else {
            m_hash_value.set_hash_value(compute_hash_value());
            return m_hash_value.get_hash_value();
        }
    }

    virtual u64 compute_hash_value() const = 0;
    virtual u64 recompute_hash_value() const = 0;

    virtual bool equals(const Hashable& other) const = 0;
};

// Base class for comparable objects
class Comparable
{
public:
    Comparable();
    virtual ~Comparable();

    virtual i64 compare(const Comparable& other) const = 0;
    inline bool operator == (const Comparable& other) const
    {
        return (compare(other) == 0);
    }

    inline bool operator != (const Comparable& other) const
    {
        return (compare(other) != 0);
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

// Base class for "Constructible" objects
// these are essentially objects which can
// generate code that constructs a "semantically
// "equivalent" copy of themselves
class Constructible
{

};

// A function to be called to construct an object
template <typename T>
class ConstructFuncBase
{
public:
    template <typename... ArgTypes>
    inline void operator () (void* mem_ptr, ArgTypes&&... args) const
    {
        new (mem_ptr) T(std::forward<ArgTypes>(args)...);
    }
};

template <typename T>
class ConstructFuncBase<T*>
{
private:
    inline void construct(void* mem_ptr, const T* initializer,
                          const std::true_type& is_ref_counted) const
    {
        auto as_T = reinterpret_cast<const T**>(mem_ptr);
        *as_T = initializer;
        memory::inc_ref(**as_T);
    }

    inline void construct(void* mem_ptr, const T* initializer,
                          const std::false_type& is_ref_counted) const
    {
        auto as_T = reinterpret_cast<const T**>(mem_ptr);
        *as_T = initializer;
    }

public:
    inline void operator () (void* mem_ptr, const T* initializer) const
    {
        typename std::is_base_of<memory::RefCountable, T>::type is_ref_counted;
        construct(mem_ptr, initializer, is_ref_counted);
    }
};

template <typename T>
class ConstructFuncBase<const T*>
{
private:
    inline void construct(void* mem_ptr, const T* initializer,
                          const std::true_type& is_ref_counted) const
    {
        auto as_T = reinterpret_cast<const T**>(mem_ptr);
        *as_T = initializer;
        memory::inc_ref(**as_T);
    }

    inline void construct(void* mem_ptr, const T* initializer,
                          const std::false_type& is_ref_counted) const
    {
        auto as_T = reinterpret_cast<const T**>(mem_ptr);
        *as_T = initializer;
    }

public:
    inline void operator () (void* mem_ptr, const T* initializer) const
    {
        typename std::is_base_of<memory::RefCountable, T>::type is_ref_counted;
        construct(mem_ptr, initializer, is_ref_counted);
    }
};


// A destructor for various classes
template <typename T>
class DestructFuncBase
{
public:
    inline void operator () (const T& object) const
    {
        (&object)->~T();
    }
};

template <typename T>
class DestructFuncBase<T*>
{
private:
    inline void destruct (const T* object_ptr,
                          const std::true_type& is_ref_counted) const
    {
        memory::dec_ref(*object_ptr);
    }

    inline void destruct (const T* object_ptr,
                          const std::false_type& is_ref_counted) const
    {
        return;
    }

public:
    inline void operator () (const T* object_ptr) const
    {
        typename std::is_base_of<memory::RefCountable, T>::type is_ref_counted;
        destruct(object_ptr, is_ref_counted);
    }
};

template <typename T>
class DestructFuncBase<const T*>
{
private:
    inline void destruct (const T* object_ptr,
                          const std::true_type& is_ref_counted) const
    {
        memory::dec_ref(*object_ptr);
    }

    inline void destruct (const T* object_ptr,
                          const std::false_type& is_ref_counted) const
    {
        return;
    }

public:
    inline void operator () (const T* object_ptr) const
    {
        typename std::is_base_of<memory::RefCountable, T>::type is_ref_counted;
        destruct(object_ptr, is_ref_counted);
    }
};

template <typename T>
class NullDestructFunc
{
public:
    inline void operator () (const T& object) const
    {
        return;
    }
};


namespace kinara_print_detail_ {
template <typename T>
static inline void print_stringifiable(std::ostream& out_stream,
                                       const T& object,
                                       const std::true_type& is_stringifiable)
{
    out_stream << object.to_string();
}

template <typename T>
static inline void print_stringifiable(std::ostream& out_stream,
                                       const T& object,
                                       const std::false_type& is_stringifiable)
{
    out_stream << object;
}
} /* end namespace kinara_print_detail_ */

// redirect operators for stringifiable objects
template <typename T>
static inline std::ostream& operator << (std::ostream& out_stream, const T& object)
{
    typename std::is_base_of<Stringifiable, T>::type is_stringifiable_type_val;
    kinara_print_detail_::print_stringifiable_(out_stream, object, is_stringifiable_type_val);
    return out_stream;
}

} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_KINARA_TYPES_HPP_ */

//
// KinaraTypes.hpp ends here
