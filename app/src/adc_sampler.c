
#include <zephyr.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(adc_sampler, LOG_LEVEL_DBG);

#include <device.h>
#include <drivers/adc.h>
#include <pm/device.h>
#include <settings/settings.h>
#include <sys/byteorder.h>

#include "app_events.h"

/* sample_count persisted via settings: key "app/sample_count" */
static uint32_t sample_count = 0;
static const char *settings_key = "app/sample_count";

static int settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    if (!strcmp(name, "sample_count")) {
        if (len != sizeof(sample_count)) {
            return -EINVAL;
        }
        if (read_cb(cb_arg, &sample_count, len) == len) {
            LOG_INF("Restored sample_count=%u from settings", sample_count);
            return 0;
        }
        return -EIO;
    }
    return -ENOENT;
}

static struct settings_handler sh = {
    .name = "app",
    .h_set = settings_set,
};

/* ADC device (using devicetree alias 'adc0') */
#define ADC_DEVICE_NODE DT_ALIAS(adc0)

#if DT_NODE_HAS_STATUS(ADC_DEVICE_NODE, okay)
#define ADC_LABEL DT_LABEL(ADC_DEVICE_NODE)
#else
#define ADC_LABEL NULL
#endif

static const struct device *adc_dev = NULL;
static struct k_work_delayable sample_work;
static uint32_t sample_interval_ms = 1000;
static int current_state = APP_STATE_IDLE;

/* Forwarding function for BLE notification (implemented in ble.c) */
extern void ble_notify_voltage(uint32_t mv);

static void save_sample_count(void)
{
    int rc = settings_save_one(settings_key, &sample_count, sizeof(sample_count));
    if (rc) {
        LOG_ERR("settings_save_one failed: %d", rc);
    } else {
        LOG_DBG("Saved sample_count=%u", sample_count);
    }
}

static int adc_read_mv(uint32_t *mv_out)
{
    if (!adc_dev) {
        return -ENODEV;
    }

    /* Example: use ADC emulator which provides property "mv".
     * Here we try to read via a single-shot ADC channel for demo purposes.
     * This code may need platform-specific tweaks (channel setup, resolution).
     */

    const struct device *dev = adc_dev;

    /* If device power management is enabled, optionally resume */
    pm_device_action_run(dev, PM_DEVICE_ACTION_RESUME);

    /* For simulator/emulator, fetch a property as a fallback */
#ifdef CONFIG_ADC
    /* Try to use emulator property via DT (if present) */
#if DT_NODE_HAS_PROP(ADC_DEVICE_NODE, mv)
    *mv_out = DT_PROP(ADC_DEVICE_NODE, mv);
    pm_device_action_run(dev, PM_DEVICE_ACTION_SUSPEND);
    return 0;
#else
    /* Generic ADC read pathway - platform specific; return error to indicate not-implemented */
    pm_device_action_run(dev, PM_DEVICE_ACTION_SUSPEND);
    return -ENOTSUP;
#endif
#else
    return -ENODEV;
#endif
}

static void sample_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);
    uint32_t mv = 0;
    current_state = APP_STATE_SAMPLING;
    /* update LED state via status_led module */
    extern void status_led_set_state(int state);
    status_led_set_state(current_state);

    int rc = adc_read_mv(&mv);
    if (rc == 0) {
        sample_count++;
        LOG_INF("ADC sample: %u mV (count=%u)", mv, sample_count);
        save_sample_count();
        /* notify over BLE if connected + enabled */
        ble_notify_voltage(mv);
        current_state = APP_STATE_IDLE;
        status_led_set_state(current_state);
    } else {
        LOG_ERR("ADC read failed: %d", rc);
        current_state = APP_STATE_ERROR;
        status_led_set_state(current_state);
    }

    /* re-schedule */
    k_work_reschedule(&sample_work, K_MSEC(sample_interval_ms));
}

int adc_sampler_init(uint32_t interval_ms)
{
    sample_interval_ms = interval_ms;
    /* init settings handler */
    settings_register(&sh);

    if (ADC_LABEL == NULL) {
        LOG_WRN("No ADC device alias found in DT (using emulator fallback where available).");
    } else {
        adc_dev = device_get_binding(ADC_LABEL);
        if (!adc_dev) {
            LOG_WRN("ADC device '%s' not ready; continuing (emulator fallback)", ADC_LABEL);
        }
    }

    k_work_init_delayable(&sample_work, sample_work_handler);
    return 0;
}

void adc_sampler_start(void)
{
    k_work_reschedule(&sample_work, K_MSEC(sample_interval_ms));
}

void adc_sampler_stop(void)
{
    k_work_cancel_delayable(&sample_work);
}
