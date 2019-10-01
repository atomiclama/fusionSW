// Based on BF bus.h


#pragma once

/**
 * @brief bus types supported 
 * 
 */
typedef enum {
    BUSTYPE_NONE = 0,   //!
    BUSTYPE_I2C,
    BUSTYPE_SPI,
} busType_e;

/**
 * @brief 
 * 
 */
typedef struct busDev_s {
    busType_e busType;

    uint8_t accessMutex;
    uint8_t reg;
    uint8_t data;
    uint8_t *buffer;
    uint8_t length;

// todo replace with chibios specific device 
    union {
        struct deviceSpi_s {
            SPI_TypeDef *instance;
            SPI_HandleTypeDef* handle;
            IO_t nssPin;
            IO_t irqPin;
            IO_t bsyPin;
        } spi;
        struct deviceI2C_s {
            I2CDevice device;
            uint8_t address;
        } i2c;
    } busType_u;

}busDev_t;

/**
 * @brief 
 * 
 * @param busDev 
 * @param reg 
 * @return true 
 * @return false 
 */
bool busReadRegister( busDev_t *busDev, uint8_t reg);
bool busWriteRegister( busDev_t *busDev, uint8_t reg, uint8_t data);
bool busReadBuffer( busDev_t *busDev, uint8_t reg, uint8_t *data, uint8_t length);
bool busWriteRegisterStart( busDev_t *busDev, uint8_t reg, uint8_t data);
bool busIsBusy( busDev_t *busDev, bool *error);

/**
 * @brief 
 * 
 * @param busDev 
 * @return true 
 * @return false 
 */
bool busReadReg( busDev_t *busDev);

/**
 * @brief 
 * 
 * @param busDev 
 * @return true 
 * @return false 
 */
bool busWriteReg( busDev_t *busDev);
bool busReadBuffer( busDev_t *busDev);
bool busWriteBuff( busDev_t *busDev);
bool busIsBusy( busDev_t *busDev);
bool busWaitBusy(busDev_t *busDev);