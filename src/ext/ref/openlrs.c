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
#include <string.h>

#include <platform.h>



#include "common/utils.h"
#include "drivers/time.h"

#include "config/feature.h"

#include "build/build_config.h"

#include "drivers/system.h"
#include "drivers/io_types.h"
#include "../drivers/serial_bridge.h"

#include "rx/rx.h"
#include "rx/rx_spi.h"
#include "../drivers/rfm22.h"
#include "rx/openlrs.h"

#include "drivers/light_led.h"

enum {
    RATE_LOW = 0, RATE_MID = 1, RATE_HIGH = 2
};

enum {
    OLRS_RC = 5, OLRS_DATA
};

olrsConfig_t bind_data = { 1,
BINDING_FREQUENCY,
BIND_MAGIC,
BINDING_POWER,
DEFAULT_CHANNEL_SPACING, { 1, 2, 3, 4, 5 }, 0, 0 };

// local

typedef enum {
    STATE_BIND = 0,
    STATE_SYNC,
    STATE_PAUSE,
    STATE_SETUP_RX,
    STATE_CHECK_RX,
    STATE_TX,
    STATE_CHECK_TX,
    STATE_CHECK_PREAMBLE,
    STATE_TX_RCDATA
} protocol_state_t;

static protocol_state_t protocolState;

#define RC_CHANNEL_CNT 8
// 4 Channel 12bit, upper 4 bit for aux 4 channel
#define PACKET_SIZE 8

// update period in ms
#define UPDATE_PERIOD 100


// TODO these 2 need to go once we are up and running move over to the payload data that is passed in
static uint8_t rx_buf[PACKET_SIZE]; // RX buffer (uplink)
static uint8_t tx_buf[PACKET_SIZE]; // TX buffer (downlink)(type plus 8 x data)

#define DIVERSITY_ENABLED   0x80

static uint8_t RF_channel;
static uint8_t hopcount;

static uint8_t linkAcquired = 0;
static uint8_t numberOfLostPackets = 0;
static uint16_t linkQuality = 0;

static uint32_t lastPacketTimeUs = 0;
static uint32_t lastRSSITimeUs = 0;
static uint32_t lastRSSIvalue = 0;
static uint32_t linkLossTimeMs;
static uint16_t lastAFCCvalue = 0;

static uint32_t rssiAvg = 0;
static uint8_t failsafeActive = 0;

// openlrsGetRcDataFromPayload and openlrsSetRcDataFromPayload must mirror each other

//static void openlrsGetRcDataToPayload(uint8_t* toP, int16_t* rcData) {
//
//    memcpy(toP, (uint8_t*) rcData, RC_CHANNEL_CNT * 2);
//}

//void openlrsSetRcDataFromPayload(uint16_t *rcData, const uint8_t *payload) {
//
//    UNUSED(payload);
//    memcpy((uint8_t*) rcData, rx_buf, RC_CHANNEL_CNT * 2);
//    uint16_t temp = rcData[2];
//    rcData[2] = rcData[3];
//    rcData[3] = temp;
//}
#define STEP 1000/16


uint8_t toHigh4Bit(int16_t in) {
    uint8_t ret = 0xF0;
//    int16_t cmp = 2000;
//
//    for (uint8_t i = 0; i < 16; i++) {
//        cmp -= STEP;
//        if (in > cmp)
//            return ret;
//        ret -= 0x10;
//    }

    if (in > 1000) {
        uint32_t tmp = in - 1000;
        tmp *=16;
        tmp /= 1000;
        if(tmp > 0x0F)
            tmp = 0x0F;
        ret = tmp;// / STEP;
    } else {
        ret = 0;
    }


    return ret<<4;
}

// get data from internal and encode for transmission
static void openlrsGetRcDataToPayload(uint8_t* toP, int16_t* rcData) {
    // copy the first 4 channels across.
    memcpy(toP, (uint8_t*) rcData, 8);
    toP[1] |= toHigh4Bit(rcData[4]);
    toP[3] |= toHigh4Bit(rcData[5]);
    toP[5] |= toHigh4Bit(rcData[6]);
    toP[7] |= toHigh4Bit(rcData[7]);
}

