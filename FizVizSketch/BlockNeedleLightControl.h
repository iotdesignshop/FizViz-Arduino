#ifndef FizViz_BlockNeedleLightControl_h
#define FizViz_BlockNeedleLightControl_h

#include "LightControl.h"
#include "HotNeedleLightControl.h"

// Number of bytes required to store one bit per neopixel
#define BYTES_ONE_BIT_PER_NEOPIXEL ((NEOPIXEL_COUNT + 7) / 8)

class BlockNeedleLightControlClass : public HotNeedleLightControlClass
{
  public:
    BlockNeedleLightControlClass(uint8_t backgroundColor[NEOPIXEL_COUNT][COLOR_BYTES], uint8_t hotNeedleColor[COLOR_BYTES], float highlightMultiplier, bool useHighlight, uint16_t fadeTime, uint16_t holdTime, uint8_t framePeriod, Adafruit_NeoPixel *strip);
    virtual void renderFrame(uint16_t position, NEEDLE_DIRECTION direction) override;

  protected:

  private:

    // position from last time renderFrame was called
    uint16_t lastPosition;

    // Number of frames to hold a block
    uint16_t holdFrames;

    // block status
    uint8_t blockLeds[BYTES_ONE_BIT_PER_NEOPIXEL];
    bool blockActive;

    // Fade counters for each LED
    uint16_t ledHoldTimes[NEOPIXEL_COUNT];
};

#endif
