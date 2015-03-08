// DequeTypes.hpp ---
//
// Filename: DequeTypes.hpp
// Author: Abhishek Udupa
// Created: Tue Mar  3 17:46:22 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_DEQUE_TYPES_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_DEQUE_TYPES_HPP_

#include <iterator>
#include <cstring>

#include "../basetypes/KinaraBase.hpp"
#include "../allocators/MemoryManager.hpp"
#include "../basetypes/KinaraErrors.hpp"

#include "ContainersBase.hpp"

namespace kinara {
namespace containers {

template <typename T, typename ConstructFunc, typename DestructFunc>
class DequeBase;

namespace deque_detail_ {

namespace kc = kinara::containers;
namespace ka = kinara::allocators;

// A class representing a block in a deque
// this class does not construct objects in manner
template <typename T>
class DequeBlock final
{
private:
    // configuration for how many elems go
    // into a block
    static constexpr u64 sc_num_elems_per_block = 64;

    T m_object_array[sc_num_elems_per_block];

public:
    inline DequeBlock() = delete;
    inline DequeBlock(const DequeBlock& other) = delete;
    inline DequeBlock(DequeBlock&& other) = delete;
    inline DequeBlock& operator = (const DequeBlock& other) = delete;
    inline DequeBlock& operator = (DequeBlock&& other) = delete;

    static inline DequeBlock* construct(void* mem_ptr)
    {
        return static_cast<DequeBlock*>(mem_ptr);
    }

    inline ~DequeBlock()
    {
        // Nothing here
    }

    inline T* get_begin() const
    {
        return const_cast<T*>(m_object_array);
    }

    inline T* get_end() const
    {
        return (const_cast<T*>(m_object_array) + sc_num_elems_per_block);
    }

