#ifndef FizViz_BackgroundRotateLightControl_h
#define FizViz_BackgroundRotateLightControl_h

#include "LightControl.h"

class BackgroundRotateLightControlClass : public LightControlClass
{
  public:
    BackgroundRotateLightControlClass(uint8_t backgroundColor[NEOPIXEL_COUNT][COLOR_BYTES], uint16_t offset, uint8_t framePeriod, Adafruit_NeoPixel *strip);
    virtual void renderFrame(uint16_t position, NEEDLE_DIRECTION direction) override;

  protected:

    
  private:
    uint16_t offset;
};

#endif
