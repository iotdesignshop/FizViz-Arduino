#include "BackgroundRotateLightControl.h"

BackgroundRotateLightControlClass::BackgroundRotateLightControlClass(uint8_t background[NEOPIXEL_COUNT][COLOR_BYTES], uint16_t offset, uint8_t framePeriod, Adafruit_NeoPixel *strip) : LightControlClass(framePeriod, strip)
{
  memcpy(this->backgroundColors, background, COLOR_BYTES*NEOPIXEL_COUNT);
  this->offset = offset;
}

// Rendering code

void BackgroundRotateLightControlClass::renderFrame(uint16_t pos, NEEDLE_DIRECTION dir)
{
  uint16_t needlePosition = pixelFromInputPosition(pos);

  for (uint16_t backgroundPosition = 0; backgroundPosition < NEOPIXEL_COUNT; backgroundPosition++)
  {
    uint16_t pixelPosition = (backgroundPosition + offset + needlePosition) % NEOPIXEL_COUNT;
    
    for (uint8_t c = 0; c < COLOR_BYTES; c++)
    {
      ledCurrentColors[pixelPosition][c] = gammaCorrect(backgroundColors[backgroundPosition][c]);
    }

    strip->setPixelColor(pixelPosition, ledCurrentColors[pixelPosition][RED],
                                        ledCurrentColors[pixelPosition][GREEN],
                                        ledCurrentColors[pixelPosition][BLUE]);
  }

  strip->show();
}
