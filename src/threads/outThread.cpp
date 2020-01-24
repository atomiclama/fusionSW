
#include "ch.h"
#include "hal.h"

#include "outThread.h"
#include "modemThread.h"

#include "main.h"
#include "proto.h"
#include "crsf.h"
#include "log_core.h"
#include "msg_core.h"

static THD_WORKING_AREA(waThread1, 256);

static THD_FUNCTION( Thread1, arg) {
    (void) arg;
    chRegSetThreadName("blinker");

    mailbox_t* rxMailbox = map_getMailbox(Clirx);
    mailbox_t* txMailbox = map_getMailbox(U2tx);

    bool level = true;
    uint8_t cnt =0;
    
    while(rxMailbox) {
        radioPacket_t* rxMsg;
        uint8_t retVal = chMBFetchTimeout(rxMailbox, (msg_t*)&rxMsg, 10000);
        if(retVal == MSG_OK) {
            if(++cnt > 50)
            {
                msg_free((uint8_t*)rxMsg);
                cnt = 0;
            }else{
            int8_t snr = rxMsg->snr;
            int8_t rssi = rxMsg->rssi;
            int8_t sig = rxMsg->dbm;

            log_msg(LOG_ALL, "SNR: %3d RSSI: %3d CRC: %3d",snr, rssi, sig);
            // // msg consumed
            // msg_free((uint8_t*)rxMsg);

            // do some filtering 
            // missed packet detection LQ calculation.
            // other low priority stuff and slow down the output update.
            // carefull rxMsg resue.
            crsfEncodeStatus(NULL, rxMsg);
            
            // if it does not post straight away then drop it.
            if(chMBPostTimeout(txMailbox, (msg_t)rxMsg, TIME_IMMEDIATE) != MSG_OK){
                msg_free((uint8_t*)rxMsg);
            }
            }
            
        } else {
            // level = false;
        }
        level ^= 1;
        led.write(level);
    }
}

void outThread_ini(void) {
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO-1, Thread1, NULL);
}
