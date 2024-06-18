//------------------------------------------------------------------------------
//
//      Xilinx zynq-7000/arm-none-eabi Interrupts Support Source
//
//      Version 1.0
//
//      Permission is hereby granted, free of charge, to any person obtaining
//      a copy of this software and associated documentation files (the
//      "Software"), to deal in the Software without restriction, including
//      without limitation the rights to use, copy, modify, merge, publish,
//      distribute, sublicense, and/or sell copies of the Software, and to
//      permit persons to whom the Software is furnished to do so, subject to
//      the following conditions:
//
//      The above copyright notice and this permission notice shall be included
//      in all copies or substantial portions of the Software.
//
//      THE SOFTWARE  IS PROVIDED  "AS IS", WITHOUT  WARRANTY OF  ANY KIND,
//      EXPRESS  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//      MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//      IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//      CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//      TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
//      THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//     Copyright (c) 2017-2024, Zynq-7000 Bare-metal Project
//     -----------------------------------------------------
//     Project sources: https://github.com/z7bm
//
//------------------------------------------------------------------------------

#include <z7int.h>

//------------------------------------------------------------------------------
isr_ptr_t ps7_handlers[PS7_MAX_IRQ_ID];
//------------------------------------------------------------------------------
void ps7_register_isr(isr_ptr_t ptr, uint32_t id)
{
    ps7_handlers[id] = ptr;
}
//------------------------------------------------------------------------------

