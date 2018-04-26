#define LOG_TAG    "APP"

#include <elog.h>
#include <rthw.h>
#include <rtthread.h>
#include <finsh.h>
#include <shell.h>
#include <board.h>
#include <stdlib.h>
#include <stdio.h>
#include <delay_conf.h>
#include <nrf_delay.h>
#include <boards.h>
#include <nrf.h>
#include <cm_backtrace.h>

#define thread_sys_monitor_prio        30
#define HARDWARE_VERSION               "V1.0.0"
#define SOFTWARE_VERSION               "V0.1.0"

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t thread_sys_monitor_stack[512];

struct rt_thread thread_sys_monitor;

/**
 * System monitor thread.
 *
 * @param parameter parameter
 */
void thread_entry_sys_monitor(void* parameter)
{
    while (1)
    {
        for (int i = 0; i < LEDS_NUMBER; i++)
        {
            bsp_board_led_invert(i);

            rt_thread_delay(rt_tick_from_millisecond(500));
        }
    }
}

static void rtt_user_assert_hook(const char* ex, const char* func, rt_size_t line) {
    uint8_t _continue = 1;

    rt_enter_critical();
    /* disable logger output lock */
    elog_output_lock_enabled(false);
    /* output rtt assert information */
    elog_a("rtt", "(%s) has assert failed at %s:%ld.\n", ex, func, line);

    while (_continue == 1);
}

static rt_err_t exception_hook(void *context) {
    uint8_t _continue = 1;

    rt_enter_critical();
    /* disable logger output lock */
    elog_output_lock_enabled(false);

#ifdef RT_USING_FINSH
    extern long list_thread(void);
    list_thread();
#endif

    cm_backtrace_fault(*((uint32_t *)(cmb_get_sp() + sizeof(uint32_t) * 8)), cmb_get_sp() + sizeof(uint32_t) * 9);

    while (_continue == 1);

    return RT_EOK;
}

/**
 * System initialization thread.
 *
 * @param parameter parameter
 */
void sys_init_thread(void* parameter){
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_init();
#endif

#ifdef RT_USING_FINSH
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

    /* initialize EasyLogger */
    elog_init();
    /* set EasyLogger log format */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO));
#ifdef ELOG_COLOR_ENABLE
    elog_set_text_color_enabled(true);
#endif
    /* start EasyLogger */
    elog_start();

    log_i("Starting...");

    /* CmBacktrace initialize */
    cm_backtrace_init("rtthread", HARDWARE_VERSION, SOFTWARE_VERSION);
    
    /* set hardware exception hook */
    rt_hw_exception_install(exception_hook);

    /* set RT-Thread assert hook */
    rt_assert_set_hook(rtt_user_assert_hook);

    rt_thread_delete(rt_thread_self());
}

int rt_application_init(void)
{
    rt_thread_t init_thread = NULL;
    rt_thread_init(&thread_sys_monitor,
                   "sys_monitor",
                   thread_entry_sys_monitor,
                   RT_NULL,
                   thread_sys_monitor_stack,
                   sizeof(thread_sys_monitor_stack),
                   thread_sys_monitor_prio, 5);
    rt_thread_startup(&thread_sys_monitor);

    init_thread = rt_thread_create("sys_init", sys_init_thread,
            NULL, 1024, 10, 10);
    if (init_thread != NULL) {
        rt_thread_startup(init_thread);
    }
    return 0;
}

/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup(void)
{
    /* init board */
    rt_hw_board_init();

    /* show version */
    rt_show_version();

    /* init tick */
    rt_system_tick_init();

    /* init kernel object */
    rt_system_object_init();

    /* init timer system */
    rt_system_timer_init();

#ifdef RT_USING_HEAP
	/* init memory system */
    rt_system_heap_init((void*)NRF_SRAM_BEGIN, (void*)NRF_SRAM_END);
#endif

    /* init scheduler system */
    rt_system_scheduler_init();

    /* initialize timer */
    rt_system_timer_init();

    /* init timer thread */
    rt_system_timer_thread_init();

    /* init application */
    rt_application_init();

    /* init idle thread */
    rt_thread_idle_init();

    /* start scheduler */
    rt_system_scheduler_start();

    /* never reach here */
    return;
}

int main(void){
    /* disable interrupt first */
    rt_hw_interrupt_disable();

    /* startup RT-Thread RTOS */
    rtthread_startup();

    return 0;
}
