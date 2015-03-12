// SequenceHeap.hpp ---
//
// Filename: SequenceHeap.hpp
// Author: Abhishek Udupa
// Created: Wed Mar 11 15:35:47 2015 (-0400)
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

// This code was adapted from the code by Peter Sanders, original
// code can be found at: http://algo2.iti.kit.edu/sanders/programs/spq/
// Thanks to Peter Sanders for making the code available

// Code:

#if !defined KINARA_KINARA_COMMON_CONTAINERS_SEQUENCE_HEAP_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_SEQUENCE_HEAP_HPP_

#include "../allocators/MemoryManager.hpp"
#include "../basetypes/KinaraErrors.hpp"
#include "../basetypes/KinaraBase.hpp"

#include <cstring>
#include <algorithm>

namespace kinara {
namespace containers {
namespace sequence_heap_detail_ {

namespace ka = kinara::allocators;

// hierarchical memory priority queue data structure
const int KNBufferSize1 = 32; // equalize procedure call overheads etc.
const int KNN = 512; // bandwidth
const int KNKMAX = 64;  // maximal arity
const int KNLevels = 4; // overall capacity >= KNN*KNKMAX^KNLevels
const int LogKNKMAX = 6;  // ceil(log KNK)

template <class Key, class Value>
struct KNElement
{
    Key key;
    Value value;
};

//////////////////////////////////////////////////////////////////////
// fixed size binary heap
template <class Key, class Value, int capacity>
class BinaryHeap
{
private:
    typedef KNElement<Key, Value> Element;
    Element m_data[capacity + 2];
    u64 m_size;  // index of last used element

public:
    inline BinaryHeap(const Key& sup, const Key& infimum)
        : m_size(0)
    {
        m_data[0].key = infimum; // sentinel
        m_data[capacity + 1].key = sup;
        reset();
    }

    inline const Key& get_supremum() const
    {
        return m_data[capacity + 1].key;
    }

    inline void reset()
    {
        m_size = 0;
        Key sup = get_supremum();
        for (int i = 1;  i <= capacity;  i++) {
            m_data[i].key = sup;
        }
    }

    inline i64 get_size() const
    {
        return m_size;
    }

    inline const Key& get_min_key() const
    {
        return m_data[1].key;
    }

    inline const Value& get_min_value() const
    {
        return m_data[1].value;
    }

    inline void delete_min()
    {
        KINARA_ASSERT(size > 0);

        // first move up elements on a min-path
        u64 hole = 1;
        u64 succ = 2;
        u64 sz   = m_size;

        while (succ < sz) {
            Key key1 = m_data[succ].key;
            Key key2 = m_data[succ + 1].key;
            if (key1 > key2) {
                succ++;
                m_data[hole].key   = key2;
                m_data[hole].value = m_data[succ].value;
            } else {
                m_data[hole].key   = key1;
                m_data[hole].value = m_data[succ].value;
            }
            hole = succ;
            succ <<= 1;
        }

        // bubble up rightmost element
        Key bubble = m_data[sz].key;
        int pred = hole >> 1;
        while (m_data[pred].key > bubble) { // must terminate since min at root
            m_data[hole] = m_data[pred];
            hole = pred;
            pred >>= 1;
        }

        // finally move m_data to hole
        m_data[hole].key = bubble;
        m_data[hole].value = m_data[sz].value;

        m_data[m_size].key = get_supremum(); // mark as deleted
        m_size = sz - 1;
    }

    inline void delete_min_fancy(Key& key, Value& value)
    {
        key = get_min_key();
        value = get_min_value();
        delete_min();
    }

    inline void insert(const Key& k, const Value& v)
    {
        KINARA_ASSERT(size < capacity);

        m_size++;
        u64 hole = m_size;
        u64 pred = hole >> 1;
        Key pred_key = m_data[pred].key;

        while (pred_key > k) { // must terminate due to sentinel at 0
            m_data[hole].key = pred_key;
            m_data[hole].value = m_data[pred].value;
            hole = pred;
            pred >>= 1;
            pred_key = m_data[pred].key;
        }

        // finally move data to hole
        m_data[hole].key = k;
        m_data[hole].value = v;
    }

    inline void sortTo(Element* to)
    {
        // sort in increasing order and empty
        const u64 sz = m_size;
        const Key& sup = get_supremum();
        Element* const beyond = to + sz;
        Element* const root = m_data + 1;

        while (to < beyond) {
            // copy minimun
            *to = *root;
            to++;

            // bubble up second smallest as in deleteMin
            u64 hole = 1;
            u64 succ = 2;
            while (succ <= sz) {
                Key key1 = m_data[succ].key;
                Key key2 = m_data[succ + 1].key;
                if (key1 > key2) {
                    succ++;
                    m_data[hole].key = key2;
                    m_data[hole].value = m_data[succ].value;
                } else {
                    m_data[hole].key = key1;
                    m_data[hole].value = m_data[succ].value;
                }
                hole = succ;
                succ <<= 1;
            }

            // just mark hole as deleted
            m_data[hole].key = sup;
        }

        m_size = 0;
    }
};

//////////////////////////////////////////////////////////////////////
// The data structure from Knuth, "Sorting and Searching", Section 5.4.1
template <class Key, class Value>
class KNLooserTree
{
private:
    typedef KNElement<Key, Value> Element;
    struct Entry
    {
        Key key;   // Key of Looser element (winner for 0)
        u64 index; // number of loosing segment
    };

