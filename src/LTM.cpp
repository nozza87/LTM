#define DEBUG false

//#include <stdlib.h>
#include <stdio.h>
//#include <string.h>
//#include <iostream>
#include <cstdlib>
#include <pico/stdlib.h>
//#include <pico/stdio.h>
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/adc.h"
#include "gears.h"
#include "handbrake.h"
//#include "bsp/board_api.h"
#include "bsp/board.h"
#include "tusb.h"
#include "tusb_config.h"
//#include "usb_descriptors.h"

#include "WS2812.hpp"

#define GPIO_LED 16 // WS2812?

//#define GPIO_UART_TX 12
//#define GPIO_UART_RX 13

#define GPIO_GEAR_M 14 // Digital
#define GEAR_MODE_SWITCH false

#define GPIO_GEAR_X 26 // Analog
#define ADC_GEAR_X 0 // ADC
#define GPIO_GEAR_Y 27 // Analog
#define ADC_GEAR_Y 1 // ADC
#define GPIO_GEAR_R 15 // Digital

#define USBVOLTAGE false
#if USBVOLTAGE // 5.0v
    // X = L:1260 - M:2870 - R:4095
    #define XAXIS_LEFT_THRESH 2100 // Calibrated
    #define XAXIS_RIGHT_THRESH 3900 // Calibrated
    // Y = D:315 - M:2940 - U:4095
    #define YAXIS_DOWN_THRESH 1600 // Calibrated
    #define YAXIS_UP_THRESH 4095 // Calibrated
#else // 3.3 Volt
    // X = L:1260 - M:2870 - R:4095
    #define XAXIS_LEFT_THRESH 1300 // Calibrated
    #define XAXIS_RIGHT_THRESH 2400 // Calibrated
    // Y = D:315 - M:2940 - U:4095
    #define YAXIS_DOWN_THRESH 1000 // Calibrated
    #define YAXIS_UP_THRESH 3100 // Calibrated
#endif

// Set both HANDBRAKE_DIGITAL and HANDBRAKE_ANALOG to false for USB
#define HANDBRAKE_DIGITAL false
#define HANDBRAKE_ANALOG false
#define GPIO_HANDBRAKE 28 // Digital or Analog
#define ADC_HANDBRAKE 2 // ADC

#define I2C_SDA 4
#define I2C_SCL 5
// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

using namespace std;

Gear gear;
Handbrake handbrake;
WS2812 ledStrip(
        GPIO_LED,           // Data line is connected to pin 0. (GP0)
        1,                  // Strip is 1 LEDs long.
        pio0,               // Use PIO 0 for creating the state machine.
        0,                  // Index of the state machine that will be created for controlling the LED strip, You can have 4 state machines per PIO-Block up to 8 overall. See Chapter 3 in: https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf
        WS2812::FORMAT_GRB  // Pixel format used by the LED strip
    );

extern void hid_app_task(void);

void gears_init(void);
void gears_task(void);

#if HANDBRAKE_ANALOG // Analog
    void handbrake_init(void);
    void handbrake_task(void);
#endif

void gpio_callback(uint gpio, uint32_t events);

void led_init(void);
void led_task(void);

void report_task(void);

