/**
 * @file    
 * @brief   UART Wakeup Transmitter
 * @details This example sends a data packet to a receiver.
 */

/* ******************************************************************************
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
 * $Date: 2017-05-02 13:57:41 -0500 (Tue, 02 May 2017) $
 * $Revision: 27735 $
 *
 ***************************************************************************** */

/* **** Includes **** */
#include <stdio.h>
#include "mxc_config.h"
#include "board.h"
#include "tmr_utils.h"

/* **** Definitions **** */
#define UART_TEST               MXC_UART1
#define UART_TEST_IDX           (MXC_UART_GET_IDX(UART_TEST))
#define UART_TEST_BAUD          1800000
#define UART_TEST_LEN           512
#define TX_DELAY                (MSEC(200))

/* **** Globals **** */

/* **** Functions **** */

/* ************************************************************************** */
int main(void)
{
    int error;
    int i, iter = 0;
    uint8_t txdata[UART_TEST_LEN];

    printf("\nUART Wakeup Test, transmitter edition\n");

    // Setup the txdata
    for(i = 0; i < UART_TEST_LEN; i++) {
        txdata[i] = (i & 0xFF);
    }

    // Initialize the UART
    sys_cfg_uart_t sys_cfg;
    sys_cfg.clk_scale = CLKMAN_SCALE_AUTO;
    sys_cfg.io_cfg = (ioman_cfg_t)IOMAN_UART(UART_TEST_IDX, IOMAN_MAP_A, IOMAN_MAP_A, IOMAN_MAP_A, 1, 1, 1);

    uart_cfg_t test_uart_cfg;
    test_uart_cfg.parity = UART_PARITY_DISABLE;
    test_uart_cfg.size = UART_DATA_SIZE_8_BITS;
    test_uart_cfg.cts = 1;
    test_uart_cfg.rts = 1;
    test_uart_cfg.baud = UART_TEST_BAUD;
    test_uart_cfg.extra_stop = 0;

    // Wait for the console UART to finish
    while(UART_Busy(MXC_UART_GET_UART(CONSOLE_UART))) {}
    
    error = UART_Init(UART_TEST, &test_uart_cfg, &sys_cfg);
    UART_Init(MXC_UART_GET_UART(CONSOLE_UART), &console_uart_cfg, NULL);

    if(error != E_NO_ERROR) {
        printf("Error initializing UART %d\n", error);
        while(1) {}
    } else {
        printf("UART %d Initialized\n", UART_TEST_IDX);
    }

    // Wait for PB0 before beginning test
    printf("Waiting for PB0\n");
    while(PB_Get(0) == 0) {}

    while(1) {

        if((error = UART_Write(UART_TEST, txdata, UART_TEST_LEN)) != UART_TEST_LEN) {
            printf("Error writing UART %d iter = %d\n", error, iter);
            while(1) {}
        } else {
            iter++;
            if(iter%100 == 1)
                printf("Write complete %d\n", iter);
        }

        TMR_Delay(MXC_TMR0, TX_DELAY);
    }

#if defined (__ICCARM__)
    return 0;
#endif
}
