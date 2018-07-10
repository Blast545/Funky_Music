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
 * Description: Human Interface Device Class (Keyboard) over USB
 * $Id: descriptors.h 23133 2016-06-01 14:26:45Z kevin.gillespie $
 *
 *******************************************************************************
 */

#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

#include <stdint.h>
#include "usb.h"
#include "hid_kbd.h"

usb_device_descriptor_t __attribute__((aligned(4))) device_descriptor = {
    0x12,         /* bLength                           */
    0x01,         /* bDescriptorType = Device          */
    0x0110,       /* bcdUSB USB spec rev (BCD)         */
    0x00,         /* bDeviceClass = Unspecified        */
    0x00,         /* bDeviceSubClass                   */
    0x00,         /* bDeviceProtocol                   */
    0x40,         /* bMaxPacketSize0 is 64 bytes       */
    0x0B6A,       /* idVendor (Maxim Integrated)       */
    0x0480,       /* idProduct                         */
    0x0100,       /* bcdDevice                         */
    0x01,         /* iManufacturer Descriptor ID       */
    0x02,         /* iProduct Descriptor ID            */
    0x03,         /* iSerialNumber Descriptor ID       */
    0x01          /* bNumConfigurations                */
};

__attribute__((aligned(4)))
struct __attribute__((packed)) {
    usb_configuration_descriptor_t  config_descriptor;
    usb_interface_descriptor_t      interface_descriptor;
    hid_descriptor_t                hid_descriptor;
    usb_endpoint_descriptor_t       endpoint_descriptor_1;
    usb_endpoint_descriptor_t       endpoint_descriptor_2;
} config_descriptor =
{
    {
        0x09,       /*  bLength                          */
        0x02,       /*  bDescriptorType = Config         */
        0x0029,     /*  wTotalLength(L/H) = 41 bytes     */
        0x01,       /*  bNumInterfaces                   */
        0x01,       /*  bConfigurationValue              */
        0x00,       /*  iConfiguration                   */
        0xA0,       /*  bmAttributes (bus-powered, remote wakeup) */
        0x32,       /*  MaxPower is 100ma (units are 2ma/bit) */
    },
    { /*  First Interface Descriptor */
        0x09,       /*  bLength                          */
        0x04,       /*  bDescriptorType = Interface (4)  */
        0x00,       /*  bInterfaceNumber                 */
        0x00,       /*  bAlternateSetting                */
        0x02,       /*  bNumEndpoints (one for OUT)      */
        0x03,       /*  bInterfaceClass = HID            */
        0x00,       /*  bInterfaceSubClass               */
        0x00,       /*  bInterfaceProtocol               */
        0x00,       /*  iInterface */
    },
    { /* HID Descriptor */
        0x09,       /*  bFunctionalLength                */
        0x21,       /*  bDescriptorType = HID            */
        0x0110,     /*  bcdHID Rev 1.1                   */
        0x00,       /*  bCountryCode                     */
        0x01,       /*  bNumDescriptors                  */
        0x22,       /*  bDescriptorType = Report         */
        0x001c,     /*  wDescriptorLength                */
    },
    { /*  IN Endpoint 1 (Descriptor #1) */
        0x07,       /*  bLength                          */
        0x05,       /*  bDescriptorType (Endpoint)       */
        0x81,       /*  bEndpointAddress (EP1-IN)        */
        0x03,       /*  bmAttributes (interrupt)         */
        0x0040,     /*  wMaxPacketSize                   */
        0x0a        /*  bInterval (milliseconds)         */
    },
    { /*  OUT Endpoint 2 (Descriptor #2) */
        0x07,       /*  bLength                          */
        0x05,       /*  bDescriptorType (Endpoint)       */
        0x02,       /*  bEndpointAddress (EP2-OUT)       */
        0x03,       /*  bmAttributes (interrupt)         */
        0x0040,     /*  wMaxPacketSize                   */
        0x0a        /*  bInterval (milliseconds)         */
    },
};

__attribute__((aligned(4)))
uint8_t report_descriptor[] = {
    0x06, 0xAB, 0xFF,   // Usage Page (vendor-defined)
    0x0A, 0x00, 0x02,   // Usage
    0xA1, 0x01,         // Collection (Application) 0x01
    0x75, 0x08,         // report size = 8 bits
    0x15, 0x00,         // logical minimum = 0
    0x26, 0xFF, 0x00,   // logical maximum = 255
    0x95, 64,           // report count 64 bytes
    0x09, 0x01,         // usage
    0x81, 0x02,         // Input (array)
    0x95, 64,           // report count 64 bytes
    0x09, 0x02,         // usage
    0x91, 0x02,         // Output (array)
    0xC0                // end collection
};

__attribute__((aligned(4)))
uint8_t lang_id_desc[] = {
    0x04,         /* bLength */
    0x03,         /* bDescriptorType */
    0x09, 0x04    /* bString = wLANGID (see usb_20.pdf 9.6.7 String) */
};

__attribute__((aligned(4)))
uint8_t mfg_id_desc[] = {
    0x22,         /* bLength */
    0x03,         /* bDescriptorType */
    'M', 0,
    'a', 0,
    'x', 0,
    'i', 0,
    'm', 0,
    ' ', 0,
    'I', 0,
    'n', 0,
    't', 0,
    'e', 0,
    'g', 0,
    'r', 0,
    'a', 0,
    't', 0,
    'e', 0,
    'd', 0,
};

__attribute__((aligned(4)))
uint8_t prod_id_desc[] = {
    0x22,         /* bLength */
    0x03,         /* bDescriptorType */
    'M', 0,
    'A', 0,
    'X', 0,
    '3', 0,
    '2', 0,
    '6', 0,
    '2', 0,
    '0', 0,
    ' ', 0,
    'H', 0,
    'I', 0,
    'D', 0,
    ' ', 0,
    'R', 0,
    'A', 0,
    'W', 0,
};

__attribute__((aligned(4)))
uint8_t serial_id_desc[] = {
    0x14,         /* bLength */
    0x03,         /* bDescriptorType */
    '0', 0,
    '0', 0,
    '0', 0,
    '0', 0,
    '0', 0,
    '0', 0,
    '0', 0,
    '0', 0,
    '1', 0
};

#endif /* _DESCRIPTORS_H_ */
