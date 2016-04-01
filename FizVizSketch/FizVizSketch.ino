#include "FizVizFirmata.h"
#include "LightControl.h"
#include "HotNeedleLightControl.h"
#include "BlockNeedleLightControl.h"
#include "BackgroundRotateLightControl.h"
#include "MinMaxLightControl.h"
#include "Error.h"
#include <Stepper.h>

uint8_t framePeriod = 20;         // 50Hz
unsigned long lastFrameTime = 0;  // last frame time in ms

// Allow display of error state
ERROR_MODE errorMode = ERROR_MODE_NONE;

LightControlClass *lightControl;
FizVizFirmataClass FizVizFirmata;

// Used for some lighting while starting up
uint8_t initMode;

// --------------------------------------------------
// Pin definitions
// --------------------------------------------------
#define NEOPIXEL_PIN 0
#define REED_PIN 1
#define STEPPER_PIN_1 2
#define STEPPER_PIN_2 3
#define STEPPER_PIN_3 4
#define STEPPER_PIN_4 5

// May use for dev/testing
// This is the onboard LED, not the NeoPixel data pin
#define LED_PIN 6

// --------------------------------------------------
// Needle
// --------------------------------------------------
// Initial zeroing can be disabled by uncommenting this if the magnet+reed switch have not been set up
//#define DISABLE_ZEROING

// the number of steps on your motor
#define STEPPER_STEPS 200
// The number of steps in a full circle of the needle (using gear ratio)
#define NEEDLE_STEPS 400

// Default speed (RPM) for the stepper
#define DEFAULT_SPEED 180

// Maximum number of steps to move per frame
#define MAX_STEPS_PER_FRAME 3

// Speeds to use while zeroing.
#define SLOW_ZEROING_SPEED 15
#define FAST_ZEROING_SPEED 50

// Uncomment DEBUG_PRINT_POSITION to get details of the needle position update each frame
//#define DEBUG_PRINT_POSITION

// Stepper motor controller
Stepper stepper(STEPPER_STEPS, STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4);

// Number of stepper/needle steps to move per frame
uint16_t stepsPerFrame = 1;

// Current needle position
uint16_t currentPosition;

// Next needle position - we will move towards this at a rate of stepsPerFrame
uint16_t nextPosition;

// The direction to move the needle
NEEDLE_DIRECTION currentDirection;

// Whether or not we have located zero position
bool zeroed;

void initNeedle();
NEEDLE_DIRECTION updateStepperPosition();
void zeroPointer();

// --------------------------------------------------
// NeoPixels
// --------------------------------------------------

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// --------------------------------------------------
// Main code
// --------------------------------------------------

void setup() {
  strip.begin();

  // Start with a basic LightControlClass
  lightControl = new LightControlClass(framePeriod, &strip);
  
  initMode = 0;

  // Initialize and zero our stepper-controlled needle
  initNeedle();
  
  initMode = 1;

  // Set up our Firmata callbacks
  FizVizFirmata.displayModeUpdateCallback = onDisplayModeUpdate;
  FizVizFirmata.needlePositionUpdateCallback = onNeedlePositionUpdate;
  FizVizFirmata.backgroundColorUpdateCallback = onBackgroundColorUpdate;
  FizVizFirmata.minMaxUpdateCallback = onMinMaxUpdate;
  FizVizFirmata.initOngoingCallback = initDraw;
  FizVizFirmata.init(processSysexCommand, processResetCommand);
  
  lastFrameTime = millis();
}

void loop() {
  // Look for Firmata messages
  FizVizFirmata.processInput();
  
  // Lock frame rate
  unsigned long nextFrameTime = lastFrameTime + framePeriod;
  unsigned long currentFrameTime = millis();
  while (nextFrameTime > currentFrameTime)
  {
    // Might as well check for Firmata messages while waiting for next frame to start
    FizVizFirmata.processInput();
    currentFrameTime = millis();
  }
  lastFrameTime = nextFrameTime;

  // Move the needle
  NEEDLE_DIRECTION direction = updateStepperPosition();

  // Update our lights
  lightControl->renderFrame(currentPosition * MAX_POSITION_INPUT / NEEDLE_STEPS, direction);

  FizVizFirmata.maintainConnection();
}

