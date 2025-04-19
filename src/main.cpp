#include <M5StickCPlus2.h>

#include <HardwareSerial.h>
#include <WiFi.h>

#include <PubSubClient.h>

#include "settings.h"
#include "setup.h"

#include "HttpsOTAUpdate.h"
#include "Preferences.h"
#include "certs.h"
#include "unit_configuration.h"

#include "converters.h"
#include "mqtt.h"
#include "mqttserial.h"
#include "restart.h"

#include "comm.h"

Converter converter;
char registryIDs[32]; // Holds the registries to query
bool busy = false;

UnitConfiguration *uc;

void showStatus(String status);

bool contains(char array[], int size, int value)
{
  for (int i = 0; i < size; i++)
  {
    if (array[i] == value)
      return true;
  }
  return false;
}

// Converts to string and add the value to the JSON message
void updateValues(char regID)
{
  LabelDef *labels[128];
  int num = 0;
  converter.getLabels(regID, labels, num);
  for (int i = 0; i < num; i++)
  {
    bool alpha = false;
    for (size_t j = 0; j < strlen(labels[i]->asString); j++)
    {
      char c = labels[i]->asString[j];
      if (!isdigit(c) && c != '.' && !(c == '-' && j == 0))
      {
        alpha = true;
        break;
      }
    }

    if (alpha)
    {

      snprintf(jsonbuff + strlen(jsonbuff), MAX_MSG_SIZE - strlen(jsonbuff), "\"%s\":\"%s\",", labels[i]->label, labels[i]->asString);
    }
    else
    { // number, no quotes
      snprintf(jsonbuff + strlen(jsonbuff), MAX_MSG_SIZE - strlen(jsonbuff), "\"%s\":%s,", labels[i]->label, labels[i]->asString);
    }
  }
}

uint16_t loopcount = 0;
boolean display_sleeping = false;

void extraLoop()
{
  client.loop();
  if (M5.BtnC.wasPressed())
  {
    mqttSerial.printf("M5.BtnC.wasPressed()");
    showStatus("Restarting...");
    M5.update();
    delay(3000);
    ESP.restart();
  }
  M5.update();
}

void checkWifi()
{
  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (i++ == 240)
    {
      Serial.printf("Tried connecting for 120 sec, rebooting now.");
      ESP.restart();
    }
  }
}

void loadConfiguration() {
  Preferences preferences;
  preferences.begin("sanit_app", false);

  if (sizeof(WIFI_SSID) > 1) {
    preferences.putString("ssid", WIFI_SSID);
  }

  if (sizeof(WIFI_PWD) > 1) {
    preferences.putString("wifi_pass", WIFI_PWD);
  }

  uc = new UnitConfiguration(preferences);
  uc->LoadConfiguration();
  preferences.end();
  heat_pump_sensor = new sanit::HeatPumpSensor(uc->name);
}

void setup_wifi()
{
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
  WiFi.begin(uc->ssid, uc->password, 0, 0, true);

  checkWifi();
}

void otaHttpEvent(HttpEvent_t *event)
{
  switch (event->event_id)
  {
  case HTTP_EVENT_ERROR:
    mqttSerial.println("OTA Http Event Error");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    mqttSerial.println("OTA Http Event On Connected");
    break;
  case HTTP_EVENT_HEADER_SENT:
    mqttSerial.println("OTA Http Event Header Sent");
    break;
  case HTTP_EVENT_ON_HEADER:
    mqttSerial.printf("OTA Http Event On Header, key=%s, value=%s\n", event->header_key, event->header_value);
    break;
  case HTTP_EVENT_ON_DATA:
    break;
  case HTTP_EVENT_ON_FINISH:
    mqttSerial.println("OTA Http Event On Finish");
    break;
  case HTTP_EVENT_DISCONNECTED:
    mqttSerial.println("OTA Http Event Disconnected");
    break;
  }
}

void checkOTAStatus()
{
  HttpsOTAStatus_t otastatus = HttpsOTA.status();
  if (otastatus == HTTPS_OTA_SUCCESS)
  {
    showStatus("Updated");
    delay(5000);
    ESP.restart();
  }
  else if (otastatus == HTTPS_OTA_FAIL)
  {
    showStatus("Monitoring");
  }
  else if (otastatus == HTTPS_OTA_UPDATING)
  {
    showStatus("Checking update");
    mqttSerial.println("Updating...");
  }
  else if (otastatus == HTTPS_OTA_IDLE)
  {
    mqttSerial.println("Idle...");
  }
}

