#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#define MSG_SIZE 256

extern void msg_init(void);
extern void msg_alloc (uint8_t * pBuff);
extern void msg_free (uint8_t * pBuff);

typedef enum {
    None,
    R1tx,
    R1rx,
    R2tx,
    R2rx,
    U1tx,
    U1rx,
    U2tx,
    U2rx,
    U3tx,
    U3rx,
    U4tx,
    U4rx,
    Clitx,
    Clirx,
    decodeIn,
    decodeOut,
    encodeIn,
    encodeOut,
    Free
} map_e;

extern void map_init(void);
extern mailbox_t * map_getMailbox(map_e id);


#ifdef __cplusplus
}
#endif
