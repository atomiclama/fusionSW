/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <platform.h>


#include "time.h"

#include "build/build_config.h"

#include "bus_spi.h"
#include "io.h"
#include "io_impl.h"
#include "drivers/rx/rx_spi.h"
#include "rx/rx.h"
#include "rx/openlrs.h"
#include "rfm22.h"

#include "system.h"

static uint8_t ItStatus1;
static uint8_t ItStatus2;

static uint8_t volatile RF_Mode;

struct rfm22_modem_regs {
    uint32_t bps;
    uint8_t r_1c, r_1d, r_1e, r_20, r_21, r_22, r_23, r_24, r_25, r_2a, r_6e, r_6f, r_70, r_71, r_72;
} modem_params[] = {
    { 4800,   0x1a, 0x40, 0x0a, 0xa1, 0x20, 0x4e, 0xa5, 0x00, 0x1b, 0x1e, 0x27, 0x52, 0x2c, 0x23, 0x30 }, // 50000 0x00
    { 9600,   0x05, 0x40, 0x0a, 0xa1, 0x20, 0x4e, 0xa5, 0x00, 0x20, 0x24, 0x4e, 0xa5, 0x2c, 0x23, 0x30 }, // 25000 0x00
    { 19200,  0x06, 0x40, 0x0a, 0xd0, 0x00, 0x9d, 0x49, 0x00, 0x7b, 0x28, 0x9d, 0x49, 0x2c, 0x23, 0x30 }, // 25000 0x01
    { 57600,  0x05, 0x40, 0x0a, 0x45, 0x01, 0xd7, 0xdc, 0x03, 0xb8, 0x1e, 0x0e, 0xbf, 0x00, 0x23, 0x2e },
    { 125000, 0x8a, 0x40, 0x0a, 0x60, 0x01, 0x55, 0x55, 0x02, 0xad, 0x1e, 0x20, 0x00, 0x00, 0x23, 0xc8 },
};


struct rfm22_modem_regs bind_params =
    { 9600, 0x05, 0x40, 0x0a, 0xa1, 0x20, 0x4e, 0xa5, 0x00, 0x20, 0x24, 0x4e, 0xa5, 0x2c, 0x23, 0x30 };



#define RF22B_PWRSTATE_READY        0x03
#define RF22B_PACKET_SENT_INTERRUPT 0x04
#define RF22B_PWRSTATE_RX           0x07
#define RF22B_PWRSTATE_TX           0x0B


#define R_REGISTER    0x00
#define W_REGISTER    0x80
#define REGISTER_MASK 0x7F


static uint8_t rfm22_WriteReg(uint8_t reg, uint8_t data) {
    return rxSpiWriteCommand(W_REGISTER | (REGISTER_MASK & reg), data);
}

static uint8_t rfm22_WriteRegisterMulti(uint8_t reg, const uint8_t *data, uint8_t length) {
    return rxSpiWriteCommandMulti(W_REGISTER | ( REGISTER_MASK & reg), data, length);
}

 uint8_t rfm22_ReadReg(uint8_t reg) {
    return rxSpiReadCommand(R_REGISTER | (REGISTER_MASK & reg), 0xff);
}

 static uint8_t rfm22_ReadRegisterMulti(uint8_t reg, uint8_t *data, uint8_t length) {
     return rxSpiReadCommandMulti(R_REGISTER | (REGISTER_MASK & reg), 0xff, data,  length);
 }



static void setModemRegs(struct rfm22_modem_regs* r) {
    rfm22_WriteReg(0x1c, r->r_1c);
    rfm22_WriteReg(0x1d, r->r_1d);
    rfm22_WriteReg(0x1e, r->r_1e);
    rfm22_WriteReg(0x20, r->r_20);
    rfm22_WriteReg(0x21, r->r_21);
    rfm22_WriteReg(0x22, r->r_22);
    rfm22_WriteReg(0x23, r->r_23);
    rfm22_WriteReg(0x24, r->r_24);
    rfm22_WriteReg(0x25, r->r_25);
    rfm22_WriteReg(0x2a, r->r_2a);
    rfm22_WriteReg(0x6e, r->r_6e);
    rfm22_WriteReg(0x6f, r->r_6f);
    rfm22_WriteReg(0x70, r->r_70);
    rfm22_WriteReg(0x71, r->r_71);
    rfm22_WriteReg(0x72, r->r_72);
}


// header
uint8_t rfm22_getHeader(void){
    return rfm22_ReadReg(0x47);
}

void rfm22_setHeader(uint8_t header){
    rfm22_WriteReg(0x3a, header);
}


// change over to no packet header
// maybe 1 byte header to allow for the different packets.
// 1 sync byte
// fixed packet size
// no crc.
// do crc in SW use the initial polynomial as the identifier.

