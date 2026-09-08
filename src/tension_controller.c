#include "tension_controller.h"

#include <limits.h>
#include <string.h>

static uint32_t saturating_add_u32(uint32_t value, uint32_t increment)
{
    if (UINT32_MAX - value < increment) {
        return UINT32_MAX;
    }
    return value + increment;
}

static int32_t clamp_i64(int64_t value, int32_t minimum, int32_t maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return (int32_t)value;
}

int32_t tension_force_dlb_to_dN(int32_t force_dlb)
{
    int32_t magnitude = force_dlb < 0 ? -force_dlb : force_dlb;
    int32_t converted;

    /* The controller clamps this input below 900. Keep the calculation 32-bit
       because the TR2 runtime does not export the compiler's 64-bit divide helper. */
    converted = (magnitude * 444822 + 50000) / 100000;
    return force_dlb < 0 ? -converted : converted;
}

int32_t tension_force_dN_to_dlb(int32_t force_dN)
{
    int32_t magnitude = force_dN < 0 ? -force_dN : force_dN;
    int32_t converted;

    /* Divide the exact ratio by two so every possible int16 telemetry value
       remains within signed 32-bit arithmetic. */
    converted = (magnitude * 50000 + 111205) / 222411;
    return force_dN < 0 ? -converted : converted;
}

static void notify_changed(tension_controller_t *controller)
{
    if (controller->io.changed != NULL) {
        controller->io.changed(controller, controller->io.user_data);
    }
}

static void set_state(tension_controller_t *controller, tension_state_t state)
{
    if (controller->state != state) {
        controller->state = state;
        notify_changed(controller);
    }
}

static tension_ctrl_result_t send_bytes(
    tension_controller_t *controller,
    const uint8_t *data,
    size_t length)
{
    if (controller->io.send == NULL || controller->io.send(data, length, controller->io.user_data) != 0) {
        controller->last_error = TENSION_CTRL_ERROR_SEND;
        return TENSION_CTRL_ERROR_SEND;
    }
    controller->last_error = TENSION_CTRL_OK;
    return TENSION_CTRL_OK;
}

static tension_ctrl_result_t send_stop_frame(tension_controller_t *controller)
{
    uint8_t frame[MOTOR_FRAME_BASE_SIZE];
    size_t written = 0u;
    motor_result_t result = motor_protocol_build_base(0, 0, false, frame, sizeof(frame), &written);

    controller->last_protocol_error = result;
    if (result != MOTOR_OK) {
        controller->last_error = TENSION_CTRL_ERROR_PROTOCOL;
        return TENSION_CTRL_ERROR_PROTOCOL;
    }
    return send_bytes(controller, frame, written);
}

static tension_ctrl_result_t send_telemetry_request(tension_controller_t *controller)
{
    uint8_t frame[MOTOR_FRAME_TELEMETRY_REQUEST_SIZE];
    size_t written = 0u;
    motor_result_t result = motor_protocol_build_telemetry_request(frame, sizeof(frame), &written);

    controller->last_protocol_error = result;
    if (result != MOTOR_OK) {
        controller->last_error = TENSION_CTRL_ERROR_PROTOCOL;
        return TENSION_CTRL_ERROR_PROTOCOL;
    }
    return send_bytes(controller, frame, written);
}

tension_ctrl_result_t tension_controller_poll_telemetry(tension_controller_t *controller)
{
    if (controller == NULL) {
        return TENSION_CTRL_ERROR_ARGUMENT;
    }
    controller->telemetry_poll_elapsed_ms = 0u;
    return send_telemetry_request(controller);
}

