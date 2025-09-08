#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Include the DHT11 library for interfacing with the sensor.
#include <DHT11.h>

// Create an instance of the DHT11 class.
// - For Arduino: Connect the sensor to Digital I/O Pin 2.
// - For ESP32: Connect the sensor to pin GPIO2 or P2.
// - For ESP8266: Connect the sensor to GPIO2 or D4.
DHT11 dht11(2);

// WiFi settings
const char *ssid = "";       // Replace with your WiFi name
const char *password = "";   // Replace with your WiFi password

#define AIO_USERNAME    ""
#define AIO_KEY         ""

// MQTT Broker settings
const char *mqtt_broker = "io.adafruit.com"; // Broker Host
const char *mqtt_topic_temp = "";            // MQTT temperatur topic
const char *mqtt_topic_hum = "";             // MQTT humidity topic
const int mqtt_port = 1883;                  // MQTT port (TCP)

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void connectToWiFi();

void connectToMQTTBroker();

void mqttCallback(char *topic, byte *payload, unsigned int length);

void setup() {
    Serial.begin(115200);
    connectToWiFi();
    mqtt_client.setServer(mqtt_broker, mqtt_port);
}

void loop() {
    if (!mqtt_client.connected()) {
        connectToMQTTBroker();
    }

    int temperature = 18;
    int humidity = 72;
    char buff[100];

    // Attempt to read the temperature and humidity values from the DHT11 sensor.
    int result = dht11.readTemperatureHumidity(temperature, humidity);

    if(result == 0){
        sprintf(buff, "%d", temperature);

        // Publish temperature.
        mqtt_client.publish(mqtt_topic_temp, buff);

        sprintf(buff, "%d", humidity);

        // Publish huimdity.
        mqtt_client.publish(mqtt_topic_hum, buff);

        sprintf(buff, "Temperatur: %d °C, Humididy:%d %%", temperature, humidity);
        Serial.print(buff);
        Serial.print("\n");

    } else{
        // Print error message based on the error code.
        Serial.println(DHT11::getErrorString(result));
    }

    delay(1000);
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to the WiFi network");
}

void connectToMQTTBroker() {
    while (!mqtt_client.connected()) {
        String client_id = "esp8266-client-" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
        if (mqtt_client.connect(client_id.c_str(), AIO_USERNAME, AIO_KEY)) {
            Serial.println("Connected to MQTT broker");
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}