void rfm22_initRegisters(uint8_t isbind) {
    ItStatus1 = rfm22_ReadReg(0x03);   // read status, clear interrupt
    ItStatus2 = rfm22_ReadReg(0x04);

    rfm22_WriteReg(0x06, 0x00);    // disable interrupts
    rfm22_WriteReg(0x07, RF22B_PWRSTATE_READY); // disable lbd, wakeup timer, use internal 32768,xton = 1; in ready mode
    rfm22_WriteReg(0x09, 0x7f);   // c = 12.5p
    rfm22_WriteReg(0x0a, 0x05);     // nothing of use. osc clock output.

    // GPIO config
    rfm22_WriteReg(0x0b, 0x12);    // gpio 0 TX State
    rfm22_WriteReg(0x0c, 0x15);    // gpio 1 RX State
    rfm22_WriteReg(0x0d, 0xfd);    // gpio 2 VDD
    rfm22_WriteReg(0x0e, 0x00);    // gpio 0,1,2 NO OTHER FUNCTION.

    if (isbind) {
        setModemRegs(&modem_params[3]);
    } else {
        setModemRegs(&modem_params[bind_data.modem_params]);
    }

    // Packet settings
    rfm22_WriteReg(0x30, 0x8c);    // enable packet handler, msb first, enable crc,
    rfm22_WriteReg(0x32, 0x0f);    // no broadcast, check header bytes 3,2,1,0
    rfm22_WriteReg(0x33, 0x42);    // 4 byte header, 2 byte synch, variable pkt size
    rfm22_WriteReg(0x34, (bind_data.flags & DIVERSITY_ENABLED) ? 0x14 : 0x0a);    // 40 bit preamble, 80 with diversity
    rfm22_WriteReg(0x35, 0x2a);    // preath = 5 (20bits), rssioff = 2
    rfm22_WriteReg(0x36, 0x2d);    // synchronize word 3
    rfm22_WriteReg(0x37, 0xd4);    // synchronize word 2
    rfm22_WriteReg(0x38, 0x00);    // synch word 1 (not used)
    rfm22_WriteReg(0x39, 0x00);    // synch word 0 (not used)

    uint32_t magic = isbind ? BIND_MAGIC : bind_data.rf_magic;
    for (uint8_t i = 0; i < 4; i++) {
        rfm22_WriteReg(0x3a + i, (magic >> 24) & 0xff);   // tx header
        rfm22_WriteReg(0x3f + i, (magic >> 24) & 0xff);   // rx header
        magic = magic << 8; // advance to next byte
    }

    rfm22_WriteReg(0x43, 0x00);    // None of header 3 needs checking.
    rfm22_WriteReg(0x44, 0xff);    // all the bit to be checked
    rfm22_WriteReg(0x45, 0xff);    // all the bit to be checked
    rfm22_WriteReg(0x46, 0xff);    // all the bit to be checked


    if (isbind) {
        rfm22_setPower(BINDING_POWER);
    } else {
        rfm22_setPower(bind_data.rf_power);
    }

    rfm22_WriteReg(0x79, 0);

    rfm22_WriteReg(0x7a, bind_data.rf_channel_spacing);   // channel spacing

    rfm22_WriteReg(0x73, 0x00);
    rfm22_WriteReg(0x74, 0x00);    // no offset

    rfm22_setCarrierFrequency(isbind ? BINDING_FREQUENCY : bind_data.rf_frequency);
}



/*
 * Transfer the payload to the RFM22 TX FIFO
 * Packets in the TX FIFO are transmitted when the
 * RFM22 next enters TX mode
 */
//uint8_t RFM22_WritePayload(const uint8_t *data, uint8_t length) {
//    return rxSpiWriteCommandMulti(W_TX_PAYLOAD, data, length);
//}
//
//uint8_t RFM22_WriteAckPayload(const uint8_t *data, uint8_t length, uint8_t pipe) {
//    return rxSpiWriteCommandMulti(W_ACK_PAYLOAD | (pipe & 0x07), data, length);
//}
//
//uint8_t RFM22_ReadRegisterMulti(uint8_t reg, uint8_t *data, uint8_t length) {
//    return rxSpiReadCommandMulti(R_REGISTER | (REGISTER_MASK & reg), NOP, data, length);
//}

/*
 * Read a packet from the RFM22 RX FIFO.
 */
//uint8_t RFM22_ReadPayload(uint8_t *data, uint8_t length) {
//    return rxSpiReadCommandMulti(R_RX_PAYLOAD, NOP, data, length);
//}

/*
 * Empty the transmit FIFO buffer.
 */
