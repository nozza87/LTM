#define DEBUG false

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021, Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "bsp/board.h"
#include "tusb.h"
#include "handbrake.h"

extern Handbrake handbrake;

/// HID USB H-Shifter Protocol Report.
typedef struct TU_ATTR_PACKED
{
    uint8_t x; ///< Delta x  movement of left analog-stick
} hid_shifter_report_t;


//HID report:   ID      ?       ?       Value (0x00 - 0xFF)
//              0x01    0x00    0x00    0x2E
//              0       1       2       3
typedef struct TU_ATTR_PACKED
{
    uint8_t a; // unknown?
    uint8_t b; // unknown?
    uint8_t c; // unknown?
    uint8_t value; // Position of Handbrake
} hid_handbrake_report_t;

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+

// If your host terminal support ansi escape code such as TeraTerm
// it can be use to simulate mouse cursor movement within terminal
#define USE_ANSI_ESCAPE 0

#define MAX_REPORT 4

static uint8_t const keycode2ascii[128][2] = {HID_KEYCODE_TO_ASCII};

// Each HID instance can has multiple reports
static struct
{
    uint8_t report_count;
    tuh_hid_report_info_t report_info[MAX_REPORT];
} hid_info[CFG_TUH_HID];

static void process_kbd_report(hid_keyboard_report_t const *report);
static void process_generic_report(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len);
static void process_handbrake_report(hid_handbrake_report_t const *report);

void hid_app_task(void)
{
    //printf("## -- hid_app_task -- ##");
    // nothing to do
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len)
{
    printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
    
    // Interface protocol (hid_interface_protocol_enum_t)
    const char *protocol_str[] = {"None", "Keyboard", "Mouse"};
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
    
    #if DEBUG
        printf("HID Interface Protocol = %s\r\n", protocol_str[itf_protocol]);
    #endif
    
    // By default host stack will use activate boot protocol on supported interface.
    // Therefore for this simple example, we only need to parse generic report descriptor (with built-in parser)
    if (itf_protocol == HID_ITF_PROTOCOL_NONE)
    {
        hid_info[instance].report_count = tuh_hid_parse_report_descriptor(hid_info[instance].report_info, MAX_REPORT, desc_report, desc_len);
        #if DEBUG
            printf("HID has %u reports \r\n", hid_info[instance].report_count);
        #endif
        
        /*
        printf("HID desc_report: ");
        for (uint16_t i = 0; i < desc_len; i++) {
            printf("0x%02X ", desc_report[i]);
        }
        */
        /*
        // 55 bytes
        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x05,        // Usage (Game Pad)
        0xA1, 0x01,        // Collection (Application)
        0x85, 0x01,        //   Report ID (1)
        0x05, 0x09,        //   Usage Page (Button)
        0x19, 0x01,        //   Usage Minimum (0x01)
        0x29, 0x10,        //   Usage Maximum (0x10)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x75, 0x01,        //   Report Size (1)
        0x95, 0x10,        //   Report Count (16)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
        0x09, 0x01,        //   Usage (Pointer)
        0xA1, 0x00,        //   Collection (Physical)
        0x09, 0x36,        //     Usage (Slider)
        0x15, 0x00,        //     Logical Minimum (0)
        0x26, 0xFF, 0x00,  //     Logical Maximum (255)
        0x75, 0x08,        //     Report Size (8)
        0x95, 0x01,        //     Report Count (1)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              //   End Collection
        0x85, 0x02,        //   Report ID (2)
        0x09, 0x01,        //   Usage (Pointer)
        0x75, 0x10,        //   Report Size (16)
        0x95, 0x05,        //   Report Count (5)
        0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0xC0,              // End Collection
        */
        //printf("\r\n");
    }

    // request to receive report
    // tuh_hid_report_received_cb() will be invoked when report is available
    if (!tuh_hid_receive_report(dev_addr, instance))
    {
        printf("Error: cannot request to receive report\r\n");
    }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
    printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
    /*
    printf("HID report: ");
    for (uint16_t i = 0; i < len; i++) {
        printf("0x%02X ", report[i]);
    }
    */
    /*
    // 4th byte is Handbrake value from 0x00 to 0xFF
    HID report: 0x01 0x00 0x00 0x00
    ...
    HID report: 0x01 0x00 0x00 0x2E
    ...
    HID report: 0x01 0x00 0x00 0xFF
    */
    //printf("\r\n");
    
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
    
    #if DEBUG
        //printf("Device Address 0x%x, Interface Protocol %d\r\n", dev_addr, itf_protocol);
    #endif

    switch (itf_protocol)
    {
    case HID_ITF_PROTOCOL_KEYBOARD:
        #if DEBUG
            TU_LOG2("HID receive boot keyboard report\r\n");
        #endif
        //process_kbd_report((hid_keyboard_report_t const *)report); // ** DO NOTHING **
        break;
    
    default:
        // Generic report requires matching ReportID and contents with previous parsed report info
        #if DEBUG
            TU_LOG2("HID receive generic report\r\n");
        #endif
        
        if (len == 4)
        {
            if (report[0] == 0x01 && report[1] == 0x00 && report[2] == 0x00)
            {
                #if DEBUG
                    TU_LOG2("HID receive ali express analog handbrake report\r\n");
                #endif
                process_handbrake_report((hid_handbrake_report_t const *)report);
            }
            else
            {
                printf("Report Format Error (%d)", report);
            }
        }
        else
        {
            printf("Report Length Error (%d)", report);
        }
        
        //process_generic_report(dev_addr, instance, report, len); // ** DO NOTHING **
        break;
    }

    // continue to request to receive report
    if (!tuh_hid_receive_report(dev_addr, instance))
    {
        printf("Error: cannot request to receive report\r\n");
    }
}


