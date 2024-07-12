#include "pins.h"
#include <Arduino.h>
#include <SD_MMC.h>
#include <FS.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include "SHT85.h"
#include "Free_Fonts.h"
#include <ArduinoJson.h>  // Include ArduinoJson library
#include <esp_task_wdt.h>

#include <src/image_helper.h>
#include <src/SD_Card_helper.h>
#include <src/wifi_helper.h>
#include <src/mqtt_helper.h>
#include <src/OTA_helper.h>

#define WAIT 1000
#define SHT30_ADDRESS 0x44

float temperature = 0;
float humidity = 0;
SHT30 sht(SHT30_ADDRESS);

// Kalman filter variables
float temp_estimate = 0;
float temp_estimate_err = 1;
float temp_measure_err = 1;
float temp_process_noise = 0.1;

float hum_estimate = 0;
float hum_estimate_err = 1;
float hum_measure_err = 1;
float hum_process_noise = 0.1;

float normalized_temp = 0;
float normalized_humidity = 0;
float average_number = 0;

void startWatchDog() {
    //Serial.println("Start WatchDog");
    esp_task_wdt_init(20, true); // Enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); // Add current thread to WDT watch
}

void stopWatchDog() {
    //Serial.println("Stopping WatchDog");
    esp_task_wdt_delete(NULL);
    esp_task_wdt_deinit();
}

float applyKalmanFilter(float measurement, float &estimate, float &estimate_err, float measure_err, float process_noise) {
    // Prediction update
    estimate_err += process_noise;
    // Measurement update
    float kalman_gain = estimate_err / (estimate_err + measure_err);
    estimate += kalman_gain * (measurement - estimate);
    estimate_err = (1 - kalman_gain) * estimate_err;
    return estimate;
}

const int BUFFER_SIZE = 50;
float temperature_list[BUFFER_SIZE];
float humidity_list[BUFFER_SIZE];

float avr_temperature = 0;
float avr_humidity = 0;
int buffer_index = 0;

long lastMsg = 0;
long lastMsg_ = 0;
long lastMsg_update = 0;

void setup() {
    snprintf(boardID, 23, "%llX", ESP.getEfuseMac());
    for (int i = 0; i < BUFFER_SIZE; i++) {
        temperature_list[i] = 0;
        humidity_list[i] = 0;
    }

    Serial.begin(115200);
    delay(100);

    Init_Screen();

    delay(100);
    Setup_SD_card();
    read_get_data_SD_card();

    setup_wifi();
    setup_mqtt();
    client.setKeepAlive(60);
    client.setCallback(callback);

    Wire.setPins(16, 15); // Set I2C pins: SDA to pin 16, SCL to pin 15
    Wire.begin();
    Wire.setClock(100000);
    if (!sht.begin()) {
        Serial.println("Couldn't find SHT30");
        while (1) delay(1); // Stop here if SHT30 initialization fails
    }

    uint16_t stat = sht.readStatus();
    Serial.print(stat, HEX);
    // Call functions as needed
    check_name();
    Get_calib_data();
    checkForUpdates();
    updateFirmwareVersion();
    delay(100);
    reconnect();
}

void loop() {
    
    if (!client.connected()) {
        startWatchDog();
        reconnect();
        stopWatchDog();
        esp_task_wdt_reset();
    }
    client.loop();
    sht.read();

    // Apply Kalman filter to the sensor readings
    temperature = applyKalmanFilter((sht.getTemperature() * tSlope) + tIntercept, temp_estimate, temp_estimate_err, temp_measure_err, temp_process_noise);
    humidity = applyKalmanFilter((sht.getHumidity() * hSlope) + hIntercept, hum_estimate, hum_estimate_err, hum_measure_err, hum_process_noise);

    temperature_list[buffer_index] = temperature;
    humidity_list[buffer_index] = humidity;
    buffer_index = (buffer_index + 1) % BUFFER_SIZE;

    avr_temperature = 0;
    avr_humidity = 0;
    for (int j = 0; j < BUFFER_SIZE; j++) {
        avr_temperature += temperature_list[j];
        avr_humidity += humidity_list[j];
    }
    avr_temperature /= BUFFER_SIZE;
    avr_humidity /= BUFFER_SIZE;

    if (avr_temperature >= high_temp || avr_temperature <= low_temp) {
        normalized_temp = 1;
    } else {
        normalized_temp = abs(avr_temperature - (high_temp + low_temp) / 2) / (high_temp - ((high_temp + low_temp) / 2));     
    }

    if (avr_humidity <= low_humid || avr_humidity >= high_humid) {
        normalized_humidity = 1;
    } else {
        normalized_humidity = abs(avr_humidity - (high_humid + low_humid) / 2) / (high_humid - (high_humid + low_humid) / 2);
    }

    average_number = ((normalized_temp + normalized_humidity) / 2) * 100;

    long now = millis();
    if (now - lastMsg_ > 2000) {
        lastMsg_ = now;
        tft.setFreeFont(FSS18);
        tft.drawString(String(temperature) + " ", 86, 100, GFXFF);
        tft.drawString(String(humidity) + " ", 86, 204, GFXFF);
    }

    if (now - lastMsg_update > 300000) {
        lastMsg_update = now;
        checkForUpdates();
    }

    if (now - lastMsg > 15000) {
        lastMsg = now;

        if (!isnan(avr_temperature) && !isnan(avr_humidity)) {
            DynamicJsonDocument doc(512);  // Ensure sufficient size for JSON document

            doc["sensor_id"] = boardID;
            doc["normalized"] = average_number;
            doc["temperature"] = avr_temperature;
            doc["humidity"] = avr_humidity;
            doc["displayName"] = Device_name;
            doc["org"] = "TG3";
            doc["dept"] = "Test Lab";
            doc["room"] = "ATF";
            doc["line"] = "123";
            char jsonBuffer[512];

            serializeJson(doc, jsonBuffer);
            client.publish("VNTESTLAB/TEMP&HUMID", jsonBuffer);
        } else {
            Serial.println("Failed to read from SHT30 sensor!");
        }
    }
}
