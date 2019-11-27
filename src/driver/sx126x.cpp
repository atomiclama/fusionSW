
#include "radio.h"

#include "sx126x.h"



#include <cstring>


/*!
 * \brief Radio registers definition
 */
typedef struct
{
    uint16_t      Addr;                             //!< The address of the register
    uint8_t       Value;                            //!< The value of the register
}RadioRegisters_t;


void SX126x::init(void) {

    // taken from Semtech app note in sx1262 data sheet
    // 1. If not in STDBY_RC mode, then go to this mode with the command SetStandby(...)
    SetStandby(STDBY_XOSC);

    // 2. Define the protocol with the command SetPacketType(...)
    SetPacketType(PACKET_TYPE_LORA);

    // 3. Define the RF frequency with the command SetRfFrequency(...)
    SetRfFrequency( 868000000 );

    // 4. Define output power and ramping time with the command SetTxParams(...)
    SetTxParams(0, RADIO_RAMP_200_US);

    // use dio2 as RF switch control so we don't have to.
    SetDio2AsRfSwitchCtrl( true );

    // 5.Define where the data payload will be stored with the command SetBufferBaseAddress(...)
    SetBufferBaseAddresses(0,0);

    // 6.Send the payload to the data buffer with the command WriteBuffer(...)
    // do this before a tx not here.
    // WriteBuffer( 0x00, tx, 10 );

    // 7.Define the modulation parameter according to the chosen protocol with the command SetModulationParams(...)
    ModulationParams_t modulationParams;
    modulationParams.PacketType = PACKET_TYPE_LORA;
    modulationParams.Params.LoRa.Bandwidth = LORA_BW_500;
    modulationParams.Params.LoRa.CodingRate = LORA_CR_4_8;
    modulationParams.Params.LoRa.SpreadingFactor =  LORA_SF5;
    modulationParams.Params.LoRa.LowDatarateOptimize = false;
    SetModulationParams(&modulationParams);

    // 8.Define the frame format to be used with the command SetPacketParams(...)
    PacketParams_t packetParams;
    packetParams.PacketType = PACKET_TYPE_LORA;
    packetParams.Params.LoRa.InvertIQ = LORA_IQ_NORMAL;
    packetParams.Params.LoRa.PreambleLength = 16;
    packetParams.Params.LoRa.HeaderType = LORA_PACKET_IMPLICIT; // fixed
    packetParams.Params.LoRa.PayloadLength = 20;
    packetParams.Params.LoRa.CrcMode = LORA_CRC_ON;
    SetPacketParams(&packetParams);

    // 9.Configure DIO and IRQ: use the command SetDioIrqParams(...) to select TxDone IRQ and map this IRQ to a DIO (DIO1, DIO2 or DIO3)
    // 10.Define Sync Word value: use the command WriteReg(...) to write the value of the register via direct register access
    // 11.Set the circuit in transmitter mode to start transmission with the command SetTx(). Use the parameter to enable Timeout
    // radio.SetTx(0);

    // 12.Wait for the IRQ TxDone or Timeout: once the packet has been sent the chip goes automatically to STDBY_RC mode
    // 13.Clear the IRQ TxDone flag
}


void SX126x::Init( void )
{
    CalibrationParams_t calibParam;
    
    /*!
     * \brief pin OPT is used to detect if the board has a TCXO or a XTAL
     *
     * OPT = 0 >> TCXO; OPT = 1 >> XTAL
     */
    // DigitalIn OPT( A3 );
    
    // Reset( );

    // IoIrqInit( dioIrq );

    // Wakeup( );
    SetStandby( STDBY_RC );

    // if( OPT == 0 )
    {
        // SetDio3AsTcxoCtxTl( TCXO_CTRL_1_7V, 320 ); //5 ms
        calibParam.Value = 0x7F;
        Calibrate( calibParam );
    }

    // SetPollingMode( );

    // AntSwOn( );
    SetDio2AsRfSwitchCtrl( true );
    
    OperatingMode = MODE_STDBY_RC;
    
    SetPacketType( PACKET_TYPE_LORA );

// #ifdef USE_CONFIG_PUBLIC_NETOWRK
//     // Change LoRa modem Sync Word for Public Networks
//     WriteReg( REG_LR_SYNCWORD, ( LORA_MAC_PUBLIC_SYNCWORD >> 8 ) & 0xFF );
//     WriteReg( REG_LR_SYNCWORD + 1, LORA_MAC_PUBLIC_SYNCWORD & 0xFF );
// #else
//     // Change LoRa modem SyncWord for Private Networks
//     WriteReg( REG_LR_SYNCWORD, ( LORA_MAC_PRIVATE_SYNCWORD >> 8 ) & 0xFF );
//     WriteReg( REG_LR_SYNCWORD + 1, LORA_MAC_PRIVATE_SYNCWORD & 0xFF );
// #endif
}