static tension_ctrl_result_t send_active_frame(tension_controller_t *controller)
{
    uint8_t frame[MOTOR_FRAME_BAND_SIZE];
    size_t written = 0u;
    motor_result_t result;

    switch (controller->mode) {
    case TENSION_MODE_BASE:
        result = motor_protocol_build_base(
            tension_force_dlb_to_dN(controller->settings.concentric_force_dlb),
            tension_force_dlb_to_dN(controller->settings.eccentric_force_dlb),
            true,
            frame,
            sizeof(frame),
            &written);
        break;
    case TENSION_MODE_SPRING:
        result = motor_protocol_build_spring(
            controller->settings.spring_travel_mm,
            tension_force_dlb_to_dN(controller->settings.spring_peak_force_dlb),
            frame,
            sizeof(frame),
            &written);
        break;
    case TENSION_MODE_BAND:
        result = motor_protocol_build_band(
            controller->settings.band_rest_length_mm,
            controller->settings.band_maximum_length_mm,
            tension_force_dlb_to_dN(controller->settings.band_peak_force_dlb),
            controller->settings.band_curve_factor,
            frame,
            sizeof(frame),
            &written);
        break;
    case TENSION_MODE_ISOKINETIC:
        result = motor_protocol_build_isokinetic(
            controller->settings.isokinetic_speed_mm_s,
            tension_force_dlb_to_dN(controller->settings.isokinetic_slope_dlb_per_m_s),
            frame,
            sizeof(frame),
            &written);
        break;
    case TENSION_MODE_VIBRATION:
        result = motor_protocol_build_vibration(
            tension_force_dlb_to_dN(controller->settings.vibration_amplitude_dlb),
            controller->settings.vibration_frequency_hz,
            frame,
            sizeof(frame),
            &written);
        break;
    case TENSION_MODE_INERTIA:
        result = motor_protocol_build_inertia(
            tension_force_dlb_to_dN(controller->settings.inertia_breakout_force_dlb),
            frame,
            sizeof(frame),
            &written);
        break;
    default:
        result = MOTOR_ERROR_COMMAND;
        break;
    }

    controller->last_protocol_error = result;
    if (result != MOTOR_OK) {
        controller->last_error = TENSION_CTRL_ERROR_PROTOCOL;
        return TENSION_CTRL_ERROR_PROTOCOL;
    }
    return send_bytes(controller, frame, written);
}

uint8_t tension_controller_parameter_count(tension_mode_t mode)
{
    switch (mode) {
    case TENSION_MODE_BASE:
    case TENSION_MODE_SPRING:
    case TENSION_MODE_ISOKINETIC:
    case TENSION_MODE_VIBRATION:
        return 2u;
    case TENSION_MODE_BAND:
        return 4u;
    case TENSION_MODE_INERTIA:
        return 1u;
    default:
        return 0u;
    }
}

void tension_controller_init(
    tension_controller_t *controller,
    const tension_controller_io_t *io)
{
    if (controller == NULL) {
        return;
    }

    memset(controller, 0, sizeof(*controller));
    if (io != NULL) {
        controller->io = *io;
    }
    controller->state = TENSION_STATE_DISCONNECTED;
    controller->mode = TENSION_MODE_BASE;
    controller->startup_status_pending = true;
    controller->settings.concentric_force_dlb = 50;
    controller->settings.eccentric_force_dlb = 50;
    controller->settings.spring_travel_mm = 100;
    controller->settings.spring_peak_force_dlb = 50;
    controller->settings.band_rest_length_mm = 100;
    controller->settings.band_maximum_length_mm = 500;
    controller->settings.band_peak_force_dlb = 50;
    controller->settings.band_curve_factor = 1u;
    controller->settings.isokinetic_speed_mm_s = 100;
    controller->settings.isokinetic_slope_dlb_per_m_s = 100;
    controller->settings.vibration_amplitude_dlb = 50;
    controller->settings.vibration_frequency_hz = 10;
    controller->settings.inertia_breakout_force_dlb = 50;
    controller->last_error = TENSION_CTRL_OK;
    controller->last_protocol_error = MOTOR_OK;
    notify_changed(controller);
}

static void handle_valid_telemetry(
    tension_controller_t *controller,
    const motor_telemetry_t *telemetry)
{
    bool first_startup_status = controller->startup_status_pending;

    controller->telemetry = *telemetry;
    controller->startup_status_pending = false;
    controller->startup_status_elapsed_ms = 0u;
    controller->telemetry_valid = true;
    controller->telemetry_age_ms = 0u;
    controller->last_error = TENSION_CTRL_OK;
    controller->last_protocol_error = MOTOR_OK;

    if (first_startup_status) {
        /* The startup feedback request must be the first UART command. Once the
           board state has been captured, force a known non-energized baseline. */
        if (send_stop_frame(controller) != TENSION_CTRL_OK) {
            controller->state = TENSION_STATE_FAULT;
        } else {
            controller->state = TENSION_STATE_STOPPED;
        }
    } else if (controller->state == TENSION_STATE_DISCONNECTED) {
        controller->state = TENSION_STATE_STOPPED;
    } else if (controller->state == TENSION_STATE_ARMED) {
        controller->state = TENSION_STATE_RUNNING;
        controller->control_elapsed_ms = TENSION_CONTROL_PERIOD_MS;
    }
    notify_changed(controller);
}

