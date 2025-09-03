//------------------------------------------------------------------------------
//
//    Xilinx zynq-7000/arm-none-eabi QSPI Support Source
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

#include <z7qspi.h>

//------------------------------------------------------------------------------
void Qspi::init(bool manmode)
{
    wpa(QSPI_EN_REG, 0); // disable QSPI module

    // reset QSPI clocks
    slcr_unlock();
    const uint32_t RST_CLK_MASK = QSPI_RST_CTRL_REF_RST_MASK | QSPI_RST_CTRL_CPU1X_RST_MASK;
    wpa(QSPI_RST_CTRL_REG, RST_CLK_MASK);
    wpa(QSPI_RST_CTRL_REG, 0);
    slcr_lock();

    wpa(QSPI_RX_THRES_REG, 1);
    wpa(QSPI_TX_THRES_REG, 1);

    // set up configuration
    cbpa(QSPI_LQSPI_CFG_REG, QSPI_LQ_MODE_MASK);  // turn off linear mode

    const uint32_t MAN_MODE = manmode ? (QSPI_MAN_START_EN_MASK | QSPI_MANUAL_CS_MASK) : 0;
    const uint32_t SET_MASK = QSPI_IFMODE_MASK     +     //  flash interface in Flash I/O Mode
                              MAN_MODE             +     //
                              QSPI_PCS_MASK        +     //  set nCS to 1
                              QSPI_FIFO_WIDTH_MASK +     //  0b11: 32 bit, the only this value supported
                              QSPI_MODE_SEL_MASK   +     //  Master Mode on
                              QSPI_HOLDB_DR_MASK   +     //
                             (1ul << QSPI_BAUD_RATE_DIV_BPOS) +
                              QSPI_CLK_PH_MASK     +     //
                              QSPI_CLK_POL_MASK;         //


    const uint32_t CLR_MASK = QSPI_BAUD_RATE_DIV_MASK +  //  set value 000: divide by 2
                              (7ul << 11)             +  //  reserved, 0
                              QSPI_ENDIAN_MASK        +  //  little endian
                              QSPI_REF_CLK_MASK;         //  reserved, must be 0
                                                         //
    cfg_reg = rpa(QSPI_CONFIG_REG);
    cfg_reg &= ~CLR_MASK;
    cfg_reg |=  SET_MASK;

    wpa(QSPI_CONFIG_REG, cfg_reg);
    wpa(QSPI_EN_REG, 1);                            // enable QSPI module


}
//------------------------------------------------------------------------------
uint16_t Qspi::read_id()
{
    wpa(QSPI_RX_THRES_REG, 2);
    cs_on();
    wpa(QSPI_TXD0_REG,  cmdREAD_ID);
    wpa(QSPI_TXD2_REG,  0);
    start_transfer();
    while( !(rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    rpa(QSPI_RX_DATA_REG);
    return rpa(QSPI_RX_DATA_REG) >> 16;
}
//------------------------------------------------------------------------------
uint8_t Qspi::read_sr()
{
    wpa(QSPI_RX_THRES_REG, 1);
    cs_on();
    wpa(QSPI_TXD2_REG,  cmdRDSR1);
    start_transfer();
    while( !(rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    return rpa(QSPI_RX_DATA_REG) >> 24;
}
//------------------------------------------------------------------------------
uint8_t Qspi::read_sr2()
{
    wpa(QSPI_RX_THRES_REG, 1);
    cs_on();
    wpa(QSPI_TXD2_REG,  cmdRDSR2);
    start_transfer();
    while( !(rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    return rpa(QSPI_RX_DATA_REG) >> 24;
}
//------------------------------------------------------------------------------
uint8_t Qspi::wren()
{
    wpa(QSPI_RX_THRES_REG, 1);
    cs_on();
    wpa(QSPI_TXD1_REG,  cmdWREN);
    start_transfer();
    while( ! (rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    return rpa(QSPI_RX_DATA_REG);
}
//------------------------------------------------------------------------------
void Qspi::wrr(uint16_t regs)      // regs[7:0] - SR; regs[15:8] - CR
{
    wren();
    wpa(QSPI_RX_THRES_REG, 1);
    cs_on();
    wpa(QSPI_TXD3_REG,  cmdWRR + ( regs << 8) );
    start_transfer();
    while( ! (rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    rpa(QSPI_RX_DATA_REG);
}
//------------------------------------------------------------------------------
void Qspi::erase(const uint32_t addr, const CommandCode cmd)
{
    wren();
    wpa(QSPI_RX_THRES_REG, 1);
    cs_on();
    uint32_t rev_addr = __builtin_bswap32(addr) >> 8;
    wpa(QSPI_TXD0_REG,  cmd + ( rev_addr << 8) );
    start_transfer();
    while( ! (rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    rpa(QSPI_RX_DATA_REG);
    while( wip() ) { }
}
//------------------------------------------------------------------------------
void Qspi::program_page(const uint32_t addr, const uint32_t *data)
{
    wren();
    cs_on();
    uint32_t rev_addr = __builtin_bswap32(addr) >> 8;
    wpa(QSPI_TXD0_REG,  cmdQPP + ( rev_addr << 8) );

    const uint32_t CHUNK0 = 16;
    const uint32_t CHUNK1 = PAGE_SIZE - CHUNK0;


    wpa( QSPI_TX_THRES_REG, FIFO_SIZE -  CHUNK1 );
    write_tx_fifo(data, CHUNK0);

    start_transfer();

    while( !(rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_TX_FIFO_NOT_FULL_MASK) ) { }
    flush_rx_fifo();
    write_tx_fifo(data + CHUNK0, CHUNK1);
    start_transfer();

    wpa(QSPI_TX_THRES_REG, 1);
    while( !(rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_TX_FIFO_NOT_FULL_MASK) ) { }

    cs_off();
    flush_rx_fifo();
    while( wip() ) { }
}
//------------------------------------------------------------------------------
void Qspi::write(const uint32_t addr, const void *data, const uint32_t count)
{
    const uint32_t WCOUNT = count/4 + (count%4 ? 1 : 0);
    const uint32_t CHUNKS = WCOUNT/PAGE_SIZE + (WCOUNT%PAGE_SIZE ? 1 : 0);
    const uint32_t *p     = reinterpret_cast<const uint32_t *>(data);

    for(uint32_t i = 0; i < CHUNKS; ++i)
    {
        program_page(addr + i*PAGE_SIZE*sizeof(uint32_t), p + i*PAGE_SIZE);
    }
}
//------------------------------------------------------------------------------
uint32_t Qspi::read(const uint32_t addr, void * const pdst, uint32_t count)
{
    if(!count)
        return 0;

    uint8_t * const dst = reinterpret_cast<uint8_t * const>(pdst);

    cs_on();

    // issue command/address
    wpa(QSPI_RX_THRES_REG, 1);
    wpa(QSPI_TXD1_REG,  cmdQOR);
    start_transfer();
    while( !(rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    rpa(QSPI_RX_DATA_REG);      // drop command/address response

    uint32_t rev_addr = __builtin_bswap32(addr);
    wpa(QSPI_TXD0_REG, rev_addr >> 8 );
    start_transfer();
    while( !(rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    rpa(QSPI_RX_DATA_REG);      // drop dummy data response

    // data transfer
    uint32_t wcount = count/4 + (count%4 ? 1 : 0);

    uint32_t rchunk;
    uint32_t rx_idx = 0;
    for(;;)
    {
        rchunk = 0;
        while( !(rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_TX_FIFO_FULL_MASK) )
        {
            wpa(QSPI_TXD0_REG, 0);
            ++rchunk;
            if(--wcount == 0)
            {
                break;
            }
        }
        start_transfer();
        
        wpa(QSPI_RX_THRES_REG, rchunk);

        while(!(rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK)) { }
        while(!(rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK)) { }
        
        read_rx_fifo(dst + rx_idx*sizeof(uint32_t), rchunk);
        rx_idx += rchunk;
        
        if(!wcount)
        {
            break;
        }
    }

    cs_off();

    return rx_idx*sizeof(uint32_t);
}
//------------------------------------------------------------------------------
void Qspi::fill_tx_fifo(const uint32_t count, const uint32_t pattern)
{
    for(uint32_t i = 0; i < count; ++i)
    {
        wpa(QSPI_TXD0_REG, pattern);
    }
}
//------------------------------------------------------------------------------
void Qspi::write_tx_fifo(const uint32_t *data, const uint32_t count)
{
    for(uint32_t i = 0; i < count; ++i)
    {
        wpa(QSPI_TXD0_REG, data[i]);
    }
}
//------------------------------------------------------------------------------
void Qspi::read_rx_fifo(uint8_t * const dst, const uint32_t count)
{
    for(uint32_t i = 0; i < count; ++i)
    {
        uint32_t val = rpa(QSPI_RX_DATA_REG);
        dst[i*sizeof(uint32_t) + 0] = val;
        dst[i*sizeof(uint32_t) + 1] = val >> 8;
        dst[i*sizeof(uint32_t) + 2] = val >> 16;
        dst[i*sizeof(uint32_t) + 3] = val >> 24;
    }
}
//------------------------------------------------------------------------------
void Qspi::flush_rx_fifo()
{
    wpa(QSPI_RX_THRES_REG, 1);
    while( rpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK )
    {
        rpa(QSPI_RX_DATA_REG);
    }
}
//------------------------------------------------------------------------------


