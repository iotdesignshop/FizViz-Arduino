# FizViz-Arduino
Home to the Arduino Project for the FizViz Package

Cycle sequence overview.


Project Defines and Setup
-------------------------

FizViz setup details below.  You'll want to adjust the setting based on the number of pixels you're using and
mechanics connected to the stepper.  Wifi setup is also covered!

Wifi Configuration (wifiConfig.h)
---------------------------------

Set WIFI_SSID and WIFI_PASSPHRASE to your favorite network.  Set STATIC_IP_ADDRESS to the IP you want the device
to operate on.  You will enter this IP into the control applicaiton to take control.

Motor Setup
-----------
The motor setup has a few things to consider.  For RotoMoto, we used a 200 step stepper geared down 2:1.
STEPPER_STEPS is the stepper step count and NEEDLE_STEPS is the number of steps after your gear ratio.

DEFAULT_SPEED defines the step control timing.  We found our motor limit was 180 RPM.  We built the system to run
at 60 fps, which shifts out data to the pixels, shifts out motor control, and leaves time for everything else.  For
RotoMoto, we used 3 motor steps per frame for timing, allowing 180 sets per second, 1.1 degress per step, so 198 degrees
per second.  We didn't push this to see how high it could go, it depends on the cycle time.

SLOW_ZEROING_SPEED and FAST_ZEROING_SPEED define the boot time speeds search for the needle zero (aka magnet crossing
over the reed switch).  SLOW is the initial search, FAST is the full 360 for verifying needle range of motion.  The
needle find the edges of switch activation and zero the needle to the mid point.