    static inline u64 get_block_size()
    {
        return sc_num_elems_per_block;
    }
};

// forward declaration of deque implementation
template <typename T, typename ConstructFunc, typename DestructFunc>
class DequeInternal;

// iterator class for deques
template <typename T, typename ConstructFunc,
          typename DestructFunc, bool ISCONST>
class IteratorBase :
        public std::iterator<std::random_access_iterator_tag, T, i64,
                             typename std::conditional<ISCONST, const T*, T*>::type,
                             typename std::conditional<ISCONST, const T&, T&>::type>
{
    friend class kc::deque_detail_::IteratorBase<T, ConstructFunc, DestructFunc, true>;
    friend class kc::deque_detail_::IteratorBase<T, ConstructFunc, DestructFunc, false>;
    friend class kc::deque_detail_::DequeInternal<T, ConstructFunc, DestructFunc>;
    friend class kc::DequeBase<T, ConstructFunc, DestructFunc>;

private:
    typedef typename std::conditional<ISCONST, const T*, T*>::type ValPtrType;
    typedef typename std::conditional<ISCONST, const T&, T&>::type ValRefType;
    typedef DequeBlock<T> BlockType;

    T* m_current;
    BlockType** m_block_array_ptr;

    // helper functions
    inline u64 get_offset_from_begin() const
    {
        return (m_current - (*m_block_array_ptr)->get_begin());
    }

    inline u64 get_offset_from_end() const
    {
        return (*(m_block_array_ptr)->get_end() - m_current);
    }

public:
    inline IteratorBase()
        : m_current(nullptr), m_block_array_ptr(nullptr)
    {
        // Nothing here
    }

    inline IteratorBase(T* current, BlockType** block_array_ptr)
        : m_current(current), m_block_array_ptr(block_array_ptr)
    {
        // Nothing here
    }

    inline IteratorBase(const IteratorBase& other)
        : m_current(other.m_current), m_block_array_ptr(other.m_block_array_ptr)
    {
        // Nothing here
    }

    template <bool OISCONST>
    inline IteratorBase(const kc::deque_detail_::IteratorBase<T, ConstructFunc,
                                                              DestructFunc, OISCONST>& other)
        : m_current(other.m_current), m_block_array_ptr(other.m_block_array_ptr)
    {
        static_assert(!OISCONST || ISCONST,
                      "Cannot construct non-const iterator from const iterator");
    }

    inline ~IteratorBase()
    {
        // Nothing here
    }

    inline IteratorBase& operator = (const IteratorBase& other)
    {
        if (&other == this) {
            return *this;
        }
        m_current = other.m_current;
        m_block_array_ptr = other.m_block_array_ptr;
        return *this;
    }

    template <bool OISCONST>
    inline IteratorBase&
    operator = (const kc::deque_detail_::IteratorBase<T, ConstructFunc,
                                                      DestructFunc, OISCONST>& other)
    {
        static_assert(!OISCONST || ISCONST,
                      "Cannot construct non-const iterator from const iterator");

        if (&other == this) {
            return *this;
        }
        m_current = other.m_current;
        m_block_array_ptr = other.m_block_array_ptr;
        return *this;
    }

    template <bool OISCONST>
    inline bool
    operator == (const kc::deque_detail_::IteratorBase<T, ConstructFunc,
                                                       DestructFunc, OISCONST>& other) const
    {
        return (m_current == other.m_current);
    }

    template <bool OISCONST>
    inline bool
    operator != (const kc::deque_detail_::IteratorBase<T, ConstructFunc,
                                                       DestructFunc, OISCONST>& other) const
    {
        return (!((*this) == other));
    }

    inline IteratorBase& operator ++ ()
    {
        auto block_end = (*m_block_array_ptr)->get_end();
        if (m_current + 1 >= block_end) {
            ++m_block_array_ptr;
            m_current = (*m_block_array_ptr)->get_begin();
        } else {
            ++m_current;
        }
        return *this;
    }

    inline IteratorBase operator ++ (int unused)
    {
        auto retval = *this;
        ++(*this);
        return retval;
    }

    inline ValRefType operator * () const
    {
        return *m_current;
    }

    inline ValPtrType operator -> () const
    {
        return m_current;
    }

    inline IteratorBase& operator -- ()
    {
        auto block_begin = (*m_block_array_ptr)->get_begin();
        if (m_current == block_begin) {
            --m_block_array_ptr;
            m_current = (*m_block_array_ptr)->get_end() - 1;
        } else {
            --m_current;
        }
        return *this;
    }

    inline IteratorBase operator -- (int unused)
    {
        auto retval = *this;
        --(*this);
        return retval;
    }

    inline IteratorBase operator + (i64 n) const
    {
        if (n == 0) {
            return *this;
        }
        if (n < 0) {
            return ((*this) - (-n));
        }
        auto retval = *this;
        retval += n;
        return retval;
    }

    inline IteratorBase operator - (i64 n) const
    {
        if (n == 0) {
            return *this;
        }
        if (n < 0) {
            return ((*this) + (-n));
        }
        auto retval = *this;
        retval -= n;
        return retval;
    }

    inline i64 operator - (const IteratorBase& other) const
    {
        i64 retval = (get_offset_from_begin() - other.get_offset_from_begin());
        retval += ((m_block_array_ptr - other.m_block_array_ptr) * BlockType::get_block_size());
        return retval;
    }

    template <bool OISCONST>
    inline i64
    operator - (const kc::deque_detail_::IteratorBase<T, ConstructFunc,
                                                      DestructFunc, OISCONST>& other) const
    {
        i64 retval = (get_offset_from_begin() - other.get_offset_from_begin());
        retval += ((m_block_array_ptr - other.m_block_array_ptr) * BlockType::get_block_size());
        return retval;
    }

    template <bool OISCONST>
    inline bool
    operator < (const kc::deque_detail_::IteratorBase<T, ConstructFunc,
                                                      DestructFunc, OISCONST>& other) const
    {
        return (((*this) - other) < 0);
    }

    template <bool OISCONST>
    inline bool
    operator <= (const kc::deque_detail_::IteratorBase<T, ConstructFunc,
                                                       DestructFunc, OISCONST>& other) const
    {
        return (((*this) - other) <= 0);
    }

    template <bool OISCONST>
    inline bool
    operator > (const kc::deque_detail_::IteratorBase<T, ConstructFunc,
                                                      DestructFunc, OISCONST>& other) const
    {
        return (((*this) - other) > 0);
    }

    template <bool OISCONST>
    inline bool
    operator >= (const kc::deque_detail_::IteratorBase<T, ConstructFunc,
                                                       DestructFunc, OISCONST>& other) const
    {
        return (((*this) - other) >= 0);
    }

    IteratorBase& operator += (i64 n)
    {
        if (n == 0) {
            return *this;
        }
        if (n < 0) {
            return ((*this) -= (-n));
        }

        u64 new_offset = n + (m_current - (*m_block_array_ptr)->get_begin());
        const u64 block_size = BlockType::get_block_size();

        if (new_offset < block_size) {
            m_current += n;
        } else {
            u64 num_blocks_to_advance = new_offset / block_size;
            m_block_array_ptr += num_blocks_to_advance;
            m_current =
                (*m_block_array_ptr)->get_begin() +
                (new_offset - (num_blocks_to_advance * block_size));
        }
        return *this;
    }

    IteratorBase& operator -= (i64 n)
    {
        if (n == 0) {
            return *this;
        }
        if (n < 0) {
            return ((*this) += (-n));
        }

        i64 new_offset = (m_current - (*m_block_array_ptr)->get_begin()) - n;
        const u64 block_size = BlockType::get_block_size();

        if (new_offset >= 0) {
            m_current -= n;
        } else {
            u64 num_blocks_to_retreat = 1 + ((- new_offset - 1) / block_size);

            KINARA_ASSERT(num_blocks_to_retreat != 0);

            m_block_array_ptr -= num_blocks_to_retreat;
            m_current =
                (*m_block_array_ptr)->get_begin() +
                ((num_blocks_to_retreat * block_size) + new_offset);
        }
        return *this;
    }

    ValRefType operator [] (i64 index) const
    {
        auto new_iterator = (*this) + index;
        return *new_iterator;
    }
};


// handle memory management (not initialization) for deques
template <typename T, typename ConstructFunc, typename DestructFunc>
class DequeInternal
{
private:
    typedef T ValueType;
    typedef T* PtrType;
    typedef T& RefType;
    typedef const T* ConstPtrType;
    typedef const T& ConstRefType;
    typedef DequeBlock<T> BlockType;
    typedef BlockType* BlockPtrType;
    typedef const BlockType* BlockConstPtrType;
    typedef IteratorBase<T, ConstructFunc, DestructFunc, false> Iterator;
    typedef IteratorBase<T, ConstructFunc, DestructFunc, true> ConstIterator;

protected:
    BlockPtrType* m_block_array;
    u64 m_block_array_size;
    Iterator m_start;
    Iterator m_finish;

