/**
 * @file 
 * @brief      Example showing how to use the CRC module. Open a terminal
 *             program to see the CRC calculated.
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
 * $Date: 2017-05-02 13:57:41 -0500 (Tue, 02 May 2017) $
 * $Revision: 27735 $
 *
 **************************************************************************** */



/***** Includes *****/
#include <stdio.h>
#include "mxc_config.h"
#include "crc.h"
#include "board.h"

/***** Definitions *****/
#define LITTLE_ENDIAN 1
#define BIG_ENDIAN 0

#define CRC16_CCITT_FALSE 0
#define CRC16_CCITT_TRUE 1

#define EXPECTED_CRC16_CCITT_FALSE_BIG		0x53A0
#define EXPECTED_CRC16_CCITT_FALSE_LITTLE	0x433B
#define EXPECTED_CRC16_CCITT_TRUE_BIG		0x5E70
#define EXPECTED_CRC16_CCITT_TRUE_LITTLE	0xFAF0

#define EXPECTED_CRC32_BIG		0xF1A0CB49
#define EXPECTED_CRC32_LITTLE	0x04E82BC6

/* **** Globals **** */
uint32_t data[] = {0x1234, 0x5678, 0x9ABC, 0xDEF0, 0x2345};


/* **** Functions **** */

/* ************************************************************************** */
int main(void)
{
    printf("********************CRC Demo*******************\n");

    int byteOrder;
    int CRC16Mode;
    int crc;
    int dataLength = sizeof data / sizeof *data;
    int i = 0;


    printf("CRC16-CCITT-FALSE\n");
    byteOrder = BIG_ENDIAN;
    CRC16Mode = CRC16_CCITT_FALSE;

    //initialize 16 bit CRC
    CRC16_Init(CRC16Mode, byteOrder);
    CRC16_Reseed(0xFFFF);

    //add array of data to crc calculation
    CRC16_AddDataArray(data, dataLength);

    //get calculated CRC
    crc = CRC16_GetCRC();
    printf("Calculated CRC = 0x%08x\n", crc);
    printf("Expected CRC = 0x%08x\n",EXPECTED_CRC16_CCITT_FALSE_BIG);
    printf("\n");



    printf("CRC32\n");
    byteOrder = BIG_ENDIAN;

    //initialize 32bit CRC
    CRC32_Init(byteOrder);
    CRC32_Reseed(0xFFFFFFFF);

    //add data one byte at a time
    for(i = 0; i < dataLength; i++) {
        CRC32_AddData(data[i]);
    }

    //get calculated CRC
    crc = CRC32_GetCRC();
    printf("Calculated CRC = 0x%08x\n", crc);
    printf("Expected CRC = 0x%08x\n",EXPECTED_CRC32_BIG);
    printf("\n");

    while(1) {}
}
