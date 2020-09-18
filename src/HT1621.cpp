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

#include "HT1621.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>

/**
 * @brief CALCULATION DEFINES BLOCK
 */
constexpr int MAX_NUM = 999999999;
constexpr int MIN_NUM = -999999999;

constexpr size_t PRECISION_MAX_POSITIVE = 5; // TODO: find better names
constexpr size_t PRECISION_MAX_NEGATIVE = 5;
constexpr size_t PRECISION_MIN = 1;

constexpr size_t BITS_PER_BYTE = 8;

#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#endif //MIN

#ifndef SET_BIT
#define SET_BIT(REG, BIT) ((REG) |= (BIT))
#endif //SET_BIT

#ifndef CLEAR_BIT
#define CLEAR_BIT(REG, BIT) ((REG) |= (BIT))
#endif //CLEAR_BIT

#define LCD_SWITCH(EN, POS, SEG) ((EN) ? (SET_BIT(buffer[POS], SEG)) : (CLEAR_BIT(buffer[POS], SEG)))
//#define LCD_TOGGLE(EN, POS1, SEG1, POS2, SEG2) ((EN) ? ({SET_BIT(buffer[POS1], SEG1); CLEAR_BIT(buffer[POS2], SEG2); }) : ({SET_BIT(buffer[POS2], SEG2); CLEAR_BIT(buffer[POS1], SEG1); }))
inline void HT1621::LCD_TOGGLE(bool EN, uint8_t POS1, uint8_t SEG1, uint8_t POS2, uint8_t SEG2)
{
    if (EN)
    {
        SET_BIT(buffer[POS1], SEG1);
        CLEAR_BIT(buffer[POS2], SEG2);
    }
    else
    {
        SET_BIT(buffer[POS2], SEG2);
        CLEAR_BIT(buffer[POS1], SEG1);
    }
}
/**
 * @brief DISPLAY HARDWARE DEFINES BLOCK
 */
#define BIAS 0x52    //0b1000 0101 0010  1/3duty 4com
#define SYSDIS 0x00  //0b1000 0000 0000  Turn off both system oscillator and LCD bias generator
#define SYSEN 0x02   //0b1000 0000 0010  Turn on system oscillator
#define LCDOFF 0x04  //0b1000 0000 0100  Turn off LCD bias generator
#define LCDON 0x06   //0b1000 0000 0110  Turn on LCD bias generator
#define XTAL 0x28    //0b1000 0010 1000  System clock source, crystal oscillator
#define RC256 0x30   //0b1000 0011 0000  System clock source, on-chip RC oscillator
#define TONEON 0x12  //0b1000 0001 0010  Turn on tone outputs
#define TONEOFF 0x10 //0b1000 0001 0000  Turn off tone outputs
#define WDTDIS1 0x0A //0b1000 0000 1010  Disable WDT time-out flag output

#define MODE_CMD 0x08
#define MODE_DATA 0x05

#define ADR0_SHIFT 0
#define ADR1_SHIFT 7

#define BAT1_SEG (1 << 5)
#define BAT2_SEG (1 << 1)
#define BAT3_SEG (1 << 2)
#define BAT4_SEG (1 << 6)

#define BAT14_POS 13
#define BAT23_POS 14

#define P1_SEG (1 << 0)
#define P1_POS 7
#define P2_SEG (1 << 0)
#define P2_POS 8
#define P3_SEG (1 << 0)
#define P3_POS 9
#define P4_SEG (1 << 0)
#define P4_POS 10
#define P5_SEG (1 << 0)
#define P5_POS 11

#define NUM1FGE_SEG (7 << 5) //0b11100000
#define NUM1FGE_POS 2
#define NUM1ABC_SEG (0x7 << 0) //0b00000111
#define NUM1D_SEG (0x08 << 0)  //0b00001000
#define NUM1ABCD_POS 3

#define MINUS_SEG (1 << 0)
#define MINUS_POS 4

#define MIN_RU_SEG (1 << 0)
#define MIN_RU_POS 5
#define MAX_RU_SEG (1 << 4)
#define MAX_RU_POS 2

