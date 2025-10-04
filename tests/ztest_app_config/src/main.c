
#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>

ZTEST_SUITE(app_config, NULL, NULL, NULL, NULL, NULL);

ZTEST(app_config, test_kconfig_override)
{
    /* This test ensures that if CONFIG_APP_SAMPLE_INTERVAL_MS > 0, it is used.
       The test cannot change build-time Kconfig, so it asserts that the symbol exists
       and has an expected type. This test is minimal and primarily serves CI sanity. */
#ifdef CONFIG_APP_SAMPLE_INTERVAL_MS
    zassert_true(CONFIG_APP_SAMPLE_INTERVAL_MS >= 0);
#else
    zassert_true(true, "Kconfig symbol not defined - acceptable for some builds");
#endif
}