    static constexpr u64 sc_initial_block_array_size = 8;

    inline void create_blocks(BlockPtrType* block_array_begin, BlockPtrType* block_array_end)
    {
        for (auto cur_ptr = block_array_begin; cur_ptr != block_array_end; ++cur_ptr) {
            *cur_ptr = BlockType::construct(ka::allocate_raw(sizeof(BlockType)));
        }
    }

    inline void destroy_blocks(BlockPtrType* block_array_begin, BlockPtrType* block_array_end)
    {
        for (auto cur_ptr = block_array_begin; cur_ptr != block_array_end; ++cur_ptr) {
            (*cur_ptr)->~BlockType();
            ka::deallocate_raw(*cur_ptr, sizeof(BlockType));
            *cur_ptr = nullptr;
        }
    }

    inline u64 get_size() const
    {
        return (m_finish - m_start);
    }

    // expands the block array towards the front
    // all iterators are invalidated!
    // of course, we fix up the m_start and m_finish
    // iterators
    // this routine only ensures that num_blocks
    // pointers are available at the front,
    // the actual blocks are not allocated
    inline void expand_block_array(u64 num_blocks, bool expand_at_front = true)
    {
        if (num_blocks == 0) {
            return;
        }

        BlockPtrType* new_block_array_start = nullptr;
        auto old_num_blocks = ((m_finish.m_block_array_ptr - m_start.m_block_array_ptr) + 1);
        auto new_num_blocks = old_num_blocks + num_blocks;
        if (m_block_array_size > (2 * new_num_blocks)) {
            new_block_array_start =
                m_block_array + ((m_block_array_size - new_num_blocks) / 2);
            if (expand_at_front) {
                new_block_array_start += num_blocks;
            }
            memmove(new_block_array_start, m_start.m_block_array_ptr,
                    old_num_blocks * sizeof(BlockPtrType));
        } else {
            // need to reallocate
            auto new_block_array_size = m_block_array_size +
                std::max(m_block_array_size, new_num_blocks) + 2;
            auto new_block_array =
                ka::casted_allocate_raw_cleared<BlockPtrType>(sizeof(BlockPtrType) *
                                                              new_block_array_size);
            new_block_array_start =
                new_block_array + ((new_block_array_size - new_num_blocks) / 2);
            if (expand_at_front) {
                new_block_array_start += num_blocks;
            }
            memmove(new_block_array_start, m_start.m_block_array_ptr,
                    old_num_blocks * sizeof(BlockPtrType));
            ka::deallocate_raw(m_block_array, sizeof(BlockPtrType) * m_block_array_size);
            m_block_array = new_block_array;
            m_block_array_size = new_block_array_size;
        }

        // fixup the start and finish iterators
        m_start.m_block_array_ptr = new_block_array_start;
        m_finish.m_block_array_ptr = (new_block_array_start + old_num_blocks - 1);
    }

