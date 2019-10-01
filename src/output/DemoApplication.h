/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Display driver header

Maintainer: Gregory Cristian & Gilbert Menth
*/

#ifndef DEMO_APPLICATION_H
#define DEMO_APPLICATION_H


/*!
 * \brief Used to display firmware version on TFT (Utilities menu)
 */
#define FIRMWARE_VERSION    ( ( char* )"Firmware Version: 180706" )

/*!
 * \brief Define range of central frequency [Hz]
 */
#define DEMO_CENTRAL_FREQ_MIN       150000000UL
#define DEMO_CENTRAL_FREQ_MAX       950000000UL

/*!
 * \brief Define 3 preset central frequencies [Hz]
 */
#define DEMO_CENTRAL_FREQ_PRESET1   169000000UL
#define DEMO_CENTRAL_FREQ_PRESET2   280000000UL
#define DEMO_CENTRAL_FREQ_PRESET3   434000000UL
#define DEMO_CENTRAL_FREQ_PRESET4   490000000UL
#define DEMO_CENTRAL_FREQ_PRESET5   783000000UL
#define DEMO_CENTRAL_FREQ_PRESET6   868000000UL
#define DEMO_CENTRAL_FREQ_PRESET7   915000000UL
#define DEMO_CENTRAL_FREQ_PRESET8   930000000UL
#define DEMO_CENTRAL_FREQ_PRESET9   510000000UL

/*!
 * \brief Define min and max Tx power [dBm]
 */
#define SX1261_POWER_TX_MIN           -17
#define SX1261_POWER_TX_MAX           15

#define SX1262_POWER_TX_MIN           -10
#define SX1262_POWER_TX_MAX           22

/*!
 * \brief Define current demo mode
 */
enum DemoMode
{
    MASTER = 0,
    SLAVE
};

/*!
 * \brief Define GFSK bitrate
 */
typedef enum
{
    DEMO_BR_100         = 100,
    DEMO_BR_600         = 600,
    DEMO_BR_4800        = 4800,
    DEMO_BR_9600        = 9600,
    DEMO_BR_19200       = 19200,
    DEMO_BR_57600       = 57600,
    DEMO_BR_100000      = 100000,
    DEMO_BR_250000      = 250000,
}DemoBitrate_t;

/*!
 * \brief Define GFSK frequency deviation
 */
typedef enum
{
    DEMO_FDEV_5000      = 5000,
    DEMO_FDEV_10000     = 10000,
    DEMO_FDEV_25000     = 25000,
    DEMO_FDEV_50000     = 50000,
    DEMO_FDEV_75000     = 75000,
    DEMO_FDEV_100000    = 100000,
    DEMO_FDEV_150000    = 150000,
}DemoFrequencyDev_t;

/*!
 * \brief List of states for demo state machine
 */
enum DemoInternalStates
{
    APP_IDLE = 0,               // nothing to do (or wait a radio interrupt)
    SEND_PING_MSG,
    SEND_PONG_MSG,
    APP_RX,                     // Rx done
    APP_RX_TIMEOUT,             // Rx timeout
    APP_RX_ERROR,               // Rx error
    APP_TX,                     // Tx done
    APP_TX_TIMEOUT,             // Tx error
    PER_TX_START,               // PER master
    PER_RX_START,               // PER slave
    CAD_DONE,                   // CAD Done
    CAD_DONE_CHANNEL_DETECTED   // Channel Detected following a CAD
};

/*!
 * \brief Demo Settings structure of Eeprom structure
 */
typedef struct
{
    uint8_t Entity;             // Master or Slave
    uint8_t HoldDemo;           // Put demo in hold status
    uint8_t BoostedRx;          // Use Boosted Rx if true
    uint32_t Frequency;         // Demo frequency
    uint8_t LastDeviceConnected;// Last Device Connected
    int8_t TxPower;             // Demo Tx power
    uint8_t RadioPowerMode;     // Radio Power Mode [0: LDO, 1:DC_DC]
    uint8_t PayloadLength;      // Demo payload length
    uint8_t ModulationType;     // Demo modulation type (LORA, GFSK)
    uint32_t ModulationParam1;  // Demo Mod. Param1 (depend on modulation type)
    uint32_t ModulationParam2;  // Demo Mod. Param2 (depend on modulation type)
    uint8_t ModulationParam3;   // Demo Mod. Param3 (depend on modulation type)
    uint8_t ModulationParam4;   // Demo Mod. Param4 (depend on modulation type)
    uint16_t PacketParam1;      // Demo Pack. Param1 (depend on packet type)
    uint8_t PacketParam2;       // Demo Pack. Param2 (depend on packet type)
    uint8_t PacketParam3;       // Demo Pack. Param3 (depend on packet type)
    uint8_t PacketParam4;       // Demo Pack. Param4 (depend on packet type)
    uint8_t PacketParam5;       // Demo Pack. Param5 (depend on packet type)
    uint8_t PacketParam6;       // Demo Pack. Param6 (depend on packet type)
    uint8_t PacketParam7;       // Demo Pack. Param7 (depend on packet type)
    uint8_t PacketParam8;       // Demo Pack. Param8 (depend on packet type)
    uint32_t MaxNumPacket;      // Demo Max Num Packet for PingPong and PER
    uint16_t InterPacketDelay;  // Demo Inter-Packet Delay for PingPong and PER
    uint32_t CntPacketTx;       // Tx packet transmitted
    uint32_t CntPacketRxOK;     // Rx packet received OK
    uint32_t CntPacketRxOKSlave;// Rx packet received OK (slave side)
    uint32_t CntPacketRxKO;     // Rx packet received KO
    uint32_t CntPacketRxKOSlave;// Rx packet received KO (slave side)
    uint16_t RxTimeOutCount;    // Rx packet received KO (by timeout)
    int8_t RssiValue;           // Demo Rssi Value
    int8_t SnrValue;            // Demo Snr Value (only for LR24 mod. type)
    uint32_t FreqErrorEst;      // Estimation of the frequency error on the Rx side
}DemoSettings_t;