    // stack of empty segments
    u64 m_empty[KNKMAX]; // indices of empty segments
    i64 m_last_free;  // where in "empty" is the last valid entry?

    u64 m_size; // total number of elements stored
    u64 m_logK; // log of current tree size
    u64 m_k; // invariant k = 1 << logK

    Element m_dummy; // target of empty segment pointers

    // upper levels of looser trees
    // entry[0] contains the winner info
    Entry m_entry[KNKMAX];

    // leaf information
    // note that Knuth uses indices k..k-1
    // while we use 0..k-1
    Element* m_current[KNKMAX]; // pointer to actual element
    Element* m_segment[KNKMAX]; // start of Segments

    // private member functions
    u64 init_winner(u64 root);
    void update_on_insert(u64 node, const Key& new_key, u64 new_index,
                          Key& winner_key, u64& winner_index, u64& mask);
    void deallocate_segment(u64 index);
    void double_K();
    void compact_tree();
    void rebuild_looser_tree();
    int segment_is_empty(u64 i);

    inline void tree_step(u64 level, Key& winner_key, u64& winner_index,
                          Entry* reg_entry, u64 logK)
        __attribute__((__always_inline__));

    inline void multi_merge_body(Element* to, u64 l, u64 logK)
        __attribute__((__always_inline__));

public:
    KNLooserTree();
    void init(const Key& sup);

    void multi_merge_unrolled_3(Element* to, u64 l);
    void multi_merge_unrolled_4(Element* to, u64 l);
    void multi_merge_unrolled_5(Element* to, u64 l);
    void multi_merge_unrolled_6(Element* to, u64 l);
    void multi_merge_unrolled_7(Element* to, u64 l);
    void multi_merge_unrolled_8(Element* to, u64 l);
    void multi_merge_unrolled_9(Element* to, u64 l);
    void multi_merge_unrolled_10(Element* to, u64 l);

    void multi_merge(Element* to, u64 l);
    void multi_merge_K(Element* to, u64 l);
    bool space_is_available() const
    {
        return (m_k < KNKMAX || m_last_free >= 0);
    }

    void insert_segment(Element* to, u64 sz);

    int  get_size() const
    {
        return m_size;
    }
    const Key& get_supremum() const
    {
        return m_dummy.key;
    }
};


//////////////////////////////////////////////////////////////////////
// 2 level multi-merge tree
template <class Key, class Value>
class KNHeap
{
private:
    typedef KNElement<Key, Value> Element;

    KNLooserTree<Key, Value> m_tree[KNLevels];

    // one delete buffer for each tree (extra space for sentinel)
    Element m_buffer2[KNLevels][KNN + 1]; // tree->buffer2->buffer1
    Element* m_min_buffer2[KNLevels];

    // overall delete buffer
    Element m_buffer1[KNBufferSize1 + 1];
    Element* m_min_buffer1;

    // insert buffer
    BinaryHeap<Key, Value, KNN> m_insert_heap;

    // how many levels are active
    u64 m_active_levels;

    // total size not counting insertBuffer and buffer1
    u64 m_size;

    // private member functions
    void refill_buffer_1();
    u64 refill_buffer_2(u64 k);
    u64 make_space_available(u64 level);
    void empty_insert_heap();
    inline const Key& get_supremum() const
    {
        return m_buffer2[0][KNN].key;
    }

    inline u64 get_size_1() const
    {
        return (m_buffer1 + KNBufferSize1) - m_min_buffer1;
    }

