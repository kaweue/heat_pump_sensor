#include "sanit_heat_pump_sensor.h"

#include <ArduinoJson.h>

namespace sanit
{

    HeatPumpSensor::HeatPumpSensor(String unit_name) : unit_name_(unit_name)
    {
        json_attributes_topic_ = unit_name_ + "/ATTR";
        lwt_topic_ = unit_name_ + "/LWT";
    }

    const String& HeatPumpSensor::GetLWTTopic() const {
        return lwt_topic_;
    }

    const String& HeatPumpSensor::GetJsonAttributesTopic() const {
        return json_attributes_topic_;
    }

    String HeatPumpSensor::GetRegistrationJson()
    {
        char json_buffer[1024];
        JsonDocument doc;
        doc["name"] = "Heat pump sensor";
        doc["stat_t"] = "~/LWT";
        doc["pl_avail"] = "Online";
        doc["pl_not_avail"] = "Offline";
        doc["uniq_id"] = unit_name_;
        doc["~"] = unit_name_;
        doc["json_attr_t"] = "~/ATTR";
        auto device = doc["device"].to<JsonObject>();
        device["name"] = unit_name_;
        auto identifiers = device["identifiers"].to<JsonArray>();
        identifiers.add(unit_name_);

        String json;
        auto size = serializeJson(doc, json_buffer, sizeof(json_buffer));
        return String(json_buffer, size);
    }

    const String &HeatPumpSensor::GetUniqueId() const
    {
        return unit_name_;
    }

}