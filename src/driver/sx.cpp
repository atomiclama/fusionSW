#include "sx.h"
#include "radio.h"


void SX12::WriteRegister( uint16_t address, uint8_t *buffer, uint16_t size ) {

    busyPin->wait(0, 100);

    txBuff[0] = RADIO_WRITE_REGISTER;
    txBuff[1] = ((address & 0xFF00 ) >> 8 );
    txBuff[2] = ( address & 0x00FF );
    txBuff[3] = ( 0 );

    aquire();
    send(txBuff, 4);
    send(buffer, size);
    release();
}


void SX12::ReadRegister( uint16_t address, uint8_t *buffer, uint16_t size ) {
   
    busyPin->wait(0, 100);  

    txBuff[0] = RADIO_READ_REGISTER;
    txBuff[1] = ((address & 0xFF00 ) >> 8 );
    txBuff[2] = ( address & 0x00FF );
    txBuff[3] = ( 0 );
    
    aquire();
    send(txBuff, 4);
    recieve(buffer, size);
    release();
    
}

void SX12::WriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    busyPin->wait(0, 100);

    txBuff[0] = command;

    aquire();
    send(txBuff, 1);
    send(buffer, size);
    release();
}

void SX12::ReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    busyPin->wait(0, 100);

    txBuff[0] = command;
    txBuff[1] = 0;

    aquire();
    send(txBuff, 2);
    recieve(buffer, size);
    release();    
}

// void SX12::WriteReg( uint16_t address, uint8_t value )
// {
//     uint8_t temp = value;
//     WriteRegister( address, &temp, 1 );
// }

// uint8_t SX12::ReadReg( uint16_t address )
// {
//     uint8_t data;
//     this->ReadRegister( address, &data, 1 );
//     return data;
// }

void SX12::WriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    busyPin->wait(0, 100);

    txBuff[0] = RADIO_WRITE_BUFFER;
    txBuff[1] = offset;

    aquire();   
    send(txBuff, 2);
    send(buffer,size);
    release();
}

void SX12::ReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    busyPin->wait(0, 100);

    txBuff[0] = RADIO_READ_BUFFER;
    txBuff[1] = offset;
    txBuff[1] = 0;

    aquire();   
    send(txBuff, 3);
    recieve(buffer,  size);
    release();
}

