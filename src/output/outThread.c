// LED control

#include "ch.h"
#include "hal.h"

#include "outThread.h"

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);

static THD_FUNCTION( Thread1, arg) {

    (void) arg;
    chRegSetThreadName("blinker");

    // Find and configure the status LEDs


    while (true) {
        palClearPad(GPIOA, GPIOA_LED_GREEN);
        chThdSleepMilliseconds(500);
        palSetPad(GPIOA, GPIOA_LED_GREEN);
        chThdSleepMilliseconds(500);
    }
}

void outThread_ini(void) {
    /*
     * Creates the blinker thread.
     */
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
}
