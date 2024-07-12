#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <iostream>
#include <string>
#include <sstream>

HTTPClient http;

float tIntercept = 0;
float hIntercept = 0;
float tSlope     = 0;
float hSlope     = 0;
float low_temp   = 0;
float high_temp  = 0;
float low_humid  = 0;
float high_humid = 0;
float Hum        = 0;
float Temp       = 0;

String currentVersion = "";
String latestVersion = "";
String filename = "";

String Device_name = "";
String exist = "";
String org;
String dept;
String room;
String line;
String display_name;

String Calib_Url = "http://10.147.37.59:5001/api/getExtInfo?key=XXV7lnIse9q4YGA11pXA&code=";
String versionUrl = "http://10.147.37.59:5001/getESPFirm?key=XXV7lnIse9q4YGA11pXA&filePrefix=tempSensorLily&screenSize=2.8&version=" + String(APPVERSION) + "&check=Y";

void downloadAndUpdate();

bool checkForUpdates() {
    http.begin(versionUrl);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Received payload: " + payload);
        StaticJsonDocument<1024> doc;
        deserializeJson(doc, payload);
        
        latestVersion = doc["hasnewversion"].as<String>();
        
    } else {
        http.end();
    }

    if (latestVersion == "Y") {
        Serial.print("has new version");
        downloadAndUpdate();
    } else {
        Serial.print("has no new version");
    }
    return false;
}

void extract_range(const std::string &range_str, float &low, float &high) {
    std::stringstream ss(range_str);
    std::string item;

    // Get the first number
    std::getline(ss, item, '-');
    low = std::stof(item);

    // Get the second number
    std::getline(ss, item, '-');
    high = std::stof(item);
}

void Get_calib_data() {
    HTTPClient http;

    // Construct the URL with boardID
    String url = Calib_Url + String(boardID);

    // Begin HTTP request
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Received payload: " + payload);

        // Parse JSON payload
        DynamicJsonDocument doc(1024); // Adjust the size as needed
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return;
        }

        // Extract calibration data
        tIntercept = doc["tIntercept"];
        hIntercept = doc["hIntercept"];
        tSlope = doc["tSlope"];
        hSlope = doc["hSlope"];

        org = doc["org"].as<String>();
        dept = doc["dept"].as<String>();
        room = doc["room"].as<String>();
        line = doc["line"].as<String>();
        display_name = doc["display_name"].as<String>();

        // Extract temperature range
        if (doc["Tem"].is<const char*>()) {
            std::string tempRangeStr = doc["Tem"].as<const char*>();
            extract_range(tempRangeStr, low_temp, high_temp);
            Serial.print("Temperature range: ");
            Serial.print(low_temp);
            Serial.print(" - ");
            Serial.println(high_temp);
        } else {
            Serial.println("Temperature range format error");
        }

        // Extract humidity range
        if (doc["Hum"].is<const char*>()) {
            std::string humidRangeStr = doc["Hum"].as<const char*>();
            extract_range(humidRangeStr, low_humid, high_humid);
            Serial.print("Humidity range: ");
            Serial.print(low_humid);
            Serial.print(" - ");
            Serial.println(high_humid);
        } else {
            Serial.println("Humidity range format error");
        }
    } else {
        Serial.print("HTTP GET failed, error: ");
        Serial.println(http.errorToString(httpCode).c_str());
    }

    // End HTTP request
    http.end();
}

void downloadAndUpdate() {
    String firmwareUrl = "http://10.147.37.59:5001/getESPFirm?key=XXV7lnIse9q4YGA11pXA&filePrefix=tempSensorLily&screenSize=2.8&version=" + String(APPVERSION) + "&update=Y";
    http.begin(firmwareUrl);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        int contentLength = http.getSize();
        WiFiClient* stream = http.getStreamPtr();
        if (contentLength > 0) {
            bool canBegin = Update.begin(contentLength);
            if (canBegin) {
                size_t written = Update.writeStream(*stream);
                if (written == contentLength) {
                    Serial.println("Update successfully written");
                    if (Update.end()) {
                        if (Update.isFinished()) {
                            Serial.println("Update successfully completed");
                            ESP.restart();
                        } else {
                            Serial.println("Update not finished");
                        }
                    } else {
                        Serial.printf("Update error: %s\n", Update.errorString());
                    }
                } else {
                    Serial.println("Written file size does not match content length");
                }
            } else {
                Serial.println("Not enough space to begin OTA");
            }
        } else {
            Serial.println("Empty payload received");
        }
    } else {
        Serial.printf("HTTP GET failed: %d\n", httpCode);
    }
    http.end();
}






void check_name() {
    String server_Url = "http://10.147.37.59:5001/api/checkexist?key=XXV7lnIse9q4YGA11pXA&code=";
    server_Url += boardID;
    server_Url += "&devicetype=TEMP";
    http.begin(server_Url);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Received payload: " + payload);
        StaticJsonDocument<1024> doc;
        deserializeJson(doc, payload);
 
        Device_name = doc["deviceName"].as<String>();
        exist = doc["exist"].as<String>();
        Serial.println("Device name: " + Device_name);
        Serial.println("Is exist: " + exist);

        if (exist == "N") {
            HTTPClient http;

            // JSON payload
            String jsonPayload = "{\"org\":\"" + org + "\",\"dept\":\"" + dept + "\",\"room\":\"" + room + "\",\"line\":\"" + line + "\",\"display_name\":\"" + display_name + "\",\"device_name\":\"" + Device_name + "\",\"u_id\":\"" + boardID + "\",\"device_type\":\"TEMP\"}";

            http.begin("http://10.147.37.59:5001/api/esp_data");
            http.addHeader("Content-Type", "application/json");

            int httpResponseCode = http.POST(jsonPayload);

            if (httpResponseCode > 0) {
                Serial.print("HTTP Response code: ");
                Serial.println(httpResponseCode);
                String response = http.getString();
                Serial.println(response);
            } else {
                Serial.print("Error code: ");
                Serial.println(httpResponseCode);
            }
            http.end();
        }

    } 
}

void updateFirmwareVersion() {
    Serial.println("Updating firmware version...");
    String url = "http://10.147.37.59:5001/api/esp_data/firm_ver?key=XXV7lnIse9q4YGA11pXA&code=" + String(boardID);

    DynamicJsonDocument doc(512);
    doc["firm_ver"] = String(APPVERSION);

    char payloadBuffer[512];
    serializeJson(doc, payloadBuffer);

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.PUT(payloadBuffer);

    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    
    http.end();
}
