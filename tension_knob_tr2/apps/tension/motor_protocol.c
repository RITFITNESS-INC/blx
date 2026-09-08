#include "motor_protocol.h"

#include <limits.h>

static void put_u16_le(uint8_t *dst, uint16_t value)
{
    dst[0] = (uint8_t)(value & 0xFFu);
    dst[1] = (uint8_t)(value >> 8);
}

static int16_t read_i16_le(const uint8_t *src)
{
    uint16_t value = (uint16_t)src[0] | ((uint16_t)src[1] << 8);
    if (value <= INT16_MAX) {
        return (int16_t)value;
    }
    return (int16_t)(-((int32_t)UINT16_MAX + 1 - value));
}

static int32_t read_i32_le(const uint8_t *src)
{
    uint32_t value = (uint32_t)src[0]
        | ((uint32_t)src[1] << 8)
        | ((uint32_t)src[2] << 16)
        | ((uint32_t)src[3] << 24);
    if (value <= INT32_MAX) {
        return (int32_t)value;
    }
    return -((int64_t)UINT32_MAX + 1 - value);
}

uint8_t motor_protocol_checksum(const uint8_t *data, size_t length)
{
    uint8_t checksum = 0u;
    size_t i;

    if (data == NULL) {
        return 0u;
    }
    for (i = 0u; i < length; ++i) {
        checksum ^= data[i];
    }
    return checksum;
}

size_t motor_protocol_expected_frame_size(uint8_t command_id, bool inbound)
{
    if (inbound) {
        return command_id == MOTOR_CMD_TELEMETRY
            ? MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE
            : 0u;
    }

    switch (command_id) {
    case MOTOR_CMD_BASE_RESISTANCE:
        return MOTOR_FRAME_BASE_SIZE;
    case MOTOR_CMD_SPRING:
        return MOTOR_FRAME_SPRING_SIZE;
    case MOTOR_CMD_BAND:
        return MOTOR_FRAME_BAND_SIZE;
    case MOTOR_CMD_ISOKINETIC:
        return MOTOR_FRAME_ISOKINETIC_SIZE;
    case MOTOR_CMD_VIBRATION:
        return MOTOR_FRAME_VIBRATION_SIZE;
    case MOTOR_CMD_INERTIA:
        return MOTOR_FRAME_INERTIA_SIZE;
    case MOTOR_CMD_TELEMETRY:
        return MOTOR_FRAME_TELEMETRY_REQUEST_SIZE;
    default:
        return 0u;
    }
}

static motor_result_t finish_frame(
    uint8_t command_id,
    const uint8_t *payload,
    size_t payload_length,
    uint8_t *out,
    size_t capacity,
    size_t *written)
{
    size_t total = payload_length + 4u;
    size_t i;

    if (out == NULL || written == NULL || (payload == NULL && payload_length != 0u)) {
        return MOTOR_ERROR_ARGUMENT;
    }
    if (capacity < total) {
        return MOTOR_ERROR_CAPACITY;
    }

    out[0] = MOTOR_FRAME_START;
    out[1] = command_id;
    for (i = 0u; i < payload_length; ++i) {
        out[i + 2u] = payload[i];
    }
    out[total - 2u] = motor_protocol_checksum(out + 1u, payload_length + 1u);
    out[total - 1u] = MOTOR_FRAME_END;
    *written = total;
    return MOTOR_OK;
}

static bool force_is_valid(int32_t force_dN)
{
    return force_dN >= 0 && force_dN <= 8000;
}

static bool positive_i16_is_valid(int32_t value)
{
    return value >= 0 && value <= INT16_MAX;
}

motor_result_t motor_protocol_build_base(
    int32_t concentric_force_dN,
    int32_t eccentric_force_dN,
    bool ssr_enabled,
    uint8_t *out,
    size_t capacity,
    size_t *written)
{
    uint8_t payload[5];
    if (!force_is_valid(concentric_force_dN) || !force_is_valid(eccentric_force_dN)) {
        return MOTOR_ERROR_RANGE;
    }
    put_u16_le(payload, (uint16_t)concentric_force_dN);
    put_u16_le(payload + 2u, (uint16_t)eccentric_force_dN);
    payload[4] = ssr_enabled ? 0xFFu : 0x00u;
    return finish_frame(MOTOR_CMD_BASE_RESISTANCE, payload, sizeof(payload), out, capacity, written);
}

motor_result_t motor_protocol_build_spring(
    int32_t travel_mm,
    int32_t peak_force_dN,
    uint8_t *out,
    size_t capacity,
    size_t *written)
{
    uint8_t payload[4];
    if (!positive_i16_is_valid(travel_mm) || !force_is_valid(peak_force_dN)) {
        return MOTOR_ERROR_RANGE;
    }
    put_u16_le(payload, (uint16_t)travel_mm);
    put_u16_le(payload + 2u, (uint16_t)peak_force_dN);
    return finish_frame(MOTOR_CMD_SPRING, payload, sizeof(payload), out, capacity, written);
}

