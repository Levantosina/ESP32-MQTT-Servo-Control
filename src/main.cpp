#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#define BUTTON_PIN_1 18
#define BUTTON_PIN_2 5

const char* ssid = "Wokwi-GUEST";  
const char* password = "";

// MQTT Broker Details
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
String stMac;
char mac[50];
char clientId[50];

WiFiClient espClient;
PubSubClient client(espClient);
Servo myservo;

// Servo Motor Configuration
const int servoPin = 21;  
int pos = 90;             
void callback(char* topic, byte* message, unsigned int length);
void wifiConnect();

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));

  Serial.println("Connecting to WiFi...");
  wifiConnect();  // Connect to WiFi
  
  Serial.println("WiFi connected!");
  Serial.println("IP Address: " + WiFi.localIP().toString());
  Serial.println("MAC Address: " + WiFi.macAddress());
  
  stMac = WiFi.macAddress();
  stMac.replace(":", "_");
  
  // MQTT Setup
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  myservo.attach(servoPin);
  myservo.write(pos);  // Set servo to initial position
}

void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    long r = random(1000);
    sprintf(clientId, "ESP32_ServoClient-%ld", r);
    
    if (client.connect(clientId)) {
      Serial.println(" Connected!");
      client.subscribe("servo/control");  // Subscribe to servo control topic
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Trying again in 5 seconds...");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String receivedMsg = "";
  for (int i = 0; i < length; i++) {
    receivedMsg += (char)message[i];
  }
  Serial.println(receivedMsg);

 
  if (receivedMsg == "UP" && pos < 180) {
    pos += 5;
  } 
  else if (receivedMsg == "DOWN" && pos > 0) {
    pos -= 5;
  }

  myservo.write(pos);
  Serial.print("Servo Position: ");
  Serial.println(pos);
}

void loop() {
  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();

  // Read button states
  int buttonState1 = digitalRead(BUTTON_PIN_1);
  int buttonState2 = digitalRead(BUTTON_PIN_2);

  // When Button 1 is pressed, send an MQTT message to move DOWN
  if (buttonState1 == LOW) {
  
    Serial.println("Button 1 Pressed! Moving DOWN");
    client.publish("servo/control", "DOWN");  
    delay(200);  // Debounce delay
  }
  
  // When Button 2 is pressed, send an MQTT message to move UP
  if (buttonState2 == LOW) {
  
    Serial.println("Button 2 Pressed! Moving UP");
    client.publish("servo/control", "UP");  
    delay(200);  // Debounce delay
  }
}