    inline u64 get_size_2(u64 i) const
    {
        return &(m_buffer2[i][KNN]) - m_min_buffer2[i];
    }

public:
    KNHeap(const Key& sup, const Key& infimum);
    u64 get_size() const;
    void get_min(Key& key, Value& value);
    void delete_min(Key& key, Value& value);
    void insert(const Key& key, const Value& value);
};


template <class Key, class Value>
inline u64 KNHeap<Key, Value>::get_size() const
{
    return (m_size + m_insert_heap.get_size() +
            ((m_buffer1 + KNBufferSize1) - m_min_buffer1));
}

template <class Key, class Value>
inline void KNHeap<Key, Value>::get_min(Key& key, Value& value)
{
    auto const& key1 = m_min_buffer1->key;
    auto const& key2 = m_insert_heap.get_min_key();
    if (key2 >= key1) {
        key = key1;
        value = m_min_buffer1->value;
    } else {
        key = key2;
        value = m_insert_heap.get_min_value();
    }
}

template <class Key, class Value>
inline void KNHeap<Key, Value>::delete_min(Key& key, Value& value)
{
    auto const& key1 = m_min_buffer1->key;
    auto const& key2 = m_insert_heap.get_min_key();
    if (key2 >= key1) {
        key = key1;
        value = m_min_buffer1->value;
        m_min_buffer1++;
        if (m_min_buffer1 == m_buffer1 + KNBufferSize1) {
            refill_buffer_1();
        }
    } else {
        key = key2;
        value = m_insert_heap.get_min_value();
        m_insert_heap.delete_min();
    }
}

template <class Key, class Value>
inline void KNHeap<Key, Value>::insert(const Key& k, const Value& v)
{
    if (m_insert_heap.get_size() == KNN) {
        empty_insert_heap();
    }
    m_insert_heap.insert(k, v);
}

/////////////////////////////////////////////////////////////////////
// auxiliary functions

// merge sz element from the two sentinel terminated input
// sequences *f0 and *f1 to "to"
// advance *fo and *f1 accordingly.
// require: at least sz nonsentinel elements available in f0, f1
// require: to may overwrite one of the sources as long as
//   *fx + sz is before the end of fx
template <class Key, class Value>
static inline void merge(KNElement<Key, Value>** f0,
                         KNElement<Key, Value>** f1,
                         KNElement<Key, Value>* to,
                         u64 sz)
{
    KNElement<Key, Value>* from0 = *f0;
    KNElement<Key, Value>* from1 = *f1;
    KNElement<Key, Value>* done = to + sz;
    Key key0 = from0->key;
    Key key1 = from1->key;

    while (to < done) {
        if (key1 <= key0) {
            to->key = key1;
            to->value = from1->value; // note that this may be the same address
            from1++; // nach hinten schieben?
            key1 = from1->key;
        } else {
            to->key = key0;
            to->value = from0->value; // note that this may be the same address
            from0++; // nach hinten schieben?
            key0 = from0->key;
        }
        to++;
    }
    *f0 = from0;
    *f1 = from1;
}


// merge sz element from the three sentinel terminated input
// sequences *f0, *f1 and *f2 to "to"
// advance *f0, *f1 and *f2 accordingly.
// require: at least sz nonsentinel elements available in f0, f1 and f2
// require: to may overwrite one of the sources as long as
//   *fx + sz is before the end of fx
template <class Key, class Value>
static inline void merge3(KNElement<Key, Value> **f0,
                          KNElement<Key, Value> **f1,
                          KNElement<Key, Value> **f2,
                          KNElement<Key, Value>  *to,
                          u64 sz)
{
    KNElement<Key, Value>* from0 = *f0;
    KNElement<Key, Value>* from1 = *f1;
    KNElement<Key, Value>* from2 = *f2;
    KNElement<Key, Value>* done = to + sz;

    Key key0 = from0->key;
    Key key1 = from1->key;
    Key key2 = from2->key;

    if (key0 < key1) {
        if (key1 < key2) {
            goto s012;
        } else {
            if (key2 < key0) {
                goto s201;
            } else {
                goto s021;
            }
        }
    } else {
        if (key1 < key2) {
            if (key0 < key2) {
                goto s102;
            } else {
                goto s120;
            }
        } else {
            goto s210;
        }
    }

#define Merge3Case____(a,b,c)                       \
    s ## a ## b ## c :                              \
        if (to == done) goto finish;                \
    to->key = key ## a;                             \
    to->value = from ## a -> value;                 \
    to++;                                           \
    from ## a ++;                                   \
    key ## a = from ## a -> key;                    \
    if (key ## a < key ## b) goto s ## a ## b ## c; \
    if (key ## a < key ## c) goto s ## b ## a ## c; \
    goto s ## b ## c ## a;                          \
    (void(0))

    // the order is choosen in such a way that
    // four of the trailing gotos can be eliminated by the optimizer
    Merge3Case____(0, 1, 2);
    Merge3Case____(1, 2, 0);
    Merge3Case____(2, 0, 1);
    Merge3Case____(1, 0, 2);
    Merge3Case____(0, 2, 1);
    Merge3Case____(2, 1, 0);

#undef Merge3Case____

finish:
    *f0   = from0;
    *f1   = from1;
    *f2   = from2;
}


// merge sz element from the three sentinel terminated input
// sequences *f0, *f1, *f2 and *f3 to "to"
// advance *f0, *f1, *f2 and *f3 accordingly.
// require: at least sz nonsentinel elements available in f0, f1, f2 and f2
// require: to may overwrite one of the sources as long as
//   *fx + sz is before the end of fx
template <class Key, class Value>
static inline void merge4(KNElement<Key, Value>** f0,
                          KNElement<Key, Value>** f1,
                          KNElement<Key, Value>** f2,
                          KNElement<Key, Value>** f3,
                          KNElement<Key, Value>* to,
                          u64 sz)
{
    KNElement<Key, Value>* from0 = *f0;
    KNElement<Key, Value>* from1 = *f1;
    KNElement<Key, Value>* from2 = *f2;
    KNElement<Key, Value>* from3 = *f3;
    KNElement<Key, Value>* done = to + sz;

    Key key0 = from0->key;
    Key key1 = from1->key;
    Key key2 = from2->key;
    Key key3 = from3->key;

#define StartMerge4____(a, b, c, d)                                 \
    if (key##a <= key##b && key##b <= key##c && key##c <= key##d) { \
        goto s ## a ## b ## c ## d;                                 \
    }                                                               \
    (void(0))

    StartMerge4____(0, 1, 2, 3);
    StartMerge4____(1, 2, 3, 0);
    StartMerge4____(2, 3, 0, 1);
    StartMerge4____(3, 0, 1, 2);

    StartMerge4____(0, 3, 1, 2);
    StartMerge4____(3, 1, 2, 0);
    StartMerge4____(1, 2, 0, 3);
    StartMerge4____(2, 0, 3, 1);

    StartMerge4____(0, 2, 3, 1);
    StartMerge4____(2, 3, 1, 0);
    StartMerge4____(3, 1, 0, 2);
    StartMerge4____(1, 0, 2, 3);

    StartMerge4____(2, 0, 1, 3);
    StartMerge4____(0, 1, 3, 2);
    StartMerge4____(1, 3, 2, 0);
    StartMerge4____(3, 2, 0, 1);

    StartMerge4____(3, 0, 2, 1);
    StartMerge4____(0, 2, 1, 3);
    StartMerge4____(2, 1, 3, 0);
    StartMerge4____(1, 3, 0, 2);

    StartMerge4____(1, 0, 3, 2);
    StartMerge4____(0, 3, 2, 1);
    StartMerge4____(3, 2, 1, 0);
    StartMerge4____(2, 1, 0, 3);

#undef StartMerge4____

#define Merge4Case____(a, b, c, d)                                  \
    s ## a ## b ## c ## d:                                          \
        if (to == done) goto finish;                                \
    to->key = key ## a;                                             \
    to->value = from ## a -> value;                                 \
    to++;                                                           \
    from ## a ++;                                                   \
    key ## a = from ## a -> key;                                    \
    if (key ## a < key ## c) {                                      \
        if (key ## a < key ## b) {                                  \
            goto s ## a ## b ## c ## d;                             \
        } else {                                                    \
            goto s ## b ## a ## c ## d;                             \
        }                                                           \
    } else {                                                        \
        if (key ## a < key ## d) {                                  \
            goto s ## b ## c ## a ## d;                             \
        } else {                                                    \
            goto s ## b ## c ## d ## a;                             \
        }                                                           \
    }                                                               \
    (void(0))

    Merge4Case____(0, 1, 2, 3);
    Merge4Case____(1, 2, 3, 0);
    Merge4Case____(2, 3, 0, 1);
    Merge4Case____(3, 0, 1, 2);

    Merge4Case____(0, 3, 1, 2);
    Merge4Case____(3, 1, 2, 0);
    Merge4Case____(1, 2, 0, 3);
    Merge4Case____(2, 0, 3, 1);

    Merge4Case____(0, 2, 3, 1);
    Merge4Case____(2, 3, 1, 0);
    Merge4Case____(3, 1, 0, 2);
    Merge4Case____(1, 0, 2, 3);

    Merge4Case____(2, 0, 1, 3);
    Merge4Case____(0, 1, 3, 2);
    Merge4Case____(1, 3, 2, 0);
    Merge4Case____(3, 2, 0, 1);

    Merge4Case____(3, 0, 2, 1);
    Merge4Case____(0, 2, 1, 3);
    Merge4Case____(2, 1, 3, 0);
    Merge4Case____(1, 3, 0, 2);

    Merge4Case____(1, 0, 3, 2);
    Merge4Case____(0, 3, 2, 1);
    Merge4Case____(3, 2, 1, 0);
    Merge4Case____(2, 1, 0, 3);

#undef Merge4Case____

finish:
    *f0   = from0;
    *f1   = from1;
    *f2   = from2;
    *f3   = from3;
}

///////////////////////// LooserTree ///////////////////////////////////
template <class Key, class Value>
KNLooserTree<Key, Value>::KNLooserTree()
    : m_last_free(0), m_size(0), m_logK(0), m_k(1)
{
    m_empty[0] = 0;
    m_segment[0] = nullptr;
    m_current[0] = &m_dummy;
    // entry and dummy are initialized by init
    // since they need the value of supremum
}


template <class Key, class Value>
void KNLooserTree<Key, Value>::init(const Key& sup)
{
    m_dummy.key = sup;
    rebuild_looser_tree();
}

// rebuild looser tree information from the values in current
template <class Key, class Value>
void KNLooserTree<Key, Value>::rebuild_looser_tree()
{
    u64 winner = init_winner(1);
    m_entry[0].index = winner;
    m_entry[0].key = m_current[winner]->key;
}


// given any values in the leaves this
// routing recomputes upper levels of the tree
// from scratch in linear time
// initialize entry[root].index and the subtree rooted there
// return winner index
template <class Key, class Value>
u64 KNLooserTree<Key, Value>::init_winner(u64 root)
{
    if (root >= m_k) { // leaf reached
        return root - m_k;
    } else {
        u64 left = init_winner(2*root);
        u64 right = init_winner(2*root + 1);
        const Key& lk = m_current[left ]->key;
        const Key& rk = m_current[right]->key;
        if (lk <= rk) { // right subtree looses
            m_entry[root].index = right;
            m_entry[root].key = rk;
            return left;
        } else {
            m_entry[root].index = left;
            m_entry[root].key = lk;
            return right;
        }
    }
}


// first go up the tree all the way to the root
// hand down old winner for the respective subtree
// based on new value, and old winner and looser
// update each node on the path to the root top down.
// This is implemented recursively
template <class Key, class Value>
void KNLooserTree<Key, Value>::update_on_insert(u64 node, const Key& new_key,
                                                u64 new_index, Key& winner_key,
                                                u64& winner_index, u64& mask)
{
    if (node == 0) { // winner part of root
        mask = 1 << (m_logK - 1);
        winner_key = m_entry[0].key;
        winner_index = m_entry[0].index;

        if (new_key < m_entry[node].key) {
            m_entry[node].key   = new_key;
            m_entry[node].index = new_index;
        }
    } else {
        update_on_insert(node >> 1, new_key, new_index, winner_key, winner_index, mask);
        const Key& looser_key = m_entry[node].key;
        u64 looser_index = m_entry[node].index;

        if ((winner_index & mask) != (new_index & mask)) { // different subtrees
            if (new_key < looser_key) { // newKey will have influence here
                if (new_key < winner_key) { // old winner loses here
                    m_entry[node].key   = winner_key;
                    m_entry[node].index = winner_index;
                } else { // new entry looses here
                    m_entry[node].key   = new_key;
                    m_entry[node].index = new_index;
                }
            }
            winner_key = looser_key;
            winner_index = looser_index;
        }
        // note that nothing needs to be done if
        // the winner came from the same subtree
        // a) newKey <= winnerKey => even more reason for the other tree to loose
        // b) newKey >  winnerKey => the old winner will beat the new
        //                           entry further down the tree
        // also the same old winner is handed down the tree

        mask >>= 1; // next level
    }
}


// make the tree two times as wide
// may only be called if no free slots are left ?? necessary ??
template <class Key, class Value>
void KNLooserTree<Key, Value>::double_K()
{
    for (u64 i = (2 * m_k) - 1; i >= m_k; i--) {
        m_current[i] = &m_dummy;
        m_last_free++;
        m_empty[m_last_free] = i;
    }

    // double the size
    m_k *= 2;
    ++m_logK;

    // recompute looser tree information
    rebuild_looser_tree();
}


// compact nonempty segments in the left half of the tree
template <class Key, class Value>
void KNLooserTree<Key, Value>::compact_tree()
{
    const Key& sup = m_dummy.key;

    // compact all nonempty segments to the left
    u64 from = 0;
    u64 to = 0;

    for (; from < m_k; ++from) {
        if (m_current[from]->key != sup) {
            m_current[to] = m_current[from];
            m_segment[to] = m_segment[from];
            ++to;
        }
    }

    // half degree as often as possible
    while (to < m_k/2) {
        m_k /= 2;
        --m_logK;
    }

    // overwrite garbage and compact the stack of empty segments
    m_last_free = -1;
    for (; to < m_k; ++to) {
        ++m_last_free;
        m_empty[m_last_free] = to;
        m_current[to] = &m_dummy;
    }

    // recompute looser tree information
    rebuild_looser_tree();
}


// insert segment beginning at to
// require: spaceIsAvailable() == 1
template <class Key, class Value>
void KNLooserTree<Key, Value>::insert_segment(Element* to, u64 sz)
{
    if (sz > 0) {
        // get a free slot
        if (m_last_free < 0) { // tree is too small
            double_K();
        }
        u64 index = m_empty[m_last_free];
        --m_last_free; // pop

        // link new segment
        m_current[index] = m_segment[index] = to;
        m_size += sz;

        // propagate new information up the tree
        Key dummy_key;
        u64 dummy_index;
        u64 dummy_mask;
        update_on_insert((index + m_k) >> 1, to->key, index,
                         dummy_key, dummy_index, dummy_mask);
    } else {
        // immediately deallocate
        // this is not only an optimization
        // but also needed to keep empty segments from
        // clogging up the tree
        ka::deallocate_array(to);
    }
}


// free an empty segment
template <class Key, class Value>
void KNLooserTree<Key, Value>::deallocate_segment(u64 index)
{
    // reroute current pointer to some empty dummy segment
    // with a sentinel key
    m_current[index] = &m_dummy;

    // free memory
    ka::deallocate_array(m_segment[index]);
    m_segment[index] = nullptr;

    // push on the stack of free segment indices
    ++m_last_free;
    m_empty[m_last_free] = index;
}

template<class Key, class Value>
inline void
KNLooserTree<Key, Value>::tree_step(u64 level, Key& winner_key,
                                    u64& winner_index, Entry* reg_entry,
                                    u64 logK)
{
    if ((1 << logK) >= (1 << level)) {
        Entry* position = reg_entry + ((winner_index + (1 << logK)) >> ((logK - level) + 1));
        Key key = position->key;
        if (key < winner_key) {
            u64 index = position->index;
            position->key = winner_key;
            position->index = winner_index;
            winner_key = key;
            winner_index = index;
        }
    }
}

template <class Key, class Value>
inline void
KNLooserTree<Key, Value>::multi_merge_body(Element* to, u64 l, u64 logK)
{
    Entry* current_pos;
    Key current_key;
    u64 current_index; // leaf pointed to by current entry
    Element* done = to + l;
    Entry* reg_entry = m_entry;
    Element** reg_current = m_current;
    u64 winner_index = reg_entry[0].index;
    Key winner_key = reg_entry[0].key;
    Element* winner_pos;
    Key sup = m_dummy.key; // supremum

    while (to < done) {
        winner_pos = reg_current[winner_index];

        // write result
        to->key   = winner_key;
        to->value = winner_pos->value;

        // advance winner segment
        ++winner_pos;
        reg_current[winner_index] = winner_pos;
        winner_key = winner_pos->key;

        // remove winner segment if empty now
        if (winner_key == sup) {
            deallocate_segment(winner_index);
        }
        ++to;

        // update looser tree
        TreeStep(10, winner_key, winner_index, reg_entry, logK);
        TreeStep(9, winner_key, winner_index, reg_entry, logK);
        TreeStep(8, winner_key, winner_index, reg_entry, logK);
        TreeStep(7, winner_key, winner_index, reg_entry, logK);
        TreeStep(6, winner_key, winner_index, reg_entry, logK);
        TreeStep(5, winner_key, winner_index, reg_entry, logK);
        TreeStep(4, winner_key, winner_index, reg_entry, logK);
        TreeStep(3, winner_key, winner_index, reg_entry, logK);
        TreeStep(2, winner_key, winner_index, reg_entry, logK);
        TreeStep(1, winner_key, winner_index, reg_entry, logK);
    }
    reg_entry[0].index = winner_index;
    reg_entry[0].key   = winner_key;
}

// multi-merge for a fixed K=1<<LogK
// this looks ugly but the include file explains
// why this is the most portable choice
template <class Key, class Value>
void KNLooserTree<Key, Value>::multi_merge_unrolled_3(Element* to, u64 l)
{
    multi_merge_body(to, l, 3);
}

template <class Key, class Value>
void KNLooserTree<Key, Value>::multi_merge_unrolled_4(Element* to, u64 l)
{
    multi_merge_body(to, l, 4);
}

template <class Key, class Value>
void KNLooserTree<Key, Value>::multi_merge_unrolled_5(Element* to, u64 l)
{
    multi_merge_body(to, l, 5);
}

template <class Key, class Value>
void KNLooserTree<Key, Value>::multi_merge_unrolled_6(Element* to, u64 l)
{
    multi_merge_body(to, l, 6);
}

template <class Key, class Value>
void KNLooserTree<Key, Value>::multi_merge_unrolled_7(Element* to, u64 l)
{
    multi_merge_body(to, l, 7);
}

template <class Key, class Value>
void KNLooserTree<Key, Value>::multi_merge_unrolled_8(Element* to, u64 l)
{
    multi_merge_body(to, l, 8);
}

template <class Key, class Value>
void KNLooserTree<Key, Value>::multi_merge_unrolled_9(Element* to, u64 l)
{
    multi_merge_body(to, l, 9);
}

template <class Key, class Value>
void KNLooserTree<Key, Value>::multi_merge_unrolled_10(Element* to, u64 l)
{
    multi_merge_body(to, l, 10);
}

// delete the l smallest elements and write them to "to"
// empty segments are deallocated
// require:
// - there are at least l elements
// - segments are ended by sentinels
template <class Key, class Value>
void KNLooserTree<Key, Value>::multi_merge(Element* to, u64 l)
{
    switch(m_logK) {
    case 0:
        std::copy(to, m_current[0], l);
        m_current[0] += l;
        m_entry[0].key = m_current[0]->key;
        if (segment_is_empty(0)) {
            deallocate_segment(0);
        }
        break;

    case 1:
        merge(m_current + 0, m_current + 1, to, l);
        rebuild_looser_tree();
        if (segment_is_empty(0)) {
            deallocate_segment(0);
        }
        if (segment_is_empty(1)) {
            deallocate_segment(1);
        }
        break;

    case 2:
        merge4(m_current + 0, m_current + 1, m_current + 2, m_current + 3, to, l);
        rebuild_looser_tree();
        if (segment_is_empty(0)) {
            deallocate_segment(0);
        }
        if (segment_is_empty(1)) {
            deallocate_segment(1);
        }
        if (segment_is_empty(2)) {
            deallocate_segment(2);
        }
        if (segment_is_empty(3)) {
            deallocate_segment(3);
        }
        break;

    case 3:
        multi_merge_unrolled_3(to, l);
        break;
    case 4:
        multi_merge_unrolled_4(to, l);
        break;
    case 5:
        multi_merge_unrolled_5(to, l);
        break;
    case 6:
        multi_merge_unrolled_6(to, l);
        break;
    case 7:
        multi_merge_unrolled_7(to, l);
        break;
    case 8:
        multi_merge_unrolled_8(to, l);
        break;
    case 9:
        multi_merge_unrolled_9(to, l);
        break;
    case 10:
        multi_merge_unrolled_10(to, l);
        break;
    default:
        multi_merge_K(to, l);
        break;
    }
    m_size -= l;

    // compact tree if it got considerably smaller
    if (m_k > 1 && m_last_free >= ((3 * m_k) / 5 - 1)) {
        compact_tree();
    }
}

// is this segment empty and does not point to dummy yet?
template <class Key, class Value>
inline int KNLooserTree<Key, Value>::segment_is_empty(u64 i)
{
    return (m_current[i]->key == get_supremum() &&
            m_current[i] != &m_dummy);
}

// multi-merge for arbitrary K
template <class Key, class Value>
void KNLooserTree<Key, Value>::multi_merge_K(Element* to, u64 l)
{
    Entry *current_pos;
    Key current_key;
    u64 current_index; // leaf pointed to by current entry
    u64 k_reg = m_k;
    Element* done = to + l;
    u64 winner_index = m_entry[0].index;
    Key winner_key = m_entry[0].key;
    Element* winner_pos;
    const Key& sup = m_dummy.key; // supremum

    while (to < done) {
        winner_pos = m_current[winner_index];

        // write result
        to->key   = winner_key;
        to->value = winner_pos->value;

        // advance winner segment
        ++winner_pos;
        m_current[winner_index] = winner_pos;
        winner_key = winner_pos->key;

        // remove winner segment if empty now
        if (winner_key == sup) {
            deallocate_segment(winner_index);
        }

        // go up the entry-tree
        for (u64 i = (winner_index + k_reg) >> 1;  i > 0;  i >>= 1)
        {
            current_pos = m_entry + i;
            current_key = current_pos->key;
            if (current_key < winner_key) {
                current_index = current_pos->index;
                current_pos->key = winner_key;
                current_pos->index = winner_index;
                winner_key = current_key;
                winner_index = current_index;
            }
        }

        ++to;
    }
    m_entry[0].index = winner_index;
    m_entry[0].key   = winner_key;
}

////////////////////////// KNHeap //////////////////////////////////////
template <class Key, class Value>
KNHeap<Key, Value>::KNHeap(const Key& sup, const Key& infimum)
    :m_insert_heap(sup, infimum),
     m_active_levels(0), m_size(0)
{
    m_buffer1[KNBufferSize1].key = sup; // sentinel
    m_min_buffer1 = m_buffer1 + KNBufferSize1; // empty

    for (u64 i = 0; i < KNLevels; ++i) {
        m_tree[i].init(sup); // put tree[i] in a consistent state
        m_buffer2[i][KNN].key = sup; // sentinel
        m_min_buffer2[i] = &(m_buffer2[i][KNN]); // empty
    }
}


//--------------------- Buffer refilling -------------------------------

// refill buffer2[j] and return number of elements found
template <class Key, class Value>
u64 KNHeap<Key, Value>::refill_buffer_2(u64 j)
{
    Element* old_target;
    u64 delete_size;
    u64 tree_size = m_tree[j].get_size();

    u64 buffer_size = (&(m_buffer2[j][0]) + KNN) - m_min_buffer2[j];

    if (tree_size + buffer_size >= KNN) { // buffer will be filled
        old_target = &(m_buffer2[j][0]);
        delete_size = KNN - buffer_size;
    } else {
        old_target = &(m_buffer2[j][0]) + KNN - tree_size - buffer_size;
        delete_size = tree_size;
    }

    // shift  rest to beginning
    // possible hack:
    // - use memcpy if no overlap
    std::copy(old_target, m_min_buffer2[j], buffer_size);
    m_min_buffer2[j] = old_target;

    // fill remaining space from tree
    m_tree[j].multi_merge(old_target + buffer_size, delete_size);
    return delete_size + buffer_size;
}


// move elements from the 2nd level buffers
// to the delete buffer
template <class Key, class Value>
void KNHeap<Key, Value>::refill_buffer_1()
{
    u64 total_size = 0;
    u64 sz;

    for (i64 i = m_active_levels - 1; i >= 0; --i) {
        if ((&(m_buffer2[i][0]) + KNN) - m_min_buffer2[i] < KNBufferSize1) {
            sz = refill_buffer_2(i);
            // max active level dry now?
            if (sz == 0 && i == m_active_levels - 1) {
                --m_active_levels;
            } else {
                total_size += sz;
            }
        } else {
            total_size += KNBufferSize1;
            // actually only a sufficient lower bound
        }
    }

    if (total_size >= KNBufferSize1) { // buffer can be filled
        m_min_buffer1 = m_buffer1;
        sz = KNBufferSize1; // amount to be copied
        m_size -= KNBufferSize1; // amount left in buffer2
    } else {
        m_min_buffer1 = m_buffer1 + KNBufferSize1 - total_size;
        sz = total_size;
        m_size = 0;
    }

    // now call simplified refill routines
    // which can make the assumption that
    // they find all they are asked to find in the buffers
    m_min_buffer1 = m_buffer1 + KNBufferSize1 - sz;

    switch(m_active_levels) {
    case 1:
        std::copy(m_min_buffer1, m_min_buffer2[0], sz);
        m_min_buffer2[0] += sz;
        break;

    case 2:
        merge(&(m_min_buffer2[0]), &(m_min_buffer2[1]), m_min_buffer1, sz);
        break;

    case 3:
        merge3(&(m_min_buffer2[0]),
               &(m_min_buffer2[1]),
               &(m_min_buffer2[2]),
               m_min_buffer1, sz);
        break;

    case 4:
        merge4(&(m_min_buffer2[0]),
               &(m_min_buffer2[1]),
               &(m_min_buffer2[2]),
               &(m_min_buffer2[3]),
               m_min_buffer1, sz);
        break;
    }
}

//--------------------------------------------------------------------

// check if space is available on level k and
// empty this level if necessary leading to a recursive call.
// return the level where space was finally available
template <class Key, class Value>
u64 KNHeap<Key, Value>::make_space_available(u64 level)
{
    u64 final_level;

    if (level == m_active_levels) {
        ++m_active_levels;
    }

    if (m_tree[level].space_is_available()) {
        final_level = level;
    } else {
        final_level = make_space_available(level + 1);
        u64 segment_size = m_tree[level].get_size();
        Element* new_segment = ka::allocate_array<Element>(segment_size + 1);
        m_tree[level].multi_merge(new_segment, segment_size); // empty this level
        //    tree[level].cleanUp();
        new_segment[segment_size].key = m_buffer1[KNBufferSize1].key; // sentinel
        // for queues where size << #inserts
        // it might make sense to stay in this level if
        // segmentSize < alpha * KNN * k^level for some alpha < 1
        m_tree[level + 1].insert_segment(new_segment, segment_size);
    }
    return final_level;
}

// empty the insert heap into the main data structure
template <class Key, class Value>
void KNHeap<Key, Value>::empty_insert_heap()
{
    const Key& sup = get_supremum();

    // build new segment
    Element* new_segment = ka::allocate_array<Element>(KNN + 1);
    Element* new_pos = new_segment;

    // put the new data there for now
    m_insert_heap.sort_to(new_segment);
    new_segment[KNN].key = sup; // sentinel

    // copy the buffer1 and buffer2[0] to temporary storage
    // (the tomporary can be eliminated using some dirty tricks)

    const u64 temp_size = KNN + KNBufferSize1;

    Element temp[temp_size + 1];
    u64 sz1 = get_size_1();
    u64 sz2 = get_size_2(0);

    Element *pos = temp + temp_size - sz1 - sz2;
    std::copy(pos, m_min_buffer1, sz1);
    std::copy(pos, m_min_buffer2[0], sz2);
    temp[temp_size].key = sup; // sentinel

    // refill buffer1
    // (using more complicated code it could be made somewhat fuller
    // in certein circumstances)
    merge(&pos, &new_pos, m_min_buffer1, sz1);

    // refill buffer2[0]
    // (as above we might want to take the opportunity
    // to make buffer2[0] fuller)
    merge(&pos, &new_pos, m_min_buffer2[0], sz2);

    // merge the rest to the new segment
    // note that merge exactly trips into the footsteps
    // of itself
    merge(&pos, &new_pos, new_segment, KNN);

    // and insert it
    u64 free_level = make_space_available(0);
    m_tree[0].insert_segment(new_segment, KNN);

    // get rid of invalid level 2 buffers
    // by inserting them into tree 0 (which is almost empty in this case)
    if (free_level > 0) {
        for (i64 i = free_level; i >= 0; --i) { // reverse order not needed
            // but would allow immediate refill
            new_segment = ka::allocate_array<Element>(get_size_2(i) + 1);
            std::copy(m_min_buffer2[i], m_min_buffer2[i] + get_size_2(i) + 1, new_segment);
            m_tree[0].insert_segment(new_segment, get_size_2(i));
            m_min_buffer2[i] = m_buffer2[i] + KNN; // empty
        }
    }

    // update size
    m_size += KNN;

    // special case if the tree was empty before
    if (m_min_buffer1 == m_buffer1 + KNBufferSize1) {
        refill_buffer_1();
    }
}

} /* end namespace sequence_heap_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_SEQUENCE_HEAP_HPP_ */

//
// SequenceHeap.hpp ends here