uint16_t fromHigh4Bit(uint8_t in) {
    uint16_t ret;

    // get upper nibble.
    in >>= 4;
    ret = (in * STEP) + 1000;
    return ret;
}
// extract data from payload for internal
void openlrsSetRcDataFromPayload(uint16_t *rcData, const uint8_t *payload) {

    UNUSED(payload);
    memcpy((uint8_t*) rcData, rx_buf, 8);
    uint16_t temp = rcData[2];
    rcData[2] = rcData[3];
    rcData[3] = temp;

    rcData[0] &= 0xFFF;
    rcData[1] &= 0xFFF;
    rcData[2] &= 0xFFF;
    rcData[3] &= 0xFFF;

    rcData[4] = fromHigh4Bit(rx_buf[1]);
    rcData[5] = fromHigh4Bit(rx_buf[3]);
    rcData[6] = fromHigh4Bit(rx_buf[5]);
    rcData[7] = fromHigh4Bit(rx_buf[7]);
}


static void doReceive(void) {

    lastPacketTimeUs = micros();

    if (rfm22_getMode() == Received) {
        rfm22_getPayload(rx_buf, PACKET_SIZE);
    }

    numberOfLostPackets = 0;
    linkQuality <<= 1;
    linkQuality |= 1;

//    if ((rx_buf[0] & 0x3e) == 0x00) {
//
//        unpackChannels(bind_data.flags & 7, PPM, rx_buf + 1);
//
//        if (rx_buf[0] & 0x01) {
//            if (!fs_saved) {
//                for (int16_t i = 0; i < PPM_CHANNELS; i++) {
//                    if (!(failsafePPM[i] & 0x1000)) {
//                        failsafePPM[i] = servoBits2Us(PPM[i]);
//                    }
//                }
//                failsafeSave();
//                fs_saved = 1;
//            }
//        } else if (fs_saved) {
//            fs_saved = 0;
//        }
//    } else {
//        // something else than servo data...
//        if ((rx_buf[0] & 0x38) == 0x38) {
//            if ((rx_buf[0] ^ tx_buf[0]) & 0x80) {
//                // We got new data... (not retransmission)
//                uint8_t i;
//                tx_buf[0] ^= 0x80; // signal that we got it
//                if (rx_config.pinMapping[TXD_OUTPUT] == PINMAP_TXD) {
//                    for (i = 0; i <= (rx_buf[0] & 7);) {
//                        i++;
////                        Serial.write(rx_buf[i]);
//                    }
//                }
//            }
//        }
//    }

    if (linkAcquired == 0) {
        linkAcquired = 1;
    }

    failsafeActive = 0;

//    if (bind_data.flags & TELEMETRY_MASK) {
//        if ((tx_buf[0] ^ rx_buf[0]) & 0x40) {
//            // resend last message
//        } else {
//            tx_buf[0] &= 0xc0;
//            tx_buf[0] ^= 0x40; // swap sequence as we have new data
//            if (serial_head != serial_tail) {
//                uint8_t bytes = 0;
//                while ((bytes < 8) && (serial_head != serial_tail)) {
//                    bytes++;
//                    tx_buf[bytes] = serial_buffer[serial_head];
//                    serial_head = (serial_head + 1) % SERIAL_BUFSIZE;
//                }
//                tx_buf[0] |= (0x37 + bytes);
//            } else {
//                // tx_buf[0] lowest 6 bits left at 0
//                tx_buf[1] = lastRSSIvalue;
//                tx_buf[2] = 0;
//                tx_buf[3] = 0;
//                tx_buf[4] = (lastAFCCvalue >> 8);
//                tx_buf[5] = lastAFCCvalue & 0xff;
//                tx_buf[6] = countSetBits(linkQuality & 0x7fff);
//            }
//        }
//
//        tx_packet_async(tx_buf, 9);
//        protocolState = STATE_TX;
//    }
}
//
//static void doHop(uint8_t willhop){
//    if (willhop == 1) {
//        RF_channel++;
//
//        if ((RF_channel == MAXHOPS) || (bind_data.hopchannel[RF_channel] == 0)) {
//            RF_channel = 0;
//        }
//
//        if ((rx_config.beacon_frequency) && (nextBeaconTimeMs)) {
//            // Listen for RSSI on beacon channel briefly for 'trigger'
//            uint8_t brssi = beaconGetRSSI();
//            if (brssi > ((beaconRSSIavg >> 2) + 20)) {
//                nextBeaconTimeMs = millis() + 1000L;
//            }
//            beaconRSSIavg = (beaconRSSIavg * 3 + brssi * 4) >> 2;
//
//            rfm22_setCarrierFrequency(bind_data.rf_frequency);
//        }
//
//        rfm22_setChannel(RF_channel);
//    }
//}

