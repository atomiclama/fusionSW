

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t rc1:10;
    uint16_t rc5:4;
    uint16_t rc9:1;
    uint16_t rc10:1;
    uint16_t rc2:10;
    uint16_t rc6:4;
    uint16_t rc11:1;
    uint16_t rc12:1;
    uint16_t rc3:10;
    uint16_t rc7:4;
    uint16_t rc13:1;
    uint16_t rc14:1;
    uint16_t rc4:10;
    uint16_t rc8:4;
    uint16_t rc15:1;   
    uint16_t rc16:1;
} rcData_s;


typedef union {
    uint8_t data[100]; // the data tx/rx
    struct {
        uint8_t id;     // channel slot some way of detiermining where it goes. 
        uint8_t cnt;    // packet count used to detect duplicate packets or dropped packets.
        uint8_t size;
    }field;
    
} radioData_t;



typedef struct {
    systime_t stamp; // time stamp 
    uint8_t id;     // channel slot some way of detiermining where it goes. 
    size_t  cnt;    // packet count used to detect duplicate packets or dropped packets.
    int8_t snr;     // extracted from radio device
    int8_t rssi;    // dito
    int8_t sig;     // dito 
    int8_t dbm;     // maybe passed from tx 
 
    uint8_t data[100]; // the data tx/rx

} radioPacket_t;


typedef enum {
    DEC_FAIL = -1,
    DEC_PASS = 0
} decodeRes_e;



#ifdef __cplusplus
}
#endif