    inline void expand_block_array_at_front(u64 num_blocks)
    {
        expand_block_array(num_blocks);
    }

    inline void expand_block_array_at_back(u64 num_blocks)
    {
        expand_block_array(num_blocks, false);
    }

    // ensure there's room for at least one more element in the front
    // fixes up m_start as if an additional element was added
    inline void expand_towards_front()
    {
        auto space_left_in_start_block =
            m_start.m_current - ((*(m_start.m_block_array_ptr))->get_begin());
        if (space_left_in_start_block >= 1) {
            --m_start;
            return;
        } else {
            expand_block_array_at_front(1);
            auto new_start_block = m_start.m_block_array_ptr - 1;
            create_blocks(new_start_block, m_start.m_block_array_ptr);
            --m_start;
            return;
        }
    }

    // Same as expand_towards_front, except that we fixup m_finish
    inline void expand_towards_back()
    {
        auto end_of_finish_block = ((*(m_finish.m_block_array_ptr))->get_end());
        auto space_left_in_finish_block = end_of_finish_block - m_finish.m_current;
        if (space_left_in_finish_block > 1) {
            ++m_finish;
            return;
        } else {
            expand_block_array_at_back(1);
            auto new_finish_block = m_finish.m_block_array_ptr + 1;
            create_blocks(m_finish.m_block_array_ptr + 1, new_finish_block + 1);
            ++m_finish;
            return;
        }
    }

    inline void expand_towards_front(u64 n)
    {
        auto const block_size = BlockType::get_block_size();
        auto space_left_in_start_block =
            m_start.m_current - ((*(m_start.m_block_array_ptr))->get_begin());
        if ((u64)space_left_in_start_block >= n) {
            m_start -= n;
            return;
        } else {
            // need to allocate more blocks
            auto num_blocks_to_create =
                1 + ((n - space_left_in_start_block) / block_size);
            expand_block_array_at_front(num_blocks_to_create);
            auto new_start_block = m_start.m_block_array_ptr - num_blocks_to_create;
            create_blocks(new_start_block, m_start.m_block_array_ptr);
            m_start -= n;
            return;
        }
    }

