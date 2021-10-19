--------------------------------------------------------------------------------
4116 DRAM Tester v3.0
Firmware fix by Uffe Lund-Hansen, 2017-2020
Mods by Alex Wierzbowsky [RBSC], 2021
--------------------------------------------------------------------------------

WARNING!
If you applied the power mod to the tester, do not use the 9v power socket!
Power the tester only with the USB cable connected to Ardino Nano's Mini/MicroUSB connector.

IMPORTANT!
It's recommended to use a computer's USB3 port to power the tester because USB3 provides enough current
for the device to work properly. You can also power the tester with any 5v 500mA rated power supply that
is connected to the Mini/MicroUSB connector (for example phone charger).


ABOUT
-----

The original project was created in November 2017. The code was officially released in March 2020. Most of the
code and design was coming from http://forum.defence-force.org/viewtopic.php?p=15035&sid=17bf402b9c2fd97c8779668b8dde2044
by forum's member "iss" and was modified to work with 4116 DRAM by Uffe Lund-Hansen, Frostbox Labs.

The board for the project was modded from the original design by Muttley Black. However, the board was not safe
to use because there was always power on the chip's socket. Inserting a DRAM chip with the enabled +12v, -5v
and +5v is rather dangerous and in case of a wrongly inserted chip, damage may occur to the chip and the tester
itself.

Also, the firmware was designed to run only once, then the Arduino had to be reset by the Start button or by
the on-board reset button. The original firmware also had some minimal serial interface reporting capabilities.

Taking into account the above mentioned drawbacks, it was decided to mod the tester's hardware and firmware
in order to make it safer and more usable. Three mods were created for this project:

1. Button mod: v1.1

   This is simple. The button needs to be disconnected from RST pin and connected to A6 pin of Arduino Nano.
   The firmware was modified to use the button not to reset the Arduino, but to start a test and then confirm
   the test result. Some led blinking was added to indicate in which mode the tester is at any moment. If
   both LEDs are blinking, the tester awaits a chip to be inserted and the button to be pressed for the test
   to start. If green or red LED is lit, this indicates a successful or a failed test. The serial interface
   messages were improved to report the test results, tester's version and when the tester needs a user's
   interaction.

2. Power mod: v1.2

   This mod requires the Button mod to be installed!

   The power mod was created to disable the power on the chip's socket and to only enable it during testing. This
   makes it safe to insert a DRAM chip and also quickly cuts off the power in case a chip is shorted or inserted
   in a wrong way. The simple relay module for Arduino was used to enable or disable +5v power to the socket and
   +12v and -5v converters. The 5v track needs to be cut in 2 places, one wire needs to be added from the cut
   off +5v track to pin 9 of the socket. And the relay needs to be connected to both sides of the cut track.

   The relay is controlled by the signal that has to be connected to D12 pin of Arduino Nano. The relay's control
   circuit needs its own power and ground wires, the +5v has to be taken from the always powered part of the
   circuit. See the pictures in the "Pic" folder for the necessary modifications.

3. OLED screen mod: v1.3

   This mod requires at least the Button mod to be installed!

   To make the tester more informative an OLED screen was added into the project. As Arduino Nano has SCL and SDA
   signals available on A4 and A5 pins and those pins were used by the original tester's designer for the chip's
   socket, before the screen's installation those connections need to be rewired. A4 needs to be rewired to D10
   and A5 needs to be rewired to D11 pins of Arduino Nano. The tracks from A4 and A5 leading to resistors need to
   be cut. Then the screen's SCL and SDA pins need to be connected to Arduino's SCL and SDA pins. See Arduino's
   pinout for reference. Don't forget to connect VCC pin of the screen to Arduino's 3.3v pin and the GND pin of
   the screen to one of GND pins of the Arduino or to any other solder point on the ground plane. See the
   pictures in the "Pic" folder for the necessary modifications.

   The mod is designed to work with the 128x64 SSD1306 OLED screen. Before compiling the firmware please add the
   required Adafruit libaray for SSD1306 screen into your Arduino IDE environment.

   Because of the large size of the firmware after adding the support for the OLED screen, the serial interface
   messages were commented out to save space in the firmware.

When applying the mods, don't forget to upload the corresponding firmware version into Arduino Nano. See the
"Firmware" folder for the available firmwares. Some Arduinos may require selecting "older Bootloader" option
in the IDE to be flashed.

And finally, 3D models of the case were created for the project. There's a wide case that can hold the relay
board and a narrow case for the tester's board by Muttley Black. Both cases can be used for installing the
OLED screen's frame. If you only want the button and screen mods, you can use the narrow case. Please keep
in mind that if you are installing the boards into these cases, you will need to make the socket's lever a bit
longer by attaching a thin plastic or metal tube to it. The current case's design is not ideal for inserting
DRAM chips, so it's recommended to use tweezers if you have trouble inserting or removing a chip.

The Gerber files for that board are located in the "Gerber" folder. The 3D models of the case as well as frames
for the screen are also available in the "Case" folder. The "Original" folder contains the narrow case, the
"Modded" folder contains the wide case.

The videos of the tester's mods can be found on Wierzbowsky's YouTube channel:
https://www.youtube.com/channel/UCefKNRX9MJG4uGqiBBtaJVA/videos
