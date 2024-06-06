//------------------------------------------------------------------------------
//
//      Project:   Any
//
//      File:      GPIO support
//
//      Version 1.0
//
//      Copyright (c) 2024, Harry E. Zhurov
//
//------------------------------------------------------------------------------

#ifndef Z7GPIO_H
#define Z7GPIO_H

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

#endif // Z7GPIO_H
//------------------------------------------------------------------------------

