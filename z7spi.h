//------------------------------------------------------------------------------
//
//    Xilinx zynq-7000/arm-none-eabi SPI Support Header
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
//    Copyright (c) 2025, Zynq-7000 Bare-metal Project
//    -----------------------------------------------------
//    Project sources: https://github.com/z7bm
//
//------------------------------------------------------------------------------

#ifndef PS7SPI_H
#define PS7SPI_H

#include <stdint.h>
#include <ps7mmrs.h>
#include <z7common.h>

//------------------------------------------------------------------------------
class Spi
{
public:
    struct Regs
    {
        uint32_t CONFIG_REG;            //  32    mixed    0x00020000    SPI configuration register
        uint32_t INT_STS_REG;           //  32    mixed    0x00000004    SPI interrupt status register
        uint32_t INT_EN_REG;            //  32    mixed    0x00000000    Interrupt Enable register
        uint32_t INT_DIS_REG;           //  32    mixed    0x00000000    Interrupt disable register
        uint32_t INT_MASK_REG;          //  32    ro       0x00000000    Interrupt mask register
        uint32_t EN_REG;                //  32    mixed    0x00000000    SPI_Enable Register
        uint32_t DELAY_REG;             //  32    rw       0x00000000    Delay Register
        uint32_t TX_DATA_REG;           //  32    wo       0x00000000    Transmit Data Register
        uint32_t RX_DATA_REG;           //  32    ro       0x00000000    Receive Data Register
        uint32_t SLAVE_IDLE_COUNT_REG;  //  32    mixed    0x000000FF    Slave Idle Count Register
        uint32_t TX_THRES_REG;          //  32    rw       0x00000001    TX_FIFO Threshold Register
        uint32_t RX_THRES_REG;          //  32    rw       0x00000001    RX FIFO Threshold Register
        uint32_t MOD_ID_REG;            //  32    ro       0x00090106    Module ID register
    };

public:
    Spi(uintptr_t addr)
           : regs( reinterpret_cast<Regs*>(addr) )
           , busy(false)
    { 
    }
    
    void set_busy(bool x) { busy = x;    }
    bool is_busy() const  { return busy; }
  
    void push_tx(char c)        const { regs->TX_DATA_REG = c; }
    //char pop_rx()               const { return regs->TX_RX_FIFO; }

    void man_start()
    {
        regs->CONFIG_REG |= SPI_MAN_START_COM_MASK;
    }

protected:
    static const uint32_t SPI0_RST_MASK = SPI_RST_CTRL_SPI0_REF_RST_MASK | SPI_RST_CTRL_SPI0_CPU1X_RST_MASK;
    static const uint32_t SPI1_RST_MASK = SPI_RST_CTRL_SPI1_REF_RST_MASK | SPI_RST_CTRL_SPI1_CPU1X_RST_MASK;
        
protected:
    volatile Regs  *regs;
    volatile bool   busy;
};
//------------------------------------------------------------------------------

#endif // PS7SPI_H
//------------------------------------------------------------------------------

