#ifndef SANIT_HEAT_PUMP_SENSOR_H_
#define SANIT_HEAT_PUMP_SENSOR_H_

#include "WString.h"

namespace sanit
{
    class HeatPumpSensor
    {
    public:
        HeatPumpSensor(String unit_name);
        String GetRegistrationJson();
        const String& GetUniqueId() const;
        const String& GetJsonAttributesTopic() const;
        const String& GetLWTTopic() const;

    private:
        String unit_name_;
        String json_attributes_topic_;
        String lwt_topic_;
    };
}

#endif // SANIT_HEAT_PUMP_SENSOR_H_