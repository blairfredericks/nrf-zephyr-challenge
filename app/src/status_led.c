
#include <zephyr.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(status_led, LOG_LEVEL_DBG);

#include <device.h>
#include <drivers/gpio.h>
#include <devicetree.h>

#include "app_events.h"

/* Use DT alias 'led-status' */
#ifdef DT_ALIAS(led_status)
#define LED_NODE DT_ALIAS(led_status)
#else
#define LED_NODE DT_NODELABEL(led0)
#endif

static const struct device *led_dev;
static gpio_pin_t led_pin = 13;
static bool led_active_low = false;

static int current_state = APP_STATE_IDLE;
static struct k_work_delayable led_worker;

static void led_update_once(struct k_work *work)
{
    ARG_UNUSED(work);
    /* Implement simple blink patterns depending on state */
    switch (current_state) {
    case APP_STATE_IDLE:
        gpio_pin_toggle(led_dev, led_pin);
        k_work_reschedule(&led_worker, K_MSEC(1000));
        break;
    case APP_STATE_SAMPLING:
        /* Two quick blinks */
        gpio_pin_set(led_dev, led_pin, !led_active_low);
        k_msleep(80);
        gpio_pin_set(led_dev, led_pin, led_active_low);
        k_msleep(80);
        gpio_pin_set(led_dev, led_pin, !led_active_low);
        k_msleep(80);
        gpio_pin_set(led_dev, led_pin, led_active_low);
        k_work_reschedule(&led_worker, K_MSEC(500));
        break;
    case APP_STATE_ERROR:
        gpio_pin_toggle(led_dev, led_pin);
        k_work_reschedule(&led_worker, K_MSEC(200));
        break;
    default:
        k_work_reschedule(&led_worker, K_MSEC(1000));
    }
}

int status_led_init(bool active_low)
{
    led_active_low = active_low;

    /* Attempt to bind to a gpio device from DT alias; fallback to gpio0 */
#if DT_NODE_HAS_STATUS(LED_NODE, okay)
    const char *label = DT_LABEL(LED_NODE);
    led_dev = device_get_binding(label);
    if (!led_dev) {
        led_dev = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
    }
#else
    led_dev = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
#endif
    if (!led_dev) {
        LOG_ERR("No GPIO controller found for LED");
        return -ENODEV;
    }

    /* Use a default pin (board overlay should set the proper pin via DT in practice) */
    gpio_pin_configure(led_dev, led_pin, GPIO_OUTPUT_ACTIVE | (led_active_low ? GPIO_ACTIVE_LOW : GPIO_ACTIVE_HIGH));
    k_work_init_delayable(&led_worker, led_update_once);
    /* start blinking */
    k_work_reschedule(&led_worker, K_MSEC(1000));
    return 0;
}

void status_led_set_state(int state)
{
    current_state = state;
    /* trigger immediate update */
    k_work_reschedule(&led_worker, K_NO_WAIT);
}
