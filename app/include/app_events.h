
#ifndef APP_EVENTS_H
#define APP_EVENTS_H

#include <stdint.h>

enum app_state {
    APP_STATE_IDLE = 0,
    APP_STATE_SAMPLING,
    APP_STATE_ERROR,
};

#endif /* APP_EVENTS_H */
