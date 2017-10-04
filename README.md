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

The whole room runs off 12vDC so nobody dies.

Trick features:
I buried a magnet in the plastic waste pipe, on a swinging arm and float. There's a magnetometer on the outside of the pipe that determines 
water level by measuring the magnetic field.

If it's a cold day, the exractor fan operation is delayed until some time after showering, because it was powerful and creating a draught.

Pressing the extractor button when it's extracting will tell the system to raise humidity threshold to just above current, so the fan shortly 
turns off and the new level is set, vice-versa if it's needed on.
