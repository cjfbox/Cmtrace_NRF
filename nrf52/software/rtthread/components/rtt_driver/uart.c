/*
 * File      : uart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2017, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-05-01     armink       the first version
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <rthw.h>
#include <rtdevice.h>

#include <nrf.h>
#include <nrf_drv_uart.h>
#include <nordic_common.h>

#include "board.h"
#include "uart.h"

#define UART0_INSTANCE_INDEX           0
#define UART0_FLOW_CONTROL             UART_CONFIG_HWFC_Disabled
#define UART0_TX_PIN                   20
#define UART0_RX_PIN                   19
#define UART0_RTS_PIN                  0xFFFFFFFF
#define UART0_CTS_PIN                  0xFFFFFFFF

#if defined(RT_USING_UART0)
static nrf_drv_uart_t uart0_dev = NRF_DRV_UART_INSTANCE(UART0_INSTANCE_INDEX);
#endif

struct nrf52_uart
{
    nrf_drv_uart_config_t config;
    nrf_drv_uart_t *device;
    struct rt_ringbuffer tx_rb;
    rt_uint8_t tx_buffer[RT_SERIAL_RB_BUFSZ];
    bool has_recved;
    uint8_t recved_data;
};

static void uart_event_handler(nrf_drv_uart_event_t * p_event, void * p_context)
{
    struct rt_serial_device *serial = (struct rt_serial_device *)p_context;
    struct nrf52_uart* uart = (struct nrf52_uart *)serial->parent.user_data;
    rt_uint8_t ch;

    RT_ASSERT(serial);
    RT_ASSERT(uart);
    
    rt_interrupt_enter();

    switch (p_event->type) {
    case NRF_DRV_UART_EVT_RX_DONE: {
        uart->has_recved = true;
        uart->recved_data = p_event->data.rxtx.p_data[0];
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
        break;
    }
    case NRF_DRV_UART_EVT_TX_DONE: {
        if (rt_ringbuffer_getchar(&uart->tx_rb, &ch) > 0) {
            nrf_drv_uart_tx(uart->device, (uint8_t *) &(ch), 1);
        }
        break;
    }
    case NRF_DRV_UART_EVT_ERROR: {
        rt_kprintf("Error: Reported by UART peripheral.\n");
        break;
    }
    }
    
    rt_interrupt_leave();
}

static rt_err_t nrf52_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct nrf52_uart* uart = (struct nrf52_uart *)serial->parent.user_data;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(uart != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart->config.hwfc = UART0_FLOW_CONTROL;
    uart->config.pseltxd = UART0_TX_PIN;
    uart->config.pselrxd = UART0_RX_PIN;
    uart->config.pselrts = UART0_RTS_PIN;
    uart->config.pselcts = UART0_CTS_PIN;
    uart->config.p_context = serial;

    switch(cfg->baud_rate) {
    case 1200   :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud1200;break;
    case 2400   :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud2400;break;
    case 4800   :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud4800;break;
    case 9600   :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud9600;break;
    case 14400  :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud14400;break;
    case 19200  :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud19200;break;
    case 28800  :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud28800;break;
    case 31250  :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud31250;break;
    case 38400  :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud38400;break;
    case 56000  :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud56000;break;
    case 57600  :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud57600;break;
    case 76800  :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud76800;break;
    case 115200 :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud115200;break;
    case 230400 :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud230400;break;
    case 250000 :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud250000;break;
    case 460800 :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud460800;break;
    case 921600 :uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud921600;break;
    case 1000000:uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud1M;break;
    default :{
        rt_kprintf("Error: Baudrate %d isn't supported. Using default 115200.\n", cfg->baud_rate);
        uart->config.baudrate = UARTE_BAUDRATE_BAUDRATE_Baud115200;
    }
    }

    nrf_drv_uart_uninit(uart->device);
    nrf_drv_uart_init(uart->device, &uart->config, uart_event_handler);
    nrf_drv_uart_rx_enable(uart->device);

    return RT_EOK;
}

static rt_err_t nrf52_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct nrf52_uart* uart = (struct nrf52_uart *)serial->parent.user_data;
    char ch;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(uart != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        nrf_drv_uart_rx_disable(uart->device);
        break;
    case RT_DEVICE_CTRL_SET_INT:
        nrf_drv_uart_rx_enable(uart->device);
        /* start new rx */
        nrf_drv_uart_rx(uart->device, (uint8_t *) &ch, 1);
        break;
    }

    return RT_EOK;
}

static int nrf52_putc(struct rt_serial_device *serial, char ch)
{
    struct nrf52_uart* uart = (struct nrf52_uart *)serial->parent.user_data;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(uart != RT_NULL);

    /* save data to ringbuffer first */
    if (rt_ringbuffer_putchar(&uart->tx_rb, ch)) {
        // The new byte has been added to ringbuffer. It will be picked up from there
        // (in 'uart_event_handler') when all preceding bytes are transmitted.
        // But if UART is not transmitting anything at the moment, we must start
        // a new transmission here.
        if (!nrf_drv_uart_tx_in_progress(uart->device)) {
            // This operation should be almost always successful, since we've
            // just added a byte to ringbuffer, but if some bigger delay occurred
            // (some heavy interrupt handler routine has been executed) since
            // that time, ringbuffer might be empty already.
            if (rt_ringbuffer_getchar(&uart->tx_rb, (rt_uint8_t *)&ch) > 0) {
                nrf_drv_uart_tx(uart->device, (uint8_t *) &(ch), 1);
                return 1;
            }
        }
    }
    return 0;
}

static int nrf52_getc(struct rt_serial_device *serial)
{
    int ch = -1;
    struct nrf52_uart* uart = (struct nrf52_uart *)serial->parent.user_data;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(uart != RT_NULL);

    if (serial->parent.open_flag & RT_DEVICE_FLAG_INT_RX) {
        /* interrupt reading */
        if (uart->has_recved) {
            /* read buffer length only 1 */
            ch = uart->recved_data;
            uart->has_recved = false;
            /* start new rx */
            nrf_drv_uart_rx(uart->device, (uint8_t *) &ch, 1);
        }
    } else if (serial->parent.open_flag & RT_DEVICE_FLAG_DMA_RX) {
        /* DMA reading */
    } else {
        /* poll reading */
        nrf_drv_uart_rx(uart->device, (uint8_t *) &ch, 1);
    }

    return ch;
}

static const struct rt_uart_ops nrf52_uart_ops =
{
    nrf52_configure,
    nrf52_control,
    nrf52_putc,
    nrf52_getc,
};

#if defined(RT_USING_UART0)
/* UART0 device driver structure */
struct nrf52_uart uart0 =
{
    NRF_DRV_UART_DEFAULT_CONFIG,
    &uart0_dev,
};
struct rt_serial_device serial0;
#endif /* RT_USING_UART0 */

int rt_hw_uart_init(void)
{
    struct nrf52_uart *uart;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

#ifdef RT_USING_UART0
    uart = &uart0;

    serial0.ops    = &nrf52_uart_ops;
    serial0.config = config;

    rt_ringbuffer_init(&(uart->tx_rb), uart->tx_buffer, sizeof(uart->tx_buffer));

    /* register UART0 device */
    rt_hw_serial_register(&serial0,
                          "uart0",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          uart);
#endif /* RT_USING_UART0 */

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_uart_init);
