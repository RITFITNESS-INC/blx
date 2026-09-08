#ifndef TENSION_CONTROLLER_H
#define TENSION_CONTROLLER_H

#include "motor_protocol.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TENSION_CONTROL_PERIOD_MS 40u
#define TENSION_TELEMETRY_PERIOD_MS 50u
#define TENSION_LINK_TIMEOUT_MS 400u
#define TENSION_CONNECT_HANDSHAKE_PERIOD_MS 200u
#define TENSION_UI_MAX_FORCE_DN 2000
#define TENSION_UI_MAX_TRAVEL_MM 1000
#define TENSION_UI_MAX_SPEED_MM_S 2000
#define TENSION_UI_MAX_FREQUENCY_HZ 100
#define TENSION_RX_BUFFER_SIZE 64u

typedef enum {
    TENSION_STATE_DISCONNECTED = 0,
    TENSION_STATE_STOPPED,
    TENSION_STATE_EDITING,
    TENSION_STATE_ARMED,
    TENSION_STATE_RUNNING,
    TENSION_STATE_FAULT
} tension_state_t;

typedef enum {
    TENSION_MODE_BASE = 0,
    TENSION_MODE_SPRING,
    TENSION_MODE_BAND,
    TENSION_MODE_ISOKINETIC,
    TENSION_MODE_VIBRATION,
    TENSION_MODE_INERTIA,
    TENSION_MODE_COUNT
} tension_mode_t;

typedef enum {
    TENSION_PARAM_PRIMARY = 0,
    TENSION_PARAM_SECONDARY,
    TENSION_PARAM_TERTIARY,
    TENSION_PARAM_QUATERNARY
} tension_parameter_t;

typedef enum {
    TENSION_CTRL_OK = 0,
    TENSION_CTRL_ERROR_ARGUMENT = -1,
    TENSION_CTRL_ERROR_STATE = -2,
    TENSION_CTRL_ERROR_SEND = -3,
    TENSION_CTRL_ERROR_PROTOCOL = -4
} tension_ctrl_result_t;

typedef struct {
    int32_t concentric_force_dN;
    int32_t eccentric_force_dN;
    int32_t spring_travel_mm;
    int32_t spring_peak_force_dN;
    int32_t band_rest_length_mm;
    int32_t band_maximum_length_mm;
    int32_t band_peak_force_dN;
    uint8_t band_curve_factor;
    int32_t isokinetic_speed_mm_s;
    int32_t isokinetic_slope_dN_per_m_s;
    int32_t vibration_amplitude_dN;
    int32_t vibration_frequency_hz;
    int32_t inertia_breakout_force_dN;
} tension_settings_t;

struct tension_controller;
typedef struct tension_controller tension_controller_t;

typedef int (*tension_send_fn)(const uint8_t *data, size_t length, void *user_data);
typedef void (*tension_changed_fn)(const tension_controller_t *controller, void *user_data);

typedef struct {
    tension_send_fn send;
    tension_changed_fn changed;
    void *user_data;
} tension_controller_io_t;

struct tension_controller {
    tension_controller_io_t io;
    tension_state_t state;
    tension_mode_t mode;
    tension_parameter_t selected_parameter;
    tension_settings_t settings;
    motor_telemetry_t telemetry;
    bool telemetry_valid;
    uint32_t telemetry_age_ms;
    uint32_t telemetry_poll_elapsed_ms;
    uint32_t control_elapsed_ms;
    tension_ctrl_result_t last_error;
    motor_result_t last_protocol_error;
    uint8_t rx_buffer[TENSION_RX_BUFFER_SIZE];
    size_t rx_length;
};

void tension_controller_init(
    tension_controller_t *controller,
    const tension_controller_io_t *io);

void tension_controller_tick(tension_controller_t *controller, uint32_t elapsed_ms);
void tension_controller_on_uart_data(
    tension_controller_t *controller,
    const uint8_t *data,
    size_t length);

tension_ctrl_result_t tension_controller_select_mode(
    tension_controller_t *controller,
    tension_mode_t mode);

tension_ctrl_result_t tension_controller_rotate(
    tension_controller_t *controller,
    int32_t detents,
    bool accelerated);

tension_ctrl_result_t tension_controller_short_press(tension_controller_t *controller);
tension_ctrl_result_t tension_controller_long_press(tension_controller_t *controller);
tension_ctrl_result_t tension_controller_stop(tension_controller_t *controller);
tension_ctrl_result_t tension_controller_clear_fault(tension_controller_t *controller);

uint8_t tension_controller_parameter_count(tension_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif
