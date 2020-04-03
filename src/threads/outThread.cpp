
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


#define RSSI_SAMPLE_COUNT 16

static int8_t updateRssiSamples(int8_t value)
{
    static int8_t samples[RSSI_SAMPLE_COUNT];
    static uint8_t sampleIndex = 0;
    static signed sum = 0;

    sum += value - samples[sampleIndex];
    samples[sampleIndex] = value;
    sampleIndex = (sampleIndex + 1) % RSSI_SAMPLE_COUNT;
    return sum / RSSI_SAMPLE_COUNT;
}

static THD_FUNCTION( Thread1, arg) {
    (void) arg;

    mailbox_t* rxMailbox = map_getMailbox(Clirx);
    mailbox_t* txMailbox = map_getMailbox(U2tx);

    bool level = true;

    systime_t prevMsg = chVTGetSystemTime();
    int8_t snr;
    int8_t rssi;

    while(rxMailbox) {
        radioPacket_t* rxMsg;
        uint8_t retVal = chMBFetchTimeout(rxMailbox, (msg_t*)&rxMsg, 10000);
        if(retVal == MSG_OK) {

            systime_t curTime = chVTGetSystemTime();
            systime_t tmpTime = curTime-prevMsg;      
    
            if(( tmpTime > 0x20) && txMailbox) {
                prevMsg = curTime;

                // take and filter the highest recorded value;
                // looks like this is done in BF
                // rssi = updateRssiSamples(rssi);


                // missed packet detection LQ calculation.
                // maybe 
                // https://dsp.stackexchange.com/questions/20333/how-to-implement-a-moving-average-in-c-without-a-buffer
                // ma_new = alpha * new_sample + (1-alpha) * ma_old
                log_msg(LOG_ALL, "SNR: %3d RSSI: %3d",snr, rssi);
               
                // https://www.loratracker.uk/lora-signal-quality-rssi-or-snr/
                // scale snr so can be used in lQ display
                // snr ~ +10 -20 dbm 
                // -7.5 dbm limit for our SF7 
                // lq +70 vgood.
                // lq 35  prob not going to work.
                // uint8_t lq = 50 + (snr*2);

                radioPacket_t* statusMsg;
                msg_alloc((uint8_t *)&statusMsg);
                
                // must be at least 250ms update rate 
                crsfEncodeStatus(statusMsg, rssi, snr);
                
                // if it does not post straight away then drop it.
                if(chMBPostTimeout(txMailbox, (msg_t)statusMsg, TIME_IMMEDIATE) != MSG_OK){
                    msg_free((uint8_t*)statusMsg);
                }
                // reset values for next pass
                snr = -12;
                rssi = -128;
            }

            if( rxMsg->snr > snr) {
                snr = rxMsg->snr;
            }
            
            if( rxMsg->rssi > rssi) {
                rssi = rxMsg->rssi;
            }

            // got what we want so ditch msg
            msg_free((uint8_t*)rxMsg);           
        } 
        level ^= 1;
        led.write(level);
    }
}

void outThread_ini(void) {
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO-1, Thread1, NULL);
}
