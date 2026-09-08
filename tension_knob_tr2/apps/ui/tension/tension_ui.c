#include "../../apps.h"

#include <stdio.h>

static grf_ctrl_t *state_label;
static grf_ctrl_t *mode_prev_button;
static grf_ctrl_t *mode_label;
static grf_ctrl_t *mode_next_button;
static grf_ctrl_t *value_arc;
static grf_ctrl_t *parameter_label;
static grf_ctrl_t *value_label;
static grf_ctrl_t *unit_label;
static grf_ctrl_t *hint_label;
static grf_ctrl_t *force_label;
static grf_ctrl_t *speed_label;
static grf_ctrl_t *position_label;
static grf_ctrl_t *parameter_button;
static grf_ctrl_t *run_button;
static grf_task_t *ui_task;
static volatile u8 ui_dirty = 1u;

typedef struct {
    s32 value;
    s32 maximum;
    u8 force_scale;
    const char *name;
    const char *unit;
} tension_display_value_t;

static const char *state_text(tension_state_t state)
{
    switch (state) {
    case TENSION_STATE_DISCONNECTED: return "未连接";
    case TENSION_STATE_STOPPED: return "已连接 · 已停止";
    case TENSION_STATE_EDITING: return "已连接 · 参数已修改";
    case TENSION_STATE_ARMED: return "准备启动";
    case TENSION_STATE_RUNNING: return "运行中";
    case TENSION_STATE_FAULT: return "通信故障 · 已停止";
    default: return "状态未知";
    }
}

static grf_color_t state_color(tension_state_t state)
{
    switch (state) {
    case TENSION_STATE_RUNNING: return 0x58E39B;
    case TENSION_STATE_ARMED: return 0xFFB84D;
    case TENSION_STATE_FAULT: return 0xFF5D73;
    case TENSION_STATE_DISCONNECTED: return 0x66758F;
    default: return 0x36D7FF;
    }
}

static const char *mode_text(tension_mode_t mode)
{
    switch (mode) {
    case TENSION_MODE_BASE: return "基础阻力";
    case TENSION_MODE_SPRING: return "弹簧模式";
    case TENSION_MODE_BAND: return "阻力带";
    case TENSION_MODE_ISOKINETIC: return "等速模式";
    case TENSION_MODE_VIBRATION: return "振动模式";
    case TENSION_MODE_INERTIA: return "惯性模式";
    default: return "未知模式";
    }
}