    inline void expand_towards_back(u64 n)
    {
        auto const block_size = BlockType::get_block_size();
        auto end_of_finish_block = (*(m_finish.m_block_array_ptr))->get_end();
        auto space_left_in_finish_block = end_of_finish_block - m_finish.m_current;
        if ((u64)space_left_in_finish_block > n) {
            m_finish += n;
            return;
        } else {
            auto num_blocks_to_create =
                1 + ((n - space_left_in_finish_block) / block_size);
            expand_block_array_at_back(num_blocks_to_create);
            auto new_finish_block = m_finish.m_block_array_ptr + num_blocks_to_create;
            create_blocks(m_finish.m_block_array_ptr + 1, new_finish_block + 1);
            m_finish += n;
            return;
        }
    }

    // Expands back or forward as required
    inline void expand_at_position(const ConstIterator& position, u64 n)
    {
        auto offset_from_start = position - m_start;
        auto offset_from_finish = m_finish - position;
        if (offset_from_start < offset_from_finish) {
            expand_towards_front(position, n);
        } else {
            expand_towards_back(position, n);
        }
    }

    // creates a hole of size n (uninitialized storage)
    // just before position
    // returns an iterator to the first location in the hole
    inline Iterator make_hole_at_position(const ConstIterator& position,
                                          u64 hole_size)
    {
        auto offset_from_start = position - m_start;
        auto offset_from_finish = m_finish - position;
        if (offset_from_start < offset_from_finish) {
            expand_towards_front(hole_size);
            // move objects in the original [ start, position ) hole_size elements up
            Iterator destination = m_start;
            Iterator source = destination + hole_size;
            auto num_elements = (u64)offset_from_start;
            move_objects(destination, source, num_elements);
            return (destination + num_elements);
        } else {
            expand_towards_back(hole_size);
            // move objects in the original [position, finish) hole_size elements down
            Iterator destination = m_finish - offset_from_finish;
            Iterator source = destination - hole_size;
            auto num_elements = (u64)offset_from_finish;
            move_objects(destination, source, num_elements);
            return source;
        }
    }

    // moves num_elements objects from the locations starting at source
    // to the corresponding locations starting at destination
    inline void move_objects_backward(const Iterator& destination,
                                      const ConstIterator& source,
                                      u64 num_elements)
    {
        auto from = source + num_elements;
        auto to = destination + num_elements;

        u64 num_left_to_move = num_elements;

        while (num_left_to_move > 0) {
            auto from_block_ptr = from.m_block_array_ptr;
            auto objs_left_in_from_block = from.m_current - (*from_block_ptr)->get_begin();
            auto to_block_ptr = to.m_block_array_ptr;
            auto objs_left_in_to_block = to.m_current - (*to_block_ptr)->get_begin();

            objs_left_in_from_block = std::min((u64)objs_left_in_from_block, num_left_to_move);
            objs_left_in_to_block = std::min((u64)objs_left_in_to_block, num_left_to_move);

            u64 num_to_move = 0;
            if (objs_left_in_from_block < objs_left_in_to_block) {
                num_to_move = objs_left_in_from_block;
            } else {
                num_to_move = objs_left_in_to_block;
            }

            memmove(to.m_current - num_to_move,
                    from.m_current - num_to_move,
                    sizeof(T) * num_to_move);

            from -= num_to_move;
            to -= num_to_move;

            if (from.m_current == (*from.m_block_array_ptr)->get_begin()) {
                --from.m_block_array_ptr;
                from.m_current = (*from.m_block_array_ptr)->get_end();
            }
            if (to.m_current == (*to.m_block_array_ptr)->get_begin()) {
                --to.m_block_array_ptr;
                to.m_current = (*to.m_block_array_ptr)->get_end();
            }

            num_left_to_move -= num_to_move;
        }
    }