static void process_rx_buffer(tension_controller_t *controller)
{
    while (controller->rx_length != 0u) {
        size_t start = 0u;
        motor_telemetry_t telemetry;
        motor_result_t result;

        while (start < controller->rx_length && controller->rx_buffer[start] != MOTOR_FRAME_START) {
            ++start;
        }
        if (start != 0u) {
            memmove(controller->rx_buffer, controller->rx_buffer + start, controller->rx_length - start);
            controller->rx_length -= start;
        }
        if (controller->rx_length < 2u) {
            return;
        }
        if (controller->rx_buffer[1] != MOTOR_CMD_TELEMETRY) {
            memmove(controller->rx_buffer, controller->rx_buffer + 1u, controller->rx_length - 1u);
            --controller->rx_length;
            continue;
        }
        if (controller->rx_length < MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE) {
            return;
        }

        result = motor_protocol_parse_telemetry(
            controller->rx_buffer,
            MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE,
            &telemetry);
        controller->last_protocol_error = result;
        if (result == MOTOR_OK) {
            memmove(
                controller->rx_buffer,
                controller->rx_buffer + MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE,
                controller->rx_length - MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE);
            controller->rx_length -= MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE;
            handle_valid_telemetry(controller, &telemetry);
        } else {
            controller->last_error = TENSION_CTRL_ERROR_PROTOCOL;
            memmove(controller->rx_buffer, controller->rx_buffer + 1u, controller->rx_length - 1u);
            --controller->rx_length;
        }
    }
}

void tension_controller_on_uart_data(
    tension_controller_t *controller,
    const uint8_t *data,
    size_t length)
{
    size_t i;

    if (controller == NULL || (data == NULL && length != 0u)) {
        return;
    }
    for (i = 0u; i < length; ++i) {
        if (controller->rx_length == sizeof(controller->rx_buffer)) {
            memmove(controller->rx_buffer, controller->rx_buffer + 1u, controller->rx_length - 1u);
            --controller->rx_length;
        }
        controller->rx_buffer[controller->rx_length++] = data[i];
        process_rx_buffer(controller);
    }
}

void tension_controller_tick(tension_controller_t *controller, uint32_t elapsed_ms)
{
    tension_ctrl_result_t result;

    if (controller == NULL) {
        return;
    }

    controller->telemetry_poll_elapsed_ms = saturating_add_u32(
        controller->telemetry_poll_elapsed_ms,
        elapsed_ms);
    controller->control_elapsed_ms = saturating_add_u32(
        controller->control_elapsed_ms,
        elapsed_ms);
    if (controller->startup_status_pending) {
        controller->startup_status_elapsed_ms = saturating_add_u32(
            controller->startup_status_elapsed_ms,
            elapsed_ms);
    }
    if (controller->telemetry_valid || controller->state == TENSION_STATE_ARMED ||
        controller->state == TENSION_STATE_RUNNING) {
        controller->telemetry_age_ms = saturating_add_u32(controller->telemetry_age_ms, elapsed_ms);
    }

    if (controller->telemetry_poll_elapsed_ms >= TENSION_TELEMETRY_PERIOD_MS) {
        controller->telemetry_poll_elapsed_ms %= TENSION_TELEMETRY_PERIOD_MS;
        result = send_telemetry_request(controller);
        if (result != TENSION_CTRL_OK &&
            (controller->state == TENSION_STATE_ARMED || controller->state == TENSION_STATE_RUNNING)) {
            set_state(controller, TENSION_STATE_FAULT);
            return;
        }
    }

    if (controller->startup_status_pending &&
        controller->startup_status_elapsed_ms > TENSION_STARTUP_STATUS_TIMEOUT_MS) {
        /* Do not wait forever if the board cannot answer. The first command was
           still the status request; this is the delayed safety fallback. */
        controller->startup_status_pending = false;
        result = send_stop_frame(controller);
        if (result != TENSION_CTRL_OK) {
            set_state(controller, TENSION_STATE_FAULT);
            return;
        }
        notify_changed(controller);
    }

    if ((controller->state == TENSION_STATE_ARMED || controller->state == TENSION_STATE_RUNNING) &&
        controller->telemetry_age_ms > TENSION_LINK_TIMEOUT_MS) {
        controller->telemetry_valid = false;
        (void)send_stop_frame(controller);
        set_state(controller, TENSION_STATE_FAULT);
        return;
    }

    if (controller->telemetry_valid && controller->telemetry_age_ms > TENSION_LINK_TIMEOUT_MS) {
        controller->telemetry_valid = false;
        (void)send_stop_frame(controller);
        set_state(controller, TENSION_STATE_DISCONNECTED);
        return;
    }

    if ((controller->state == TENSION_STATE_ARMED || controller->state == TENSION_STATE_RUNNING) &&
        controller->control_elapsed_ms >= TENSION_CONTROL_PERIOD_MS) {
        controller->control_elapsed_ms %= TENSION_CONTROL_PERIOD_MS;
        result = send_active_frame(controller);
        if (result != TENSION_CTRL_OK) {
            (void)send_stop_frame(controller);
            set_state(controller, TENSION_STATE_FAULT);
        }
    }
}

