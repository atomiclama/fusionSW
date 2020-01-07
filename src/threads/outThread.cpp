
#include "ch.h"
#include "hal.h"

#include "outThread.h"
#include "modemThread.h"

#include "main.h"
#include "log_core.h"
#include "msg_core.h"

extern mailbox_t rxMailbox;

static THD_WORKING_AREA(waThread1, 64);

static THD_FUNCTION( Thread1, arg) {
    (void) arg;
    chRegSetThreadName("blinker");
    bool level = true;
    while(true) {
        radioPacket_t* rxMsg;
        uint8_t retVal = chMBFetchTimeout(&rxMailbox, (msg_t*)&rxMsg, 50000);
        if(retVal == MSG_OK) {
            int8_t snr = rxMsg->snr;
            int8_t rssi = rxMsg->rssi;
            // int8_t sig = rxMsg->sig;

            log_msg(LOG_ALL, "SNR: %3d RSSI: %3d",snr, rssi);
            // msg consumed
            msg_free((uint8_t*)rxMsg);
            
        } else {
            // level = false;
        }
        level ^= 1;
        led.write(level);
    }
}

void outThread_ini(void) {
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
}
