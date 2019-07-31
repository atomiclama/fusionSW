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

#pragma once

#include <stdbool.h>
#include <stdint.h>


//####### RADIOLINK RF POWER (beacon is always 100/13/1.3mW) #######
// 7 == 100mW (or 1000mW with M3)
// 6 == 50mW (use this when using booster amp), (800mW with M3)
// 5 == 25mW
// 4 == 13mW
// 3 == 6mW
// 2 == 3mW
// 1 == 1.6mW
// 0 == 1.3mW
#define BINDING_POWER     0x02
#define MAX_POWER 7
#define BINDING_VERSION   9
#define BIND_MAGIC (0xDEC1BE15 + BINDING_VERSION)

#define MIN_RFM_FREQUENCY 413000000
#define MAX_RFM_FREQUENCY 463000000
#define DEFAULT_CARRIER_FREQUENCY 435000000  // Hz  (ch 0)
#define DEFAULT_CHANNEL_SPACING 5
#define BINDING_FREQUENCY 435000000 // Hz
#define DIVERSITY_ENABLED   0x80


#define RF22B_Rx_packet_received_interrupt   0x02

#define RFM22_MAX_PAYLOAD_SIZE 32

//this needs to goto an enum
#define Available 0
#define Transmit 1
#define Transmitted 2
#define Receive 3
#define Received 4

//#define BV(x) (1<<(x)) // bit value