//void RFM22_FlushTx(void) {
//    rxSpiWriteByte(FLUSH_TX);
//}

/*
 * Empty the receive FIFO buffer.
 */
//void RFM22_FlushRx(void) {
//    rxSpiWriteByte(FLUSH_RX);
//}

//uint8_t RFM22_Activate(uint8_t code) {
//    return rxSpiWriteCommand(ACTIVATE, code);
//}

// standby configuration, used to simplify switching between RX, TX, and Standby modes
//static uint8_t standbyConfig;

///*
// * Common setup of registers
// */
//void RFM22_SetupBasic(void) {
//    rfm22_WriteReg(RFM22_01_EN_AA, 0x00); // No auto acknowledgement
//    rfm22_WriteReg(RFM22_02_EN_RXADDR, (RFM22_02_EN_RXADDR_ERX_P0));
//    rfm22_WriteReg(RFM22_03_SETUP_AW, RFM22_03_SETUP_AW_5BYTES); // 5-byte RX/TX address
//    rfm22_WriteReg(RFM22_1C_DYNPD, 0x00); // Disable dynamic payload length on all pipes
//}

/*
 * Enter standby mode
 */
//void RFM22_SetStandbyMode(void) {
//    // set CE low and clear the PRIM_RX bit to enter standby mode
//    rfm22_CE_LO();
//    rfm22_WriteReg(RFM22_00_CONFIG, standbyConfig);
//}

///*
// * Enter receive mode
// */
//void RFM22_SetRxMode(void) {
//    RFM22_CE_LO(); // drop into standby mode
//    // set the PRIM_RX bit
//    rfm22_WriteReg(RFM22_00_CONFIG, standbyConfig | BV(RFM22_00_CONFIG_PRIM_RX));
//    RFM22_ClearAllInterrupts();
//    // finally set CE high to start enter RX mode
//    RFM22_CE_HI();
//    // RFM22+ will now transition from Standby mode to RX mode after 130 microseconds settling time
//}
//
///*
// * Enter transmit mode. Anything in the transmit FIFO will be transmitted.
// */
//void RFM22_SetTxMode(void) {
//    // Ensure in standby mode, since can only enter TX mode from standby mode
//    RFM22_SetStandbyMode();
//    RFM22_ClearAllInterrupts();
//    // pulse CE for 10 microseconds to enter TX mode
//    rfm22_CE_HI();
//    delayMicroseconds(10);
//    RFM22_CE_LO();
//    // RFM22+ will now transition from Standby mode to TX mode after 130 microseconds settling time.
//    // Transmission will then begin and continue until TX FIFO is empty.
//}

//void RFM22_ClearAllInterrupts(void) {
//    // Writing to the STATUS register clears the specified interrupt bits
//    rfm22_WriteReg(RFM22_07_STATUS, BV(RFM22_07_STATUS_RX_DR) | BV(RFM22_07_STATUS_TX_DS) | BV(RFM22_07_STATUS_MAX_RT));
//}

//bool RFM22_ReadPayloadIfAvailable(uint8_t *data, uint8_t length) {
//    if (rfm22_ReadReg(RFM22_17_FIFO_STATUS) & BV(RFM22_17_FIFO_STATUS_RX_EMPTY)) {
//        return false;
//    }
//    RFM22_ReadPayload(data, length);
//    return true;
//}

//#ifndef UNIT_TEST
//#define DISABLE_RX()    {IOHi(DEFIO_IO(RX_NSS_PIN));}
//#define ENABLE_RX()     {IOLo(DEFIO_IO(RX_NSS_PIN));}
///*
// * Fast read of payload, for use in interrupt service routine
// */
//bool RFM22_ReadPayloadIfAvailableFast(uint8_t *data, uint8_t length) {
//    // number of bits transferred = 8 * (3 + length)
//    // for 16 byte payload, that is 8*19 = 152
//    // at 50MHz clock rate that is approximately 3 microseconds
//    bool ret = false;
//    ENABLE_RX();
//    rxSpiTransferByte(R_REGISTER | (REGISTER_MASK & RFM22_07_STATUS));
//    const uint8_t status = rxSpiTransferByte(0xff);
//    if ((status & BV(RFM22_07_STATUS_RX_DR)) == 0) {
//        ret = true;
//        // clear RX_DR flag
//        rxSpiTransferByte(W_REGISTER | (REGISTER_MASK & RFM22_07_STATUS));
//        rxSpiTransferByte(BV(RFM22_07_STATUS_RX_DR));
//        rxSpiTransferByte(R_RX_PAYLOAD);
//        for (uint8_t i = 0; i < length; i++) {
//            data[i] = rxSpiTransferByte(0xff);
//        }
//    }
//    DISABLE_RX();
//    return ret;
//}
//#endif // UNIT_TEST
//#endif //