RadioOperatingModes_t SX126x::GetOperatingMode( void )
{
    return OperatingMode;
}

void SX126x::CheckDeviceReady( void )
{
    if( ( GetOperatingMode( ) == MODE_SLEEP ) || ( GetOperatingMode( ) == MODE_RX_DC ) )
    {
        Wakeup( );
        // Switch is turned off when device is in sleep mode and turned on is all other modes
        // AntSwOn( );
    }
}

void SX126x::SetPayload( uint8_t *payload, uint8_t size )
{
    WriteBuffer( 0x00, payload, size );
}

uint8_t SX126x::GetPayload( uint8_t *buffer, uint8_t *size,  uint8_t maxSize )
{
    uint8_t offset = 0;

    GetRxBufferStatus( size, &offset );
    if( *size > maxSize )
    {
        return 1;
    }
    ReadBuffer( offset, buffer, *size );
    return 0;
}

void SX126x::SendPayload( uint8_t *payload, uint8_t size, uint32_t timeout )
{
    SetPayload( payload, size );
    SetTx( timeout );
}

uint8_t SX126x::SetSyncWord( uint8_t *syncWord )
{
    WriteRegister( REG_LR_SYNCWORDBASEADDRESS, syncWord, 8 );
    return 0;
}

void SX126x::SetCrcSeed( uint16_t seed )
{
    uint8_t buf[2];

    buf[0] = ( uint8_t )( ( seed >> 8 ) & 0xFF );
    buf[1] = ( uint8_t )( seed & 0xFF );

    switch( GetPacketType( ) )
    {
        case PACKET_TYPE_GFSK:
            WriteRegister( REG_LR_CRCSEEDBASEADDR, buf, 2 );
            break;

        default:
            break;
    }
}

void SX126x::SetCrcPolynomial( uint16_t polynomial )
{
    uint8_t buf[2];

    buf[0] = ( uint8_t )( ( polynomial >> 8 ) & 0xFF );
    buf[1] = ( uint8_t )( polynomial & 0xFF );

    switch( GetPacketType( ) )
    {
        case PACKET_TYPE_GFSK:
            WriteRegister( REG_LR_CRCPOLYBASEADDR, buf, 2 );
            break;

        default:
            break;
    }
}

void SX126x::SetWhiteningSeed( uint16_t seed )
{
    uint8_t regValue = 0;

    switch( GetPacketType( ) )
    {
        case PACKET_TYPE_GFSK:
            ReadRegister( REG_LR_WHITSEEDBASEADDR_MSB, &regValue, 1 );
            regValue &= 0xFE;
            regValue = ( ( seed >> 8 ) & 0x01 ) | regValue;
            // WriteReg( REG_LR_WHITSEEDBASEADDR_MSB, regValue ); // only 1 bit.
            // WriteReg( REG_LR_WHITSEEDBASEADDR_LSB, ( uint8_t )seed );
            break;

        default:
            break;
    }
}

uint32_t SX126x::GetRandom( void )
{
    uint8_t buf[] = { 0, 0, 0, 0 };

    // Set radio in continuous reception
    SetRx( 0 );

    // wait_ms( 1 );

    ReadRegister( RANDOM_NUMBER_GENERATORBASEADDR, buf, 4 );

    SetStandby( STDBY_RC );

    return ( buf[0] << 24 ) | ( buf[1] << 16 ) | ( buf[2] << 8 ) | buf[3];
}

