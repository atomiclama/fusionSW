#include "ch.h"

#include "proto.h"
#include "crsf.h"


#include "msg_core.h"
#include "modemThread.h"
#include "convThread.h"

#include "crc.h"

#include <string.h>

static uint8_t doCrc(uint8_t * data, uint8_t length) {
    uint8_t crc = 0;

    for (int ii = 0; ii < length; ++ii) {
        crc = crc8_dvb_s2(crc, data[ii]);
    }
    return crc;
}

decodeRes_e crsfEncodeAir(radioPacket_t* out, radioPacket_t* in) {
    (void)in;
    (void)out;
    return DEC_PASS;
}

// taken from BF
    /* conversion from crsf RC value to PWM
     *       RC     PWM
     * min  172 ->  988us
     * mid  992 -> 1500us
     * max 1811 -> 2012us
     * scale factor = (2012-988) / (1811-172) = 0.62477120195241
     * offset = 988 - 172 * 0.62477120195241 = 880.53935326418548
     */
    // return (0.62477120195241f * crsfChannelData[chan]) + 881;
const uint16_t rcMin = 172;
const uint16_t rcMax = 1811;   
const uint16_t rcMid = 992;

/*
Conversion from air -> crsf rc
min 0 -> 190

max 1000 -> 1791 
gain = (1811-172)/(1024-0) = 1.601
off = 172

*/
const float crsfGain = 1.6f;
const uint16_t crsfOff = 172;

decodeRes_e crsfDecodeAir(radioPacket_t* out, rcData_s* airData) {

    // this is the size of the frame that is to be transfered.
    out->cnt = sizeof(crsfChanFrame_s);
    crsfChanFrame_s * buffer = (crsfChanFrame_s *)out->data;
    buffer->header.device_addr = CRSF_ADDRESS_FLIGHT_CONTROLLER;
    buffer->header.frame_size = sizeof(crsfPackedChan_s) + 2;    // +2 due to type and crc
    buffer->header.type = CRSF_FRAMETYPE_RC_CHANNELS_PACKED;

    // rcData_s* airData = (rcData_s*)(in->data);

    // 4 10 bit channels
    buffer->chan.ch0 = (airData->rc1 *crsfGain)+crsfOff;
    buffer->chan.ch1 = (airData->rc2 *crsfGain)+crsfOff;
    buffer->chan.ch2 = (airData->rc3 *crsfGain)+crsfOff;
    buffer->chan.ch3 = (airData->rc4 *crsfGain)+crsfOff;

    // 4 4 bit channels
    buffer->chan.ch4 = ((airData->rc5<<6) *crsfGain)+crsfOff;
    buffer->chan.ch5 = ((airData->rc6<<6) *crsfGain)+crsfOff;
    buffer->chan.ch6 = ((airData->rc7<<6) *crsfGain)+crsfOff;
    buffer->chan.ch7 = ((airData->rc8<<6) *crsfGain)+crsfOff;

    // 4 1 bit channels
    buffer->chan.ch8 = airData->rc9 ? rcMax : rcMin;
    buffer->chan.ch9 = airData->rc10 ? rcMax : rcMin;
    buffer->chan.ch10 = airData->rc11 ? rcMax : rcMin;
    buffer->chan.ch11 = airData->rc12 ? rcMax : rcMin;

    // unsupported 
    buffer->chan.ch12 = rcMin;
    buffer->chan.ch13 = rcMin;
    buffer->chan.ch14 = rcMin;
    buffer->chan.ch15 = rcMin;

    // perform the crc generation
    buffer->crc = doCrc(&(buffer->header.type), sizeof(crsfPackedChan_s)+1 );
    return DEC_PASS;
}

// generate a link status frame to send to the FC.
decodeRes_e crsfEncodeStatus(radioPacket_t* out, int8_t rssi, uint8_t lq) {

    // this is the size of the frame that is to be transfered.
    out->cnt = sizeof(crsfStatsFrame_s);
    // clear our stats data.
    memset(out->data, 0, out->cnt);

    crsfStatsFrame_s * buffer = (crsfStatsFrame_s *)out->data;
    buffer->header.device_addr = CRSF_ADDRESS_FLIGHT_CONTROLLER;
    buffer->header.frame_size = sizeof(crsfLinkStats_s) + 2;    // +2 due to type and crc
    buffer->header.type = CRSF_FRAMETYPE_LINK_STATISTICS;

    // hardcode some debug data just to test.
    buffer->stats.active_antenna = 0;
    buffer->stats.uplink_RSSI_1 = rssi * -1;
    buffer->stats.uplink_Link_quality = lq;
    buffer->stats.uplink_SNR = 10;          // not used by BF
    // perform the crc generation
    buffer->crc = doCrc(&(buffer->header.type), sizeof(crsfLinkStats_s)+1 );
    return DEC_PASS;
}



