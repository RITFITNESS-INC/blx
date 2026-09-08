#ifndef TENSION_GIRAFFE_ADAPTER_H
#define TENSION_GIRAFFE_ADAPTER_H

#include "tension_controller.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*grf_tension_ui_changed_fn)(const tension_controller_t *controller);

/* Call after grf_prj_create(), so generated UI controls already exist. */
int grf_tension_app_init(void);
void grf_tension_set_ui_changed_callback(grf_tension_ui_changed_fn callback);
const tension_controller_t *grf_tension_controller_get(void);

void grf_tension_knob_rotate(int detents, int accelerated);
void grf_tension_knob_short_press(void);
void grf_tension_knob_long_press(void);
void grf_tension_knob_cycle_mode(int direction);
void grf_tension_touch_select_mode(tension_mode_t mode);
void grf_tension_touch_stop(void);
void grf_tension_clear_fault(void);

#ifdef __cplusplus
}
#endif

#endif
