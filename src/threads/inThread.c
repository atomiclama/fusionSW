// Handle ADC for the stick inputs, switches, pots and trim
// buttons for menu system.
// power button
// voltage monitoring

#include "ch.h"
#include "hal.h"

#include "inThread.h"

static THD_WORKING_AREA(waThread2, 128);

static THD_FUNCTION( Thread2, arg) {

    (void) arg;
    chRegSetThreadName("adc");

    // find and configure the ADC

    // find and configure the digital inputs.

    while (true) {

        chThdSleepMilliseconds(500);
    }
}

void inThread_ini(void) {
    chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);
}
