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
    wrpa(QSPI_EN_REG, 0); // disable QSPI module

    // reset QSPI clocks
    slcr_unlock();
    const uint32_t RST_CLK_MASK = QSPI_RST_CTRL_REF_RST_MASK | QSPI_RST_CTRL_CPU1X_RST_MASK;
    wrpa(QSPI_RST_CTRL_REG, RST_CLK_MASK);
    wrpa(QSPI_RST_CTRL_REG, 0);
    slcr_lock();

    wrpa(QSPI_RX_THRES_REG, 1);
    wrpa(QSPI_TX_THRES_REG, 1);

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
    cfg_reg = rdpa(QSPI_CONFIG_REG);
    cfg_reg &= ~CLR_MASK;
    cfg_reg |=  SET_MASK;
    wrpa(QSPI_CONFIG_REG, cfg_reg);

    wrpa(QSPI_EN_REG, 1);                            // enable QSPI module


}
//------------------------------------------------------------------------------
uint16_t Qspi::read_id()
{
    wrpa(QSPI_RX_THRES_REG, 2);
    cs_on();
    wrpa(QSPI_TXD0_REG,  cmdREAD_ID);
    wrpa(QSPI_TXD2_REG,  0);
    start_transfer();
    while( !(rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    rdpa(QSPI_RX_DATA_REG);
    return rdpa(QSPI_RX_DATA_REG) >> 16;
}
//------------------------------------------------------------------------------
uint8_t Qspi::read_sr()
{
    wrpa(QSPI_RX_THRES_REG, 1);
    cs_on();
    wrpa(QSPI_TXD2_REG,  cmdRDSR1);
    start_transfer();
    while( !(rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    return rdpa(QSPI_RX_DATA_REG) >> 24;
}
//------------------------------------------------------------------------------
uint8_t Qspi::read_cr()
{
    wrpa(QSPI_RX_THRES_REG, 1);
    cs_on();
    wrpa(QSPI_TXD2_REG,  cmdRDCR);
    start_transfer();
    while( !(rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    return rdpa(QSPI_RX_DATA_REG) >> 24;
}
//------------------------------------------------------------------------------
uint8_t Qspi::wren()
{
    wrpa(QSPI_RX_THRES_REG, 1);
    cs_on();
    wrpa(QSPI_TXD1_REG,  cmdWREN);
    start_transfer();
    while( ! (rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    return rdpa(QSPI_RX_DATA_REG);
}
//------------------------------------------------------------------------------
void Qspi::wrr(uint16_t regs)      // regs[7:0] - SR; regs[15:8] - CR
{
    wren();
    wrpa(QSPI_RX_THRES_REG, 1);
    cs_on();
    wrpa(QSPI_TXD3_REG,  cmdWRR + ( regs << 8) );
    start_transfer();
    while( ! (rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    rdpa(QSPI_RX_DATA_REG);
}
//------------------------------------------------------------------------------
void Qspi::erase(const uint32_t addr, const CommandCode cmd)
{
    wren();
    wrpa(QSPI_RX_THRES_REG, 1);
    cs_on();
    uint32_t rev_addr = __builtin_bswap32(addr) >> 8;
    wrpa(QSPI_TXD0_REG,  cmd + ( rev_addr << 8) );
    start_transfer();
    while( ! (rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    cs_off();
    rdpa(QSPI_RX_DATA_REG);
}
//------------------------------------------------------------------------------
void Qspi::program_page(const uint32_t addr, const uint32_t *data)
{
    wren();
    cs_on();
    uint32_t rev_addr = __builtin_bswap32(addr) >> 8;
    wrpa(QSPI_TXD0_REG,  cmdQPP + ( rev_addr << 8) );

    const uint32_t CHUNK0 = 16;
    const uint32_t CHUNK1 = PAGE_SIZE - CHUNK0;


    wrpa( QSPI_TX_THRES_REG, FIFO_SIZE -  CHUNK1 );
    write_tx_fifo(data, CHUNK0);

    start_transfer();

    while( !(rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_TX_FIFO_NOT_FULL_MASK) ) { }
    flush_rx_fifo();
    write_tx_fifo(data + CHUNK0, CHUNK1);
    start_transfer();

    wrpa(QSPI_TX_THRES_REG, 1);
    while( !(rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_TX_FIFO_NOT_FULL_MASK) ) { }

    cs_off();
    flush_rx_fifo();
    while( wip() ) { }
}
//------------------------------------------------------------------------------
void Qspi::write(const uint32_t addr, const uint32_t *data, const uint32_t count)
{
    const uint32_t CHUNKS = count/PAGE_SIZE + (count%PAGE_SIZE ? 1 : 0);

    for(uint32_t i = 0; i < CHUNKS; ++i)
    {
        program_page(addr + i*PAGE_SIZE*sizeof(uint32_t), data + i*PAGE_SIZE);
    }
}
//------------------------------------------------------------------------------
uint32_t Qspi::read(const uint32_t addr, uint32_t * const dst, uint32_t count)
{
    if(!count)
        return 0;

    cs_on();

    // issue command/address
    wrpa(QSPI_RX_THRES_REG, 1);
    wrpa(QSPI_TXD1_REG,  cmdQOR);
    start_transfer();
    while( !(rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    rdpa(QSPI_RX_DATA_REG);      // drop command/address response

    uint32_t rev_addr = __builtin_bswap32(addr);
    wrpa(QSPI_TXD0_REG, rev_addr >> 8 );
    start_transfer();
    while( !(rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK) ) { }
    rdpa(QSPI_RX_DATA_REG);      // drop dummy data response

    const uint32_t CHUNK_SIZE = 32; // words
          uint32_t rchunk;

    // data transfer
    uint32_t rcount = count;
    if(count > 63)
    {
        fill_tx_fifo(63);
        count -= 63;
        wrpa(QSPI_TX_THRES_REG, FIFO_SIZE - CHUNK_SIZE + 1);
        wrpa(QSPI_RX_THRES_REG, CHUNK_SIZE);
        rchunk = CHUNK_SIZE;
    }
    else
    {
        fill_tx_fifo(count);
        wrpa(QSPI_RX_THRES_REG, count);
        rchunk = count;
        count = 0;
    }

    start_transfer();
    uint32_t rx_idx = 0;
    for(;;)
    {
        if( count && (rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_TX_FIFO_NOT_FULL_MASK) )
        {
            if(count > CHUNK_SIZE)
            {
                fill_tx_fifo(CHUNK_SIZE);
                count -= CHUNK_SIZE;
            }
            else
            {
                fill_tx_fifo(count);
                count = 0;
            }
        }

        if( rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK )
        {
            read_rx_fifo(dst + rx_idx, rchunk);
            rx_idx += rchunk;
            rcount -= rchunk;
            if(rcount <= 63)
            {
                if(rcount == 0)
                {
                    break;
                }

                wrpa(QSPI_RX_THRES_REG, rcount);
                rchunk = rcount;
            }
        }
    }

    cs_off();
    
    return rx_idx;
}
//------------------------------------------------------------------------------
void Qspi::fill_tx_fifo(const uint32_t count, const uint32_t pattern)
{
    for(uint32_t i = 0; i < count; ++i)
    {
        wrpa(QSPI_TXD0_REG, pattern);
    }
}
//------------------------------------------------------------------------------
void Qspi::write_tx_fifo(const uint32_t *data, const uint32_t count)
{
    for(uint32_t i = 0; i < count; ++i)
    {
        wrpa(QSPI_TXD0_REG, data[i]);
    }
}
//------------------------------------------------------------------------------
void Qspi::read_rx_fifo(uint32_t * const dst, const uint32_t count)
{
    for(uint32_t i = 0; i < count; ++i)
    {
        dst[i] = rdpa(QSPI_RX_DATA_REG);
    }
}
//------------------------------------------------------------------------------
void Qspi::flush_rx_fifo()
{
    wrpa(QSPI_RX_THRES_REG, 1);
    while( rdpa(QSPI_INT_STS_REG) & QSPI_INT_STS_RX_FIFO_NOT_EMPTY_MASK )
    {
        rdpa(QSPI_RX_DATA_REG);
    }
}
//------------------------------------------------------------------------------


