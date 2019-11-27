#pragma once

/*!
 * \brief List of matching supported by the sx126x
 */
#define MATCHING_FREQ_915                           0
#define MATCHING_FREQ_780                           1
#define MATCHING_FREQ_490                           2
#define MATCHING_FREQ_434                           3
#define MATCHING_FREQ_280                           4
#define MATCHING_FREQ_169                           5
#define MATCHING_FREQ_868                           6


/*!
 * \brief List of devices supported by this driver
 */
#define SX1261  0
#define SX1262  1
#define SX1268  2


/*!
 * \brief Represents all possible opcode understood by the radio
 */
typedef enum RadioCommands_e
{
    RADIO_GET_STATUS                        = 0xC0,
    RADIO_WRITE_REGISTER                    = 0x0D,
    RADIO_READ_REGISTER                     = 0x1D,
    RADIO_WRITE_BUFFER                      = 0x0E,
    RADIO_READ_BUFFER                       = 0x1E,
    RADIO_SET_SLEEP                         = 0x84,
    RADIO_SET_STANDBY                       = 0x80,
    RADIO_SET_FS                            = 0xC1,
    RADIO_SET_TX                            = 0x83,
    RADIO_SET_RX                            = 0x82,
    RADIO_SET_RXDUTYCYCLE                   = 0x94,
    RADIO_SET_CAD                           = 0xC5,
    RADIO_SET_TXCONTINUOUSWAVE              = 0xD1,
    RADIO_SET_TXCONTINUOUSPREAMBLE          = 0xD2,
    RADIO_SET_PACKETTYPE                    = 0x8A,
    RADIO_GET_PACKETTYPE                    = 0x11,
    RADIO_SET_RFFREQUENCY                   = 0x86,
    RADIO_SET_TXPARAMS                      = 0x8E,
    RADIO_SET_PACONFIG                      = 0x95,
    RADIO_SET_CADPARAMS                     = 0x88,
    RADIO_SET_BUFFERBASEADDRESS             = 0x8F,
    RADIO_SET_MODULATIONPARAMS              = 0x8B,
    RADIO_SET_PACKETPARAMS                  = 0x8C,
    RADIO_GET_RXBUFFERSTATUS                = 0x13,
    RADIO_GET_PACKETSTATUS                  = 0x14,
    RADIO_GET_RSSIINST                      = 0x15,
    RADIO_GET_STATS                         = 0x10,
    RADIO_RESET_STATS                       = 0x00,
    RADIO_CFG_DIOIRQ                        = 0x08,
    RADIO_GET_IRQSTATUS                     = 0x12,
    RADIO_CLR_IRQSTATUS                     = 0x02,
    RADIO_CALIBRATE                         = 0x89,
    RADIO_CALIBRATEIMAGE                    = 0x98,
    RADIO_SET_REGULATORMODE                 = 0x96,
    RADIO_GET_ERROR                         = 0x17,
    RADIO_SET_TCXOMODE                      = 0x97,
    RADIO_SET_TXFALLBACKMODE                = 0x93,
    RADIO_SET_RFSWITCHMODE                  = 0x9D,
    RADIO_SET_STOPRXTIMERONPREAMBLE         = 0x9F,
    RADIO_SET_LORASYMBTIMEOUT               = 0xA0,
}RadioCommands_t;
