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
 * $Date: 2016-04-21 18:01:16 -0500 (Thu, 21 Apr 2016) $
 * $Revision: 22467 $
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   Hello World!
 * @details This example uses the UART to print to a terminal and flashes an LED.
 */

/***** Includes *****/
#include "mxc_config.h"
#include "stdio.h"
#include "board.h"

#include <iostream>

/***** Definitions *****/

class myClass {
public:
  myClass(int);
  int geti(void);
  int inci(int);
  
private:
  int i;
};

/***** Globals *****/

/***** Functions *****/

myClass::myClass(int x) : i(x) { }

int myClass::geti(void) {
  return i;
}

int myClass::inci(int j) {
  i += j;
  return i;
}

struct test_class {
  test_class () { 
    m = new char[10];
    if (m != NULL) {
      m[0] = 'M';
      m[1] = 'X';
      m[2] = 'I';
      m[3] = 'M';
      m[4] = 0x00;
    }
  }
  ~test_class () {
    delete m;
  }
  char *foo() {
    return m;
  }
  char * m;
} g;

// *****************************************************************************
int main(void)
{
  myClass my_class(5);

  test_class h;

  std::cout << "MAX326xx C++ Test Program\n";
  
  /* This object's constructor was static, and was run before main() */
  printf("g.m = 0x%08lx\n", g.m);
  /* This object's constructor was run after object was allocated on stack */
  printf("h.m = 0x%08lx\n", h.m);
  
  printf("my_class has %d\n", my_class.geti());
  my_class.inci(4);
  printf("my_class has %d\n", my_class.geti());
  my_class.inci(-5);
  printf("my_class has %d\n", my_class.geti());
  my_class.inci(-4);
  printf("my_class has %d\n", my_class.geti());

  return 0;
}
