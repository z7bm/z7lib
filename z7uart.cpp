//******************************************************************************
//*
//*      Project:   Any
//*
//*      File:      UART device 
//*
//*      Version 1.0
//*
//*      Copyright (c) 2009-2017, Harry E. Zhurov
//*
//*      $Revision$
//*      $Date::             $
//*
//------------------------------------------------------------------------------


#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <z7uart.h>
#include <z7int.h>

    
const uint32_t UART1_TX_BUF_SIZE = 2048;
extern usr::ring_buffer<char, UART1_TX_BUF_SIZE, uint32_t> Uart1_TxBuf;
extern class TUart Uart1;
                                       
//---------------------------------------------------------------------------
void uart1_isr_handler();

//---------------------------------------------------------------------------
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
void TUart::init()
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
    
    ps7_register_isr_handler(&uart1_isr_handler, PS7IRQ_ID_UART1);
    

}
//---------------------------------------------------------------------------
void uart1_isr_handler()
{
    if( Uart1.tx_empty() )
    {
        uint32_t Count = Uart1_TxBuf.get_count();
        Count = Count > 64 ? 64 : Count;
        if( Count )
        {
            for(uint32_t i = 0; i < Count; ++i)
            {
                Uart1.push_tx(Uart1_TxBuf.pop());
            }
        }
        else Uart1.disable_tx_empty_int();
        
        Uart1.clear_tx_empty_flag();
    }
}
//---------------------------------------------------------------------------
void TUart::send(const char c)
{
    Uart1_TxBuf.push(c); 
    enable_tx_empty_int();
//    MMR16(UART_THR)   = c;
//    MMR16(UART_IER)  |= ETBEI;
}
//---------------------------------------------------------------------------
void TUart::send(const char* s)
{
    uint32_t i = 0;
    while(s[i])
    {
        Uart1_TxBuf.push(s[i++]);
    }
    enable_tx_empty_int();
    if( !(Regs->CHANNEL_STS & UART_CHNL_STS_TFUL_MASK) )
    {
        push_tx(Uart1_TxBuf.pop());
    }

//
////    MMR16(UART_THR)   = TxBuf.pop();
//    MMR16(UART_IER)  |= ETBEI;
//
}
//---------------------------------------------------------------------------
//void UART::send(const uint8_t *data, const uint16_t count)
//{
//    for(uint16_t i = 0; i < count; i++)
//    {
//        if( TxBuf.get_count() )
//        {
//            TxBuf.push(data[i]);
//        }
//    }
//
//    MMR16(UART_IER)  |= ETBEI;
//}
//---------------------------------------------------------------------------

