#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "RemoteDebug.h"
#include <SDM.h>                                                                //import SDM template library
//SDM<2400, 12, 13> sdm;                                                        //SDM120T  baud, rx pin, tx pin
//SDM<4800, 12, 13> sdm;                                                        //SDM120C baud, rx pin, tx pin
//SDM<9600, 12, 13> sdm;                                                        //SDM630  baud, rx pin, tx pin
//or without parameters (default from SDM.h will be used): 

ESP8266WebServer server(80);
const int led = 5;

SDM<2400, 12, 13, 4> sdm;
float m = 0;
char charBuf[50];
String s;

  const char* ssid = "ssid";                                                // Wifi SSID
  const char* password = "wifipass";                                          // Wifi password
  const char* mqttClientName = "sdmweb"; //will also be used hostname and OTA name

  IPAddress ip(192,168,1,25);                                                  // IP address
  IPAddress dns(192,168,1,1);                                                       // DNS server
  IPAddress gateway(192,168,1,1);                                               // Gateway
  IPAddress subnet(255,255,255,0);                                              // Subnet mask
const char* mqtt_server = "192.168.1.6";

#define mqtt_user "......"
#define mqtt_password "......"
  int MQTT_WILL_QOS = 1;                                                        // MQTT last will QoS (0,1 or 2)
  int MQTT_WILL_RETAIN = 1;                                                     // MQTT last will retain (0 or 1)

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//************* PROJECT AND VERSION **********************************************************************
//********************************************************************************************************
const char* proj_ver = "Leitor MODBUS Eastron SDM230.v.0.1 (29/05/2018)";             // Project name and version
const char* todefine = "";             // Project name and version

//************* CONFIG DEBUG *****************************************************************************
//********************************************************************************************************
RemoteDebug Debug;                                                              // Remote debug type

char wattString[7];                                                             // Variable -
char kwhString[7];                                                              // Variable -
char powreading[7];                                                              // Variable -
char amperreading[7];                                                              // Variable -
double ampe;                                                                // Variable -
double poww;                                                                // Variable -
double kwhaccum;                                                                // Variable -
double voltage;                                                                // Variable -
double frequency;                                                                // Variable -
double pfactor;                                                                // Variable -
char kwhaccumString[7];                                                         // Variable -
char voltagestring[7];                                                         // Variable -
char frequencystring[7];                                                         // Variable -
char pfactorstring[7];                                                         // Variable -
float kwhReading;                                                               // Variable -

