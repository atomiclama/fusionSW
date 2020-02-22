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
#include "serialThread.h"
#include "convThread.h"
#include "main.h"
#include "proto.h"


// board specific
DigitalOut led(GPIOA, 8);

SX126x radio1Hw(&SPID2);  
SX126x radio2Hw(&SPID2);

DigitalOut radio1Nss(GPIOC, 13);
DigitalIn  radio1Busy(GPIOC, 14);

DigitalOut radio2Nss(GPIOC, 15);
DigitalIn  radio2Busy(GPIOB, 12);

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

    inThread_init();
    modemThread_ini();
    outThread_ini();
    serialThread_ini();
    convThread_ini();
    
    // get same mailbox that rx would use just to send some periodic data.
    // mailbox_t* txMailbox = map_getMailbox(R1rx);

    while (true) {    
        chThdSleepMilliseconds(100);
    }
}
