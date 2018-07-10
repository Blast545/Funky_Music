/**
 * @file
 * @brief      UART Wakeup Receiver
 * @details    This example wakes up and receives data from a transmitter.
 *             Connect the TX of the transmitter (P2.1) to RX (P2.0) of the
 *             receiver. Connect CTS of the transmitter (P2.2) to RTS (P2.3) of
 *             the receiver. The debugger has the potential to reset the
 *             receiver as it's transitioning from LP1 to LP3 so it's best to
 *             disconnect the debugger after programming. Once both devices are
 *             programmed, reset the transmitter, reset the receiver, press PB0
 *             on the receiver, and press PB0 on the transmitter to start the
 *             test.
 */

 /* *****************************************************************************
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
 **************************************************************************** */



/* **** Includes **** */
#include <stdio.h>
#include <string.h>
#include "mxc_config.h"
#include "board.h"
#include "tmr_utils.h"
#include "lp.h"
#include "tmr.h"

/* **** Definitions **** */
#define UART_TEST               MXC_UART1
#define UART_TEST_IDX           (MXC_UART_GET_IDX(UART_TEST))
#define UART_ENDING_LEN         4
#define UART_TEST_BAUD          1800000
#define UART_TEST_LEN           512

// UART2 is on port 3, UART1 is on port 2
#define WAKE_PORT               (UART_TEST_IDX + 1)
#define WAKE_PIN                ((0x1 << 0) | (0x1 << 3))

#define STARTUP_DELAY           (USEC(20))
#define SLEEP_DELAY             (USEC(30))

#define ACT_LOW_WAKEUP          0 /**< Active Low Wakeup on GPIO */

/* **** Globals **** */

/* **** Functions **** */
volatile int read_error;
volatile int tmr_flag;

/* **************************************************************************** */
void read_cb(uart_req_t* req, int error)
{
    read_error = error;
}

/* **************************************************************************** */
void UART1_IRQHandler(void)
{
    LED_On(3);
    UART_Handler(MXC_UART1);
    LED_Off(3);
}

/* **************************************************************************** */
void UART2_IRQHandler(void)
{
    UART_Handler(UART_TEST);
}

/* **************************************************************************** */
void TMR1_IRQHandler(void)
{
    tmr_flag = 0;
    TMR32_ClearFlag(MXC_TMR1);
}

// *****************************************************************************
int verify_rx(uint8_t *rxbuf, int start)
{
    int i;
    for(i = start; i < UART_TEST_LEN; i++) {
        if(rxbuf[i] != (i & 0xFF)) {
            return i;
        }
    }

    return 0;
}

