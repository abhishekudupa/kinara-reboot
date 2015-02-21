// KinaraErrors.cpp ---
// Filename: KinaraErrors.cpp
// Author: Abhishek Udupa
// Created: Fri Feb 20 15:29:54 2015 (-0500)
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

#include <sys/types.h>
#include <unistd.h>

#include <KinaraConfig.h>

#include "KinaraErrors.hpp"

namespace kinara {
namespace error_handlers {

volatile bool gdb_is_attached__ = false;

void notify_assertion_violation(const char* function_name,
                                const char* filename,
                                int line,
                                const char* condition,
                                const char* message)
{
    fprintf(stderr,
            "Assertion failure:\n"
            "Function : %s\n"
            "File     : %s\n"
            "Line     : %d\n"
            "Condition: %s\n"
            "Error Msg: %s\n",
            function_name, filename, line,
            (condition != nullptr ? condition : "no information"),
            (message != nullptr ? message : "no information"));
}

void invoke_debugger()
{
#ifdef KINARA_CFG_HAVE_GDB_
    fprintf(stderr, "Invoking Debugger...\n");
    fflush(stderr);

    auto my_pid = getpid();

    if(fork()) {
        // wait for gdb to attach
        while(!gdb_is_attached__) {
            // sleep for 100 ms
            usleep(100000);
        }
        // reset for next time around
        gdb_is_attached__ = false;
        // okay, gdb has attached, break
        asm volatile ("int3;");
    } else {
        char pid_string[1024];
        char exec_name[1024];
        sprintf(pid_string, "%d", my_pid);
        sprintf(exec_name, "/proc/%d/exe", my_pid);

        execl(KINARA_CFG_PATH_TO_GDB_,
              KINARA_CFG_PATH_TO_GDB_,
              exec_name,
              pid_string,
              "-ex",
              "call notify_gdb_attached____()",
              "-ex",
              "continue",
              (char*)nullptr);
    }
#endif /* KINARA_CFG_HAVE_GDB_ */
}

} /* end namespace error_handlers */
} /* end namespace kinara */

// global and with C linkage
// because gdb seems to have trouble
// with namespaces in commands executed
// using -ex
extern "C" void notify_gdb_attached____();

void notify_gdb_attached____()
{
    kinara::error_handlers::gdb_is_attached__ = true;
}

//
// KinaraErrors.cpp ends here
