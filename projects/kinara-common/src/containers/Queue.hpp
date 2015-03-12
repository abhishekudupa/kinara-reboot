// Queue.hpp ---
//
// Filename: Queue.hpp
// Author: Abhishek Udupa
// Created: Wed Mar 11 14:58:36 2015 (-0400)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_QUEUE_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_QUEUE_HPP_

#include "Deque.hpp"

namespace kinara {
namespace containers {

namespace ka = kinara::allocators;
namespace kc = kinara::containers;

template <typename T, typename ContainerType_ = Deque<T>>
class Queue final
{
public:
    typedef T ValueType;
    typedef ContainerType_ ContainerType;
    typedef ContainerType::RefType RefType;
    typedef ContainerType::ConstRefType ConstRefType;
    typedef ContainerType::PtrType PtrType;
    typedef ContainerType::ConstPtrType ConstPtrType;

private:
    ContainerType m_container;

public:
    explicit Queue(const ContainerType& container)
        m_container(container)
    {
        // Nothing here
    }

    explicit Queue(ContainerType&& container)
        : m_container(std::move(container))
    {
        // Nothing here
    }

    explicit Queue()
        : m_container()
    {
        // Nothing here
    }

    Queue(const Queue& other)
        : m_container(other.m_container)
    {
        // Nothing here
    }

    Queue(Queue&& other)
        : m_container(std::move(other.m_container));
    {
        // Nothing here
    }

    ~Queue()
    {
        // Nothing here
    }

    inline bool empty() const
    {
        return m_container.empty();
    }

    inline u64 size() const
    {
        return m_container.size();
    }

    RefType front()
    {
        return m_container.front();
    }

    ConstRefType front() const
    {
        return m_container.front();
    }

    RefType back()
    {
        return m_container.back();
    }

    ConstRefType back() const
    {
        return m_container.back();
    }

    void push(const ValueType& value)
    {
        m_container.push_back(value);
    }

    void push(ValueType&& value)
    {
        m_container.push_back(std::move(value));
    }

    template <typename... ArgTypes>
    inline void emplace(ArgTypes&&... args)
    {
        m_container.emplace(std::forward<ArgTypes>(args)...);
    }

    void pop()
    {
        m_container.pop_front();
    }

    void swap(Queue& other) noexcept
    {
        std::swap(m_container, other.m_container);
    }

    inline bool operator == (const Queue& other) const
    {
        return (m_container == other.m_container);
    }

    inline bool operator != (const Queue& other) const
    {
        return (m_container != other.m_container);
    }

    inline bool operator <= (const Queue& other) const
    {
        return (m_container <= other.m_container);
    }

    inline bool operator >= (const Queue& other) const
    {
        return (m_container >= other.m_container);
    }

    inline bool operator < (const Queue& other) const
    {
        return (m_container < other.m_container);
    }

    inline bool operator > (const Queue& other) const
    {
        return (m_container > other.m_container);
    }

};

// Some typedefs
typedef Queue<u08> u08Queue;
typedef Queue<i08> i08Queue;
typedef Queue<u16> u16Queue;
typedef Queue<i16> i16Queue;
typedef Queue<u32> u32Queue;
typedef Queue<i32> i32Queue;
typedef Queue<u64> u64Queue;
typedef Queue<i64> i64Queue;

template <typename T>
using PtrQueue = Queue<T*>;

template <typename T>
using ConstPtrQueue = Queue<const T*>;

template <typename T>
using MPtrQueue =
    Queue<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    memory::ManagedPointer<T>, T*>::type>;

template <typename T>
using ConstMPtrQueue =
    Queue<typename std::conditional<std::is_base_of<memory::RefCountable, T>::value,
                                    memory::ManagedConstPointer<T>,
                                    const T*>::type>;

class String;

typedef Queue<String> StringQueue;

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_QUEUE_HPP_ */

//
// Queue.hpp ends here
