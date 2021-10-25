#include "Secret.h"

//Include the required libraries
#include <ESP8266WiFi.h>
#include <ArduinoHA.h>
#include "DHT.h"

//Define DHT sensor pin and type
#define DHTPIN 13
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//Define variables
unsigned long lastTemperatureSend = millis();
bool lastInputState = false;

float temperatureValue;
float humidityValue;
float signalstrengthValue;

//Initialize WiFi
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);

//Define the sensors and/or devices
//The string must not contain any spaces!!! Otherwise the sensor will not show up in Home Assistant
HASensor sensorOwner("Owner");
HASensor sensorLong("Long");
HASensor sensorLat("Lat");
HASensor sensorTemperature("Temperature");
HASensor sensorHumidity("Humidity");
HASensor sensorSignalstrength("Signal_strength");

void setup() {
    Serial.begin(9600);
    Serial.println("Starting...");

    // Unique ID must be set!
    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));

    // Connect to wifi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500); // waiting for the connection
    }
    Serial.println();
    Serial.println("Connected to the network");

    // Set sensor and/or device names
    // String conversion for incoming data from Secret.h
    String student_id = STUDENT_ID;
    String student_name = STUDENT_NAME;

    //Add student ID number with sensor name
    String stationNameStr = student_name + "'s Home Station";
    String ownerNameStr = student_id + " Station owner";
    String longNameStr = student_id + " Long";
    String latNameStr = student_id + " Lat";
    String temperatureNameStr = student_id + " Temperature";
    String humidityNameStr = student_id + " Humidity";
    String signalstrengthNameStr = student_id + " Signal Strength";
    
    //Convert the strings to const char*
    const char* stationName = stationNameStr.c_str();
    const char* ownerName = ownerNameStr.c_str();
    const char* longName = longNameStr.c_str();
    const char* latName = latNameStr.c_str();
    const char* temperatureName = temperatureNameStr.c_str();
    const char* humidityName = humidityNameStr.c_str();
    const char* signalstrengthName = signalstrengthNameStr.c_str();

    //Set main device name
    device.setName(stationName);
    device.setSoftwareVersion(SOFTWARE_VERSION);
    device.setManufacturer(STUDENT_NAME);
    device.setModel(MODEL_TYPE);

    sensorOwner.setName(ownerName);
    sensorOwner.setIcon("mdi:account");

    sensorLong.setName(longName);
    sensorLong.setIcon("mdi:crosshairs-gps");
    sensorLat.setName(latName);
    sensorLat.setIcon("mdi:crosshairs-gps");
    
    sensorTemperature.setName(temperatureName);
    sensorTemperature.setDeviceClass("temperature");
    sensorTemperature.setUnitOfMeasurement("°C");

    sensorHumidity.setName(humidityName);
    sensorHumidity.setDeviceClass("humidity");
    sensorHumidity.setUnitOfMeasurement("%");
    
    sensorSignalstrength.setName(signalstrengthName);
    sensorSignalstrength.setDeviceClass("signal_strength");
    sensorSignalstrength.setUnitOfMeasurement("dBm");

    mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);

    while (!mqtt.isConnected()) {
        mqtt.loop();
        Serial.print(".");
        delay(500); // waiting for the connection
    }
    
    Serial.println();
    Serial.println("Connected to MQTT broker");

    sensorOwner.setValue(STUDENT_NAME);

    sensorLat.setValue(LAT, (uint8_t)15U);
    sensorLong.setValue(LONG, (uint8_t)15U);
    
    dht.begin();
}

void loop() {
    mqtt.loop();
    
    humidityValue = dht.readHumidity();
    temperatureValue = dht.readTemperature();
    signalstrengthValue = WiFi.RSSI();

    if (isnan(humidityValue)) {
      humidityValue = 0;
    }
  
    if (isnan(temperatureValue)) {
      temperatureValue = 0;
    }

    if ((millis() - lastTemperatureSend) > 10000) { // read in 30ms interval
        
        sensorTemperature.setValue(temperatureValue);
        Serial.print("Current temperature is: ");
        Serial.print(temperatureValue);
        Serial.println("°C");

        sensorHumidity.setValue(humidityValue);
        Serial.print("Current humidity is: ");
        Serial.print(humidityValue);
        Serial.println("%");

        sensorSignalstrength.setValue(signalstrengthValue);
        Serial.print("Current signal strength is: ");
        Serial.print(signalstrengthValue);
        Serial.println("%");
        
        lastTemperatureSend = millis();
    }
}
