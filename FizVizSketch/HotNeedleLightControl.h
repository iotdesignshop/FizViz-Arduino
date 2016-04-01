#ifndef FizViz_HotNeedleLightControl_h
#define FizViz_HotNeedleLightControl_h

#include "LightControl.h"

class HotNeedleLightControlClass : public LightControlClass
{
  public:
    HotNeedleLightControlClass(uint8_t backgroundColor[NEOPIXEL_COUNT][COLOR_BYTES], uint8_t hotNeedleColor[COLOR_BYTES], float highlightMultiplier, bool useHighlight, uint16_t fadeTime, uint8_t framePeriod, Adafruit_NeoPixel *strip);
    virtual void renderFrame(uint16_t position, NEEDLE_DIRECTION direction) override;

  protected:
    void draw(uint16_t needlePosition);
    
    // Number of frames it takes to fade
    uint16_t fadeFrames;
    
  private:
    // not gamma corrected
    uint8_t hotNeedleColor[COLOR_BYTES];

    // Toggle to use highlight as needle color
    bool useHighlight;

    // Multiplier for hotneedle
    float highlightMultiplier;
};

#endif