#define MIN_EN_SEG (1 << 0)
#define MIN_EN_POS 6
#define MAX_EN_SEG (1 << 0)
#define MAX_EN_POS 3

#define BURST_RU_SEG (1 << 4)
#define BURST_RU_POS 1
#define BURST_EN_SEG (1 << 7)
#define BURST_EN_POS 1

#define LEAK_RU_SEG (1 << 6)
#define LEAK_RU_POS 1
#define LEAK_EN_SEG (1 << 3)
#define LEAK_EN_POS 1

#define REV_RU_SEG (1 << 2)
#define REV_RU_POS 1
#define REV_EN_SEG (1 << 1)
#define REV_EN_POS 1

#define FROST_SEG (1 << 0)
#define FROST_POS 2

#define Q_SEG (1 << 3)
#define Q_POS 2

#define VER_RU_SEG (1 << 1)
#define VER_RU_POS 2
#define VER_EN_SEG (1 << 2)
#define VER_EN_POS 2

#define SN_RU_SEG (1 << 5)
#define SN_RU_POS 1
#define SN_EN_SEG (1 << 7)
#define SN_EN_POS 16

#define WARN_SEG (1 << 0)
#define WARN_POS 17

#define MAGNET_SEG (1 << 4)
#define MAGNET_POS 16

#define LEFT_SEG (1 << 0)
#define LEFT_POS 16

#define RIGHT_SEG (1 << 0)
#define RIGHT_POS 15

#define NOWATER_SEG (1 << 4)
#define NOWATER_POS 15

#define CRC_SEG (1 << 6)
#define CRC_POS 16

#define DELTA_SEG (1 << 5)
#define DELTA_POS 16

#define T_SEG (1 << 2)
#define T_POS 16

#define T1_SEG (1 << 3)
#define T1_POS 16

#define T2_SEG (1 << 7)
#define T2_POS 15

#define NBFI_SEG (1 << 4)
#define NBFI_POS 14

#define NBIOT_SEG (1 << 0)
#define NBIOT_POS 14

#define SIG1_SEG (1 << 7)
#define SIG1_POS 14
#define SIG2_SEG (1 << 3)
#define SIG2_POS 14
#define SIG3_SEG (1 << 7)
#define SIG3_POS 13

#define DEGREE_SEG (1 << 6)
#define DEGREE_POS 15

#define GCAL_SEG (1 << 2)
#define GCAL_POS 13
#define GCAL_H_SEG (1 << 6)
#define GCAL_H_POS 12

#define GJ_SEG (1 << 5)
#define GJ_POS 15
#define GJ_H_SEG (1 << 1)
#define GJ_H_POS 13

#define KW_SEG (1 << 5)
#define KW_POS 11
#define MW_SEG (1 << 6)
#define MW_POS 11
#define W_SEG (1 << 5)
#define W_POS 12
#define WH_SEG (1 << 1)
#define WH_POS 12

#define GAL_SEG (1 << 2)
#define GAL_POS 13
#define GAL_PM_SEG (1 << 6)
#define GAL_PM_POS 12

#define M3_SEG (1 << 0)
#define M3_POS 12
#define M3_H_SEG (1 << 3)
#define M3_H_POS 12
#define M3_H_EN_SEG (1 << 4)
#define M3_H_EN_POS 12

#define FT3_SEG (1 << 2)
#define FT3_POS 15
#define FT3_PM_SEG (1 << 1)
#define FT3_PM_POS 15

#define MMBTU_SEG (1 << 3)
#define MMBTU_POS 15

#define GALLONS_SEG (1 << 5)
#define GALLONS_POS 14
#define US_SEG (1 << 6)
#define US_POS 14

#define ALL_CLEAR_SEG (0xfe << 0) //0b11111110
#define ALL_CLEAR_POS 1

#define DOT_SEG 0x80

