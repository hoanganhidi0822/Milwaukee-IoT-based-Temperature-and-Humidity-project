#include <Arduino.h>
#include <SD_MMC.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "src/Cipher.h"

char key[32] = "abcdefghijklmnop";
Cipher *cipher = new Cipher();

char ssid[32];
char password[64];
char mqtt_password[64];

char mqtt_server[64];
char mqtt_username[64];
char device_name[64];  // Allocate sufficient space for device_name

void Setup_SD_card() {  // Initialize SD card
    pinMode(PWR_EN_PIN, OUTPUT);
    digitalWrite(PWR_EN_PIN, HIGH);
    SD_MMC.setPins(SD_SCLK_PIN, SD_MOSI_PIN, SD_MISO_PIN);
    bool rlst = SD_MMC.begin("/sdcard", true);
    if (!rlst) {
        Serial.println("SD init failed");
        Serial.println("➸ No detected SdCard");
        while (1) delay(1); // Stop here if SD card init fails
    } else {
        Serial.println("SD init success");
        Serial.printf("➸ Detected SdCard insert: %.2f GB\r\n", SD_MMC.cardSize() / 1024.0 / 1024.0 / 1024.0);
    }
}

void read_get_data_SD_card() {
    cipher->setKey(key);
    // Read data from the file
    File file = SD_MMC.open("/Jsondata.json");
    if (!file) {
        Serial.println("Failed to open file for reading");
        while (1) delay(1); // Stop here if JSON file read fails
    } else {
        Serial.println("Reading from file:");
        String fileContent;
        while (file.available()) {
            fileContent += (char)file.read();
        }
        file.close();
        Serial.println(fileContent);
        Serial.println("\nRead complete....");

        // Parse JSON data
        DynamicJsonDocument doc(1024); // Adjust the size as per your JSON size
        DeserializationError error = deserializeJson(doc, fileContent);
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.f_str());
            while (1) delay(1); // Stop here if JSON parsing fails
        }

        // Check and encode/decode WiFi and MQTT passwords
        const char* wifi_pwd = doc["wifi_pwd"];
        const char* wifi_encoded = doc["wifi_encoded"];
        const char* mqtt_pwd = doc["mqtt_pwd"];
        const char* mqtt_encoded = doc["mqtt_encoded"];
        
        if (strcmp(wifi_encoded, "False") == 0 || strcmp(mqtt_encoded, "False") == 0) {
            if (strcmp(wifi_encoded, "False") == 0) {
                String wifi_pwd_encrypted = cipher->encryptString(wifi_pwd);
                doc["wifi_pwd"] = wifi_pwd_encrypted; 
                doc["wifi_encoded"] = "True";
            }

            if (strcmp(mqtt_encoded, "False") == 0) {
                String mqtt_pwd_encrypted = cipher->encryptString(mqtt_pwd);
                doc["mqtt_pwd"] = mqtt_pwd_encrypted;
                doc["mqtt_encoded"] = "True";   
            }
            
            // Serialize JSON data back to a string
            String newJsonData;
            serializeJson(doc, newJsonData);

            // Write modified JSON data back to the file
            file = SD_MMC.open("/Jsondata.json", FILE_WRITE);
            if (!file) {
                Serial.println("Failed to open file for writing");
            } else {  
                file.println(newJsonData);
                file.close();
                Serial.println("Write complete.");
            }
        }

        // Decrypt WiFi and MQTT passwords
        String wifi_pwd_decrypted = cipher->decryptString(doc["wifi_pwd"]);
        String mqtt_pwd_decrypted = cipher->decryptString(doc["mqtt_pwd"]);

        // Copy configurations
        strlcpy(ssid, doc["wifi_ssid"], sizeof(ssid));
        strlcpy(password, wifi_pwd_decrypted.c_str(), sizeof(password));
        strlcpy(mqtt_server, doc["mqtt_broker"], sizeof(mqtt_server));
        strlcpy(mqtt_username, doc["mqtt_user"], sizeof(mqtt_username));
        strlcpy(device_name, doc["DeviceName"] | "TEMP0022", sizeof(device_name)); // Ensure device_name is correctly assigned
        strlcpy(mqtt_password, mqtt_pwd_decrypted.c_str(), sizeof(mqtt_password));
    }
}
