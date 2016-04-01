#include "FizVizFirmata.h"

#include "utility/firmataDebug.h"
/*
 * Uncomment the #define SERIAL_DEBUG line below to receive serial output messages relating to your
 * connection that may help in the event of connection issues. If defined, some boards may not begin
 * executing this sketch until the Serial console is opened.
 */
//#define SERIAL_DEBUG

// Disable wifi initialization by uncommenting this
//#define DISABLE_FIRMATA

FizVizFirmataClass::FizVizFirmataClass()
{
  displayModeUpdateCallback = NULL;
  needlePositionUpdateCallback = NULL;
  backgroundColorUpdateCallback = NULL;
  initOngoingCallback = NULL;

  wifiConnectionAttemptCounter = 0;
  wifiStatus = WL_IDLE_STATUS;

  reset();
}

void FizVizFirmataClass::init(sysexCallbackFunction sysex, systemResetCallbackFunction sysreset)
{
#ifndef DISABLE_FIRMATA
    if (initOngoingCallback) initOngoingCallback();

    initializeWifi();

    if (initOngoingCallback) initOngoingCallback();

    Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);

    if (initOngoingCallback) initOngoingCallback();
  
    Firmata.attach(START_SYSEX, sysex);
    Firmata.attach(SYSTEM_RESET, sysreset);

    if (initOngoingCallback) initOngoingCallback();

    // start up Network Firmata:
    Firmata.begin(stream);

    if (initOngoingCallback) initOngoingCallback();
#endif
}

void FizVizFirmataClass::processInput()
{
#ifndef DISABLE_FIRMATA
  while (Firmata.available())
  {
    Firmata.processInput();
  }
#endif
}

void FizVizFirmataClass::maintainConnection()
{
#ifndef DISABLE_FIRMATA
  stream.maintain();
#endif
}


void FizVizFirmataClass::sysex(byte command, byte argc, byte *argv)
{
  switch (command) {
    // The Microsoft Firmata library will send a capability query to
    // confirm connection.  We respond with no pins available.
    case CAPABILITY_QUERY:
      respondToCapabilityQuery();
      break;
    case SET_DISPLAY_MODE:
      setDisplayMode(argc, argv);
      break;
    case SET_NEEDLE_POSITION:
      setNeedlePosition(argc, argv);
      break;
    case SET_MIN_MAX_CONFIG:
      setMinMaxConfig(argc, argv);
      break;
    case SET_BACKGROUND:
      setBackgroundColor(argc, argv);
      break;
  }
}

void FizVizFirmataClass::respondToCapabilityQuery()
{
  Firmata.write(START_SYSEX);
  Firmata.write(CAPABILITY_RESPONSE);
  Firmata.write(END_SYSEX);
}

void FizVizFirmataClass::setDisplayMode(byte argc, byte *argv)
{
  if (argc > 0) {
    displayMode = (DISPLAY_MODE)argv[0];
  }
  switch (displayMode)
  {
    case DISPLAY_MODE_HOT_NEEDLE:
    {
      if (argc == 18)
      {
        hotPixelColor[RED] = argv[1] + (argv[2] << 7);
        hotPixelColor[GREEN] = argv[3] + (argv[4] << 7);
        hotPixelColor[BLUE] = argv[5] + (argv[6] << 7);
        useHighlight = argv[7];
        *((byte*)(&highlightMultiplier)) = argv[8] + (argv[9] << 7);
        *((byte*)(&highlightMultiplier) + 1) = argv[10] + (argv[11] << 7);
        *((byte*)(&highlightMultiplier) + 2) = argv[12] + (argv[13] << 7);
        *((byte*)(&highlightMultiplier) + 3) = argv[14] + (argv[15] << 7);
        fadeTime = argv[16] + (argv[17] << 7);
      }
      break;
    }
    case DISPLAY_MODE_BLOCK_NEEDLE:
    {
      if (argc == 20)
      {
        hotPixelColor[RED] = argv[1] + (argv[2] << 7);
        hotPixelColor[GREEN] = argv[3] + (argv[4] << 7);
        hotPixelColor[BLUE] = argv[5] + (argv[6] << 7);
        useHighlight = argv[7];
        *((byte*)(&highlightMultiplier)) = argv[8] + (argv[9] << 7);
        *((byte*)(&highlightMultiplier) + 1) = argv[10] + (argv[11] << 7);
        *((byte*)(&highlightMultiplier) + 2) = argv[12] + (argv[13] << 7);
        *((byte*)(&highlightMultiplier) + 3) = argv[14] + (argv[15] << 7);
        fadeTime = argv[16] + (argv[17] << 7);
        holdTime = argv[18] + (argv[19] << 7);
      }
      break;
    }
    case DISPLAY_MODE_BACKGROUND_ROTATE:
    {
      if (argc == 3)
      {
        rotateOffset = argv[1] + (argv[2] << 7);
      }
      break;
    }
    case DISPLAY_MODE_MIN_MAX:
    {
      if (argc == 9)
      {
        fadeTime = argv[1] + (argv[2] << 7);
        hotPixelColor[RED] = argv[3] + (argv[4] << 7);
        hotPixelColor[GREEN] = argv[5] + (argv[6] << 7);
        hotPixelColor[BLUE] = argv[7] + (argv[7] << 7);
      }
      break;
    }
  }
  if (displayModeUpdateCallback)
  {
    displayModeUpdateCallback();
  }
}

