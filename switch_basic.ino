/**
 * lamp basic
 * sleep time - https://github.com/esp8266/Arduino/issues/1381
 * connects to wifi
 * connects to mqtt
 * just waits and toggle lamp
 * 
 */

/**
 * GPIO Mapping
 * static const uint8_t D0   = 16; // Reset - Deep Sleep
 * static const uint8_t D1   = 5;  
 * static const uint8_t D2   = 4;  
 * static const uint8_t D3   = 0;  
 * static const uint8_t D4   = 2;  // Status LED
 * static const uint8_t D5   = 14;
 * static const uint8_t D6   = 12;
 * static const uint8_t D7   = 13;
 * static const uint8_t D8   = 15;
 * static const uint8_t D9   = 3;
 * static const uint8_t D10  = 1;
 */

/**
 * Additional Libraries
 */
#include <ESP8266WiFi.h> // wifi library
#include <PubSubClient.h> // mqtt client library
#include <Bounce2.h> // buttonswitch handling library
#include <EEPROM.h> // write/read eeprom library
#include <everytime.h> // timer library to set timeouts
#include <TaskScheduler.h> // addtional timing
#include <stdlib.h> // string conversion
extern "C" {
  #include "user_interface.h" // additional types library
}

/**
 * wlan settings
 */ 
const char* ssid = "";
const char* wlanPassword = "";

IPAddress router(192, 168, 178, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress fixedIP(192, 168, 178, 153);

/**
 * mqtt settings
 */
const char mqttClientServer[] = "192.168.178.33";
const char mqttClientId[] = "home/bedroom/switch/0"; // location/room/type/id
const char mqttPublishCommand[] = "home/bedroom/lamp/0/toggle"; // location/room/type/id/command

/**
 * system settings
 */
const byte statusGPIO = B10; // 2
const bool debugMode = true;

/**
 * shutter status
 */
bool currentStatus = false;

/**
 * instantiate objects
 */
WiFiClient espClient;
PubSubClient client(espClient);

/**
 * Main Loop
 */
void loop() {
  yield();
}

/**
 * mqtt
 */
void mqttEventHandler(char* topic, byte* payload, unsigned int length) {
  char msg[length];
  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
  }

  if(debugMode) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.print(msg);
    Serial.println();
  }
}


void mqttConnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    if(debugMode) {
      Serial.print("Attempting MQTT connection...");
    }
    // Attempt to connect
    if (client.connect(mqttClientId)) {
      if(debugMode) {
        Serial.println("connected");
      }
    } else {
      if(debugMode) {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      }
      // Wait 5 seconds before retrying
      delay(5000);
      
    }
  }
}


/** 
 *  Setup
 */
void setup() {
  // start debug mode
  if(debugMode) {
    Serial.begin(115200);
    Serial.print("\n");
    Serial.print("Start Debugmode\n");
      
    rst_info *resetInfo;
    resetInfo = ESP.getResetInfoPtr();
    Serial.println(resetInfo->reason);
    
    if( resetInfo->reason == REASON_DEEP_SLEEP_AWAKE ) {
      Serial.println("Deep Sleep Reset");
    } else {
      Serial.println("Other Reset Reason");
    }
  }
  // setup button
  setupGpio();
  // setup wifi
  setupWifi();
  // setup mqtt
  setupMQTT();
  // toggleCommand
  toggleCommand();
  // go back to deepSleep
  enterSleepMode();
}

void toggleCommand() {
  client.publish(mqttPublishCommand, "1");
}

/**
 * SleepMode
 */
void enterSleepMode() {
  if(debugMode) {
    Serial.println("Enter Deep Sleep");
  }
  ESP.deepSleep(0); 
}

void setupGpio() {
  if(debugMode) {
    Serial.print("Setup GPIOs\n"); 
  }
}

void setupWifi() {
  // We start by connecting to a WiFi network
  if(debugMode) {
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  }

  // we use a fixed ip for quick access
  WiFi.begin(ssid, wlanPassword);
  WiFi.config(fixedIP, router, subnet, router, router);

  while (WiFi.status() != WL_CONNECTED) {
    for(int i = 0; i<500; i++){
      delay(1);
    }
    if(debugMode) {
      Serial.print(".");
    }
  }
  if(debugMode) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  
}

void setupMQTT() {
  client.setServer(mqttClientServer, 1883);
  client.setCallback(mqttEventHandler);
  mqttConnect();
}