// *****************************************************************************
int main(void)
{
    int error, iter, startup_bytes, total_bytes, framing_errors;
    unsigned bytes_left, uart_errors;
    uint32_t ticks;
    uint8_t rxbuf[UART_TEST_LEN];

    printf("\nUART Wakeup Test, receiver edition\n");

    // Setup the wakeup source
    gpio_cfg_t gpioLP;
    gpioLP.port = WAKE_PORT;
    gpioLP.mask = WAKE_PIN;

    // Setup the ASYNC request
    NVIC_EnableIRQ(MXC_UART_GET_IRQ(UART_TEST_IDX));
    uart_req_t read_req;
    read_req.callback = read_cb;

    // Setup the Sleep Timer
    NVIC_EnableIRQ(MXC_TMR_GET_IRQ_32(1));
    TMR_Init(MXC_TMR1, TMR_PRESCALE_DIV_2_0, NULL);
    TMR32_EnableINT(MXC_TMR1);
    TMR32_TimeToTicks(MXC_TMR1, SLEEP_DELAY, TMR_UNIT_MICROSEC, &ticks);
    tmr32_cfg_t tmr_cfg;
    tmr_cfg.mode = TMR32_MODE_ONE_SHOT;
    tmr_cfg.compareCount = ticks;

    // Initialize the UART
    sys_cfg_uart_t sys_cfg;
    sys_cfg.clk_scale = CLKMAN_SCALE_AUTO;
    sys_cfg.io_cfg = (ioman_cfg_t)IOMAN_UART(UART_TEST_IDX, IOMAN_MAP_A, 
        IOMAN_MAP_A, IOMAN_MAP_A, 1, 1, 1);

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
    // Put a ~2 second delay at the start of the program if you want to remove this
    // Allows the debugger to halt the system after reset
    printf("Waiting for PB0\n");
    while(PB_Get(0) == 0) {}
    printf("Waiting for transmitter\n");

    iter = 0;
    framing_errors = 0;
    while(1) {

        // Clear the buffer
        memset(rxbuf, 0x0, UART_TEST_LEN);

        // Wait for console printing to finish before sleeping
        while(Console_PrepForSleep() != E_NO_ERROR) {}

        // Configure LP1 wake up
        LP_ClearWakeUpConfig();
        LP_ClearWakeUpFlags();
        LP_ConfigGPIOWakeUpDetect(&gpioLP, ACT_LOW_WAKEUP, LP_WEAK_PULL_UP);
        LP_EnterLP1();

        // Reverse the polarity of RTS to synchronize with the transmitter
        UART_TEST->ctrl ^= MXC_F_UART_CTRL_RTS_POLARITY;

        // Wait for the last byte interrupted by RTS to enter the RXFIFO
        TMR_Delay(MXC_TMR0, STARTUP_DELAY);

        // Wait for there to be at least one byte in the RXFIFO
        while((startup_bytes = UART_NumReadAvail(UART_TEST)) == 0) {}

        // Clear any errors
        uart_errors = UART_GetFlags(UART_TEST);
        UART_ClearFlags(UART_TEST,  (MXC_F_UART_INTEN_RX_FIFO_OVERFLOW  |
                                     MXC_F_UART_INTEN_RX_FRAMING_ERR |
                                     MXC_F_UART_INTEN_RX_PARITY_ERR));

        if(uart_errors & MXC_F_UART_INTEN_RX_FRAMING_ERR) {
            framing_errors++;
        }

        // Read out the potentially corrupted data
        UART_Read(UART_TEST, rxbuf, startup_bytes, NULL);
        total_bytes = startup_bytes;

        // Read out most of the data, save the last portion for another transaction
        // in case we missed some bytes at the beginning
        read_req.data = &rxbuf[startup_bytes];
        read_req.len = UART_TEST_LEN-total_bytes-UART_ENDING_LEN;
        read_req.data = &rxbuf[startup_bytes];
        read_error = 1;
        error = UART_ReadAsync(UART_TEST, &read_req);
        if(error != E_NO_ERROR) {
            printf("Error starting ReadAsync %d\n", error);
            while(1) {}
        }

        // Reverse the polarity of RTS to continue the transaction
        UART_TEST->ctrl ^= MXC_F_UART_CTRL_RTS_POLARITY;

        // Wait for the ReadAsync to complete
        LED_On(2);
        while(read_error == 1) {}
        LED_Off(2);

        // Check the callback for errors
        if((read_error == E_NO_ERROR) &&
            (read_req.num == UART_TEST_LEN-startup_bytes-UART_ENDING_LEN)) {

            total_bytes += read_req.num;
        } else {
            printf("Error ReadAsync callback %d num = %d\n", read_error, read_req.num);
            while(1) {}
        }

        // Wait for the transmitter to finish sending its data
        LED_On(1);
        tmr_flag = 1;
        TMR32_Config(MXC_TMR1, &tmr_cfg);
        TMR32_Start(MXC_TMR1);
        while(tmr_flag == 1) {}
        LED_Off(1);

        // Read the rest of the data in the FIFO
        bytes_left = UART_NumReadAvail(UART_TEST);
        error = UART_Read(UART_TEST, &rxbuf[UART_TEST_LEN-bytes_left], bytes_left, NULL);
        if(error != bytes_left) {
            printf("Error reading remainder %d\n", error);
        }
        total_bytes += error;

#if 0
        // Print some debugging data
        printf("total_bytes = %d startup_bytes = %d ", total_bytes, startup_bytes);
        printf("error_flag = 0x%x\n", uart_errors);

        // Print the beginning and end of the buffer
        int i;
        for(i = 0; i < 16; i++) {
            printf("%02x", rxbuf[i]);
        }
        printf("  ");
        for(i = (UART_TEST_LEN-16); i < UART_TEST_LEN; i++) {
            printf("%02x", rxbuf[i]);
        }
        printf("\n");
#endif

        // Verify the data
        error = verify_rx(rxbuf, startup_bytes);
        if(error != 0) {
            printf("Error verifying data at index %d i = %d\n", error, iter);
            while(1) {}
        }
        
        iter++;
        if(iter%100 == 1)    
            printf("Verified %d %d\n", iter, framing_errors);
    }
#if defined ( __ICCARM__ )
    return 0;
#endif
}
