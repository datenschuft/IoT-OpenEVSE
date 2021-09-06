// based on 
/////////////////////////////////////////////////////////////////////////////////////////
// Dieser Sketch ist Teil des Download-Paketes zum Buch "Projekte mit Arduino und ESP" //
// von Danny Schreiter, erschienen 2020 im BMU-Verlag. Im Buch finden Sie Hinweise und //
// Erklärungen zur Funktion des Programmcodes.                  https://bmu-verlag.de/ //
/////////////////////////////////////////////////////////////////////////////////////////

// wenger florian 
// smadaemon überträgt die daten via mqtt an node red 
// und node red steuert die autoladung via mqtt
// esp8266 empfängt die mqtt nachrichten und steuert die openevse

#define NETZWERKNAME "wlan-iot"       // WLAN-SSID muss angepasst werden
#define PASSWORT "verysecure123"       // WLAN-Passwort muss angepasst werden
#define MQTT_BROKER "IPOFMQTTBROKER"
#define MQTT_TOPIC_OUT "ESP/EVSE1/Output/"

#define MQTT_TOPIC_IN "ESP/EVSE1/Input/#"
#define MQTT_CLIENT "ESP_EVSE1"

#include "ESP8266WiFi.h"    // ESP-Wifi-Bibliothek (im Boardmanager enthalten)
#include "PubSubClient.h"   // MQTT-Bibliothek (von Nick O'Leary, verfügbar über den Bibliotheksmanager)

WiFiClient WLAN_Client;
PubSubClient MQTT_Client(WLAN_Client);

unsigned long startTime;

void setup() {
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  pinMode(0, OUTPUT);
  digitalWrite(2, 0);
  digitalWrite(0, 1);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(NETZWERKNAME);

  delay(100); 
  WiFi.begin(NETZWERKNAME, PASSWORT);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
    //communicate ("FP 0 1 wifi-setup      ");
    Serial.print(WiFi.status());
    WiFi.printDiag(Serial);
  }
  //Serial.println("");
  String ip = WiFi.localIP().toString();
  //Serial.print("Erfolgreich. Eigene IP-Adresse: ");
  //Serial.println(WiFi.localIP());
  //communicate ("FP 0 1 " + ip);
  delay(500);
  MQTT_Client.setServer(MQTT_BROKER, 1883);
  startTime=millis(); // Initialize
}


void MQTT_publish(String Topic, String Message)
{
  char ZwischenspeicherA[100], ZwischenspeicherB[100];
  Topic = String(MQTT_TOPIC_OUT) + Topic;
  Topic.toCharArray(ZwischenspeicherA, 100);
  Message.toCharArray(ZwischenspeicherB, 100);  
  MQTT_Client.publish(ZwischenspeicherA, ZwischenspeicherB);
}




// send serial communication
void communicate (String s) {
  digitalWrite(2, 1);
  int hexcount, i, checksumlenght;
  String checksum, checksumshort;
  s = "$" + s;
  hexcount = 0;
  //Serial.print("-----");
  //Serial.println();
  //Serial.print(s);
  //Serial.println();
  for (i = 0; i < s.length(); i++) {
    int is = (int)s.charAt(i);
    hexcount = hexcount + is;
  }
  checksum = (String(hexcount, HEX));
  checksumlenght = String(checksum).length();
  checksumshort = String(checksum).substring(checksumlenght - 2, checksumlenght);
  checksumshort.toUpperCase();
  while (Serial.available()) {
    // clear serial answers
    Serial.print(char(Serial.read())); 
  }
  Serial.print(s + "*" + checksumshort + "\r");
  //Serial.println();
  
  //Serial.println();
  // Antwort
  String content = "";
  char character;
  //waiting for serial send buffer cleared
  delay(300);
  //Serial.flush();
  while (Serial.available()) {
    character = Serial.read();
    content.concat(character);
  }
  //content = "seriell offline";
  // send mqtt MQTT_TOPIC_OUT
  //MQTT_publish("",String(s + "*" + checksumshort));  
  MQTT_publish("",String(content));  
  //return content;
  delay(100);
  digitalWrite(2, 0);
  
}


void loop() {
  // put your main code here, to run repeatedly:
    while(WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.begin(NETZWERKNAME, PASSWORT);    
    Serial.println("WLAN-Verbindung verloren.");
    //communicate ("FP 0 0 WiFi conn.error ");
    delay(1000);
  }

  // MQTT connect
  MQTT_reconnect();
  MQTT_Client.loop();
  // get status every x-seconds
  if (millis()>startTime+10000){
    communicate ("GS");
    startTime=millis();
  }

}


void MQTT_reconnect()
{
  if (MQTT_Client.connected())
    return;
  while (!MQTT_Client.connected())
  {
    //Serial.print("Verbinde mit MQTT.");
    
    if (!MQTT_Client.connect(MQTT_CLIENT))
    {
      Serial.print("Fehler: ");
      Serial.print(MQTT_Client.state());
      Serial.println(" Neuer Versuch in 5 Sekunden.");
      //communicate ("FP 0 1                 ");
      //delay(200);
      //communicate ("FP 0 1 mqtt-err.  retry");   
      delay(2000);
    }
  }

  MQTT_Client.setCallback(MQTT_receive);
  MQTT_Client.subscribe(MQTT_TOPIC_IN);
}


void MQTT_receive(char* Topic, byte* Nutzdaten, unsigned int Laenge)
{
  String STopic = Topic;      
  char character;
  String content = "";
  int mqttlenght;
  String mqttbase;
  //Serial.println("### MQTT Message wurde empfangen ###");
  //Serial.print("Topic: "); Serial.println(STopic);
  //Serial.print("Nutzdaten: ");
    int Zahlenwert = 0;
  for(int i = 0; i < Laenge; i++)
  {
    //Serial.print(char(Nutzdaten[i]));
    character=char(Nutzdaten[i]);
    content.concat(character);
    Zahlenwert= Zahlenwert * 10 + (Nutzdaten[i]-'0'); 
  }
  //Serial.println(content);
  mqttlenght = String(MQTT_TOPIC_IN).length();
  mqttbase = String(MQTT_TOPIC_IN).substring(0, mqttlenght - 1);
  //Serial.println(mqttbase);
  if (STopic==mqttbase+"current/"){
    communicate ("SC " + content);
  } else if (STopic==mqttbase+"control/") {
    communicate (content);
  } else if (STopic==mqttbase+"special/" && content=="reconnect") {
      communicate("FS");
      digitalWrite(0, 0);
      delay(1500);
      digitalWrite(0, 1);
      delay(1000);
      communicate("FE");
  } else if (STopic==mqttbase+"special/" && content=="blink") {
      communicate ("FP 0 0 blink        ");  
      digitalWrite(2, 1);
      delay(500);
      digitalWrite(2, 0);
      delay(500);
  } else {
    communicate ("FP 0 0 " + content);  
  }
  //communicate ("FP 0 0 mqtt-rec");
}
