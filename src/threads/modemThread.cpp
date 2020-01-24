#include "ch.h"
#include "hal.h"

#include "ccportab.h"

#include "modemThread.h"
#include "msg_core.h"

#include "main.h"
#include "proto.h"

typedef struct {
    SX126x * radio;
    map_e idRx;
    map_e idTx;
}radioThreadCfg_s;

#if defined USE_RADIO1    
static THD_WORKING_AREA(radio1Threadwa, 1024);
const radioThreadCfg_s configR1 = {
    .radio = &radio1Hw,
    #ifdef USE_DEBUG_RADIO
    .idRx = None,
    #else
    .idRx = R1rx,
    #endif
    .idTx = R1tx,
};
#endif

#if defined USE_RADIO2
static THD_WORKING_AREA(radio2Threadwa, 1024);
const radioThreadCfg_s configR2 = {
    .radio = &radio2Hw,
    .idRx = R1rx,
    .idTx = None,
};
#endif

// Should give me -120dB sens
// ~25Km link budget for 0dBm tx power. Yeah right!
loraConfig config(LORA_BW_500, LORA_CR_4_5, LORA_SF7);


static THD_FUNCTION( radioThread, arg) {

    SX126x* radio = ((radioThreadCfg_s*)arg)->radio;
    map_e mapTx = ((radioThreadCfg_s *)arg)->idTx;
    map_e mapRx = ((radioThreadCfg_s *)arg)->idRx;

    mailbox_t* txMailbox = map_getMailbox(mapTx); 
    mailbox_t* rxMailbox = map_getMailbox(mapRx);
    mailbox_t* statusMailbox = map_getMailbox(Clirx);

    config.tcxo = USE_TXCO;
    radio->init(config);
    radioPacket_t* txMsg;
    radioState_t state = SETUP_;       
    uint16_t crcErrorCnt = 0; 
      
    while(rxMailbox || txMailbox) {
        // poll for reception by read regs to look for packet.
        chThdSleepMilliseconds(1);
        uint16_t status = radio->GetIrqStatus();

        switch(state) {
            case SETUP_: {
                if(txMailbox) {
                    uint8_t retVal = chMBFetchTimeout(txMailbox, (msg_t*)&txMsg, 1);
                    
                    if(retVal == MSG_OK) {
                        radio->SetStandby(STDBY_XOSC);
                        radio->WriteBuffer( 0x00, txMsg->data, 32 );
                        radio->SetDioIrqParams(IRQ_TX_DONE, 0, 0, 0);
                        radio->ClearIrqStatus(IRQ_RADIO_ALL);
                        radio->SetTx(0);  
                        state = TX_;
                        break;
                    }
                }
                if(rxMailbox) {
                    // rx a single packet then goto standby
                    radio->SetDioIrqParams(IRQ_RX_DONE|IRQ_TX_DONE|IRQ_PREAMBLE_DETECTED|IRQ_HEADER_VALID|IRQ_HEADER_ERROR|IRQ_CRC_ERROR, 0, 0, 0);
                    radio->ClearIrqStatus(IRQ_RADIO_ALL);
                    radio->SetRxBoosted(0); 
                    state = WAIT_;
                }
                break;
            }
            case WAIT_: {
                if (status & (IRQ_PREAMBLE_DETECTED|IRQ_HEADER_VALID))  {
                    state = RX_;
                    break;
                }
                if(txMailbox) {
                    uint8_t retVal = chMBFetchTimeout(txMailbox, (msg_t*)&txMsg, 1);
                    
                    if(retVal == MSG_OK) {
                        radio->SetStandby(STDBY_XOSC);
                        radio->WriteBuffer( 0x00, txMsg->data, 32 );
                        radio->SetDioIrqParams(IRQ_TX_DONE, 0, 0, 0);
                        radio->ClearIrqStatus(IRQ_RADIO_ALL);
                        radio->SetTx(0);  
                        state = TX_;
                        break;
                    }
                }
                break;
            }
            case RX_: {
                if(status & IRQ_CRC_ERROR) {
                    crcErrorCnt++;
                    state = SETUP_;
                    break;
                }
                if (status & IRQ_RX_DONE ) {
                    radioPacket_t* rxMsg;
                    msg_alloc((uint8_t *)&rxMsg);
                    rxMsg->stamp = chVTGetSystemTime();
                    radio->ReadBuffer( 0, rxMsg->data, 32 );
                    rxMsg->cnt = 32;
                    
                    // msg produced
                    // if it does not post straight away then drop it.
                    if(chMBPostTimeout(rxMailbox, (msg_t)rxMsg, TIME_IMMEDIATE) != MSG_OK){
                        msg_free((uint8_t*)rxMsg);
                    }
                    
                    radioPacket_t* statusMsg;
                    // get another msg
                    msg_alloc((uint8_t *)&statusMsg);
                    PacketStatus_t *status = (PacketStatus_t*)statusMsg->data;
                    radio->GetPacketStatus(status);
                    statusMsg->snr = status->Params.LoRa.SnrPkt;
                    statusMsg->rssi = status->Params.LoRa.RssiPkt;
                    // statusMsg->sig = status->Params.LoRa.SignalRssiPkt;
                    statusMsg->dbm = crcErrorCnt;
                    // if it does not post straight away then drop it.
                    if(chMBPostTimeout(statusMailbox, (msg_t)statusMsg, TIME_IMMEDIATE) != MSG_OK){
                        msg_free((uint8_t*)statusMsg);
                    }
      
                
                    state = SETUP_;
                    break;
                }
                break;
            }
            case TX_: {
                if (status & IRQ_TX_DONE ) {
                    // msg consumed
                    msg_free((uint8_t*)txMsg);
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

void modemThread_ini(void) {

#if defined USE_RADIO1    
    // connect to IO  
    radio1Hw.nssPin = &radio1Nss;
    radio1Hw.busyPin = &radio1Busy;
    chThdCreateStatic(radio1Threadwa, sizeof(radio1Threadwa), NORMALPRIO, radioThread, (void*)&configR1);
#endif

#if defined USE_RADIO2
    // connect to IO   
    radio2Hw.nssPin = &radio2Nss;
    radio2Hw.busyPin = &radio2Busy;
    chThdCreateStatic(radio2Threadwa, sizeof(radio2Threadwa), NORMALPRIO, radioThread, (void*)&configR2);
#endif

}
