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

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    void (*PinCs)(bool);
    void (*PinSck)(bool);
    void (*PinMosi)(bool);
} HT1620_HAL_st;

/**
     * @brief Construct a new HT1621 object
     *
     * Starts the lcd with the pin assignment declared. The backlight pin is optional
     *  
     * @param hal_ptr 
     */
void HT1620Init(HT1620_HAL_st *hal_ptr);

/**
     * @brief Turns on the display (doesn't affect the backlight)
     */
void HT1620displayOn();

/**
     * @brief Turns off the display (doesn't affect the backlight)
     */
void HT1620displayOff();

/**
     * @brief Show battery level.
     *
     * @param percents - battery charge state. May vary from 0 up to 100
     */
void HT1620batteryLevel(uint8_t percents);

/**
     * @brief Print string (up to 6 characters)
     *
     * @param str String to be displayed.
     * Allowed: letters, digits, space, minus, underscore
     * Not allowed symbols will be displayed as spaces. See symbols appearance in README.md
     */
void HT1620printStr(const char *str);

/**
     * @brief Prints a signed integer between -99999 and 999999.
     * Larger and smaller values will be displayed as -99999 and 999999
     *
     * @param num - number to be printed
     */
void HT1620printNum(int32_t num);

/**
     * @brief Prints a float with 0 to 3 decimals, based on the `precision` parameter. Default value is 3
     * This method may be slow on many systems. Try to avoid float usage.
     * You may use `void print(int32_t multiplied_float, uint32_t multiplier)` instead
     *
     * @param num  - number to be printed
     * @param precision - precision of the number
     */
void HT1620printFloat(float num, uint8_t precision);

/**
     * @brief Prints number with dot. Use it instead float. Float type usage may slow down many systems
     */
void HT1620printFixedPoint(int32_t multiplied_float, uint32_t multiplier);

/**
     * @brief Clears the display
     */
void HT1620clear();

/*!
    * \brief display min or max value
    *
    * \param enable true for enable symbols, false for disable all symbols
    * \param mode true if russian or false if english symbols
    * \param min true if display min, false if display max
    */
void HT1620DispMinMax(bool enable, bool mode, bool min);

/*!
    * \brief display burst or proriv
    *
    * \param enable true for enable symbols, false for disable all symbols
    * \param mode true if russian or false if english symbols
    */
void HT1620DispBurst(bool enable, bool mode);

/*!
    * \brief display leak or TEch`
    *
    * \param enable true for enable symbols, false for disable all symbols
    * \param mode true if russian or false if english symbols
    */
void HT1620DispLeak(bool enable, bool mode);

/*!
    * \brief display reverse or obr
    *
    * \param enable true for enable symbols, false for disable all symbols
    * \param mode true if russian or false if english symbols
    */
void HT1620DispRev(bool enable, bool mode);

/*!
    * \brief display snowflake frost symbol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispFrost(bool enable);

/*!
    * \brief display snowflake frost symbol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispQ(bool enable);

/*!
    * \brief display version or no
    *
    * \param enable true for enable symbols, false for disable all symbols
    * \param mode true if russian or false if english symbols
    */
void HT1620DispVer(bool enable, bool mode);

/*!
    * \brief display serial number or B
    *
    * \param enable true for enable symbols, false for disable all symbols
    * \param mode true if russian or false if english symbols
    */
void HT1620DispSN(bool enable, bool mode);

/*!
    * \brief display warning sybmol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispWarn(bool enable);

/*!
    * \brief display magnet sybmol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispMagn(bool enable);

/*!
    * \brief display left sybmol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispLeft(bool enable);

/*!
    * \brief display right sybmol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispRight(bool enable);

/*!
    * \brief display no water sybmol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispNoWater(bool enable);
/*!
    * \brief display CRC symbol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispCRC(bool enable);
/*!
    * \brief display delta symbol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispDelta(bool enable);

/*!
    * \brief display T symbol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispT(bool enable);

/*!
    * \brief display 1 sybmol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620Disp1(bool enable);

/*!
    * \brief display T2 sybmol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispT2(bool enable);

/*!
    * \brief display NBFI radio sybmol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispNBFi(bool enable);

/*!
    * \brief display NB-iot radio sybmol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispNBIoT(bool enable);

/*!
    * \brief display level of signal
    *
    * \param percents if 0 - no display, 0--30 one segment, 30--60 two segments, more than 60 - 3 segments
    */
void HT1620SignalLevel(uint8_t percents);

/*!
    * \brief display DegreePoint sybmol
    *
    * \param enable true for enable symbols, false for disable all symbols
    */
void HT1620DispDegreePoint(bool enable);

/*!
    * \brief display energy in GJ or Kkal
    *
    * \param enable true for enable symbols, false for disable all symbols
    * \param mode true if Kkal or false if GJ symbols
    * \param perH true for display per hour symbol
    */
void HT1620DispEnergyJ(bool enable, bool mode, bool perH);

/*!
    * \brief display energy in MkWh
    *
    * \param enable true for enable symbols, false for disable all symbols
    * \param M if true than display MW else display kW
    * \param perH if true than display Wh else display W
    */
void HT1620DispEnergyW(bool enable, bool M, bool perH);

/*!
    * \brief display flow in m3 per hour
    *
    * \param enable true for enable symbols, false for disable all symbols
    * \param mode if true than display per chas else display per hour
    * \param perH if true than display per time else display only volume
    */
void HT1620DispFlowM3(bool enable, bool mode, bool perH);

/*!
     * \brief display flow in GAL per min
     *
     * \param enable true for enable symbols, false for disable all symbols
     * \param perH if true than display per minute else display only volume
     */
void HT1620DispFlowGAL(bool enable, bool perH);

/*!
     * \brief display flow in FT3 per min
     *
     * \param enable true for enable symbols, false for disable all symbols
     * \param perH if true than display per minute else display only volume
     */
void HT1620DispFlowFT(bool enable, bool perH);

/*!
     * \brief display britan meter unit for heat
     *
     * \param enable true for enable symbols, false for disable all symbols
     */
void HT1620DispMMBTU(bool enable);

/*!
     * \brief display u.s. gallons or gallons
     *
     * \param enable true for enable symbols, false for disable all symbols
     * \param mode if true then display U.S. else display only GALLONS
     */
void HT1620DispGal(bool enable, bool mode);

#define DISPLAY_SIZE 9                             // 16 * 8  = 128 symbols on display plus 2 byte for address
#define SYS_SIZE 2                                 // 2 byte for address and commands
#define DATA_SIZE 16                               // 16 * 8  = 128 symbols on display
#define DISPLAY_BUFFER_SIZE (DATA_SIZE + SYS_SIZE) //  plus 2 byte for address

// defines to set display pin to low or high level
#define LOW 0
#define HIGH 1

#endif
