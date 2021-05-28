#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "mecanum_wheel_1.h"
#define MAX_MSG_LEN (128)
#define LED (2)

const char* ssid = "RoboWifi";
const char* password = "73333449";
const char *serverHostname = "192.168.1.17";
// const IPAddress serverIPAddress(192, 168, 1, 3);
const char *topic = "raspberry/new";
const char* mqtt_username = "sumobot";
const char* mqtt_password = "sumobot";
WiFiClient espClient;
PubSubClient client(espClient);

StaticJsonDocument<60> jsonBuffer;
// Motor A connections
//1,2 is for left front, 3,4 is for right front
//7,8 is for left back, 5,6 is for right back
int inRF1 = D3;
int inRF2 = D2;
int inLF1 = D1;
int inLF2 = D0;
int inRB1 = D6;
int inRB2 = D5;
int inLB1 = D8;
int inLB2 = D7;

//Functions for wheels to not moving
void leftFrontStp(){
  digitalWrite(inLF1, LOW);
  digitalWrite(inLF2, LOW);
}

void rightFrontStp(){
  digitalWrite(inRF1, LOW);
  digitalWrite(inRF2, LOW);
}

void leftBackStp(){
  digitalWrite(inLB1, LOW);
  digitalWrite(inLB2, LOW);
}

void rightBackStp(){
  digitalWrite(inRB1, LOW);
  digitalWrite(inRB2, LOW);
}

//functions for 4 wheels to move forwards and backwards
void leftFrontFd(){
  digitalWrite(inLF1, HIGH);
  digitalWrite(inLF2, LOW);
}

void rightFrontFd(){
  digitalWrite(inRF1, HIGH);
  digitalWrite(inRF2, LOW);
}

void leftBackFd(){
  digitalWrite(inLB1, HIGH);
  digitalWrite(inLB2, LOW);
}

void rightBackFd(){
  digitalWrite(inRB1, HIGH);
  digitalWrite(inRB2, LOW);
}

void leftFrontBd(){
  digitalWrite(inLF1, LOW);
  digitalWrite(inLF2, HIGH);
}

void rightFrontBd(){
  digitalWrite(inRF1, LOW);
  digitalWrite(inRF2, HIGH);
}

void leftBackBd(){
  digitalWrite(inLB1, LOW);
  digitalWrite(inLB2, HIGH);
}

void rightBackBd(){
  digitalWrite(inRB1, LOW);
  digitalWrite(inRB2, HIGH);
}

//THIS WILL BE THE ACTUAL FUNCTION THAT WE CAN USE
void allStop(){
  digitalWrite(inLF1, LOW);
  digitalWrite(inLF2, LOW);
  digitalWrite(inRF1, LOW);
  digitalWrite(inRF2, LOW);
  digitalWrite(inLB1, LOW);
  digitalWrite(inLB2, LOW);
  digitalWrite(inRB1, LOW);
  digitalWrite(inRB2, LOW);
}

void moveForwards(){
  leftFrontFd();
  rightFrontFd();
  leftBackFd();
  rightBackFd();
}

void moveBackwards(){
  leftFrontBd();
  rightFrontBd();
  leftBackBd();
  rightBackBd();
}

void moveRight(){
  rightFrontFd();
  leftBackFd();
  rightBackBd();
  leftFrontBd();
}

void moveLeft(){
  rightBackFd();
  leftFrontFd();
  rightFrontBd();
  leftBackBd();
}

void turnClockwise(){
  leftFrontFd();
  leftBackFd();
  rightFrontBd();
  leftBackBd();
}

void turnAntiClockwise(){
  leftFrontBd();
  leftBackBd();
  rightFrontFd();
  leftBackFd();
}


void diagonalDownLeft(){
  leftFrontBd();
  rightBackBd();
  leftBackStp();
  rightFrontStp();
}

void diagonalUpRight(){
  leftFrontFd();
  rightBackFd();
  leftBackStp();
  rightFrontStp();
}

void diagonalUpLeft(){
  rightFrontFd();
  leftBackFd();
  rightBackStp();
  leftFrontStp();
}

void diagonalDownRight(){
  rightFrontBd();
  leftBackBd();
  rightBackStp();
  leftFrontStp();
}

void setup() {

  // Set all the motor control pins to outputs

  pinMode(inLF1, OUTPUT);
  pinMode(inLF2, OUTPUT);
  pinMode(inRF1, OUTPUT);
  pinMode(inRF2, OUTPUT);
  pinMode(inLB1, OUTPUT);
  pinMode(inLB2, OUTPUT);
  pinMode(inRB1, OUTPUT);
  pinMode(inRB2, OUTPUT);
  
  // Turn off motors - Initial state
  digitalWrite(inLF1, LOW);
  digitalWrite(inLF2, LOW);
  digitalWrite(inRF1, LOW);
  digitalWrite(inRF2, LOW);
  digitalWrite(inLB1, LOW);
  digitalWrite(inLB2, LOW);
  digitalWrite(inRB1, LOW);
  digitalWrite(inRB2, LOW);
  
  // LED pin as output
  // LED pin is the same as D4!!
 // pinMode(LED, OUTPUT);      
 // digitalWrite(LED, HIGH);
  // Configure serial port for debugging
  Serial.begin(115200);
  // Initialise wifi connection - this will wait until connected
  connectWifi();
  // connect to MQTT server  
client.setServer(serverHostname, 1883);
client.setCallback(callback);

while (!client.connected()) {
    Serial.println("Connected to MQTT Broker!");
    
  if(client.connect("ESP", mqtt_username, mqtt_password)){
    Serial.println("connected");
  }

  else{
    Serial.println("failed");
    Serial.println(client.state());
    delay(200);
  }
    
  }

  client.subscribe(topic);
  
}

void loop() {
    /*if (!client.connected()) {
      connectMQTT();
    }
    // this is ESSENTIAL!*/
    client.loop();
    // idle
   // delay(500);
}

void connectWifi() {
  delay(10);
  // Connecting to a WiFi network
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected on IP address ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  // Wait until we're connected 
  while (!client.connect(serverHostname, mqtt_username, mqtt_password)) {
    // Create a random client ID
    String clientId = "ESP8266-";
    clientId += String(random(0xffff), HEX);
    Serial.printf("MQTT connecting as client %s...\n", clientId.c_str());
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      client.subscribe(topic);
    } else {
      Serial.println("MQTT failed, state");
      // Wait before retrying
      delay(2500);
    }
  }
}

void callback(char *msgTopic, byte *msgPayload, unsigned int msgLength) {
  // copy payload to a static string
static char message[MAX_MSG_LEN+1];
  if (msgLength > MAX_MSG_LEN) {
    msgLength = MAX_MSG_LEN;
  }
  strncpy(message, (char *)msgPayload, msgLength);
  message[msgLength] = '\0';
  
//  Serial.printf("topic %s, message received: %s\n", msgTopic, message);
  DeserializationError err = deserializeJson(jsonBuffer, msgPayload);
//  JsonObject& root = jsonBuffer.parseObject(msgPayload);
  if (err) {
    Serial.print(F("deserializeJson() failed with code ")); 
    Serial.println(err.c_str());
  }
  int move = jsonBuffer["move"];
  Serial.println(move);
  if(move == 1)
    moveForwards();
  if(move == 0)
    allStop();
  
  // decode message
//  if (strcmp(message, "off") == 0) {
//    setLedState(false);
//  } else if (strcmp(message, "on") == 0) {
//    setLedState(true);
//  }
}

void setLedState(boolean state) {
  // LED logic is inverted, low means on
  digitalWrite(LED, !state);
}
