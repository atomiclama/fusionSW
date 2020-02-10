
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

    mailbox_t* rxMailbox = map_getMailbox(Clirx);
    mailbox_t* txMailbox = map_getMailbox(U2tx);

    bool level = true;
    uint8_t cnt =0;
    
    while(rxMailbox) {
        radioPacket_t* rxMsg;
        uint8_t retVal = chMBFetchTimeout(rxMailbox, (msg_t*)&rxMsg, 10000);
        if(retVal == MSG_OK) {
            if(++cnt > 10) {
                cnt = 0;

                int8_t snr = rxMsg->snr;
                int8_t rssi = rxMsg->rssi;
                int8_t crcErr = rxMsg->dbm;

                // got what we want so ditch msg
                msg_free((uint8_t*)rxMsg);

                log_msg(LOG_ALL, "SNR: %3d RSSI: %3d d",snr, rssi);
               
                // https://www.loratracker.uk/lora-signal-quality-rssi-or-snr/
                // scale snr so can be used in lQ display
                // snr ~ +10 -20 dbm 
                // -7.5 dbm limit for our SF7 
                // lq +70 vgood.
                // lq 35  prob not going to work.
                uint8_t lq = 50 + (snr*2);

                radioPacket_t* statusMsg;
                msg_alloc((uint8_t *)&statusMsg);
                
                // must be at least 250ms update rate 
                crsfEncodeStatus(statusMsg, rssi, lq);
                
                // if it does not post straight away then drop it.
                if(chMBPostTimeout(txMailbox, (msg_t)statusMsg, TIME_IMMEDIATE) != MSG_OK){
                    msg_free((uint8_t*)statusMsg);
                }
                
            } else {
    
                // msg consumed
                msg_free((uint8_t*)rxMsg);

                // do some filtering 
                // missed packet detection LQ calculation.
                // maybe 
                // https://dsp.stackexchange.com/questions/20333/how-to-implement-a-moving-average-in-c-without-a-buffer
                // ma_new = alpha * new_sample + (1-alpha) * ma_old
            }
            
        } 
        level ^= 1;
        led.write(level);
    }
}

void outThread_ini(void) {
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO-1, Thread1, NULL);
}
