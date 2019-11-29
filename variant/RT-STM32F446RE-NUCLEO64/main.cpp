#include "ch.h"
#include "hal.h"
//#include "rt_test_root.h"
//#include "oslib_test_root.h"

#include "msg_core.h"
#include "log_core.h"
#include "log_serial.h"
#include "inThread.h"
#include "modemThread.h"
#include "outThread.h"

extern mailbox_t txMailbox; 

const SerialConfig serial_config = {
    .speed = 115200,
    .cr1 = 0,                  
    .cr2 = USART_CR2_STOP1_BITS,
    .cr3 = 0,
  };


#define NUM_BUFF 10
#define SIZE_BUFF 40

uint8_t buffer[NUM_BUFF][SIZE_BUFF];

static msg_t free_buffers_queue[NUM_BUFF];
static mailbox_t free_buffers;

static msg_t filled_buffers_queue[NUM_BUFF];
static mailbox_t filled_buffers;

static THD_WORKING_AREA(waThreadSerial, 256);

static THD_FUNCTION( ThreadSerial, arg) {
    
    SerialDriver *sdp = (SerialDriver *)arg;
    sdStart(sdp, &serial_config);

    while(true){
        // chThdSleepMilliseconds(10);
        void *pbuf;
 
        /* get an empty buffer.*/
        chMBFetchTimeout(&free_buffers, (msg_t *)&pbuf, TIME_INFINITE);
        uint8_t tmp;
        // flush read buffer and sync to start of packet by looking for a gap, of at least 1ms.
        while( sdReadTimeout(sdp, &tmp, 1, TIME_MS2I(1))) {
            ;
        }
        sdReadTimeout(sdp, (uint8_t*)pbuf, 32, TIME_MS2I(20));
        // post to full buffer
        (void)chMBPostTimeout(&filled_buffers, (msg_t)pbuf, TIME_INFINITE);

        // get from full buffer
        chMBFetchTimeout(&filled_buffers, (msg_t *)&pbuf, TIME_INFINITE);
        sdWrite(sdp, (uint8_t*)pbuf, SIZE_BUFF);

        // post to free buffer.
        (void)chMBPostTimeout(&free_buffers, (msg_t)pbuf, TIME_INFINITE);
    }
}

void serialThread_ini(void) {
    palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7)); // tx
    palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7)); // rx
        
    /* Creating the mailboxes.*/
    chMBObjectInit(&filled_buffers, filled_buffers_queue, NUM_BUFF);
    chMBObjectInit(&free_buffers, free_buffers_queue, NUM_BUFF);
 
    /* Pre-filling the free buffers pool with the available buffers, the post
        will not stop because the mailbox is large enough.*/
    for (uint8_t i = 0; i < NUM_BUFF; i++)
        (void)chMBPostTimeout(&free_buffers, (msg_t)&buffer[i], TIME_INFINITE);

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

    // must do early as it is used by logger and reset of system.
    msg_init();
    
    // ini the logging system early on.
    log_init(LOG_ALL);

    // start and register a serial logger.
    sdStart(&SD2, NULL);
    log_serialInit((BaseSequentialStream *)&SD2);

    // and now the file

    inThread_ini();
    modemThread_ini();
    outThread_ini();
    serialThread_ini();
    
    testThread_ini();

    uint32_t cnt=0;

    while (true) {
        if (!palReadPad(GPIOC, GPIOC_BUTTON)) {

            log_msg(LOG_ALL, "button pressed: %X", cnt);

            msg_t msgData = cnt++;
            chMBPostTimeout(&txMailbox, msgData, TIME_INFINITE);
        }
        else{
            msg_t msgData = 0xAA;
            chMBPostTimeout(&txMailbox, msgData, TIME_INFINITE);
        }
        chThdSleepMilliseconds(100);
    }
}
