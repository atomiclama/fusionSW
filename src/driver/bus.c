

// This file is based on code from Cleanflight/BetaFlight.

#include <stdbool.h>
#include <stdint.h>

#include "bus.h"
#include "bus_spi.h"
#include "bus_i2c.h"

// todo replace with specific calls to chibios device drivers and add mutex handling.

bool  busReadRegister( busDev_t *busDev, uint8_t reg)
{
    uint8_t data;
    switch (busDev->busType) {
        case BUSTYPE_SPI:
            data = spiBusReadRegister(busDev, reg & 0x7f);

        case BUSTYPE_I2C:
            return i2cBusReadRegister(busDev, reg, data);

        default:
            return false;
    }
    busDev->data = data;
    return true;
}

bool busWriteRegister( busDev_t *busDev, uint8_t reg, uint8_t data)
{
    switch (busDev->busType) {
        case BUSTYPE_SPI:
            return spiBusWriteRegister(busDev, reg & 0x7f, data);

        case BUSTYPE_I2C:
            return i2cBusWriteRegister(busDev, reg, data);

        default:
            return false;
    }
}

bool busReadBuffer( busDev_t *busDev, uint8_t reg, uint8_t *data, uint8_t length)
{
    switch (busDev->busType) {
        case BUSTYPE_SPI:
            return spiBusReadRegisterBuffer(busDev, reg | 0x80, data, length);

        case BUSTYPE_I2C:
            return i2cBusReadRegisterBuffer(busDev, reg, data, length);

        default:
            return false;
    }
}

bool busWriteBuffer( busDev_t *busDev, uint8_t reg, uint8_t *data, uint8_t length)
{
    switch (busDev->busType) {
        case BUSTYPE_SPI:
            return spiBusWriteRegisterBuffer(busDev, reg | 0x80, data, length);

        case BUSTYPE_I2C:
            return i2cBusWriteRegisterBuffer(busDev, reg, data, length);

        default:
            return false;
    }
}

bool busIsBusy( busDev_t *busDev, bool *error)
{
    switch (busDev->busType) {
        case BUSTYPE_SPI:
            return false;

        case BUSTYPE_I2C:
            return i2cBusBusy(busDev, error);

        default:
            return false;
    }
}

bool busWaitBusy( busDev_t *busDev){
    switch (busDev->busType) {
        case BUSTYPE_SPI:
            while(busDev->busType_u.spi.bsyPin == true){
                osWait(10);
            }
            return false;

        case BUSTYPE_I2C:
            bool error;
            while(i2cBusBusy(busDev, &error)){
                osWait(10);
            }
            return false;

        default:
            return false;
    }
}
