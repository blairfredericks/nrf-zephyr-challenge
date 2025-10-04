
#include <zephyr.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#include "app_config.h"
#include "app_events.h"

#include <settings/settings.h>

/* Submodules */
extern int adc_sampler_init(uint32_t interval_ms);
extern void adc_sampler_start(void);
extern void adc_sampler_stop(void);
extern int status_led_init(bool active_low);
extern void status_led_set_state(int state);
extern int buttons_init(void);
extern int ble_init(void);

static void settings_set_cb(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg);

static struct settings_handler app_sh = {
    .name = "app",
    .h_set = settings_set_cb,
};

static void settings_set_cb(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    /* implemented in adc_sampler.c via settings registration wrapper */
    /* Intentionally left blank here; the real handler is in adc_sampler */
}

void main(void)
{
    LOG_INF("FW Challenge booting");

    /* Settings subsystem (NVS backend) */
    settings_subsys_init();
    settings_load();
    settings_register(&app_sh);

    /* initialize modules */
    status_led_init(IS_ENABLED(CONFIG_APP_LED_ACTIVE_LOW));
    status_led_set_state(APP_STATE_IDLE);

#if CONFIG_APP_ENABLE_BLE
    if (ble_init() == 0) {
        LOG_INF("BLE initialized");
    } else {
        LOG_ERR("BLE failed to init");
    }
#else
    LOG_INF("BLE disabled by Kconfig/overlay");
#endif

    if (adc_sampler_init(app_sample_interval_ms()) == 0) {
        adc_sampler_start();
    } else {
        LOG_ERR("ADC sampler init failed");
        status_led_set_state(APP_STATE_ERROR);
    }

    buttons_init();

    LOG_INF("App started; sample interval: %u ms", app_sample_interval_ms());
}
