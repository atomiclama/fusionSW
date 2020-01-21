
#include "ch.h"
#include "hal.h"

#include "main.h"
#include "proto.h"
#include "serialThread.h"
#include "modemThread.h"
#include "msg_core.h"


const UARTConfig uartConfig = {
    .txend1_cb = NULL,
    .txend2_cb = NULL,
    .rxend_cb = NULL,
    .rxchar_cb = NULL,
    .rxerr_cb = NULL,
    .timeout_cb = NULL,
    .speed = 115200,
    .cr1 = USART_CR1_IDLEIE,                  
    .cr2 = USART_CR2_STOP_1,
    .cr3 = 0,
};


const UARTConfig uartGpsConfig = {
    .txend1_cb = NULL,
    .txend2_cb = NULL,
    .rxend_cb = NULL,
    .rxchar_cb = NULL,
    .rxerr_cb = NULL,
    .timeout_cb = NULL,
    .speed = 57600,
    .cr1 = USART_CR1_IDLEIE,                  
    .cr2 = USART_CR2_STOP_1,
    .cr3 = 0,
};


typedef struct {
    UARTDriver *uartp;
    map_e id;
} threadConfig_s;

#if STM32_UART_USE_USART1 == TRUE
static THD_WORKING_AREA(waThreadU1Tx, 128);
static THD_WORKING_AREA(waThreadU1Rx, 128);

const threadConfig_s configU1rx = {
    .uartp = &UARTD1,
    .id = U1rx,
};
const threadConfig_s configU1tx = {
    .uartp = &UARTD1,
    .id = U1tx,
};
#endif

#if STM32_UART_USE_USART2 == TRUE
static THD_WORKING_AREA(waThreadU2Tx, 128);
static THD_WORKING_AREA(waThreadU2Rx, 128);

const threadConfig_s configU2tx = {
    .uartp = &UARTD2,
    .id = U2tx,
};
const threadConfig_s configU2rx = {
    .uartp = &UARTD2,
    .id = U2rx,
};
#endif 

#if STM32_UART_USE_USART3 == TRUE
static THD_WORKING_AREA(waThreadU3Tx, 128);
static THD_WORKING_AREA(waThreadU3Rx, 128);
#endif


static THD_FUNCTION( ThreadURx, arg) {
   
    UARTDriver *uartp = ((threadConfig_s *)arg)->uartp;

    map_e map = ((threadConfig_s *)arg)->id;
    mailbox_t * mailbox = map_getMailbox(map);
    
    while(mailbox) {

        radioPacket_t* pbuf;
 
        /* get an empty buffer.*/
        msg_alloc((uint8_t *)&pbuf);
 
        // rx upto this number of bytes.
        pbuf->cnt = 100;

        uartReceiveTimeout(uartp, &(pbuf->cnt), pbuf->data, TIME_INFINITE);

        if(pbuf->cnt != 0) {
            // if it does not post staright away then drop it.
            if(chMBPostTimeout(mailbox, (msg_t)pbuf, TIME_IMMEDIATE) != MSG_OK){
                msg_free((uint8_t*)pbuf);
            }
        } else {
            msg_free((uint8_t*)pbuf);
        }    
    }
}
static THD_FUNCTION( ThreadUTx, arg) {
    
    UARTDriver *uartp = ((threadConfig_s *)arg)->uartp;

    map_e map = ((threadConfig_s *)arg)->id;
    mailbox_t* mailbox = map_getMailbox(map);
 
    // if no mailbox to use then this thread will just exit.
    while(mailbox) {

        radioPacket_t* pbuf;
 
        // get from full buffer
        uint8_t retVal = chMBFetchTimeout(mailbox, (msg_t *)&pbuf, TIME_INFINITE);
        if(retVal == MSG_OK) {
            uartSendTimeout(uartp, &(pbuf->cnt), pbuf->data, TIME_INFINITE);
            // return the buffer
            msg_free((uint8_t*)pbuf);

            // this is a dirty hack.
            // if there is a msg already waiting then drop it probably duplicate from radio.
            retVal = chMBFetchTimeout(mailbox, (msg_t *)&pbuf, TIME_IMMEDIATE);
            if(retVal == MSG_OK) {
                msg_free((uint8_t*)pbuf);
            }
        }
    }
}

void serialThread_ini(void) {
#if STM32_UART_USE_USART1 == TRUE
    #ifdef USE_DEBUG_RADIO
        uartStart(&UARTD1, &uartConfig);
    #else
        uartStart(&UARTD1, &uartGpsConfig);
    #endif
    chThdCreateStatic(waThreadU1Tx, sizeof(waThreadU1Tx), NORMALPRIO, ThreadUTx, (void*)&configU1tx);
    chThdCreateStatic(waThreadU1Rx, sizeof(waThreadU1Rx), NORMALPRIO, ThreadURx, (void*)&configU1rx);
#endif
#if STM32_UART_USE_USART2 == TRUE
    uartStart(&UARTD2, &uartConfig);
    chThdCreateStatic(waThreadU2Tx, sizeof(waThreadU2Tx), NORMALPRIO, ThreadUTx, (void*)&configU2tx);
    chThdCreateStatic(waThreadU2Rx, sizeof(waThreadU2Rx), NORMALPRIO, ThreadURx, (void*)&configU2rx);
#endif
}
