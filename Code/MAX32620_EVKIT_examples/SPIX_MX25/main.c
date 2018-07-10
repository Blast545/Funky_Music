/*******************************************************************************
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
 * $Date: 2018-03-19 13:57:54 -0500 (Mon, 19 Mar 2018) $
 * $Revision: 34046 $
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   SPIX example using the MX25.
 * @details Uses the MX25 on the EvKit to show the SPIX. Code is written to the
 *          external flash, and then that code is executed from the flash.
 *          MX25_BAUD, MX25_ADDR, and MX25_SPIM_WIDTH can be changed to alter 
 *          the communication between the devices. 
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_config.h"
#include "clkman.h"
#include "ioman.h"
#include "spim.h"
#include "mx25.h"
#include "led.h"
#include "spix.h"

/***** Definitions *****/
#define MX25_BAUD           48000000    // 48 MHz maximum, 20 kHz minimum
#define MX25_ADDR           0x0
#define MX25_SPIM_WIDTH     SPIM_WIDTH_4
#define MX25_EXP_ID         0xc22538

/***** Globals *****/

/***** Functions *****/
void xip_function(void);

// These are set in the linkerfile and give the starting and ending address of xip_section
// Note: This is just internal flash, from whence the sample program is stored until loaded into XIP
//   In a real system, code can be programmed into XIP from a variety of sources (network, serial, etc.)
#if defined ( __GNUC__)
extern uint8_t __load_start_xip, __load_length_xip;
#elif defined ( __CC_ARM )
extern int Image$$RW_IRAM2$$Length;
extern char Image$$RW_IRAM2$$Base[];
uint8_t * __xip_addr;
#endif

/******************************************************************************/
int main(void)
{
    int error;
    
    uint32_t id;
    spim_cfg_t spim_cfg;
    sys_cfg_spim_t spim_sys_cfg;
    sys_cfg_spix_t spix_sys_cfg;
    spix_fetch_t spix_fetch;

    printf("\n\n***** SPIX MX25 Example *****\n");
    printf(" System freq \t: %u Hz\n", SystemCoreClock);
    printf(" MX25 freq \t: %u Hz\n", MX25_BAUD);
    printf(" MX25 ID \t: 0x%x\n", MX25_EXP_ID);
    printf(" SPI data width : %d bits\n\n", (0x1 << MX25_SPIM_WIDTH));

    // Initialize the SPIM
    spim_cfg.mode = 0;
    spim_cfg.ssel_pol = 0;
    spim_cfg.baud = MX25_BAUD;

    // MX25 IO Config                       core I/O, ss0, ss1, ss2, quad, fast I/O
    spim_sys_cfg.io_cfg = (ioman_cfg_t)IOMAN_SPIM1(1,   1,  0,    0,    1,        1);
    spim_sys_cfg.clk_scale = CLKMAN_SCALE_AUTO;

    if ((error = SPIM_Init(MXC_SPIM1, &spim_cfg, &spim_sys_cfg)) != E_NO_ERROR) {
        printf("Error initializing SPIM %d\n", error);
        while (1) {}
    } else {
        printf("SPIM Initialized\n");
    }

    // Initialize the MX25
    MX25_init(MXC_SPIM1, 0);
    MX25_reset();
    printf("MX25 Initialized\n");

    // Get the ID of the MX25
    id = MX25_ID();
    if (id != MX25_EXP_ID) {
        printf("Error verifying MX25 ID: 0x%x\n", id);
        while (1) {}
    } else {
        printf("MX25 ID verified\n");
    }

    // Erase the test sector
    MX25_erase(MX25_ADDR, MX25_ERASE_4K);

    // Enable Quad mode if we're using quad
    if (MX25_SPIM_WIDTH == SPIM_WIDTH_4) {
        if (MX25_quad(1) != E_NO_ERROR) {
            printf("Error enabling quad mode\n");
        }
    } else {
        if (MX25_quad(0) != E_NO_ERROR) {
            printf("Error disabling quad mode\n");
        }
    }

    // Program the MX25
#if defined ( __GNUC__ )
    printf("Programming function (%d bytes @ 0x%08x) into external MX25 flash ..\n", (uint32_t)&__load_length_xip, &__load_start_xip);
    MX25_program_page(MX25_ADDR, &__load_start_xip, (uint32_t)&__load_length_xip, MX25_SPIM_WIDTH);
    printf("  done.\n");
#elif defined ( __CC_ARM )
    MX25_program_page(MX25_ADDR, (uint8_t *)Image$$RW_IRAM2$$Base, (uint32_t)&Image$$RW_IRAM2$$Length, MX25_SPIM_WIDTH);
#endif
    
    // Disable SPIM
    // MX25 IO Config                       core I/O, ss0, ss1, ss2, quad, fast I/O
    spim_sys_cfg.io_cfg = (ioman_cfg_t)IOMAN_SPIM1(0,   0,  0,    0,    0,        0);
    spim_sys_cfg.clk_scale = CLKMAN_SCALE_AUTO;

    if ((error = SPIM_Init(MXC_SPIM1, &spim_cfg, &spim_sys_cfg)) != E_NO_ERROR) {
        printf("Error disabling SPIM %d\n", error);
        while (1) {}
    } else {
        printf("SPIM Disabled\n");
    }

    // Setup SPIX
    // SPIX IO Config                          core, ss0, ss1, ss2, quad, fast
    spix_sys_cfg.io_cfg = (ioman_cfg_t)IOMAN_SPIX(1,   1,   0,   0,    1,    1);
    spix_sys_cfg.clk_scale = CLKMAN_SCALE_AUTO;

    spix_fetch.cmd_width = SPIX_SINGLE_IO;
    spix_fetch.addr_width = SPIX_SINGLE_IO;
    spix_fetch.data_width = SPIX_SINGLE_IO;
    spix_fetch.addr_size = SPIX_3BYTE_FETCH_ADDR;
    spix_fetch.cmd = 0x3;
    spix_fetch.mode_clocks = 0;
    spix_fetch.no_cmd_mode = 0;
    spix_fetch.mode_data = 0;
    
    if (MX25_SPIM_WIDTH == SPIM_WIDTH_4) {
        spix_fetch.cmd_width = SPIX_SINGLE_IO;
        spix_fetch.addr_width = SPIX_QUAD_IO;
        spix_fetch.data_width = SPIX_QUAD_IO;
        spix_fetch.cmd = 0xEB;
        spix_fetch.mode_clocks = 6;
    }

    SPIX_ConfigClock(&spix_sys_cfg, MX25_BAUD, 0);
    SPIX_ConfigSlave(0, 0, 0, 0);
    SPIX_ConfigFetch(&spix_fetch);

    printf("Jumping to external flash function (@ 0x%08x)\n", xip_function);
    // This function runs from the XIP address range
    xip_function();
    printf("Returned from external flash\n");

    printf(" *** END OF DEMO ***\n");

    return 0;
}
