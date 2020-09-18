/*******************************************************************************
Copyright 2016-2018 anxzhu (github.com/anxzhu)
Copyright 2018 Valerio Nappi (github.com/5N44P) (changes)
Based on segment-lcd-with-ht1621 from anxzhu (2016-2018)
(https://github.com/anxzhu/segment-lcd-with-ht1621)

Partially rewritten and extended by Valerio Nappi (github.com/5N44P) in 2018
https://github.com/5N44P/ht1621-7-seg

Refactored. Removed dependency on any MCU hardware by Viacheslav Balandin
https://github.com/hedgehogV/HT1621-lcd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#ifndef HT1621_H_
#define HT1621_H_

#include <cstdint>
#include <cstddef>

#include "WVT_Display.h"

class HT1621
{
public:
    using pPinSet = void(bool);
    using pInterface = void(uint8_t *, uint8_t);

    /**
     * @brief Construct a new HT1621 object
     *
     * Starts the lcd with the pin assignment declared. The backlight pin is optional
     *
     * @param pCs - pointer to CS pin toggle function
     * @param pSck - pointer to SCK pin toggle function
     * @param pMosi - pointer to MOSI pin toggle function
     * @param pBacklight - pointer to backlight pin toggle function. Optional
     */
    HT1621(pPinSet *pCs, pPinSet *pSck, pPinSet *pMosi, pPinSet *pBacklight = nullptr);

    /**
     * @brief Construct a new HT1621::HT1621 object
     *
     * Starts the lcd with SPI interface. CS and backlight pins are optional
     *
     * @param pSpi - pointer to SPI write function
     * @param pCs - pointer to CS pin toggle function. Optional if SPI has hardware CS configured
     * @param pBacklight - pointer to backlight pin toggle function. Optional
     */
    HT1621(pInterface *pSpi, pPinSet *pCs = nullptr, pPinSet *pBacklight = nullptr);

    /**
     * @brief Turns on the display (doesn't affect the backlight)
     */
    void displayOn();

    /**
     * @brief Turns off the display (doesn't affect the backlight)
     */
    void displayOff();

    /**
     * @brief Turns on the backlight
     */
    void backlightOn();

    /**
     * @brief Turns off the backlight
     */
    void backlightOff();

    /**
     * @brief Show battery level.
     *
     * @param percents - battery charge state. May vary from 0 up to 100
     */
    void batteryLevel(uint8_t percents);

    /**
     * @brief Print string (up to 6 characters)
     *
     * @param str String to be displayed.
     * Allowed: letters, digits, space, minus, underscore
     * Not allowed symbols will be displayed as spaces. See symbols appearance in README.md
     */
    void print(const char *str);

    /**
     * @brief Prints a signed integer between -99999 and 999999.
     * Larger and smaller values will be displayed as -99999 and 999999
     *
     * @param num - number to be printed
     */
    void print(int32_t num);

    /**
     * @brief Prints a float with 0 to 3 decimals, based on the `precision` parameter. Default value is 3
     * This method may be slow on many systems. Try to avoid float usage.
     * You may use `void print(int32_t multiplied_float, uint32_t multiplier)` instead
     *
     * @param num  - number to be printed
     * @param precision - precision of the number
     */
    void print(float num, uint8_t precision = 3);

    /**
     * @brief Prints number with dot. Use it instead float. Float type usage may slow down many systems
     */
    void print(int32_t multiplied_float, uint32_t multiplier);

    /**
     * @brief Clears the display
     */
    void clear();

    void DisplayMinMax(display_mode_t mode, min_max_t disp);

private:
    static const size_t DISPLAY_SIZE = 9;                           // 16 * 8  = 128 symbols on display plus 2 byte for address
    static const size_t SYS_SIZE = 2;                               // 2 byte for address and commands
    static const size_t DATA_SIZE = 16;                             // 16 * 8  = 128 symbols on display
    static const size_t DISPLAY_BUFFER_SIZE = DATA_SIZE + SYS_SIZE; //  plus 2 byte for address
    uint8_t buffer[DISPLAY_BUFFER_SIZE] = {};                       // buffer where display data will be stored

    // defines to set display pin to low or high level
    const bool LOW = 0;
    const bool HIGH = 1;

    pPinSet *pCsPin = nullptr;           // SPI CS pin
    pPinSet *pSckPin = nullptr;          // for display it is WR pin
    pPinSet *pMosiPin = nullptr;         // for display it is Data pin
    pPinSet *pBacklightPin = nullptr;    // display backlight pin
    pInterface *pSpiInterface = nullptr; // ptr to SPI_tx implementation

    // the most low-level function. Sends array of bytes into display
    void wrBytes(uint8_t *ptr, uint8_t size);
    // write buffer to the display
    void wrBuffer();
    // write command sequence to display
    void wrCmd(uint8_t cmd);
    // set decimal separator. Used when print float numbers
    void decimalSeparator(uint8_t dpPosition);
    // takes the buffer and puts it straight into the driver
    void update();
    // remove battery symbol from display buffer
    void batteryBufferClear();
    // remove dot symbol from display buffer
    void dotsBufferClear();
    // remove all symbols from display buffer except battery and dots
    void lettersBufferClear();
    //Clear all segments
    void AllClear();
    // coverts buffer symbols to format, which can be displayed by lcd
    void bufferToAscii(const char *in, uint8_t *out);
};

#endif
