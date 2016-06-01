/*--------------------------------------------------
HTTP 1.1 Webserver for ESP8266 
for ESP8266 adapted Arduino IDE
based on https://blog.thesen.eu/stabiler-http-1-1-wlan-webserver-mit-dem-esp8266-microcontroller/
--------------------------------------------------*/
#include <ESP8266WiFi.h>

const char* ssid = "YOURSSID";
const char* password = "YOURPASSWORD";


unsigned long ulReqcount;
unsigned long ulReconncount;
boolean ignoreremote = false;

// Create an instance of the server on Port 80
WiFiServer server(80);

void setup() 
{
  // setup globals
  ulReqcount=0; 
  ulReconncount=0;
  
  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 1);
  ignoreremote = false;
  
  // start serial
  Serial.begin(115200);
  delay(1);
  
  // inital connect
  WiFi.mode(WIFI_STA);
  WiFiStart();
}

void WiFiStart()
{
  ulReconncount++;
  
  // Connect to WiFi network
  //Serial.println();
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  //Serial.println("");
  //Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  //Serial.println("Server started");

  // Print the IP address
  //Serial.println(WiFi.localIP());
}

// read serial commuinication


// send serial communication
int communicate (String s) {
  int hexcount, i,checksumlenght;
  String checksum, checksumshort;
  s="$"+s;
  hexcount=0;
  //Serial.print("-----");
  //Serial.println();
  //Serial.print(s);
  //Serial.println();
  for (i = 0; i < s.length(); i++) {
    int is = (int)s.charAt(i);
    hexcount=hexcount+is;
  }
  checksum=(String(hexcount, HEX));
  checksumlenght=String(checksum).length();
  checksumshort=String(checksum).substring(checksumlenght-2, checksumlenght);
  checksumshort.toUpperCase();
  Serial.print(s + "*" + checksumshort);
  Serial.println(); 
}



void loop() 
{

  // check if WLAN is connected
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFiStart();
  }
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }
  
  // Wait until the client sends some data
  //Serial.println("new client");
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout) 
  { 
    //Serial.println("client connection time-out!");
    return; 
  }
  
  // Read the first line of the request
  String sRequest = client.readStringUntil('\r');
  //Serial.println(sRequest);
  client.flush();
  
  // stop client, if request is empty
  if(sRequest=="")
  {
    //Serial.println("empty request! - stopping client");
    client.stop();
    return;
  }
  
  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
  String sPath="",sParam="", sCmd="", str_begin="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);
    
    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // output parameters to serial, you may connect e.g. an Arduino and react on it
  ///////////////////////////////////////////////////////////////////////////////
  if(sParam.length()>0)
  {
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {
      sCmd = sParam.substring(iEqu+1,sParam.length());
    }
  }
  
  
  ///////////////////////////
  // format the html response
  ///////////////////////////
  String sResponse,sHeader;
  
  ////////////////////////////
  // 404 for non-matching path
  ////////////////////////////
  if(sPath!="/")
  {
    sResponse="<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p>ESP8266 OpenEvse IOT by datenschuft</body></html>";
    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  ///////////////////////
  // format the html page
  ///////////////////////
  else
  {
    //ulReqcount++;
    sResponse  = "<html><head><title>OpenEvse IOT</title></head><body>";
    sResponse += "<font color=\"#000000\"><body bgcolor=\"#E1E9F0\">";
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<h1>Control your Openevse</h1>";
    sResponse += "<FONT SIZE=+1>";
    //sResponse += "<p>Funktion 1 <a href=\"?pin=FUNCTION1ON\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=FUNCTION1OFF\"><button>ausschalten</button></a></p>";
    //sResponse += "<p>Funktion 2 <a href=\"?pin=FUNCTION2ON\"><button>einschalten</button></a>&nbsp;<a href=\"?pin=FUNCTION2OFF\"><button>ausschalten</button></a></p>";
    sResponse += "<p><a href=\"?control=EVSE\"><button>Evse-Status</button></a></p>";
    sResponse += "<p><a href=\"?control=noauto\"><button>disable remote control</button></a></p>";
    sResponse += "<p><a href=\"?control=auto\"><button>enable remote control</button></a></p>";
    
    //////////////////////
    // react on parameters
    //////////////////////
    if (sCmd.length()>0)
    {
      // write received command to html page
      String csCmd = sCmd;
      csCmd.replace("%20", " ");
      sResponse += "Empfangenes Kommando:" + csCmd + "(" + sCmd + ")<BR>";
      
    if(sCmd.indexOf("EVSE")>=0)
      {
        communicate ("GS");
         String content = "";
          char character;
        
          while(Serial.available()) {
              character = Serial.read();
              content.concat(character);
          }
          sResponse += "serial get:" + content + "<BR>";
      }
      else if (sCmd.indexOf("noauto")>=0)
      {
        ignoreremote = true;
        digitalWrite(2, 0);
      }
      else if (sCmd.indexOf("auto")>=0)
      {
        ignoreremote = false;
        digitalWrite(2, 1);
      }      
      else 
      {
        if (ignoreremote == true) {
          sCmd = "NOOP";
        } else {
        //communicate (sCmd);
         String content = "";
         
         communicate (csCmd);
          char character;
        
          while(Serial.available()) {
              character = Serial.read();
              content.concat(character);
          }
          sResponse += "serial get:" + content + "<BR>";
        }
      }
    }
    
    sResponse += "<FONT SIZE=-2>";
    //sResponse += "<BR>Aufrufz&auml;hler="; 
    //sResponse += ulReqcount;
    sResponse += " - Verbindungsz&auml;hler="; 
    sResponse += ulReconncount;
    sResponse += "<BR>";
    if (ignoreremote == false) {
      sResponse += "remote control enabled";
    } else {
        sResponse += "remote control ignored";
    }
    sResponse += "<BR>";
    sResponse += "Wenger Florian";
    sResponse += "</body></html>";
    
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  
  // Send the response to the client
  client.print(sHeader);
  client.print(sResponse);
  
  // and stop the client
  client.stop();
  Serial.println("Client disonnected");
}
