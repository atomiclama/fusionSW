#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#define MSG_SIZE 256

extern void msg_init(void);
extern void msg_alloc (uint8_t ** pBuff);

extern void msg_free (uint8_t * pBuff);

#ifdef __cplusplus
}
#endif
