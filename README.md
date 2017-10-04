# Wet-Room-Controller
Possibly useful for general ideas, unless you're building the same hardware setup

I made a wet room for our house, and used an Arduino Mega 2560 to control everything:

* PWM control of waste water pumps (floor was below drain level)
* Water level sensors (magnetometer and pressure sensors)
* Power shower pump
* Flow meter
* Humidity sensor
* Input controls and display screen (illumination of buttons for feedback)
* Extractor fan
* Diverter valves (overhead, hand-wand)
* PWM control of shower lights and alcove lights
* Real time clock
* Temperature sensors

I thought I'd put it up here in case it's useful to someone but will mostly add words if there's anyone out there interested!

The whole room runs off 12vDC so nobody dies. Hardware used:
* Humidity sensor DHT22
* BMP085 pressure sensor
* SSD1306 OLED display (128x64 monochrome)
* HMC5883 3-axis magnetometer
* Cheap water wheel type flow meter in outlet
* 2 x Whale 220 Gulper 12v marine waste pumps
* 2 x SSRs for switching (240v external) extractor fan and external 240v shower pump
* Power MOSFETs to control PWM motor pumps and shower lights
* Individual settings selectable and adjustable with rotary knob/button
* USB socket in control panel for firmware updates

Experimental:
* TX433 433 MHz transmitter to send remote debug data

Main design criteria:
* Use like any other shower, turn the water lever to get water, turn the heat lever to change heat. That part is all mechanical.
* Power failure results in cutting shower power, also pumping. Water will not pump away until power is restored.

Trick features:
I buried a magnet in the plastic waste pipe, on a swinging arm and float. There's a magnetometer on the outside of the pipe that determines 
water level by measuring the magnetic field.

If it's a cold day, the exractor fan operation is delayed until some time after showering, because it was powerful and creating a draught.

Pressing the extractor button when it's extracting will tell the system to raise humidity threshold to just above current, so the fan shortly 
turns off and the new level is set, vice-versa if it's needed on.
