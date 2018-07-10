/**
 * @file
 * @brief      Main for UART example.
 * @details    This example loops back the TX to the RX on UART1. For this
 *             example you must connect a jumper across P2.0 to P2.1 and connect
 *             P2.2 and P2.3. UART_BAUD and the BUFF_SIZE can be changed in this
 *             example.
 */

/* ****************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 * 
 * $Date: 2017-02-28 11:08:17 -0600 (Tue, 28 Feb 2017) $ 
 * $Revision: 26742 $
 *
 *************************************************************************** */

/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "clkman.h"
#include "ioman.h"
#include "uart.h"
#include "board.h"

/* **** Definitions **** */
#define UART_BAUD           1800000 /**< Desired baud rate (1.8Mbps) */
#define BUFF_SIZE           512     /**< Size of the transmit and receive data blocks */

/* **** Globals **** */
volatile int read_flag;     /**< Flag used to indicate read status/error, written under ISR */
volatile int write_flag;    /**< Flag used to indicate write status, written under ISR */

/* **** Functions **** */

/**
 * @brief      UART Read Callback Handler
 *
 * @param      req    The request object for the UART
 * @param[in]  error  The status/error of the UART read
 */
void read_cb(uart_req_t* req, int error)
{
    read_flag = error; /* Save the status to the read_flag for main to interpret. */
}


/**
 * @brief      UART Write Callback Handler
 *
 * @param      req    Pointer to the request object that resulted in this callback.
 * @param[in]  error  The status of the write operation. 
 */
void write_cb(uart_req_t* req, int error)
{
    write_flag = error; /* Save the status to the write_flag for main to interpret. */
}


/**
 * @brief      UART 1 IRQ Handler The handler for UART1 can be modified for
 *             desired behavior, however, in this example it only calls the
 *             default UART_Handler from the API.
 */
void UART1_IRQHandler(void)
{
    UART_Handler(MXC_UART1);
}

/* ************************************************************************* */
int main(void)
{
    int error, i;
    uint8_t txdata[BUFF_SIZE]; /* Transmit buffer */
    uint8_t rxdata[BUFF_SIZE]; /* Receive buffer */
    
    // stdout is handled in the BSP, so UART0 is where printf will show up.
    printf("\n\n***** UART Example *****\n"); 

    printf(" System freq \t: %d Hz\n", SystemCoreClock); // 96MHz
    printf(" UART freq \t: %d Hz\n", UART_BAUD);
    printf(" Loop back \t: %d bytes\n\n", BUFF_SIZE);

    // Initialize the data buffers
    for(i = 0; i < BUFF_SIZE; i++) {
        txdata[i] = i;
    }
    memset(rxdata, 0x0, BUFF_SIZE);

    // Setup the interrupt
    NVIC_ClearPendingIRQ(MXC_UART_GET_IRQ(1));
    NVIC_DisableIRQ(MXC_UART_GET_IRQ(1));
    NVIC_SetPriority(MXC_UART_GET_IRQ(1), 1);
    NVIC_EnableIRQ(MXC_UART_GET_IRQ(1));

    // Initialize the UART using the configuration structure. 
    uart_cfg_t cfg; 
    cfg.parity = UART_PARITY_DISABLE;
    cfg.size = UART_DATA_SIZE_8_BITS;
    cfg.extra_stop = 0;
    cfg.cts = 1;
    cfg.rts = 1;
    cfg.baud = UART_BAUD;

    // System configuration object to set up the clock & I/O mapping for UART1. 
    sys_cfg_uart_t sys_cfg; 
    // Auto scaled to achieve desired baud, which means the system clock will change. 
    sys_cfg.clk_scale = CLKMAN_SCALE_AUTO; 
    // Set the GPIO pin mapping for UART1 to configuration A, P2.0 and P2.2
    sys_cfg.io_cfg = (ioman_cfg_t)IOMAN_UART(1, IOMAN_MAP_A, IOMAN_MAP_A, IOMAN_MAP_A, 1, 1, 1);

    // Wait for the console UART to finish
    while(UART_Busy(MXC_UART_GET_UART(CONSOLE_UART))) {} 
    
    // The console needs to be finished prior to the call to UART_Init, because CLKMAN_SCALE_AUTO will
    // result in the system clock changing to a divisor that works for the desired baud on UART1.
    error = UART_Init(MXC_UART1, &cfg, &sys_cfg);
    
    // Any peripheral using the system clock needs to be re-initialized due to the system clock changing, 
    UART_Init(MXC_UART_GET_UART(CONSOLE_UART), &console_uart_cfg, NULL); 

    if(error != E_NO_ERROR) {
        printf("Error initializing UART %d\n", error);
        while(1) {}
    } else {
        printf("UART Initialized\n");
    }

    // Setup the asynchronous requests
    uart_req_t read_req;
    read_req.data = rxdata;
    read_req.len = BUFF_SIZE;
    read_req.callback = read_cb;

    uart_req_t write_req;
    write_req.data = txdata;
    write_req.len = BUFF_SIZE;
    write_req.callback = write_cb;

    read_flag = 1;
    write_flag = 1;

    error = UART_ReadAsync(MXC_UART1, &read_req);
    if(error != E_NO_ERROR) {
        printf("Error starting async read %d\n", error);
        while(1) {}
    }

    error = UART_WriteAsync(MXC_UART1, &write_req);
    if(error != E_NO_ERROR) {
        printf("Error starting async write %d\n", error);
        while(1) {}
    }

    while(write_flag == 1) {}
    if(write_flag != E_NO_ERROR) {
        printf("Error with UART_WriteAsync callback\n");
    }

    while(read_flag == 1) {}
    if(read_flag != E_NO_ERROR) {
        printf("Error with UART_ReadAsync callback %d\n", read_flag);
    }

    if((error = memcmp(rxdata, txdata, BUFF_SIZE)) != 0) {
        printf("Error verifying rxdata %d\n", error);
    } else {
        printf("rxdata verified\n");
    }

    while(1) {}
}