    inline void move_objects_forward(const Iterator& destination,
                                     const ConstIterator& source,
                                     u64 num_elements)
    {
        auto from = source;
        auto to = destination;

        u64 num_left_to_move = num_elements;

        while (num_left_to_move > 0) {
            auto from_block_ptr = from.m_block_array_ptr;
            auto objs_left_in_from_block = (*from_block_ptr)->get_end() - from.m_current;
            auto to_block_ptr = to.m_block_array_ptr;
            auto objs_left_in_to_block = (*to_block_ptr)->get_end() - to.m_current;

            objs_left_in_to_block = std::min((u64)objs_left_in_to_block, num_left_to_move);
            objs_left_in_from_block = std::min((u64)objs_left_in_from_block, num_left_to_move);

            u64 num_to_move = 0;
            if (objs_left_in_from_block < objs_left_in_to_block) {
                num_to_move = objs_left_in_from_block;
            } else {
                num_to_move = objs_left_in_to_block;
            }

            memmove(to.m_current, from.m_current, sizeof(T) * num_to_move);

            from += num_to_move;
            to += num_to_move;
            num_left_to_move -= num_to_move;
        }
    }

    inline void move_objects(const Iterator& destination,
                             const ConstIterator& source,
                             u64 num_elements)
    {
        // if moving the range backwards, then prefer
        // moving objects from beginning to end
        if (destination < source) {
            move_objects_forward(destination, source, num_elements);
        } else {
            move_objects_backward(destination, source, num_elements);
        }
    }

    // shrink by discarding elements AFTER position
    inline void shrink_after(ConstIterator position, bool strict = false)
    {
        if (position == m_finish) {
            return;
        }

        auto new_finish = position + 1;

        for (auto block_ptr = new_finish.m_block_array_ptr + 1;
             block_ptr != m_finish.m_block_array_ptr + 1;
             ++block_ptr) {
            --m_block_array_size;
            ka::deallocate_raw(*block_ptr, sizeof(BlockType));
            *block_ptr = nullptr;
        }

        m_finish.m_block_array_ptr = position.m_block_array_ptr;
        m_finish.m_current = position.m_current;

        compact(strict);
    }

    inline void compact(bool strict = false)
    {
        for (auto block_ptr = m_start.m_block_array_ptr - 1;
             block_ptr != m_block_array - 1;
             --block_ptr) {
            if (*block_ptr != nullptr) {
                (*block_ptr)->~BlockType();
                ka::deallocate_raw(*block_ptr, sizeof(BlockType));
                (*block_ptr) = nullptr;
            } else {
                break;
            }
        }
        for (auto block_ptr = m_finish.m_block_array_ptr + 1;
             block_ptr != m_block_array + m_block_array_size;
             ++block_ptr) {
            if (*block_ptr != nullptr) {
                (*block_ptr)->~BlockType();
                ka::deallocate_raw(*block_ptr, sizeof(BlockType));
                (*block_ptr) = nullptr;
            } else {
                break;
            }
        }

        auto blocks_in_use = m_finish.m_block_array_ptr - m_start.m_block_array_ptr + 1;
        auto init_block_array_size = sc_initial_block_array_size;
        const u64 block_limit =
            strict ? blocks_in_use : std::max((u64)(blocks_in_use * 2), init_block_array_size);

        if (m_block_array_size > block_limit) {
            // yes, so resize
            auto const new_block_array_size = block_limit;
            auto new_block_array =
                ka::casted_allocate_raw_cleared<BlockPtrType>(sizeof(BlockPtrType) *
                                                              new_block_array_size);
            auto start_block_ptr = new_block_array + ((new_block_array_size - blocks_in_use) / 2);
            auto finish_block_ptr = start_block_ptr + blocks_in_use;

            for (auto block_ptr = start_block_ptr, src_block_ptr = m_start.m_block_array_ptr;
                 block_ptr != finish_block_ptr;
                 ++block_ptr, ++src_block_ptr) {
                *block_ptr = *src_block_ptr;
            }

            ka::deallocate_raw(m_block_array, sizeof(BlockPtrType) * m_block_array_size);
            m_block_array = new_block_array;
            m_block_array_size = new_block_array_size;
            m_start.m_block_array_ptr = start_block_ptr;
            m_finish.m_block_array_ptr = finish_block_ptr - 1;
        }
    }

