
#include "ch.h"


#include "proto.h"
#include "ibus.h"

#include <string.h>

#define IBUS_MAX_CHANNEL 12
#define IBUS_BUFFSIZE 32

// TODO this will go
volatile uint32_t ibusChannelData[IBUS_MAX_CHANNEL];


//__attribute__((optimize("O0")))

bool  isChecksumOk(radioPacket_t* in) {
    uint8_t* ibus =  &(in->data[0]);
    uint16_t chksum = 0;
    
    uint8_t i = 0;
    for (; i < IBUS_BUFFSIZE-2; ) {
        chksum += ibus[i++];
    }
    uint16_t rxsum = ibus[i++];
    rxsum += (ibus[i++] << 8);

    return chksum + rxsum == 0xffff;
}

// __attribute__((optimize("O0")))
decodeRes_e iBusEncode(radioPacket_t* in, radioPacket_t* out) {
    decodeRes_e retVal = DEC_FAIL;
    // first validate frame checksum
    if(isChecksumOk(in)) {
        retVal = DEC_PASS;
        // extract data
        uint8_t offset = 2;
        uint8_t* ibus =  in->data;
        for (uint8_t i = 0; i < IBUS_MAX_CHANNEL; i++, offset += 2) {
            ibusChannelData[i] = ibus[offset] + ((ibus[offset + 1] ) << 8) - 988; // & 0x0F
        }
    }
    rcData_s* outData = (rcData_s*)(out->data);
    // 4 10 bit channels
    outData->rc1 = ibusChannelData[0];
    outData->rc2 = ibusChannelData[1];
    outData->rc3 = ibusChannelData[2];
    outData->rc4 = ibusChannelData[3];
    // 4 4 bit channels
    outData->rc5 = ibusChannelData[4] >> 6;
    outData->rc6 = ibusChannelData[5] >> 6;
    outData->rc7 = ibusChannelData[6] >> 6;
    outData->rc8 = ibusChannelData[7] >> 6;
    // 4 1 bit channels
    outData->rc9 = (ibusChannelData[8] > 500);
    outData->rc10 = (ibusChannelData[9] > 500);
    outData->rc11 = (ibusChannelData[10] > 500);
    outData->rc12 = (ibusChannelData[11] > 500);

    out->cnt = sizeof(rcData_s);

    // scrap the above until tested.
    // memcpy(out, in, sizeof(radioPacket_t));
    return retVal;
}

decodeRes_e iBusDecode(radioPacket_t* in, radioPacket_t* out) {
    decodeRes_e retVal = DEC_FAIL;

    out->cnt = 32;
    out->data[0] = 32;
    out->data[1] = 0xAA; // check and change

    uint16_t* ibus =  (uint16_t*)&(out->data[2]);
    rcData_s* inData = (rcData_s*)(in->data);
    // convert compressed rc data back to 1000 -> 2000 range 

    // 4 10 bit channels
    ibus[0] = inData->rc1 + 1000;
    ibus[1] = inData->rc2 + 1000;
    ibus[2] = inData->rc3 + 1000;
    ibus[3] = inData->rc4 + 1000;
    // 4 4 bit channels
    ibus[4] = (inData->rc5 << 6)  + 1000;
    ibus[5] = (inData->rc6 << 6)  + 1000;
    ibus[6] = (inData->rc7 << 6)  + 1000;
    ibus[7] = (inData->rc8 << 6)  + 1000;
    // 8 1 bit channels
    ibus[8] = inData->rc9;
    ibus[9] = inData->rc10;
    ibus[10] = inData->rc11;
    ibus[11] = inData->rc12;

    // insert radio quality metrics RSSI LQ 
    ibus[12] = in->rssi;
    ibus[13] = in->snr;

    // calc CS

    return retVal;
}

