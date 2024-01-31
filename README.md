# LILYGO-T-Display-Graphics #
A small graphics utility library for the ESP32 with a LILYGO T-Display LCD screen. 
The library is written in C and implemented using the Espressif Iot Development Framework (ESP-IDF).

Feel free to contribute yourself.

# Installation and usage guide #
The following is a guide on how to get the examples up and running. 

If you just want to include the source code in your project, just copy the implementation file and header file into your repo and include it by writing: include "graphics.h"

## Step 1. ##
Install the ESP-IDF Visual Studio Code extension.

![image](https://github.com/simonsvale/LILYGO-T-Display-Graphics/assets/8054877/e4fa7094-d52f-46aa-8f91-92cf93c20dcf)

Follow the installation guide provided by the ESP-IDF extension.

## Step 2. ##
Open the 'examples' folder in Visual Studio Code.


## Step 3. ##
Plug your esp32 into your computer and select the port to use.

This is done by pressing the plug in the bottom left:
![image](https://github.com/simonsvale/LILYGO-T-Display-Graphics/assets/8054877/25364882-a503-48cf-bc76-00f9c38665d3)

This will make a drop down menu appear in the top middle of Visual Studio Code. Select the port the esp32 is connected to:

![image](https://github.com/simonsvale/LILYGO-T-Display-Graphics/assets/8054877/6877c759-fe19-4c61-b311-fef1ab994d4c)

(In this case it was COM as i use windows, but it may show up as tty on linux or usbserial on other machines)

## Step 4. ##
Set the device target, by pressing the circuit board in the bottom left:
![image](https://github.com/simonsvale/LILYGO-T-Display-Graphics/assets/8054877/f1f05e7c-cb66-45e0-aab8-32f5f5d04724)

A dropdown menu will now appear, pick: examples workspace:
![image](https://github.com/simonsvale/LILYGO-T-Display-Graphics/assets/8054877/698a74f3-3ebe-45c7-831b-a99f9b4af587)

Now pick, esp32:

![image](https://github.com/simonsvale/LILYGO-T-Display-Graphics/assets/8054877/1c8a7475-4bd6-49eb-ba1b-bcbc5b31cdd0)


Now pick, Custom board:

![image](https://github.com/simonsvale/LILYGO-T-Display-Graphics/assets/8054877/516f44ed-3e1e-47b7-9cae-82dc5e305a3b)


## Step 5. ##
Build the project by pressing ctrl + e, then press b.

This can also be done by pressing the cylinder in the bottom left:
![image](https://github.com/simonsvale/LILYGO-T-Display-Graphics/assets/8054877/c6b82dff-7455-440d-9b28-dc46c5078c8a)


## Step 6. ##
Flash the code onto the ESP32 by pressing: ctrl + e, then press f.

This can also be done by pressing the lightning in the bottom left:

![image](https://github.com/simonsvale/LILYGO-T-Display-Graphics/assets/8054877/910cd7d4-4c80-4e2c-b670-c47eb68a4250)

Now the examples should be showing on the LILYGO T-Display LCD screen.







 
