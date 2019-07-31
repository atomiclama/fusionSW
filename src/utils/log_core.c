
#include "hal.h"
#include "chprintf.h"

#include "log_core.h"

static LOG_LEVEL log_level;

static char msgString[128];

#define MAX_LOGGER 2

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

    if (level <= log_level) {
        msgString[0] = '\n';
        msgString[1] = '\r';
        chsnprintf(&msgString[2], 126, msg);

        for (int i = 0; i < MAX_LOGGER; i++) {
            if (loggerList[i] != 0) {
                loggerList[i](msgString);
            } else {
                break;
            }
        }
    }
}

