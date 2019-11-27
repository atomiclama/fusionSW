

#include "spi.h"


SpiBaseClass::SpiBaseClass(SPIDriver *spip) {
    this->spip = spip;
}

void SpiBaseClass::aquire(SpiSettings &settings) {
    spiAcquireBus(spip); 
   
    config.circular = false;
    config.end_cb = NULL;
    config.cr1 = SPI_CR1_MSTR | SPI_CR1_BR_2;
    config.cr2 = 0;
    spiStart(spip, &config); 
}

void SpiBaseClass::send( void *txbuf, uint32_t n ) {
    spiSend(spip, n, txbuf);
}

void SpiBaseClass::recieve( void *rxbuf, uint32_t n ) {
    spiReceive(spip, n, rxbuf);
}


void SpiBaseClass::transfer(void *_bufout, void *_bufin, uint32_t _count) {
    spiExchange(spip, _count, _bufout, _bufin); 
}

void SpiBaseClass::release(void) {
    spiReleaseBus(spip);
}

void SpiNssClass::aquire(void) {
    SpiBaseClass::aquire(spiSettings);
    nssPin->write(0);
}

void SpiNssClass::release(void) {
    nssPin->write(1);
    SpiBaseClass::release();
}