void FizVizFirmataClass::setNeedlePosition(byte argc, byte *argv)
{
  if (argc == 3)
  {
    needlePosition = argv[0] + (argv[1] << 7);
    needleDirection = (NEEDLE_DIRECTION)argv[2];
  }
  if (needlePositionUpdateCallback)
  {
    needlePositionUpdateCallback();
  }
}

void FizVizFirmataClass::setBackgroundColor(byte argc, byte *argv)
{
  if (argc >= 2)
  {
    uint16_t startPixel = argv[0] + (argv[1] << 7);
    byte pixelCount = argv[2];

    if (startPixel + pixelCount > NEOPIXEL_COUNT)
    {
      Serial.println("Warning - received background pixel data that was too long");
      return;
    }

    if (argc == (3 + 6 * pixelCount))
    {
      uint16_t p = startPixel;
      byte a = 3;
      for (int p = startPixel; p < pixelCount + startPixel; p++)
      {
        for (int j = 0; j < COLOR_BYTES; j++)
        {
          backgroundColor[p][j] = argv[a++] + (argv[a++] << 7);
        }
      }
    }

    if (startPixel + pixelCount == NEOPIXEL_COUNT && backgroundColorUpdateCallback)
    {
      backgroundColorUpdateCallback();
    }
  }
}

void FizVizFirmataClass::setMinMaxConfig(byte argc, byte *argv)
{
  if (argc == 16)
  {
    displayMin = argv[0];
    displayMax = argv[1];
    
    minColor[RED] = argv[2] + (argv[3] << 7);
    minColor[GREEN] = argv[4] + (argv[5] << 7);
    minColor[BLUE] = argv[6] + (argv[7] << 7);
    maxColor[RED] = argv[8] + (argv[9] << 7);
    maxColor[GREEN] = argv[10] + (argv[11] << 7);
    maxColor[BLUE] = argv[12] + (argv[13] << 7);
    minMaxResetDuration = argv[14] + (argv[15] << 7);

    if (minMaxUpdateCallback)
    {
      minMaxUpdateCallback();
    }
  }
}

void FizVizFirmataClass::reset()
{
  displayMode = DISPLAY_MODE_HOT_NEEDLE;
  
  hotPixelColor[0] = 0x80;
  hotPixelColor[1] = 0x00;
  hotPixelColor[2] = 0x00;
  useHighlight = false;
  highlightMultiplier = 2.0;
  for (int i = 0; i < COLOR_BYTES; i++)
  {
    for (int j = 0; j < NEOPIXEL_COUNT; j++)
    {
      backgroundColor[i][j] = 0x00;
    }
  }
  holdTime = 1500;
  fadeTime = 5000;
}

uint16_t FizVizFirmataClass::getNeedlePosition()
{
  return this->needlePosition;
}

NEEDLE_DIRECTION FizVizFirmataClass::getNeedleDirection()
{
  return needleDirection;
}

void FizVizFirmataClass::initializeWifi()
{
  IPAddress local_ip(STATIC_IP_ADDRESS);

  stream.config( local_ip );

  /*
   * Configure WiFi security
   */
  while (wifiStatus != WL_CONNECTED) {
    wifiStatus = stream.begin(WIFI_SSID, WIFI_PASSPHRASE, SERVER_PORT);

    unsigned long delayDone = millis() + 5000;
    while (millis() < delayDone)
    {
      if (initOngoingCallback) initOngoingCallback();
    }
//    delay(5000); // TODO - determine minimum delay
    if (++wifiConnectionAttemptCounter > WIFI_MAX_CONN_ATTEMPTS) break;
  }
}