tension_ctrl_result_t tension_controller_select_mode(
    tension_controller_t *controller,
    tension_mode_t mode)
{
    if (controller == NULL || mode < TENSION_MODE_BASE || mode >= TENSION_MODE_COUNT) {
        return TENSION_CTRL_ERROR_ARGUMENT;
    }
    if (controller->state == TENSION_STATE_RUNNING || controller->state == TENSION_STATE_ARMED) {
        return TENSION_CTRL_ERROR_STATE;
    }
    controller->mode = mode;
    controller->selected_parameter = TENSION_PARAM_PRIMARY;
    if (controller->state == TENSION_STATE_STOPPED) {
        controller->state = TENSION_STATE_EDITING;
    }
    notify_changed(controller);
    return TENSION_CTRL_OK;
}

static void adjust_selected_value(
    tension_controller_t *controller,
    int32_t detents,
    bool accelerated)
{
    int32_t multiplier = accelerated ? 5 : 1;
    int32_t step = 1;
    int32_t *value = NULL;
    int32_t minimum = 0;
    int32_t maximum = INT16_MAX;

    switch (controller->mode) {
    case TENSION_MODE_BASE:
        value = controller->selected_parameter == TENSION_PARAM_PRIMARY
            ? &controller->settings.concentric_force_dlb
            : &controller->settings.eccentric_force_dlb;
        step = 10;
        maximum = TENSION_UI_MAX_FORCE_DLB;
        break;
    case TENSION_MODE_SPRING:
        if (controller->selected_parameter == TENSION_PARAM_PRIMARY) {
            value = &controller->settings.spring_travel_mm;
            maximum = TENSION_UI_MAX_TRAVEL_MM;
        } else {
            value = &controller->settings.spring_peak_force_dlb;
            step = 10;
            maximum = TENSION_UI_MAX_FORCE_DLB;
        }
        break;
    case TENSION_MODE_BAND:
        if (controller->selected_parameter == TENSION_PARAM_PRIMARY) {
            value = &controller->settings.band_rest_length_mm;
            maximum = controller->settings.band_maximum_length_mm;
        } else if (controller->selected_parameter == TENSION_PARAM_SECONDARY) {
            value = &controller->settings.band_maximum_length_mm;
            minimum = controller->settings.band_rest_length_mm;
            maximum = TENSION_UI_MAX_TRAVEL_MM;
        } else if (controller->selected_parameter == TENSION_PARAM_TERTIARY) {
            value = &controller->settings.band_peak_force_dlb;
            step = 10;
            maximum = TENSION_UI_MAX_FORCE_DLB;
        } else {
            int64_t adjusted = (int64_t)controller->settings.band_curve_factor +
                (int64_t)detents * multiplier;
            controller->settings.band_curve_factor = (uint8_t)clamp_i64(adjusted, 0, 4);
        }
        break;
    case TENSION_MODE_ISOKINETIC:
        if (controller->selected_parameter == TENSION_PARAM_PRIMARY) {
            value = &controller->settings.isokinetic_speed_mm_s;
            step = 10;
            maximum = TENSION_UI_MAX_SPEED_MM_S;
        } else {
            value = &controller->settings.isokinetic_slope_dlb_per_m_s;
            step = 10;
            maximum = TENSION_UI_MAX_SLOPE_DLB_PER_M_S;
        }
        break;
    case TENSION_MODE_VIBRATION:
        if (controller->selected_parameter == TENSION_PARAM_PRIMARY) {
            value = &controller->settings.vibration_amplitude_dlb;
            step = 10;
            maximum = TENSION_UI_MAX_FORCE_DLB;
        } else {
            value = &controller->settings.vibration_frequency_hz;
            maximum = TENSION_UI_MAX_FREQUENCY_HZ;
        }
        break;
    case TENSION_MODE_INERTIA:
        value = &controller->settings.inertia_breakout_force_dlb;
        step = 10;
        maximum = TENSION_UI_MAX_FORCE_DLB;
        break;
    default:
        break;
    }

    if (value != NULL) {
        int64_t adjusted = (int64_t)*value + (int64_t)detents * step * multiplier;
        *value = clamp_i64(adjusted, minimum, maximum);
    }
}

