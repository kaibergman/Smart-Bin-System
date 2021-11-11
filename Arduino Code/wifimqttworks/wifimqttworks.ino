#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";              //WiFi Name
const char* password = "";      //WiFi Password
const char* server= "";    //RPi or Machine IP on which the broker is
//assigned pins for the ultrasonic sensor
const int TRIG_PIN = 18; // ESP32 pin GIOP18 connected to Ultrasonic Sensor's TRIG pin
const int ECHO_PIN = 23; // ESP32 pin GIOP23 connected to Ultrasonic Sensor's ECHO pin

float duration_us, distance_cm;

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;
WiFiClient espClient;
PubSubClient client(espClient);

int setup_WiFi(){
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Attempting MQTT connection...");
  client.connect("esp32", "esp32", "1234");      
  if (client.connect("esp32", "esp32", "1234")){                 
      Serial.println("connected");
    }
    else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
    }
    return 0;
}

int reconnect() {
  unsigned long startAttemptTime = millis();
  while (!client.connected()) 
  {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("esp32", "esp32", "1234")){                 //if (client.connect("esp32", MQTT_USER, MQTT_PASS)) {
      Serial.println("connected");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  return 0;
}

int send_mqtt(){
  setup_WiFi();
  char sss[15];
  sprintf(sss, "%f", distance_cm);
  //char sss[15]= (char) distance_cm;
  
   if (!client.connected()){
    reconnect();
  } 
  client.publish("esp32/test", sss);    //send message
  WiFi.disconnect(true);
  Serial.println("Sent");
  return 0;
}
void setup() {
  Serial.begin(115200);
  
  // configure the trigger pin to output mode
  pinMode(TRIG_PIN, OUTPUT);
  // configure the echo pin to input mode
  pinMode(ECHO_PIN, INPUT);
  
  client.setServer(server, 1883);   //mqtt server details
  setup_WiFi();
  reconnect();
  send_mqtt();
}

void loop() {
 // generate 10-microsecond pulse to TRIG pin
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

   // measure duration of pulse from ECHO pin
  duration_us = pulseIn(ECHO_PIN, HIGH);
    // calculate the distance
  distance_cm = 0.017 * duration_us;
  
  send_mqtt();
  delay(1000);     //Wait 1 secs before next transmission
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
