#pragma once

#include <iostream>
#include "hardware/i2c.h"

enum HandbrakeState : uint8_t
{
    ON = 0x00,
    OFF = 0xff
};

class Handbrake
{
private:
    uint8_t m_size = 14;
    uint8_t m_addr = 0x01;

    uint8_t m_data[14] = {
        0x02, // TSS handbrake
        0x0C, // Unknown
        0x02, // Unknown
        0x00, // Unknown, 01 when max break
        0xFF, // FF - no break, 00 max break
        0x80, // Unknown ◄─┐
        0x81, // Unknown ◄─│─┐
        0x80, // Unknown ◄─┘ │
        0x81, // Unknown ◄───┘
        0x00, // Unknown
        0x00, // Unknown
        0x00, // Unknown
        0x00, // Unknown
        0x00  // Unknown
    };

    uint8_t m_min[4] = {
        0x80, // Unknown ◄─┐
        0x81, // Unknown ◄─│─┐
        0x80, // Unknown ◄─┘ │
        0x81, // Unknown ◄───┘
    };

    uint8_t m_max[4] = {
        0x7F, // Unknown ◄─┐
        0x1D, // Unknown ◄─│─┐
        0x7F, // Unknown ◄─┘ │
        0x1D, // Unknown ◄───┘
    };

public:
    void set_handbrake(uint8_t value);
    uint8_t get_value();
    bool get_state();
    uint8_t *get_data();
    uint8_t get_size();
    void send(i2c_inst_t *i2c);
    void send(i2c_inst_t *i2c, uint8_t addr);
};

/**
 * TSS Sparco handbrake I2C data
 *
 * Handbrake min -> max
 *
 * 0  02 02 02 02 02 // TSS handbrake
 * 1  0C 0C 0C 0C 0C // Unknown always 0x0C?
 * 2  02 02 02 02 02 // Type 0x01 - Shifter, 0x02 - Handbrake
 * 3  00 00 00 01 01 // Digital Brake State 0x00 - OFF, 0x01 - ON
 * 4  FF 8A 82 17 0F // Analog Brake Value 0xFF - no break, 0x00 max break
 * 5  80 80 80 7F 7F // Digital Brake State checksum (0x80 - value)
 * 6  81 4D 4A 20 1D // Analog Brake Value checksum (?)
 * 7  80 80 80 7F 7F // Digital Brake State checksum (0x80 - value)
 * 8  81 4D 4A 20 1D // Analog Brake Value checksum (?)
 * 9  00 00 00 00 00 // Unknown always 0x00?
 * A  00 00 00 00 00 // Unknown always 0x00?
 * B  00 00 00 00 00 // Unknown always 0x00?
 * C  00 00 00 00 00 // Unknown always 0x00?
 * D  00 00 00 00 00 // Unknown always 0x00?
 *
 * https://www.isrtv.com/forums/topic/24532-gearbox-connector-on-base/?do=findComment&comment=232234
 */