/*
 * This is called periodically by the scheduler.
 * Returns RX_SPI_RECEIVED_DATA if a data packet was received.
 */
rx_spi_received_e openlrsDataReceived(uint8_t *payload) {

    UNUSED(payload);

    rx_spi_received_e ret = RX_SPI_RECEIVED_NONE;

    uint32_t timeUs, timeMs;
    static bool willhop = 0;

// don't think we need this.
//    if (!rfm22_isAlive()) {     // detect the locked module and reboot
//        rfm22_initialise();
//        rfm22_setRxMode();
//    }

    // scan rssi only during certain states / times
    switch (protocolState) {
        case STATE_CHECK_RX: {
            rssiAvg += rfm22_getRSSI();
            rssiAvg = rssiAvg > 2;
            break;
        }

        default: {
            break;
        }
    }

    // TODO General timing
    // missed packets
    // no packet at expected time
    // adjust rssi
    // time outs.
    // etc.

    // State transitions
    switch (protocolState) {

        case STATE_PAUSE: {
            if (millis() - lastPacketTimeUs > 500) {
                lastPacketTimeUs = millis();
                protocolState = STATE_TX;
            }
            break;
        }

        case STATE_BIND: {
            // do nothing as we are going through the bind procedure from the CLI
            break;
        }

        case STATE_SYNC: {
            break;
        }

        case STATE_SETUP_RX: {
            rfm22_setRxMode();
            protocolState = STATE_CHECK_RX;
            break;
        }

        case STATE_CHECK_PREAMBLE: {
            uint8_t pkt_len;
            if ((rfm22_ReadReg(0x04) & 0x80) != 0) {
                //                packet underway don't look for something to tx.

                protocolState = STATE_CHECK_RX;

            } else if ((pkt_len = serialbridge_getChunk(tx_buf, PACKET_SIZE)) > 0) {

                rfm22_setPayload(tx_buf, pkt_len);
                protocolState = STATE_CHECK_TX;
            }
            break;
        }

        case STATE_CHECK_RX: {
            uint8_t pkt_len;
            if ((rfm22_ReadReg(0x07) & 0x4) == 0) {
                if ((rfm22_ReadReg(0x03) & 0x2) != 0) {
                    DBG1_TOG;
                    lastPacketTimeUs = millis();
                    pkt_len = rfm22_ReadReg(0x4B);
                    if (pkt_len > PACKET_SIZE) {
                        pkt_len = PACKET_SIZE;
                    }
                    rfm22_getPayload(rx_buf, pkt_len);
                    if (rfm22_getHeader() == OLRS_DATA) {
                        serialBridge_setChunk(rx_buf, pkt_len);
                    } else if (rfm22_getHeader() == OLRS_RC) {
                        ret = RX_SPI_RECEIVED_DATA;
                    }
                }
                protocolState = STATE_SETUP_RX;
            }
            if ((pkt_len = serialbridge_getChunk(tx_buf, PACKET_SIZE)) > 0) {
                rfm22_setHeader(OLRS_DATA);
                rfm22_setPayload(tx_buf, pkt_len);
                protocolState = STATE_CHECK_TX;
            }
            if (feature(FEATURE_TX_SPI)) {
                if (millis() - lastPacketTimeUs > UPDATE_PERIOD) {
                    lastPacketTimeUs = millis();
                    protocolState = STATE_TX_RCDATA;
                }
            }
            break;
        }
        case STATE_TX: {
            uint8_t pkt_len;
            if ((pkt_len = serialbridge_getChunk(tx_buf, PACKET_SIZE)) > 0) {
                rfm22_setHeader(OLRS_DATA);
                rfm22_setPayload(tx_buf, pkt_len);
                protocolState = STATE_CHECK_TX;
            } else {
                protocolState = STATE_SETUP_RX;
            }
            break;
        }
        case STATE_CHECK_TX: {
            // timeout?

            // packet sent ?
            if ((rfm22_ReadReg(0x07) & 0x8) == 0) {

                protocolState = STATE_TX;
            }
            break;
        }
        case STATE_TX_RCDATA: {
            rfm22_setHeader(OLRS_RC);
            openlrsGetRcDataToPayload(tx_buf, rcRaw);
            rfm22_setPayload(tx_buf, PACKET_SIZE);
            protocolState = STATE_CHECK_TX;
            break;
        }
    }

//    // sample RSSI when packet is in the 'air'
//    if ((numberOfLostPackets < 2) && (lastRSSITimeUs != lastPacketTimeUs) && (timeUs - lastPacketTimeUs) > (getInterval(&bind_data) - 1500)) {
//        lastRSSITimeUs = lastPacketTimeUs;
//        lastRSSIvalue = rfm22_getRSSI(); // Read the RSSI value
//        RSSI_sum += lastRSSIvalue;    // tally up for average
//        RSSI_count++;
//
//        if (RSSI_count > 8) {
//            RSSI_sum /= RSSI_count;
//            smoothRSSI = (((uint16_t) smoothRSSI * 3 + (uint16_t) RSSI_sum * 1) / 4);
//            set_RSSI_output();
//            RSSI_sum = 0;
//            RSSI_count = 0;
//        }
//    }
//
//    if (linkAcquired) {
//        if ((numberOfLostPackets < hopcount) && ((timeUs - lastPacketTimeUs) > (getInterval(&bind_data) + 1000))) {
//            // we lost packet, hop to next channel
//            linkQuality <<= 1;
//            willhop = 1;
//            if (numberOfLostPackets == 0) {
//                linkLossTimeMs = timeMs;
//                nextBeaconTimeMs = 0;
//            }
//            numberOfLostPackets++;
//            lastPacketTimeUs += getInterval(&bind_data);
//            willhop = 1;
//
//
//
//        } else if ((numberOfLostPackets == hopcount) && ((timeUs - lastPacketTimeUs) > (getInterval(&bind_data) * hopcount))) {
//            // hop slowly to allow resync with TX
//            linkQuality = 0;
//            willhop = 1;
//            smoothRSSI = 0;
//            lastPacketTimeUs = timeUs;
//        }
//
//        if (numberOfLostPackets) {
//            if (rx_config.failsafeDelay && (!failsafeActive) && ((timeMs - linkLossTimeMs) > delayInMs(rx_config.failsafeDelay))) {
//                failsafeActive = 1;
//                failsafeApply();
//                nextBeaconTimeMs = (timeMs + delayInMsLong(rx_config.beacon_deadtime)) | 1; //beacon activating...
//            }
//            if (rx_config.pwmStopDelay && (!disablePWM) && ((timeMs - linkLossTimeMs) > delayInMs(rx_config.pwmStopDelay))) {
//                disablePWM = 1;
//            }
//            if (rx_config.ppmStopDelay && (!disablePPM) && ((timeMs - linkLossTimeMs) > delayInMs(rx_config.ppmStopDelay))) {
//                disablePPM = 1;
//            }
//
//            if ((rx_config.beacon_frequency) && (nextBeaconTimeMs) && ((timeMs - nextBeaconTimeMs) < 0x80000000)) {
//                beacon_send((rx_config.flags & STATIC_BEACON));
//                init_rfm(0);   // go back to normal RX
//                rx_reset();
//                nextBeaconTimeMs = (millis() + (1000UL * rx_config.beacon_interval)) | 1; // avoid 0 in time
//            }
//        }
//    } else {
//        // Waiting for first packet, hop slowly
//        if ((timeUs - lastPacketTimeUs) > (getInterval(&bind_data) * hopcount)) {
//            lastPacketTimeUs = timeUs;
//            willhop = 1;
//        }
//    }
//
//    doHop(willhop);

    return ret;
}