/*!
 * \brief Define freq offset for config central freq in "Radio Config Freq" menu
 */
enum FreqBase
{
    FB1     = 1,            //   1 Hz
    FB10    = 10,           //  10 Hz
    FB100   = 100,          // 100 Hz
    FB1K    = 1000,         //   1 kHz
    FB10K   = 10000,        //  10 kHz
    FB100K  = 100000,       // 100 kHz
    FB1M    = 1000000,      //   1 MHz
    FB10M   = 10000000      //  10 MHz
};


/*!
 * \brief Simple Function which return the device connected.
 *
 * \retval      deviceConnected    device type connected
 */
uint8_t GetConnectedDevice( void );

/*!
 * \brief Simple Function which return the board matching frequency
 *
 * \retval      freq    1: 868 MHz   0: 915 MHz
 */
uint8_t GetMatchingFrequency( void );

/*!
 * \brief Init RAM copy of Eeprom structure and init radio with it.
 *
 */
void InitDemoApplication( void );

/*!
 * \brief Init vars of demo and fix APP_IDLE state to demo state machine.
 */
void StopDemoApplication( void );

/*!
 * \brief Run demo reading constantly the RSSI.
 *
 * \retval      demoStatusUpdate    page refresh status ( >0 : refresh)
 */
uint8_t RunDemoTestRssi( void );

/*!
 * \brief Run Demo in sleep mode.
 *
 * \retval      demoStatusUpdate    page refresh status ( >0 : refresh)
 */
uint8_t RunDemoSleepMode( void );

/*!
 * \brief Run Demo in standby RC mode.
 *
 * \retval      demoStatusUpdate    page refresh status ( >0 : refresh)
 */
uint8_t RunDemoStandbyRcMode( void );

/*!
 * \brief Run Demo in standby XOSC mode.
 *
 * \retval      demoStatusUpdate    page refresh status ( >0 : refresh)
 */
uint8_t RunDemoStandbyXoscMode( void );

/*!
 * \brief Run Demo Tx in continuous mode without modulation.
 *
 * \retval      demoStatusUpdate    page refresh status ( >0 : refresh)
 */
uint8_t RunDemoTxCw( void );

/*!
 * \brief Run Demo Tx in continuous modulation.
 *
 * \retval      demoStatusUpdate    page refresh status ( >0 : refresh)
 */
uint8_t RunDemoTxContinuousModulation( void );

/*!
 * \brief Run Demo Rx in continuous mode.
 *
 * \retval      demoStatusUpdate    page refresh status ( >0 : refresh)
 */
uint8_t RunDemoRxContinuous( void );

/*!
 * \brief Run demo PingPong.
 *
 * \retval      demoStatusUpdate    page refresh status ( >0 : refresh)
 */
uint8_t RunDemoApplicationPingPong( void );

/*!
 * \brief Compute payload of Rx frame and update current counts and indicators.
 *
 * \param [in]  buffer        buffer with frame to compute
 * \param [in]  buffersize    size of frame data in the buffer
 */
void ComputePingPongPayload( uint8_t *buffer, uint8_t bufferSize );

/*!
 * \brief Run demo PER.
 *
 * \retval      demoStatusUpdate    page refresh status ( >0 : refresh)
 */
uint8_t RunDemoApplicationPer( void );

/*!
 * \brief Compute payload of Rx frame and update current counts and indicators.
 *
 * \param [in]  buffer        buffer with frame to compute
 * \param [in]  buffersize    size of frame data in the buffer
 */
void ComputePerPayload( uint8_t *buffer, uint8_t bufferSize );

#endif // DEMO_APPLICATION_H