static const char ascii[] =
    {
        /*       0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
        /*      ' '   ' '   ' '   ' '   ' '   ' '   ' '   ' '   ' '   ' '   ' '   ' '   ' '   '-'   ' '   ' ' */
        /*2*/ 0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x02,
        0x00,
        0x00,
        /*      '0'   '1'   '2'   '3'   '4'   '5'   '6'   '7'   '8'   '9'   ' '   ' '   ' '   ' '   ' '   ' ' */
        /*3*/ 0x7D,
        0x60,
        0x3e,
        0x7a,
        0x63,
        0x5b,
        0x5f,
        0x70,
        0x7f,
        0x7b,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        /*      ' '   'A'   'B'   'C'   'D'   'E'   'F'   'G'   'H'   'I'   'J'   'K'   'L'   'M'   'N'   'O' */
        /*4*/ 0x00,
        0x77,
        0x4f,
        0x1d,
        0x6e,
        0x1f,
        0x17,
        0x5d,
        0x47,
        0x05,
        0x68,
        0x27,
        0x0d,
        0x54,
        0x75,
        0x4e,
        /*      'P'   'Q'   'R'   'S'   'T'   'U'   'V'   'W'   'X'   'Y'   'Z'   ' '   ' '   ' '   ' '   '_' */
        /*5*/ 0x37,
        0x73,
        0x06,
        0x59,
        0x0f,
        0x6d,
        0x23,
        0x29,
        0x67,
        0x6b,
        0x3c,
        0x00,
        0x00,
        0x00,
        0x00,
        0x08,
        /*      ' '   'A'   'B'   'C'   'D'   'E'   'F'   'G'   'H'   'I'   'J'   'K'   'L'   'M'   'N'   'O' */
        /*6*/ 0x00,
        0x77,
        0x4f,
        0x1d,
        0x6e,
        0x1f,
        0x17,
        0x5d,
        0x47,
        0x05,
        0x68,
        0x27,
        0x0d,
        0x54,
        0x75,
        0x4e,
        /*      'P'   'Q'   'R'   'S'   'T'   'U'   'V'   'W'   'X'   'Y'   'Z'   ' '   ' '   ' '   ' '   ' ' */
        /*7*/ 0x37,
        0x73,
        0x06,
        0x59,
        0x0f,
        0x6d,
        0x23,
        0x29,
        0x67,
        0x6b,
        0x3c,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
};

#define ASCII_SPACE_SYMBOL 0x00

#define LITTLE_ENDIAN

#if !defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN)
#error "Unable to determine endian. Set it manually"
#endif

union tCmdSeq
{
    struct __attribute__((packed))
    {
#if defined LITTLE_ENDIAN
        uint16_t padding : 4;
        uint16_t data : 8;
        uint16_t type : 4;
#elif defined BIG_ENDIAN
        uint16_t type : 4;
        uint16_t data : 8;
        uint16_t padding : 4;
#endif
    };
    uint8_t arr[2];
};

union tDataSeq
{
    struct //__attribute__((packed))
    {
#if defined LITTLE_ENDIAN
        uint8_t padding : 7;
        uint64_t data1 : 64;
        uint64_t data0 : 64;
        uint8_t addr : 6;
        uint8_t type : 3;
#elif defined BIG_ENDIAN
        uint8_t type : 3;
        uint8_t addr : 6;
        uint16_t data0 : 16;
        uint16_t data1 : 16;
        uint16_t data2 : 16;
        uint8_t padding : 7;
#endif
    };
    uint8_t arr[18];
};

HT1621::HT1621(pPinSet *pCs, pPinSet *pSck, pPinSet *pMosi, pPinSet *pBacklight)
{
    pCsPin = pCs;
    pSckPin = pSck;
    pMosiPin = pMosi;
    pBacklightPin = pBacklight;

    wrCmd(BIAS);
    wrCmd(RC256);
    wrCmd(SYSDIS);
    wrCmd(WDTDIS1);
    wrCmd(SYSEN);
    wrCmd(LCDON);
}

HT1621::HT1621(pInterface *pSpi, pPinSet *pCs, pPinSet *pBacklight)
{
    pSpiInterface = pSpi;
    pCsPin = pCs;
    pBacklightPin = pBacklight;

    wrCmd(BIAS);
    wrCmd(RC256);
    wrCmd(SYSDIS);
    wrCmd(WDTDIS1);
    wrCmd(SYSEN);
    wrCmd(LCDON);
}

void HT1621::backlightOn()
{
    if (pBacklightPin)
        pBacklightPin(HIGH);
}

