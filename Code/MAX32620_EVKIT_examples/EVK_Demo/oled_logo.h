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
********************************************************************************
*/

/* $Revision: 3953 $ $Date: 2015-01-19 14:22:25 -0600 (Mon, 19 Jan 2015) $ */

#include "nhd12832.h"


/* Monochrome Maxim Integrated logo for OLED display */
nhd12832_bitmap_t maxim_integrated_logo = {
    128,   32,    0,
    {
    		0x00000000, 0x00000000, 0x00000000, 0x00000000,
    		0x00038000, 0x007FFC00, 0x01FFFF00, 0x03FFFF80,
            0x0FFFFFE0, 0x1FFFFFF0, 0x3FFFFFF8, 0x3FFFFFFC,
            0x7F0003FC, 0xFF0001FE, 0xFF0001FE, 0xFFFFF9FF,
            0xFFFFF9FF, 0xFFFFF9FF, 0xFF3FF1FF, 0xFF1FC1FF,
            0xFF03C7FF, 0xFFC1FFFF, 0xFFE07FFF, 0xFFE01FFF,
            0xFF830FFF, 0xFF0F83FF, 0xFF1FE1FF, 0xFF7FF9FF,
            0xFFFFF9FF, 0xFFFFF9FF, 0xFF0001FE, 0xFF0001FE,
            0x7F0001FC, 0x3FFFFFFC, 0x3FFFFFF8, 0x1FFFFFF0,
            0x0FFFFFE0, 0x03FFFF80, 0x01FFFF00, 0x007FFC00,
            0x00038000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00030000, 0x0FFB3FE0, 0x0FFB3FE0, 0x00000060,
            0x00000060, 0x0FF80060, 0x0FF83FE0, 0x00383FE0,
            0x00180060, 0x00180060, 0x0FF80060, 0x0FF03FE0,
            0x00003FE0, 0x00180000, 0x00180000, 0x0FFF1E60,
            0x0FFF3E60, 0x0C183360, 0x0C183320, 0x03C03B60,
            0x0FF03FE0, 0x0FF83FE0, 0x0CD80000, 0x0CD83020,
            0x0CD838E0, 0x0CF81FE0, 0x0EF00FC0, 0x00000F80,
            0x78E01FE0, 0x7FF03CE0, 0x6FF83020, 0x4F980000,
            0x4D983FE6, 0x6DF83FEE, 0x7DF00004, 0x78780000,
            0x38183FE0, 0x00003FE0, 0x0FF83FE0, 0x0FF00060,
            0x00300060, 0x00380060, 0x00183FE0, 0x0FB03FE0,
            0x0F980060, 0x0D980060, 0x0CD80060, 0x0CD83FE0,
            0x0FF83FE0, 0x0FF00000, 0x00000000, 0x00180000,
            0x00180000, 0x0FFF0000, 0x0FFF0000, 0x0C180000,
            0x0C180000, 0x00000000, 0x0FF00000, 0x0FF80000,
            0x0CD80000, 0x0CD80000, 0x0CD80000, 0x0CF80000,
            0x0EF00000, 0x00000000, 0x03C00000, 0x0FF00000,
            0x0FF00000, 0x0C380000, 0x0C180000, 0x0C180000,
            0x0FFF8000, 0x0FFF8000, 0x0FFF8000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,

    }
};
