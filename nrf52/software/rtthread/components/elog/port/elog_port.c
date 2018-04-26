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
#include <rtthread.h>
#include <nrf.h>
#include <nrf_drv_uart.h>
#include <nordic_common.h>

static struct rt_mutex output_lock;

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    rt_mutex_init(&output_lock, "elog lock", RT_IPC_FLAG_PRIO);

    return result;
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    /* output to RT-Thread terminal */
    rt_kprintf("%.*s", size, log);
    //TODO output to flash
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    rt_mutex_take(&output_lock, RT_WAITING_FOREVER);
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    rt_mutex_release(&output_lock);
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
#ifdef RT_USING_RTC
    static char cur_system_time[32] = { 0 };
    static time_t now;
    static struct tm* p_tm;
    /* output current time */
    now = time(RT_NULL);
    p_tm=localtime(&now);
    rt_snprintf(cur_system_time, 32, "%02d-%02d-%02d %02d:%02d:%02d.%03d", p_tm->tm_year - 100,
            p_tm->tm_mon + 1, p_tm->tm_mday, p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec,
            999 - RTC_GetSubSecond()/10);
    return cur_system_time;
#else
    static char cur_system_time[16] = { 0 };
    rt_snprintf(cur_system_time, 16, "tick:%010d", rt_tick_get());
    return cur_system_time;
#endif
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    return rt_thread_self()->name;
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
#include <string.h>
#include <stdlib.h>
static void elog(uint8_t argc, char **argv) {
    if (argc > 1) {
        if (!strcmp(argv[1], "on") || !strcmp(argv[1], "ON")) {
            elog_set_output_enabled(true);
        } else if (!strcmp(argv[1], "off") || !strcmp(argv[1], "OFF")) {
            elog_set_output_enabled(false);
        } else {
            rt_kprintf("Please input elog on or elog off.\n");
        }
    } else {
        rt_kprintf("Please input elog on or elog off.\n");
    }
}
MSH_CMD_EXPORT(elog, EasyLogger output enabled [on/off]);

static void elog_lvl(uint8_t argc, char **argv) {
    if (argc > 1) {
        if ((atoi(argv[1]) <= ELOG_LVL_VERBOSE) && (atoi(argv[1]) >= 0)) {
            elog_set_filter_lvl(atoi(argv[1]));
        } else {
            rt_kprintf("Please input correct level(0-5).\n");
        }
    } else {
        rt_kprintf("Please input level.\n");
    }
}
MSH_CMD_EXPORT(elog_lvl, Set EasyLogger filter level);

static void elog_tag(uint8_t argc, char **argv) {
    if (argc > 1) {
        if (rt_strlen(argv[1]) <= ELOG_FILTER_TAG_MAX_LEN) {
            elog_set_filter_tag(argv[1]);
        } else {
            rt_kprintf("The tag length is too long. Max is %d.\n", ELOG_FILTER_TAG_MAX_LEN);
        }
    } else {
        elog_set_filter_tag("");
    }
}
MSH_CMD_EXPORT(elog_tag, Set EasyLogger filter tag);

static void elog_kw(uint8_t argc, char **argv) {
    if (argc > 1) {
        if (rt_strlen(argv[1]) <= ELOG_FILTER_KW_MAX_LEN) {
            elog_set_filter_kw(argv[1]);
        } else {
            rt_kprintf("The keyword length is too long. Max is %d.\n", ELOG_FILTER_KW_MAX_LEN);
        }
    } else {
        elog_set_filter_kw("");
    }
}
MSH_CMD_EXPORT(elog_kw, Set EasyLogger filter keyword);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */
