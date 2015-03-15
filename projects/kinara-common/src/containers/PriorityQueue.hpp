// PriorityQueue.hpp ---
//
// Filename: PriorityQueue.hpp
// Author: Abhishek Udupa
// Created: Fri Mar 13 19:07:49 2015 (-0400)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_PRIORITY_QUEUE_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_PRIORITY_QUEUE_HPP_

#include <utility>

#include "HeapCommon.hpp"
#include "MultiWayHeap.hpp"

namespace kinara {
namespace containers {

namespace ka = kinara::allocators;

template <typename T, typename Comparator = std::less<T>,
          typename HeapType = BinaryHeap<T, Comparator> >
class PriorityQueue final
{
private:
    HeapType m_heap;

public:
    inline PriorityQueue()
        : m_heap()
    {
        // Nothing here
    }

    template <typename InputIterator>
    inline PriorityQueue(const InputIterator& first, const InputIterator& last)
        : PriorityQueue()
    {
        for (auto it = first; it != last; ++it) {
            m_heap.insert(*it);
        }
    }

    inline PriorityQueue(PriorityQueue&& other)
        : m_heap()
    {
        std::swap(other.m_heap, m_heap);
    }

    inline ~PriorityQueue()
    {
        // Nothing here
    }

    inline bool empty() const
    {
        return (m_heap.get_size() == 0);
    }

    inline u64 size() const
    {
        return m_heap.get_size();
    }

    inline const T& top() const
    {
        return m_heap.get_min();
    }

    inline void push(const T& value)
    {
        return m_heap.insert(value);
    }

    inline void pop()
    {
        m_heap.delete_min();
    }

    inline void swap(PriorityQueue& other) noexcept
    {
        std::swap(m_heap, other.m_heap);
    }
};

namespace priority_queue_detail_ {

template <typename Key, typename Value, typename Comparator = std::less<Key> >
class PairComparator
{
public:
    typedef std::pair<Key, Value> PairType;

    inline bool operator () (const PairType& pair1, const PairType& pair2) const
    {
        Comparator comparator;
        return comparator(pair1.first, pair2.first);
    }
};

} /* end namespace priority_queue_detail_ */

template <typename Value>
using u32PriorityQueue = PriorityQueue<std::pair<u32, Value>,
                                       priority_queue_detail_::PairComparator<u32, Value> >;

template <typename Value>
using i32PriorityQueue = PriorityQueue<std::pair<i32, Value>,
                                       priority_queue_detail_::PairComparator<i32, Value> >;

template <typename Value>
using u64PriorityQueue = PriorityQueue<std::pair<u64, Value>,
                                       priority_queue_detail_::PairComparator<u64, Value> >;

template <typename Value>
using i64PriorityQueue = PriorityQueue<std::pair<i64, Value>,
                                       priority_queue_detail_::PairComparator<i64, Value> >;

template <typename Value>
using floatPriorityQueue = PriorityQueue<std::pair<float, Value>,
                                         priority_queue_detail_::PairComparator<float, Value> >;

template <typename Value>
using doublePriorityQueue = PriorityQueue<std::pair<double, Value>,
                                          priority_queue_detail_::PairComparator<double, Value> >;


} /* end namespace kinara */
} /* end namespace containers */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_PRIORITY_QUEUE_HPP_ */

//
// PriorityQueue.hpp ends here
