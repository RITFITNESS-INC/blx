#include "tension_controller.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t frames[32][MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE];
    size_t lengths[32];
    size_t count;
    size_t changes;
    int fail_send;
} fake_io_t;

static int fake_send(const uint8_t *data, size_t length, void *user_data)
{
    fake_io_t *fake = user_data;
    if (fake->fail_send) {
        return -1;
    }
    assert(fake->count < 32u);
    assert(length <= MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE);
    memcpy(fake->frames[fake->count], data, length);
    fake->lengths[fake->count] = length;
    ++fake->count;
    return 0;
}

static void fake_changed(const tension_controller_t *controller, void *user_data)
{
    fake_io_t *fake = user_data;
    (void)controller;
    ++fake->changes;
}

static void init_controller(tension_controller_t *controller, fake_io_t *fake)
{
    tension_controller_io_t io = {
        fake_send,
        fake_changed,
        fake
    };
    memset(fake, 0, sizeof(*fake));
    tension_controller_init(controller, &io);
}

static void build_telemetry(uint8_t *frame, int16_t force_dN)
{
    memset(frame, 0, MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE);
    frame[0] = MOTOR_FRAME_START;
    frame[1] = MOTOR_CMD_TELEMETRY;
    frame[2] = 1u;
    frame[9] = (uint8_t)(force_dN & 0xFF);
    frame[10] = (uint8_t)(((uint16_t)force_dN) >> 8);
    frame[19] = motor_protocol_checksum(frame + 1u, 18u);
    frame[20] = MOTOR_FRAME_END;
}

static void test_adjustment_and_selection(void)
{
    tension_controller_t controller;
    fake_io_t fake;

    init_controller(&controller, &fake);
    assert(controller.settings.concentric_force_dlb == 50);
    assert(tension_controller_rotate(&controller, 100, false) == TENSION_CTRL_OK);
    assert(controller.settings.concentric_force_dlb == TENSION_UI_MAX_FORCE_DLB);
    assert(tension_controller_short_press(&controller) == TENSION_CTRL_OK);
    assert(controller.selected_parameter == TENSION_PARAM_SECONDARY);
    assert(tension_controller_rotate(&controller, -100, false) == TENSION_CTRL_OK);
    assert(controller.settings.eccentric_force_dlb == 0);
    assert(tension_controller_select_mode(&controller, TENSION_MODE_BAND) == TENSION_CTRL_OK);
    assert(tension_controller_parameter_count(controller.mode) == 4u);
}

static void test_lb_conversion(void)
{
    assert(tension_force_dlb_to_dN(0) == 0);
    assert(tension_force_dlb_to_dN(10) == 44);
    assert(tension_force_dlb_to_dN(100) == 445);
    assert(tension_force_dlb_to_dN(500) == 2224);
    assert(tension_force_dN_to_dlb(445) == 100);
    assert(tension_force_dN_to_dlb(-445) == -100);
}

static void test_immediate_and_periodic_telemetry_poll(void)
{
    tension_controller_t controller;
    fake_io_t fake;

    init_controller(&controller, &fake);
    assert(tension_controller_poll_telemetry(&controller) == TENSION_CTRL_OK);
    assert(fake.count == 1u);
    assert(fake.lengths[0] == MOTOR_FRAME_TELEMETRY_REQUEST_SIZE);
    assert(fake.frames[0][0] == 0x02u);
    assert(fake.frames[0][1] == 0x0Eu);
    assert(fake.frames[0][2] == 0x0Eu);
    assert(fake.frames[0][3] == 0x03u);

    tension_controller_tick(&controller, TENSION_TELEMETRY_PERIOD_MS - 1u);
    assert(fake.count == 1u);
    tension_controller_tick(&controller, 1u);
    assert(fake.count == 2u);
    assert(memcmp(fake.frames[1], fake.frames[0], MOTOR_FRAME_TELEMETRY_REQUEST_SIZE) == 0);
}

static void test_startup_reads_status_before_safe_stop(void)
{
    tension_controller_t controller;
    fake_io_t fake;
    uint8_t telemetry[MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE];

    init_controller(&controller, &fake);
    assert(controller.startup_status_pending);
    assert(tension_controller_long_press(&controller) == TENSION_CTRL_ERROR_STATE);
    assert(tension_controller_poll_telemetry(&controller) == TENSION_CTRL_OK);
    assert(fake.count == 1u);
    assert(fake.frames[0][1] == MOTOR_CMD_TELEMETRY);

    build_telemetry(telemetry, 123);
    telemetry[2] = 0xA5u;
    telemetry[19] = 0x00u; /* V3/V5 feedback CHK is explicitly unreliable. */
    tension_controller_on_uart_data(&controller, telemetry, sizeof(telemetry));

    assert(!controller.startup_status_pending);
    assert(controller.telemetry_valid);
    assert(controller.telemetry.system_state == 0xA5u);
    assert(controller.state == TENSION_STATE_STOPPED);
    assert(fake.count == 2u);
    assert(fake.frames[1][1] == MOTOR_CMD_BASE_RESISTANCE);
    assert(fake.frames[1][6] == 0x00u);
}

