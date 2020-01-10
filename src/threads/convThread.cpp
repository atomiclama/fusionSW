
#include "ch.h"

#include "main.h"
#include "log_core.h"
#include "msg_core.h"
#include "modemThread.h"
#include <string.h>

#include "convThread.h"

// turn off op to enable debugging
// #pragma GCC optimize ("O0")

static THD_WORKING_AREA(waConvThread, 128);

typedef enum {
    DEC_FAIL = -1,
    DEC_PASS = 0
}decodeRes_e;


#define IBUS_MAX_CHANNEL 12
#define IBUS_BUFFSIZE 32

volatile uint32_t ibusChannelData[IBUS_MAX_CHANNEL];

bool __attribute__((optimize("O0"))) isChecksumOk(radioPacket_t* in)
{
    // uint8_t offset = 2;
    uint8_t* ibus =  &(in->data[2]);
    uint16_t chksum = 0xFFFF;
    
    
    for (uint8_t i = 0; i < IBUS_MAX_CHANNEL; i++) {
        chksum += ibus[i] + ((ibus[i + 1]) << 8);
    }
    uint16_t rxsum = ibus[IBUS_BUFFSIZE - 2] + (ibus[IBUS_BUFFSIZE - 1] << 8);
    return true;//chksum == rxsum;
}

// __attribute__((optimize("O0")))
decodeRes_e iBusDecode(radioPacket_t* in, radioPacket_t* out) {
    decodeRes_e retVal = DEC_FAIL;
    // first validate frame checksum
    if(isChecksumOk(in)) {
        retVal = DEC_PASS;
        // extract data
        uint8_t offset = 2;
        uint8_t* ibus =  in->data;
        for (uint8_t i = 0; i < IBUS_MAX_CHANNEL; i++, offset += 2) {
            ibusChannelData[i] = ibus[offset] + ((ibus[offset + 1] ) << 8); // & 0x0F
        }
    }
    memcpy(out, in, sizeof(radioPacket_t));
    return retVal;
}


static THD_FUNCTION( convThread, arg) {
    (void) arg;

    mailbox_t* inMbx = map_getMailbox(decodeIn);
    mailbox_t* outMbx = map_getMailbox(decodeOut);
       
    // thread will exit if nothong configured
    while(inMbx && outMbx) {

        radioPacket_t* inMsg;
        uint8_t retVal = chMBFetchTimeout(inMbx, (msg_t*)&inMsg, TIME_INFINITE);
        if(retVal == MSG_OK) {

            radioPacket_t* outMsg;
            msg_alloc((uint8_t *)&outMsg);

            // decode the iBus or other frame to our internal
            if(iBusDecode(inMsg, outMsg) == DEC_PASS) {
                chMBPostTimeout(outMbx, (msg_t)outMsg, TIME_INFINITE);
            } else {
                // didn't decode so drop msg
                msg_free((uint8_t*)outMsg); 
            }  
            // msg consumed          
            msg_free((uint8_t*)inMsg);            
        } 
    }
}

void convThread_ini(void) {
    chThdCreateStatic(waConvThread, sizeof(waConvThread), NORMALPRIO, convThread, NULL);
}
