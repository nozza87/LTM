#pragma once

#include <iostream>
#include "hardware/i2c.h"
//#include <string>

enum GearSequential : uint8_t
{
    CENTER = 0x04,
    DOWN = 0x05,
    UP = 0x06
};

enum GearMode : uint8_t
{
    S = 0x00,
    H = 0x80
};

enum GearH : uint8_t
{
    NEUTRAL = 0x00,
    ONE = 0x01,
    TWO = 0x02,
    THREE = 0x03,
    FOUR = 0x04,
    FIVE = 0x05,
    SIX = 0x06,
    SEVEN = 0x07,
    REVERSE = 0x08,
};

enum GearHLookup : uint8_t
{
    lNEUTRAL = 0x00,
    lONE = 0x01,
    lTWO = 0x02,
    lTHREE = 0x04,
    lFOUR = 0x08,
    lFIVE = 0x10,
    lSIX = 0x20,
    lSEVEN = 0x40,
    lREVERSE = 0x80,
};

class Gear
{

private:
    bool m_reverse = false; // reverse modifier button state
    uint8_t m_size = 14;
    uint8_t m_addr = 0x01;
    uint8_t m_data[14] = {
        0x00, // Shifter mode 0 - S / 0x80 - H
        0x0C, // Unknown
        0x01, // Unknown
        0x00, // Gear in H-mode 0 - 8
        0x04, // Gear in S-Mode 0x04 - center, 0x05 - down, 0x06 - up
        0x80, // Unknown
        0x80, // Unknown
        0x00, // Y cordinate
        0x00, // X cordinate
        0x00, // Unknown
        0x00, // Unknown
        0x00, // Unknown
        0x00, // Unknown
        0x00  // Unknown
    };

public:
    void set_gear_mode(GearMode mode);
    void set_reverse(bool state);
    void set_gear_sequential(uint8_t gear);
    void set_gear_sequential(GearSequential gear);
    void set_gear_h(uint8_t gear, bool direct);
    void set_gear_h(GearH gear, bool direct);
    unsigned char get_mode();
    bool get_reverse();
    uint8_t get_gear_h();
    std::string get_gear_h_friendly();
    uint8_t get_gear_sequential();
    std::string get_gear_sequential_friendly();
    uint8_t *get_data();
    uint8_t get_size();
    void send(i2c_inst_t *i2c);
    void send(i2c_inst_t *i2c, uint8_t addr);
};

/**
 * TH8A Gear Shifter I2C data (H-Pattern)
 *
 * Gear N -> 1 -> N -> 1
 *
 * 0  80 80 80 80 // Shifter mode 0 - S / 0x80 - H
 * 1  0C 0C 0C 0C // Unknown always 0x0C?
 * 2  01 01 01 01 // Type 0x01 - Shifter, 0x02 - Handbrake
 * 3  01 00 01 00 // Gear in H-mode 0/N - 0x00, 1 - 0x01, 2 - 0x02, 3 - 0x04, 4 - 0x08, 5 - 0x10, 6 - 0x20, 7 - 0x40, 8/R - 0x80
 * 4  00 00 00 00 // Gear in S-Mode N/A - 0x00
 * 5  80 80 80 80 // Unknown always 0x80?
 * 6  80 80 80 80 // Unknown always 0x80?
 * 7  B4 B3 B5 B3 // Y cordinate
 * 8  F0 EC EF EF // X cordinate
 * 9  00 00 00 00 // Unknown always 0x00?
 * A  00 00 00 00 // Unknown always 0x00?
 * B  00 00 00 00 // Unknown always 0x00?
 * C  00 00 00 00 // Unknown always 0x00?
 * D  00 00 00 00 // Unknown always 0x00?
 * 
 * TH8A N-1-N-1.vcd
 */

/**
 * TH8A Gear Shifter I2C data (Sequential)
 *
 * Gear D -> C -> U -> C
 *
 * 0  00 00 00 00 // Shifter mode 0 - S / 0x80 - H
 * 1  0C 0C 0C 0C // Unknown always 0x0C?
 * 2  01 01 01 01 // Type 0x01 - Shifter, 0x02 - Handbrake
 * 3  00 00 00 00 // Gear in H-mode N/A - 0x00
 * 4  05 04 06 04 // Gear in S-Mode 0x04 - center, 0x05 - down, 0x06 - up
 * 5  80 80 80 80 // Unknown always 0x80?
 * 6  80 80 80 80 // Unknown always 0x80?
 * 7  7E 7D 7E 7D // Y cordinate
 * 8  D9 D8 A6 D8 // X cordinate
 * 9  00 00 00 00 // Unknown always 0x00?
 * A  00 00 00 00 // Unknown always 0x00?
 * B  00 00 00 00 // Unknown always 0x00?
 * C  00 00 00 00 // Unknown always 0x00?
 * D  00 00 00 00 // Unknown always 0x00?
 * 
 * TH8A seq.vcd
 */