    // initialize storage for num_elements
    // no objects are constructed. Only storage is
    // allocated, and the start and finish are set
    // to indicate that num_elements are used.
    inline void initialize(u64 num_elements)
    {
        auto num_nodes = (num_elements / BlockType::get_block_size()) + 1;
        // one node extra on either side
        auto const initial_size = sc_initial_block_array_size;
        m_block_array_size = std::max(initial_size, num_nodes + 2);

        m_block_array =
            ka::casted_allocate_raw_cleared<BlockPtrType>(sizeof(BlockPtrType) *
                                                          m_block_array_size);

        auto start_block_ptr = m_block_array + ((m_block_array_size - num_nodes) / 2);
        auto finish_block_ptr = start_block_ptr + num_nodes - 1;
        create_blocks(start_block_ptr, finish_block_ptr + 1);

        m_start.m_block_array_ptr = start_block_ptr;
        m_start.m_current = (*start_block_ptr)->get_begin();
        m_finish.m_block_array_ptr = finish_block_ptr;
        m_finish.m_current = (*finish_block_ptr)->get_end();

        auto full_capacity = m_finish - m_start;
        m_start += ((full_capacity - num_elements) / 2);
        m_finish = m_start + num_elements;
    }

    // release all the memory
    // and reset to state just after calling
    // the default constructor
    inline void reset()
    {
        for (auto block_ptr = m_start.m_block_array_ptr;
             block_ptr != m_finish.m_block_array_ptr + 1;
             ++block_ptr) {
            ka::deallocate_raw(*block_ptr, sizeof(BlockType));
            *block_ptr = nullptr;
        }
        ka::deallocate_raw(m_block_array, sizeof(BlockPtrType) * m_block_array_size);
        initialize(0);
    }

    inline DequeInternal(bool do_initialization = true)
        : m_block_array(nullptr), m_block_array_size(0),
          m_start(), m_finish()
    {
        if (do_initialization) {
            initialize(0);
        }
    }

    inline DequeInternal(DequeInternal&& other)
        : DequeInternal()
    {
        std::swap(m_block_array, other.m_block_array);
        std::swap(m_block_array_size, other.m_block_array_size);
        std::swap(m_start, other.m_start);
        std::swap(m_finish, other.m_finish);
    }

    // preallocate space for num_elems elements
    inline DequeInternal(u64 num_elems)
        : m_block_array(nullptr), m_block_array_size(0),
          m_start(), m_finish()
    {
        initialize(num_elems);
    }

    inline ~DequeInternal()
    {
        for (auto block_ptr = m_start.m_block_array_ptr;
             block_ptr != m_finish.m_block_array_ptr + 1;
             ++block_ptr) {
            ka::deallocate_raw(*block_ptr, sizeof(BlockType));
            *block_ptr = nullptr;
        }
        ka::deallocate_raw(m_block_array, sizeof(BlockPtrType) * m_block_array_size);
    }

    inline void assign(DequeInternal&& other)
    {
        reset();

        std::swap(m_block_array, other.m_block_array);
        std::swap(m_block_array_size, other.m_block_array_size);
        std::swap(m_start, other.m_start);
        std::swap(m_finish, other.m_finish);
    }

    inline void swap(DequeInternal& other)
    {
        std::swap(m_block_array, other.m_block_array);
        std::swap(m_block_array_size, other.m_block_array_size);
        std::swap(m_start, other.m_start);
        std::swap(m_finish, other.m_finish);
    }
};

} /* end namespace deque_detail_ */

} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_DEQUE_TYPES_HPP_ */

//
// DequeTypes.hpp ends here