void SX126x::SetSleep( SleepParams_t sleepConfig )
{
    // AntSwOff( );

    WriteCommand( RADIO_SET_SLEEP, &sleepConfig.Value, 1 );
    OperatingMode = MODE_SLEEP;
}

void SX126x::SetStandby( RadioStandbyModes_t standbyConfig )
{

    WriteCommand( RADIO_SET_STANDBY, ( uint8_t* )&standbyConfig, 1 );
    if( standbyConfig == STDBY_RC )
    {
        OperatingMode = MODE_STDBY_RC;
    }
    else
    {
        OperatingMode = MODE_STDBY_XOSC;
    }
}

void SX126x::SetFs( void )
{

    WriteCommand( RADIO_SET_FS, 0, 0 );
    OperatingMode = MODE_FS;
}

void SX126x::SetTx( uint32_t timeout )
{
    uint8_t buf[3];

    OperatingMode = MODE_TX;
 
    buf[0] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( timeout & 0xFF );
    WriteCommand( RADIO_SET_TX, buf, 3 );
}

void SX126x::SetRxBoosted( uint32_t timeout )
{
    uint8_t buf[3];

    OperatingMode = MODE_RX;

    uint8_t regVal = 0x96;
    WriteRegister( REG_RX_GAIN, &regVal, 1 ); // max LNA gain, increase current by ~2mA for around ~3dB in sensivity

    buf[0] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( timeout & 0xFF );
    WriteCommand( RADIO_SET_RX, buf, 3 );
}

void SX126x::SetRx( uint32_t timeout )
{
    uint8_t buf[3];

    OperatingMode = MODE_RX;

    buf[0] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( timeout & 0xFF );
    WriteCommand( RADIO_SET_RX, buf, 3 );
}

void SX126x::SetRxDutyCycle( uint32_t rxTime, uint32_t sleepTime )
{
    uint8_t buf[6];

    buf[0] = ( uint8_t )( ( rxTime >> 16 ) & 0xFF );
    buf[1] = ( uint8_t )( ( rxTime >> 8 ) & 0xFF );
    buf[2] = ( uint8_t )( rxTime & 0xFF );
    buf[3] = ( uint8_t )( ( sleepTime >> 16 ) & 0xFF );
    buf[4] = ( uint8_t )( ( sleepTime >> 8 ) & 0xFF );
    buf[5] = ( uint8_t )( sleepTime & 0xFF );
    WriteCommand( RADIO_SET_RXDUTYCYCLE, buf, 6 );
    OperatingMode = MODE_RX_DC;
}

void SX126x::SetCad( void )
{
    WriteCommand( RADIO_SET_CAD, 0, 0 );
    OperatingMode = MODE_CAD;
}

void SX126x::SetTxContinuousWave( void )
{
    WriteCommand( RADIO_SET_TXCONTINUOUSWAVE, 0, 0 );
}

void SX126x::SetTxInfinitePreamble( void )
{
    WriteCommand( RADIO_SET_TXCONTINUOUSPREAMBLE, 0, 0 );
}

void SX126x::SetStopRxTimerOnPreambleDetect( bool enable )
{
    WriteCommand( RADIO_SET_STOPRXTIMERONPREAMBLE, ( uint8_t* )&enable, 1 );
}

void SX126x::SetLoRaSymbNumTimeout( uint8_t SymbNum )
{
    WriteCommand( RADIO_SET_LORASYMBTIMEOUT, &SymbNum, 1 );
}

void SX126x::SetRegulatorMode( RadioRegulatorMode_t mode )
{
    WriteCommand( RADIO_SET_REGULATORMODE, ( uint8_t* )&mode, 1 );
}

void SX126x::Calibrate( CalibrationParams_t calibParam )
{
    WriteCommand( RADIO_CALIBRATE, &calibParam.Value, 1 );
}

