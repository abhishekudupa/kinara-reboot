// StringTable.hpp ---
// Filename: StringTable.hpp
// Author: Abhishek Udupa
// Created: Wed Feb 18 00:53:02 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_CONTAINERS_STRING_TABLE_HPP_
#define KINARA_KINARA_COMMON_CONTAINERS_STRING_TABLE_HPP_

#include "../basetypes/KinaraBase.hpp"

namespace kinara {
namespace containers {

namespace string_detail_ {

// forward declaration
class StringRepr;

} /* end namespace string_detail_ */

namespace string_table_detail_ {

namespace kcsd = kinara::containers::string_detail_;
using kcsd::StringRepr;

class StringTable
{
public:
    // default values for configurable parameters
    static constexpr float s_default_resize_factor = 1.618f;
    static constexpr float s_default_max_load_factor = 0.7f;
    static constexpr float s_default_min_load_factor = 0.05f;

private:
    StringTable() = delete;
    StringTable(const StringTable&) = delete;
    StringTable(StringTable&&) = delete;
    StringTable& operator = (const StringTable&) = delete;
    StringTable& operator = (StringTable&&) = delete;

    // constants used as deleted and unused values in the hash table
    static constexpr u64 s_nonused_slot_marker = (0x0);
    static constexpr u64 s_deleted_slot_marker = (0x1);

    // actually configurable parameters, which are set to the
    // values above by default
    static float s_resize_factor;
    static float s_max_load_factor;
    static float s_min_load_factor;

    static inline StringRepr**& hash_table();
    static inline u64& hash_table_size();
    static inline u64& hash_table_used();

    static inline bool is_slot_nonused(const StringRepr* slot_ptr);
    static inline bool is_slot_deleted(const StringRepr* slot_ptr);

    static inline const StringRepr* find(const char* string_value, u64 length);
    static inline const StringRepr* insert_into_table(const char* string_value,
                                                      u64 length,
                                                      StringRepr** string_table,
                                                      u64 table_size);
    static inline const StringRepr* move_into_table(StringRepr* repr_ptr,
                                                    StringRepr** string_table,
                                                    u64 table_size);
    static inline const StringRepr* insert(const char* string_value, u64 length);

    static inline void expand_table();
    static inline void garbage_collect();

public:
    static const StringRepr* get_repr(const char* string_value, u64 length);
    static void finalize();

    // configuration
    static void set_resize_factor(float new_resize_factor);
    static void set_min_load_factor(float new_min_load_factor);
    static void set_max_load_factor(float new_max_load_factor);
    static void gc();
};

} /* end namespace string_table_detail_ */
} /* end namespace containers */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_CONTAINERS_STRING_TABLE_HPP_ */

//
// StringTable.hpp ends here
