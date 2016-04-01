#include "BlockNeedleLightControl.h"

BlockNeedleLightControlClass::BlockNeedleLightControlClass(uint8_t background[NEOPIXEL_COUNT][COLOR_BYTES], uint8_t hotNeedleColor[COLOR_BYTES], float highlightMultiplier, bool useHighlight, uint16_t fadeTime, uint16_t holdTime, uint8_t framePeriod, Adafruit_NeoPixel *strip) :
  HotNeedleLightControlClass(background, hotNeedleColor, highlightMultiplier, useHighlight, fadeTime, framePeriod, strip)
{
  holdFrames = holdTime / framePeriod;
  Serial.print("hold frames ");
  Serial.println(holdFrames);
  lastPosition = 0;

  memset(blockLeds, 0, BYTES_ONE_BIT_PER_NEOPIXEL);
  memset(ledHoldTimes, 0, sizeof(uint16_t) * NEOPIXEL_COUNT);
  
  blockActive = false;
}

// Rendering code

void BlockNeedleLightControlClass::renderFrame(uint16_t pos, NEEDLE_DIRECTION dir)
{
  uint16_t needlePosition = pixelFromInputPosition(pos);

  // The current position is always lit
  ledHoldTimes[needlePosition] = holdFrames;

  // Update the block status
  if (pos == lastPosition)
  {
    if (blockActive)
    {
      blockActive = false;
      memset(blockLeds, 0, BYTES_ONE_BIT_PER_NEOPIXEL);
    }
  }
  else
  {
    if (!blockActive)
    {
      uint16_t lastNeedlePosition = pixelFromInputPosition(lastPosition);
      blockLeds[lastNeedlePosition / 8] |= (1 << (lastNeedlePosition % 8));
    }
    blockLeds[needlePosition / 8] |= (1 << (needlePosition % 8));
    blockActive = true;

  }

  

  // If there is an active block, everything in it gets held
  if (blockActive)
  {
    for (int i = 0; i < BYTES_ONE_BIT_PER_NEOPIXEL; i++)
    {
      for (int j = 0; j < 8 && i * 8 + j < NEOPIXEL_COUNT; j++)
      {
        if (blockLeds[i] & (1 << j)) ledHoldTimes[i * 8 + j] = holdFrames;
      }
    }
  }

  // Update hold times
  decrementCounters(ledHoldTimes);

  // Now decrement fade timers wherever hold timers have run out
  for(uint16_t i = 0; i < NEOPIXEL_COUNT; i++)
  {
    if (ledHoldTimes[i] == 0)
    {
      if (ledCounters[i] > 0)
      {
        ledCounters[i]--;
      }
    }
    else
    {
      // Anywhere the hold time is active will be fully foreground color
      ledCounters[i] = fadeFrames;
    }
  }

  // Finally, draw
  draw(needlePosition);

  lastPosition = pos;
}
