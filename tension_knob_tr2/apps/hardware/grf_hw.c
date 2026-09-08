#include "grf_hw.h"

void grf_hw_init(void)
{
    (void)grf_tension_app_init();
    knob_gpio_irq_init();
}








