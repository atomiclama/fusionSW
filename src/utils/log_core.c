
#include "hal.h"
#include "chprintf.h"

#include "log_core.h"
#include "msg_core.h"

static LOG_LEVEL log_level;

#define MAX_LOGGER 5

static LOGGER loggerList[MAX_LOGGER];

void log_init(LOG_LEVEL level) {
    log_setLogLevel(level);
    for (int i = 0; i < MAX_LOGGER; i++) {
        loggerList[i] = 0;
    }
}

void log_setLogLevel(LOG_LEVEL level) {
    log_level = level;
}

// register a function to perform logging to a particular device.
int log_registerLogger(LOGGER logger) {
    for (int i = 0; i < MAX_LOGGER; i++) {
        if (loggerList[i] == 0) {
            loggerList[i] = logger;
            break;
        }
    }
    return 0;
}

void log_msg(LOG_LEVEL level, const char *msg, ...) {
    va_list va;
    if (level <= log_level) {
        systime_t now = chVTGetSystemTime();
        char *msgString;
        msg_alloc((uint8_t *)&msgString);

        msgString[0] = '\n';
        msgString[1] = '\r';
        chsnprintf(&msgString[2], 10, "%8X: ", now);
        va_start(va, msg);
        chvsnprintf(&msgString[11], MSG_SIZE-12, msg, va);
        va_end(va);

        for (int i = 0; i < MAX_LOGGER; i++) {
            if (loggerList[i] != 0) {
                loggerList[i](msgString);
            } else {
                break;
            }
        }
        msg_free((uint8_t *)msgString);
    }
}

