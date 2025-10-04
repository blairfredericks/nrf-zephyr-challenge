
#ifndef APP_UUIDS_H
#define APP_UUIDS_H

#include <zephyr/types.h>

/* 128-bit base UUID for the app service: change to your own */
#define BT_UUID_APP_SERVICE_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x9abc, 0xdef012345678)

#endif /* APP_UUIDS_H */
