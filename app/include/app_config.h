
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <zephyr/device.h>
#include <devicetree.h>

/* Devicetree / Kconfig precedence.
 * Policy used in this project:
 * - If Kconfig value APP_SAMPLE_INTERVAL_MS > 0 -> Kconfig takes precedence.
 * - Else use the devicetree value (DT_PATH(app) property sample_interval_ms).
 * - Else use compile-time hard-coded fallback.
 */

#ifdef CONFIG_APP_SAMPLE_INTERVAL_MS
#define KCONFIG_SAMPLE_INTERVAL_MS CONFIG_APP_SAMPLE_INTERVAL_MS
#else
#define KCONFIG_SAMPLE_INTERVAL_MS 0
#endif

/* devicetree defaults */
#if DT_NODE_HAS_PROP(DT_PATH(app), sample_interval_ms)
#define DT_SAMPLE_INTERVAL_MS DT_PROP(DT_PATH(app), sample_interval_ms)
#else
#define DT_SAMPLE_INTERVAL_MS 1000
#endif

static inline uint32_t app_sample_interval_ms(void)
{
#if KCONFIG_SAMPLE_INTERVAL_MS > 0
    return KCONFIG_SAMPLE_INTERVAL_MS;
#else
    return DT_SAMPLE_INTERVAL_MS;
#endif
}

#endif /* APP_CONFIG_H */
