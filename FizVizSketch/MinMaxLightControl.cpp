#include "MinMaxLightControl.h"

MinMaxLightControlClass::MinMaxLightControlClass(uint8_t backgroundColor[NEOPIXEL_COUNT][COLOR_BYTES], uint16_t resetTime, uint8_t hotNeedleColor[COLOR_BYTES], uint8_t framePeriod, Adafruit_NeoPixel *strip) : LightControlClass(framePeriod, strip)
{
  memcpy(this->hotNeedleColor, hotNeedleColor, COLOR_BYTES);
  memcpy(this->backgroundColors, backgroundColor, COLOR_BYTES*NEOPIXEL_COUNT);
  resetFrames = resetTime * 1000 / framePeriod;
  minimumLedPosition = 0;
  maximumLedPosition = 0;
  minResetCounter = resetFrames;
  maxResetCounter = resetFrames;
}

// Rendering code

void MinMaxLightControlClass::renderFrame(uint16_t pos, NEEDLE_DIRECTION dir)
{
  uint16_t needlePosition = pixelFromInputPosition(pos);

  // Check for minimum update
  if (needlePosition < minimumLedPosition || minResetCounter == 0)
  {
    minimumLedPosition = needlePosition;
    minResetCounter = resetFrames;
  }
  else
  {
    // Count down to reset if unchanged
    minResetCounter--;
  }

  if (needlePosition > maximumLedPosition || maxResetCounter == 0)
  {
    maximumLedPosition = needlePosition;
    maxResetCounter = resetFrames;
  }
  else
  {
    maxResetCounter--;
  }

  // Reset
  for (uint16_t p = 0; p < NEOPIXEL_COUNT; p++)
  {
    for (uint8_t c = 0; c < COLOR_BYTES; c++)
    {
      ledCurrentColors[p][c] = gammaCorrect(hotNeedleColor[c]);
    }
    strip->setPixelColor(p, ledCurrentColors[p][RED],
                            ledCurrentColors[p][GREEN],
                            ledCurrentColors[p][BLUE]);
  }

  // Now draw: minimum bar
  for (uint16_t p = 0; p <= minimumLedPosition; p++)
  {
    for (uint8_t c = 0; c < COLOR_BYTES; c++)
    {
      ledCurrentColors[p][c] = gammaCorrect(minimumColor[c]);
    }

    strip->setPixelColor(p, ledCurrentColors[p][RED],
                            ledCurrentColors[p][GREEN],
                            ledCurrentColors[p][BLUE]);
  }

  // Maximum bar
  for (uint16_t p = maximumLedPosition; p < NEOPIXEL_COUNT; p++)
  {
    for (uint8_t c = 0; c < COLOR_BYTES; c++)
    {
      ledCurrentColors[p][c] = gammaCorrect(maximumColor[c]);
    }

    strip->setPixelColor(p, ledCurrentColors[p][RED],
                            ledCurrentColors[p][GREEN],
                            ledCurrentColors[p][BLUE]);
  }

  strip->show();
}
