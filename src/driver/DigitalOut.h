
#pragma once

// #include chibios gpio hal
#include "ch.h"
#include "hal.h"

class GPIO {
    public:

    GPIO (stm32_gpio_t * port, uint32_t pin) {
        this->port = port;
        this->pin = pin;
    }
    /** Return the output setting, represented as 0 or 1 (int)
     *
     *  @returns
     *    an integer representing the output setting of the pin,
     *    0 for logical 0, 1 for logical 1
     */
    int read()
    {
        // Thread safe / atomic HAL call
        internValue = palReadPad(port, pin);
        return internValue;
    }



    /** Set the output, specified as 0 or 1 (int)
     *
     *  @param value An integer specifying the pin output value,
     *      0 for logical 0, 1 (or any other non-zero value) for logical 1
     */
    void write(uint32_t value)
    {
        internValue = value;
        palWritePad(port, pin, internValue);   
    }

    /** A shorthand for write()
     * \sa DigitalOut::write()
     * @code
     *      DigitalIn  button(BUTTON1);
     *      DigitalOut led(LED1);
     *      led = button;   // Equivalent to led.write(button.read())
     * @endcode
     */
    GPIO &operator= (int value)
    {
        write(value);
        return *this;
    }

    /** A shorthand for write() using the assignment operator which copies the
     * state from the DigitalOut argument.
     * \sa DigitalOut::write()
     */
    GPIO &operator= (GPIO &rhs)
    {
        write(rhs.read());
        return *this;
    }

    /** A shorthand for read()
     * \sa DigitalOut::read()
     * @code
     *      DigitalIn  button(BUTTON1);
     *      DigitalOut led(LED1);
     *      led = button;   // Equivalent to led.write(button.read())
     * @endcode
     */
    operator int()
    {
        // Underlying call is thread safe
        return read();
    }

    protected:
    uint32_t pin;
    stm32_gpio_t * port;
    uint32_t internValue;
};





class DigitalOut : public GPIO {

    public:
        // just use the base class constructor
        DigitalOut (stm32_gpio_t * port, uint32_t pin) : GPIO (port, pin) {
            palSetPadMode(port, pin, PAL_MODE_OUTPUT_PUSHPULL);
        }
};


class DigitalIn : public GPIO {

    public:
        // just use the base class constructor
        DigitalIn (stm32_gpio_t * port, uint32_t pin) : GPIO (port, pin) {
            palSetPadMode(port, pin, PAL_MODE_INPUT);            
        }

        int wait(uint32_t level, sysinterval_t timeout) {
            
            if(palReadPad(port, pin) != level) {
                palEnablePadEvent(port, pin, PAL_EVENT_MODE_BOTH_EDGES);
                palWaitPadTimeout(port, pin, timeout);
                palDisablePadEvent(port, pin);
            }
            return true;
        }
};