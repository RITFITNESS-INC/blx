#ifndef TENSION_GIRAFFE_UI_H
#define TENSION_GIRAFFE_UI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GRF_TENSION_CTRL_UNUSED UINT32_MAX

typedef struct {
    uint32_t view_id;
    uint32_t state_label_id;
    uint32_t mode_label_id;
    uint32_t parameter_label_id;
    uint32_t value_label_id;
    uint32_t unit_label_id;
    uint32_t arc_id;
    uint32_t force_label_id;
    uint32_t speed_label_id;
    uint32_t position_label_id;
} grf_tension_ui_ids_t;

/* Call after grf_tension_app_init() and after the page has been created. */
int grf_tension_ui_bind(const grf_tension_ui_ids_t *ids);
void grf_tension_ui_refresh_now(void);

#ifdef __cplusplus
}
#endif

#endif