//************* CONFIG WEBSERVER ROOT PAGE ***************************************************************
//********************************************************************************************************
String getPage(){                                                               // Create webpage content
  String page = "<!DOCTYPE html>";
  page += "<html lang='en'>";
  page +=   "<head>";
  page +=     "<meta charset='utf-8'>";
  page +=     "<meta http-equiv='X-UA-Compatible' content='IE=edge'>";
  page +=     "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page +=     "<meta http-equiv='refresh' content='15'/>";
  page +=     "<link rel='shortcut icon' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAABGdBTUEAALGPC/xhBQAAAAFzUkdCAK7OHOkAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAAAAZiS0dEAAAAAAAA+UO7fwAAAAlwSFlzAAAASAAAAEgARslrPgAAAyRJREFUaN7tmM1vDWEUxn/6QUhK4itxW6ErCQsSafEHSDStWAmxKhJKRCokysbdoHtCb22JpBo2SiyEv6FpoldYqK9ElbaKCnItZiamz7y9fWfuO23UfZJZnDnnPOc5M++8HwNllFHGf4Hqf7lmNdAHdM6i+CxwH6hyQZYDCv6VnSXxQb1rpZIdCJEF15EUxbcZ6u1LSrYS+Chkg/79tJAB3krNYWB5ErIuIZoANqYoPkAD8E1qX41LUgf8EJJ2Q1yjA8EmjtNSexKojUPaKQTPiM4Iu4HflPZhZ32OFrlfBeRFwyVb0krgjSS3SkwtME5ps1M2lD+GN/7DOCgahoAKG+IdkjgKLJaYG0Rf8foY4uuJDtGcxCyRh1TAcsiel6Sb4l8L/JKYtgRv4Lhw/CQ6zm9LTIeSmF7JVrGfir0Hb5gF6Ae6DU32+k9wHLgHbJCYHDAQsqt87jCezKDNiAHpukH8D8V/0iB+RGIKwCffF0a7xPSJf5v4+20a0OK6cOnsoGtDr0F8cPVI7CbxD4p/tfiHbRrQj2uh+PXDqpnBH77GJLZG/OPiXyT+SRVrNS05RME1oamBCbGXiv1ebB3Xj4vUeyT2OrHfib1M7C82DajAerFfir1T7A7gs4F3BG+LUCz3xQy1VZuxgbzYm8V+IPZh4cn7OXf4O432AFvwVvgAlX5uMW6trdqMOMfUD+eW+OvwFp1wzDEbYsEJ4bBZyM7aEG8nOnPoVqKb6OxQb0Puw7SV6JIY01aiwYa8Angtia0Sk/Ebc7WZGwXWiP+QaHhFjFnzMtEFRrfTLbjbTjfL/WrguWi4GIfYdKA5ZYgr9kp1EZsOJo4zRIdohpi4LiRf8ZZ+W9g2oGgEvkvulbjiAVYQPdTngVUpNlBL9FD/gYSHeoD9BiFHU2zA9Ftlb1LxAcJDKRsjL+kQyoZyYv+NMKESuAtciJmXtAHwFixnvxYh2Y/WUhqwrrnAVYfTNJB6LZfngWam/o4xNVTAW+Wb0mimVOj2o9g15Kqo6bU6PzU5xhTNs32kdI5yA/MJc/IRu0STZRNDwK65FltGGfMFfwB1P4cc2cai3gAAACV0RVh0ZGF0ZTpjcmVhdGUAMjAxNy0xMC0xM1QyMDo1OTo1MiswMDowMGf6eRAAAAAldEVYdGRhdGU6bW9kaWZ5ADIwMTctMTAtMTNUMjA6NTk6NTIrMDA6MDAWp8GsAAAAKHRFWHRzdmc6YmFzZS11cmkAZmlsZTovLy90bXAvbWFnaWNrLWZDN1pLU3MxVs1E8gAAAABJRU5ErkJggg=='>";
  page +=     "<title>";
  page +=       mqttClientName;
  page +=     "</title>";
  page +=     "<link rel='stylesheet' href='//maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'>";
  page +=     "<link rel='stylesheet' href='//maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css' integrity='sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp' crossorigin='anonymous'>";
  page +=     "<script src='//maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js' integrity='sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa' crossorigin='anonymous'></script>";
  page +=     "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.0/jquery.min.js'></script>";
  page +=   "</head>";
  page +=   "<body>";
  page +=   "<p></p>";
  page +=     "<div class='container-fluid'>";
  page +=     "<div class='row'>";
  page +=       "<div class='col-md-12'>";
  page +=         "<div class='jumbotron'>";
  page +=           "<h2>";
  page +=             proj_ver;
  page +=           "</h2>";
  page +=           "<p>This project uses a MQTT topic to store the accumulated kWh value, so that when there is a reboot, reset or power off the value won't be lost. <small><em style='color: #ababab;'>";
  page +=             todefine;
  page +=           "</em></small></p>";
  page +=         "</div>";
  page +=         "<div class='alert alert-dismissable alert-info'>";
  page +=           "<button type='button' class='close' data-dismiss='alert' aria-hidden='true'>×</button>";
  page +=           "<strong>Attention!</strong> This page auto-refreshes every 15 seconds.";
  page +=         "</div>";
  page +=       "</div>";
  page +=     "</div>";
  page +=     "<div class='row'>";
  page +=       "<div class='col-md-4'>";
  page +=         "<h3 class='text-primary'>Current Consumption</h3>";
  page +=           "<p class='lead'>";
  page +=             powreading;
  page +=           " Wh</p>";
  page +=         "</div>";
  page +=       "<div class='col-md-4'>";
  page +=         "<h3 class='text-primary'>Current Amperage</h3>";
  page +=           "<p class='lead'>";
  page +=             amperreading;
  page +=           " A</p>";
  page +=         "</div>";
  page +=       "<div class='col-md-4'>";
  page +=         "<h3 class='text-primary'>Accum Consumption</h3>";
  page +=           "<p class='lead'>";
  page +=             kwhaccumString;
  page +=           " kWh (total)</p>";
  page +=         "</div>";
  page +=       "</div>";
  page +=       "<div class='row'>";
  page +=       "<div class='col-md-4'>";
  page +=         "<h4 class='text-primary'>Frequency</h3>";
  page +=           "<p class='lead'>";
  page +=             frequencystring;
  page +=           " Hz</p>";
  page +=         "</div>";
  page +=       "<div class='col-md-4'>";
  page +=         "<h4 class='text-primary'>Voltage/Tension</h3>";
  page +=           "<p class='lead'>";
  page +=             voltagestring;
  page +=           " V</p>";
  page +=         "</div>";
  page +=       "<div class='col-md-4'>";
  page +=         "<h4 class='text-primary'>Power Factor</h3>";
  page +=           "<p class='lead'>";
  page +=             pfactorstring;
  page +=           " </p>";
  page +=       "</div>";
  page +=       "<div class='row'>";
  page +=         "<div class='col-md-12'>";
  page +=           "<div class='alert alert-dismissable alert-danger'>";
  page +=             "<button type='button' class='close' data-dismiss='alert' aria-hidden='true'>×</button>";
  page +=           "<h5>Warning!</h4>Sometimes the board does not restart correctly. If you can't get back to this page after one minute, you need to reset on the board directly on the fisical reset button.&nbsp;&nbsp;";
  page +=           "<button type='button' class='btn btn-warning btn-default'>RESET</button>";
  page +=         "</div>";
  page +=     "</div>";
  page +=   "</div>";
  page +=     "<div class='col-md-12'>";
  page +=       "copyright <br/><br/>";
  page +=     "</div>";
  page += "</div>";
  page +=   "</body>";
  page += "</html>";
  return page;
}

