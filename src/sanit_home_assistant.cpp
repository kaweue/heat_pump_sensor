#include "sanit_home_assistant.h"

#include "ArduinoJson.h"

namespace
{
    String GetRestartButtonRegistrationJson()
    {
        char json_buffer[1024];
        JsonDocument doc;
        doc["name"] = "Restart Sanit Test Sensor 1";
        doc["payload_press"] = "RESTART";
        doc["unique_id"] = "sanit_test_sensor_1_restart";
        doc["command_topic"] = "~/COMMANDS",
        doc["~"] = "sanit_test_sensor_1";
        auto device = doc["device"].to<JsonObject>();
        auto identifiers = device["identifiers"].to<JsonArray>();
        identifiers.add("sanit_test_sensor_1");

        String json;
        auto size = serializeJson(doc, json_buffer, sizeof(json_buffer));
        return String(json_buffer, size);
    }
}

namespace sanit
{

    void HomeAssistant::OnMqttMsg(char *topic, byte *payload, unsigned int length)
    {
        if (sensor_reboot_handler_ && !strncmp("RESTART", reinterpret_cast<char *>(payload), length))
        {
            sensor_reboot_handler_();
        }
    }

    bool HomeAssistant::RegisterSensorRebootButton(ButtonHandler handler)
    {
        pub_sub_client_.subscribe("sanit_test_sensor_1/COMMANDS");
        const auto restart_Button_registration_json = GetRestartButtonRegistrationJson();
        String config_topic = String("homeassistant/button/") + "sanit_test_sensor_1_restart" + String("/config");
        pub_sub_client_.publish(config_topic.c_str(), restart_Button_registration_json.c_str(), true);

        sensor_reboot_handler_ = handler;
        return true;
    }

    bool HomeAssistant::RegisterSensorUpdateButton(ButtonHandler handler)
    {
        sensor_update_handler_ = handler;
        return true;
    }
}