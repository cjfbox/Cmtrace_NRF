/*
 * File      : board.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <startup_config.h>

extern int __StackLimit;
#define NRF_SRAM_BEGIN       (&(__StackLimit) + __STARTUP_CONFIG_STACK_SIZE/sizeof(int))
#define NRF_SRAM_END         (0x20000000 + 64*1024)

#define RT_USING_UART0

void rt_hw_board_init(void);

#endif
