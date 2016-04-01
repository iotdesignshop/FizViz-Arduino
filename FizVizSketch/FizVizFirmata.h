/*==============================================================================
 * Options defined via Firmata Sysex calls
 *============================================================================*/
#ifndef FizVizFirmata_h
#define FizVizFirmata_h

#include <Firmata.h>
#include "LightControl.h"
#include "wifiConfig.h"
#include "utility/WiFi101Stream.h"

typedef enum DISPLAY_MODE_TAG
{
    DISPLAY_MODE_HOT_NEEDLE,
    DISPLAY_MODE_BLOCK_NEEDLE,
    DISPLAY_MODE_BACKGROUND_ROTATE,
    DISPLAY_MODE_MIN_MAX
} DISPLAY_MODE;

// These are our custom SysEx commands
#define SET_DISPLAY_MODE 0x40
#define SET_NEEDLE_POSITION 0x41
#define SET_MIN_MAX_CONFIG 0x42
#define SET_BACKGROUND 0x43

#define WIFI_MAX_CONN_ATTEMPTS      3

// --------------------------------------------------
// Callbacks
// --------------------------------------------------
extern "C" {
  typedef void(*fizVizFirmataUpdateCallback)(void);
  typedef void(*fizVizFirmataInitOngoingCallback)(void);
}

class FizVizFirmataClass
{
  public:
    FizVizFirmataClass();

    void init(sysexCallbackFunction sysex, systemResetCallbackFunction sysreset);
    void processInput();
    void maintainConnection();
    
    fizVizFirmataUpdateCallback displayModeUpdateCallback;
    fizVizFirmataUpdateCallback needlePositionUpdateCallback;
    fizVizFirmataUpdateCallback backgroundColorUpdateCallback;
    fizVizFirmataUpdateCallback minMaxUpdateCallback;
    fizVizFirmataInitOngoingCallback initOngoingCallback; 

    // --------------------------------------------------
    // Display Mode Control Parameters
    // --------------------------------------------------

    DISPLAY_MODE displayMode;
    
    uint8_t hotPixelColor[COLOR_BYTES];
    float highlightMultiplier;
    bool useHighlight;
    uint8_t backgroundColor[NEOPIXEL_COUNT][COLOR_BYTES];
    
    uint16_t holdTime; // in ms
    uint16_t fadeTime; // in ms

    uint16_t rotateOffset;

    // --------------------------------------------------
    // Min / Max Parameters
    // --------------------------------------------------

    uint8_t minColor[COLOR_BYTES];
    uint8_t maxColor[COLOR_BYTES];
    bool displayMin;
    bool displayMax;
    uint16_t minMaxResetDuration; // in min

    // --------------------------------------------------
    // NeedleSettings implementation
    // --------------------------------------------------

    // This needlePosition will be in the range 0-999.  It needs to be
    // mapped by the caller to the appropriate range for the given motor.
    uint16_t getNeedlePosition();
    NEEDLE_DIRECTION getNeedleDirection();

    // --------------------------------------------------
    // Processing
    // --------------------------------------------------
    
    void sysex(byte command, byte argc, byte *argv);
    void reset();

  private:
    uint16_t needlePosition;
    NEEDLE_DIRECTION needleDirection;

    int wifiConnectionAttemptCounter = 0;
    int wifiStatus = WL_IDLE_STATUS;
    void initializeWifi();

    WiFi101Stream stream;

    void respondToCapabilityQuery();
    void setDisplayMode(byte argc, byte *argv);
    void setNeedlePosition(byte argc, byte *argv);
    void setMinMaxConfig(byte argc, byte *argv);
    void setBackgroundColor(byte argc, byte *argv);
};

void processSysexCommand(byte command, byte argc, byte *argv);
void processResetCommand();

#endif