static void test_startup_status_timeout_falls_back_to_stop(void)
{
    tension_controller_t controller;
    fake_io_t fake;

    init_controller(&controller, &fake);
    assert(tension_controller_poll_telemetry(&controller) == TENSION_CTRL_OK);
    tension_controller_tick(&controller, TENSION_STARTUP_STATUS_TIMEOUT_MS + 1u);
    assert(!controller.startup_status_pending);
    assert(!controller.telemetry_valid);
    assert(controller.state == TENSION_STATE_DISCONNECTED);
    assert(fake.frames[0][1] == MOTOR_CMD_TELEMETRY);
    assert(fake.frames[fake.count - 1u][1] == MOTOR_CMD_BASE_RESISTANCE);
    assert(fake.frames[fake.count - 1u][6] == 0x00u);
}

static void test_start_sends_selected_mode_immediately(void)
{
    tension_controller_t controller;
    fake_io_t fake;
    uint8_t telemetry[MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE];

    init_controller(&controller, &fake);
    assert(tension_controller_select_mode(&controller, TENSION_MODE_SPRING) == TENSION_CTRL_OK);
    controller.settings.spring_travel_mm = 100;
    controller.settings.spring_peak_force_dlb = 100;

    build_telemetry(telemetry, 0);
    tension_controller_on_uart_data(&controller, telemetry, sizeof(telemetry));

    assert(tension_controller_long_press(&controller) == TENSION_CTRL_OK);
    assert(controller.state == TENSION_STATE_ARMED);
    assert(fake.count == 2u);
    assert(fake.lengths[1] == MOTOR_FRAME_SPRING_SIZE);
    assert(fake.frames[1][1] == MOTOR_CMD_SPRING);
    assert(fake.frames[1][2] == 0x64u);
    assert(fake.frames[1][3] == 0x00u);
    assert(fake.frames[1][4] == 0xBDu); /* 10.0 lb -> 44.5 N -> 445 dN */
    assert(fake.frames[1][5] == 0x01u);

    tension_controller_tick(&controller, TENSION_LINK_TIMEOUT_MS + 1u);
    assert(controller.state == TENSION_STATE_FAULT);
    assert(fake.frames[fake.count - 1u][1] == MOTOR_CMD_BASE_RESISTANCE);
    assert(fake.frames[fake.count - 1u][6] == 0x00u);
}

static void test_each_mode_sends_its_own_command(void)
{
    static const tension_mode_t modes[] = {
        TENSION_MODE_BASE,
        TENSION_MODE_SPRING,
        TENSION_MODE_BAND,
        TENSION_MODE_ISOKINETIC,
        TENSION_MODE_VIBRATION,
        TENSION_MODE_INERTIA
    };
    static const uint8_t commands[] = {0x00u, 0x01u, 0x14u, 0x11u, 0x04u, 0x15u};
    static const size_t lengths[] = {9u, 8u, 11u, 8u, 8u, 6u};
    size_t i;

    for (i = 0u; i < sizeof(modes) / sizeof(modes[0]); ++i) {
        tension_controller_t controller;
        fake_io_t fake;
        uint8_t telemetry[MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE];

        init_controller(&controller, &fake);
        build_telemetry(telemetry, 0);
        tension_controller_on_uart_data(&controller, telemetry, sizeof(telemetry));
        assert(tension_controller_select_mode(&controller, modes[i]) == TENSION_CTRL_OK);
        assert(tension_controller_long_press(&controller) == TENSION_CTRL_OK);
        assert(fake.count == 2u);
        assert(fake.frames[1][1] == commands[i]);
        assert(fake.lengths[1] == lengths[i]);
    }
}

