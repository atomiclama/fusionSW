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

const SerialConfig serialConfig = {
    .speed = 115200,
    .cr1 = 0,                  
    .cr2 = USART_CR2_STOP1_BITS,
    .cr3 = 0,
  };


#define NUM_BUFF 10

static msg_t filled_buffers_queue[NUM_BUFF];
static mailbox_t filled_buffers;

static THD_WORKING_AREA(waThreadSerial, 256);

static THD_FUNCTION( ThreadSerial, arg) {
    
    SerialDriver *sdp = (SerialDriver *)arg;
    sdStart(sdp, &serialConfig);
    while(true) {
        // chThdSleepMilliseconds(10);
        uint8_t *pbuf;
 
        /* get an empty buffer.*/
        msg_alloc((uint8_t *)&pbuf);
        uint8_t tmp;
        // flush read buffer and sync to start of packet by looking for a gap, of at least 1ms.
        while( sdReadTimeout(sdp, &tmp, 1, TIME_MS2I(1))) {
            ;
        }
        while(sdReadTimeout(sdp, pbuf, 32, TIME_MS2I(20)) == 0);
        // post to full buffer
        (void)chMBPostTimeout(&filled_buffers, (msg_t)pbuf, TIME_INFINITE);

        // get from full buffer
        chMBFetchTimeout(&filled_buffers, (msg_t *)&pbuf, TIME_INFINITE);
        sdWrite(sdp, pbuf, 32);

        // return the buffer
        msg_free(pbuf);
    }
}

void serialThread_ini(void) {
    /* Creating the mailboxes.*/
    chMBObjectInit(&filled_buffers, filled_buffers_queue, NUM_BUFF);
    chThdCreateStatic(waThreadSerial, sizeof(waThreadSerial), NORMALPRIO, ThreadSerial, &SD1);
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

    // start and register a serial logger.
    // this stopped working after I did the low level uart2 AF pin slection
    // sdStart(&SD2, NULL);
    // log_serialInit((BaseSequentialStream *)&SD2);

    log_swoInit();

    // and now the file

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
