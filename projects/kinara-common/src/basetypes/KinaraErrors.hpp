// KinaraErrors.hpp ---
// Filename: KinaraErrors.hpp
// Author: Abhishek Udupa
// Created: Fri Feb 20 15:09:42 2015 (-0500)
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

// This file makes no assumptions about any kinara types being
// available and only provides low level debugging and error
// handling capabilities

#if !defined KINARA_KINARA_COMMON_BASETYPES_KINARA_ERRORS_HPP_
#define KINARA_KINARA_COMMON_BASETYPES_KINARA_ERRORS_HPP_

#include <cstdio>

namespace kinara {
namespace error_handlers {
extern void notify_assertion_violation(const char* function_name,
                                       const char* filename, int line,
                                       const char* condition,
                                       const char* message);
extern void invoke_debugger();

} /* end namespace error_handlers */
} /* end namespace kinara */

// Assertions are always enabled in debug builds
// Assertions may be optionally enabled in non-debug builds
// If we're in debug mode, an assertion failure notifies failure
// on stderr and invokes the debugger.
// Otherwise, IF assertions are enabled, we only notify
// about the assertion failure on stderr

#if defined KINARA_CFG_DEBUG_MODE_BUILD_

#define KINARA_ASSERT_WITH_MSG(CONDITION__, MESSAGE__)                        \
    if (!(CONDITION__)) {                                                     \
        kinara::error_handlers::notify_assertion_violation(__FUNCTION__,      \
                                                           __FILE__,          \
                                                           __LINE__,          \
                                                           #CONDITION__,      \
                                                           MESSAGE__);        \
        kinara::error_handlers::invoke_debugger();                            \
    }                                                                         \
    ((void)0)

#elif defined KINARA_CFG_ASSERTIONS_ENABLED_ /* !KINARA_CFG_DEBUG_MODE_BUILD_ */

#define KINARA_ASSERT_WITH_MSG(CONDITION__, MESSAGE__)                        \
    if (!(CONDITION__)) {                                                     \
        notify_assertion_violation(__FUNCTION__,                              \
                                   __FILE__,                                  \
                                   __LINE__,                                  \
                                   #CONDITION__,                              \
                                   MESSAGE__);                                \
    }                                                                         \
    ((void)0)

#else /* !KINARA_CFG_DEBUG_MODE_BUILD_ && !KINARA_CFG_ASSERTIONS_ENABLED_ */

#define KINARA_ASSERT_WITH_MSG(CONDITION__, MESSAGE__)                        \
    ((void)0)

#endif /* !KINARA_CFG_ASSERTIONS_ENABLED_ && !KINARA_CFG_ASSERTIONS_ENABLED_ */


#define KINARA_ASSERT(CONDITION__)                                      \
    KINARA_ASSERT_WITH_MSG(CONDITION__, nullptr)

#if defined KINARA_CFG_DEBUG_MODE_BUILD_

#define KINARA_UNREACHABLE_CODE()                                       \
    KINARA_ASSERT_WITH_MSG(false, "Error: Code that should never have " \
                           "been reached was executed!");               \
    __builtin_unreachable();                                            \
    (void(0))

#else /* !KINARA_CFG_DEBUG_MODE_BUILD_ */

// Non-debug mode build, trigger a crash!
#define KINARA_UNREACHABLE_CODE()                                       \
    KINARA_ASSERT_WITH_MSG(false, "Error: Code that should never have " \
                           "been reached was executed!");               \
    {                                                                   \
        int* int_ptr__ = nullptr;                                       \
        *int_ptr__ = 0xDEADBEEF;                                        \
    }                                                                   \
    __builtin_unreachable();                                            \
    (void(0))

#endif /* KINARA_CFG_DEBUG_MODE_BUILD_ */

#endif /* KINARA_KINARA_COMMON_BASETYPES_KINARA_ERRORS_HPP_ */

//
// KinaraErrors.hpp ends here
