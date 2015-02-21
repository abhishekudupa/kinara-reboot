// Timers.cpp ---
// Filename: Timers.cpp
// Author: Abhishek Udupa
// Created: Sat Feb 21 13:14:20 2015 (-0500)
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

#include <cstring>
// load config for KINARA_CFG_HAVE_LIBRT_
#include <KinaraConfig.h>

#include "../basetypes/KinaraTypes.hpp"
#include "../basetypes/KinaraErrors.hpp"

#include "Timers.hpp"

namespace kinara {
namespace utils {

#if defined KINARA_CFG_HAVE_LIBRT_

inline void ScopedTimer::handle_timer_expiration(union sigval the_sigval)
{
    auto ptr = the_sigval.sival_ptr;
    auto timer_ptr = static_cast<ScopedTimer*>(ptr);
    if (timer_ptr->m_use_event_handler) {
        timer_ptr->m_handler.m_event_handler();
    } else {
        timer_ptr->m_handler.m_interruptible_object->interrupt();
    }
}

inline clockid_t ScopedTimer::get_clock_id(ClockIDType clock_to_use) const
{
    switch(clock_to_use) {
    case ClockIDType::RealtimeClock:
        return CLOCK_REALTIME;
    case ClockIDType::MonotonicClock:
        return CLOCK_MONOTONIC;
    case ClockIDType::ProcessCPUTimeClock:
        return CLOCK_PROCESS_CPUTIME_ID;
    case ClockIDType::ThreadCPUTimeClock:
        return CLOCK_THREAD_CPUTIME_ID;
    default:
        KINARA_UNREACHABLE_CODE();
    }
}

inline void ScopedTimer::register_and_arm_timer(ClockIDType clock_to_use, u64 timer_interval_in_ms)
{
    struct sigevent sigev;
    memset(&sigev, 0, sizeof(struct sigevent));
    auto clock_id = get_clock_id(clock_to_use);

    sigev.sigev_notify = SIGEV_THREAD;
    sigev.sigev_value.sival_ptr = this;
    sigev.sigev_notify_function = ScopedTimer::handle_timer_expiration;
    auto status = timer_create(clock_id, &sigev, &m_timer);
    if (status) {
        throw KinaraException("Could not create timer!");
    }

    struct itimerspec ts;
    ts.it_value.tv_sec = timer_interval_in_ms / 1000;
    ts.it_value.tv_nsec = (timer_interval_in_ms % 1000) * 1000000;
    if (m_repeat_timer) {
        ts.it_interval.tv_sec = ts.it_value.tv_sec;
        ts.it_interval.tv_nsec = ts.it_value.tv_nsec;
    } else {
        ts.it_interval.tv_sec = 0;
        ts.it_interval.tv_nsec = 0;
    }

    if (timer_settime(m_timer, 0, &ts, nullptr) == -1) {
        throw KinaraException("Could not set timer!");
    }
}

ScopedTimer::ScopedTimer(const std::function<void()>& event_handler,
                         u64 timer_interval_in_ms,
                         ClockIDType clock_to_use,
                         bool repeat_timer)
    : m_use_event_handler(true), m_repeat_timer(repeat_timer)
{
    m_handler.m_event_handler = event_handler;
    register_and_arm_timer(clock_to_use, timer_interval_in_ms);
}

ScopedTimer::ScopedTimer(const Interruptible* interruptible_ptr,
                         u64 timer_interval_in_ms,
                         ClockIDType clock_to_use,
                         bool repeat_timer)
    : m_use_event_handler(false), m_repeat_timer(repeat_timer)
{
    m_handler.m_interruptible_object = interruptible_ptr;
    register_and_arm_timer(clock_to_use, timer_interval_in_ms);
}

ScopedTimer::ScopedTimer(const Interruptible& interruptible_ref,
                         u64 timer_interval_in_ms,
                         ClockIDType clock_to_use,
                         bool repeat_timer)
    : m_use_event_handler(false), m_repeat_timer(repeat_timer)
{
    m_handler.m_interruptible_object = &interruptible_ref;
    register_and_arm_timer(clock_to_use, timer_interval_in_ms);
}

ScopedTimer::~ScopedTimer()
{
    timer_delete(m_timer);
}

#endif /* KINARA_CFG_HAVE_LIBRT_ */

} /* end namespace utils */
} /* end namespace kinara */

//
// Timers.cpp ends here