static tension_display_value_t selected_value(const tension_controller_t *controller)
{
    tension_display_value_t selected = {0, 1, 0, "参数", ""};

    switch (controller->mode) {
    case TENSION_MODE_BASE:
        selected.force_scale = 1;
        selected.maximum = TENSION_UI_MAX_FORCE_DN;
        selected.unit = "lb";
        if (controller->selected_parameter == TENSION_PARAM_PRIMARY) {
            selected.value = controller->settings.concentric_force_dN;
            selected.name = "向心力";
        } else {
            selected.value = controller->settings.eccentric_force_dN;
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
            selected.value = controller->settings.spring_peak_force_dN;
            selected.maximum = TENSION_UI_MAX_FORCE_DN;
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
            selected.value = controller->settings.band_peak_force_dN;
            selected.maximum = TENSION_UI_MAX_FORCE_DN;
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
            selected.value = controller->settings.isokinetic_slope_dN_per_m_s;
            selected.maximum = 4000;
            selected.name = "速度斜率";
            selected.unit = "dN/(m/s)";
        }
        break;
    case TENSION_MODE_VIBRATION:
        if (controller->selected_parameter == TENSION_PARAM_PRIMARY) {
            selected.value = controller->settings.vibration_amplitude_dN;
            selected.maximum = TENSION_UI_MAX_FORCE_DN;
            selected.force_scale = 1;
            selected.name = "振动幅值";
            selected.unit = "lb";
        } else {
            selected.value = controller->settings.vibration_frequency_hz;
            selected.maximum = 100;
            selected.name = "振动频率";
            selected.unit = "Hz";
        }
        break;
    case TENSION_MODE_INERTIA:
        selected.value = controller->settings.inertia_breakout_force_dN;
        selected.maximum = TENSION_UI_MAX_FORCE_DN;
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
    if (selected.maximum > 32767) {
        selected.maximum = 32767;
    }
    return selected;
}

static void format_force(char *buffer, s32 force_dN)
{
    /* Display in pounds: 1 dN (0.1 N) = 0.02248 lb; result in tenths of lb.
       32-bit math only: max 32767 * 2248 fits s32; avoid 64-bit helpers
       (firmware links -nostdlib, no libgcc on RV32). */
    s32 absolute = force_dN < 0 ? -force_dN : force_dN;
    s32 tenths_lb = absolute * 2248 / 10000;
    sprintf(buffer, "%s%ld.%ld", force_dN < 0 ? "-" : "", (long)(tenths_lb / 10),
        (long)(tenths_lb % 10));
}

static void refresh_ui(const tension_controller_t *controller)
{
    tension_display_value_t selected;
    char value[32];
    char force[48];
    char speed[32];
    char position[32];

    if (controller == NULL || value_label == NULL) {
        return;
    }

    selected = selected_value(controller);
    grf_label_set_txt(state_label, state_text(controller->state));
    grf_label_set_txt_color(state_label, state_color(controller->state));
    grf_label_set_txt(mode_label, mode_text(controller->mode));
    grf_label_set_txt(parameter_label, selected.name);
    grf_label_set_txt(unit_label, selected.unit);
    grf_arc_set_value_range(value_arc, 0, (s16)selected.maximum);
    grf_arc_set_value(value_arc, (s16)selected.value);

    if (selected.force_scale) {
        format_force(value, selected.value);
    } else {
        sprintf(value, "%ld", (long)selected.value);
    }
    grf_label_set_txt(value_label, value);

    if (controller->state == TENSION_STATE_RUNNING || controller->state == TENSION_STATE_ARMED) {
        grf_btn_set_txt(run_button, "停止");
        grf_label_set_txt(hint_label, "旋转实时调节 · 长按停止");
    } else if (controller->state == TENSION_STATE_FAULT) {
        grf_btn_set_txt(run_button, "清除故障");
        grf_label_set_txt(hint_label, "检查连线后点击清除");
    } else {
        grf_btn_set_txt(run_button, "启动");
        grf_label_set_txt(hint_label, "按住旋转切模式 · 长按启动");
    }

    if (!controller->telemetry_valid) {
        grf_label_set_txt(force_label, "拉力 --.- lb");
        grf_label_set_txt(speed_label, "速度 --");
        grf_label_set_txt(position_label, "位置 --");
        return;
    }

    format_force(value, controller->telemetry.total_force_dN);
    sprintf(force, "拉力 %s lb", value);
    sprintf(speed, "速度 %d", (int)controller->telemetry.speed_mm_s);
    sprintf(position, "位置 %ld", (long)controller->telemetry.position_mm);
    grf_label_set_txt(force_label, force);
    grf_label_set_txt(speed_label, speed);
    grf_label_set_txt(position_label, position);
}

static void tension_changed(const tension_controller_t *controller)
{
    (void)controller;
    ui_dirty = 1u;
}

static void tension_ui_task(grf_task_t *task)
{
    (void)task;
    if (ui_dirty) {
        ui_dirty = 0u;
        refresh_ui(grf_tension_controller_get());
    }
}

static void tension_mode_prev_event(grf_ctrl_t *ctrl, grf_event_e event)
{
    (void)ctrl;
    if (event == GRF_EVENT_CLICKED) {
        grf_tension_knob_cycle_mode(-1);
    }
}

static void tension_mode_next_event(grf_ctrl_t *ctrl, grf_event_e event)
{
    (void)ctrl;
    if (event == GRF_EVENT_CLICKED) {
        grf_tension_knob_cycle_mode(1);
    }
}

static void tension_parameter_event(grf_ctrl_t *ctrl, grf_event_e event)
{
    (void)ctrl;
    if (event == GRF_EVENT_CLICKED) {
        grf_tension_knob_short_press();
    }
}

static void tension_run_event(grf_ctrl_t *ctrl, grf_event_e event)
{
    const tension_controller_t *controller;

    (void)ctrl;
    if (event != GRF_EVENT_CLICKED) {
        return;
    }
    controller = grf_tension_controller_get();
    if (controller->state == TENSION_STATE_FAULT) {
        grf_tension_clear_fault();
    } else {
        grf_tension_knob_long_press();
    }
}

#include "../../../libs/appscc/tension_cc.h"

void tension_init(void)
{
    grf_view_create(GRF_TENSION_ID, tension_ctrls_fun,
        sizeof(tension_ctrls_fun) / sizeof(grf_ctrl_fun_t));

    state_label = GCL(GRF_TENSION_ID, TENSION_STATE_LABEL_ID);
    mode_prev_button = GCL(GRF_TENSION_ID, TENSION_MODE_PREV_BUTTON_ID);
    mode_label = GCL(GRF_TENSION_ID, TENSION_MODE_LABEL_ID);
    mode_next_button = GCL(GRF_TENSION_ID, TENSION_MODE_NEXT_BUTTON_ID);
    value_arc = GCL(GRF_TENSION_ID, TENSION_VALUE_ARC_ID);
    parameter_label = GCL(GRF_TENSION_ID, TENSION_PARAMETER_LABEL_ID);
    value_label = GCL(GRF_TENSION_ID, TENSION_VALUE_LABEL_ID);
    unit_label = GCL(GRF_TENSION_ID, TENSION_UNIT_LABEL_ID);
    hint_label = GCL(GRF_TENSION_ID, TENSION_HINT_LABEL_ID);
    force_label = GCL(GRF_TENSION_ID, TENSION_FORCE_LABEL_ID);
    speed_label = GCL(GRF_TENSION_ID, TENSION_SPEED_LABEL_ID);
    position_label = GCL(GRF_TENSION_ID, TENSION_POSITION_LABEL_ID);
    parameter_button = GCL(GRF_TENSION_ID, TENSION_PARAMETER_BUTTON_ID);
    run_button = GCL(GRF_TENSION_ID, TENSION_RUN_BUTTON_ID);
}

void tension_entry(void)
{
    grf_tension_set_ui_changed_callback(tension_changed);
    ui_dirty = 1u;
    if (ui_task == NULL) {
        ui_task = grf_task_create(tension_ui_task, 50u, NULL);
    }
}

void tension_exit(void)
{
    if (ui_task != NULL) {
        grf_task_del(ui_task);
        ui_task = NULL;
    }
}
