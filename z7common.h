//------------------------------------------------------------------------------
//
//    Xilinx zynq-7000/arm-none-eabi Common Stuff Header
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

#ifndef PS7COMMON_H
#define PS7COMMON_H

#include <stdint.h>
#include <ps7slcr.h>

#ifndef INLINE
#define INLINE __attribute__((__always_inline__)) inline
#endif

//------------------------------------------------------------------------------
//
//    Physical address access
// 
//        suffix 'pa' means 'physical address'
//        wr: write
//        rd: read
//        cb: clear bits
//        sb: set bits
//
INLINE void     wrpa(uintptr_t addr, const uint32_t data) { *( reinterpret_cast<volatile uint32_t*>(addr) ) =  data;  }
INLINE uint32_t rdpa (uintptr_t addr)                     { return *( reinterpret_cast<volatile uint32_t*>(addr) );   }
INLINE void     cbpa(uintptr_t addr, const uint32_t mask) { *( reinterpret_cast<volatile uint32_t*>(addr) ) &= ~mask; }
INLINE void     sbpa(uintptr_t addr, const uint32_t mask) { *( reinterpret_cast<volatile uint32_t*>(addr) ) |=  mask; }
//------------------------------------------------------------------------------
INLINE void sbpa(uintptr_t addr, const uint32_t mask, const uint32_t bfmask)
{
    uint32_t reg = *( reinterpret_cast<volatile uint32_t*>(addr) );

    reg &= ~bfmask;
    reg |=  mask;
    *( reinterpret_cast<volatile uint32_t*>(addr) ) = reg;
}
//------------------------------------------------------------------------------
//
//    Intrinsics
//
INLINE void __nop() { __asm__ __volatile__("    nop"); }

INLINE void __wfi() { __asm__ __volatile__("    wfi"); }
INLINE void __wfe() { __asm__ __volatile__("    wfe"); }
INLINE void __sev() { __asm__ __volatile__("    sev"); }

INLINE void __dmb() { __asm__ __volatile__("    dmb"); }
INLINE void __dsb() { __asm__ __volatile__("    dsb"); }
INLINE void __isb() { __asm__ __volatile__("    isb"); }

INLINE uint_fast8_t __clz(uint32_t val) 
{
  uint_fast8_t res;
  __asm__ __volatile__ ("clz %0, %1" : "=r" (res) : "r" (val) );
  return(res);
}

//------------------------------------------------------------------------------
INLINE void slcr_lock()   { wrpa(SLCR_LOCK_REG,   0x767B); }
INLINE void slcr_unlock() { wrpa(SLCR_UNLOCK_REG, 0xDF0D); }
//------------------------------------------------------------------------------

#endif  // PS7COMMON_H