void SX126x::CalibrateImage( uint32_t freq )
{
    uint8_t calFreq[2];

    if( freq > 900000000 )
    {
        calFreq[0] = 0xE1;
        calFreq[1] = 0xE9;
    }
    else if( freq > 850000000 )
    {
        calFreq[0] = 0xD7;
        calFreq[1] = 0xD8;
    }
    else if( freq > 770000000 )
    {
        calFreq[0] = 0xC1;
        calFreq[1] = 0xC5;
    }
    else if( freq > 460000000 )
    {
        calFreq[0] = 0x75;
        calFreq[1] = 0x81;
    }
    else if( freq > 425000000 )
    {
        calFreq[0] = 0x6B;
        calFreq[1] = 0x6F;
    }
    WriteCommand( RADIO_CALIBRATEIMAGE, calFreq, 2 );
}

void SX126x::SetPaConfig( uint8_t paDutyCycle, uint8_t HpMax, uint8_t deviceSel, uint8_t paLUT )
{
    uint8_t buf[4];

    buf[0] = paDutyCycle;
    buf[1] = HpMax;
    buf[2] = deviceSel;
    buf[3] = paLUT;
    WriteCommand( RADIO_SET_PACONFIG, buf, 4 );
}

void SX126x::SetRxTxFallbackMode( uint8_t fallbackMode )
{
    WriteCommand( RADIO_SET_TXFALLBACKMODE, &fallbackMode, 1 );
}

void SX126x::SetDioIrqParams( uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask )
{
    uint8_t buf[8];

    buf[0] = ( uint8_t )( ( irqMask >> 8 ) & 0x00FF );
    buf[1] = ( uint8_t )( irqMask & 0x00FF );
    buf[2] = ( uint8_t )( ( dio1Mask >> 8 ) & 0x00FF );
    buf[3] = ( uint8_t )( dio1Mask & 0x00FF );
    buf[4] = ( uint8_t )( ( dio2Mask >> 8 ) & 0x00FF );
    buf[5] = ( uint8_t )( dio2Mask & 0x00FF );
    buf[6] = ( uint8_t )( ( dio3Mask >> 8 ) & 0x00FF );
    buf[7] = ( uint8_t )( dio3Mask & 0x00FF );
    WriteCommand( RADIO_CFG_DIOIRQ, buf, 8 );
}

uint16_t SX126x::GetIrqStatus( void )
{
    uint8_t irqStatus[2];

    ReadCommand( RADIO_GET_IRQSTATUS, irqStatus, 2 );
    return ( irqStatus[0] << 8 ) | irqStatus[1];
}

void SX126x::SetDio2AsRfSwitchCtrl( uint8_t enable )
{

    WriteCommand( RADIO_SET_RFSWITCHMODE, &enable, 1 );
}

void SX126x::SetDio3AsTcxoCtrl( RadioTcxoCtrlVoltage_t tcxoVoltage, uint32_t timeout )
{
    uint8_t buf[4];

    buf[0] = tcxoVoltage & 0x07;
    buf[1] = ( uint8_t )( ( timeout >> 16 ) & 0xFF );
    buf[2] = ( uint8_t )( ( timeout >> 8 ) & 0xFF );
    buf[3] = ( uint8_t )( timeout & 0xFF );

    WriteCommand( RADIO_SET_TCXOMODE, buf, 4 );
}

void SX126x::SetRfFrequency( uint32_t frequency )
{
    uint8_t buf[4];
    uint32_t freq = 0;



    if( ImageCalibrated == false )
    {
        CalibrateImage( frequency );
        ImageCalibrated = true;
    }

    freq = ( uint32_t )( ( double )frequency / ( double )FREQ_STEP );
    buf[0] = ( uint8_t )( ( freq >> 24 ) & 0xFF );
    buf[1] = ( uint8_t )( ( freq >> 16 ) & 0xFF );
    buf[2] = ( uint8_t )( ( freq >> 8 ) & 0xFF );
    buf[3] = ( uint8_t )( freq & 0xFF );
    WriteCommand( RADIO_SET_RFFREQUENCY, buf, 4 );
}

