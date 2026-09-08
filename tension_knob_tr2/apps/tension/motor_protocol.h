#ifndef MOTOR_PROTOCOL_H
#define MOTOR_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MOTOR_FRAME_START 0x02u
#define MOTOR_FRAME_END 0x03u

#define MOTOR_CMD_BASE_RESISTANCE 0x00u
#define MOTOR_CMD_SPRING 0x01u
#define MOTOR_CMD_VIBRATION 0x04u
#define MOTOR_CMD_TELEMETRY 0x0Eu
#define MOTOR_CMD_ISOKINETIC 0x11u
#define MOTOR_CMD_BAND 0x14u
#define MOTOR_CMD_INERTIA 0x15u

#define MOTOR_FRAME_BASE_SIZE 9u
#define MOTOR_FRAME_SPRING_SIZE 8u
#define MOTOR_FRAME_BAND_SIZE 11u
#define MOTOR_FRAME_ISOKINETIC_SIZE 8u
#define MOTOR_FRAME_VIBRATION_SIZE 8u
#define MOTOR_FRAME_INERTIA_SIZE 6u
#define MOTOR_FRAME_TELEMETRY_REQUEST_SIZE 4u
#define MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE 21u

typedef enum {
    MOTOR_OK = 0,
    MOTOR_ERROR_ARGUMENT = -1,
    MOTOR_ERROR_CAPACITY = -2,
    MOTOR_ERROR_RANGE = -3,
    MOTOR_ERROR_LENGTH = -4,
    MOTOR_ERROR_FRAME = -5,
    MOTOR_ERROR_CHECKSUM = -6,
    MOTOR_ERROR_COMMAND = -7
} motor_result_t;

/* Temperature and bus-voltage scaling are not defined in the supplied protocol. */
typedef struct {
    uint8_t system_state;
    int32_t position_mm;
    int16_t speed_mm_s;
    int16_t total_force_dN;
    int16_t fet_temperature_raw;
    int16_t motor_temperature_raw;
    int16_t brake_temperature_raw;
    int16_t bus_voltage_raw;
} motor_telemetry_t;

uint8_t motor_protocol_checksum(const uint8_t *data, size_t length);
size_t motor_protocol_expected_frame_size(uint8_t command_id, bool inbound);

motor_result_t motor_protocol_build_base(
    int32_t concentric_force_dN,
    int32_t eccentric_force_dN,
    bool ssr_enabled,
    uint8_t *out,
    size_t capacity,
    size_t *written);

motor_result_t motor_protocol_build_spring(
    int32_t travel_mm,
    int32_t peak_force_dN,
    uint8_t *out,
    size_t capacity,
    size_t *written);

motor_result_t motor_protocol_build_band(
    int32_t rest_length_mm,
    int32_t maximum_length_mm,
    int32_t peak_force_dN,
    uint8_t curve_factor,
    uint8_t *out,
    size_t capacity,
    size_t *written);

motor_result_t motor_protocol_build_isokinetic(
    int32_t target_speed_mm_s,
    int32_t slope_dN_per_m_s,
    uint8_t *out,
    size_t capacity,
    size_t *written);

motor_result_t motor_protocol_build_vibration(
    int32_t amplitude_dN,
    int32_t frequency_hz,
    uint8_t *out,
    size_t capacity,
    size_t *written);

motor_result_t motor_protocol_build_inertia(
    int32_t breakout_force_dN,
    uint8_t *out,
    size_t capacity,
    size_t *written);

motor_result_t motor_protocol_build_telemetry_request(
    uint8_t *out,
    size_t capacity,
    size_t *written);

motor_result_t motor_protocol_parse_telemetry(
    const uint8_t *frame,
    size_t length,
    motor_telemetry_t *telemetry);

#ifdef __cplusplus
}
#endif

#endif