void HT1621::backlightOff()
{
    if (pBacklightPin)
        pBacklightPin(LOW);
}

void HT1621::displayOn()
{
    wrCmd(LCDON);
}

void HT1621::displayOff()
{
    wrCmd(LCDOFF);
}

void HT1621::wrBytes(uint8_t *ptr, uint8_t size)
{
    // probably need to use microsecond delays in this method
    // after every pin toggle function to give display
    // driver time for data reading. But current solution works for me

    // TODO: check wrong size
    // lcd driver expects data in reverse order
    std::reverse(ptr, ptr + size);

    if (pCsPin)
        pCsPin(LOW);

    if (pSpiInterface)
        pSpiInterface(ptr, size);
    else if (pSckPin && pMosiPin)
    {
        for (size_t k = 0; k < size; k++)
        {
            // send bits into display one by one
            for (size_t i = 0; i < BITS_PER_BYTE; i++)
            {
                pSckPin(LOW);
                pMosiPin((ptr[k] & (0x80 >> i)) ? HIGH : LOW);
                pSckPin(HIGH);
            }
        }
    }

    if (pCsPin)
        pCsPin(HIGH);
}

void HT1621::wrBuffer()
{
    buffer[0] = MODE_DATA | (0 << ADR0_SHIFT);
    buffer[1] |= (0 << ADR1_SHIFT);

    wrBytes(buffer, sizeof(buffer));
}

void HT1621::wrCmd(uint8_t cmd)
{
    tCmdSeq CommandSeq = {};
    CommandSeq.type = MODE_CMD;
    CommandSeq.data = cmd;

    wrBytes(CommandSeq.arr, sizeof(tCmdSeq));
}

void HT1621::batteryLevel(uint8_t percents)
{
    batteryBufferClear();
    SET_BIT(buffer[BAT14_POS], BAT4_SEG);
    if (percents > 75)
    {
        SET_BIT(buffer[BAT14_POS], BAT1_SEG);
    }
    if (percents > 50)
    {
        SET_BIT(buffer[BAT23_POS], BAT2_SEG);
    }
    if (percents > 25)
    {
        SET_BIT(buffer[BAT23_POS], BAT3_SEG);
    }
    wrBuffer();
}

void HT1621::batteryBufferClear()
{
    CLEAR_BIT(buffer[BAT14_POS], BAT1_SEG | BAT4_SEG);
    CLEAR_BIT(buffer[BAT23_POS], BAT2_SEG | BAT3_SEG);
}

void HT1621::dotsBufferClear()
{
    for (size_t i = 0; i < PRECISION_MAX_POSITIVE; i++)
    {
        CLEAR_BIT(buffer[P1_POS + i], P1_SEG);
    }
}

void HT1621::lettersBufferClear()
{
    for (size_t i = 0; i < DISPLAY_SIZE; i++)
    {
        CLEAR_BIT(buffer[NUM1FGE_POS + i], NUM1FGE_SEG);
        CLEAR_BIT(buffer[NUM1ABCD_POS + i], NUM1ABC_SEG);
        CLEAR_BIT(buffer[NUM1ABCD_POS + i], NUM1D_SEG);
    }
}

void HT1621::AllClear()
{
    CLEAR_BIT(buffer[ALL_CLEAR_POS], ALL_CLEAR_SEG);
    for (size_t i = 0; i < DATA_SIZE; i++)
    {
        buffer[i + SYS_SIZE] = 0;
    }
}
void HT1621::clear()
{
    AllClear();

    wrBuffer();
}

void HT1621::bufferToAscii(const char *in, uint8_t *out)
{
    for (size_t i = 0; i < MIN(DISPLAY_SIZE, strlen(in)); i++)
    {
        char c = in[i];
        // Handle situation when char is out of displayable ascii table part.
        // Show space instead
        if ((c < ' ') || (c - ' ' > (int)sizeof(ascii)))
        {
            CLEAR_BIT(out[NUM1FGE_POS + i], NUM1FGE_SEG);
            CLEAR_BIT(out[NUM1ABCD_POS + i], NUM1ABC_SEG);
            CLEAR_BIT(out[NUM1ABCD_POS + i], NUM1D_SEG);
        }
        else
        {
            //02345678 conversion to 67805234
            SET_BIT(out[NUM1FGE_POS + i], NUM1FGE_SEG & (ascii[c - ' ']) << 5);  // shift 5 for changing data format from library to our display
            SET_BIT(out[NUM1ABCD_POS + i], NUM1ABC_SEG & (ascii[c - ' ']) >> 4); // shift 3 for changing data format from library to our display
            SET_BIT(out[NUM1ABCD_POS + i], NUM1D_SEG & (ascii[c - ' ']));        // 4 bit not shifted in our display
        }
    }
}