extern "C" int main()
{
    // Enable UART so we can print status output
    stdio_init_all();
    //uart_init(uart0_hw, 115200);
    //gpio_set_function(GPIO_UART_TX, GPIO_FUNC_UART);
    //gpio_set_function(GPIO_UART_RX, GPIO_FUNC_UART);
    
    board_init();
    printf("\r\n< INIT >\r\n");
    
    printf(" - LED\r\n");
    led_init();
    
    printf(" - ADC\r\n");
    adc_init();
    
    #if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c requires a board with I2C pins
        puts("Default I2C pins were not defined");
    #else
        //i2C
        printf(" - I2C\r\n");
        
        i2c_init(i2c_default, 100 * 1000); // 100 kHz
        
        //gpio_init(I2C_SDA);
        gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
        gpio_pull_up(I2C_SDA);
        
        //gpio_init(I2C_SCL);
        gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
        gpio_pull_up(I2C_SCL);
        
        // Make the I2C pins available to picotool
        bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
        
        /*/
        printf("\nI2C Bus Scan\n");
        printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

        for (int addr = 0; addr < (1 << 7); ++addr) {
            if (addr % 16 == 0) {
                printf("%02x ", addr);
            }

            // Perform a 1-byte dummy read from the probe address. If a slave
            // acknowledges this address, the function returns the number of bytes
            // transferred. If the address byte is ignored, the function returns
            // -1.

            // Skip over any reserved addresses.
            int ret;
            uint8_t rxdata;
            if (reserved_addr(addr))
                ret = PICO_ERROR_GENERIC;
            else
                ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);

            printf(ret < 0 ? "." : "@");
            printf(addr % 16 == 15 ? "\n" : "  ");
        }
        printf("Done.\n");
        */
    #endif
    
    printf(" - USB\r\n");
    tusb_init();
    
    printf(" - GPIO\r\n");
    #if GEAR_MODE_SWITCH
        gpio_set_irq_enabled_with_callback(GPIO_GEAR_M, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
        gpio_callback(GPIO_GEAR_M, 0);
    #endif
    
    adc_gpio_init(GPIO_GEAR_X);
    //gpio_set_dir(GPIO_GEAR_X, false);
    adc_gpio_init(GPIO_GEAR_Y);
    //gpio_set_dir(GPIO_GEAR_Y, false);
    gears_init();
    gpio_set_irq_enabled_with_callback(GPIO_GEAR_R, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_callback(GPIO_GEAR_R, 0);
    
    #if HANDBRAKE_ANALOG // Analog
        adc_gpio_init(GPIO_HANDBRAKE);
        //gpio_set_dir(GPIO_HANDBRAKE, false);
        handbrake_init();
    #endif
    #if HANDBRAKE_DIGITAL // Digital
        gpio_set_irq_enabled_with_callback(GPIO_HANDBRAKE, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
        gpio_callback(GPIO_HANDBRAKE, 0); 
    #endif
    
    printf("< RUN >\r\n");
    
    while (true)
    {
        // tinyusb host task
        tuh_task();
        
        gears_task();
        #if HANDBRAKE_ANALOG // Analog
            handbrake_task();
        #endif
        #if DEBUG
            report_task(); // For Debugging only!
        #endif
        led_task();
        
        #if CFG_TUH_HID
            hid_app_task();
        #endif
    }
    
    return 0;
}

void gears_init(void)
{
    gear.set_reverse(false);
    #if GEAR_MODE_SWITCH
        if (gpio_get(GPIO_GEAR_M))
        {
            gear.set_gear_mode(GearMode::S);
            gear.set_gear_sequential(GearSequential::CENTER);
        }
        else
        {
            gear.set_gear_mode(GearMode::H);
            gear.set_gear_h(GearH::NEUTRAL, false);
        }
    #else // Default to sequential
        gear.set_gear_mode(GearMode::S);
        gear.set_gear_sequential(GearSequential::CENTER);
    #endif
    //printf("Init Gear Mode: %c\r\n", gear.get_mode());
    //printf("Init Gear: %d - %s\r\n", 0, gear.get_gear_h_friendly().c_str() );
}

void gears_task(void)
{
    // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
    const float conversion_factor = 3.3f / (1 << 12);
    
    static uint32_t last_X = 65535;
    static uint32_t last_y = 65535;
    static uint8_t lastGear = 0x00;
    static bool pre_mode_Change = false;
    
    const uint32_t interval_ms = 5;
    static uint32_t start_ms = 0;
    if (to_ms_since_boot(get_absolute_time()) - start_ms < interval_ms)
    {
        return; // not enough time
    }
    //start_ms += interval_ms;
    start_ms = to_ms_since_boot(get_absolute_time());
    
    adc_select_input(ADC_GEAR_X);
    uint16_t gear_x = adc_read();
    //printf("gear_x: %d, voltage: %f V\n", gear_x, gear_x * conversion_factor); // 0 - 4095
    
    adc_select_input(ADC_GEAR_Y);
    uint16_t gear_y = adc_read();
    //printf("gear_y: %d, voltage: %f V\n", gear_y, gear_y * conversion_factor); // 0 - 4095
    
    uint8_t newGear = 0x00;
    if (gear.get_mode() == 'H') // H-Pattern
    {
        newGear = GearH::NEUTRAL;
        if (gear_y >= YAXIS_UP_THRESH)
        {
            if (gear_x <= XAXIS_LEFT_THRESH) // One
            {
                if (!gear.get_reverse())
                {
                    newGear = GearH::ONE;
                }
                else
                {
                    newGear = lastGear;
                }
            }
            else if (gear_x > XAXIS_LEFT_THRESH && gear_x < XAXIS_RIGHT_THRESH) // Three / Up
            {
                if (!gear.get_reverse())
                {
                    newGear = GearH::THREE;
                }
                else
                {
                    if (!pre_mode_Change)
                    {
                        printf("H-Pattern pre Mode change\r\n");
                        pre_mode_Change = true;
                        newGear = lastGear;
                    }
                }
            }
            else if (gear_x >= XAXIS_RIGHT_THRESH) // Five
            {
                if (!gear.get_reverse())
                {
                    newGear = GearH::FIVE;
                }
                else {
                    if (pre_mode_Change)
                    {
                        gear.set_gear_mode(GearMode::S);
                        gear.set_gear_sequential(GearSequential::CENTER);
                        //printf("Gear Mode changed: Sequential\r\n");
                        pre_mode_Change = false;
                        lastGear = GearSequential::CENTER;
                    }
                    newGear = lastGear;
                }
            }
        }
        else if (gear_y <= YAXIS_DOWN_THRESH)
        {
            if (gear_x <= XAXIS_LEFT_THRESH) // Two
            {
                if (!gear.get_reverse())
                {
                    newGear = GearH::TWO;
                }
                else
                {
                    newGear = lastGear;
                }
            }
            else if (gear_x > XAXIS_LEFT_THRESH && gear_x < XAXIS_RIGHT_THRESH) // Four / Down
            {
                if (!gear.get_reverse())
                {
                    newGear = GearH::FOUR;
                }
                else
                {
                    newGear = lastGear;
                }
            }
            else if (gear_x >= XAXIS_RIGHT_THRESH) // Six / Reverse
            {
                if (!gear.get_reverse())
                {
                    newGear = GearH::SIX;
                }
                else
                {
                    pre_mode_Change = false;
                    newGear = GearH::REVERSE;
                }
            }
        }
        
        if (!gear.get_reverse())
        {
            pre_mode_Change = false;
        }
        
        if (lastGear != newGear)
        {
            lastGear = newGear;
            gear.set_gear_h(newGear, false);
            #if DEBUG
                printf("gear_x: %d, voltage: %f V\n", gear_x, gear_x * conversion_factor); // 0 - 4095
                printf("gear_y: %d, voltage: %f V\n", gear_y, gear_y * conversion_factor); // 0 - 4095
                printf("H-Pattern Gear changed: %d - %s\r\n", newGear, gear.get_gear_h_friendly().c_str() );
            #endif
        }
    }
    else { // Sequential
        newGear = GearSequential::CENTER;
        if (gear_y >= YAXIS_UP_THRESH)
        {
            if (gear_x <= XAXIS_LEFT_THRESH) // One
            {
                newGear = lastGear;
            }
            else if (gear_x > XAXIS_LEFT_THRESH && gear_x < XAXIS_RIGHT_THRESH) // Three / Up
            {
                if (!gear.get_reverse())
                {
                    newGear = GearSequential::UP;
                }
                else
                {
                    if (!pre_mode_Change)
                    {
                        printf("Sequential pre Mode change\r\n");
                        pre_mode_Change = true;
                        newGear = lastGear;
                    }
                }
            }
            else if (gear_x >= XAXIS_RIGHT_THRESH) // Five
            {
                if (!gear.get_reverse())
                {
                    newGear = lastGear;
                }
                else {
                    if (pre_mode_Change)
                    {
                        gear.set_gear_mode(GearMode::H);
                        gear.set_gear_h(GearH::NEUTRAL, false);
                        //printf("Gear Mode changed: H-Pattern\r\n");
                        pre_mode_Change = false;
                        lastGear = GearH::NEUTRAL;
                        newGear = lastGear;
                    }
                    else {
                        newGear = lastGear;
                    }
                }
            }
        }
        else if (gear_y <= YAXIS_DOWN_THRESH)
        {
            if (gear_x <= XAXIS_LEFT_THRESH) // Two
            {
                newGear = lastGear;
            }
            else if (gear_x > XAXIS_LEFT_THRESH && gear_x < XAXIS_RIGHT_THRESH) // Four / Down
            {
                if (!gear.get_reverse())
                {
                    newGear = GearSequential::DOWN;
                }
                else
                {
                    newGear = lastGear;
                }
            }
            else if (gear_x >= XAXIS_RIGHT_THRESH) // Six / Reverse
            {
                newGear = lastGear;
            }
        }
        
        if (!gear.get_reverse())
        {
            pre_mode_Change = false;
        }
        
        if (lastGear != newGear)
        {
            lastGear = newGear;
            gear.set_gear_sequential(newGear);
            #if DEBUG
                printf("gear_x: %d, voltage: %f V\n", gear_x, gear_x * conversion_factor); // 0 - 4095
                printf("gear_y: %d, voltage: %f V\n", gear_y, gear_y * conversion_factor); // 0 - 4095
                printf("Sequential Gear changed: %d - %s\r\n", newGear, gear.get_gear_sequential_friendly().c_str() );
            #endif
        }
    }
}

void handbrake_init(void)
{
    handbrake.set_handbrake(HandbrakeState::OFF); // OFF
    //printf("Init Handbrake: %d\r\n", 255);
    //printf("Init Handbrake State: OFF\r\n");
}

#if HANDBRAKE_ANALOG // Analog
    void handbrake_task(void)
    {
        static uint8_t lastValue = 255;
        
        const uint32_t interval_ms = 1000;
        static uint32_t start_ms = 0;
        if (to_ms_since_boot(get_absolute_time()) - start_ms < interval_ms)
        {
            return; // not enough time
        }
        //start_ms += interval_ms;
        start_ms = to_ms_since_boot(get_absolute_time());
        
        //uint16_t handbrake_value = gpio_get(GPIO_HANDBRAKE); // 0 - 4095
        adc_select_input(ADC_HANDBRAKE);
        uint16_t handbrake_value = adc_read();
        //printf("handbrake: %d, voltage: %f V\n", handbrake_value, handbrake_value * conversion_factor);
        
        uint8_t reranged_value = (handbrake_value * 255) / 4095; // convert form  0-4095 to 0 - 255
        
        if (lastValue != reranged_value)
        {
            // TODO Read handbrake position and set handbrake accordingly
            lastValue = reranged_value;
            handbrake.set_handbrake(reranged_value);
            
            /*
            printf("Handbrake changed: %d\r\n", reranged_value);
            if (handbrake.get_state())
            {
                printf("Handbrake State: ON\r\n");
            }
            else
            {
                printf("Handbrake State: OFF\r\n");
            }
            */
        }
    }
#endif

void gpio_callback(uint gpio, uint32_t events)
{
    static uint8_t lastGearModeState = false;
    static uint8_t lastReverseState = false;
    static uint8_t lastHandbrakeState = false;
    
    bool gpioState = gpio_get(gpio);
    if (gpio == GPIO_GEAR_R) // Gear Reverse
    {
        if (gpioState != lastReverseState)
        {
            lastReverseState = gpioState;
            if (gpioState) // ON
            {
                gear.set_reverse(true);
                //printf("Reverse changed: ON\r\n");
            }
            else // OFF
            {
                gear.set_reverse(false);
                //printf("Reverse changed: OFF\r\n");
            }
        }
    }
    #if GEAR_MODE_SWITCH
        else if (gpio == GPIO_GEAR_M) // Gear Mode
        {
            if (gpioState != lastGearModeState)
            {
                lastGearModeState = gpioState;
                if (gpioState) // ON
                {
                    gear.set_gear_mode(GearMode::S);
                    gear.set_gear_sequential(GearSequential::CENTER);
                    //printf("Gear Mode changed: Sequential\r\n");
                }
                else // OFF
                {
                    gear.set_gear_mode(GearMode::H);
                    gear.set_gear_h(GearH::NEUTRAL, false);
                    //printf("Gear Mode changed: H-Pattern\r\n");
                }
            }
        }
    #endif
    #if HANDBRAKE_DIGITAL
        else if (gpio == GPIO_HANDBRAKE) // Handbrake
        {
            if (gpioState != lastHandbrakeState)
            {
                lastHandbrakeState = gpioState;
                if (gpioState) // ON
                {
                    handbrake.set_handbrake(HandbrakeState::ON); // ON
                    //printf("Handbrake State changed: ON\r\n");
                }
                else // OFF
                {
                    handbrake.set_handbrake(HandbrakeState::OFF); // OFF
                    //printf("Handbrake State changed: OFF\r\n");
                }
            }
        }
    #endif
}

void led_init(void)
{
    ledStrip.fill(WS2812::RGB(255, 0, 0) );
    ledStrip.show();
}

void led_task(void)
{
    static bool led_state = false;
    
    const uint32_t interval_ms = 1000;
    static uint32_t start_ms = 0;
    if (to_ms_since_boot(get_absolute_time()) - start_ms < (interval_ms - (led_state ? 500 : 0) ) )
    {
        return; // not enough time
    }
    //start_ms += interval_ms;
    start_ms = to_ms_since_boot(get_absolute_time());
    
    // Blink every interval ms
    if (led_state == 1) // ON
    {
        if (gear.get_mode() == 'H')
        {
            ledStrip.fill(WS2812::RGB(0, 255, 0) );
        }
        else if (gear.get_mode() == 'S')
        {
            ledStrip.fill(WS2812::RGB(0, 0, 255) );
        }
        else
        {
            ledStrip.fill(WS2812::RGB(255, 0, 0) );
        }
    }
    else // OFF
    {
        ledStrip.fill(WS2812::RGB(0, 0, 0) );
    }
    ledStrip.show();
    led_state = 1 - led_state; // toggle
}

void report_task(void)
{
    const uint32_t interval_ms = 10000;
    static uint32_t start_ms = 0;
    if (to_ms_since_boot(get_absolute_time()) - start_ms < interval_ms)
    {
        return; // not enough time
    }
    //start_ms += interval_ms;
    start_ms = to_ms_since_boot(get_absolute_time());
    
    printf("** Start Report **\r\n");
    printf("start_ms = %d\r\n", start_ms);
    
    printf("Gear Mode: %c\r\n", gear.get_mode());
    switch (gear.get_mode())
    {
    case 'H':
        printf("Gear: %d - %s\r\n", gear.get_gear_h(), gear.get_gear_h_friendly().c_str() );
        break;
    
    case 'S':
        printf("Position: %d - %s\r\n", gear.get_gear_sequential(), gear.get_gear_sequential_friendly() );
        break;
    }
    switch (gear.get_reverse())
    {
    case true:
        printf("Reverse: ON\r\n");
        break;
    
    case false:
        printf("Reverse: OFF\r\n");
        break;
    }
    
    printf("Handbrake: %d\r\n", handbrake.get_value());
    if (handbrake.get_state() )
    {
        printf("Handbrake State: ON\r\n");
    }
    else
    {
        printf("Handbrake State: OFF\r\n");
    }
    printf("** End Report **\r\n");
}