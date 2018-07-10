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
 *******************************************************************************
 */

/* MXC */
#include "mxc_config.h"
#include "board.h"
#include "mxc_assert.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"

/* Maxim CMSIS */
#include "lp.h"
#include "pwrseq_regs.h"
#include "rtc_regs.h"

#define RTC_RATIO (configRTC_TICK_RATE_HZ / configTICK_RATE_HZ)
#define MAX_SNOOZE (MXC_F_RTC_SNZ_VAL_VALUE >> MXC_F_RTC_SNZ_VAL_VALUE_POS)
#define MIN_SYSTICK 2
#define MIN_RTC_TICKS 3

static uint32_t residual = 0;

/* 
 * Sleep-check function
 *
 * Your code should over-ride this weak function and return E_NO_ERROR if
 *  tickless sleep is permissible (ie. no UART/SPI/I2C activity). Any other
 *  return code will prevent FreeRTOS from entering tickless idle.
 */
__attribute__((weak)) int freertos_permit_lp1(void)
{
  return E_NO_ERROR;
}

/* 
 * This function overrides vPortSuppressTicksAndSleep in portable/.../ARM_CM4F/port.c 
 *
 * LP1 mode will stop SysTick from counting, so that can't be
 *  used to wake up. Instead, calculate a wake-up period for the RTC to 
 *  interrupt the WFI and continue execution. 
 *
 * If an asynchronous interrupt woke the processor from LP1, lock-out further
 *  tick-less sleep until the RTC increments by at least 1 tick. TODO: Try to
 *  account for any missed time.
 *
 * Caveat: The minimum usable LP1 sleep time with this method is 3 ticks of 
 *  the 4096Hz RTC due to hardware limitations. Also, if a SysTick interrupt will likely
 *  occur soon (within 2 SysTick clock cycles), this code defers the sleep entry.
 *
 */
void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
  uint32_t rtc_ticks;
  uint32_t actual_ticks;
  uint32_t capture_timer;
    
  /* Example:
   *
   *  configTICK_RATE_HZ      512
   *  configRTC_TICK_RATE_HZ 4096
   *
   *  RTC is 8x more accurate than the normal tick in this case. We can accumulate an error term and
   *   fix up when called again as the error term equals 1 task tick
   */
  
  /* We do not currently handle to case where the RTC is slower than the RTOS tick */
  MXC_ASSERT(configRTC_TICK_RATE_HZ > configTICK_RATE_HZ);

  if (SysTick->VAL < MIN_SYSTICK) {
    /* Avoid sleeping too close to a systick interrupt */
    return;
  }
  
  /* LP1 time is limited by the snooze counter length */
  if (xExpectedIdleTime > (MAX_SNOOZE / RTC_RATIO)) {
    xExpectedIdleTime = (MAX_SNOOZE / RTC_RATIO);
  }
  /* Calculate the number of RTC ticks, assuming that the current tick will not be included */
  rtc_ticks = (xExpectedIdleTime - 1UL) * RTC_RATIO;
  
  /* Hardware limitations dictate we need at least 3 RTC ticks for LP1 sleep */
  if (rtc_ticks < MIN_RTC_TICKS) {
    /* Finish out the rest of this tick in LP2 */
    LP_EnterLP2();
    return;
  }
  
  /* Enter a critical section but don't use the taskENTER_CRITICAL()
     method as that will mask interrupts that should exit sleep mode. */
  __asm volatile( "cpsid i" );

  /* If a context switch is pending or a task is waiting for the scheduler
     to be unsuspended then abandon the low power entry. */
  /* Also check the MXC drivers for any in-progress activity */
  if ((eTaskConfirmSleepModeStatus() == eAbortSleep) ||
      (freertos_permit_lp1() != E_NO_ERROR)) { 
    /* Re-enable interrupts - see comments above the cpsid instruction()
       above. */
    __asm volatile( "cpsie i" );
    return;
  }

  /* Insert snooze value and trigger behavior B (current RTC timer + snooze value) */
  MXC_RTCTMR->snz_val = rtc_ticks;
  MXC_RTCTMR->flags = MXC_F_RTC_FLAGS_SNOOZE_B; 
  /* COMP1 flag automatically cleared by hardware */

  /* Clear any pending interrupt for COMP1 */
  NVIC_ClearPendingIRQ(RTC1_IRQn);
  /* Enable interrupt output from RTC block */
  MXC_RTCTMR->inten |= MXC_F_RTC_INTEN_COMP1;
  /* Enable interrupt in NVIC */
  NVIC_EnableIRQ(RTC1_IRQn);

  /* Clear pending wake-up flags */
  LP_ClearWakeUpFlags();

  /* Wait on RTC pending bit (if set) */
  while (MXC_RTCTMR->ctrl & MXC_F_RTC_CTRL_PENDING);
  
  /* Sleep */
  LP_EnterLP1();

  /* -- WAKE HERE -- */

  /* Snapshot the current RTC value */
  capture_timer = MXC_RTCTMR->timer;
  
  /* Dermine wake cause */
  if (LP_GetWakeUpFlags() & MXC_F_PWRSEQ_FLAGS_RTC_CMPR1) {
    /* COMP1 woke the processor */
    actual_ticks = rtc_ticks;
  } else {
    /* Determine the actual duration of LP1 */
    if (capture_timer > MXC_RTCTMR->comp[1]) {
      /* Slept for longer than we should have? RTC COMP1 did not fire, but some other interrupt brought us out */
      /* This is an error case, but handle it gracefully */
      actual_ticks = rtc_ticks + (capture_timer - MXC_RTCTMR->comp[1]);
    } else {
      /* Slept for shorter time than expected due to interrupt */
      actual_ticks = rtc_ticks - (MXC_RTCTMR->comp[1] - capture_timer);
    }
    /* Add residual from any previous early wake */
    actual_ticks += residual;
    /* Find new residual */
    residual = actual_ticks % RTC_RATIO;
  }
  
  /* Disable COMP1 interrupt at NVIC */
  NVIC_DisableIRQ(RTC1_IRQn);
  NVIC_ClearPendingIRQ(RTC1_IRQn);
  
  /* Re-enable interrupts - see comments above the cpsid instruction()
     above. */
  __asm volatile( "cpsie i" ); 

  /* 
   * Advance ticks by # actually elapsed
   */
  portENTER_CRITICAL();
  /* Future enhancement: Compare time in seconds to RTC and slew to correct */
  vTaskStepTick( actual_ticks / RTC_RATIO );
  portEXIT_CRITICAL();
}
