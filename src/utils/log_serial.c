#include "hal.h"
#include "chprintf.h"

#include "log_core.h"
#include "log_serial.h"

//static FILE * logSerial;
static BaseSequentialStream *chp;

static void serialLogger(const char * msg) {
    chprintf(chp, msg);
}

void log_serialInit(BaseSequentialStream *chp1) {
    chp = chp1;
    log_registerLogger(serialLogger);
}

