#include "ch.h"
#include "hal.h"

#include "ccportab.h"


#include "spi.h"
#include "sx126x.h"

#include "modemThread.h"


// 2 radio devices share the same spi bus.

 SX126x radio1Hw(&SPID2);  
 SX126x radio2Hw(&SPID2);

static DigitalOut radio1Nss(GPIOC, 3);
static DigitalIn radio1Busy(GPIOC, 0);

static DigitalOut radio2Nss(GPIOB, 4);
static DigitalIn radio2Busy(GPIOB, 5);

static THD_WORKING_AREA(waThread3, 512);
static THD_WORKING_AREA(waThread4, 512);


#define NUM_BUFFERS 5

 mailbox_t txMailbox; 
 msg_t     txMailboxBuffer[NUM_BUFFERS];
 
 mailbox_t rxMailbox;
 msg_t     rxMailboxBuffer[NUM_BUFFERS];


static THD_FUNCTION( Thread3, arg) {

    (void) arg;
    // any old junk to tx
    uint8_t tx[10] = {0,0,1,2,3,4,5,6,7,8};

    // connect to IO  
    radio1Hw.nssPin = &radio1Nss;
    radio1Hw.busyPin = &radio1Busy;

    radio1Hw.init();

    chThdSleepMilliseconds(10);

    while (true) {

        msg_t msgData;
 
        // wait for something to tx ignore response for now
        (void)chMBFetchTimeout(&txMailbox, &msgData, TIME_INFINITE);

        tx[0] = msgData;
        radio1Hw.WriteBuffer( 0x00, tx, 10 );
        radio1Hw.SetTx(0);  
        chThdSleepMilliseconds(20);
    }
}

 

static THD_FUNCTION( Thread4, arg) {

    (void) arg;

    // junk received.
    uint8_t rx2[10];
      
    // connect to IO   
    radio2Hw.nssPin = &radio2Nss;
    radio2Hw.busyPin = &radio2Busy;

    radio2Hw.init();

    chThdSleepMilliseconds(10);

    while (true) {
        // rx a single packet then goto standby
        radio2Hw.SetDioIrqParams(IRQ_RX_DONE, 0, 0, 0);
        radio2Hw.ClearIrqStatus(IRQ_RADIO_ALL);
        radio2Hw.SetRx(0); 
        
        while(true) {
            chThdSleepMilliseconds(1);
            // read regs to look for packet.
            uint16_t status = radio2Hw.GetIrqStatus();
        
            if (status & IRQ_RX_DONE ) {
                break;
            }            
        }
        // read buffer
        radio2Hw.ReadBuffer( 0, rx2, 10 );
        // post message
        msg_t msgData = rx2[0];
        
        (void)chMBPostTimeout(&rxMailbox, msgData, TIME_INFINITE);
        // chThdSleepMilliseconds(50);

    }
}

void modemThread_ini(void) {

    palSetPadMode(GPIOC, 1, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOC, 2, PAL_MODE_ALTERNATE(5));
    palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(5));

    chMBObjectInit(&rxMailbox, rxMailboxBuffer, NUM_BUFFERS);
    chMBObjectInit(&txMailbox, txMailboxBuffer, NUM_BUFFERS);

    chThdCreateStatic(waThread3, sizeof(waThread3), NORMALPRIO, Thread3, NULL);
    chThdCreateStatic(waThread4, sizeof(waThread4), NORMALPRIO, Thread4, NULL);
}
