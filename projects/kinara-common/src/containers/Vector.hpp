// Vector.hpp ---
// Filename: Vector.hpp
// Author: Abhishek Udupa
// Created: Sat Feb 14 00:57:31 2015 (-0500)
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

// Custom vector class

#if !defined KINARA_KINARA_COMMON_CONTAINERS_VECTOR_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_VECTOR_HPP_

#include <cmath>
#include <initializer_list>

#include "../basetypes/KinaraBase.hpp"

#include "IteratorBase.hpp"

namespace kinara {
namespace containers {

// Some useful size increment functions

namespace detail {

template <u64 INCREMENT=8>
struct VectorAdditiveIncrementer
{
    inline u64 operator () (u64 current_size) const
    {
        return current_size + INCREMENT;
    }
};

template <u32 NUMERATOR, u32 DENOMINATOR>
struct VectorMultiplicativeIncrementer
{

    inline u64 operator () (u64 current_size) const
    {
        float factor = (float)NUMERATOR / (float)DENOMINATOR;
        return (u64)(ceil((float)current_size * factor));
    }
};

} /* end namespace detail */

// custom vector class. API compatible with
// the C++ stl vector, except for the allocators
// bit, which we tailor to use our allocator

template <typename T, typename Incrementer,
          bool CALLDESTRUCTORS=true>
class Vector
{
public:
    template <bool ISCONST>
    class Iterator :
        public IteratorBase<RandomAccessIteratorKind,
                            T, typename std::conditional<ISCONST, const T*, T*>::type,
                            typename std::conditional<ISCONST, T const&, T&>::type>
    {
    private:
        typedef IteratorBase<RandomAccessIteratorKind, T,
                             typename std::conditional<ISCONST, const T*, T*>::type,
                             typename std::conditional<ISCONST, T const&, T&>::type>
        IteratorBaseType;
        typedef typename IteratorBaseType::ValueType ValueType;
        typedef typename IteratorBaseType::PtrType PtrType;
        typedef typename IteratorBaseType::RefType RefType;
        typedef typename IteratorBaseType::DistanceType DistanceType;

        T* m_current_ptr;

        inline i64 compare(const Iterator& other) const
        {
            return (i64)(m_current_ptr - other.m_current_ptr);
        }

    public:
        inline Iterator()
            : m_current_ptr(nullptr)
        {
            // Nothing here
        }

        inline Iterator(const Iterator& other)
            : m_current_ptr(other.m_current_ptr)
        {
            // Nothing here
        }

        inline Iterator(T* current_ptr)
            : m_current_ptr(current_ptr)
        {
            // Nothing here
        }

        inline ~Iterator()
        {
            // Nothing here
        }

        inline Iterator& operator = (const Iterator& other)
        {
            if (&other == this) {
                return *this;
            }
            m_current_ptr = other.m_current_ptr;
            return *this;
        }

        inline Iterator& operator ++ ()
        {
            m_current_ptr++;
            return *this;
        }

        inline Iterator operator ++ (int unused)
        {
            auto retval = *this;
            m_current_ptr++;
            return retval;
        }

        inline Iterator& operator -- ()
        {
            m_current_ptr--;
            return *this;
        }

        inline Iterator operator -- (int unused)
        {
            auto retval = *this;
            m_current_ptr--;
            return retval;
        }

        inline Iterator operator + (i64 addend) const
        {
            auto retval = *this;
            retval.m_current_ptr += addend;
            return retval;
        }

        inline Iterator operator - (i64 addend) const
        {
            auto retval = *this;
            retval.m_current_ptr -= addend;
            return retval;
        }

        inline Iterator& operator += (i64 addend)
        {
            m_current_ptr += addend;
            return *this;
        }

        inline DistanceType operator - (const Iterator& other) const
        {
            return (m_current_ptr - other.m_current_ptr);
        }

        inline Iterator& operator -= (i64 addend)
        {
            m_current_ptr -= addend;
            return *this;
        }

        inline bool operator == (const Iterator& other) const
        {
            return (compare(other) == 0);
        }

        inline bool operator != (const Iterator& other) const
        {
            return (compare(other) != 0);
        }

        inline bool operator < (const Iterator& other) const
        {
            return (compare(other) < 0);
        }

        inline bool operator <= (const Iterator& other) const
        {
            return (compare(other) <= 0);
        }

        inline bool operator > (const Iterator& other) const
        {
            return (compare(other) > 0);
        }

        inline bool operator >= (const Iterator& other) const
        {
            return (compare(other) >= 0);
        }

        inline RefType operator * () const
        {
            return *m_current_ptr;
        }

        inline PtrType operator -> () const
        {
            return m_current_ptr;
        }

        inline RefType operator [] (i64 index) const
        {
            return m_current_ptr[index];
        }
    };

    typedef T ValueType;
    typedef Incrementer IncrementerFunction;

private:
    T* m_data;

public:
    explicit Vector();
    explicit Vector(u64 size);
    Vector(u64 size, const ValueType& value);
    template <typename InputIterator>
    Vector(const InputIterator& first, const InputIterator& last);
    Vector(const Vector& other);
    Vector(Vector&& other);
    Vector(std::initializer_list<ValueType> init_list);
    ~Vector();

    Vector& operator = (const Vector& other);
    Vector& operator = (Vector&& other);
    Vector& operator = (std::initializer_list<ValueType> init_list);


};

// reverse addition of iterator
template <typename T, typename Incrementer, bool ISCONST>
static inline typename Vector<T, Incrementer, ISCONST>::Iterator
operator + (i64 addend, const typename Vector<T, Incrementer, ISCONST>::Iterator& it)
{
    return (it + addend);
}

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_VECTOR_HPP_ */

//
// Vector.hpp ends here