void SX126x::SetPacketType( RadioPacketTypes_t packetType )
{

    // Save packet type internally to avoid questioning the radio
    this->PacketType = packetType;
    WriteCommand( RADIO_SET_PACKETTYPE, ( uint8_t* )&packetType, 1 );
}

RadioPacketTypes_t SX126x::GetPacketType( void )
{
    return this->PacketType;
}

void SX126x::SetTxParams( int8_t power, RadioRampTimes_t rampTime )
{
    uint8_t buf[2];
    // DigitalIn OPT( A3 );

    if( false )//GetDeviceType( ) == SX1261 )
    {
        if( power == 15 )
        {
            SetPaConfig( 0x06, 0x00, 0x01, 0x01 );
        }
        else
        {
            SetPaConfig( 0x04, 0x00, 0x01, 0x01 );  
        }
        if( power >= 14 )
        {
            power = 14;
        }
        else if( power < -3 )
        {
            power = -3;
        }
        // WriteReg( REG_OCP, 0x18 ); // current max is 80 mA for the whole device
    }
    else // sx1262 or sx1268
    {
        SetPaConfig( 0x04, 0x07, 0x00, 0x01 );
        if( power > 22 )
        {
            power = 22;
        }
        else if( power < -3 )
        {
            power = -3;
        }
        // WriteReg( REG_OCP, 0x38 ); // current max 160mA for the whole device
    }
    buf[0] = power;
    if( false) //OPT == 0 )
    {
        if( ( uint8_t )rampTime < RADIO_RAMP_200_US )
        {
            buf[1] = RADIO_RAMP_200_US;
        }
        else
        {
            buf[1] = ( uint8_t )rampTime;
        }
    }
    else
    {
        buf[1] = ( uint8_t )rampTime;
    }
    WriteCommand( RADIO_SET_TXPARAMS, buf, 2 );
}

void SX126x::SetModulationParams( ModulationParams_t *modulationParams )
{
    uint8_t n;
    uint32_t tempVal = 0;
    uint8_t buf[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    // Check if required configuration corresponds to the stored packet type
    // If not, silently update radio packet type
    if( this->PacketType != modulationParams->PacketType )
    {
        this->SetPacketType( modulationParams->PacketType );
    }

    switch( modulationParams->PacketType )
    {
    case PACKET_TYPE_GFSK:
        n = 8;
        tempVal = ( uint32_t )( 32 * ( ( double )XTAL_FREQ / ( double )modulationParams->Params.Gfsk.BitRate ) );
        buf[0] = ( tempVal >> 16 ) & 0xFF;
        buf[1] = ( tempVal >> 8 ) & 0xFF;
        buf[2] = tempVal & 0xFF;
        buf[3] = modulationParams->Params.Gfsk.ModulationShaping;
        buf[4] = modulationParams->Params.Gfsk.Bandwidth;
        tempVal = ( uint32_t )( ( double )modulationParams->Params.Gfsk.Fdev / ( double )FREQ_STEP );
        buf[5] = ( tempVal >> 16 ) & 0xFF;
        buf[6] = ( tempVal >> 8 ) & 0xFF;
        buf[7] = ( tempVal& 0xFF );
        break;
    case PACKET_TYPE_LORA:
        n = 4;
        switch( modulationParams->Params.LoRa.Bandwidth )
        {
            case LORA_BW_500:
                 modulationParams->Params.LoRa.LowDatarateOptimize = 0x00;
                break;
            case LORA_BW_250:
                if( modulationParams->Params.LoRa.SpreadingFactor == 12 )
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x01;
                }
                else
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x00;
                }
                break;
            case LORA_BW_125:
                if( modulationParams->Params.LoRa.SpreadingFactor >= 11 )
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x01;
                }
                else
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x00;
                }
                break;
            case LORA_BW_062:
                if( modulationParams->Params.LoRa.SpreadingFactor >= 10 )
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x01;
                }
                else
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x00;
                }
                break;
            case LORA_BW_041:
                if( modulationParams->Params.LoRa.SpreadingFactor >= 9 )
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x01;
                }
                else
                {
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x00;
                }
                break;
            case LORA_BW_031:
            case LORA_BW_020:
            case LORA_BW_015:
            case LORA_BW_010:
            case LORA_BW_007:
                    modulationParams->Params.LoRa.LowDatarateOptimize = 0x01;
                break;
            default:
                break;
        }
        buf[0] = modulationParams->Params.LoRa.SpreadingFactor;
        buf[1] = modulationParams->Params.LoRa.Bandwidth;
        buf[2] = modulationParams->Params.LoRa.CodingRate;
        buf[3] = modulationParams->Params.LoRa.LowDatarateOptimize;
        break;
    default:
    case PACKET_TYPE_NONE:
        return;
    }
    WriteCommand( RADIO_SET_MODULATIONPARAMS, buf, n );
}

