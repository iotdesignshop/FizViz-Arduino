#include "LightControl.h"

LightControlClass::LightControlClass(uint8_t framePeriod, Adafruit_NeoPixel *strip) : framePeriod(framePeriod), strip(strip)
{
  reset();
}

void LightControlClass::reset()
{
  memset(ledCurrentColors, 0, sizeof(uint8_t)*COLOR_BYTES*NEOPIXEL_COUNT);
  memset(backgroundColors, 0, sizeof(uint8_t)*COLOR_BYTES*NEOPIXEL_COUNT);
  memset(ledCounters, 0, sizeof(uint16_t)*NEOPIXEL_COUNT);
  strip->clear();
  strip->show();
  
  setTime(0,0,0,0,0,0);
}

void LightControlClass::renderFrame(uint16_t position, NEEDLE_DIRECTION direction)
{
  blink(strip->Color(0,150,0));
}

void LightControlClass::blink(uint32_t blinkColor)
{
  static uint8_t fps = 1000 / framePeriod;
  static uint8_t frameCounter = 0;
  static bool blinkOn = true;

  if (blinkOn)
  {
      for (uint16_t i=0; i<strip->numPixels(); i++)
      {
        strip->setPixelColor(i, blinkColor);
      }
  }
  else
  {
      for (uint16_t i=0; i<strip->numPixels(); i++)
      {
        strip->setPixelColor(i, gammaCorrect(backgroundColors[i][RED]), gammaCorrect(backgroundColors[i][GREEN]), gammaCorrect(backgroundColors[i][BLUE]));
      }
  }
  strip->show();

  frameCounter = (frameCounter + 1) % fps;
  if (frameCounter == 0)
  {
    blinkOn = !blinkOn;
  }
}

void LightControlClass::drawDuringInit(uint8_t initMode)
{
  static uint16_t chasePosition = 0;
  static unsigned long nextDrawTime = millis();

  unsigned long currentTime = millis();
  if (currentTime < nextDrawTime)
  {
    return;
  }

  nextDrawTime = currentTime + 20;
  chasePosition = (chasePosition + 2) % NEOPIXEL_COUNT;
  
  for(uint16_t i = 0; i < NEOPIXEL_COUNT; i++)
  {
    strip->setPixelColor(i, 0);
  }

  switch (initMode)
  {
    case 0:
      strip->setPixelColor(chasePosition, strip->Color(0, 150, 0));
      break;
    case 1:
      strip->setPixelColor(chasePosition, strip->Color(0, 0, 150));
      break;
    case 2:
      strip->setPixelColor(chasePosition, strip->Color(0, 150, 150));
      break;
    case 3:
      strip->setPixelColor(chasePosition, strip->Color(150, 150, 0));
      break;
    default:
      strip->setPixelColor(chasePosition, strip->Color(150, 0, 150));
      break;
  }

  strip->show();
}

void LightControlClass::decrementCounters(uint16_t counter[NEOPIXEL_COUNT])
{
  for(uint16_t i = 0; i < NEOPIXEL_COUNT; i++)
  {  
    if (counter[i] > 0)
    {
      counter[i]--;
    }
  }
}

void LightControlClass::setBackground(uint8_t background[NEOPIXEL_COUNT][COLOR_BYTES])
{
  memcpy(backgroundColors, background, NEOPIXEL_COUNT * COLOR_BYTES);
}

void LightControlClass::updateMaximum(uint16_t ledPosition)
{
  static time_t lastMoveTime = 0;

  //Set maximumLedPosition if reset time has reached OR new maximum reached
  if(now() > lastMoveTime + minMaxResetDuration || ledPosition >= maximumLedPosition){
    maximumLedPosition = ledPosition;
    lastMoveTime = now();
  }

  // TODO temporary fix for stuck maximum
  if(maximumLedPosition == NEOPIXEL_COUNT-1){
    maximumLedPosition = 0;
  }
}

void LightControlClass::updateMinimum(uint16_t ledPosition)
{
  static time_t lastMoveTime = 0;

  //Set minimumLedPosition if reset time has reached OR new minimum reached
  if(now() > lastMoveTime + minMaxResetDuration || ledPosition <= minimumLedPosition){
    minimumLedPosition = ledPosition;
    lastMoveTime = now();
  }

  if(minimumLedPosition == 0){
    minimumLedPosition = NEOPIXEL_COUNT;
  }
}

void LightControlClass::drawMaximum()
{
  uint16_t maxLocation = (maximumLedPosition + 1) % NEOPIXEL_COUNT;

  // draws from maxLocation till maxLocation + width
  for(int i = 0; i < minMaxDrawWidth; i++){
    
    ledCurrentColors[maxLocation][RED]   = maximumColor[RED];
    ledCurrentColors[maxLocation][GREEN] = maximumColor[GREEN];
    ledCurrentColors[maxLocation][BLUE]  = maximumColor[BLUE];
    
    strip->setPixelColor(maxLocation, ledCurrentColors[maxLocation][RED],
                                      ledCurrentColors[maxLocation][GREEN],
                                      ledCurrentColors[maxLocation][BLUE]);

    maxLocation = (maxLocation + 1) % NEOPIXEL_COUNT;
  }
}

void LightControlClass::drawMinimum()
{
  int minLocation = minimumLedPosition - 1;

  // draws from minLocation till minLocation - width
  for(uint16_t i = 0; i < minMaxDrawWidth; i++){

    if(minLocation < 0){
      minLocation = NEOPIXEL_COUNT + minLocation;
    }
    
    ledCurrentColors[minLocation][RED]   = minimumColor[RED];
    ledCurrentColors[minLocation][GREEN] = minimumColor[GREEN];
    ledCurrentColors[minLocation][BLUE]  = minimumColor[BLUE];
    
    strip->setPixelColor(minLocation, ledCurrentColors[minLocation][RED],
                                      ledCurrentColors[minLocation][GREEN],
                                      ledCurrentColors[minLocation][BLUE]);

    minLocation--;
  }
}

void LightControlClass::setMinMaxConfig(bool useMinimum, bool useMaximum, uint8_t minimumColor[COLOR_BYTES], 
                                        uint8_t maximumColor[COLOR_BYTES], uint16_t minMaxResetDurationMinutes, uint8_t minMaxDrawWidth){
  this->useMinimum = useMinimum;
  this->useMaximum = useMaximum;

  memcpy(this->minimumColor, minimumColor, COLOR_BYTES);
  memcpy(this->maximumColor, maximumColor, COLOR_BYTES);

  this->minMaxResetDuration = minMaxResetDurationMinutes*SECONDS_PER_MINUTE;

  this->minMaxDrawWidth = minMaxDrawWidth;
}

