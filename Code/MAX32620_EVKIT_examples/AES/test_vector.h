/**
 * @file 
 * @brief 		NIST Monte Carlo test CAVS 11.1
 *
 * @details    	Start with PLAINTEXT -> AES128(KEY) -> CIPHERTEXT Repeat 999 more
 *             	times, feeding previous CIPHERTEXT in as next PLAINTEXT Verify
 *             	last CIPHERTEXT against expected result shown above
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
 *************************************************************************** */

#include "aes.h"

/*
 * 128-bit Encrypt vector
 *
 * KEY = 139a35422f1d61de3c91787fe0507afd
 * PLAINTEXT = b9145a768b7dc489a096b546f43b231f
 * CIPHERTEXT = d7c3ffac9031238650901e157364c386
 *
 */

/**
 * 128-bit Encryption Key
 * KEY = 139a35422f1d61de3c91787fe0507afd
 */
const uint8_t nist_enc_128_key[MXC_AES_KEY_128_LEN] = {
    0xfd, 0x7a, 0x50, 0xe0,
    0x7f, 0x78, 0x91, 0x3c,
    0xde, 0x61, 0x1d, 0x2f,
    0x42, 0x35, 0x9a, 0x13
};

/**
 * Plain text for 128-bit test
 * PLAINTEXT = b9145a768b7dc489a096b546f43b231f
 */
const uint8_t nist_enc_128_pt[MXC_AES_DATA_LEN] = {
    0x1f, 0x23, 0x3b, 0xf4,
    0x46, 0xb5, 0x96, 0xa0,
    0x89, 0xc4, 0x7d, 0x8b,
    0x76, 0x5a, 0x14, 0xb9
};

/**
 * 128-bit Cipher Text for 128-bit test. 
 * CIPHERTEXT = d7c3ffac9031238650901e157364c386
 */
const uint8_t nist_enc_128_ct[MXC_AES_DATA_LEN] = {
    0x86, 0xc3, 0x64, 0x73,
    0x15, 0x1e, 0x90, 0x50,
    0x86, 0x23, 0x31, 0x90,
    0xac, 0xff, 0xc3, 0xd7
};

/*
 * 128-bit Decrypt vector
 *
 * KEY = 0c60e7bf20ada9baa9e1ddf0d1540726
 * CIPHERTEXT = b08a29b11a500ea3aca42c36675b9785
 * PLAINTEXT = b613b87085fed1bb87f07a574e6d2879
 *
 */

const uint8_t nist_dec_128_key[MXC_AES_KEY_128_LEN] = {
    0x26, 0x07, 0x54, 0xd1,
    0xf0, 0xdd, 0xe1, 0xa9,
    0xba, 0xa9, 0xad, 0x20,
    0xbf, 0xe7, 0x60, 0x0c
};

const uint8_t nist_dec_128_ct[MXC_AES_DATA_LEN] = {
    0x85, 0x97, 0x5b, 0x67,
    0x36, 0x2c, 0xa4, 0xac,
    0xa3, 0x0e, 0x50, 0x1a,
    0xb1, 0x29, 0x8a, 0xb0
};

const uint8_t nist_dec_128_pt[MXC_AES_DATA_LEN] = {
    0x79, 0x28, 0x6d, 0x4e,
    0x57, 0x7a, 0xf0, 0x87,
    0xbb, 0xd1, 0xfe, 0x85,
    0x70, 0xb8, 0x13, 0xb6
};

/*
 * 192-bit Encrypt vector
 *
 * KEY = b9a63e09e1dfc42e93a90d9bad739e5967aef672eedd5da9
 * PLAINTEXT = 85a1f7a58167b389cddc8a9ff175ee26
 * CIPHERTEXT = ee83d85279e022d2048031abeefbc4a4
 *
 */

const uint8_t nist_enc_192_key[MXC_AES_KEY_192_LEN] = {
    0xa9, 0x5d, 0xdd, 0xee,
    0x72, 0xf6, 0xae, 0x67,
    0x59, 0x9e, 0x73, 0xad,
    0x9b, 0x0d, 0xa9, 0x93,
    0x2e, 0xc4, 0xdf, 0xe1,
    0x09, 0x3e, 0xa6, 0xb9
};

const uint8_t nist_enc_192_pt[MXC_AES_DATA_LEN] = {
    0x26, 0xee, 0x75, 0xf1,
    0x9f, 0x8a, 0xdc, 0xcd,
    0x89, 0xb3, 0x67, 0x81,
    0xa5, 0xf7, 0xa1, 0x85
};

const uint8_t nist_enc_192_ct[MXC_AES_DATA_LEN] = {
    0xa4, 0xc4, 0xfb, 0xee,
    0xab, 0x31, 0x80, 0x04,
    0xd2, 0x22, 0xe0, 0x79,
    0x52, 0xd8, 0x83, 0xee
};

