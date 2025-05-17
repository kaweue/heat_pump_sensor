#include "labeldef.h"
#include "sanit_home_assistant.h"


#include "ArduinoJson.h"

namespace
{

    struct SensorConfiguration {
        String device_class;
        String unit_of_measurement;
    };

    SensorConfiguration GetSensorConfiguration(const LabelDef::DataType &data_type)
    {
        switch (data_type)
        {
        case LabelDef::DataType::TEMPERATURE:
            return {"temperature", "°C"};
        case LabelDef::DataType::PRESSURE:
            return {"pressure", "kg/cm2"};
        case LabelDef::DataType::VOLTAGE:
            return {"voltage", "V"};
        case LabelDef::DataType::CURRENT:
            return {"current", "A"};
        case LabelDef::DataType::FLOW:
            return {"volume_flow_rate", "L/min"};
        case LabelDef::DataType::DATA_SIZE:
            return {"data_size", "bit"};
        case LabelDef::DataType::SIGNAL_STRENGTH:
            return {"signal_strength", "dBm"};
        default:
            return {"", ""};
        }
    }

    String GetRestartButtonRegistrationJson(const String &unit_name)
    {
        char json_buffer[1024];
        JsonDocument doc;
        doc["name"] = "Restart button";
        doc["payload_press"] = "RESTART";
        doc["unique_id"] = unit_name + "_restart";
        doc["command_topic"] = "~/COMMANDS",
        doc["~"] = unit_name;
        auto device = doc["device"].to<JsonObject>();
        device["name"] = unit_name;
        auto identifiers = device["identifiers"].to<JsonArray>();
        identifiers.add(unit_name);

        String json;
        auto size = serializeJson(doc, json_buffer, sizeof(json_buffer));
        return String(json_buffer, size);
    }

    struct RegistrationInformation
    {
        String registration_json;
        String uniq_id;
    };

    RegistrationInformation GetSensorRegistrationJson(const String &unit_name, const String &attribute,
                                                      const LabelDef::DataType &data_type)
    {
        auto lower_attribute = attribute;
        lower_attribute.toLowerCase();
        lower_attribute.replace(" ", "_");
        lower_attribute.replace("(", "");
        lower_attribute.replace(")", "");
        lower_attribute.replace(".", "");
        auto uniq_id = unit_name + "_" + lower_attribute;
        char json_buffer[1024];
        JsonDocument doc;
        doc["name"] = attribute;
        doc["stat_t"] = unit_name + "/ATTR";
        doc["availability_topic"] = unit_name + "/LWT";
        doc["pl_avail"] = "Online";
        doc["pl_not_avail"] = "Offline";
        doc["uniq_id"] = uniq_id;
        doc["~"] = uniq_id;

        const auto [device_class, unit_of_measurement] = GetSensorConfiguration(data_type);
        if (!unit_of_measurement.isEmpty()) {
            doc["unit_of_measurement"] = unit_of_measurement;
            doc["state_class"] = "measurement";
        }
        if (!device_class.isEmpty()) {
            doc["device_class"] = device_class;
        }
        doc["value_template"] = "{{ value_json['" + attribute + "'] }}";
        auto device = doc["device"].to<JsonObject>();
        device["name"] = unit_name;
        auto identifiers = device["identifiers"].to<JsonArray>();
        identifiers.add(unit_name);

        String json;
        auto size = serializeJson(doc, json_buffer, sizeof(json_buffer));
        return {
            String(json_buffer, size),
            uniq_id,
        };
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
        const auto restart_button_registration_json = GetRestartButtonRegistrationJson(unit_name_);
        String config_topic = String("homeassistant/button/") + unit_name_ + "_restart" + String("/config");
        pub_sub_client_.publish(config_topic.c_str(), restart_button_registration_json.c_str(), true);

        sensor_reboot_handler_ = handler;
        return true;
    }

    bool HomeAssistant::RegisterSensor(const String &attribute, const LabelDef::DataType &data_type)
    {
        const auto [sensor_registration_json, uniq_id] = GetSensorRegistrationJson(unit_name_, attribute, data_type);
        String config_topic = String("homeassistant/sensor/") + uniq_id + String("/config");
        pub_sub_client_.publish(config_topic.c_str(), sensor_registration_json.c_str(), true);
        return true;
    }

    bool HomeAssistant::RegisterSensorUpdateButton(ButtonHandler handler)
    {
        sensor_update_handler_ = handler;
        return true;
    }
}