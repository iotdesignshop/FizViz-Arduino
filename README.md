# FizViz-Arduino
Home to the Arduino Project for the FizViz Package

We developed this for the new kid MKR1000 board.  It's unlikely this project will run on the small boards as we went a bit crazy, but you never
know!

The main loop of the FizViz runs three phases, each timed to give roughly equal CPU time repeated at 50Hz (aka 50 fps).
First phase is Firmata action processing messages, then motor control phase, then NeoPixels.  Motor and Neopixel take up
about 5ms each with the rest available for Firmata.  Motor timing and FPS can be tuned and experimented with.  Have at it!


Project Defines and Setup
-------------------------
The framePeriod define is key to the timing of the system.  We've selected 20ms (aka 50Hz) as a safe value providing lots of CPU time
for everybody while providing a servicable frame rate.  Depending on the application this can certainly be cranked to the max.

Wifi Configuration (wifiConfig.h)
---------------------------------
Set WIFI_SSID and WIFI_PASSPHRASE to your favorite network.  Set STATIC_IP_ADDRESS to the IP you want the device
to operate on.  You will enter this IP into the control applicaiton to take control.

Motor Setup
-----------
The motor setup has a few things to consider.  For RotoMoto (what we call this implementation of the FivFiz codebase), we used a 200 step stepper geared down 2:1.
STEPPER_STEPS is the stepper step count and NEEDLE_STEPS is the number of steps after your gear ratio.

DEFAULT_SPEED defines the step control timing.  We found our motor limit was 180 RPM.  We built the system to run
at 50 fps, which shifts out data to the pixels, shifts out motor control, and leaves time for everything else.  For
RotoMoto, we used 3 motor steps per frame for timing, allowing 180 sets per second, 1.1 degress per step, so 198 degrees
per second.  We didn't push this to see how high it could go, it depends on the cycle time.

SLOW_ZEROING_SPEED and FAST_ZEROING_SPEED define the boot time speeds search for the needle zero (aka magnet crossing
over the reed switch).  SLOW is the initial search, FAST is the full 360 for verifying needle range of motion.  The
needle find the edges of switch activation and zero the needle to the mid point.

Pin out for the motors are defined by STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4.  Check the schematic
to sync up pin to motor wires.

Lighting Setup
--------------
Adafruit NeoPixel strip, the universal light!  In LightControl.h you will find the setup for the NeoPixel strip including
Pixel count, number of color bytes and all that.  We've also included Adafruit's gamma correction table.

We built a couple fun light modes that sync lighting effects with needle movement.  We love the physical dial with lighting combination and
discussed about at least a couple dozen things that would look super awesome.  If you build one, please send us videos of the cool effects made!

Control
-------
See the Windows project in GitHub for the control and setup application.  


