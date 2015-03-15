// MultiWayHeap.hpp ---
//
// Filename: MultiWayHeap.hpp
// Author: Abhishek Udupa
// Created: Fri Mar 13 16:45:59 2015 (-0400)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_MULTI_WAY_HEAP_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_MULTI_WAY_HEAP_HPP_

#include "Vector.hpp"

namespace kinara {
namespace containers {
namespace multiway_heap_detail_ {

template <typename T, typename Comparator, u32 ARITY>
class MultiWayHeap
{
    static_assert(ARITY >= 2, "ARITY of multiway heap must be at least 2");

private:
    Vector<T> m_data;

    inline u64 get_first_child(u64 index) const __attribute__ ((__always_inline__))
    {
        return (index * ARITY) + 1;
    }

    inline u64 get_parent(u64 index) const __attribute__ ((__always_inline__))
    {
        return (index - 1) / ARITY;
    }

public:
    inline MultiWayHeap()
        : m_data()
    {
        // Nothing here
    }

    inline MultiWayHeap(const MultiWayHeap& other)
        : m_data(other.m_data)
    {
        // Nothing here
    }

    inline MultiWayHeap(MultiWayHeap&& other)
        : m_data()
    {
        std::swap(m_data, other.m_data);
    }

    inline ~MultiWayHeap()
    {
        // Nothing here
    }

    inline MultiWayHeap& operator = (const MultiWayHeap& other)
    {
        if (&other == this) {
            return *this;
        }
        m_data = other.m_data;
        return *this;
    }

    inline MultiWayHeap& operator = (MultiWayHeap&& other)
    {
        if (&other == this) {
            return *this;
        }
        std::swap(m_data, other.m_data);
        return *this;
    }

    inline u64 get_size() const
    {
        return (m_data.size());
    }

    inline const T& get_min() const
    {
        return m_data[0];
    }

    inline void delete_min()
    {
        u64 hole = 0;
        u64 child0 = get_first_child(hole);
        u64 min_index = child0;

        auto orig_size = get_size();

        Comparator comparator;

        while (min_index < orig_size) {

            for (u64 i = child0 + 1, last = std::min(orig_size - 1, child0 + ARITY);
                 i < last; ++i) {
                if (comparator(m_data[i], m_data[min_index])) {
                    min_index = i;
                }
            }

            m_data[hole] = m_data[min_index];

            hole = min_index;
            child0 = get_first_child(hole);
            min_index = child0;
        }

        // we now have an empty slot at m_data[hole].
        // move m_data[orig_size - 1] to this place and
        // bubble up
        auto const& bubble = m_data[orig_size - 1];
        auto parent = get_parent(hole);

        while (hole != 0 && comparator(bubble, m_data[parent])) {
            m_data[hole] = m_data[parent];
            hole = parent;
            parent = get_parent(hole);
        }

        // finally move the bubble into the hole
        m_data[hole] = bubble;
        m_data.pop_back();
    }

    inline void insert(const T& new_elem)
    {
        Comparator comparator;

        u64 hole = m_data.size();
        m_data.push_back(new_elem);

        if (hole == 0) {
            return;
        }

        u64 parent = get_parent(hole);

        while (hole != 0 && comparator(new_elem, m_data[parent])) {
            m_data[hole] = m_data[parent];
            hole = parent;
            parent = get_parent(hole);
        }

        m_data[hole] = new_elem;
    }
};

// We specialize (partially) for the case where ARITY = 2, because the
// get_parent and get_first_child can be done much more effectively
// with shifts

template <typename T, typename Comparator>
class MultiWayHeap<T, Comparator, 2>
{
private:
    Vector<T> m_data;

    inline u64 get_first_child(u64 index) __attribute__ ((__always_inline__))
    {
        return ((index << 1) | 1);
    }

    inline u64 get_parent(u64 index) __attribute__ ((__always_inline__))
    {
        return (index - 1) >> 1;
    }

public:
    inline MultiWayHeap()
        : m_data()
    {
        // Nothing here
    }

    inline MultiWayHeap(const MultiWayHeap& other)
        : m_data(other.m_data)
    {
        // Nothing here
    }

    inline MultiWayHeap(MultiWayHeap&& other)
        : m_data()
    {
        std::swap(m_data, other.m_data);
    }

    inline ~MultiWayHeap()
    {
        // Nothing here
    }

    inline MultiWayHeap& operator = (const MultiWayHeap& other)
    {
        if (&other == this) {
            return *this;
        }
        m_data = other.m_data;
        return *this;
    }

    inline MultiWayHeap& operator = (MultiWayHeap&& other)
    {
        if (&other == this) {
            return *this;
        }
        std::swap(m_data, other.m_data);
        return *this;
    }

    inline u64 get_size() const
    {
        return m_data.size();
    }

    inline const T& get_min() const
    {
        return m_data[0];
    }

    inline void delete_min()
    {
        u64 hole = 0;
        u64 child0 = get_first_child(hole);
        u64 child1 = child0 + 1;
        auto orig_size = get_size();

        Comparator comparator;
        u64 min_index = child0;

        while (min_index + 1 < orig_size) {
            if (comparator(m_data[child1], m_data[child0])) {
                min_index = child1;
            }
            m_data[hole] = m_data[min_index];
            hole = min_index;
            child0 = get_first_child(hole);
            child1 = child0 + 1;
            min_index = child0;
        }

        auto const& bubble = m_data[orig_size - 1];
        auto parent = get_parent(hole);

        while (comparator(bubble, m_data[parent]) && hole != 0) {
            m_data[hole] = m_data[parent];
            hole = parent;
            parent = get_parent(hole);
        }

        m_data[hole] = bubble;
        m_data.pop_back();
    }

    inline void insert(const T& new_elem)
    {
        Comparator comparator;

        u64 hole = m_data.size();
        m_data.push_back(new_elem);

        if (hole == 0) {
            return;
        }

        u64 parent = get_parent(hole);

        while (comparator(new_elem, m_data[parent]) && hole != 0) {
            m_data[hole] = m_data[parent];
            hole = parent;
            parent = get_parent(hole);
        }

        m_data[hole] = new_elem;
    }
};

} /* end namespace multiway_heap_detail_ */

template <typename T, typename Comparator, u32 ARITY>
using MultiWayHeap = multiway_heap_detail_::MultiWayHeap<T, Comparator, ARITY>;

template <typename T, typename Comparator = std::less<T> >
using BinaryHeap = multiway_heap_detail_::MultiWayHeap<T, Comparator, 2>;

template <typename T, typename Comparator = std::less<T> >
using TernaryHeap = multiway_heap_detail_::MultiWayHeap<T, Comparator, 3>;

template <typename T, typename Comparator = std::less<T> >
using QuaternaryHeap = multiway_heap_detail_::MultiWayHeap<T, Comparator, 4>;

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_MULTI_WAY_HEAP_HPP_ */

//
// MultiWayHeap.hpp ends here
