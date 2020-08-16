
#include <WiFi.h>
#include <PubSubClient.h>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/encodedstream.h>
#include <fstream>
#include <json/value.h>
#include <json/json.h>
#include <assert.h>
#include <cstdio>
#include "info.h"


using namespace std;
using namespace rapidjson;


boolean status = false;
long currentTemp = 0;
long firstTime = 0, lastTime = 0;
char messagess[50];



WiFiClient espClient;
PubSubClient client(espClient);

void setupWiFi(){
  delay(100);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print("-");
  }
  Serial.print("\nConnected to ");
  Serial.println(ssid);
}

void reconnect(){
  while(!client.connected()){
    Serial.print("\nConnecting to");
    Serial.println(broker);
    status = client.connect("TestClient", userId, brokerPass);
    if(status){
      Serial.print("\nConnected to");
      Serial.println(broker);
      client.subscribe(outTopicLED);
      printf("Subscribing to %s\n", outTopicLED);
    }
    else{
      Serial.println("\nTrying to connect again..");
      delay(5000);
    }
  }
}

int get_temperature(long time) {
  // calls every 2 second/ 2000 milis.
  static int inc = 0;
  static float ofs = 2;
  if(inc <= 40){
    float min = 30, max = 33;
    
    return (inc += ofs) + (min + 1) + (((float)rand()) / (float)RAND_MAX) * (max - (min + 1));
  }
  else{
    float min = 75, max = 80;
    return (min + 1) + (((float)rand()) / (float)RAND_MAX) * (max - (min + 1));
  }

}

void switchLED(byte* payload){
  if((char) payload[0] == '1'){
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else{
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void callback(char* topic, byte* payload, unsigned int length){
    if( strcmp(topic, outTopicLED) == 0){
      Serial.println("Topic match!");
    }
    Serial.print("Recived messages: ");
    Serial.println(topic);
    switchLED(payload);
    for(int i = 0; i < length; i++){
      if((char) payload[i] == '1'){
        Serial.print("LED_STATE = [ON]");
      }
      else{
        Serial.print("LED_STATE = [OFF]");
      }
    }
    Serial.println();
  }


void setup() {
  //json setup.


  // put your setup code here, to run once:
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(9600);
  //Serial.println(d["ssid"].GetString());
  setupWiFi();
  client.setServer(broker, 1883);
  //auto lambda = [](char* topic, byte* payload, unsigned int length){;};
  client.setCallback(callback);
}


void loop() {
  // put your main code here, to run repeatedly:
  if(!status){
    reconnect();
  }
  // calling to allow incomming messages and publish data and refresh the connection.
  client.loop();
  
  firstTime = millis();

  if(firstTime - lastTime > 5000){
    currentTemp = get_temperature(firstTime);
    sniprintf(messagess, 75, "temperature of ESP: %ld", currentTemp);
    Serial.print("Sending messages: ");
    Serial.println(messagess);
    client.publish(outTopicTemp, messagess);
    lastTime = millis();
  }
}