#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "DroidMech";
const char* password = "tuilatuila";
const char* mqttServer = "m12.cloudmqtt.com";
const int mqttPort = 13196;
const char* mqttUser = "nowarpld";
const char* mqttPassword = "rAunLzck-Jsy";

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi network");
  }

  Serial.println("Connected!");

  client.setServer(mqttServer, mqttPort);

  while (!client.connected())
  {
    Serial.println("Connecting to MQTT");

    if (client.connect("ESP32Client", mqttUser, mqttPassword))
    {
      Serial.println("Connected to MQTT!");
    } else {
      Serial.print("Failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }

  client.publish("esp/test", "Hello from Esp32!");

  Serial.println("Published message!");
}

void loop() {
  // put your main code here, to run repeatedly:
  client.loop();
}