// --------------------------------------------------
// Firmata control update callbacks
// --------------------------------------------------
void onDisplayModeUpdate()
{
  // When we get a new display mode command from Firmata, delete the old light controller and create a new one with the updated settings.
  delete lightControl;
  switch(FizVizFirmata.displayMode)
  {
    case DISPLAY_MODE_HOT_NEEDLE:
      lightControl = new HotNeedleLightControlClass(FizVizFirmata.backgroundColor, FizVizFirmata.hotPixelColor, FizVizFirmata.highlightMultiplier, FizVizFirmata.useHighlight, FizVizFirmata.fadeTime, framePeriod, &strip);
      break;
    case DISPLAY_MODE_BLOCK_NEEDLE:
      lightControl = new BlockNeedleLightControlClass(FizVizFirmata.backgroundColor, FizVizFirmata.hotPixelColor, FizVizFirmata.highlightMultiplier, FizVizFirmata.useHighlight, FizVizFirmata.fadeTime, FizVizFirmata.holdTime, framePeriod, &strip);
      break;
    case DISPLAY_MODE_BACKGROUND_ROTATE:
      lightControl = new BackgroundRotateLightControlClass(FizVizFirmata.backgroundColor, FizVizFirmata.rotateOffset, framePeriod, &strip);
      break;
    case DISPLAY_MODE_MIN_MAX:
      lightControl = new MinMaxLightControlClass(FizVizFirmata.backgroundColor, FizVizFirmata.fadeTime, FizVizFirmata.hotPixelColor, framePeriod, &strip);
      break;
  }
  lightControl->setMinMaxConfig(FizVizFirmata.displayMin, FizVizFirmata.displayMax, FizVizFirmata.minColor, FizVizFirmata.maxColor, FizVizFirmata.minMaxResetDuration, MIN_MAX_DRAW_WIDTH_DEFAULT);
}

void onBackgroundColorUpdate()
{
  // Update the background color array
  lightControl->setBackground(FizVizFirmata.backgroundColor);
}

void onMinMaxUpdate()
{
  // Update light control min / max configuration
  lightControl->setMinMaxConfig(FizVizFirmata.displayMin, FizVizFirmata.displayMax, FizVizFirmata.minColor, FizVizFirmata.maxColor, FizVizFirmata.minMaxResetDuration, MIN_MAX_DRAW_WIDTH_DEFAULT);
}

void onNeedlePositionUpdate()
{
  // Set the desired needle position based on the Firmata command
  currentDirection = FizVizFirmata.getNeedleDirection();
  nextPosition = (FizVizFirmata.getNeedlePosition() % (int)MAX_POSITION_INPUT) * ((float)NEEDLE_STEPS / MAX_POSITION_INPUT);
}

// Helper to draw during initialization (i.e. before we hit the main rendering loop)
void initDraw()
{
  lightControl->drawDuringInit(initMode);
}

void processSysexCommand(byte command, byte argc, byte *argv)
{
  FizVizFirmata.sysex(command, argc, argv);
}
void processResetCommand()
{
  FizVizFirmata.reset();
}

// Initialize our stepper needle - zero the pointer
void initNeedle()
{
  pinMode(REED_PIN, INPUT_PULLUP);
  zeroed = false;
  zeroPointer();
}

// Move the stepper based on the next position and direction. Returns the actual direction moved (clockwise or counterclockwise).
NEEDLE_DIRECTION updateStepperPosition()
{
  NEEDLE_DIRECTION direction = NEEDLE_DIRECTION_CLOCKWISE;
  
  if (currentPosition != nextPosition)
  {
    stepper.setSpeed(DEFAULT_SPEED);

    int steps = 0;
    
    switch (currentDirection)
    {
      case NEEDLE_DIRECTION_CLOCKWISE:
      case NEEDLE_DIRECTION_COUNTERCLOCKWISE:
      {
        direction = currentDirection;
      }
      break;
      case NEEDLE_DIRECTION_CLOSEST:
      {
        int diff2 =  (nextPosition - currentPosition) * 2;
        if (diff2 < -NEEDLE_STEPS || (diff2 > 0 && diff2 < NEEDLE_STEPS))
        {
          direction = NEEDLE_DIRECTION_CLOCKWISE;
        }
        else
        {
          direction = NEEDLE_DIRECTION_COUNTERCLOCKWISE;
        }
      }
      break;
      case NEEDLE_DIRECTION_DO_NOT_PASS_ZERO:
      {
        if (nextPosition > currentPosition)
        {
          direction = NEEDLE_DIRECTION_CLOCKWISE;
        }
        else
        {
          direction = NEEDLE_DIRECTION_COUNTERCLOCKWISE;
        }
      }
      break;
    }

    switch (direction)
    {
      case NEEDLE_DIRECTION_CLOCKWISE:
      {
        if (nextPosition > currentPosition)
        {
          steps = nextPosition - currentPosition;
        }
        else
        {
          steps = (NEEDLE_STEPS - currentPosition) + nextPosition;
        }
      }
      break;
      case NEEDLE_DIRECTION_COUNTERCLOCKWISE:
      {
        if (nextPosition < currentPosition)
        {
          steps = nextPosition - currentPosition;
        }
        else
        {
          steps = (nextPosition - NEEDLE_STEPS) - currentPosition;
        }
      }
      break;
    }

#ifdef DEBUG_PRINT_POSITION
    Serial.print("Updating position: ");
    Serial.print(currentPosition);
    Serial.print(" -> ");
    Serial.print(nextPosition);
    Serial.print(", direction ");
    Serial.print(direction);
    Serial.print(", total steps: ");
    Serial.print(steps);
#endif

    // Modify steps to apply some acceleration at the beginning/deceleration at the end.
    
    // these #defines can be changed to tweak acceleration
#define ACCEL_STEPS_MIN 10
#define ACCEL_STEPS_MED 20

    if ((steps > 0 && steps < ACCEL_STEPS_MIN) || (steps < 0 && steps > -ACCEL_STEPS_MIN))
    {
      stepsPerFrame = 1;
    }
    else if ((steps > 0 && steps < ACCEL_STEPS_MED) || (steps < 0 && steps > -ACCEL_STEPS_MED))
    {
      stepsPerFrame = 2;
    }
    else
    {
      stepsPerFrame = 3;
    }
    
    if (steps < -stepsPerFrame)
    {
      steps = -stepsPerFrame;
    }
    else if (steps > stepsPerFrame)
    {
      steps = stepsPerFrame;
    }
    
#ifdef DEBUG_PRINT_POSITION
    Serial.print(", steps this frame: ");
    Serial.print(steps);
    Serial.print(",");
#endif
    
    stepper.step(steps);
    if (steps < 0 && -steps > currentPosition)
    {
      currentPosition = NEEDLE_STEPS + (steps + currentPosition);
    }
    else
    {
      currentPosition = (steps + currentPosition) % NEEDLE_STEPS;
    }
    
#ifdef DEBUG_PRINT_POSITION
    Serial.print(" new pos ");
    Serial.println(currentPosition);
#endif
  }
  else
  {
  }

  return direction;
}

