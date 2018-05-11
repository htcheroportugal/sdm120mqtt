#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SDM.h>                                                                //import SDM template library
//SDM<2400, 12, 13> sdm;                                                        //SDM120T  baud, rx pin, tx pin
//SDM<4800, 12, 13> sdm;                                                        //SDM120C baud, rx pin, tx pin
//SDM<9600, 12, 13> sdm;                                                        //SDM630  baud, rx pin, tx pin
//or without parameters (default from SDM.h will be used): 
SDM<> sdm;
float m = 0;
char charBuf[50];
String s;

const char* ssid = "your nework name";
const char* password = "password";
const char* mqtt_server = "192.168.1.10";

#define mqtt_user "YOUR_MQTT_USERNAME"
#define mqtt_password "YOUR_MQTT_PASSWORD"

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.begin(115200);                                                         //initialize serial
  sdm.begin();                                                                  //initalize SDM220 communication baudrate
}

void setup_wifi() {
  delay(10);
  // config static IP
  IPAddress ip(192, 168, 1, 43); // where xx is the desired IP Address
  IPAddress gateway(192, 168, 1, 1); // set gateway to match your network
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your
  WiFi.config(ip, gateway, subnet);
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
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
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
    // Attempt to connect
    if (client.connect("SDM120",mqtt_user,mqtt_password,"sdm120/status", 2, 0, "0")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("sdm120/status", "1");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    
    s = String(sdm.readVal(SDM220T_VOLTAGE));
    if (s != "nan") {
      s.toCharArray(charBuf, 50);
      client.publish("sdm120/volt", charBuf);
      Serial.print("Voltage:   ");
      Serial.print(s);
      Serial.println(" V");
    }
    delay(50);

    s = String(sdm.readVal(SDM220T_CURRENT));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/curr", charBuf);
    Serial.print("Current:   ");    
    Serial.print(s);
    Serial.println(" A");
    }
    delay(50);

    s = String(sdm.readVal(SDM220T_POWER));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/pow", charBuf);
    Serial.print("Power:   ");    
    Serial.print(s);
    Serial.println(" W");
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
    }
    delay(50);    

    s = String(sdm.readVal(SDM220T_TOTAL_ACTIVE_ENERGY));
    if (s != "nan") {
    s.toCharArray(charBuf, 50);
    client.publish("sdm120/tot_act_en", charBuf);
    Serial.print("Total active energy:   ");    
    Serial.print(s);
    Serial.println(" Wh");
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
    delay(50);

    Serial.println("----------------------------------------");

  }
  //wait a while before next loop
}
