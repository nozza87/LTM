# LTM
### Logitech g driving force shifter (+ optional progressive handbrake) to Thrustmaster
<br/>

## About
This repo contains all the information required to connect a Logitech G Driving Force shifter via it's serial DB9 connector and a USB Progressive handbrake to any Thrustmaster wheel with a 6-pin mini-DIN (PS/2) port via a RP2040.
No modifications are required to any hardware maintaining full normal use.  

The Shifter sends raw analog voltages for the 2 axis and a push button for reverse to the RP2040.  
The handbrake communicates with the RP2040 via standard USB HID.  
The RP2040 communicates to the Thrustmaster via I<sup>2</sup>C.  


## Parts required
* Logitech G Driving Force shifter (For G29, G920, G923, etc..)
* Thrustmaster wheel with a DIN-6 (PS/2) port (T248, T300, etc..)
* [Optional] USB progressive (or digital) handbrake [./info/Ali Handbrake.png](https://github.com/nozza87/LTM/blob/1c2dc2a5008e90b9e4f392d21dffb8929dff3b57/info/Ali%20Handbrake.png?raw=true)
* Male Serial DB9 cable [./info/DB9-pinout-diagram-345x.png](https://github.com/nozza87/LTM/blob/1c2dc2a5008e90b9e4f392d21dffb8929dff3b57/info/DB9-pinout-diagram-345x.png?raw=true)
* Male 6-pin mini-DIN (PS/2) cable [./info/PS-2 Connector.jpeg](https://github.com/nozza87/LTM/blob/1c2dc2a5008e90b9e4f392d21dffb8929dff3b57/info/PS-2%20Connector.jpeg?raw=true)
* Raspberry Pi RP2040 MCU [waveshare.com/wiki/RP2040-Zero](https://www.waveshare.com/wiki/RP2040-Zero)

## Steps
> [!CAUTION]
> Attempt this at your own risk, This will probably void your warranty!
* Solder Male Serial DB9 cable to RP2040 as per the following:

    | Male DB9 Pin  | RP2040 Pin | Description |
    | ------------- | ------------- | ------------- |
    | 1  | -  | Not Connected |
    | 2  | 31 / GPIO26  | X-Axis |
    | 3  | 39 / **5V**  | VCC |
    | 4  | 20 / GPIO15  | Reverse |
    | 5  | -  | Not Connected |
    | 6  | -  | Not Connected |
    | 7  | 32 / GPIO27  | Y-Axis |
    | 8  | 39 / **5V**  | VCC |
    | 9  | GND  | Ground |
    
* Solder Male 6-pin mini-DIN (PS/2) cable to RP2040 as per the following:  

    | Male PS/2 Pin  | RP2040 Pin | Description |
    | ------------- | ------------- | ------------- |
    | 1  | -  | Not Connected |
    | 2  | 7 / GPIO05  | I<sup>2</sup>C SCL |
    | 3  | GND  | Ground |
    | 4  | 6 / GPIO04  | I<sup>2</sup>C SDA |
    | 5  | 37 / **3v3**  | VCC |
    | 6  | GND  | Ground |

* **1.** Install VS Code: [visualstudio.com/VS-Code](https://code.visualstudio.com/)  
* **2.** Install Pico VS Code Extension: [visualstudio.com/raspberry-pi-pico](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico)  
      Tutorial: [raspberrypi.com/pico-vscode-extension](https://www.raspberrypi.com/news/pico-vscode-extension/)  
    (Optional) Install CMake Tools Extension: [visualstudio.com/cmake-tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)  
* **3.** Copy the [.src/](https://github.com/nozza87/LTM/tree/1c2dc2a5008e90b9e4f392d21dffb8929dff3b57/src) directory to your computer and open it in VS Code.  
* **4.** Clean and Configure CMake using either CMake Tools or Pico  
* **5.** Build project and upload  
* **6.** Connect all cables and have fun :)  

## Thanks
The idea to do this and a lot of the information came from:
* Generic USB Shifter to Thrustmaster TH8A Emulator: [github.com/va1da5/th8a-emulator-pico](https://github.com/va1da5/th8a-emulator-pico)