tension_ctrl_result_t tension_controller_rotate(
    tension_controller_t *controller,
    int32_t detents,
    bool accelerated)
{
    if (controller == NULL) {
        return TENSION_CTRL_ERROR_ARGUMENT;
    }
    if (controller->state == TENSION_STATE_ARMED || controller->state == TENSION_STATE_FAULT) {
        return TENSION_CTRL_ERROR_STATE;
    }
    if (detents == 0) {
        return TENSION_CTRL_OK;
    }

    adjust_selected_value(controller, detents, accelerated);
    if (controller->state == TENSION_STATE_STOPPED) {
        controller->state = TENSION_STATE_EDITING;
    }
    notify_changed(controller);
    return TENSION_CTRL_OK;
}

tension_ctrl_result_t tension_controller_short_press(tension_controller_t *controller)
{
    uint8_t count;

    if (controller == NULL) {
        return TENSION_CTRL_ERROR_ARGUMENT;
    }
    if (controller->state == TENSION_STATE_ARMED || controller->state == TENSION_STATE_FAULT) {
        return TENSION_CTRL_ERROR_STATE;
    }
    count = tension_controller_parameter_count(controller->mode);
    if (count == 0u) {
        return TENSION_CTRL_ERROR_STATE;
    }
    controller->selected_parameter = (tension_parameter_t)(
        ((uint8_t)controller->selected_parameter + 1u) % count);
    notify_changed(controller);
    return TENSION_CTRL_OK;
}

tension_ctrl_result_t tension_controller_long_press(tension_controller_t *controller)
{
    tension_ctrl_result_t result;

    if (controller == NULL) {
        return TENSION_CTRL_ERROR_ARGUMENT;
    }
    if (controller->state == TENSION_STATE_RUNNING || controller->state == TENSION_STATE_ARMED) {
        return tension_controller_stop(controller);
    }
    if (controller->state == TENSION_STATE_FAULT) {
        return tension_controller_clear_fault(controller);
    }
    if (controller->startup_status_pending || !controller->telemetry_valid) {
        return TENSION_CTRL_ERROR_STATE;
    }

    /* The startup device-state read has succeeded, so starting is now safe. */
    result = send_active_frame(controller);
    if (result != TENSION_CTRL_OK) {
        (void)send_stop_frame(controller);
        set_state(controller, TENSION_STATE_FAULT);
        return result;
    }
    controller->control_elapsed_ms = 0u;
    controller->telemetry_age_ms = 0u;
    set_state(controller, TENSION_STATE_ARMED);
    return TENSION_CTRL_OK;
}

tension_ctrl_result_t tension_controller_stop(tension_controller_t *controller)
{
    tension_ctrl_result_t result;

    if (controller == NULL) {
        return TENSION_CTRL_ERROR_ARGUMENT;
    }
    result = send_stop_frame(controller);
    if (result != TENSION_CTRL_OK) {
        set_state(controller, TENSION_STATE_FAULT);
        return result;
    }
    set_state(
        controller,
        controller->telemetry_valid ? TENSION_STATE_STOPPED : TENSION_STATE_DISCONNECTED);
    return TENSION_CTRL_OK;
}

tension_ctrl_result_t tension_controller_clear_fault(tension_controller_t *controller)
{
    tension_ctrl_result_t result;

    if (controller == NULL) {
        return TENSION_CTRL_ERROR_ARGUMENT;
    }
    if (controller->state != TENSION_STATE_FAULT) {
        return TENSION_CTRL_ERROR_STATE;
    }
    result = send_stop_frame(controller);
    if (result != TENSION_CTRL_OK) {
        return result;
    }
    controller->last_error = TENSION_CTRL_OK;
    set_state(
        controller,
        controller->telemetry_valid ? TENSION_STATE_STOPPED : TENSION_STATE_DISCONNECTED);
    return TENSION_CTRL_OK;
}
