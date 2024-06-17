//------------------------------------------------------------------------------
//
//    Project:   Any
//
//    File:      UART device
//
//    Version 2.0
//
//    Copyright (c) 2009-2024, Harry E. Zhurov
//
//------------------------------------------------------------------------------


#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <z7uart.h>
#include <z7int.h>

extern class Uart uart1;
                                       
//------------------------------------------------------------------------------
void uart1_isr_handler();

//------------------------------------------------------------------------------
//
//    Baud rate. Input clock 100 MHz, UART baud rate 115200.
// 
//    Prescaler:   1  | Mode.CLKSEL       = 0 
//    CD       : 124  | BAUD_RATE_GEN     = 124 
//    BDIV     :   6  | BAUD_RATE_DIVIDER = 6
// 
//    SEL_CLK     = INPUT_CLK/Prescaler
//    BAUD_SAMPLE = SEL_CLK/CD
//    BAUD_RATE   = BAUD_SAMPLE/(BDIV + 1) = INPUT_CLK/(Prescaler*CD*(BDIV + 1)
// 
//    BAUD_RATE   = 100e6/(1*124*7) = 115207
//    
void Uart::init()
{
    // reset UART
    const uint32_t RST_MASK = reinterpret_cast<uintptr_t>(Regs) == UART0_ADDR ? UART0_RST_MASK : UART1_RST_MASK;
    slcr_unlock();
    sbpa(UART_RST_CTRL_REG, RST_MASK);
    cbpa(UART_RST_CTRL_REG, RST_MASK);
    slcr_lock();
    
    uint32_t Ctrl = Regs->CTRL;
    Ctrl &= ~(UART_CTRL_RXEN_MASK | UART_CTRL_TXEN_MASK);
    Ctrl |=  (UART_CTRL_RXDIS_MASK | UART_CTRL_TXDIS_MASK);
    Regs->CTRL = Ctrl;

    Regs->MODE &= ~UART_MODE_CLKS_MASK;          // Prescaler = 0
    Regs->MODE |= (4ul << UART_MODE_PAR_BPOS);   // No parity
    Regs->BAUD_RATE_GEN     = 124;
    Regs->BAUD_RATE_DIVIDER = 6;
    
    Regs->CTRL = Ctrl | UART_CTRL_RXRES_MASK | UART_CTRL_TXRES_MASK;
    Ctrl &= ~(UART_CTRL_RXDIS_MASK | UART_CTRL_TXDIS_MASK);
    Ctrl |=  (UART_CTRL_RXEN_MASK | UART_CTRL_TXEN_MASK);
    Regs->CTRL = Ctrl;
    
    ps7_register_isr(&uart1_isr_handler, PS7IRQ_ID_UART1);
    

}
//------------------------------------------------------------------------------
void uart1_isr_handler()
{
    if( uart1.tx_empty() )
    {
        uint32_t Count = uart1.tx_buf.get_count();
        Count = Count > 64 ? 64 : Count;
        if( Count )
        {
            for(uint32_t i = 0; i < Count; ++i)
            {
                uart1.push_tx(uart1.tx_buf.pop());
            }
        }
        else
        {
            uart1.disable_tx_empty_int();
            uart1.set_busy(false);
        }
        
        uart1.clear_tx_empty_flag();

    }
}
//------------------------------------------------------------------------------
void Uart::send(const char c)
{
    busy = true;
    tx_buf.push(c);
    enable_tx_empty_int();
}
//------------------------------------------------------------------------------
void Uart::send(const char* s)
{
    busy = true;
    uint32_t i = 0;
    while(s[i])
    {
        tx_buf.push(s[i++]);
    }
    enable_tx_empty_int();
    if( !(Regs->CHANNEL_STS & UART_CHNL_STS_TFUL_MASK) )
    {
        push_tx(tx_buf.pop());
    }
}
//------------------------------------------------------------------------------

