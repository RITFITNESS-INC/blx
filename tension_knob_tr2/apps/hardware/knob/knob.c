#include "../grf_hw.h"

#if GRF_HW_ENABLE

#define KNOB_A_VALUE grf_gpio_readInputPin(GRF_GPIO_GROUP_C, 4)
#define KNOB_BUTTON_VALUE grf_gpio_readInputPin(GRF_GPIO_GROUP_E, 12)

#define KNOB_ROTATE_DEBOUNCE_MS 2u
#define KNOB_FAST_ROTATE_MS 80u
#define KNOB_BUTTON_DEBOUNCE_MS 25u
#define KNOB_LONG_PRESS_MS 1200u

/* Change to -1 only if the installed encoder mechanically reports backwards. */
#define KNOB_CLOCKWISE_SIGN 1

static volatile s32 pending_value_detents;
static volatile s32 pending_mode_detents;
static volatile u8 pending_accelerated;
static volatile u8 pending_short_press;
static volatile u8 button_pressed;
static volatile u8 button_rotated;
static volatile u8 long_press_sent;
static volatile u32 button_down_ms;
static u32 last_rotate_ms;
static u32 last_button_irq_ms;
static grf_task_t *knob_task;

static void encoder_irq(grf_gpio_irq_t *irq)
{
    u32 now = grf_sys_time_ms();
    s32 direction;

    (void)irq;
    if ((u32)(now - last_rotate_ms) < KNOB_ROTATE_DEBOUNCE_MS) {
        return;
    }
    direction = KNOB_A_VALUE == 0 ? KNOB_CLOCKWISE_SIGN : -KNOB_CLOCKWISE_SIGN;
    if ((u32)(now - last_rotate_ms) < KNOB_FAST_ROTATE_MS) {
        pending_accelerated = 1u;
    }
    last_rotate_ms = now;

    if (button_pressed) {
        pending_mode_detents += direction;
        button_rotated = 1u;
    } else {
        pending_value_detents += direction;
    }
}

static void button_irq(grf_gpio_irq_t *irq)
{
    u32 now = grf_sys_time_ms();
    u8 pressed;

    (void)irq;
    if ((u32)(now - last_button_irq_ms) < KNOB_BUTTON_DEBOUNCE_MS) {
        return;
    }
    last_button_irq_ms = now;
    pressed = KNOB_BUTTON_VALUE == 0 ? 1u : 0u;

    if (pressed && !button_pressed) {
        button_pressed = 1u;
        button_rotated = 0u;
        long_press_sent = 0u;
        button_down_ms = now;
    } else if (!pressed && button_pressed) {
        button_pressed = 0u;
        if (!long_press_sent && !button_rotated) {
            pending_short_press = 1u;
        }
    }
}

static void knob_task_cb(grf_task_t *task)
{
    s32 value_detents;
    s32 mode_detents;
    u8 accelerated;
    u8 short_press;

    (void)task;
    value_detents = pending_value_detents;
    mode_detents = pending_mode_detents;
    accelerated = pending_accelerated;
    short_press = pending_short_press;
    pending_value_detents = 0;
    pending_mode_detents = 0;
    pending_accelerated = 0u;
    pending_short_press = 0u;

    if (value_detents != 0) {
        grf_tension_knob_rotate((int)value_detents, accelerated != 0u);
    }
    while (mode_detents > 0) {
        grf_tension_knob_cycle_mode(1);
        --mode_detents;
    }
    while (mode_detents < 0) {
        grf_tension_knob_cycle_mode(-1);
        ++mode_detents;
    }
    if (short_press) {
        grf_tension_knob_short_press();
    }

    if (button_pressed && !button_rotated && !long_press_sent &&
        (u32)(grf_sys_time_ms() - button_down_ms) >= KNOB_LONG_PRESS_MS) {
        long_press_sent = 1u;
        grf_tension_knob_long_press();
    }
}

void knob_gpio_irq_init(void)
{
    grf_gpio_para_t gpio = {0};

    gpio.gpio_group = GRF_GPIO_GROUP_C;
    gpio.gpio_pin = 4;
    gpio.gpio_mode = GRF_GPIO_INPUT_PULLUP;
    gpio.irq_mode = GRF_GPIO_IRQ_RISING;
    (void)grf_gpio_set_irq(&gpio, NULL);

    gpio.gpio_group = GRF_GPIO_GROUP_C;
    gpio.gpio_pin = 5;
    gpio.gpio_mode = GRF_GPIO_INPUT_PULLUP;
    gpio.irq_mode = GRF_GPIO_IRQ_RISING;
    (void)grf_gpio_set_irq(&gpio, encoder_irq);

    gpio.gpio_group = GRF_GPIO_GROUP_E;
    gpio.gpio_pin = 12;
    gpio.gpio_mode = GRF_GPIO_INPUT_PULLUP;
    gpio.irq_mode = GRF_GPIO_IRQ_RISING_FALLING;
    (void)grf_gpio_set_irq(&gpio, button_irq);

    button_pressed = KNOB_BUTTON_VALUE == 0 ? 1u : 0u;
    button_down_ms = grf_sys_time_ms();
    if (knob_task == NULL) {
        knob_task = grf_task_create(knob_task_cb, 10u, NULL);
    }
}

#endif
