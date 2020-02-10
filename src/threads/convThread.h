
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern binary_semaphore_t airTxSema;
extern binary_semaphore_t airRxSema;

extern void convThread_ini(void);

#ifdef __cplusplus
}
#endif