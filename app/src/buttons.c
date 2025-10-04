
#include <zephyr.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(buttons, LOG_LEVEL_DBG);

#include <device.h>
#include <drivers/gpio.h>
#include <devicetree.h>

/* Use alias btn-user */
#if DT_NODE_HAS_STATUS(DT_ALIAS(btn_user), okay)
#define BTN_NODE DT_ALIAS(btn_user)
#else
#define BTN_NODE DT_NODELABEL(button0)
#endif

static const struct device *btn_dev;
static struct gpio_callback btn_cb_data;
static struct k_work button_work;

extern void app_button_pressed(void);

/* deferred work to handle the button press logic (debounce + toggle) */
static void button_work_handler(struct k_work *work)
{
    app_button_pressed();
}

static void button_pressed_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);
    /* schedule deferred handling */
    k_work_submit(&button_work);
}

int buttons_init(void)
{
    k_work_init(&button_work, button_work_handler);

#if DT_NODE_HAS_STATUS(BTN_NODE, okay)
    const char *label = DT_LABEL(BTN_NODE);
    btn_dev = device_get_binding(label);
    if (!btn_dev) {
        btn_dev = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
    }
#else
    btn_dev = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
#endif
    if (!btn_dev) {
        return -ENODEV;
    }

    gpio_pin_configure(btn_dev, 11, GPIO_INPUT | GPIO_INT_EDGE_TO_ACTIVE | GPIO_PULL_UP);
    gpio_init_callback(&btn_cb_data, button_pressed_isr, BIT(11));
    gpio_add_callback(btn_dev, &btn_cb_data);
    gpio_pin_interrupt_configure(btn_dev, 11, GPIO_INT_EDGE_TO_ACTIVE);

    return 0;
}
