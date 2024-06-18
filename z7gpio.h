//------------------------------------------------------------------------------
//
//    Xilinx zynq-7000/arm-none-eabi GPIO Support Header
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

#ifndef PS7GPIO_H
#define PS7GPIO_H

#include <stdint.h>
#include <ps7mmrs.h>
#include <z7common.h>

//------------------------------------------------------------------------------

namespace gpio
{

//------------------------------------------------------------------------------
template<typename T>
constexpr void pin_on(const T id)
{
    auto     reg        = GPIO_MASK_DATA_0_LSW_REG + id/16;
    uint32_t num        = id%16;
    uint32_t clear_mask = ~(1ul << num) << 16;
    uint32_t set_mask   =   1ul << num;
    
    wrpa(reg, clear_mask | set_mask);
}
//------------------------------------------------------------------------------
template<typename T>
constexpr void pin_off(const T id)
{
    auto     reg        = GPIO_MASK_DATA_0_LSW_REG + id/16;
    uint32_t num        = id%16;
    uint32_t clear_mask = ~(1ul << num) << 16;

    wrpa(reg, clear_mask);
}
//------------------------------------------------------------------------------
bool pin_is_set(const uint32_t id)
{
    return rdpa(GPIO_DATA_0_REG) & (1ul << id);
}
//------------------------------------------------------------------------------


}

//------------------------------------------------------------------------------

#endif // PS7GPIO_H
//------------------------------------------------------------------------------

