#define DEBUG false

#include "gears.h"
//#include <string>

void Gear::set_gear_mode(GearMode mode)
{
    if (mode != this->m_data[0])
    {
        this->m_data[0] = mode;
        #if DEBUG
            printf("Gear Mode changed: %d [0x%02X] - %s\r\n", mode, mode, mode == GearMode::H ? "H-Pattern" : "Sequential");
            /*
            if (mode == GearMode::H)
            {
                printf("Gear Mode changed: H-Pattern\r\n");
            }
            else
            {
                printf("Gear Mode changed: Sequential\r\n");
            }
            */
        #endif
        send(i2c_default);
    }
}

void Gear::set_reverse(bool state)
{
    if (state != this->m_reverse)
    {
        this->m_reverse = state;
        #if DEBUG
            printf("Reverse button state changed: %d [0x%02X] - %s\r\n", state, state, state ? "ON" : "OFF");
            /*
            if (state)
            {
                printf("Reverse button state: ON\r\n");
            }
            else
            {
                printf("Reverse button state: OFF\r\n");
            }
            */
        #endif
    }
}


void Gear::set_gear_sequential(GearSequential gear)
{
    //printf("set_gear_sequential(GearSequential gear)");
    set_gear_sequential((uint8_t) gear);
}
void Gear::set_gear_sequential(uint8_t gear)
{
    //printf("set_gear_sequential(uint8_t gear)");
    if (gear != this->m_data[4])
    {
        this->m_data[4] = gear;
        #if DEBUG
            printf("Sequential Gear changed: %d [0x%02X] - %s\r\n", gear, gear, get_gear_sequential_friendly().c_str() );
        #endif
        send(i2c_default);
    }
}


void Gear::set_gear_h(GearH gear, bool direct = false)
{
    //printf("set_gear_h(GearH gear)");
    set_gear_h((uint8_t) gear, (bool) direct);
}
void Gear::set_gear_h(uint8_t gear, bool direct = false)
{
    //printf("set_gear_h(uint8_t gear)");
    if (direct) // Sets gear as hex value
    {
        if (gear != this->m_data[3])
        {
            this->m_data[3] = gear;
        }
    }
    else // Sets gear in human terms (0-8)
    {
        if ((0x80 >> (8 - gear)) != this->m_data[3])
        {
            this->m_data[3] = (0x80 >> (8 - gear));
        }
    }
    #if DEBUG
        printf("H-Pattern Gear changed: %d [0x%02X] - %s\r\n", gear, this->m_data[3], get_gear_h_friendly().c_str() );
    #endif
    send(i2c_default);
}

unsigned char Gear::get_mode()
{
    if (this->m_data[0] == GearMode::H)
    {
        return 'H';
    }
    return 'S';
}

bool Gear::get_reverse()
{
    return this->m_reverse;
}

uint8_t Gear::get_gear_h()
{
    return this->m_data[3];
    /*
    uint8_t value = this->m_data[3];

    for (uint8_t i = 0; i < 9; i++)
    {
        if (value == (0x80 >> (8 - i)))
        {
            return i;
        }
    }
    return 0;
    */
}

std::string Gear::get_gear_h_friendly()
{
    switch (get_gear_h())
    {
        case GearHLookup::lNEUTRAL:
            return "Neutral";
        case GearHLookup::lONE:
            return "One";
        case GearHLookup::lTWO:
            return "Two";
        case GearHLookup::lTHREE:
            return "Three";
        case GearHLookup::lFOUR:
            return "Four";
        case GearHLookup::lFIVE:
            return "Five";
        case GearHLookup::lSIX:
            return "Six";
        case GearHLookup::lSEVEN:
            return "Seven";
        case GearHLookup::lREVERSE:
            return "Reverse";
        default:
            return "Unknown";
    }
}

uint8_t Gear::get_gear_sequential()
{
    return this->m_data[4];
}
std::string Gear::get_gear_sequential_friendly()
{
    switch (get_gear_sequential())
    {
        case GearSequential::CENTER:
            return "Center";
        case GearSequential::UP:
            return "Up";
        case GearSequential::DOWN:
            return "Down";
        default:
            return "Unknown";
    }
}

uint8_t *Gear::get_data()
{
    return static_cast<uint8_t *>(this->m_data);
}

uint8_t Gear::get_size()
{
    return this->m_size;
}

void Gear::send(i2c_inst_t *i2c)
{
    this->send(i2c, this->m_addr);
}

void Gear::send(i2c_inst_t *i2c, uint8_t addr)
{
    auto data = this->get_data();
    
    // for (int i = 0; i < 14; i++)
    // {
    //     printf("%02X ", m_data[i]);
    // }
    
    if (i2c_get_write_available(i2c_default) != 0)
    {
        i2c_write_timeout_us(i2c, addr, data, this->m_size, false, 100000); // 100ms
        //i2c_write_blocking(i2c, addr, data, this->m_size, false);
    }
    else {
        printf("Error: I2C not for write\r\n");
    }
}