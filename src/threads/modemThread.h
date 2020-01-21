
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SETUP_,
    WAIT_,
    RX_,
    TX_,
} radioState_t;

void modemThread_ini(void);

#ifdef __cplusplus
}
#endif


