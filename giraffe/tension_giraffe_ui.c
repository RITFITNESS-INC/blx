#include "tension_giraffe_ui.h"

#include "tension_giraffe_adapter.h"
#include "grf_apis.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    grf_ctrl_t *state_label;
    grf_ctrl_t *mode_label;
    grf_ctrl_t *parameter_label;
    grf_ctrl_t *value_label;
    grf_ctrl_t *unit_label;
    grf_ctrl_t *arc;
    grf_ctrl_t *force_label;
    grf_ctrl_t *speed_label;
    grf_ctrl_t *position_label;
} grf_tension_ui_controls_t;

typedef struct {
    int32_t value;
    int32_t maximum;
    int force_scale;
    const char *name;
    const char *unit;
} selected_value_t;

static grf_tension_ui_controls_t controls;

static grf_ctrl_t *find_control(uint32_t view_id, uint32_t control_id)
{
    if (control_id == GRF_TENSION_CTRL_UNUSED) {
        return NULL;
    }
    return grf_ctrl_get_form_id(view_id, control_id);
}

static const char *state_text(tension_state_t state)
{
    switch (state) {
    case TENSION_STATE_DISCONNECTED:
        return "未连接";
    case TENSION_STATE_STOPPED:
        return "已停止";
    case TENSION_STATE_EDITING:
        return "调节中";
    case TENSION_STATE_ARMED:
        return "准备启动";
    case TENSION_STATE_RUNNING:
        return "运行中";
    case TENSION_STATE_FAULT:
        return "故障";
    default:
        return "未知";
    }
}

static const char *mode_text(tension_mode_t mode)
{
    switch (mode) {
    case TENSION_MODE_BASE:
        return "基础阻力";
    case TENSION_MODE_SPRING:
        return "弹簧";
    case TENSION_MODE_BAND:
        return "阻力带";
    case TENSION_MODE_ISOKINETIC:
        return "等速";
    case TENSION_MODE_VIBRATION:
        return "震动";
    case TENSION_MODE_INERTIA:
        return "惯性";
    default:
        return "未知模式";
    }
}

static selected_value_t selected_value(const tension_controller_t *controller)
{
    selected_value_t selected = {0, 1, 0, "参数", ""};

    switch (controller->mode) {
    case TENSION_MODE_BASE:
        selected.force_scale = 1;
        selected.maximum = TENSION_UI_MAX_FORCE_DLB;
        selected.unit = "lb";
        if (controller->selected_parameter == TENSION_PARAM_PRIMARY) {
            selected.value = controller->settings.concentric_force_dlb;
            selected.name = "向心力";
        } else {
            selected.value = controller->settings.eccentric_force_dlb;
            selected.name = "离心力";
        }
        break;
    case TENSION_MODE_SPRING:
        if (controller->selected_parameter == TENSION_PARAM_PRIMARY) {
            selected.value = controller->settings.spring_travel_mm;
            selected.maximum = 1000;
            selected.name = "行程";
            selected.unit = "mm";
        } else {
            selected.value = controller->settings.spring_peak_force_dlb;
            selected.maximum = TENSION_UI_MAX_FORCE_DLB;
            selected.force_scale = 1;
            selected.name = "峰值力";
            selected.unit = "lb";
        }
        break;
    case TENSION_MODE_BAND:
        if (controller->selected_parameter == TENSION_PARAM_PRIMARY) {
            selected.value = controller->settings.band_rest_length_mm;
            selected.maximum = controller->settings.band_maximum_length_mm;
            selected.name = "静止长度";
            selected.unit = "mm";
        } else if (controller->selected_parameter == TENSION_PARAM_SECONDARY) {
            selected.value = controller->settings.band_maximum_length_mm;
            selected.maximum = 1000;
            selected.name = "最大长度";
            selected.unit = "mm";
        } else if (controller->selected_parameter == TENSION_PARAM_TERTIARY) {
            selected.value = controller->settings.band_peak_force_dlb;
            selected.maximum = TENSION_UI_MAX_FORCE_DLB;
            selected.force_scale = 1;
            selected.name = "峰值力";
            selected.unit = "lb";
        } else {
            selected.value = controller->settings.band_curve_factor;
            selected.maximum = 4;
            selected.name = "曲线系数";
        }
        break;
    case TENSION_MODE_ISOKINETIC:
        if (controller->selected_parameter == TENSION_PARAM_PRIMARY) {
            selected.value = controller->settings.isokinetic_speed_mm_s;
            selected.maximum = 2000;
            selected.name = "目标速度";
            selected.unit = "mm/s";
        } else {
            selected.value = controller->settings.isokinetic_slope_dlb_per_m_s;
            selected.maximum = TENSION_UI_MAX_SLOPE_DLB_PER_M_S;
            selected.force_scale = 1;
            selected.name = "斜率";
            selected.unit = "lb/(m/s)";
        }
        break;
    case TENSION_MODE_VIBRATION:
        if (controller->selected_parameter == TENSION_PARAM_PRIMARY) {
            selected.value = controller->settings.vibration_amplitude_dlb;
            selected.maximum = TENSION_UI_MAX_FORCE_DLB;
            selected.force_scale = 1;
            selected.name = "幅值";
            selected.unit = "lb";
        } else {
            selected.value = controller->settings.vibration_frequency_hz;
            selected.maximum = 100;
            selected.name = "频率";
            selected.unit = "Hz";
        }
        break;
    case TENSION_MODE_INERTIA:
        selected.value = controller->settings.inertia_breakout_force_dlb;
        selected.maximum = TENSION_UI_MAX_FORCE_DLB;
        selected.force_scale = 1;
        selected.name = "突破力";
        selected.unit = "lb";
        break;
    default:
        break;
    }
    if (selected.maximum < 1) {
        selected.maximum = 1;
    }
    return selected;
}