void HT1621::print(const char *str)
{
    dotsBufferClear();
    lettersBufferClear();
    bufferToAscii(str, buffer);
    wrBuffer();
}

void HT1621::print(int32_t num)
{
    if (num > MAX_NUM)
        num = MAX_NUM;
    if (num < MIN_NUM)
        num = MIN_NUM;

    dotsBufferClear();
    lettersBufferClear();

    char str[DISPLAY_SIZE + 1] = {};
    snprintf(str, sizeof(str), "%6li", num);

    bufferToAscii(str, buffer);

    wrBuffer();
}

void HT1621::print(float num, uint8_t precision)
{
    if (num >= 0 && precision > PRECISION_MAX_POSITIVE)
        precision = PRECISION_MAX_POSITIVE;
    else if (num < 0 && precision > PRECISION_MAX_NEGATIVE)
        precision = PRECISION_MAX_NEGATIVE;

    if (num < MIN_NUM / 10)
    {
        num = num * (-1);
        SET_BIT(buffer[MINUS_POS], MINUS_SEG);
    }
    else
    {
        CLEAR_BIT(buffer[MINUS_POS], MINUS_SEG);
    }

    int32_t integerated = (int32_t)(num * pow(10, precision));

    if (integerated > MAX_NUM)
        integerated = MAX_NUM;
    if (integerated < MIN_NUM)
        integerated = MIN_NUM;

    print(integerated);
    decimalSeparator(precision);

    wrBuffer();
}

// TODO: make multiplier more strict.
void HT1621::print(int32_t multiplied_float, uint32_t multiplier)
{
    uint8_t precision = 0;

    if (multiplier == 100000)
        precision = 5;
    else if (multiplier == 10000)
        precision = 4;
    else if (multiplier == 1000)
        precision = 3;
    else if (multiplier == 100)
        precision = 2;
    else if (multiplier == 10)
        precision = 1;

    if (multiplied_float > MAX_NUM)
        multiplied_float = MAX_NUM;
    if (multiplied_float < MIN_NUM)
        multiplied_float = MIN_NUM;

    print((int32_t)multiplied_float);
    decimalSeparator(precision);

    wrBuffer();
}

void HT1621::decimalSeparator(uint8_t dpPosition)
{
    dotsBufferClear();

    if (dpPosition < PRECISION_MIN || dpPosition > PRECISION_MAX_POSITIVE)
        // selected dot position not supported by display hardware
        return;

    SET_BIT(buffer[P5_POS - dpPosition + 1], P1_SEG);
}

void HT1621::DispMinMax(bool enable, bool mode, bool min)
{
    if (enable)
    {
        if (mode)
        {
            LCD_TOGGLE(min, MIN_RU_POS, MIN_RU_SEG, MAX_RU_POS, MAX_RU_SEG);
        }
        else
        {
            LCD_TOGGLE(min, MIN_EN_POS, MIN_EN_SEG, MAX_EN_POS, MAX_EN_SEG);
        }
    }
    else
    {
        CLEAR_BIT(buffer[MIN_RU_POS], MIN_RU_SEG);
        CLEAR_BIT(buffer[MAX_RU_POS], MAX_RU_SEG);
        CLEAR_BIT(buffer[MIN_EN_POS], MIN_EN_SEG);
        CLEAR_BIT(buffer[MAX_EN_POS], MAX_EN_SEG);
    }
}