static void test_fragmented_telemetry_and_start(void)
{
    tension_controller_t controller;
    fake_io_t fake;
    uint8_t telemetry[MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE];
    uint8_t noisy_prefix[] = {0x55, 0x02, 0x99, 0x44};

    init_controller(&controller, &fake);
    build_telemetry(telemetry, 123);
    tension_controller_on_uart_data(&controller, noisy_prefix, sizeof(noisy_prefix));
    tension_controller_on_uart_data(&controller, telemetry, 7u);
    tension_controller_on_uart_data(
        &controller,
        telemetry + 7u,
        MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE - 7u);
    assert(controller.telemetry_valid);
    assert(controller.telemetry.total_force_dN == 123);
    assert(controller.state == TENSION_STATE_STOPPED);

    assert(tension_controller_long_press(&controller) == TENSION_CTRL_OK);
    assert(controller.state == TENSION_STATE_ARMED);
    assert(fake.lengths[fake.count - 1u] == MOTOR_FRAME_BASE_SIZE);
    assert(fake.frames[fake.count - 1u][6] == 0xFFu);

    tension_controller_on_uart_data(&controller, telemetry, sizeof(telemetry));
    assert(controller.state == TENSION_STATE_RUNNING);
    tension_controller_tick(&controller, TENSION_CONTROL_PERIOD_MS);
    assert(fake.frames[fake.count - 1u][0] == MOTOR_FRAME_START);
    assert(fake.frames[fake.count - 1u][1] == MOTOR_CMD_BASE_RESISTANCE);
    assert(fake.frames[fake.count - 1u][6] == 0xFFu);
}

static void test_ui_safety_limits(void)
{
    tension_controller_t controller;
    fake_io_t fake;

    init_controller(&controller, &fake);
    assert(tension_controller_select_mode(&controller, TENSION_MODE_SPRING) == TENSION_CTRL_OK);
    assert(tension_controller_rotate(&controller, 10000, true) == TENSION_CTRL_OK);
    assert(controller.settings.spring_travel_mm == TENSION_UI_MAX_TRAVEL_MM);

    assert(tension_controller_select_mode(&controller, TENSION_MODE_ISOKINETIC) == TENSION_CTRL_OK);
    assert(tension_controller_rotate(&controller, 10000, true) == TENSION_CTRL_OK);
    assert(controller.settings.isokinetic_speed_mm_s == TENSION_UI_MAX_SPEED_MM_S);

    assert(tension_controller_select_mode(&controller, TENSION_MODE_VIBRATION) == TENSION_CTRL_OK);
    assert(tension_controller_short_press(&controller) == TENSION_CTRL_OK);
    assert(tension_controller_rotate(&controller, 10000, true) == TENSION_CTRL_OK);
    assert(controller.settings.vibration_frequency_hz == TENSION_UI_MAX_FREQUENCY_HZ);
}

static void test_timeout_fault_and_clear(void)
{
    tension_controller_t controller;
    fake_io_t fake;
    uint8_t telemetry[MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE];

    init_controller(&controller, &fake);
    build_telemetry(telemetry, 0);
    tension_controller_on_uart_data(&controller, telemetry, sizeof(telemetry));
    assert(tension_controller_long_press(&controller) == TENSION_CTRL_OK);
    tension_controller_on_uart_data(&controller, telemetry, sizeof(telemetry));
    assert(controller.state == TENSION_STATE_RUNNING);

    tension_controller_tick(&controller, TENSION_LINK_TIMEOUT_MS + 1u);
    assert(controller.state == TENSION_STATE_FAULT);
    assert(!controller.telemetry_valid);
    assert(tension_controller_rotate(&controller, 1, false) == TENSION_CTRL_ERROR_STATE);

    tension_controller_on_uart_data(&controller, telemetry, sizeof(telemetry));
    assert(controller.state == TENSION_STATE_FAULT);
    assert(tension_controller_long_press(&controller) == TENSION_CTRL_OK);
    assert(controller.state == TENSION_STATE_STOPPED);
}

static void test_send_failure_faults_running_controller(void)
{
    tension_controller_t controller;
    fake_io_t fake;
    uint8_t telemetry[MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE];

    init_controller(&controller, &fake);
    build_telemetry(telemetry, 0);
    tension_controller_on_uart_data(&controller, telemetry, sizeof(telemetry));
    assert(tension_controller_long_press(&controller) == TENSION_CTRL_OK);
    tension_controller_on_uart_data(&controller, telemetry, sizeof(telemetry));
    fake.fail_send = 1;
    tension_controller_tick(&controller, TENSION_CONTROL_PERIOD_MS);
    assert(controller.state == TENSION_STATE_FAULT);
}

int main(void)
{
    test_lb_conversion();
    test_immediate_and_periodic_telemetry_poll();
    test_startup_reads_status_before_safe_stop();
    test_startup_status_timeout_falls_back_to_stop();
    test_adjustment_and_selection();
    test_ui_safety_limits();
    test_start_sends_selected_mode_immediately();
    test_each_mode_sends_its_own_command();
    test_fragmented_telemetry_and_start();
    test_timeout_fault_and_clear();
    test_send_failure_faults_running_controller();
    puts("tension_controller tests passed");
    return 0;
}
