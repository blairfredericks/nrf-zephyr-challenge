
#include <zephyr/ztest.h>
#include <zephyr/sys/byteorder.h>

ZTEST_SUITE(ble_codec, NULL, NULL, NULL, NULL, NULL);

ZTEST(ble_codec, test_le_encoding)
{
    uint32_t v = 0x12345678;
    uint8_t buf[4];
    sys_put_le32(v, buf);
    uint32_t read = sys_get_le32(buf);
    zassert_equal(v, read, "LE encode/decode mismatch");
}
