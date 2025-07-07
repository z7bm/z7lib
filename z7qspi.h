//------------------------------------------------------------------------------
//
//    Xilinx zynq-7000/arm-none-eabi QSPI Support Header
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

#ifndef PS7QSPI_H
#define PS7QSPI_H

#include "z7common.h"
#include <ps7mmrs.h>

//------------------------------------------------------------------------------
//
//    Quad-SPI
//
//    Notes:
//    ~~~~~
//    Default initialization brings modude to I/O Flash Mode with manual start
//    transfer. That is,
//
//        CONFIG_REG[MAN_START_EN] = 1
//        CONFIG_REG[MANUAL_CS]    = 0
//        CONFIG_REG[PCS]          = 1
//
//    In this mode the user's software should write data into TxFIFO buffer and
//    then issue write to CONFIG_REG[MAN_START_COM] = 1 (write-only bit). This
//    begins transfer: hardware asserts nCS line, send out/receive to data
//    through data I/O lines and deasserts nCS. In this case transfer length
//    is limited with 252 bytes (63 words).
//
//    For full manual control the user's software must turn on nCS line direct
//    control: CONFIG_REG[MANUAL_CS] = 1. This case, transaction consists of
//
//        * load data to TxFIFO buffer;
//        * assert nCS by CONFIG_REG[PCS] = 0;
//        * issue manual start command by CONFIG_REG[MAN_START_COM] = 1;
//        * supply output buffer with data (output data or dummy data
//          in case of flash reading) if need;
//        * deassert nCS by CONFIG_REG[PCS] = 1;
//
//    This method is little bit intrusive but is not limited with transfer
//    length. By the way, even when CONFIG_REG[MANUAL_CS] = 1 if manual start
//    command is issued without manual asserting of nCS line, hardware controls
//    nCS - asserts before data send/receive and deasserts after.
//
class Qspi
{
public:
    Qspi() : cfg_reg(0)
    {
    }

    void init(bool manmode = true);

    void cs_on()  { cfg_reg &= ~QSPI_PCS_MASK; wpa(QSPI_CONFIG_REG, cfg_reg); }
    void cs_off() { cfg_reg |=  QSPI_PCS_MASK; wpa(QSPI_CONFIG_REG, cfg_reg); }

    void man_cs_enable()  { cfg_reg &= ~QSPI_MANUAL_CS_MASK;     wpa(QSPI_CONFIG_REG, cfg_reg); }
    void man_cs_disable() { cfg_reg |=  QSPI_MANUAL_CS_MASK;     wpa(QSPI_CONFIG_REG, cfg_reg); }
    void start_transfer() { cfg_reg |=  QSPI_MAN_START_COM_MASK; wpa(QSPI_CONFIG_REG, cfg_reg); }


    enum CommandCode : uint8_t
    {
        // status
        cmdREAD_ID   = 0x90,
        cmdRDID      = 0x9f,
        cmdRES       = 0xab,

        // register access
        cmdRDSR1     = 0x05,
        cmdRDSR2     = 0x35,
        cmdWRR       = 0x01,
        cmdWRDI      = 0x04,
        cmdWREN      = 0x06,
        cmdCLSR      = 0x30,
        cmdABRD      = 0x14,
        cmdABWR      = 0x15,
        cmdBRRD      = 0x16,
        cmdBRWR      = 0x17,
        cmdBRAC      = 0xb9,
        cmdDLPRD     = 0x41,
        cmdPNVDLR    = 0x43,
        cmdWVDLR     = 0x4a,

        // read flash array
        cmdREAD      = 0x03,
        cmdFAST_READ = 0x0b,
        cmdDOR       = 0x3b,
        cmdQOR       = 0x6b,
        cmdDIOR      = 0xbb,
        cmdQIOR      = 0xeb,

        // program flash array
        cmdPP        = 0x02,
        cmdQPP       = 0x32,

        // erase flash array
        cmdEB4K       = 0x20,     // erase block 4k
        cmdEB32K      = 0x52,     // erase block 32k
        cmdEB64K      = 0xd8,     // erase block 64k
        cmdCE         = 0x60      // chip erase
    };

    enum BufSize
    {
        FIFO_SIZE = 63,           // words
        PAGE_SIZE = 64            // words
    };

    enum StatusRegBitMask
    {
        SRWD  = 1ul << 7,
        P_ERR = 1ul << 6,
        E_ERR = 1ul << 5,
        BP2   = 1ul << 4,
        BP1   = 1ul << 3,
        BP0   = 1ul << 2,
        WEL   = 1ul << 1,
        WIP   = 1ul << 0
    };

public:
    uint16_t read_id();
    uint8_t  read_sr();
    uint8_t  read_sr2();
    uint8_t  wren();
    void     wrr(uint16_t regs);   // regs[7:0] - SR; regs[15:8] - CR

    uint32_t read (const uint32_t addr, uint8_t * const dst, uint32_t count);
    void     write(const uint32_t addr, const uint8_t *data, const uint32_t count);
    void     erase(const uint32_t addr, const CommandCode = cmdEB64K);

private:
    INLINE bool wip() { return read_sr() & WIP; }
    void program_page  (const uint32_t addr, const uint32_t *data);
    void fill_tx_fifo  (const uint32_t count, const uint32_t pattern = 0);
    void write_tx_fifo (const uint32_t *data, const uint32_t count);
    void read_rx_fifo  (uint8_t * const dst, const uint32_t count);
    void flush_rx_fifo ();

private:
    volatile  uint32_t cfg_reg;     // "cache" access to QSPI_CONFIG_REG
};
//------------------------------------------------------------------------------

#endif  // PS7QSPI_H