void handleRoot() {
  digitalWrite(led, 1);
  server.send ( 200, "text/html", getPage() );
  digitalWrite(led, 0);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.begin(115200);                                                         //initialize serial
  sdm.begin();                                                                  //initalize SDM220 communication baudrate

 //----------- OTA
  ArduinoOTA.setHostname(mqttClientName);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    delay(1000);
    ESP.restart();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

    if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  Debug.println("HTTP server started");                                         // Send text to telnet debug interface

  Debug.begin(mqttClientName);                                                 // Start Telnet server
}

void setup_wifi() {
  delay(10);

  /** TELNET **/
  Debug.begin(mqttClientName);                                                       // Initiaze the telnet server
  Debug.setResetCmdEnabled(true);                                               // Enable/disable (true/false) the reset command (true/false)
  Debug.showTime(false);                                                        // Enable/disable (true/false) timestamps
  Debug.showProfiler(false);                                                    // Enable/disable (true/false) Profiler - time between messages of Debug
  Debug.showDebugLevel(false);                                                  // Enable/disable (true/false) debug levels
  Debug.showColors(true);                                                       // Enable/disable (true/false) colors

  
  // config static IP
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
      WiFi.config(ip, gateway, subnet, dns);
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

 Debug.println("- - - - - - - - - - - - - - - - - - - - - - - - - - -");       // Block separator to telnet debug interface
 Debug.println(proj_ver);                                                      // Send project name and version to telnet debug interface
 Debug.println("- - - - - - - - - - - - - - - - - - - - - - - - - - -");       // Block separator to telnet debug interface
 Debug.println();                                                              // Send space to telnet debug interface
 Debug.println("WiFi connected");                                              // Send successful connection to telnet debug interface
 Debug.printf("IP address is "); Debug.println(WiFi.localIP());                // Send IP address to telnet debug interface

  
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Debug.println();                                                              // Block space to telnet debug interface
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {

  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    Debug.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("SDM120",mqtt_user,mqtt_password,"sdm120/status", 2, 0, "0")) {
      Serial.println("connected");
      Debug.print("connected to mqtt");
      // Once connected, publish an announcement...
      client.publish("sdm120/status", "1");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Debug.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      Debug.print(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  server.handleClient();
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //handle OTA
  ArduinoOTA.handle();
  
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    
delay(5000);
client.loop();
    
    s = String(sdm.readVal(SDM220T_VOLTAGE));
    if (s != "nan") {
      s.toCharArray(charBuf, 50);
      client.publish("sdm120/volt", charBuf);
      Serial.print("Voltage:   ");
      Serial.print(s);
      Serial.println(" V");
      voltage = sdm.readVal(SDM220T_VOLTAGE);
      Debug.println("voltage read");
    }
    delay(50);

    s = String(sdm.readVal(SDM220T_CURRENT));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/curr", charBuf);
    Serial.print("Current:   ");    
    Serial.print(s);
    Serial.println(" A");
    ampe = sdm.readVal(SDM220T_CURRENT);
    }
    delay(50);

    s = String(sdm.readVal(SDM220T_POWER));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/pow", charBuf);
    Serial.print("Power:   ");    
    Serial.print(s);
    Serial.println(" W");
    poww = sdm.readVal(SDM220T_POWER);
    }
    delay(50);   
  
    s = String(sdm.readVal(SDM220T_ACTIVE_APPARENT_POWER));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/act_app_pow", charBuf);
    Serial.print("Active apparent power:   ");    
    Serial.print(s);
    Serial.println(" VA");
    }
    delay(50);   

    s = String(sdm.readVal(SDM220T_REACTIVE_APPARENT_POWER));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/react_app_pow", charBuf);
    Serial.print("Active apparent power:   ");    
    Serial.print(s);
    Serial.println(" VAR");
    }
    delay(50);   
  
    s = String(sdm.readVal(SDM220T_POWER_FACTOR));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/pow_factor", charBuf);
    Serial.print("Power factor:   ");    
    Serial.print(s);
    Serial.println(" ");
    pfactor = sdm.readVal(SDM220T_POWER_FACTOR);
    }
    delay(50);   
  
    s = String(sdm.readVal(SDM220T_PHASE_ANGLE));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/phase_angle", charBuf);
    Serial.print("Phase angle:   ");    
    Serial.print(s);
    Serial.println(" Degree");
    }
    delay(50);     

    s = String(sdm.readVal(SDM220T_FREQUENCY));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/freq", charBuf);
    Serial.print("Frequency:   ");    
    Serial.print(s);
    Serial.println(" Hz");
    frequency = sdm.readVal(SDM220T_FREQUENCY);
    }
    delay(50);    

    s = String(sdm.readVal(SDM220T_TOTAL_ACTIVE_ENERGY));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/tot_act_en", charBuf);
    Serial.print("Total active energy:   ");    
    Serial.print(s);
    Serial.println(" Wh");
    kwhaccum = sdm.readVal(SDM220T_TOTAL_ACTIVE_ENERGY);
    }
    delay(50);    

    s = String(sdm.readVal(SDM220T_TOTAL_REACTIVE_ENERGY));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/tot_react_en", charBuf);
    Serial.print("Total reactive energy:   ");    
    Serial.print(s);
    Serial.println(" Wh");
    }
    delay(50);    

    s = String(sdm.readVal(SDM220T_IMPORT_ACTIVE_ENERGY));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/import_act_en", charBuf);
    Serial.print("Import active energy:   ");    
    Serial.print(s);
    Serial.println(" Wh");
    }
    delay(50);    

    s = String(sdm.readVal(SDM220T_EXPORT_ACTIVE_ENERGY));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/export_act_en", charBuf);
    Serial.print("Export active energy:   ");    
    Serial.print(s);
    Serial.println(" Wh");
    }
    delay(50);    

    s = String(sdm.readVal(SDM220T_IMPORT_REACTIVE_ENERGY));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/import_react_en", charBuf);
    Serial.print("Import reactive energy:   ");    
    Serial.print(s);
    Serial.println(" VARh");
    }
    delay(50); 

    s = String(sdm.readVal(SDM220T_EXPORT_REACTIVE_ENERGY));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/export_react_en", charBuf);
    Serial.print("Export reactive energy:   ");    
    Serial.print(s);
    Serial.println(" VARh");


    
    }
    dtostrf(ampe, 5, 2, amperreading);                                  
    dtostrf(poww, 5, 2, powreading);                                  
    dtostrf(kwhaccum, 5, 2, kwhaccumString);                                  // Convert Accum kWh to string
    dtostrf(voltage, 5, 2, voltagestring);       
    dtostrf(frequency, 5, 2, frequencystring);       
    dtostrf(pfactor, 5, 2, pfactorstring);       
    Debug.println("Amperes"); Debug.println(amperreading);
    Debug.println("Consumo instant w"); Debug.println(powreading);
    Debug.println("Acumlado Kwh"); Debug.println(kwhaccumString);
    Debug.println("Voltage"); Debug.println(voltagestring);
    Debug.println("Frequency"); Debug.println(frequencystring);
    Debug.println("Power Factor"); Debug.println(pfactorstring);
    Debug.handle();                                                               // Remote debug over telnet
        yield(); 

    Serial.println("----------------------------------------");
  }
  //wait a while before next loop
}