static void format_tenths(char *output, size_t capacity, int32_t value_tenths)
{
    int32_t absolute = value_tenths < 0 ? -value_tenths : value_tenths;
    (void)snprintf(
        output,
        capacity,
        "%s%ld.%ld",
        value_tenths < 0 ? "-" : "",
        (long)(absolute / 10),
        (long)(absolute % 10));
}

static void refresh_ui(const tension_controller_t *controller)
{
    selected_value_t selected;
    char value_text[24];
    char telemetry_text[32];

    if (controller == NULL) {
        return;
    }
    selected = selected_value(controller);

    if (controls.state_label != NULL) {
        grf_label_set_txt(controls.state_label, state_text(controller->state));
    }
    if (controls.mode_label != NULL) {
        grf_label_set_txt(controls.mode_label, mode_text(controller->mode));
    }
    if (controls.parameter_label != NULL) {
        grf_label_set_txt(controls.parameter_label, selected.name);
    }
    if (selected.force_scale) {
        format_tenths(value_text, sizeof(value_text), selected.value);
    } else {
        (void)snprintf(value_text, sizeof(value_text), "%ld", (long)selected.value);
    }
    if (controls.value_label != NULL) {
        grf_label_set_txt(controls.value_label, value_text);
    }
    if (controls.unit_label != NULL) {
        grf_label_set_txt(controls.unit_label, selected.unit);
    }
    if (controls.arc != NULL) {
        grf_arc_set_value_range(controls.arc, 0, (int16_t)selected.maximum);
        grf_arc_set_value(controls.arc, (int16_t)selected.value);
    }

    if (!controller->telemetry_valid) {
        if (controls.force_label != NULL) {
            grf_label_set_txt(controls.force_label, "--.- lb");
        }
        if (controls.speed_label != NULL) {
            grf_label_set_txt(controls.speed_label, "-- mm/s");
        }
        if (controls.position_label != NULL) {
            grf_label_set_txt(controls.position_label, "-- mm");
        }
        return;
    }

    if (controls.force_label != NULL) {
        format_tenths(
            telemetry_text,
            sizeof(telemetry_text),
            tension_force_dN_to_dlb(controller->telemetry.total_force_dN));
        (void)strncat(telemetry_text, " lb", sizeof(telemetry_text) - strlen(telemetry_text) - 1u);
        grf_label_set_txt(controls.force_label, telemetry_text);
    }
    if (controls.speed_label != NULL) {
        (void)snprintf(
            telemetry_text,
            sizeof(telemetry_text),
            "%d mm/s",
            (int)controller->telemetry.speed_mm_s);
        grf_label_set_txt(controls.speed_label, telemetry_text);
    }
    if (controls.position_label != NULL) {
        (void)snprintf(
            telemetry_text,
            sizeof(telemetry_text),
            "%ld mm",
            (long)controller->telemetry.position_mm);
        grf_label_set_txt(controls.position_label, telemetry_text);
    }
}

int grf_tension_ui_bind(const grf_tension_ui_ids_t *ids)
{
    if (ids == NULL) {
        return -1;
    }
    memset(&controls, 0, sizeof(controls));
    controls.state_label = find_control(ids->view_id, ids->state_label_id);
    controls.mode_label = find_control(ids->view_id, ids->mode_label_id);
    controls.parameter_label = find_control(ids->view_id, ids->parameter_label_id);
    controls.value_label = find_control(ids->view_id, ids->value_label_id);
    controls.unit_label = find_control(ids->view_id, ids->unit_label_id);
    controls.arc = find_control(ids->view_id, ids->arc_id);
    controls.force_label = find_control(ids->view_id, ids->force_label_id);
    controls.speed_label = find_control(ids->view_id, ids->speed_label_id);
    controls.position_label = find_control(ids->view_id, ids->position_label_id);
    grf_tension_set_ui_changed_callback(refresh_ui);
    return controls.value_label != NULL && controls.arc != NULL ? 0 : -1;
}

void grf_tension_ui_refresh_now(void)
{
    refresh_ui(grf_tension_controller_get());
}
