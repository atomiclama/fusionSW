
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "ch.h"
#include "hal.h"

#include "DigitalOut.h"

#include "ccportab.h"

class SpiSettings {
  public:
    uint32_t clk;       //specifies the spi bus maximum clock speed
    uint32_t bOrder;    //bit order (MSBFirst or LSBFirst)
    uint32_t dMode;     //one of the data mode
    uint32_t msb;       //set to 1 if msb first
};

class SpiBaseClass {
  public:
    SpiBaseClass(SPIDriver *spip);
   
    /** 
     * aquire access to underlying SPI HW .
     * mutex are used as there maybe multiple instances trying to acces the HW.
     */
    void aquire(SpiSettings& settings);

    /**
     * @brief  Transfer several bytes. One buffer contains the data to send and
     *         another one will contains the data received. 
     * @param  _bufout: pointer to the bytes to send.
     * @param  _bufin: pointer to the bytes received.
     */
    void transfer( void *_bufout, void *_bufin, uint32_t _count);

    void send( void *txbuf, uint32_t n );

    void recieve( void *rxbuf, uint32_t n );

    /**
     * @brief release the mutex.
     * 
     */
    void release(void);

  private:
    SPIDriver *spip;  // chibios spi instance
    SPIConfig config;
};


class SpiNssClass : public SpiBaseClass {
  public:
    
    SpiNssClass(SPIDriver *spiDevice) : SpiBaseClass (spiDevice) {
    }

    /* Contains various spiSettings for the same spi instance. 
    Using this to store the various configs / extern device.*/

    SpiSettings   spiSettings;

    void aquire(void);
    void release(void);

    DigitalOut *nssPin;     // CS pin associated to the configuration

  protected:

  private:
    
};