motor_result_t motor_protocol_build_band(
    int32_t rest_length_mm,
    int32_t maximum_length_mm,
    int32_t peak_force_dN,
    uint8_t curve_factor,
    uint8_t *out,
    size_t capacity,
    size_t *written)
{
    uint8_t payload[7];
    if (!positive_i16_is_valid(rest_length_mm)
        || !positive_i16_is_valid(maximum_length_mm)
        || maximum_length_mm < rest_length_mm
        || !force_is_valid(peak_force_dN)
        || curve_factor > 4u) {
        return MOTOR_ERROR_RANGE;
    }
    put_u16_le(payload, (uint16_t)rest_length_mm);
    put_u16_le(payload + 2u, (uint16_t)maximum_length_mm);
    put_u16_le(payload + 4u, (uint16_t)peak_force_dN);
    payload[6] = curve_factor;
    return finish_frame(MOTOR_CMD_BAND, payload, sizeof(payload), out, capacity, written);
}

motor_result_t motor_protocol_build_isokinetic(
    int32_t target_speed_mm_s,
    int32_t slope_dN_per_m_s,
    uint8_t *out,
    size_t capacity,
    size_t *written)
{
    uint8_t payload[4];
    if (!positive_i16_is_valid(target_speed_mm_s)
        || slope_dN_per_m_s < 0
        || slope_dN_per_m_s > 4000) {
        return MOTOR_ERROR_RANGE;
    }
    put_u16_le(payload, (uint16_t)target_speed_mm_s);
    put_u16_le(payload + 2u, (uint16_t)slope_dN_per_m_s);
    return finish_frame(MOTOR_CMD_ISOKINETIC, payload, sizeof(payload), out, capacity, written);
}

motor_result_t motor_protocol_build_vibration(
    int32_t amplitude_dN,
    int32_t frequency_hz,
    uint8_t *out,
    size_t capacity,
    size_t *written)
{
    uint8_t payload[4];

    /* The protocol omits an amplitude limit; use the documented global force ceiling. */
    if (!force_is_valid(amplitude_dN) || !positive_i16_is_valid(frequency_hz)) {
        return MOTOR_ERROR_RANGE;
    }
    put_u16_le(payload, (uint16_t)amplitude_dN);
    put_u16_le(payload + 2u, (uint16_t)frequency_hz);
    return finish_frame(MOTOR_CMD_VIBRATION, payload, sizeof(payload), out, capacity, written);
}

motor_result_t motor_protocol_build_inertia(
    int32_t breakout_force_dN,
    uint8_t *out,
    size_t capacity,
    size_t *written)
{
    uint8_t payload[2];
    if (!force_is_valid(breakout_force_dN)) {
        return MOTOR_ERROR_RANGE;
    }
    put_u16_le(payload, (uint16_t)breakout_force_dN);
    return finish_frame(MOTOR_CMD_INERTIA, payload, sizeof(payload), out, capacity, written);
}

motor_result_t motor_protocol_build_telemetry_request(
    uint8_t *out,
    size_t capacity,
    size_t *written)
{
    return finish_frame(MOTOR_CMD_TELEMETRY, NULL, 0u, out, capacity, written);
}

motor_result_t motor_protocol_parse_telemetry(
    const uint8_t *frame,
    size_t length,
    motor_telemetry_t *telemetry)
{
    uint8_t expected_checksum;

    if (frame == NULL || telemetry == NULL) {
        return MOTOR_ERROR_ARGUMENT;
    }
    if (length != MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE) {
        return MOTOR_ERROR_LENGTH;
    }
    if (frame[0] != MOTOR_FRAME_START || frame[20] != MOTOR_FRAME_END) {
        return MOTOR_ERROR_FRAME;
    }
    if (frame[1] != MOTOR_CMD_TELEMETRY) {
        return MOTOR_ERROR_COMMAND;
    }
    /* Known firmware issue (V3-V5 protocol doc section 5): the board's
       telemetry checksum only covers CMD + the first 4 payload bytes, not
       the full payload. Per the official doc, validation must rely on the
       start/end markers and the fixed 21-byte length only. Do not verify
       the checksum here, or every real telemetry frame gets rejected. */
    (void)expected_checksum;

    telemetry->system_state = frame[2];
    telemetry->position_mm = read_i32_le(frame + 3u);
    telemetry->speed_mm_s = read_i16_le(frame + 7u);
    telemetry->total_force_dN = read_i16_le(frame + 9u);
    telemetry->fet_temperature_raw = read_i16_le(frame + 11u);
    telemetry->motor_temperature_raw = read_i16_le(frame + 13u);
    telemetry->brake_temperature_raw = read_i16_le(frame + 15u);
    telemetry->bus_voltage_raw = read_i16_le(frame + 17u);
    return MOTOR_OK;
}
