// StringTable.cpp ---
// Filename: StringTable.cpp
// Author: Abhishek Udupa
// Created: Wed Feb 18 01:05:45 2015 (-0500)
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

#include <algorithm>
#include <cmath>

#include "../basetypes/KinaraBase.hpp"
#include "../basetypes/KinaraErrors.hpp"
#include "../primeutils/PrimeGenerator.hpp"
#include "../allocators/MemoryManager.hpp"
#include "../allocators/PoolAllocator.hpp"
#include "../hashfuncs/HashFunctions.hpp"

#include "StringRepr.hpp"
#include "StringTable.hpp"

namespace kinara {
namespace containers {
namespace string_table_detail_ {

namespace ka = kinara::allocators;
namespace kc = kinara::containers;
namespace ku = kinara::utils;

float StringTable::s_resize_factor = StringTable::sc_default_resize_factor;
float StringTable::s_max_load_factor = StringTable::sc_default_max_load_factor;
float StringTable::s_min_load_factor = StringTable::sc_default_min_load_factor;
float StringTable::s_deleted_nonused_rehash_ratio = StringTable::sc_deleted_nonused_rehash_ratio;

inline StringRepr**& StringTable::hash_table()
{
    static StringRepr** the_hash_table = nullptr;
    return the_hash_table;
}

inline ka::PoolAllocator* StringTable::allocator()
{
    static ka::PoolAllocator* the_pool_allocator =
        ka::allocate_object_raw<ka::PoolAllocator>(sizeof(StringRepr), 128);
    return the_pool_allocator;
}

inline u64& StringTable::hash_table_size()
{
    static u64 the_hash_table_size = 0;
    return the_hash_table_size;
}

inline u64& StringTable::hash_table_used()
{
    static u64 the_hash_table_used = 0;
    return the_hash_table_used;
}

inline bool StringTable::is_slot_nonused(const StringRepr* slot_ptr)
{
    return (reinterpret_cast<u64>(slot_ptr) == sc_nonused_slot_marker);
}

inline bool StringTable::is_slot_deleted(const StringRepr* slot_ptr)
{
    return (reinterpret_cast<u64>(slot_ptr) == sc_deleted_slot_marker);
}

inline const StringRepr* StringTable::find(const char* string_value, u64 length)
{
    auto the_hash_table = hash_table();
    if (the_hash_table == nullptr) {
        return nullptr;
    }
    auto the_hash_table_size = hash_table_size();
    if (the_hash_table_size == 0) {
        return nullptr;
    }
    auto the_hash_table_used = hash_table_used();
    if (the_hash_table_used == 0) {
        return nullptr;
    }

    KINARA_ASSERT(the_hash_table_size == 0 ||
                  (the_hash_table_used < the_hash_table_size));

    // okay, we begin searching the hash table
    auto h1 = ku::spooky_hash_64(string_value, length) % the_hash_table_size;
    u64 h2 = 0;
    bool h2_computed = false;

    auto index = h1;
    auto entry = the_hash_table[index];
    u64 num_probes = 0;
    while (!is_slot_nonused(entry) && num_probes < the_hash_table_size) {
        ++num_probes;
        if (!is_slot_deleted(entry) && entry->equals(string_value, length)) {
            return entry;
        }
        // rehash
        if (!h2_computed) {
            h2 = 1 + (ku::city_hash_64(string_value, length) % (the_hash_table_size - 1));
            h2_computed = true;
        }

        index = (index + h2) % the_hash_table_size;
        entry = the_hash_table[index];
    }

    // no slot is marked unused in the table, but
    // slots could possibly be marked deleted, but
    // nothing that is actually in use matches the
    // required string ==> not found in the table.
    return nullptr;
}

// precondition: hash_table() does not contain the string_value
inline const StringRepr* StringTable::insert(const char* string_value, u64 length)
{
    expand_table();
    auto retval = insert_into_table(string_value, length, hash_table(), hash_table_size());
    ++(hash_table_used());
    return retval;
}

// precondition:
// 1. string_table does not contain the string_value
// 2. string_table is large enough to accommodate the value
// 3. table_size is a prime number
inline const StringRepr* StringTable::insert_into_table(const char* string_value, u64 length,
                                                        StringRepr** string_table, u64 table_size)
{
    auto h1 = ku::spooky_hash_64(string_value, length) % table_size;
    u64 h2;
    bool h2_computed = false;
    auto index = h1;
    auto entry = string_table[index];

    u64 num_probes = 0;

    while (!is_slot_nonused(entry) &&
           !is_slot_deleted(entry) &&
           num_probes < table_size) {

        ++num_probes;
        if (!h2_computed) {
            h2 = 1 + (ku::city_hash_64(string_value, length) % (table_size - 1));
            h2_computed = true;
        }
        index = (index + h2) % table_size;
        entry = string_table[index];
    }

    if (!is_slot_deleted(entry) &&
        !is_slot_deleted(entry) &&
        num_probes >= table_size) {
        // No slot left in the table
        // should never happen!
        KINARA_UNREACHABLE_CODE();
    }

    // we now have an empty or deleted slot
    auto new_object = ka::allocate<StringRepr>(*(allocator()), string_value, length);
    string_table[index] = new_object;
    return new_object;
}

// precondition:
// 1. string_table does not contain repr_ptr
// 2. string_table is large enough to accommodate the value
// 3. table_size is a prime number
inline const StringRepr* StringTable::move_into_table(StringRepr* repr_ptr,
                                                      StringRepr **string_table,
                                                      u64 table_size)
{
    auto string_value = repr_ptr->c_str();
    auto length = repr_ptr->length();

    auto h1 = ku::spooky_hash_64(string_value, length) % table_size;
    u64 h2 = 0;
    bool h2_computed = false;

    auto index = h1;
    auto entry = string_table[index];

    while (!is_slot_nonused(entry) && !is_slot_deleted(entry)) {
        if (!h2_computed) {
            h2 = 1 + (ku::city_hash_64(string_value, length) % (table_size - 1));
            h2_computed = true;
        }
        index = (index + h2) % table_size;
        entry = string_table[index];
    }
    // we now have an empty or deleted slot
    string_table[index] = repr_ptr;
    return repr_ptr;
}

// precondition: true
// ensures: the_hash_table() contains no slots marked "deleted"
inline void StringTable::rehash_table()
{
    auto the_table = hash_table();
    auto table_size = hash_table_size();
    auto new_table =
        ka::casted_allocate_raw_cleared<StringRepr*>(sizeof(StringRepr*) * table_size);
    rebuild_table(new_table, the_table, table_size, table_size);
}

inline void StringTable::rebuild_table(StringRepr** new_table, StringRepr** old_table,
                                       u64 new_table_size, u64 old_table_size)
{
    for (u64 i = 0; i < old_table_size; ++i) {
        auto entry = old_table[i];
        if (is_slot_nonused(entry) || is_slot_deleted(entry)) {
            continue;
        }
        move_into_table(entry, new_table, new_table_size);
    }

    // free the old table and set the appropriate variables
    ka::deallocate_raw(old_table, old_table_size * sizeof(StringRepr*));

    hash_table_size() = new_table_size;
    hash_table() = new_table;
}

// precondition: true
// ensures: the_hash_table() contains AT LEAST two empty slots
// ensures: min_utilization <= utilization factor of the_hash_table() < max_utilization
inline void StringTable::expand_table()
{
    auto the_table = hash_table();
    auto table_size = hash_table_size();
    auto table_used = hash_table_used();
    float table_utilization = (float)(table_used + 1) / (float)table_size;

    if (table_used <= table_size - 2 && table_utilization < s_max_load_factor) {
        return;
    }

    // try doing a gc
    garbage_collect();

    table_size = hash_table_size();
    table_used = hash_table_used();
    table_utilization = (float)(table_used + 1) / (float)table_size;

    if (table_used <= table_size - 2 && table_utilization < s_max_load_factor) {
        return;
    }

    // okay, we really need to resize this table
    auto new_table_size = (u64)ceil((float)table_size * s_resize_factor);
    new_table_size = std::max(new_table_size, table_used + 3);
    new_table_size = ku::PrimeGenerator::get_next_prime(new_table_size);

    auto new_table =
        ka::casted_allocate_raw_cleared<StringRepr*>(sizeof(StringRepr*) * new_table_size);
    rebuild_table(new_table, the_table, new_table_size, table_size);
}

// precondition: true
// ensures: no objects with ref-count 0 left in table
// ensures: min_utilization <= utilization factor <= max_utilization
inline void StringTable::garbage_collect()
{
    auto the_table = hash_table();
    auto table_size = hash_table_size();
    auto table_used = hash_table_used();
    u64 table_deleted = 0;

    for (u64 i = 0; i < table_size; ++i) {
        auto entry = the_table[i];
        if (is_slot_nonused(entry) || is_slot_deleted(entry)) {
            continue;
        }
        if (entry->get_ref_count() == 0) {
            ka::deallocate(*(allocator()), entry);
            the_table[i] = (StringRepr*)sc_deleted_slot_marker;
            --table_used;
            ++table_deleted;
        }
    }

    hash_table_used() = table_used;
    auto hash_table_nonused = table_size - table_used;

    float table_utilization = (float)table_used / (float)table_size;
    auto deleted_nonused_ratio = (float)table_deleted / (float)hash_table_nonused;

    if (table_utilization >= s_min_load_factor) {
        if (deleted_nonused_ratio < s_deleted_nonused_rehash_ratio) {
            return;
        } else {
            // rebuild the hash table
            rehash_table();
            return;
        }
    }

    // shrink the table, so that the load factor is
    // somewhat between the min_utilization and max_utilization
    auto target_utilization = (s_min_load_factor + s_max_load_factor) / 2.0f;
    auto new_table_size = (u64)ceil((float)table_used / target_utilization);

    new_table_size = ku::PrimeGenerator::get_next_prime(new_table_size);
    new_table_size = std::max(new_table_size, table_used + 3);

    auto new_table =
        ka::casted_allocate_raw_cleared<StringRepr*>(sizeof(StringRepr*) * new_table_size);
    rebuild_table(new_table, the_table, new_table_size, table_size);

    // garbage collect the pool as well
    auto the_allocator = allocator();
    auto bytes_allocated = the_allocator->get_bytes_allocated();
    auto bytes_claimed = the_allocator->get_bytes_claimed();

    if (((float)bytes_allocated / (float)bytes_claimed) < sc_allocator_utilization_low) {
        the_allocator->garbage_collect();
    }
    return;
}

const StringRepr* StringTable::get_repr(const char* string_value, u64 length)
{
    KINARA_ASSERT(length > 0);

    auto existing = find(string_value, length);
    if (existing == nullptr) {
        return insert(string_value, length);
    } else {
        return existing;
    }
}

void StringTable::finalize()
{
    auto the_table = hash_table();
    auto table_size = hash_table_size();

    for (u64 i = 0; i < table_size; ++i) {
        auto entry = the_table[i];
        if (!is_slot_nonused(entry) &&
            !is_slot_deleted(entry)) {
            ka::deallocate((*(allocator())), entry);
        }
    }

    ka::deallocate_raw(the_table, sizeof(StringRepr*) * table_size);
    hash_table() = nullptr;
    hash_table_size() = (u64)0;
    hash_table_used() = (u64)0;

    // deallocate the pool
    ka::deallocate_object_raw(allocator(), sizeof(ka::PoolAllocator));
}

float StringTable::get_resize_factor()
{
    return s_resize_factor;
}

float StringTable::get_min_load_factor()
{
    return s_min_load_factor;
}

float StringTable::get_max_load_factor()
{
    return s_max_load_factor;
}

float StringTable::get_deleted_nonused_rehash_ratio()
{
    return s_deleted_nonused_rehash_ratio;
}

void StringTable::set_resize_factor(float new_resize_factor)
{
    s_resize_factor = std::max((float)fabs(new_resize_factor), 1.1f);
}

void StringTable::set_min_load_factor(float new_min_load_factor)
{
    s_min_load_factor = std::max((float)fabs(new_min_load_factor), 0.05f);
}

void StringTable::set_max_load_factor(float new_max_load_factor)
{
    s_max_load_factor = std::min((float)fabs(new_max_load_factor), 0.9f);
}

void StringTable::set_deleted_nonused_rehash_ratio(float new_rehash_ratio)
{
    s_deleted_nonused_rehash_ratio = std::max((float)fabs(new_rehash_ratio), 0.9f);
}

void StringTable::gc()
{
    garbage_collect();
}

__attribute__ ((destructor)) void finalize_string_table()
{
    StringTable::finalize();
}

} /* end namespace string_table_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

//
// StringTable.cpp ends here
