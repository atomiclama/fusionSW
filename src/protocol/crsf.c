#include "ch.h"

#include "proto.h"
#include "crsf.h"


#include "msg_core.h"
#include "modemThread.h"
#include "convThread.h"

#include "crc.h"

uint8_t doCrc(uint8_t * data, uint8_t length) {
    uint8_t crc = 0;

    for (int ii = 0; ii < length; ++ii) {
        crc = crc8_dvb_s2(crc, data[ii]);
    }
    return crc;
}

// encode to air
// not yet supported.
// take the crsf data frame and encode to out air format

// decode our air format and encode to the crsf frame.
decodeRes_e crsfDecodeAir(radioPacket_t* in, radioPacket_t* out) {

    // this is the size of the frame that is to be transfered.
    out->cnt = sizeof(crsfChanFrame_s);
    crsfChanFrame_s * buffer = (crsfChanFrame_s *)out->data;
    buffer->header.device_addr = CRSF_ADDRESS_FLIGHT_CONTROLLER;
    buffer->header.frame_size = sizeof(crsfPackedChan_s) + 2;    // +2 due to type and crc
    buffer->header.type = CRSF_FRAMETYPE_RC_CHANNELS_PACKED;
    // populate channels take care with endianess 
    // debug populate with mid point 
    buffer->chan.ch0 = 988;

    // perform the crc generation
    buffer->crc = doCrc(&(buffer->header.type), sizeof(crsfPackedChan_s)+1 );
    return DEC_PASS;
}

// generate a link status frame to send to the FC.