void HT1621::DispBurst(bool enable, bool mode)
{
    if (enable)
    {
        LCD_SWITCH(mode, BURST_RU_POS, BURST_RU_SEG);
    }
    else
    {
        CLEAR_BIT(buffer[BURST_RU_POS], BURST_RU_SEG);
        CLEAR_BIT(buffer[BURST_EN_POS], BURST_EN_SEG);
    }
}

void HT1621::DispLeak(bool enable, bool mode)
{
    if (enable)
    {
        LCD_SWITCH(mode, LEAK_RU_POS, LEAK_RU_SEG);
    }
    else
    {
        CLEAR_BIT(buffer[LEAK_RU_POS], LEAK_RU_SEG);
        CLEAR_BIT(buffer[LEAK_EN_POS], LEAK_EN_SEG);
    }
}

void HT1621::DispRev(bool enable, bool mode)
{
    if (enable)
    {
        LCD_SWITCH(mode, REV_RU_POS, REV_RU_SEG);
    }
    else
    {
        CLEAR_BIT(buffer[REV_RU_POS], REV_RU_SEG);
        CLEAR_BIT(buffer[REV_EN_POS], REV_EN_SEG);
    }
}

void HT1621::DispFrost(bool enable)
{
    LCD_SWITCH(enable, FROST_POS, FROST_SEG);
}

void HT1621::DispQ(bool enable)
{
    LCD_SWITCH(enable, Q_POS, Q_SEG);
}

void HT1621::DispVer(bool enable, bool mode)
{
    if (enable)
    {
        LCD_SWITCH(mode, VER_RU_POS, VER_RU_SEG);
    }
    else
    {
        CLEAR_BIT(buffer[VER_RU_POS], VER_RU_SEG);
        CLEAR_BIT(buffer[VER_EN_POS], VER_EN_SEG);
    }
}

void HT1621::DispSN(bool enable, bool mode)
{
    if (enable)
    {
        LCD_SWITCH(mode, SN_RU_POS, SN_RU_SEG);
    }
    else
    {
        CLEAR_BIT(buffer[SN_RU_POS], SN_RU_SEG);
        CLEAR_BIT(buffer[SN_EN_POS], SN_EN_SEG);
    }
}

void HT1621::DispWarn(bool enable)
{
    LCD_SWITCH(enable, WARN_POS, WARN_SEG);
}

void HT1621::DispMagn(bool enable)
{
    LCD_SWITCH(enable, MAGNET_POS, MAGNET_SEG);
}

void HT1621::DispLeft(bool enable)
{
    LCD_SWITCH(enable, LEFT_POS, LEFT_SEG);
}

void HT1621::DispRight(bool enable)
{
    LCD_SWITCH(enable, RIGHT_POS, RIGHT_SEG);
}

void HT1621::DispNoWater(bool enable)
{
    LCD_SWITCH(enable, NOWATER_POS, NOWATER_SEG);
}

void HT1621::DispCRC(bool enable)
{
    LCD_SWITCH(enable, CRC_POS, CRC_SEG);
}

void HT1621::DispDelta(bool enable)
{
    LCD_SWITCH(enable, DELTA_POS, DELTA_SEG);
}

void HT1621::DispT(bool enable)
{
    LCD_SWITCH(enable, T_POS, T_SEG);
}

void HT1621::Disp1(bool enable)
{
    LCD_SWITCH(enable, T1_POS, T1_SEG);
}

void HT1621::DispT2(bool enable)
{
    LCD_SWITCH(enable, T2_POS, T2_SEG);
}

void HT1621::DispNBFi(bool enable)
{
    LCD_SWITCH(enable, NBFI_POS, NBFI_SEG);
}

void HT1621::DispNBIoT(bool enable)
{
    LCD_SWITCH(enable, NBIOT_POS, NBIOT_SEG);
}

void HT1621::SignalLevel(uint8_t percents)
{
    CLEAR_BIT(buffer[SIG1_POS], SIG1_SEG);
    CLEAR_BIT(buffer[SIG2_POS], SIG2_SEG);
    CLEAR_BIT(buffer[SIG3_POS], SIG3_SEG);
    if (percents > 60)
    {
        SET_BIT(buffer[SIG3_POS], SIG3_SEG);
    }
    if (percents > 30)
    {
        SET_BIT(buffer[SIG2_POS], SIG2_SEG);
    }
    if (percents > 0)
    {
        SET_BIT(buffer[SIG1_POS], SIG1_SEG);
    }
}