void SX126x::SetPacketParams( PacketParams_t *packetParams )
{
    uint8_t n;
    uint8_t crcVal = 0;
    uint8_t buf[9] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#ifdef ADV_DEBUG
    printf("SetPacketParams ");
#endif

    // Check if required configuration corresponds to the stored packet type
    // If not, silently update radio packet type
    if( this->PacketType != packetParams->PacketType )
    {
        this->SetPacketType( packetParams->PacketType );
    }

    switch( packetParams->PacketType )
    {
    case PACKET_TYPE_GFSK:
        if( packetParams->Params.Gfsk.CrcLength == RADIO_CRC_2_BYTES_IBM )
        {
            SetCrcSeed( CRC_IBM_SEED );
            SetCrcPolynomial( CRC_POLYNOMIAL_IBM );
            crcVal = RADIO_CRC_2_BYTES;
        }
        else if(  packetParams->Params.Gfsk.CrcLength == RADIO_CRC_2_BYTES_CCIT )
        {
            SetCrcSeed( CRC_CCITT_SEED );
            SetCrcPolynomial( CRC_POLYNOMIAL_CCITT );
            crcVal = RADIO_CRC_2_BYTES_INV;
        }
        else
        {
            crcVal = packetParams->Params.Gfsk.CrcLength;
        }
        n = 9;
        // convert preamble length from byte to bit
        packetParams->Params.Gfsk.PreambleLength = packetParams->Params.Gfsk.PreambleLength << 3;

        buf[0] = ( packetParams->Params.Gfsk.PreambleLength >> 8 ) & 0xFF;
        buf[1] = packetParams->Params.Gfsk.PreambleLength;
        buf[2] = packetParams->Params.Gfsk.PreambleMinDetect;
        buf[3] = ( packetParams->Params.Gfsk.SyncWordLength << 3 ); // convert from byte to bit
        buf[4] = packetParams->Params.Gfsk.AddrComp;
        buf[5] = packetParams->Params.Gfsk.HeaderType;
        buf[6] = packetParams->Params.Gfsk.PayloadLength;
        buf[7] = crcVal;
        buf[8] = packetParams->Params.Gfsk.DcFree;
        break;
    case PACKET_TYPE_LORA:
        n = 6;
        buf[0] = ( packetParams->Params.LoRa.PreambleLength >> 8 ) & 0xFF;
        buf[1] = packetParams->Params.LoRa.PreambleLength;
        buf[2] = packetParams->Params.LoRa.HeaderType;
        buf[3] = packetParams->Params.LoRa.PayloadLength;
        buf[4] = packetParams->Params.LoRa.CrcMode;
        buf[5] = packetParams->Params.LoRa.InvertIQ;
        break;
    default:
    case PACKET_TYPE_NONE:
        return;
    }
    WriteCommand( RADIO_SET_PACKETPARAMS, buf, n );
}

void SX126x::SetCadParams( RadioLoRaCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin, RadioCadExitModes_t cadExitMode, uint32_t cadTimeout )
{
    uint8_t buf[7];

    buf[0] = ( uint8_t )cadSymbolNum;
    buf[1] = cadDetPeak;
    buf[2] = cadDetMin;
    buf[3] = ( uint8_t )cadExitMode;
    buf[4] = ( uint8_t )( ( cadTimeout >> 16 ) & 0xFF );
    buf[5] = ( uint8_t )( ( cadTimeout >> 8 ) & 0xFF );
    buf[6] = ( uint8_t )( cadTimeout & 0xFF );
    WriteCommand( RADIO_SET_CADPARAMS, buf, 7 );
    OperatingMode = MODE_CAD;
}

