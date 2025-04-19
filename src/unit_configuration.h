#include "Preferences.h"

struct UnitConfiguration
{
    UnitConfiguration(Preferences &preferences) : preferences_(preferences) {}
    String name;
    String ssid;
    String password;
    String mqtt_password;
    String mqtt_username;
    void LoadConfiguration()
    {
        name = preferences_.getString("unit_name", String{});
        ssid = preferences_.getString("ssid", String{});
        password = preferences_.getString("wifi_pass", String{});
        mqtt_username = preferences_.getString("mqtt_username", String{});
        mqtt_password = preferences_.getString("mqtt_password", String{});
    }

    bool IsWifiSet()
    {
        return !ssid.isEmpty() && !password.isEmpty();
    }

private:
    Preferences &preferences_;
};