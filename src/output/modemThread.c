#include "ch.h"
#include "hal.h"

#include "modemThread.h"

static THD_WORKING_AREA(waThread3, 128);

static THD_FUNCTION( Thread3, arg) {

    (void) arg;
    chRegSetThreadName("modem");

    // find and open the HW
    // find and init the protocol
    // attach to the rfmodem

    while (true) {
        chThdSleepMilliseconds(500);
    }
}

void modemThread_ini(void) {

    chThdCreateStatic(waThread3, sizeof(waThread3), NORMALPRIO, Thread3, NULL);
}
