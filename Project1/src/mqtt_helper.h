#include <PubSubClient.h>
#include <WiFi.h>

char boardID[23];
WiFiClient espClient;
PubSubClient client(espClient);



void callback(char* topic, byte* message, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;

    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
    }
    Serial.println();

    if (String(topic) == "/ESP32TempCMD") {
        Serial.println(messageTemp);
        if (messageTemp == "RESTART") {
            ESP.restart();
        } 
    }
}

void reconnect() {
    // Ensure WiFi is connected
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Loop until we're reconnected
    while (!client.connected()) {
        drawRGBImage(44, 293, 24, 24, mqtt_off_logo);
        Serial.print("Attempting MQTT connection...");
        client.disconnect();
        char connect_status[150];
        snprintf(connect_status, sizeof(connect_status), "{\"status\":\"%s\", \"client\": \"%s\"}", "Disconnected", boardID);
        // Attempt to connect with MQTT username and password
        if (client.connect(boardID, mqtt_username, mqtt_password, ("/ConnectStatus/" + String(boardID)).c_str() , 1, true, connect_status)) {
            Serial.println("connected");
            drawRGBImage(44, 293, 24, 24, mqtt_on_logo);

            // Publish connection status
            
            snprintf(connect_status, sizeof(connect_status), "{\"status\":\"%s\", \"client\": \"%s\", \"appver\": \"%s\"}", "Connected", boardID, APPVERSION);
            client.publish(("/ConnectStatus/" + String(boardID)).c_str(), connect_status,true);

            // Subscribe to topics
            client.subscribe("/ESP32TempCMD");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup_mqtt() {
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}