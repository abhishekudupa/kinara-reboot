// RefCountable.hpp ---
//
// Filename: RefCountable.hpp
// Author: Abhishek Udupa
// Created: Sun Jun 29 13:47:16 2014 (-0400)
//
//
// Copyright (c) 2015, Abhishek Udupa, University of Pennsylvania
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

// Basic ref countable type

#if !defined KINARA_MEMORY_REF_COUNTABLE_HPP_
#define KINARA_MEMORY_REF_COUNTABLE_HPP_

#include <type_traits>

#include "../basetypes/KinaraBase.hpp"

namespace kinara {
namespace memory {

class RefCountable;

class RefCountableDeleterBase
{
public:
    virtual void delete_ptr(const RefCountable* ref_countable_ptr) const = 0;
};

class RefCountable
{
private:
    mutable i64 m_ref_count_;
    const RefCountableDeleterBase* m_the_deleter;

public:
    inline RefCountable(const RefCountableDeleterBase* the_deleter = nullptr)
        : m_ref_count_((i64)0), m_the_deleter(the_deleter)
    {
        // Nothing here
    }

    virtual ~RefCountable()
    {
        // Nothing here
    }

    inline void inc_ref_() const
    {
        m_ref_count_++;
    }

    inline void dec_ref_() const
    {
        m_ref_count_--;
        if (m_ref_count_ <= 0) {
            if (m_the_deleter == nullptr) {
                delete this;
            } else {
                m_the_deleter->delete_ptr(this);
            }
        }
    }

    inline i64 get_ref_count_() const
    {
        return m_ref_count_;
    }
};

// manage ref count of a RefCountable pointer or reference
static inline void inc_ref_(const RefCountable* ptr,
                            std::true_type& unused)
{
    ptr->inc_ref_();
}

static inline void inc_ref_(const RefCountable& ref,
                            std::false_type& unused)
{
    ref.inc_ref_();
}

static inline void dec_ref_(const RefCountable* ptr,
                            std::true_type& unused)
{
    ptr->dec_ref_();
}

static inline void dec_ref_(const RefCountable& ref,
                            std::false_type& unused)
{
    ref.dec_ref_();
}

static inline i64 get_ref_count_(const RefCountable* ptr,
                                 std::true_type& unused)
{
    return ptr->get_ref_count_();
}

static inline i64 get_ref_count_(const RefCountable& ref,
                                 std::false_type& unused)
{
    return ref.get_ref_count_();
}

template <typename T>
static inline void inc_ref(const T& ref_countable_thing)
{
    typename std::is_pointer<T>::type is_pointer_type_val;
    inc_ref_(ref_countable_thing, is_pointer_type_val);
}

template <typename T>
static inline void dec_ref(const T& ref_countable_thing)
{
    typename std::is_pointer<T>::type is_pointer_type_val;
    dec_ref_(ref_countable_thing, is_pointer_type_val);
}

template <typename T>
static inline i64 get_ref_count(const T& ref_countable_thing)
{
    typename std::is_pointer<T>::type is_pointer_type_val;
    return get_ref_count_(ref_countable_thing, is_pointer_type_val);
}


} /* end namespace memory */
} /* end namespace kinara */

#endif /* KINARA_MEMORY_REF_COUNTABLE_HPP_ */

//
// RefCountable.hpp ends here