// Copied from openlrs ng.

// **** RFM22 access functions




static inline void clearFIFO() {
    //clear FIFO, disable multi packet
    rfm22_WriteReg(0x08, 0x03);
    rfm22_WriteReg(0x08, 0x00);

}

void rx_reset(void) {
    rfm22_WriteReg(0x07, RF22B_PWRSTATE_READY);
    rfm22_WriteReg(0x7e, 36);    // threshold for rx almost full, interrupt when 1 byte received
    clearFIFO();
    rfm22_WriteReg(0x07, RF22B_PWRSTATE_RX);   // to rx mode
    //rfm22_WriteReg(0x05, RF22B_Rx_packet_received_interrupt);
    ItStatus1 = rfm22_ReadReg(0x03);   //read the Interrupt Status1 register
    ItStatus2 = rfm22_ReadReg(0x04);
}

uint32_t tx_start = 0;



uint8_t tx_done() {
    if (RF_Mode == Transmitted) {

        RF_Mode = Available;
        return 1; // success
    }
    return 0;
}

// ISR routine
void RFM22B_Int() {
    if (RF_Mode == Transmit) {
        RF_Mode = Transmitted;
    }

    if (RF_Mode == Receive) {
        RF_Mode = Received;
    }
}

//public

void rfm22_initialise(void) {
    rfm22_initRegisters(1);
}


bool rfm22_isAlive(void) {
    return (rfm22_ReadReg(0x01) != 0);
}

void rfm22_setRxMode_old(void) {
    ItStatus1 = rfm22_ReadReg(0x03);
    ItStatus2 = rfm22_ReadReg(0x04);
    rfm22_WriteReg(0x07, RF22B_PWRSTATE_READY);
    delayMicroseconds(10);
    rx_reset();
    RF_Mode = Receive;
}

void rfm22_setRxMode(void) {
    //Cancel any ongoing TX/RX
    rfm22_WriteReg(0x07, RF22B_PWRSTATE_READY);
//    delayMicroseconds(10);
    clearFIFO();
    rfm22_WriteReg(0x07, RF22B_PWRSTATE_RX);
}


uint8_t rfm22_getRSSI(void) {
    return rfm22_ReadReg(0x26);
}

void rfm22_setChannel(uint8_t ch) {
    uint8_t magicLSB = (bind_data.rf_magic & 0xff) ^ ch;
    rfm22_WriteReg(0x79, bind_data.rf_channel[ch]);
    rfm22_WriteReg(0x3a + 3, magicLSB);
    rfm22_WriteReg(0x3f + 3, magicLSB);
}

void rfm22_setPower(uint8_t p) {
    if (p > MAX_POWER) {
        p = MAX_POWER;
    }
    rfm22_WriteReg(0x6d, p);
}

/**
 * @brief 
 * 
 * @param f 
 */
void rfm22_setCarrierFrequency(uint32_t f) {
    uint16_t fb, fc, hbsel;
    if (f < 480000000) {
        hbsel = 0;
        fb = f / 10000000 - 24;
        fc = (f - (fb + 24) * 10000000) * 4 / 625;
    } else {
        hbsel = 1;
        fb = f / 20000000 - 24;
        fc = (f - (fb + 24) * 20000000) * 2 / 625;
    }
    rfm22_WriteReg(0x75, 0x40 + (hbsel ? 0x20 : 0) + (fb & 0x1f));
    rfm22_WriteReg(0x76, (fc >> 8));
    rfm22_WriteReg(0x77, (fc & 0xff));
}

uint16_t rfm22_getMode(void) {
    return RF_Mode;
}

void rfm22_getPayload(uint8_t* data, uint8_t size) {
    // burst read of payload data
    rfm22_ReadRegisterMulti(0x7f, data, size);
//    while (size--) {
//        *(data++) = rfm22_ReadReg(0x7f);
//    }
}

void rfm22_setPayload(uint8_t* pkt, uint8_t size) {
    rfm22_WriteReg(0x07, RF22B_PWRSTATE_READY);

    rfm22_WriteReg(0x3e, size);   // total TX size

    // burst write of payload data
    rfm22_WriteRegisterMulti(0x7f, pkt, size);

//    for (uint8_t i = 0; i < size; i++) {
//        rfm22_WriteReg(0x7f, pkt[i]);
//    }

//    rfm22_WriteReg(0x05, RF22B_PACKET_SENT_INTERRUPT); don't use int
    // clear the status registers
    rfm22_ReadReg(0x02);
    rfm22_ReadReg(0x03);
    // transition to TX state
    rfm22_WriteReg(0x07, RF22B_PWRSTATE_TX);
}


