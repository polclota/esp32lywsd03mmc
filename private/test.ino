enum Model
{
  LYWSD03MMC,
  CGD1
};
enum Types
{
  temperature,
  humidity,
  battery
};
// const std::string MAC[] = {"A4:C1:38:89:F7:CA", "A4:C1:38:D3:A4:5D", "A4:C1:38:85:65:E4", "58:2D:34:51:B7:FB"};
// const String name[] = {"Outdoors(1)", "Mobile(2)", "Downstairs(3)", "Alarm"};
// const Model model[] = {LYWSD03MMC, LYWSD03MMC, LYWSD03MMC, CGD1};
const std::string MAC[] = {"A4:C1:38:89:F7:CA", "A4:C1:38:D3:A4:5D", "A4:C1:38:85:65:E4"};
const String name[] = {"Outdoors", "Mobile", "Downstairs"};
const String number[] = {"1", "2", "3"};
const Model model[] = {LYWSD03MMC, LYWSD03MMC, LYWSD03MMC};
const String modelName[] = {"LYWSD03MMC", "CGD1"};

const Types sensType[] = {temperature, humidity, battery};
const String sensTypeTxt[] = {"Temperature", "Humidity", "Battery"};

auto opts = sizeof(model);
auto types = sizeof(sensType);

#include <mqtt.h>

void setup()
{
  Serial.begin(115200);
  Serial.println("test...");

  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  delay(300);
  digitalWrite(BUILTIN_LED, HIGH);

  defineDevices();
}

void loop()
{
}
