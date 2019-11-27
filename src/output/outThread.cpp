// LED control

#include "ch.h"
#include "hal.h"

#include "outThread.h"
#include "DigitalOut.h"

/*
 * Green LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);

DigitalOut led(GPIOA, 5);

extern mailbox_t rxMailbox; 
uint32_t cnt;
bool level;

static THD_FUNCTION( Thread1, arg) {
    (void) arg;
    chRegSetThreadName("blinker");

    while (true) {
        msg_t msgData; 
        (void)chMBFetchTimeout(&rxMailbox, &msgData, TIME_INFINITE);

        if (++cnt > 5){
            cnt = 0;
            level^=1;
            led.write(level);
        }
        
        // chThdSleepMilliseconds(500);
        // led.write(0);
    }
}

void outThread_ini(void) {
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
}