//--------------------------------------------------------------------+
// Aliexpress USB analog Handbrake
//--------------------------------------------------------------------+
static void process_handbrake_report(hid_handbrake_report_t const *report)
{
    static hid_handbrake_report_t prev_report = {0}; // previous report to check key released
    
    //HID report:   ID      ?       ?       Value (0x00 - 0xFF)
    //              0x01    0x00    0x00    0x2E
    //              0       1       2       3
    if (prev_report.value != report->value)
    {
        uint8_t actualValue = (0xff - report->value);
        #if DEBUG
            printf("set_handbrake(0x%02X): 0x%02X\r\n", report->value, actualValue);
        #endif
        handbrake.set_handbrake(actualValue); // NEED TO INVERT AS 0xFF = OFF and 0x00 = ON
        prev_report = *report;
    }
}

//--------------------------------------------------------------------+
// Keyboard
//--------------------------------------------------------------------+

// look up new key in previous keys
static inline bool find_key_in_report(hid_keyboard_report_t const *report, uint8_t keycode)
{
    for (uint8_t i = 0; i < 6; i++)
    {
        if (report->keycode[i] == keycode)
            return true;
    }

    return false;
}

static void process_kbd_report(hid_keyboard_report_t const *report)
{
    static hid_keyboard_report_t prev_report = {0, 0, {0}}; // previous report to check key released

    //------------- example code ignore control (non-printable) key affects -------------//
    for (uint8_t i = 0; i < 6; i++)
    {
        if (report->keycode[i])
        {
            if (find_key_in_report(&prev_report, report->keycode[i]))
            {
                // exist in previous report means the current key is holding
            }
            else
            {
                // not existed in previous report means the current key is pressed
                bool const is_shift = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
                uint8_t ch = keycode2ascii[report->keycode[i]][is_shift ? 1 : 0];
                putchar(ch);
                if (ch == '\r')
                    putchar('\n'); // added new line for enter key

                fflush(stdout); // flush right away, else nanolib will wait for newline
            }
        }
        // TODO example skips key released
    }

    prev_report = *report;
}

//--------------------------------------------------------------------+
// Joystick
//--------------------------------------------------------------------+
static void process_joystick_report(hid_shifter_report_t const *report)
{
    static hid_shifter_report_t prev_report = {0}; // previous report to check key released

    // printf("[lx:%x, ly:%x]\r\n", report->x, report->y);
    if (prev_report.x == report->x)
        return;
    #if DEBUG
        printf("USB Report: %d\r\n", report->x);
    #endif
    
    prev_report = *report;
}

//--------------------------------------------------------------------+
// Generic Report
//--------------------------------------------------------------------+
static void process_generic_report(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
    (void)dev_addr;

    uint8_t const rpt_count = hid_info[instance].report_count;
    tuh_hid_report_info_t *rpt_info_arr = hid_info[instance].report_info;
    tuh_hid_report_info_t *rpt_info = NULL;

    if (rpt_count == 1 && rpt_info_arr[0].report_id == 0)
    {
        // Simple report without report ID as 1st byte
        rpt_info = &rpt_info_arr[0];
    }
    else
    {
        // Composite report, 1st byte is report ID, data starts from 2nd byte
        uint8_t const rpt_id = report[0];

        // Find report id in the arrray
        for (uint8_t i = 0; i < rpt_count; i++)
        {
            if (rpt_id == rpt_info_arr[i].report_id)
            {
                rpt_info = &rpt_info_arr[i];
                break;
            }
        }

        report++;
        len--;
    }

    if (!rpt_info)
    {
        //printf("Couldn't find the report info for this report !\r\n");
        return;
    }

    // For complete list of Usage Page & Usage checkout src/class/hid/hid.h. For examples:
    // - Keyboard                     : Desktop, Keyboard
    // - Mouse                        : Desktop, Mouse
    // - Gamepad                      : Desktop, Gamepad
    // - Consumer Control (Media Key) : Consumer, Consumer Control
    // - System Control (Power key)   : Desktop, System Control
    // - Generic (vendor)             : 0xFFxx, xx

    if (rpt_info->usage_page == HID_USAGE_PAGE_DESKTOP)
    {
        switch (rpt_info->usage)
        {
        case HID_USAGE_DESKTOP_KEYBOARD:
            #if DEBUG
                printf("HID receive keyboard report\r\n");
            #endif
            // Assume keyboard follow boot report layout
            process_kbd_report((hid_keyboard_report_t const *)report);
            break;

        case HID_USAGE_DESKTOP_JOYSTICK:
            #if DEBUG
                printf("HID receive shifter report\r\n");
            #endif
            process_joystick_report((hid_shifter_report_t const *)report);
            break;

        default:
            break;
        }
    }
}