void setupOTA()
{
  HttpsOTA.onHttpEvent(otaHttpEvent);
  static String ota_url = "https://update.sanitapp.xyz/" + heat_pump_sensor->GetUniqueId() + "/" SANIT_HEAT_PUMP_SENSOR_FIRMWARE_VERSION "/firmware.bin";
  mqttSerial.printf("Ota url %s\n.", ota_url.c_str());
  HttpsOTA.begin(ota_url.c_str(), ota_server_certificate, true);
}

void initRegistries()
{
  // getting the list of registries to query from the selected values
  for (size_t i = 0; i < sizeof(registryIDs); i++)
  {
    registryIDs[i] = 0xff;
  }

  int i = 0;
  for (auto &&label : labelDefs)
  {
    if (!contains(registryIDs, sizeof(registryIDs), label.registryID))
    {
      mqttSerial.printf("Adding registry 0x%2x to be queried.\n", label.registryID);
      registryIDs[i++] = label.registryID;
    }
  }
  if (i == 0)
  {
    mqttSerial.printf("ERROR - No values selected in the include file. Stopping.\n");
    while (true)
    {
      extraLoop();
    }
  }
}

void setupScreen()
{
  M5.Lcd.setRotation(1);
  M5.Lcd.setBrightness(60);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setFont(&FreeSans9pt7b);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.setTextColor(TFT_GREEN);
}

void showStatus(String status) {
  M5.Lcd.fillScreen(TFT_BLACK);
  int xpos = M5.Lcd.width() / 2;  // Half the screen width
  int ypos = M5.Lcd.height() / 2; // Half the screen width
  heat_pump_sensor->GetUniqueId();
  M5.Lcd.drawString(heat_pump_sensor->GetUniqueId().c_str(), xpos, 10);
  M5.Lcd.drawString(SANIT_HEAT_PUMP_SENSOR_FIRMWARE_VERSION, xpos, 25);
  M5.Lcd.drawString(status.c_str(), xpos, ypos);
}

void setup()
{
  Serial.begin(115200);
  M5.begin();
  loadConfiguration();
  setupScreen();
  showStatus("Sanit Sensor");
  MySerial.begin(9600, SERIAL_CONFIG, RX_PIN, TX_PIN);

  mqttSerial.print("Setting up wifi...");
  showStatus("Wifi connecting...");
  setup_wifi();

  // Required to establish encrypted connections.
  // If you want to be more secure here, you can use the CA certificate to allow the wifi client to verify the other party. NOTE: If you use the CA certificate here, then you need to make sure to update it here regulary!
  espClient.setInsecure();
  espClient.setTimeout(5);

  client.setBufferSize(MAX_MSG_SIZE); // to support large json message
  client.setCallback(callback);
  client.setServer(MQTT_SERVER, MQTT_PORT);

  auto timeout = espClient.getTimeout();
  Serial.printf("Wifi client timeout: %d\n", timeout);

  String log_topic = "sanit_heat_pump_sensor/" + heat_pump_sensor->GetUniqueId() + "/log";
  mqttSerial.begin(&client, log_topic.c_str());
  reconnectMqtt();
  initRegistries();
  showStatus("Monitoring");
  setupOTA();
}

void waitLoop(uint ms)
{
  unsigned long start = millis();
  while (millis() < start + ms) // wait .5sec between registries
  {
    extraLoop();
  }
}

void loop()
{
  unsigned long start = millis();
  if (WiFi.status() != WL_CONNECTED)
  { // restart board if needed
    checkWifi();
  }

  checkOTAStatus();

  if (!client.connected())
  { //(re)connect to MQTT if needed
    reconnectMqtt();
  }
  // Querying all registries
  for (size_t i = 0; (i < 32) && registryIDs[i] != 0xFF; i++)
  {
    unsigned char buff[64] = {0};
    int tries = 0;
    while (!queryRegistry(registryIDs[i], buff, PROTOCOL) && tries++ < 3)
    {
      waitLoop(1000);
    }
    unsigned char receivedRegistryID = PROTOCOL == 'S' ? buff[0] : buff[1];
    if (registryIDs[i] == receivedRegistryID) // if replied registerID is coherent with the command
    {
      converter.readRegistryValues(buff, PROTOCOL); // process all values from the register
      updateValues(registryIDs[i]);                 // send them in mqtt
      // waitLoop(500);//wait .5sec between registries
    }
  }
  sendValues(); // Send the full json message
  mqttSerial.printf("Done. Waiting %ld ms...", FREQUENCY - millis() + start);
  waitLoop(FREQUENCY - millis() + start);
}
