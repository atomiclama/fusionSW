#include "ch.h"
#include "hal.h"

#include "outThread.h"

#include "ccportab.h"


class baseSPI {
    private:
        SPIDriver *spip;
        SPIConfig config;

    public:

        baseSPI(SPIDriver *spip) {
            this->spip = spip;
        }

        void aquire(void) {
            spiAcquireBus(spip);          
            
            config.circular = false;
            config.end_cb = NULL;
            config.cr1 = SPI_CR1_MSTR | SPI_CR1_BR_2;
            config.cr2 = 0;
            spiStart(spip, &config); 
        }

        void transfer(void *_bufout, void *_bufin, uint32_t _count) {
            spiExchange(spip, _count, _bufout, _bufin); 
        }

        void release(void) {
            spiReleaseBus(spip);
        }
};

class testClass : public baseSPI{
    public:

    testClass(SPIDriver *spiDevice) : baseSPI (spiDevice) {
    }

    void testFail(void){
        uint8_t tx[10] = {0xAA,2,0xAA,4,0xAA,6,0xAA,8};
        uint8_t rx[10];
        baseSPI::aquire();
        baseSPI::transfer(tx, rx, 10);
        baseSPI::release();
    }

    void testPass(void){
        static uint8_t tx[10] = {1,2,3,4,5,6,7,8};
        static uint8_t rx[10];
        baseSPI::aquire();
        baseSPI::transfer(tx, rx, 10);
        baseSPI::release();
    }
};

testClass testHw(&SPID2);

static THD_WORKING_AREA(waThread5, 512);

static THD_FUNCTION( Thread5, arg) {

    (void) arg;
    chRegSetThreadName("modem");

    while (true) {
        // testHw.testPass();
        testHw.testFail();
    }
}

void testThread_ini(void) {

    palSetPadMode(GPIOC, 1, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOC, 2, PAL_MODE_ALTERNATE(5));
    palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(5));

    chThdCreateStatic(waThread5, sizeof(waThread5), NORMALPRIO, Thread5, NULL);
}