void HT1621::DispDegreePoint(bool enable)
{
    LCD_SWITCH(enable, DEGREE_POS, DEGREE_SEG);
}

void HT1621::DispEnergyJ(bool enable, bool mode, bool perH)
{
    if (enable)
    {
        if (mode)
        {
            SET_BIT(buffer[GCAL_POS], GCAL_SEG);
            LCD_SWITCH(perH, GCAL_H_POS, GCAL_H_SEG);
        }
        else
        {
            SET_BIT(buffer[GJ_POS], GJ_SEG);
            LCD_SWITCH(perH, GJ_H_POS, GJ_H_SEG);
        }
    }
    else
    {
        CLEAR_BIT(buffer[GJ_POS], GJ_SEG);
        CLEAR_BIT(buffer[GJ_H_POS], GJ_H_SEG);
        CLEAR_BIT(buffer[GCAL_POS], GCAL_SEG);
        CLEAR_BIT(buffer[GCAL_H_POS], GCAL_H_SEG);
    }
}

void HT1621::DispEnergyW(bool enable, bool M, bool perH)
{
    if (enable)
    {
        SET_BIT(buffer[W_POS], W_SEG);
        LCD_TOGGLE(M, MW_POS, MW_SEG, KW_POS, KW_SEG);
        LCD_SWITCH(perH, WH_POS, WH_SEG);
    }
    else
    {
        CLEAR_BIT(buffer[W_POS], W_SEG);
        CLEAR_BIT(buffer[KW_POS], KW_SEG);
        CLEAR_BIT(buffer[MW_POS], MW_SEG);
        CLEAR_BIT(buffer[WH_POS], WH_SEG);
    }
}

void HT1621::DispFlowM3(bool enable, bool mode, bool perH)
{
    if (enable)
    {
        SET_BIT(buffer[M3_POS], M3_SEG);
        if (perH)
        {
            SET_BIT(buffer[M3_H_POS], M3_H_SEG);
            LCD_SWITCH(perH, M3_H_EN_POS, M3_H_EN_SEG);
        }
        else
        {
            CLEAR_BIT(buffer[M3_H_POS], M3_H_SEG);
            CLEAR_BIT(buffer[M3_H_EN_POS], M3_H_EN_SEG);
        }
    }
    else
    {
        CLEAR_BIT(buffer[M3_POS], M3_SEG);
        CLEAR_BIT(buffer[M3_H_POS], M3_H_SEG);
        CLEAR_BIT(buffer[M3_H_EN_POS], M3_H_EN_SEG);
    }
}

void HT1621::DispFlowGAL(bool enable, bool perH)
{
    if (enable)
    {
        SET_BIT(buffer[GAL_POS], GAL_SEG);
        LCD_SWITCH(perH, GAL_PM_POS, GAL_PM_SEG);
    }
    else
    {
        CLEAR_BIT(buffer[GAL_POS], GAL_SEG);
        CLEAR_BIT(buffer[GAL_PM_POS], GAL_PM_SEG);
    }
}

void HT1621::DispFlowFT(bool enable, bool perH)
{
    if (enable)
    {
        SET_BIT(buffer[FT3_POS], FT3_SEG);
        LCD_SWITCH(perH, FT3_PM_POS, FT3_PM_SEG);
    }
    else
    {
        CLEAR_BIT(buffer[FT3_POS], FT3_SEG);
        CLEAR_BIT(buffer[FT3_PM_POS], FT3_PM_SEG);
    }
}

void HT1621::DispMMBTU(bool enable)
{
    LCD_SWITCH(enable, MMBTU_POS, MMBTU_SEG);
}

void HT1621::DispGal(bool enable, bool mode)
{
    if (enable)
    {
        SET_BIT(buffer[GALLONS_POS], GALLONS_SEG);
        LCD_SWITCH(mode, US_POS, US_SEG);
    }
    else
    {
        CLEAR_BIT(buffer[GALLONS_POS], GALLONS_SEG);
        CLEAR_BIT(buffer[US_POS], US_SEG);
    }
}