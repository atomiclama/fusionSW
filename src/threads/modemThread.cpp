#include "ch.h"
#include "hal.h"

#include "ccportab.h"

#include "modemThread.h"
#include "msg_core.h"

#include "main.h"

static THD_WORKING_AREA(waThread3, 512);
static THD_WORKING_AREA(waThread4, 512);

static THD_WORKING_AREA(radio1Threadwa, 512);
static THD_WORKING_AREA(radio2Threadwa, 512);

#define NUM_BUFFERS 5

// data to be transmitted gets queued through here.
mailbox_t txMailbox; 
msg_t     txMailboxBuffer[NUM_BUFFERS];

// any data recieved come through here.
mailbox_t rxMailbox;
msg_t     rxMailboxBuffer[NUM_BUFFERS];

// Should give me -120dB sens
// ~25Km link budget for 0dBm tx power. Yeah right!
loraConfig config(LORA_BW_500, LORA_CR_4_5, LORA_SF9);


static THD_FUNCTION( radioThread, arg) {
    SX126x * radio = (SX126x *) arg;

    config.tcxo = true;
    radio->init(config);
    uint8_t* txMsg;
    radioState_t state = SETUP_;        
      
    while(true) {
        // poll for reception by read regs to look for packet.
        chThdSleepMilliseconds(1);
        uint16_t status = radio->GetIrqStatus();

        switch(state) {
            case SETUP_: {
                // rx a single packet then goto standby
                radio->SetDioIrqParams(IRQ_RX_DONE|IRQ_TX_DONE|IRQ_PREAMBLE_DETECTED|IRQ_HEADER_VALID|IRQ_HEADER_ERROR, 0, 0, 0);
                radio->ClearIrqStatus(IRQ_RADIO_ALL);
                radio->SetRxBoosted(0); 
                state = WAIT_;
                break;
            }
            case WAIT_: {
                if (status & (IRQ_PREAMBLE_DETECTED|IRQ_HEADER_VALID))  {
                    state = RX_;
                    break;
                }
                break;
            }
            case RX_: {
                if (status & IRQ_RX_DONE ) {
                    radioPacket_t* rxMsg;
                    msg_alloc((uint8_t *)&rxMsg);
                    radio->ReadBuffer( 0, rxMsg->data, 10 );
                    PacketStatus_t status;
                    radio->GetPacketStatus(&status);
                    rxMsg->snr = status.Params.LoRa.SnrPkt;
                    rxMsg->rssi = status.Params.LoRa.RssiPkt;
                    rxMsg->sig = status.Params.LoRa.SignalRssiPkt;
                    // msg produced
                    (void)chMBPostTimeout(&rxMailbox, (msg_t)rxMsg, TIME_INFINITE);

                    state = SETUP_;
                    break;
                }
                break;
            }
            case TX_: {
                if (status & IRQ_TX_DONE ) {
                    // msg consumed
                    msg_free(txMsg);
                    state = SETUP_;
                    break;
                }
                break;
            }
            default: {
                break;
            }
        }
    }
}



static THD_FUNCTION( radioThreadOld, arg) {
    SX126x * radio = (SX126x *) arg;
    radio->init(config);

    while(true) {
        
        // rx a single packet then goto standby
        radio->SetDioIrqParams(IRQ_RX_DONE, 0, 0, 0);
        radio->ClearIrqStatus(IRQ_RADIO_ALL);
        radio->SetRx(0); 

        while (true) {
            // get with timeout a message to tx
            // use this to time the loop for polling
            uint8_t* txMsg;
            uint8_t retVal = chMBFetchTimeout(&txMailbox, (msg_t*)&txMsg, 1);
            
            if(retVal == MSG_OK) {

                radio->WriteBuffer( 0x00, txMsg, 10 );
                radio->SetDioIrqParams(IRQ_TX_DONE, 0, 0, 0);
                radio->SetTx(0);  
                // wait for completion
                while(true){
                    chThdSleepMilliseconds(1);
                    volatile uint16_t status = radio->GetIrqStatus();
                    if (status & IRQ_TX_DONE ) {
                        break;
                    }
                }
                
                // msg consumed
                msg_free(txMsg);
                break;
            }

            // poll for reception by read regs to look for packet.
            uint16_t status = radio->GetIrqStatus();
        
            // todo Look into why this is getting triggered when nothing is being TX
            if (status & IRQ_RX_DONE ) {
                uint8_t* rxMsg;
                msg_alloc((uint8_t *)&rxMsg);
                radio->ReadBuffer( 0, rxMsg, 10 );
                // msg produced
                (void)chMBPostTimeout(&rxMailbox, (msg_t)rxMsg, TIME_INFINITE);
                break;
            }
        }
    }
}

static THD_FUNCTION( Thread3, arg) {

    (void) arg;
    radio1Hw.init(config);
    // any old junk to tx
    uint8_t tx[10] = {0,0,1,2,3,4,5,6,7,8};

    // connect to IO  
    radio1Hw.nssPin = &radio1Nss;
    radio1Hw.busyPin = &radio1Busy;

    radio1Hw.init(config);

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

    radio2Hw.init(config);

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

    chMBObjectInit(&rxMailbox, rxMailboxBuffer, NUM_BUFFERS);
    chMBObjectInit(&txMailbox, txMailboxBuffer, NUM_BUFFERS);

#if defined USE_DEBUG_RADIO
    chThdCreateStatic(waThread3, sizeof(waThread3), NORMALPRIO, Thread3, NULL);
    chThdCreateStatic(waThread4, sizeof(waThread4), NORMALPRIO, Thread4, NULL);
#endif

#if defined USE_RADIO1    
    // connect to IO  
    radio1Hw.nssPin = &radio1Nss;
    radio1Hw.busyPin = &radio1Busy;
    chThdCreateStatic(radio1Threadwa, sizeof(radio1Threadwa), NORMALPRIO, radioThread, &radio1Hw);
#endif

#if defined USE_RADIO2
    // connect to IO   
    radio2Hw.nssPin = &radio2Nss;
    radio2Hw.busyPin = &radio2Busy;
    chThdCreateStatic(radio2Threadwa, sizeof(radio2Threadwa), NORMALPRIO, radioThread, &radio2Hw);
#endif

}
