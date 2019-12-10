
#include "ch.h"
#include "hal.h"

#include "outThread.h"

#include "main.h"
#include "log_core.h"

static THD_WORKING_AREA(waThread1, 32);

static THD_FUNCTION( Thread1, arg) {
    (void) arg;
    chRegSetThreadName("blinker");
    bool level = false;
    while (true) {
        chThdSleepMilliseconds(500);
        level^=1;
        led.write(level);
        log_msg(LOG_ALL, "Heart Beat");
    }
}

void outThread_ini(void) {
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
}
