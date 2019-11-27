
#include "spi.h"
#include "radio.h"
#include "DigitalOut.h"

class SX12 : public SpiNssClass {

public:
    SX12(SPIDriver *spiDevice) : SpiNssClass (spiDevice) {
    }
    
    // void Test(void);

    DigitalIn *busyPin; 

    /*!
     * \brief Writes multiple radio registers starting at address
     *
     * \param [in]  address       First Radio register address
     * \param [in]  buffer        Buffer containing the new register's values
     * \param [in]  size          Number of registers to be written
     */
    void WriteRegister( uint16_t address, uint8_t *buffer, uint16_t size );
   
    /*!
     * \brief Reads multiple radio registers starting at address
     *
     * \param [in]  address       First Radio register address
     * \param [out] buffer        Buffer where to copy the registers data
     * \param [in]  size          Number of registers to be read
     */
    void ReadRegister( uint16_t address, uint8_t *buffer, uint16_t size );

    /*!
     * \brief Send a command that write data to the radio
     *
     * \param [in]  opcode        Opcode of the command
     * \param [in]  buffer        Buffer to be send to the radio
     * \param [in]  size          Size of the buffer to send
     */
    void WriteCommand( RadioCommands_t opcode, uint8_t *buffer, uint16_t size );

    /*!
     * \brief Send a command that read data from the radio
     *
     * \param [in]  opcode        Opcode of the command
     * \param [out] buffer        Buffer holding data from the radio
     * \param [in]  size          Size of the buffer
     */
    void ReadCommand( RadioCommands_t opcode, uint8_t *buffer, uint16_t size );

    // void WriteReg( uint16_t address, uint8_t value );

    // uint8_t ReadReg( uint16_t address );

    /*!
     * \brief Writes Radio Data Buffer with buffer of size starting at offset.
     *
     * \param [in]  offset        Offset where to start writing
     * \param [in]  buffer        Buffer pointer
     * \param [in]  size          Buffer size
     */
    void WriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size );
    
    /*!
     * \brief Reads Radio Data Buffer at offset to buffer of size
     *
     * \param [in]  offset        Offset where to start reading
     * \param [out] buffer        Buffer pointer
     * \param [in]  size          Buffer size
     */
    void ReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size );

protected:
    
     uint8_t txBuff[ 4];
    //  uint8_t rxBuff[ 4];

};

