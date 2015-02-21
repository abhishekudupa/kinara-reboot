// Timers.hpp ---
// Filename: Timers.hpp
// Author: Abhishek Udupa
// Created: Sat Feb 21 13:14:16 2015 (-0500)
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

#if !defined KINARA_KINARA_COMMON_TIMERS_TIMERS_HPP_
#define KINARA_KINARA_COMMON_TIMERS_TIMERS_HPP_

#include <csignal>
#include <ctime>
#include <functional>

#include "../basetypes/KinaraBase.hpp"
#include "../basetypes/KinaraTypesFwd.hpp"

namespace kinara {
namespace utils {

enum class ClockIDType {
    RealtimeClock,
    MonotonicClock,
    ProcessCPUTimeClock,
    ThreadCPUTimeClock
};

#if defined KINARA_CFG_HAVE_LIBRT_

class ScopedTimer
{
private:
    bool m_use_event_handler;
    bool m_repeat_timer;
    union HandlerType {
        std::function<void()> m_event_handler;
        const Interruptible* m_interruptible_object;

        inline HandlerType()
        {
            // Nothing here
        }

        inline ~HandlerType()
        {
            // Nothing here
        }
    };

    HandlerType m_handler;
    timer_t m_timer;

    static inline void handle_timer_expiration(union sigval the_sigval);
    inline clockid_t get_clock_id(ClockIDType clock_id_type) const;
    inline void register_and_arm_timer(ClockIDType clock_to_use,
                                       u64 timer_interval_in_ms);

public:
    ScopedTimer(const std::function<void()>& event_handler,
                u64 timer_interval_in_ms,
                ClockIDType clock_to_use = ClockIDType::ProcessCPUTimeClock,
                bool repeat_timer = false);
    ScopedTimer(const Interruptible* interruptible_ptr,
                u64 timer_interval_in_ms,
                ClockIDType clock_to_use = ClockIDType::ProcessCPUTimeClock,
                bool repeat_timer = false);
    ScopedTimer(const Interruptible& interruptible_ref,
                u64 timer_interval_in_ms,
                ClockIDType clock_to_use = ClockIDType::ProcessCPUTimeClock,
                bool repeat_timer = false);

    ScopedTimer(const ScopedTimer& other) = delete;
    ScopedTimer(ScopedTimer&& other) = delete;
    ScopedTimer() = delete;

    ScopedTimer& operator = (const ScopedTimer& other) = delete;
    ScopedTimer& operator = (ScopedTimer&& other) = delete;

    ~ScopedTimer();
};

#else /* !KINARA_CFG_HAVE_LIBRT_ */

// A dummy class in case we don't have librt
class ScopedTimer
{
public:
    inline ScopedTimer(const std::function<void()>& event_handler,
                       u64 timer_interval_in_ms,
                       ClockIDType clock_to_use = ClockIDType::ProcessCPUTimeClock,
                       bool repeat_timer = false)
    {
        // nothing here
    }

    inline ScopedTimer(const Interruptible* interruptible_ptr,
                       u64 timer_interval_in_ms,
                       ClockIDType clock_to_use = ClockIDType::ProcessCPUTimeClock,
                       bool repeat_timer = false)
    {
        // Nothing here
    }

    inline ScopedTimer(const Interruptible& interruptible_ref,
                       u64 timer_interval_in_ms,
                       ClockIDType clock_to_use = ClockIDType::ProcessCPUTimeClock,
                       bool repeat_timer = false)
    {
        // Nothing here
    }

    ScopedTimer(const ScopedTimer& other) = delete;
    ScopedTimer(ScopedTimer&& other) = delete;
    ScopedTimer() = delete;

    ScopedTimer& operator = (const ScopedTimer& other) = delete;
    ScopedTimer& operator = (ScopedTimer&& other) = delete;

    inline ~ScopedTimer()
    {
        // Nothing here
    }
};

#endif /* KINARA_CFG_HAVE_LIBRT_ */

} /* end namespace utils */
} /* end namespace kinara */

#endif /* KINARA_KINARA_COMMON_TIMERS_TIMERS_HPP_ */

//
// Timers.hpp ends here
