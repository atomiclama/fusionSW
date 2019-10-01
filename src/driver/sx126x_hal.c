
#include <stdbool.h>
#include <stdint.h>

#include "bus.h"


/**
 * @brief Write a value to a register.
 * 
 * This function takes the generic reg and val and translates to the format required by our bus access.
 * 
 * @param reg Register index.
 * @param val Value to write.
 */
static void writeReg(busDev_t *busDev, uint8_t reg, uint8_t val) {
    busWaitBusy(busDev);
    busDev->reg = 0x80 | reg;
    busDev->val = val;
    busWriteReg(busDev);
}

/**
 * @brief Read a value from a register.
 * 
 * This function takes the generic reg and val and translates to the format required by our bus access.
 * 
 * @param reg Register index.
 * @param val Value read from the register.
 */
static void readReg(busDev_t *busDev, uint8_t reg, uint8_t *val) {
    busWaitBusy(busDev);
    busDev->reg = reg;
    busReadReg(busDev);
    *val = busDev->val;
}
