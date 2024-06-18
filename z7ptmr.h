//------------------------------------------------------------------------------
//
//    Xilinx zynq-7000/arm-none-eabi CPU Private Timer Stuff Header
//
//    Version 1.0
//
//    Permission is hereby granted, free of charge, to any person obtaining
//    a copy of this software and associated documentation files (the
//    "Software"), to deal in the Software without restriction, including
//    without limitation the rights to use, copy, modify, merge, publish,
//    distribute, sublicense, and/or sell copies of the Software, and to
//    permit persons to whom the Software is furnished to do so, subject to
//    the following conditions:
//
//    The above copyright notice and this permission notice shall be included
//    in all copies or substantial portions of the Software.
//
//    THE SOFTWARE  IS PROVIDED  "AS IS", WITHOUT  WARRANTY OF  ANY KIND,
//    EXPRESS  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
//    THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//    Copyright (c) 2017-2024, Zynq-7000 Bare-metal Project
//    -----------------------------------------------------
//    Project sources: https://github.com/z7bm
//
//------------------------------------------------------------------------------

#ifndef PS7CPUPTMR_H
#define PS7CPUPTMR_H

#include <stdint.h>
#include <ps7mmrs.h>
#include <z7common.h>

struct PrivateTimer
{
    // Calculate and set private timer reload value implying prescaler value is 0
    //
    //  Private timer interval equation:
    //
    //      T = (Prescaler + 1)(Reload + 1)/fclk
    //
    //  Because Prescaler = 0 Reload value is:
    //
    //      Reload = T*fclk - 1
    //
    //  The following function accept clock frequency in MHz and time inteval in ms

    template<typename T> constexpr static void set_reload_value(T f, T t)  // f: MHz, t: us
    {
        wrpa(PTMR_LOAD_REG, f*t - 1);
    }

    static void start()
    {
        wrpa(PTMR_CTLR_REG, PTMR_CTLR_TIMER_ENABLE_MASK |
                            PTMR_CTLR_AUTO_RELOAD_MASK  |
                            PTMR_CTLR_IRQ_ENABLE_MASK);
    }
};
//------------------------------------------------------------------------------

#endif // PS7CPUPTMR_H
//------------------------------------------------------------------------------

