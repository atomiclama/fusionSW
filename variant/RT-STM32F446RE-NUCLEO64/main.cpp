#include "ch.h"
#include "hal.h"

#include "msg_core.h"
#include "log_core.h"
#include "log_serial.h"
#include "log_swo.h"
#include "inThread.h"
#include "modemThread.h"
#include "outThread.h"
#include "serialThread.h"
#include "convThread.h"
#include "main.h"
#include "proto.h"


// board specific
DigitalOut led(GPIOA, 5);

SX126x radio1Hw(&SPID2);  
SX126x radio2Hw(&SPID2);

DigitalOut radio1Nss(GPIOC, 3);
DigitalIn  radio1Busy(GPIOC, 0);

DigitalOut radio2Nss(GPIOB, 4);
DigitalIn  radio2Busy(GPIOB, 5);


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
    map_init();

    inThread_ini();
    modemThread_ini();
    outThread_ini();
    serialThread_ini();
    convThread_ini();
 
    mailbox_t* txMailbox = map_getMailbox(Clitx);

    while (true) { 
        if(txMailbox) {
            radioPacket_t* txMsg;
            msg_alloc((uint8_t *)&txMsg);
            txMsg->data[0] = 0xAA;
            chMBPostTimeout(txMailbox, (msg_t)txMsg, TIME_INFINITE);
        }
        chThdSleepMilliseconds(50);
    }
}
