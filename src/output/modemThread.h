


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SETUP_,
    WAIT_,
    RX_,
    TX_,
} radioState_t;

typedef union {
    uint8_t data[100]; // the data tx/rx
    struct {
        uint8_t id;     // channel slot some way of detiermining where it goes. 
        uint8_t cnt;    // packet count used to detect duplicate packets or dropped packets.
        uint8_t size;
    }field;
    
}radioData_t;



typedef struct {
    uint8_t id;     // channel slot some way of detiermining where it goes. 
    uint8_t cnt;    // packet count used to detect duplicate packets or dropped packets.
    int8_t snr;     // extracted from radio device
    int8_t rssi;    // dito
    int8_t sig;     // dito 
    int8_t dbm;     // maybe passed from tx 
 
    uint8_t data[100]; // the data tx/rx

} radioPacket_t;


void modemThread_ini(void);

#ifdef __cplusplus
}
#endif