// Zero the pointer based on reed switch
void zeroPointer()
{
  // Allow bypassing the zeroing during development
#ifdef DISABLE_ZEROING
    nextPosition = 0;
    currentPosition = 0;
    zeroed = 1;
    return;
#endif
  
  if (zeroed)
  {
    nextPosition = 0;
    currentDirection = NEEDLE_DIRECTION_CLOSEST;
  }
  else
  {
    // Initial zero-ing; we need to look for the reed switch
    stepper.setSpeed(SLOW_ZEROING_SPEED);

    // Find that magnet
    while (!zeroed)
    {
      initDraw();

      uint8_t reed = digitalRead(REED_PIN);
      if (reed)
      {
        stepper.step(1);
      }
      else
      {
        zeroed = 1;
        nextPosition = 0;
        currentPosition = 0;
      }
    }
    
    
    // Now loop around and make sure our steps are correct
    int stepCount = 0;
    int maxSteps = NEEDLE_STEPS * 1.25;
    uint8_t buffer = 6;  // Allowable overlap

    uint8_t slowStage1 = buffer * 6;
    uint8_t slowStage2 = buffer * 3;
    uint16_t midSpeed = (SLOW_ZEROING_SPEED + FAST_ZEROING_SPEED) / 2;

    while (stepCount++ < slowStage2)
    {
      initDraw();
      stepper.step(1);
    }
    
    stepper.setSpeed(midSpeed);

    while (stepCount++ < slowStage1)
    {
      initDraw();
      stepper.step(1);
    }
    
    stepper.setSpeed(FAST_ZEROING_SPEED);

    while (stepCount++ < maxSteps)
    {
      initDraw();
      stepper.step(1);

      // Slow down towards end
      int diff = NEEDLE_STEPS - stepCount;

      if (diff < slowStage2)
      {
        stepper.setSpeed(SLOW_ZEROING_SPEED);
      }
      else if (diff < slowStage1)
      {
        stepper.setSpeed(midSpeed);
      }

      uint8_t reed = digitalRead(REED_PIN);
      if (!reed)
      {
        // Re-found zero
        if (diff < -buffer || diff > buffer)
        {
          // Zeroing failed
          Serial.print("Zeroing failed - zero found at unexpected location on second pass. Difference was ");
          Serial.println(diff);
          errorMode = ERROR_MODE_ZERO_FAILED;
          return;
        }
        break;
      }
    }

    if (stepCount > maxSteps)
    {
      // Zeroing failed
      Serial.println("Zeroing failed - did not re-find zero.");
      errorMode = ERROR_MODE_ZERO_FAILED;
      return;
    }

    // Found minimum reed position.  Now look for maximum.
    stepper.setSpeed(SLOW_ZEROING_SPEED);
    stepCount = 0;
    maxSteps = buffer * 4;
    while (stepCount++ < maxSteps)
    {
      initDraw();
      stepper.step(1);

      uint8_t reed = digitalRead(REED_PIN);
      if (reed)
      {
        // Left the zero zone.
        break;
      }
    }

    if (stepCount > maxSteps)
    {
      // Zeroing failed
      Serial.println("Zeroing failed - did not exit zero on final pass.");
      errorMode = ERROR_MODE_ZERO_FAILED;
      return;
    }

    // Now step back to the middle of the zone we just located
    maxSteps = stepCount / 2;
    for (stepCount = 0; stepCount < maxSteps; stepCount++)
    {
      initDraw();
      stepper.step(-1);
    }
    
    stepper.setSpeed(DEFAULT_SPEED);
  }
}

