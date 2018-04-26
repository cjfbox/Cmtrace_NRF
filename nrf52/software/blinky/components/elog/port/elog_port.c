/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for non-os stm32f10x.
 * Created on: 2015-04-28
 */

#include "elog.h"
#include <stdio.h>
#include <nrf.h>
#include <nrf_drv_uart.h>
#include <nordic_common.h>

static nrf_drv_uart_t uart_dev = NRF_DRV_UART_INSTANCE(NRF_LOG_BACKEND_UART_INSTANCE);

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;
    nrf_drv_uart_config_t uart_config = NRF_DRV_UART_DEFAULT_CONFIG;

    uart_config.hwfc = (nrf_uart_hwfc_t) NRF_LOG_BACKEND_SERIAL_UART_FLOW_CONTROL;
    uart_config.pseltxd = NRF_LOG_BACKEND_SERIAL_UART_TX_PIN;
    uart_config.pselrxd = NRF_LOG_BACKEND_SERIAL_UART_RX_PIN;
    uart_config.pselrts = NRF_LOG_BACKEND_SERIAL_UART_RTS_PIN;
    uart_config.pselcts = NRF_LOG_BACKEND_SERIAL_UART_CTS_PIN;
    uart_config.baudrate = (nrf_uart_baudrate_t) NRF_LOG_BACKEND_SERIAL_UART_BAUDRATE;
    nrf_drv_uart_uninit(&uart_dev);
    nrf_drv_uart_init(&uart_dev, &uart_config, NULL);


    return result;
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    /* output to uart port */
    nrf_drv_uart_tx(&uart_dev, (uint8_t *)log, size);
    //TODO output to flash
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    __disable_irq();
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    __enable_irq();
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    return "10:08:12";
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    return "pid:1008";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    return "tid:24";
}