void SX126x::SetBufferBaseAddresses( uint8_t txBaseAddress, uint8_t rxBaseAddress )
{
    uint8_t buf[2];

#ifdef ADV_DEBUG
    printf("SetBufferBaseAddresses ");
#endif

    buf[0] = txBaseAddress;
    buf[1] = rxBaseAddress;
    WriteCommand( RADIO_SET_BUFFERBASEADDRESS, buf, 2 );
}

RadioStatus_t SX126x::GetStatus( void )
{
    uint8_t stat = 0;
    RadioStatus_t status;

    ReadCommand( RADIO_GET_STATUS, ( uint8_t * )&stat, 1 );
    status.Value = stat;
    return status;
}

int8_t SX126x::GetRssiInst( void )
{
    uint8_t rssi;

    ReadCommand( RADIO_GET_RSSIINST, ( uint8_t* )&rssi, 1 );
    return( -( rssi / 2 ) );
}

void SX126x::GetRxBufferStatus( uint8_t *payloadLength, uint8_t *rxStartBufferPointer )
{
    uint8_t status[2];

    ReadCommand( RADIO_GET_RXBUFFERSTATUS, status, 2 );

    // In case of LORA fixed header, the payloadLength is obtained by reading
    // the register REG_LR_PAYLOADLENGTH
    uint8_t regVal;
    ReadRegister( REG_LR_PACKETPARAMS, &regVal, 1);
    if( ( this->GetPacketType( ) == PACKET_TYPE_LORA ) && ( regVal >> 7 == 1 ) )
    {
        ReadRegister( REG_LR_PAYLOADLENGTH, payloadLength, 1 );
    }
    else
    {
        *payloadLength = status[0];
    }
    *rxStartBufferPointer = status[1];
}

void SX126x::GetPacketStatus( PacketStatus_t *pktStatus )
{
    uint8_t status[3];

    ReadCommand( RADIO_GET_PACKETSTATUS, status, 3 );

    pktStatus->packetType = this -> GetPacketType( );
    switch( pktStatus->packetType )
    {
        case PACKET_TYPE_GFSK:
            pktStatus->Params.Gfsk.RxStatus = status[0];
            pktStatus->Params.Gfsk.RssiSync = -status[1] / 2;
            pktStatus->Params.Gfsk.RssiAvg = -status[2] / 2;
            pktStatus->Params.Gfsk.FreqError = 0;
            break;

        case PACKET_TYPE_LORA:
            pktStatus->Params.LoRa.RssiPkt = -status[0] / 2;
            ( status[1] < 128 ) ? ( pktStatus->Params.LoRa.SnrPkt = status[1] / 4 ) : ( pktStatus->Params.LoRa.SnrPkt = ( ( status[1] - 256 ) /4 ) );
            pktStatus->Params.LoRa.SignalRssiPkt = -status[2] / 2;
            pktStatus->Params.LoRa.FreqError = FrequencyError;
            break;

        default:
        case PACKET_TYPE_NONE:
            // In that specific case, we set everything in the pktStatus to zeros
            // and reset the packet type accordingly
            memset( pktStatus, 0, sizeof( PacketStatus_t ) );
            pktStatus->packetType = PACKET_TYPE_NONE;
            break;
    }
}

RadioError_t SX126x::GetDeviceErrors( void )
{
    RadioError_t error;

    ReadCommand( RADIO_GET_ERROR, ( uint8_t * )&error, 2 );
    return error;
}

void SX126x::ClearIrqStatus( uint16_t irq )
{
    uint8_t buf[2];

    buf[0] = ( uint8_t )( ( ( uint16_t )irq >> 8 ) & 0x00FF );
    buf[1] = ( uint8_t )( ( uint16_t )irq & 0x00FF );
    WriteCommand( RADIO_CLR_IRQSTATUS, buf, 2 );
}
