#ifndef SANIT_HOME_ASSISTANT_H_
#define SANIT_HOME_ASSISTANT_H_

#include "PubSubClient.h"
#include "WString.h"

namespace sanit
{
    typedef void (*ButtonHandler)(void);
    class HomeAssistant
    {
    public:
        HomeAssistant(String unit_name, PubSubClient &pub_sub_client) : unit_name_(unit_name), pub_sub_client_(pub_sub_client)
        {
            pub_sub_client_.setCallback([this](char *topic, byte *payload, unsigned int length)
                                        { OnMqttMsg(topic, payload, length); });
        };

        void OnMqttMsg(char *topic, byte *payload, unsigned int length);

        bool RegisterSensorRebootButton(ButtonHandler handler);
        bool RegisterSensorUpdateButton(ButtonHandler handler);

    private:
        ButtonHandler sensor_update_handler_;
        ButtonHandler sensor_reboot_handler_;
        PubSubClient &pub_sub_client_;
        String unit_name_;
    };

}

#endif // SANIT_HOME_ASSISTANT_H_