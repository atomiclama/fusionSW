
#include "ch.h"
#include "hal.h"

#include "outThread.h"

#include "main.h"
#include "log_core.h"
#include "msg_core.h"

extern mailbox_t rxMailbox;

static THD_WORKING_AREA(waThread1, 32);

static THD_FUNCTION( Thread1, arg) {
    (void) arg;
    chRegSetThreadName("blinker");
    bool level = false;
    while (true) {
        uint8_t* rxMsg;
        uint8_t retVal = chMBFetchTimeout(&rxMailbox, (msg_t*)&rxMsg, 5000);
        if(retVal == MSG_OK) {
            // msg consumed
            msg_free(rxMsg);
            log_msg(LOG_ALL, "message RX");
        }
        // chThdSleepMilliseconds(500);
        level^=1;
        led.write(level);
        log_msg(LOG_ALL, "Heart Beat");
    }
}

void outThread_ini(void) {
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
}
