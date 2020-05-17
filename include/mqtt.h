#include <arduinoJson.h>
#include <mytime.h>

const String mqtt_topic_base = devName();

void mqtt_callBack(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String payload_str;
  for (int i = 0; i < length; i++) {
    payload_str += (char)payload[i];
  } 
  Serial.println(payload_str);

  if ((char)payload[0] == '1')
  {
    digitalWrite(BUILTIN_LED, LOW);
  }
  else
  {
    digitalWrite(BUILTIN_LED, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

void publishStatus(Status s, String result)
{
  String topic = mqtt_topic_base + "/status/" + status[s];
  topic.toLowerCase();
  Serial.print("Status topic: " + topic);
  String payload;

  // #define min 500
  const size_t capacity = JSON_OBJECT_SIZE(12) + 230;
  // if (capacity < min)
  //   capacity = min;
  DynamicJsonDocument doc(capacity);

  doc["Timestamp"] = convertDateTime2ISO(myRealTime());
  doc["MAC"] = WiFi.macAddress();
  doc["Bootcount"] = bootCount;
  doc["Option"] = opt;
  doc["On_secs"] = ontime + millis() / 1000;
  if (s == on)
  {
    doc["Updatetime"] = updatetime;
    doc["SensorName"] = name[opt];
    doc["SensorAddress"] = MAC[opt];
    doc["SSID"] = WiFi.SSID();
    doc["BSSID"] = WiFi.BSSIDstr();
    doc["IP"] = WiFi.localIP().toString();
    doc["RSSI"] = WiFi.RSSI();
  }
  else
  {
    doc["Result"] = result;
  }

  Serial.println();
  serializeJson(doc, payload);
  doc.clear();
  if (!client.connected())
  {
    String clientId = mqtt_topic_base + "_" + String(random(0xFFFF), HEX);
    if (!client.connect(clientId.c_str(), mqtt_user, mqtt_pw))
    {
      Serial.println(". Unable to reconnect to MQTT server!");
      return;
    }
  }

  Serial.print(payload);
  if (client.publish(topic.c_str(), payload.c_str(), true))
  {
    if (s == off)
      client.disconnect();
    Serial.println(", done.");
  }
  else
    Serial.println(". Error publishing!");
}

void defineDevices()
{
  const size_t capacity = JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6)+500;
  DynamicJsonDocument doc(capacity);
  for (size_t i = 0; i < opts; i++)
    for (size_t s = 0; s < types; s++)
    {
      String nm = name[i] + " via " + mqtt_topic_base;
      doc["name"] = sensTypeTxt[s] + " " + nm;

      JsonObject device = doc.createNestedObject("device");
      String n = nm;
      n.replace(" ", "_");
      n.toLowerCase();
      device["name"] = nm;
      device["identifiers"] = n;
      device["sw_version"] = "1.0.0_0106";
      device["manufacturer"] = "Xiaomi";
      device["model"] = modelName[model[i]];

      String u = mqtt_topic_base + "/sensor/" + name[i] + "_" + sensTypeTxt[s] + "/state";
      u.toLowerCase();
      doc["state_topic"] = u;
      u = name[i] + " " + sensTypeTxt[s] + " " + mqtt_topic_base;
      u.replace(" ", "_");
      u.toLowerCase();
      doc["unique_id"] = u;
      if (sensType[s] != voltage)
        doc["device_class"] = sensTypeTxt[s];
      doc["unit_of_measurement"] = unitMesurement[s];

      String topic = "homeassistant/sensor/";
      topic += u;
      topic += "/config";
      Serial.print("\r");
      String payload;
      serializeJson(doc, payload);
      Serial.printf("i: %d s: %d", i, s);
      // Serial.println(payload);
      if (client.publish(topic.c_str(), payload.c_str(), true))
        Serial.print(", ");
      else
        Serial.println(", Error!");
      doc.clear();
    }
  Serial.println("done.");
}

void reconnect()
{
  // Loop until we're reconnected
  digitalWrite(BUILTIN_LED, LOW);
  while (!client.connected())
  {
    // Attempt to connect
    String clientId = mqtt_topic_base + "_" + String(random(0xFFFF), HEX);
    Serial.print("Attempting MQTT connection as: " + clientId);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pw))
    {
      Serial.println(", connected!");
      do_updateSensors = true;

      if (bootCount == 1)
        defineDevices();

      publishStatus(on, "");

      String topic = mqtt_topic_base + "/cmd";
      if (client.subscribe(topic.c_str()))
      {
        client.setCallback(mqtt_callBack);
        Serial.println("Subscribed to: " + topic);
      }

      digitalWrite(BUILTIN_LED, HIGH);
    }
    else
    {
      digitalWrite(BUILTIN_LED, HIGH);
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(2500);
      digitalWrite(BUILTIN_LED, LOW);
      delay(2500);
      // Wait 5 seconds before retrying
    }
  }
}
