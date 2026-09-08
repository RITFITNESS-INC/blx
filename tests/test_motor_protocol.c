#include "motor_protocol.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_base_frame(void)
{
    uint8_t frame[16] = {0};
    const uint8_t expected[] = {0x02, 0x00, 0xE8, 0x03, 0xE8, 0x03, 0xFF, 0xFF, 0x03};
    size_t written = 0u;

    assert(motor_protocol_build_base(1000, 1000, true, frame, sizeof(frame), &written) == MOTOR_OK);
    assert(written == sizeof(expected));
    assert(memcmp(frame, expected, sizeof(expected)) == 0);
    assert(motor_protocol_build_base(-1, 1000, false, frame, sizeof(frame), &written) == MOTOR_ERROR_RANGE);
    assert(motor_protocol_build_base(8001, 1000, false, frame, sizeof(frame), &written) == MOTOR_ERROR_RANGE);
}

static void test_telemetry_request(void)
{
    uint8_t frame[4] = {0};
    const uint8_t expected[] = {0x02, 0x0E, 0x0E, 0x03};
    size_t written = 0u;

    assert(motor_protocol_build_telemetry_request(frame, sizeof(frame), &written) == MOTOR_OK);
    assert(written == sizeof(expected));
    assert(memcmp(frame, expected, sizeof(expected)) == 0);
}

static void test_band_validation(void)
{
    uint8_t frame[16] = {0};
    size_t written = 0u;

    assert(motor_protocol_build_band(100, 500, 1200, 3, frame, sizeof(frame), &written) == MOTOR_OK);
    assert(written == MOTOR_FRAME_BAND_SIZE);
    assert(motor_protocol_build_band(500, 100, 1200, 3, frame, sizeof(frame), &written) == MOTOR_ERROR_RANGE);
    assert(motor_protocol_build_band(100, 500, 1200, 5, frame, sizeof(frame), &written) == MOTOR_ERROR_RANGE);
}

static void test_telemetry_parse(void)
{
    uint8_t frame[MOTOR_FRAME_TELEMETRY_RESPONSE_SIZE] = {
        0x02, 0x0E, 0x03,
        0xD2, 0x04, 0x00, 0x00,
        0x88, 0xFF,
        0xC8, 0x01,
        0x5E, 0x01,
        0x9A, 0x01,
        0x2C, 0x01,
        0xE0, 0x01,
        0x00, 0x03
    };
    motor_telemetry_t telemetry = {0};

    frame[19] = motor_protocol_checksum(frame + 1u, 18u);
    assert(motor_protocol_parse_telemetry(frame, sizeof(frame), &telemetry) == MOTOR_OK);
    assert(telemetry.system_state == 3u);
    assert(telemetry.position_mm == 1234);
    assert(telemetry.speed_mm_s == -120);
    assert(telemetry.total_force_dN == 456);
    assert(telemetry.fet_temperature_raw == 350);
    assert(telemetry.motor_temperature_raw == 410);
    assert(telemetry.brake_temperature_raw == 300);
    assert(telemetry.bus_voltage_raw == 480);
    assert(telemetry.feedback_checksum_matches_full);

    frame[19] = motor_protocol_checksum(frame + 1u, 5u);
    assert(motor_protocol_parse_telemetry(frame, sizeof(frame), &telemetry) == MOTOR_OK);
    assert(telemetry.feedback_checksum_matches_legacy);

    frame[19] ^= 0x5Au;
    assert(motor_protocol_parse_telemetry(frame, sizeof(frame), &telemetry) == MOTOR_OK);
    assert(!telemetry.feedback_checksum_matches_full);
    assert(!telemetry.feedback_checksum_matches_legacy);

    frame[20] = 0x00u;
    assert(motor_protocol_parse_telemetry(frame, sizeof(frame), &telemetry) == MOTOR_ERROR_FRAME);
}

int main(void)
{
    test_base_frame();
    test_telemetry_request();
    test_band_validation();
    test_telemetry_parse();
    puts("motor_protocol tests passed");
    return 0;
}
