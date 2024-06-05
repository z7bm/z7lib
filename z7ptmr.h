//------------------------------------------------------------------------------
//
//      Project:   Any
//
//      File:      CPU Private Timer
//
//      Version 1.0
//
//      Copyright (c) 2024, Harry E. Zhurov
//
//------------------------------------------------------------------------------

#ifndef Z7CPUPTMR_H
#define Z7CPUPTMR_H

#include <stdint.h>
#include <ps7mmrs.h>
#include <z7common.h>
#include "usrlib.h"


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

    template<typename T> constexpr static void set_reload_value(T f, T t)  // f: MHz, t: ms
    {
        wrpa(PTMR_LOAD_REG, f*t*1000 - 1);
    }

    static void start()
    {
        wrpa(PTMR_CTLR_REG, PTMR_CTLR_TIMER_ENABLE_MASK |
                            PTMR_CTLR_AUTO_RELOAD_MASK  |
                            PTMR_CTLR_IRQ_ENABLE_MASK);
    }
};
//------------------------------------------------------------------------------

#endif // Z7CPUPTMR_H
//------------------------------------------------------------------------------

