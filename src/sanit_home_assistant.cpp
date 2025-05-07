#include "sanit_home_assistant.h"

#include "ArduinoJson.h"

namespace
{
    String GetRestartButtonRegistrationJson(const String &unit_name)
    {
        char json_buffer[1024];
        JsonDocument doc;
        doc["name"] = "Restart " + unit_name + " button";
        doc["payload_press"] = "RESTART";
        doc["unique_id"] = unit_name + "_restart";
        doc["command_topic"] = "~/COMMANDS",
        doc["~"] = unit_name;
        auto device = doc["device"].to<JsonObject>();
        auto identifiers = device["identifiers"].to<JsonArray>();
        identifiers.add(unit_name);

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
        pub_sub_client_.subscribe((unit_name_ + "/COMMANDS").c_str());
        const auto restart_Button_registration_json = GetRestartButtonRegistrationJson(unit_name_);
        String config_topic = String("homeassistant/button/") + unit_name_ + "_restart" + String("/config");
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