# OpenEVSE IoT remote control

German description of openEVSE RAPI commands.
http://elektrischezukunft.blogspot.co.at/2015/03/openevse-fernzugriff.html

## ESP_Webserver (http-based)
old stuff
Based on Webserver - Coding Stefan Thesen 04/2015
https://blog.thesen.eu/stabiler-http-1-1-wlan-webserver-mit-dem-esp8266-microcontroller/

no leading $ or tailing hex checksums required.

http:/<ip>/?control=**FS** switch OpenEVSE in sleep mode...

## ESP8266_mqtt
Mqtt-based communication 
Source: "Projekte mit Arduino und ESP" Danny Schreiter https://bmu-verlag.de/


Tested with OpenEVSE 3.7.8
see more on [unifox.at](http://www.unifox.at/iot-openevse/)
