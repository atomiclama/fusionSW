
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

static THD_WORKING_AREA(waConvThread, 256);

binary_semaphore_t airTxSema;
binary_semaphore_t airRxSema;

rcData_s airData;

#if defined USE_RX_CFG == true

static THD_FUNCTION( convThread, arg) {
    (void) arg;

    mailbox_t* outMbx = map_getMailbox(decodeOut);
       
    systime_t prevMsg = chVTGetSystemTime();
    // thread will exit if nothing configured
    while(outMbx) {   
        uint8_t retVal = chBSemWaitTimeout(&airRxSema, TIME_INFINITE);
        if(retVal == MSG_OK) {
            systime_t curTime = chVTGetSystemTime();
            systime_t tmpTime = curTime-prevMsg;      
    
            if( tmpTime > 0x20) {
                prevMsg = curTime;
                radioPacket_t* outMsg;
                msg_alloc((uint8_t *)&outMsg);

                // decode the air frame 
                if(crsfDecodeAir(outMsg, &airData) == DEC_PASS) {
                    chMBPostTimeout(outMbx, (msg_t)outMsg, TIME_INFINITE);
                } else {
                    // didn't decode so drop msg
                    msg_free((uint8_t*)outMsg); 
                }  
            }       
        } 
    }
}
#endif

#if defined USE_TX_CFG == true
static THD_FUNCTION( convThread, arg) {
    (void) arg;

    mailbox_t* inMbx = map_getMailbox(decodeIn);

    // thread will exit if nothing configured
    while(inMbx) {
        radioPacket_t* inMsg;
        uint8_t retVal = chMBFetchTimeout(inMbx, (msg_t*)&inMsg, TIME_INFINITE);
        if(retVal == MSG_OK) {
            // encode to the air frame
            if(iBusEncodeAir(&airData, inMsg) == DEC_PASS) {
                chBSemSignal(&airTxSema);
            }
            // msg consumed          
            msg_free((uint8_t*)inMsg);            
        } 
    }
}
#endif
void convThread_ini(void) {
    chBSemObjectInit(&airTxSema, true);
    chBSemObjectInit(&airRxSema, true);
    chThdCreateStatic(waConvThread, sizeof(waConvThread), NORMALPRIO, convThread, NULL);
}
