#include "ch.h"
#include "hal.h"
//#include "rt_test_root.h"
//#include "oslib_test_root.h"

#include "msg_core.h"
#include "log_core.h"
#include "log_serial.h"
#include "log_swo.h"
#include "inThread.h"
#include "modemThread.h"
#include "outThread.h"
#include "main.h"


// board specific
DigitalOut led(GPIOA, 8);

SX126x radio1Hw(&SPID2);  
SX126x radio2Hw(&SPID2);

DigitalOut radio1Nss(GPIOC, 13);
DigitalIn  radio1Busy(GPIOC, 14);

DigitalOut radio2Nss(GPIOC, 15);
DigitalIn  radio2Busy(GPIOB, 12);

extern mailbox_t txMailbox; 



void timeout_cb(UARTDriver *uartp) {

    // determine number of bytes RX
    osalSysLockFromISR();                                                   
    osalThreadResumeI(&(uartp)->threadrx, MSG_OK);                          
    osalSysUnlockFromISR(); 
}


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



#define NUM_BUFF 10

static msg_t filled_buffers_queue[NUM_BUFF];
static mailbox_t filled_buffers;

static THD_WORKING_AREA(waThreadU1Tx, 128);
static THD_WORKING_AREA(waThreadU1Rx, 128);
static THD_WORKING_AREA(waThreadU3Tx, 128);
static THD_WORKING_AREA(waThreadU3Rx, 128);


static THD_FUNCTION( ThreadSerial, arg) {
    
    UARTDriver *uartp = (UARTDriver *)arg;
    uartStart(uartp, &uartConfig);

    while(true) {
        // chThdSleepMilliseconds(10);
        uint8_t *pbuf;
 
        /* get an empty buffer.*/
        msg_alloc((uint8_t *)&pbuf);
 
        size_t n = 10;
        uartReceiveTimeout(uartp, &n, pbuf, TIME_INFINITE);

        // uartStartReceive(uartp, 32, pbuf);
        // chThdSleepMilliseconds(100);
        // size_t n = uartStopReceive(uartp);

        // while(sdReadTimeout(sdp, pbuf, 32, TIME_MS2I(20)) == 0);
        // post to full buffer
        (void)chMBPostTimeout(&filled_buffers, (msg_t)pbuf, TIME_INFINITE);

        // get from full buffer
        chMBFetchTimeout(&filled_buffers, (msg_t *)&pbuf, TIME_INFINITE);
        
        uartSendTimeout(uartp, &n, pbuf, TIME_INFINITE);

        // return the buffer
        msg_free(pbuf);
    }
}


static THD_FUNCTION( ThreadURx, arg) {
    
    UARTDriver *uartp = (UARTDriver *)arg;

    while(true) {
        // chThdSleepMilliseconds(10);
        radioPacket_t* pbuf;
 
        /* get an empty buffer.*/
        msg_alloc((uint8_t *)&pbuf);
 
        // rx upto this number of bytes.
        pbuf->cnt = 100;

        uartReceiveTimeout(uartp, &(pbuf->cnt), pbuf->data, TIME_INFINITE);

        if(pbuf->cnt != 0) {
            (void)chMBPostTimeout(&filled_buffers, (msg_t)pbuf, TIME_INFINITE);
        }else{
            msg_free((uint8_t*)pbuf);
        }
    }
}
static THD_FUNCTION( ThreadUTx, arg) {
    
    UARTDriver *uartp = (UARTDriver *)arg;
 
    while(true) {

        radioPacket_t* pbuf;
 
        // get from full buffer
        uint8_t retVal = chMBFetchTimeout(&filled_buffers, (msg_t *)&pbuf, TIME_INFINITE);
        if(retVal == MSG_OK) {
            uartSendTimeout(uartp, &(pbuf->cnt), pbuf->data, TIME_INFINITE);

            // return the buffer
            msg_free((uint8_t*)pbuf);
        }
    }
}





void serialThread_ini(void) {
    /* Creating the mailboxes.*/
    chMBObjectInit(&filled_buffers, filled_buffers_queue, NUM_BUFF);
    // chThdCreateStatic(waThreadSerial, sizeof(waThreadSerial), NORMALPRIO, ThreadSerial, &UARTD1);

    uartStart(&UARTD1, &uartGpsConfig);
    uartStart(&UARTD2, &uartConfig);

    chThdCreateStatic(waThreadU1Tx, sizeof(waThreadU1Tx), NORMALPRIO, ThreadUTx, &UARTD2);
    chThdCreateStatic(waThreadU1Rx, sizeof(waThreadU1Rx), NORMALPRIO, ThreadURx, &UARTD1);
}



/*
 * Application entry point.
 */
int main(void) {

    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    chSysInit();

    // must do early as it is used by logger and rest of system.
    msg_init();
    
    // ini the logging system early on.
    log_init(LOG_ALL);
    log_swoInit();

    // config
    // matrix mapping stuff
    


    inThread_ini();
    modemThread_ini();
    outThread_ini();
    serialThread_ini();
    
    // testThread_ini();

    uint32_t cnt=0;

    while (true) {

        uint8_t* rxMsg;
        msg_alloc((uint8_t *)&rxMsg);

        if (!palReadPad(GPIOC, GPIOC_BUTTON)) {

            log_msg(LOG_ALL, "button pressed: %X", cnt);

            rxMsg[0] = cnt++;
   
            chMBPostTimeout(&txMailbox, (msg_t)rxMsg, TIME_INFINITE);
        }
        else{
            rxMsg[0] = 0xAA;
            chMBPostTimeout(&txMailbox, (msg_t)rxMsg, TIME_INFINITE);
        }
        chThdSleepMilliseconds(100);
    }
}
