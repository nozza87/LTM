#define DEBUG false
#define VERBOSE false

#include "handbrake.h"

//#include "gears.h"
//extern Gear gear;

#define STATETHRESHOLD 50 // [00 - 255 ; 0x00 - 0xFF]
void path_data(uint8_t (&data)[14], uint8_t (&update)[4])
{
    // TODO: Actually calculate these properly!!!
    
    data[5] = (0x80 - data[3]); //data[5] = update[0]; // 80 80 80 7F 7F
    data[6] = update[1]; // 81 4D 4A 20 1D
    data[7] = (0x80 - data[3]); //data[7] = update[2]; // 80 80 80 7F 7F
    data[8] = update[3]; // 81 4D 4A 20 1D
    
    for (uint8_t i = 0; i < 4; i++)
    {
        data[i + 5] = update[i];
    }
}

void Handbrake::set_handbrake(uint8_t value)
{
    if (value != this->m_data[4])
    {
        this->m_data[4] = value;
        
        if (value < STATETHRESHOLD) // Handbrake ON
        {
            if (this->m_data[3] != 0x01)
            {
                this->m_data[3] = 0x01;
                path_data(this->m_data, this->m_max);
                #if DEBUG
                    printf("Handbrake State changed, value: %d [0x%02X], state: %d [0x%02X] - %s\r\n", value, value, this->m_data[3], this->m_data[3], this->m_data[3] == 0x01 ? "true" : "false");
                #endif
            }
        }
        else // Handbrake OFF
        {
            if (this->m_data[3] != 0x00)
            {
                this->m_data[3] = 0x00;
                path_data(this->m_data, this->m_min);
                #if DEBUG
                    printf("Handbrake State changed, value: %d [0x%02X], state: %d [0x%02X] - %s\r\n", value, value, this->m_data[3], this->m_data[3], this->m_data[3] == 0x01 ? "true" : "false");
                #endif
            }
        }
        #if VERBOSE
            printf("Handbrake changed, value: %d [0x%02X], state: %d [0x%02X] - %s\r\n", value, value, this->m_data[3], this->m_data[3], this->m_data[3] == 0x01 ? "true" : "false");
        #endif
        send(i2c_default);
    }
}

uint8_t Handbrake::get_value()
{
    return this->m_data[4];
    /*
    uint8_t value = this->m_data[4];
    
    return value;
    */
}

bool Handbrake::get_state()
{
    switch (this->m_data[3])
    {
    case 0x01:
        return true;
    case 0x00:
        return false;
    default:
        return false;
    }
}

uint8_t *Handbrake::get_data()
{
    return static_cast<uint8_t *>(this->m_data);
}

uint8_t Handbrake::get_size()
{
    return this->m_size;
}

void Handbrake::send(i2c_inst_t *i2c)
{
    this->send(i2c, this->m_addr);
}

void Handbrake::send(i2c_inst_t *i2c, uint8_t addr)
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
    //gear.send(i2c_default); // Reset gear after handbrake?, grinds as no clutch!
}
