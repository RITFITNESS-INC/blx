#include "tension_giraffe_adapter.h"

#include "../../inc/grf_apis.h"

#ifndef GRF_TENSION_UART_PORT
/* TDO series spec: the rear PH2.0-6P connector maps to UART2 (TX=UART2-TX, RX=UART2-RX). */
#define GRF_TENSION_UART_PORT 2
#endif

#ifndef GRF_TENSION_WIN_PORT
#define GRF_TENSION_WIN_PORT 2
#endif

#define GRF_TENSION_TASK_PERIOD_MS 20u

static grf_drv_t *motor_uart = NULL;
static tension_controller_t motor_controller;
static grf_tension_ui_changed_fn ui_changed_callback = NULL;

static int giraffe_send(const uint8_t *data, size_t length, void *user_data)
{
    (void)user_data;
    if (motor_uart == NULL) {
        return -1;
    }
    return grf_drv_uart_send(motor_uart, (char *)data, (int)length) < 0 ? -1 : 0;
}

static void controller_changed(const tension_controller_t *controller, void *user_data)
{
    (void)user_data;
    if (ui_changed_callback != NULL) {
        ui_changed_callback(controller);
    }
}

static void motor_uart_receive(u8 *data, u32 length)
{
    tension_controller_on_uart_data(&motor_controller, data, length);
}

static void motor_control_task(grf_task_t *task)
{
    (void)task;
    tension_controller_tick(&motor_controller, GRF_TENSION_TASK_PERIOD_MS);
}

int grf_tension_app_init(void)
{
    grf_uart_cfg_t uart_config = {0};
    tension_controller_io_t io = {0};

    io.send = giraffe_send;
    io.changed = controller_changed;
    tension_controller_init(&motor_controller, &io);

    uart_config.port = GRF_TENSION_UART_PORT;
    uart_config.win_port = GRF_TENSION_WIN_PORT;
    uart_config.speed_e = UART_SPEED_115200;
    uart_config.bit_e = UART_BIT_8;
    uart_config.parity_e = UART_PARITY_NONE;
    uart_config.stop_e = UART_STOP_1;

    motor_uart = grf_drv_uart_open(uart_config);
    if (motor_uart == NULL) {
        return -1;
    }
    grf_drv_uart_rev_set_timeout(motor_uart, 5u);
    if (grf_drv_uart_rev_set_bfun(motor_uart, motor_uart_receive, TENSION_RX_BUFFER_SIZE) < 0) {
        grf_drv_uart_close(motor_uart);
        motor_uart = NULL;
        return -1;
    }
    if (grf_task_create(motor_control_task, GRF_TENSION_TASK_PERIOD_MS, NULL) == NULL) {
        grf_drv_uart_close(motor_uart);
        motor_uart = NULL;
        return -1;
    }
    /* Force a non-energized command before accepting user input. */
    (void)tension_controller_stop(&motor_controller);
    return 0;
}

void grf_tension_set_ui_changed_callback(grf_tension_ui_changed_fn callback)
{
    ui_changed_callback = callback;
    if (callback != NULL) {
        callback(&motor_controller);
    }
}

const tension_controller_t *grf_tension_controller_get(void)
{
    return &motor_controller;
}

void grf_tension_knob_rotate(int detents, int accelerated)
{
    (void)tension_controller_rotate(&motor_controller, detents, accelerated != 0);
}

void grf_tension_knob_short_press(void)
{
    (void)tension_controller_short_press(&motor_controller);
}

void grf_tension_knob_long_press(void)
{
    (void)tension_controller_long_press(&motor_controller);
}

void grf_tension_knob_cycle_mode(int direction)
{
    int mode;

    if (direction == 0) {
        return;
    }
    mode = (int)motor_controller.mode + (direction > 0 ? 1 : -1);
    if (mode >= (int)TENSION_MODE_COUNT) {
        mode = (int)TENSION_MODE_BASE;
    } else if (mode < (int)TENSION_MODE_BASE) {
        mode = (int)TENSION_MODE_COUNT - 1;
    }
    (void)tension_controller_select_mode(&motor_controller, (tension_mode_t)mode);
}

void grf_tension_touch_select_mode(tension_mode_t mode)
{
    (void)tension_controller_select_mode(&motor_controller, mode);
}

void grf_tension_touch_stop(void)
{
    (void)tension_controller_stop(&motor_controller);
}

void grf_tension_clear_fault(void)
{
    (void)tension_controller_clear_fault(&motor_controller);
}
