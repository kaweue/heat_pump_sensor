#define SANIT_HEAT_PUMP_SENSOR_FIRMWARE_VERSION "0.0.1"

#define MQTT_SERVER "sanitapp.xyz"
#define MQTT_PORT 8883
#define MQTT_ENCRYPTED

#define FREQUENCY 30000 //query values every 30 sec

#define RX_PIN    36// Pin connected to the TX pin of X10A 
#define TX_PIN    26// Pin connected to the RX pin of X10A

#define MAX_MSG_SIZE 7120//max size of the json message sent in mqtt 
#define PROTOCOL 'I'