//// Register map of RFM22
//enum {
//    RFM22_00_CONFIG      = 0x00,
//    RFM22_01_EN_AA       = 0x01, // Auto Acknowledge
//    RFM22_02_EN_RXADDR   = 0x02,
//    RFM22_03_SETUP_AW    = 0x03, // Address Width
//    RFM22_04_SETUP_RETR  = 0x04, // automatic RETRansmission
//    RFM22_05_RF_CH       = 0x05, // RF CHannel
//    RFM22_06_RF_SETUP    = 0x06,
//    RFM22_07_STATUS      = 0x07,
//    RFM22_08_OBSERVE_TX  = 0x08,
//    RFM22_09_RPD         = 0x09, //Received Power Detector in the nRF23L01+, called CD (Carrier Detect) in the RFM22
//    RFM22_0A_RX_ADDR_P0  = 0x0A,
//    RFM22_0B_RX_ADDR_P1  = 0x0B,
//    RFM22_0C_RX_ADDR_P2  = 0x0C,
//    RFM22_0D_RX_ADDR_P3  = 0x0D,
//    RFM22_0E_RX_ADDR_P4  = 0x0E,
//    RFM22_0F_RX_ADDR_P5  = 0x0F,
//    RFM22_10_TX_ADDR     = 0x10,
//    RFM22_11_RX_PW_P0    = 0x11, // Payload Width
//    RFM22_12_RX_PW_P1    = 0x12,
//    RFM22_13_RX_PW_P2    = 0x13,
//    RFM22_14_RX_PW_P3    = 0x14,
//    RFM22_15_RX_PW_P4    = 0x15,
//    RFM22_16_RX_PW_P5    = 0x16,
//    RFM22_17_FIFO_STATUS = 0x17,
//    RFM22_1C_DYNPD       = 0x1C, // DYNamic PayloaD
//    RFM22_1D_FEATURE     = 0x1D
//};
//
//// Bit position mnemonics
//enum {
//    RFM22_00_CONFIG_MASK_RX_DR       = 6,
//    RFM22_00_CONFIG_MASK_TX_DS       = 5,
//    RFM22_00_CONFIG_MASK_MAX_RT      = 4,
//    RFM22_00_CONFIG_EN_CRC           = 3,
//    RFM22_00_CONFIG_CRCO             = 2,
//    RFM22_00_CONFIG_PWR_UP           = 1,
//    RFM22_00_CONFIG_PRIM_RX          = 0,
//
//    RFM22_01_EN_AA_ENAA_P5           = 5,
//    RFM22_01_EN_AA_ENAA_P4           = 4,
//    RFM22_01_EN_AA_ENAA_P3           = 3,
//    RFM22_01_EN_AA_ENAA_P2           = 2,
//    RFM22_01_EN_AA_ENAA_P1           = 1,
//    RFM22_01_EN_AA_ENAA_P0           = 0,
//
//    RFM22_02_EN_RXADDR_ERX_P5        = 5,
//    RFM22_02_EN_RXADDR_ERX_P4        = 4,
//    RFM22_02_EN_RXADDR_ERX_P3        = 3,
//    RFM22_02_EN_RXADDR_ERX_P2        = 2,
//    RFM22_02_EN_RXADDR_ERX_P1        = 1,
//    RFM22_02_EN_RXADDR_ERX_P0        = 0,
//
//    RFM22_06_RF_SETUP_RF_DR_LOW      = 5,
//    RFM22_06_RF_SETUP_RF_DR_HIGH     = 3,
//    RFM22_06_RF_SETUP_RF_PWR_HIGH    = 2,
//    RFM22_06_RF_SETUP_RF_PWR_LOW     = 1,
//
//    RFM22_07_STATUS_RX_DR            = 6,
//    RFM22_07_STATUS_TX_DS            = 5,
//    RFM22_07_STATUS_MAX_RT           = 4,
//
//    RFM22_17_FIFO_STATUS_TX_FULL     = 5,
//    RFM22_17_FIFO_STATUS_TX_EMPTY    = 4,
//    RFM22_17_FIFO_STATUS_RX_FULL     = 1,
//    RFM22_17_FIFO_STATUS_RX_EMPTY    = 0,
//
//    RFM22_1C_DYNPD_DPL_P5            = 5,
//    RFM22_1C_DYNPD_DPL_P4            = 4,
//    RFM22_1C_DYNPD_DPL_P3            = 3,
//    RFM22_1C_DYNPD_DPL_P2            = 2,
//    RFM22_1C_DYNPD_DPL_P1            = 1,
//    RFM22_1C_DYNPD_DPL_P0            = 0,
//
//    RFM22_1D_FEATURE_EN_DPL          = 2,
//    RFM22_1D_FEATURE_EN_ACK_PAY      = 1,
//    RFM22_1D_FEATURE_EN_DYN_ACK      = 0
//};
//
//// Pre-shifted and combined bits
//enum {
//    RFM22_01_EN_AA_ALL_PIPES         = 0x3F,
//
//    RFM22_02_EN_RXADDR_ERX_ALL_PIPES = 0x3F,
//
//    RFM22_03_SETUP_AW_3BYTES         = 0x01,
//    RFM22_03_SETUP_AW_4BYTES         = 0x02,
//    RFM22_03_SETUP_AW_5BYTES         = 0x03,
//
//    RFM22_04_SETUP_RETR_ARD_250us    = 0x00,
//    RFM22_04_SETUP_RETR_ARD_500us    = 0x10,
//    RFM22_04_SETUP_RETR_ARD_750us    = 0x20,
//    RFM22_04_SETUP_RETR_ARD_1000us   = 0x30,
//    RFM22_04_SETUP_RETR_ARD_1250us   = 0x40,
//    RFM22_04_SETUP_RETR_ARD_1500us   = 0x50,
//    RFM22_04_SETUP_RETR_ARD_1750us   = 0x60,
//    RFM22_04_SETUP_RETR_ARD_2000us   = 0x70,
//    RFM22_04_SETUP_RETR_ARD_2250us   = 0x80,
//    RFM22_04_SETUP_RETR_ARD_2500us   = 0x90,
//    RFM22_04_SETUP_RETR_ARD_2750us   = 0xa0,
//    RFM22_04_SETUP_RETR_ARD_3000us   = 0xb0,
//    RFM22_04_SETUP_RETR_ARD_3250us   = 0xc0,
//    RFM22_04_SETUP_RETR_ARD_3500us   = 0xd0,
//    RFM22_04_SETUP_RETR_ARD_3750us   = 0xe0,
//    RFM22_04_SETUP_RETR_ARD_4000us   = 0xf0,
//
//    RFM22_04_SETUP_RETR_ARC_0        = 0x00,
//    RFM22_04_SETUP_RETR_ARC_1        = 0x01,
//    RFM22_04_SETUP_RETR_ARC_2        = 0x02,
//    RFM22_04_SETUP_RETR_ARC_3        = 0x03,
//    RFM22_04_SETUP_RETR_ARC_4        = 0x04,
//    RFM22_04_SETUP_RETR_ARC_5        = 0x05,
//    RFM22_04_SETUP_RETR_ARC_6        = 0x06,
//    RFM22_04_SETUP_RETR_ARC_7        = 0x07,
//    RFM22_04_SETUP_RETR_ARC_8        = 0x08,
//    RFM22_04_SETUP_RETR_ARC_9        = 0x09,
//    RFM22_04_SETUP_RETR_ARC_10       = 0x0a,
//    RFM22_04_SETUP_RETR_ARC_11       = 0x0b,
//    RFM22_04_SETUP_RETR_ARC_12       = 0x0c,
//    RFM22_04_SETUP_RETR_ARC_13       = 0x0d,
//    RFM22_04_SETUP_RETR_ARC_14       = 0x0e,
//    RFM22_04_SETUP_RETR_ARC_15       = 0x0f,
//
//
//    RFM22_06_RF_SETUP_RF_DR_2Mbps    = 0x08,
//    RFM22_06_RF_SETUP_RF_DR_1Mbps    = 0x00,
//    RFM22_06_RF_SETUP_RF_DR_250Kbps  = 0x20,
//    RFM22_06_RF_SETUP_RF_PWR_n18dbm  = 0x01,
//    RFM22_06_RF_SETUP_RF_PWR_n12dbm  = 0x02,
//    RFM22_06_RF_SETUP_RF_PWR_n6dbm   = 0x04,
//    RFM22_06_RF_SETUP_RF_PWR_0dbm    = 0x06,
//
//    RFM22_1C_DYNPD_ALL_PIPES         = 0x3F
//};
//
//
//uint8_t rfm22_WriteReg(uint8_t reg, uint8_t data);
//uint8_t rfm22_WriteRegisterMulti(uint8_t reg, const uint8_t *data, uint8_t length);
//uint8_t RFM22_WritePayload(const uint8_t *data, uint8_t length);
//uint8_t RFM22_WriteAckPayload(const uint8_t *data, uint8_t length, uint8_t pipe);
//uint8_t rfm22_ReadReg(uint8_t reg);
//uint8_t RFM22_ReadRegisterMulti(uint8_t reg, uint8_t *data, uint8_t length);
//uint8_t RFM22_ReadPayload(uint8_t *data, uint8_t length);

// Utility functions
//
//void RFM22_FlushTx(void);
//void RFM22_FlushRx(void);
//uint8_t RFM22_Activate(uint8_t code);
//
//void RFM22_SetupBasic(void);
//void RFM22_SetStandbyMode(void);
//void RFM22_SetRxMode(void);
//void RFM22_SetTxMode(void);
//void RFM22_ClearAllInterrupts(void);
//
//bool RFM22_ReadPayloadIfAvailable(uint8_t *data, uint8_t length);

void rfm22_initialise(void);

bool rfm22_isAlive(void);

void rfm22_setRxMode(void);
uint8_t rfm22_getRSSI(void);
void rfm22_setChannel(uint8_t ch);
void rfm22_setPower(uint8_t p);
void rfm22_setCarrierFrequency(uint32_t f);
uint16_t rfm22_getAFCC(void);
uint16_t rfm22_getMode(void);

void rfm22_setPayload(uint8_t* data, uint8_t size);
void rfm22_getPayload(uint8_t* data, uint8_t size);

void rfm22_setHeader(uint8_t header);
uint8_t rfm22_getHeader(void);
uint8_t rfm22_ReadReg(uint8_t reg);


