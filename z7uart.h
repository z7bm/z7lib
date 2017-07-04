//******************************************************************************
//*
//*      Project:   TBx
//*
//*      File:      UART device 
//*
//*      Version 1.0
//*
//*      Copyright (c) 2009, Harry E. Zhurov
//*
//*      $Revision$
//*      $Date::             $
//*
//------------------------------------------------------------------------------

#ifndef Z7UART_H
#define Z7UART_H

#include <stdint.h>
#include <ps7mmrs.h>
#include <z7common.h>
#include "usrlib.h"



class TUart
{
public:
    struct TRegs
    {
        uint32_t CTRL;               //  32    mixed    0x00000128    UART Control Register
        uint32_t MODE;               //  32    mixed    0x00000000    UART Mode Register
        uint32_t INT_EN;             //  32    mixed    0x00000000    Interrupt Enable Register
        uint32_t INT_DIS;            //  32    mixed    0x00000000    Interrupt Disable Register
        uint32_t INT_MASK;           //  32    ro       0x00000000    Interrupt Mask Register
        uint32_t CHNL_INT_STS;       //  32    wtc      0x00000000    Channel Interrupt Status Register
        uint32_t BAUD_RATE_GEN;      //  32    mixed    0x0000028B    Baud Rate Generator Register
        uint32_t RX_TIMEOUT;         //  32    mixed    0x00000000    Receiver Timeout Register
        uint32_t RX_FIFO_TRG_LVL;    //  32    mixed    0x00000020    Receiver FIFO Trigger Level Register
        uint32_t MODEM_CTRL;         //  32    mixed    0x00000000    Modem Control Register
        uint32_t MODEM_STS;          //  32    mixed    x             Modem Status Register
        uint32_t CHANNEL_STS;        //  32    ro       0x00000000    Channel Status Register
        uint32_t TX_RX_FIFO;         //  32    mixed    0x00000000    Transmit and Receive FIFO
        uint32_t BAUD_RATE_DIVIDER;  //  32    mixed    0x0000000F    Baud Rate Divider Register
        uint32_t FLOW_DELAY;         //  32    mixed    0x00000000    Flow Control Delay Register
        uint32_t TX_FIFO_TRG_LVL;    //  32    mixed    0x00000020    Transmitter FIFO Trigger Level Register
    };

public:
    TUart(uintptr_t addr) 
            : Regs( reinterpret_cast<TRegs*>(addr) ) 
    { 
    }
    
    void init();
    void send(const char  c);
    void send(const char *s);
  
    bool tx_empty() const { return Regs->CHNL_INT_STS & UART_CHNL_INT_STS_TEMPTY_MASK; }
    void enable_tx_empty_int()  const { Regs->INT_EN  = UART_INT_EN_TEMPTY_MASK; }
    void disable_tx_empty_int() const { Regs->INT_DIS = UART_INT_DIS_TEMPTY_MASK; }
    void clear_tx_empty_flag() const { Regs->CHNL_INT_STS = UART_CHNL_INT_STS_TEMPTY_MASK; }
    void push_tx(char c) const { Regs->TX_RX_FIFO = c; }
      
private:
    static const uint32_t UART0_RST_MASK = UART_RST_CTRL_UART0_REF_RST_MASK | UART_RST_CTRL_UART0_CPU1X_RST_MASK;
    static const uint32_t UART1_RST_MASK = UART_RST_CTRL_UART1_REF_RST_MASK | UART_RST_CTRL_UART1_CPU1X_RST_MASK;
    //constexpr uint32_t RST_MASK = (reinterpret_cast<uintptr_t>(Regs))
        
private:
    volatile TRegs *Regs;
    
    
};


//extern char RxBuf[RX_BUF_SIZE];


#endif // Z7UART_H