/*
 * 192-bit Decrypt vector
 *
 * KEY = 4b97585701c03fbebdfa8555024f589f1482c58a00fdd9fd
 * CIPHERTEXT = d0bd0e02ded155e4516be83f42d347a4
 * PLAINTEXT = c7b6581ccc88f7fc26d15d2731e7251b
 *
 */

const uint8_t nist_dec_192_key[MXC_AES_KEY_192_LEN] = {
    0xfd, 0xd9, 0xfd, 0x00,
    0x8a, 0xc5, 0x82, 0x14,
    0x9f, 0x58, 0x4f, 0x02,
    0x55, 0x85, 0xfa, 0xbd,
    0xbe, 0x3f, 0xc0, 0x01,
    0x57, 0x58, 0x97, 0x4b
};

const uint8_t nist_dec_192_ct[MXC_AES_DATA_LEN] = {
    0xa4, 0x47, 0xd3, 0x42,
    0x3f, 0xe8, 0x6b, 0x51,
    0xe4, 0x55, 0xd1, 0xde,
    0x02, 0x0e, 0xbd, 0xd0
};

const uint8_t nist_dec_192_pt[MXC_AES_DATA_LEN] = {
    0x1b, 0x25, 0xe7, 0x31,
    0x27, 0x5d, 0xd1, 0x26,
    0xfc, 0xf7, 0x88, 0xcc,
    0x1c, 0x58, 0xb6, 0xc7
};

/*
 * 256-bit Encrypt vector
 *
 * KEY = f9e8389f5b80712e3886cc1fa2d28a3b8c9cd88a2d4a54c6aa86ce0fef944be0
 * PLAINTEXT = b379777f9050e2a818f2940cbbd9aba4
 * CIPHERTEXT = 6893ebaf0a1fccc704326529fdfb60db
 *
 */

const uint8_t nist_enc_256_key[MXC_AES_KEY_256_LEN] = {
    0xe0, 0x4b, 0x94, 0xef,
    0x0f, 0xce, 0x86, 0xaa,
    0xc6, 0x54, 0x4a, 0x2d,
    0x8a, 0xd8, 0x9c, 0x8c,
    0x3b, 0x8a, 0xd2, 0xa2,
    0x1f, 0xcc, 0x86, 0x38,
    0x2e, 0x71, 0x80, 0x5b,
    0x9f, 0x38, 0xe8, 0xf9
};

const uint8_t nist_enc_256_pt[MXC_AES_DATA_LEN] = {
    0xa4, 0xab, 0xd9, 0xbb,
    0x0c, 0x94, 0xf2, 0x18,
    0xa8, 0xe2, 0x50, 0x90,
    0x7f, 0x77, 0x79, 0xb3
};

const uint8_t nist_enc_256_ct[MXC_AES_DATA_LEN] = {
    0xdb, 0x60, 0xfb, 0xfd,
    0x29, 0x65, 0x32, 0x04,
    0xc7, 0xcc, 0x1f, 0x0a,
    0xaf, 0xeb, 0x93, 0x68
};

/*
 * 256-bit Decrypt vector
 *
 * KEY = 2b09ba39b834062b9e93f48373b8dd018dedf1e5ba1b8af831ebbacbc92a2643
 * CIPHERTEXT = 89649bd0115f30bd878567610223a59d
 * PLAINTEXT = 1f9b9b213f1884fa98b62dd6639fd33b
 *
 */

const uint8_t nist_dec_256_key[MXC_AES_KEY_256_LEN] = {
    0x43, 0x26, 0x2a, 0xc9,
    0xcb, 0xba, 0xeb, 0x31,
    0xf8, 0x8a, 0x1b, 0xba,
    0xe5, 0xf1, 0xed, 0x8d,
    0x01, 0xdd, 0xb8, 0x73,
    0x83, 0xf4, 0x93, 0x9e,
    0x2b, 0x06, 0x34, 0xb8,
    0x39, 0xba, 0x09, 0x2b
};

const uint8_t nist_dec_256_ct[MXC_AES_DATA_LEN] = {
    0x9d, 0xa5, 0x23, 0x02,
    0x61, 0x67, 0x85, 0x87,
    0xbd, 0x30, 0x5f, 0x11,
    0xd0, 0x9b, 0x64, 0x89
};

const uint8_t nist_dec_256_pt[MXC_AES_DATA_LEN] = {
    0x3b, 0xd3, 0x9f, 0x63,
    0xd6, 0x2d, 0xb6, 0x98,
    0xfa, 0x84, 0x18, 0x3f,
    0x21, 0x9b, 0x9b, 0x1f
};
