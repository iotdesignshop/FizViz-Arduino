#ifndef FizVizLightControl_h
#define FizVizLightControl_h

#include <Adafruit_NeoPixel.h>
#include "NeedleSettings.h"
#include <Time.h>
#include <TimeLib.h>

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#define NEOPIXEL_COUNT 133
#define COLOR_BYTES 3
#define SECONDS_PER_MINUTE 60
#define MIN_MAX_DRAW_WIDTH_DEFAULT 3

#define RED 0
#define GREEN 1
#define BLUE 2

#define MAX_POSITION_INPUT 1200.0

// Gamma correction table
const uint8_t PROGMEM fz_gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

class LightControlClass
{
  public:
    LightControlClass(uint8_t framePeriod, Adafruit_NeoPixel *strip);

    virtual void renderFrame(uint16_t position, NEEDLE_DIRECTION direction);

    virtual void reset();

    // Sets background with color value stored in background array
    void setBackground(uint8_t background[NEOPIXEL_COUNT][COLOR_BYTES]);

    // Sets the configuration for maximum and minimum endstop
    void setMinMaxConfig(bool useMinimum, bool useMaximum, uint8_t minimumColor[COLOR_BYTES], uint8_t maximumColor[COLOR_BYTES], uint16_t minMaxResetDurationMinutes, uint8_t minMaxDrawWidth);

    // Simple draw function that doesn't rely on being called at a set frame rate
    void drawDuringInit(uint8_t initMode);

  protected:
    Adafruit_NeoPixel *strip;

    // Length of time of a frame in ms
    uint8_t framePeriod;

    // Simple blink function
    void blink(uint32_t blinkColor);

    // Gamma correct a single color byte
    static inline uint8_t gammaCorrect(uint8_t color) { return pgm_read_byte(&fz_gamma[color]); }

    // Current LED color values.  These should be gamma corrected.
    uint8_t ledCurrentColors[NEOPIXEL_COUNT][COLOR_BYTES];
    
    // RGB Values for the background
    // These are not gamma corrected
    uint8_t backgroundColors[NEOPIXEL_COUNT][COLOR_BYTES];

    // Counters for each LED; used for animations
    uint16_t ledCounters[NEOPIXEL_COUNT];

    void decrementCounters(uint16_t counter[NEOPIXEL_COUNT]);

    // Color for maximum end stop
    uint8_t maximumColor[COLOR_BYTES];

    // Color for minimum end stop
    uint8_t minimumColor[COLOR_BYTES];

    // Toggle to display maxmimum
    bool useMaximum;

    // Toggle to display minimum
    bool useMinimum;

    // Stores the maximum led position displayed
    uint16_t maximumLedPosition;

    // Stores the minimum led position displayed
    uint16_t minimumLedPosition;

    // Stores the width of the max and min endstop
    uint8_t minMaxDrawWidth;

    // Duration before max and min resets to current led position
    time_t minMaxResetDuration;

    // Computes maximumLedPositon and monitor reset timing
    void updateMaximum(uint16_t ledPosition);

    // Computes minimumLedPositon and monitor reset timing
    void updateMinimum(uint16_t ledPosition);

    // Draws maximum endstop
    void drawMaximum();

    // Draws minimum endstop
    void drawMinimum();

    static inline uint16_t pixelFromInputPosition(uint16_t inputPosition) { return (uint16_t)(inputPosition * NEOPIXEL_COUNT / MAX_POSITION_INPUT) % NEOPIXEL_COUNT; }
    
  private:
};

#endif