bool openlrsInit(const struct rxConfig_s *rxConfig, struct rxRuntimeConfig_s *rxRuntimeConfig) {

    UNUSED(rxConfig);

    rxRuntimeConfig->channelCount = RC_CHANNEL_CNT;

    // radio ini
    rfm22_initialise();

    RF_channel = 0;
    rfm22_setChannel(RF_channel);

    // Count hop channels as we need it later
    hopcount = 0;
    while ((hopcount < MAX_CHANNEL) && (bind_data.rf_channel[hopcount] != 0)) {
        hopcount++;
    }

    linkAcquired = 0;
    lastPacketTimeUs = millis();

    protocolState = STATE_SETUP_RX;
    return true;
}
//
//// to be called from cli.
//uint8_t bindReceive(uint32_t timeout) {
//    uint32_t start = millis();
//    uint8_t rxb;
//    init_rfm(1);
//    RF_Mode = Receive;
//    to_rx_mode();
////    Serial.println("Waiting bind\n");
//
//    while ((!timeout) || ((millis() - start) < timeout)) {
//        if (RF_Mode == Received) {
////            Serial.println("Got pkt\n");
//            spiSendAddress(0x7f);   // Send the package read command
//            rxb = spiReadData();
//            if (rxb == 'b') {
//                for (uint8_t i = 0; i < sizeof(bind_data); i++) {
//                    *(((uint8_t*) &bind_data) + i) = spiReadData();
//                }
//
//                if (bind_data.version == BINDING_VERSION) {
////                    Serial.println("data good\n");
//                    rxb = 'B';
//                    tx_packet(&rxb, 1); // ACK that we got bound
////                    Green_LED_ON; //signal we got bound on LED:s
//                    return 1;
//                }
//            } else if ((rxb == 'p') || (rxb == 'i')) {
//                uint8_t rxc_buf[sizeof(rx_config) + 1];
//                if (rxb == 'p') {
//                    rxc_buf[0] = 'P';
//                    timeout = 0;
//                } else {
//                    rxInitDefaults(1);
//                    rxc_buf[0] = 'I';
//                }
//                if (watchdogUsed) {
//                    rx_config.flags |= WATCHDOG_USED;
//                } else {
//                    rx_config.flags &= ~WATCHDOG_USED;
//                }
//                memcpy(rxc_buf + 1, &rx_config, sizeof(rx_config));
//                tx_packet(rxc_buf, sizeof(rx_config) + 1);
//            } else if (rxb == 't') {
//                uint8_t rxc_buf[sizeof(rxSpecialPins) + 5];
//                timeout = 0;
//                rxc_buf[0] = 'T';
//                rxc_buf[1] = (version >> 8);
//                rxc_buf[2] = (version & 0xff);
//                rxc_buf[3] = OUTPUTS;
//                rxc_buf[4] = sizeof(rxSpecialPins) / sizeof(rxSpecialPins[0]);
//                memcpy(rxc_buf + 5, &rxSpecialPins, sizeof(rxSpecialPins));
//                tx_packet(rxc_buf, sizeof(rxSpecialPins) + 5);
//            } else if (rxb == 'u') {
//                for (uint8_t i = 0; i < sizeof(rx_config); i++) {
//                    *(((uint8_t*) &rx_config) + i) = spiReadData();
//                }
//                accessEEPROM(0, true);
//                rxb = 'U';
//                tx_packet(&rxb, 1); // ACK that we updated settings
//            } else if (rxb == 'f') {
//                uint8_t rxc_buf[33];
//                rxc_buf[0] = 'F';
//                for (uint8_t i = 0; i < 16; i++) {
//                    uint16_t us = failsafePPM[i];
//                    rxc_buf[i * 2 + 1] = (us >> 8);
//                    rxc_buf[i * 2 + 2] = (us & 0xff);
//                }
//                tx_packet(rxc_buf, 33);
//            } else if (rxb == 'g') {
//                for (uint8_t i = 0; i < 16; i++) {
//                    failsafePPM[i] = ((uint16_t) spiReadData() << 8) + spiReadData();
//                }
//                rxb = 'G';
//                failsafeSave();
//                tx_packet(&rxb, 1);
//            } else if (rxb == 'G') {
//                for (uint8_t i = 0; i < 16; i++) {
//                    failsafePPM[i] = 0;
//                }
//                failsafeSave();
//                rxb = 'G';
//                tx_packet(&rxb, 1);
//            }
//            RF_Mode = Receive;
//            rx_reset();
//        }
//    }
//    return 0;
//}



