#ifndef DAIKIN_EXAMPLE_RESPONSES_H
#define DAIKIN_EXAMPLE_RESPONSES_H

#include <math.h>

namespace sanit
{

class DaikinExampleResponses {
public:
    static const int MAX_BUFFER_SIZE = 16;
    
    DaikinExampleResponses() : 
        current_value(75),         // 7.5A
        voltage_value(230),        // 230V
        leaving_water_temp(420),   // 42.0°C
        inlet_water_temp(380),     // 38.0°C
        dhw_tank_temp(480),        // 48.0°C
        flow_sensor(155),          // 15.5 l/min
        time_counter(0) {}
    
    int GetExampleResponse(unsigned char registerId, unsigned char* buffer) {
        UpdateSimulatedValues();
        
        switch(registerId) {
            case 0x10:
                return GetOperationModeResponse(buffer);
            case 0x21:
                return GetCurrentAndVoltageResponse(buffer);
            case 0x60:
                return GetFreezeProtectionResponse(buffer);
            case 0x61:
                return GetTemperaturesResponse(buffer);
            case 0x62:
                return GetFlowSensorResponse(buffer);
            default:
                return 0;
        }
    }

private:
    // Simulated values
    int current_value;      // Current in decisamps (75 = 7.5A)
    int voltage_value;      // Voltage in volts
    int leaving_water_temp; // Temperature in decisamps (420 = 42.0°C)
    int inlet_water_temp;   // Temperature in decisamps
    int dhw_tank_temp;      // Temperature in decisamps
    int flow_sensor;        // Flow in decisamps (155 = 15.5 l/min)
    int time_counter;       // For creating periodic changes

    void UpdateSimulatedValues() {
        time_counter++;
        
        // Simulate current variations (7.0A to 8.0A)
        current_value = 75 + (sin(time_counter * 0.1) * 5);
        
        // Simulate voltage variations (225V to 235V)
        voltage_value = 230 + (sin(time_counter * 0.05) * 5);

        // Simulate leaving water temperature (41°C to 43°C)
        leaving_water_temp = 420 + (sin(time_counter * 0.03) * 10);

        // Simulate inlet water temperature (37°C to 39°C)
        inlet_water_temp = 380 + (sin(time_counter * 0.03) * 10);

        // Simulate DHW tank temperature (47°C to 49°C)
        dhw_tank_temp = 480 + (sin(time_counter * 0.02) * 10);

        // Simulate flow sensor (15.0 to 16.0 l/min)
        flow_sensor = 155 + (sin(time_counter * 0.08) * 5);
    }

    unsigned char CalculateCRC(const unsigned char* data, int len) {
        unsigned char sum = 0;
        for (int i = 0; i < len; i++) {
            sum += data[i];
        }
        return ~sum;
    }

    int GetOperationModeResponse(unsigned char* buff) {
        buff[0] = 0x03;  // Header
        buff[1] = 0x10;  // Register
        buff[2] = 0x0A;  // Length
        buff[3] = 0x01;  // Value (Heating)
        buff[4] = CalculateCRC(buff, 4);
        return 5;
    }

    int GetCurrentAndVoltageResponse(unsigned char* buff) {
        buff[0] = 0x03;  // Header
        buff[1] = 0x21;  // Register
        buff[2] = 0x0A;  // Length
        buff[3] = current_value & 0xFF;       // Current LSB
        buff[4] = (current_value >> 8) & 0xFF; // Current MSB
        buff[5] = 0x00;  // Padding
        buff[6] = 0x00;  // Padding
        buff[7] = voltage_value & 0xFF;       // Voltage LSB
        buff[8] = (voltage_value >> 8) & 0xFF; // Voltage MSB
        buff[9] = CalculateCRC(buff, 9);
        return 10;
    }

    int GetFreezeProtectionResponse(unsigned char* buff) {
        buff[0] = 0x03;  // Header
        buff[1] = 0x60;  // Register
        buff[2] = 0x0A;  // Length
        buff[3] = 0x00;  // Padding
        buff[4] = 0x00;  // Padding
        buff[5] = 0x04;  // Value (ON)
        buff[6] = CalculateCRC(buff, 6);
        return 7;
    }

    int GetTemperaturesResponse(unsigned char* buff) {
        buff[0] = 0x03;   // Header
        buff[1] = 0x61;   // Register
        buff[2] = 0x0A;   // Length
        buff[3] = 0x00;   // Padding
        buff[4] = 0x00;   // Padding
        
        // Leaving water temp (offset 2)
        buff[5] = leaving_water_temp & 0xFF;        // LSB
        buff[6] = (leaving_water_temp >> 8) & 0xFF; // MSB
        
        // Padding for offset 8 (Inlet water temp)
        buff[7] = 0x00;
        buff[8] = 0x00;
        
        // Inlet water temp (offset 8)
        buff[9] = inlet_water_temp & 0xFF;        // LSB
        buff[10] = (inlet_water_temp >> 8) & 0xFF; // MSB
        
        // DHW tank temp (offset 10)
        buff[11] = dhw_tank_temp & 0xFF;        // LSB
        buff[12] = (dhw_tank_temp >> 8) & 0xFF; // MSB
        
        buff[13] = CalculateCRC(buff, 13);
        return 14;
    }

    int GetFlowSensorResponse(unsigned char* buff) {
        buff[0] = 0x03;   // Header
        buff[1] = 0x62;   // Register
        buff[2] = 0x0A;   // Length
        
        // Padding for offset 9
        for(int i = 3; i < 12; i++) {
            buff[i] = 0x00;
        }
        
        // Flow sensor value (offset 9)
        buff[12] = flow_sensor & 0xFF;        // LSB
        buff[13] = (flow_sensor >> 8) & 0xFF; // MSB
        
        buff[14] = CalculateCRC(buff, 14);
        return 15;
    }
};
}

#endif // DAIKIN_EXAMPLE_RESPONSES_H
