#ifndef FizViz_MinMaxLightControl_h
#define FizViz_MinMaxLightControl_h

#include "LightControl.h"

class MinMaxLightControlClass : public LightControlClass
{
  public:
    MinMaxLightControlClass(uint8_t backgroundColor[NEOPIXEL_COUNT][COLOR_BYTES], uint16_t resetTime, uint8_t hotNeedleColor[COLOR_BYTES], uint8_t framePeriod, Adafruit_NeoPixel *strip);
    virtual void renderFrame(uint16_t position, NEEDLE_DIRECTION direction) override;

  protected:

  private:
    // not gamma corrected
    uint8_t hotNeedleColor[COLOR_BYTES];

    // How many frames until a min or max value resets
    uint16_t resetFrames;

    // Frames remaining on reset
    uint16_t minResetCounter;
    uint16_t maxResetCounter;
};

#endif
