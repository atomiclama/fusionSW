#include "ch.h"
#include "hal.h"
//#include "rt_test_root.h"
//#include "oslib_test_root.h"

#include "log_core.h"
#include "log_serial.h"
#include "inThread.h"
#include "modemThread.h"
#include "outThread.h"

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

    // ini the logging system early on.
    log_init(LOG_ALL);

    // start and register a serial logger.
    sdStart(&SD2, NULL);
    log_serialInit((BaseSequentialStream *)&SD2);

    // and now the file



    inThread_ini();
    modemThread_ini();
    outThread_ini();


    while (true) {
        if (!palReadPad(GPIOC, GPIOC_BUTTON)) {
            // Handle power down
            // wait for all threads to complete
            log_msg(LOG_ALL, "button pressed");
        }
        chThdSleepMilliseconds(500);
    }
}
