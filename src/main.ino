// inspired by:
// https://github.com/karolkalinski/esp32-snippets/tree/master/Mijia-LYWSD03MMC-Client

#include <BLEDevice.h>
#include <SimpleTimer.h>
#include <WiFi.h>
#include <PubSubClient.h>

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int opt = 0;
RTC_DATA_ATTR unsigned long ontime = 0;

WiFiClient espClient;
PubSubClient client(espClient);

enum Model
{
  LYWSD03MMC,
  CGD1
};
enum Types
{
  temperature,
  humidity,
  battery,
  voltage
};
enum Status
{
  on,
  off
};

const String status[] = {"on", "off"};

#include <userconfig.h>
// You should not change this
const Types sensType[] = {temperature, humidity, battery, voltage};
const String sensTypeTxt[] = {"Temperature", "Humidity", "Battery", "Voltage"};
const String unitMesurement[] = {"°C", "%", "%", "v"};

auto opts = sizeof(model) / sizeof(model[0]);
auto types = sizeof(sensType) / sizeof(sensType[0]);

String devName()
{
  char ssid[23];
  uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(ssid, 23, "esp32BleHub_%04X%08X", chip, (uint32_t)chipid);
  return String(ssid);
}

bool done = false;
bool do_updateSensors = false;
bool notified = false;

#include <mqtt.h>

BLEClient *pClient;

const std::string service[] = {"ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6", "0000fe95-0000-1000-8000-00805f9b34fb"};
const std::string charact[] = {"ebe0ccc1-7a0a-4b0c-8a1a-6ff2997da3a6", "00000017-0000-1000-8000-00805f9b34fb"};

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pclient)
  {
    Serial.println("Connected!");
  }

  void onDisconnect(BLEClient *pclient)
  {
    do_updateSensors = false;
    done = true;
  }
};

void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{

  Serial.print("Notify callback from: " + name[opt] + " (" + number[opt] + "), characteristic ");
  Serial.println(pBLERemoteCharacteristic->getUUID().toString().c_str());

  String result;
  if (length == 5)
  {
    float states[types];
    float volt = ((pData[4] * 256) + pData[3]) / 1000.0;
    for (size_t s = 0; s < types; s++)
    {
      switch (s)
      {
      case temperature:
        /* code */
        states[temperature] = (pData[0] | (pData[1] << 8)) * 0.01; //little endian
        break;
      case humidity:
        states[humidity] = pData[2];
        break;
      case voltage:
        states[voltage] = volt;
        break;
      case battery:
        states[battery] = (volt - 2.1) * 100.0;
        break;
      }

      String topic = mqtt_topic_base + "/sensor/" + name[opt] + "_" + sensTypeTxt[s] + "/state";
      topic.toLowerCase();
      Serial.print(topic + ": " + String(states[s], 2));
      if (client.publish(topic.c_str(), String(states[s], 2).c_str()))
      {
        result = "published!";
        Serial.println(", " + result);
        for (size_t i = 0; i < 3; i++)
        {
          digitalWrite(BUILTIN_LED, LOW);
          delay(300);
          digitalWrite(BUILTIN_LED, HIGH);
        }
      }
      else
      {
        result = "ERROR!";
        Serial.println(". " + result);
      }
    }
    do_updateSensors = false;
    done = true;
  }
  else
  {
    for (size_t i = 0; i < length; i++)
      Serial.printf("byte %d:%d, ", i, pData[i]);
    Serial.println();
    result = "Unknown data length: ";
    result += String(length);
    result += "Provably is not a LYWSD03MMC device!";
  }
  // Serial.println(result);
  publishStatus(off, result);
  notified = true;
  do_updateSensors = false;
  done = true;
  pClient->disconnect();
}

void registerNotification(std::string serviceUUID_str, std::string charUUID_str)
{

  BLEUUID serviceUUID(serviceUUID_str);
  BLEUUID charUUID(charUUID_str);

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("-Failed to find our service UUID: ");
    Serial.println(serviceUUID_str.c_str());
    pClient->disconnect();
    return;
  }
  else
    Serial.printf("-Found our service: %s\n", serviceUUID_str.c_str());

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  if (pClient->isConnected())
  {
    BLERemoteCharacteristic *pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr)
    {
      Serial.print("-Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
    }
    else
    {
      Serial.printf("-Found our characteristic: %s\n", charUUID_str.c_str());
      Serial.println("Waiting for notifications!");
      pRemoteCharacteristic->registerForNotify(notifyCallback);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting BLE client: " + devName());
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  Serial.println("Loop option: " + String(opt));

  esp_sleep_enable_timer_wakeup(updatetime * 1000000);

  Serial.print("Connecting to ");

  Serial.print(ssid);

  WiFi.begin(ssid, password);

  pinMode(BUILTIN_LED, OUTPUT);
  uint8_t c = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    // if wifi takes too long to connect then reset
    if (c * 300 < wifi_watchdog)
      c++;
    else
      ESP.restart();
    digitalWrite(BUILTIN_LED, LOW);
    delay(300);
    digitalWrite(BUILTIN_LED, HIGH);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);
  setup_time();
}

void createBleClientWithCallbacks()
{
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());
}

bool connectSensor(std::string mac)
{
  BLEAddress htSensorAddress(mac);
  return pClient->connect(htSensorAddress);
}

void updateSensors()
{
  do_updateSensors = false;
  Serial.println("Updating BLE device: " + name[opt] + " (" + number[opt] + "), " + MAC[opt].c_str());
  BLEDevice::init(devName().c_str());
  createBleClientWithCallbacks();
  delay(500);
  if (connectSensor(MAC[opt]))
    registerNotification(service[model[opt]], charact[model[opt]]);
}

void updateSensors_t()
{
  do_updateSensors = true;
}

void loop()
{
  digitalWrite(BUILTIN_LED, LOW);
  delay(300);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(300);
  if (done)
  {
    BLEDevice::deinit();
    String result = "Disconnected";
    if (notified)
      result += "!";
    else
    {
      result += ", without result!";
      publishStatus(off, result);
    }
    Serial.println(result);
    if (opt < opts - 1)
      opt++;
    else
      opt = 0;
    Serial.println("Going into deepsleep for: " + String(updatetime) + "secs.");
    Serial.flush();
    ontime += millis() / 1000;
    delay(1000);
    esp_deep_sleep_start();
  }
  if (!client.connected())
  {
    reconnect();
  }
  if (do_updateSensors)
  {
    do_updateSensors = false;
    updateSensors();
  }
  client.loop();
}