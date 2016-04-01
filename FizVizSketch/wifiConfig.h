/*==============================================================================
 * WIFI CONFIGURATION
 *============================================================================*/

#ifndef FizVizWifiConfig_h
#define FizVizWifiConfig_h

// replace this with your wireless network SSID
#define WIFI_SSID "my wifi ssid"

// replace this with your wireless network passphrase
#define WIFI_PASSPHRASE "password"

// replace this with your static IP address
#define STATIC_IP_ADDRESS 192,168,1,209

// define your port number here, you will need this to open a TCP connection to your Arduino
#define SERVER_PORT 3030

/*==============================================================================
 * PIN IGNORE MACROS (don't change anything here)
 *============================================================================*/

// ignore SPI pins, pin 5 (reset WiFi101 shield), pin 7 (WiFi handshake) and pin 10 (WiFi SS)
// also don't ignore SS pin if it's not pin 10
// TODO - need to differentiate between Arduino WiFi1 101 Shield and Arduino MKR1000
#define IS_IGNORE_WIFI101_SHIELD(p)  ((p) == 10 || (IS_PIN_SPI(p) && (p) != SS) || (p) == 5 || (p) == 7)

// ignore SPI pins, pin 4 (SS for SD-Card on WiFi-shield), pin 7 (WiFi handshake) and pin 10 (WiFi SS)
#define IS_IGNORE_WIFI_SHIELD(p)     ((IS_PIN_SPI(p) || (p) == 4) || (p) == 7 || (p) == 10)

#endif
