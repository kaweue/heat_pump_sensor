#include "restart.h"
#include <PubSubClient.h>

#include "sanit_heat_pump_sensor.h"

#define EEPROM_CHK 1
#define EEPROM_STATE 0

char jsonbuff[MAX_MSG_SIZE] = "{\0";

#include <WiFiClientSecure.h>
WiFiClientSecure espClient;

PubSubClient client(espClient);

sanit::HeatPumpSensor *heat_pump_sensor;

const char *url = "https://update.sanitapp.xyz/firmware.bin";

void sendValues()
{
  Serial.printf("Sending values in MQTT.\n");
  // Add Power values
  //  getBatteryVoltage returns battery voltage [mV] as an int16_t
  float batteryVoltage = (float)M5.Power.getBatteryVoltage() / 1000; // convert to V as a float
  snprintf(jsonbuff + strlen(jsonbuff), MAX_MSG_SIZE - strlen(jsonbuff), "\"%s\":\"%.3g\",", "M5BatV", batteryVoltage);

  snprintf(jsonbuff + strlen(jsonbuff), MAX_MSG_SIZE - strlen(jsonbuff), "\"%s\":\"%ddBm\",", "WifiRSSI", WiFi.RSSI());
  snprintf(jsonbuff + strlen(jsonbuff), MAX_MSG_SIZE - strlen(jsonbuff), "\"%s\":\"%d\",", "FreeMem", ESP.getFreeHeap());
  jsonbuff[strlen(jsonbuff) - 1] = '}';

  client.publish(heat_pump_sensor->GetJsonAttributesTopic().c_str(), jsonbuff);
  strcpy(jsonbuff, "{\0");
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

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.printf("Message arrived [%s] : %s\n", topic, payload);
}
