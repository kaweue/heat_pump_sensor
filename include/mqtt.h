#include "restart.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "sanit_heat_pump_sensor.h"

#include <WiFiClientSecure.h>

JsonDocument heat_pump_attribues;
WiFiClientSecure espClient;

PubSubClient client(espClient);

sanit::HeatPumpSensor *heat_pump_sensor;

char json_buffer[MAX_MSG_SIZE];

void sendValues()
{
  Serial.printf("Sending values in MQTT.\n");
  // Add Power values
  //  getBatteryVoltage returns battery voltage [mV] as an int16_t
  float batteryVoltage = (float)M5.Power.getBatteryVoltage() / 1000; // convert to V as a float

  heat_pump_attribues["FreeMem"] = ESP.getFreeHeap();
  heat_pump_attribues["M5BatV"] = batteryVoltage;
  heat_pump_attribues["WifiRSSI"] = WiFi.RSSI();

  auto size = serializeJson(heat_pump_attribues, json_buffer, sizeof(json_buffer));
  json_buffer[size] = '\0'; // null terminate the string
  client.publish(heat_pump_sensor->GetJsonAttributesTopic().c_str(), json_buffer);

  heat_pump_attribues.clear();
}

void reconnectMqtt()
{
  Preferences preferences;
  preferences.begin("sanit_app", true);
  UnitConfiguration uc(preferences);
  uc.LoadConfiguration();
  preferences.end();
  // Loop until we're reconnected
  int i = 0;
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    const auto unique_id = heat_pump_sensor->GetUniqueId();
    if (client.connect(unique_id.c_str(), uc.mqtt_username.c_str(), uc.mqtt_password.c_str(), heat_pump_sensor->GetLWTTopic().c_str(), 0, true, "Offline"))
    {
      Serial.println("connected!");
      auto registration_json = heat_pump_sensor->GetRegistrationJson();
      String config_topic = String("homeassistant/sensor/") + unique_id + String("/config");
      client.publish(config_topic.c_str(), registration_json.c_str(), true);
      client.publish(heat_pump_sensor->GetLWTTopic().c_str(), "Online", true);
    }
    else
    {
      Serial.printf("failed, rc=%d, try again in 5 seconds", client.state());
      delay(5000);
      if (i++ == 100)
      {
        Serial.printf("Tried for 500 sec, rebooting now.");
        ESP.restart();
      }
    }
  }
}
