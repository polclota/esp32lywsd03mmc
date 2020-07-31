#include <myprivatedate.h> // might be deleted once your data is provided

// next data must be subsituted by yours
const char *ssid = WIFINAME;
const char *password = PASSWORD;
const char *mqtt_server = MQTT_SERVER;
const char *mqtt_user = MQTT_USER;
const char *mqtt_pw = MQTT_PW;

uint8_t updatetime = 180;   // secs
uint8_t wifi_watchdog = 60; // secs

// planned for future use of other sensors
const String modelName[] = {"LYWSD03MMC", "CGD1"};

const Model model[] = {LYWSD03MMC, LYWSD03MMC, LYWSD03MMC};
// add your MAC addresses 
const std::string MAC[] = {"A4:C1:38:D3:A4:5D", "A4:C1:38:89:F7:CA", "A4:C1:38:85:65:E4"};
// give them a name
const String name[] = {"Mobile", "Outdoors", "Downstairs"};
// give them a number/id in case you need additional coding, I myself wrote this numbers to the back of the sensor
const String number[] = {"1", "2", "3"};
