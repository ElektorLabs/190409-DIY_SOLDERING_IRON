#  190409 - Soldering station
Software for the soldering station ( 190409 ) based on the arduino framwork.

Shows the Temperature in °C no current limit set.

## Getting Started

Download the source and open it with the arduino ide ( >= 1.8.x ) and select Arduino Leonardo as target.
The station shall be detected via usb as serial port. Set the ide to use the port and compile the sketch.
If the leonardo bootloader is working you can just press upload and the software will be transfered to the station.
Be aware that after flashing the new software, your current temperature settings may be gone and set back to 50°C


### Prerequisites

You need the following libraries installed in your libs path:
 - OneWire 2.3.5 by Paul Stoffregen
 - Grove 4-Digit Display 1.0 by Seeed Studio

For the Atmega4809 install the MegaCoreX Board package
 - MegaCoreX Version 1.0.4 ( https://github.com/MCUdude/MegaCoreX ) 
  
To compile the code make sure you have the follwing settings set in you ArduinIDE:

- Board: Atmega4809
- Clock: Internal 20 MHz
- BOD: BOD 2.6V
- Pinout: 48 Pin standard
- Reset Pin: Reset

To uploade code to the Atmega4809 use a ICE2UDPI programmer or a MPLAB PIC KIT4 / MPLAB SNAP 
To build your own ICE2UPDI Adapter have a look at : https://www.elektormagazine.com/labs/arduino-for-updi-programming-for-atmega4809-and-tiny816817-with-jtag2updi
This programmer now works with the ArduinoIDE out of the box. 

### Serialinterface 
!! Be warned: If you connect the serail interface to your computer you will loose the galvanic isolation for the soldering tip !!

The UART0 ( K10 )  on the station works as serial device. If you plug it at TTL to USB Adaptor you can than access
the serial port with the terminal form the arduino ide or putty / puttytel or any other serial terminal. Baudrate is 115200 (8N1)

##### Supported commands of the console

* set/get setpoint [xxx] -> This will set or get the setpoint
* set/get irontype [xxx] -> This will change the configure iron type
* get temperature        -> This will report the current temperature
* clear error            -> If an error is shown this will clear it
* help                   -> This will show the help
 
