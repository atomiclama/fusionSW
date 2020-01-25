
#include "ch.h"
#include "config.h"

#include "main.h"
#include "log_core.h"
#include "msg_core.h"
#include "modemThread.h"
#include <string.h>

#include "convThread.h"

#include "proto.h"
#include "crsf.h"
#include "ibus.h"

static THD_WORKING_AREA(waConvThread, 128);

static THD_FUNCTION( convThread, arg) {
    (void) arg;

    mailbox_t* inMbx = map_getMailbox(decodeIn);
    mailbox_t* outMbx = map_getMailbox(decodeOut);
       
    systime_t prevMsg = chVTGetSystemTime();
    // thread will exit if nothing configured
    while(inMbx && outMbx) {

        radioPacket_t* inMsg;
        uint8_t retVal = chMBFetchTimeout(inMbx, (msg_t*)&inMsg, TIME_INFINITE);
        if(retVal == MSG_OK) {
            systime_t curTime = chVTGetSystemTime();
            if(curTime-prevMsg > 0x20) {
                prevMsg = curTime;
                radioPacket_t* outMsg;
                msg_alloc((uint8_t *)&outMsg);

                // decode the iBus or other frame to our internal
#if defined USE_TX_CFG == true
                if(iBusEncodeAir(outMsg, inMsg) == DEC_PASS) {
#endif

#if defined USE_RX_CFG == true
                if(crsfDecodeAir(outMsg, inMsg) == DEC_PASS) {
#endif
                    // flush the out mbx so that the latest rc data is available
                    uint8_t * flushMsg;
                    while(chMBFetchTimeout(outMbx, (msg_t*)&flushMsg, TIME_IMMEDIATE) == MSG_OK){
                        msg_free(flushMsg); 
                    }
                    chMBPostTimeout(outMbx, (msg_t)outMsg, TIME_INFINITE);
                } else {
                    // didn't decode so drop msg
                    msg_free((uint8_t*)outMsg); 
                }  
            }
            // msg consumed          
            msg_free((uint8_t*)inMsg);            
        } 
    }
}

void convThread_ini(void) {
    chThdCreateStatic(waConvThread, sizeof(waConvThread), NORMALPRIO, convThread, NULL);
}
