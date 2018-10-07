#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspServer.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

volatile unsigned char pm25;
volatile unsigned char pm10;
volatile bool flag = false;
unsigned char inChar;
int count1 = 0;

//###################################################################
//WIFI configuration
//SoftwareSerial esp(7 ,6); // RX, TX
//SoftwareSerial dustSerial(4 ,3); // RX, TX

char ssid[] = "Namli";            // your network SSID (name)
char pass[] = "9903098610";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

//char mqttServer[] = "mqtt.thingspeak.com";
char mqttServer[] = "171.253.28.46";
char ClientID[] = "Station1";
String arrivedData;

WiFiEspClient espClient;
PubSubClient client(espClient);

//###################################################################
//DHT configuration
DHT dht(5, DHT11);

//######################################################################
//Delay
unsigned long count;

void delay_ms (uint16_t millisecond) {
  unsigned long sec;
  sec = ((16000000/12)/1000)*millisecond;
  for (count = 0; count < sec; count ++);
}
//###############################################################

void RobotInfor(){
  
  // print the SSID of the network you're attached to
  Serial.print("###ClientID: ");
  Serial.print (ClientID);

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("---IP Address: ");
  Serial.println(ip);
}

void ConnectToWiFi (){
    Serial2.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial2);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("ESP is not connected");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
}

void callback(char* topic, byte* payload, unsigned int length) {
  arrivedData = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    arrivedData += (char)payload[i];
  }
  Serial.println();
  Serial.println("########################");
  Serial.println(arrivedData);
  Serial.println("########################");
  Serial.println();
  //delay_ms(300);
  //turnLED();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println ("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    if (client.connect(ClientID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay_ms(5000);
    }
  }
}

void serialEvent1()
{    
   inChar = Serial1.read();
   if (inChar == 0x42)
   {
     count1 = 1;
   }
   else 
   {
     count1 ++;
     if (count1 == 14)
     {   
       pm25 = inChar;           
     }
     else if (count1 == 16)
     {
       pm10 = inChar;
       flag = true;
     }
   }
   
}

void mqttPublish (){
  Serial.println ("Sending data");
  int h = dht.readHumidity();
  int t = dht.readTemperature();
  // Data
  String Data = String(h) + String (";") + String(t) + String (";") + String (pm25) + String(";") + String (pm10);
  int length = Data.length();
  char Buff[length];
  Data.toCharArray (Buff, length + 1);
  //Publish packet
  client.publish( ClientID, Buff );
  Serial.println (Data);
  return;
}

void setup()
{
  //###################
  //General 
  Serial.begin(9600);
  Serial1.begin (9600);
  //####################
  //ESP
  ConnectToWiFi ();
  client.setServer (mqttServer, 1883);
  client.setCallback (callback);
  //####################
  //DHT
  dht.begin ();
//  RobotInfor();
Serial.print ("Hello");
}

void loop(){
if (flag == true){
    if (!client.connected()) {
    reconnect();
    client.subscribe ("System", 0);
    //client.loop();
  }
  client.loop();
if (flag == false){
  return;
}
  mqttPublish();
  flag = false;
  return;
}


  return;
}
