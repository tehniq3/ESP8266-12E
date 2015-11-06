// https://forum.arduino.cc/index.php?topic=353502.0
// https://github.com/anakod/ESP8266pro
// http://nicuflorica.blogspot.ro/
// http://arduinotehniq.blogspot.com/
// version 1m5_shield by niq_ro with DS18B20 & DHT11

//#include <SoftwareSerial.h> 
#include <ESP8266pro.h> 
#include <ESP8266proServer.h> 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

#define DHTPIN 8     // what pin we're connected the DHT output
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22 
DHT dht(DHTPIN, DHTTYPE);

// long int interval = 21600000; // 6 hours
long int kil = 3600000; // 1 hour
long int taim = 0;
int taim2 = 0;
int taim3 = 0;
int taim4 = 0;
#define rezet 7   // pin for reset
float te;

#define DEBUG true

//const int  RX=10;     
//const int  TX=11;     
const int  ts=9;     //temp sensor DS18B20
const int  il=13;    //indicator led
const int  ms=12;    //master switch relay

  #define ONE_WIRE_BUS ts
  OneWire oneWire(ONE_WIRE_BUS);
  DallasTemperature sensors(&oneWire);
         
const int   TN = 1400;                  //tone
//float t;            //temperature  
 
unsigned long previousMillis = 0;         //check sensors
const long interval = 60000;               //Sampling interval time mSec

const long COM_BAUD = 115200;                       //ESP UART baudrate
const String mode = "3";                           //1 - Client, 2 - Access Point, 3 - Both                             
const String ssid = "SSID";       //Network SSID
const String password = "pasword";              //Network Password
//const String apipaddress = "192.168.4.1";          //Desired IP address
//const String cipaddress = "192.168.0.201";          //Desired IP address
//const String subnetworkmask = "255.255.255.0";  
//const String gateway = "192.168.0.1";

//SoftwareSerial esp8266(RX, TX); // RX, TX   

//ESP8266pro wifi(esp8266, Serial); 
ESP8266pro wifi(Serial2, Serial); 
ESP8266proServer server(wifi, onClientRequest); 
String requestPaths[ESP_MAX_CONNECTIONS];     

void setup() {
  digitalWrite(rezet, HIGH); //We need to set it HIGH immediately on boot
  pinMode(rezet,OUTPUT);     //We can declare it an output ONLY AFTER it's HIGH
                         // (( HACKHACKHACKHACK ))
  
    dht.begin();   
 //   pinMode(7, INPUT);        // switch is attached to Arduino pin 7
 //   pinMode(8, INPUT);        // switch is attached to Arduino pin 8 
    pinMode(ts, INPUT);
    pinMode(ms, OUTPUT);
    pinMode(il, OUTPUT);     
//    esp8266.begin(COM_BAUD);
   Serial2.begin(COM_BAUD);
    Serial.begin(COM_BAUD);    //for debug

  sendData("AT+RST\r\n",3400,DEBUG); // reset module
  sendData("AT+GMR\r\n",2000,DEBUG); // Sofware version
  sendData("AT+CWMODE="+mode+"\r\n",1000,DEBUG); // configure as access point
  sendData("AT+CWJAP=\""+ssid+"\",\""+password+"\"\r\n",10000,DEBUG); // connect to AP
  sendData("AT+CIFSR\r\n",2000,DEBUG); // set ip address
//  sendData("AT+CIPSTA_DEF=\""+cipaddress+"\",\""+gateway+"\",\""+subnetworkmask+"\"\r\n",1000,DEBUG); // set ip address
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  //sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80 
  
  // Start server on port 80
  server.start(80);
  Serial.println();
  Serial.println("=================================");
  if(mode=="1"){
//  Serial.println("Server started: http://" + cipaddress);
  }
/*
  if(mode=="2"){
  Serial.println("Server started: http://" + apipaddress); 
  }
  if(mode=="3"){
  Serial.println("Server started:");
  Serial.println("Access Point: http://" + apipaddress);
  Serial.println("Client: http://" + cipaddress); 
  }
*/
  Serial.println("================================="); 

  sensors.begin();  //for dallas temp sensor
 // 9 bit resolution by default 
  // Note the programmer is responsible for the right delay
  // we could do something usefull here instead of the delay
  int resolution = 11;
  sensors.setResolution(resolution);
  delay(750/ (1 << (12-resolution)));
  checkSensors();   //check all available sensors and save values to RAM
  
  digitalWrite(il,HIGH);   
  delay(1000);   
  digitalWrite(il,LOW);
  
             
 }
    
    void loop() {                                                                                  // Process incoming requests   
                 server.processRequests();
                 delay(100);  
            
  taim = millis();
  if(taim > kil){
  Serial.println("RESET!");
  digitalWrite(rezet, LOW); //Pulling the RESET pin LOW triggers the reset.
  }     
                }
        
        void onClientRequest(ESP8266proConnection* connection,           
        char* buffer, int length, boolean completed) {
          Serial.print(buffer);
          if (strncmp(buffer, "GET ", 4) == 0)   {    
            // We found GET HTTP request
                 
            char* p = strstr(buffer + 4, " ");     
            *p = '\0'; // erase string after path     
            requestPaths[connection->getId()] = (String)((char*)buffer + 4);   }      
            if (completed)   {     String path = requestPaths[connection->getId()];
       //if (path == "/")
       if (path.startsWith("/ajax_switch&nocache="))      
              {
              Serial.println("AJAX request received"); 
    
         /*       if (digitalRead(7)) {
                connection->send(F("<p>Switch 7 state: ON</p>"));
                }
                else {
                connection->send(F("<p>Switch 7 state: OFF</p>"));
                }
         */
         /*      if (digitalRead(8)) {
                connection->send(F("<p>Switch 8 state: ON</p>"));
                }
              else {
                connection->send(F("<p>Switch 8 state: OFF</p>"));
                }
           */     
           /*   // read analog pin A2
               int analog_val = analogRead(2);
               connection->send(F("<p>Analog A2: "));
               connection->send(String(analog_val));
               connection->send(F("</p>"));
           */

int h = dht.readHumidity();
//float te = dht.readTemperature();  
checkSensors(); 
int te2 = te*10;

  connection->send(F("<h2>humidity = "));
  connection->send(String(h));
  connection->send(F(" % RH <br> temperature = "));
if (te2 > 0) connection->send(F("+"));
if (te2 < 0)
    {
    te2 = -te2;
    connection->send(F("+"));
    }
 int te21 = te2/10;
 int te22 = te2 - 10*te21;

  connection->send(String(te21));
  connection->send(F(","));
  connection->send(String(te22));
  connection->send(F("<sup>o</sup>C <br>"));
  connection->send(F("<h3><p><p>"));
/*  connection->send(F("  Up time: "));
  connection->send(String(millis()/60000));
  connection->send(F(" minute(s)..."));
*/
  // time untill reset
       connection->send(F("<br>I'll reset in "));
       taim2 = (kil-taim)/1000; // all seconds
       taim3 = taim2/60;  // minutes
       taim4 = taim2 - 60*taim3;
       connection->send(String(taim3));
       if (taim4 < 10) connection->send(F(":0"));
       else connection->send(F(":"));
       connection->send(String(taim4));
       connection->send(F(" minutes.."));                 
            }            
       else if (path == "/"){
             connection->send(F(
                "HTTP/1.1 200 OK\r\n\r\n"
                "<!DOCTYPE html>\r\n"
                "<html>\r\n"         
                "<head>\r\n"                  
                "<title>Arduino Web Page</title>\r\n" 
                "<meta name='viewport' content='width=200px'/>\r\n"
"<style>h1{text-align: center;font-family:Arial, 'Trebuchet MS', Helvetica, sans-serif;}"
"h2{text-align: center;font-family:Arial, 'Trebuchet MS', Helvetica, sans-serif;}"
"a{text-decoration:none;width:75px;height:50px;border-color: black;border-top:2px solid;"
"border-bottom:2px solid;border-right:2px solid;border-left:2px solid;"
"border-radius:10px 10px 10px;-o-border-radius:10px 10px 10px;-webkit-border-radius:10px 10px 10px;"
"font-family:'Trebuchet MS',Arial, Helvetica, sans-serif;-moz-border-radius:10px 10px 10px;"
"background-color: #68a2d1;padding:8px;text-align:center;}"
"a:link {color: white;} a:visited {color: blue;} a:hover {color: yellow;}"
"a:active {color: red;} </style>"
                "<script>\r\n"
                "function GetSwitchAnalogData() {\r\n"
                "nocache = \"&nocache=\" + Math.random() * 1000000;\r\n"
                "var request = new XMLHttpRequest();\r\n"
                "request.onreadystatechange = function() {\r\n"
                "if (this.readyState == 4) {\r\n"
                "if (this.status == 200) {\r\n"
                "if (this.responseText != null) {\r\n"
                "document.getElementById(\"sw_an_data\").innerHTML = this.responseText;\r\n" 
                "}}}}\r\n"
                "request.open(\"GET\", \"ajax_switch\" + nocache, true);\r\n"
                "request.send(null);\r\n"
                "setTimeout('GetSwitchAnalogData()', 15000);\r\n"
                "}\r\n"
                "</script>\r\n"                       
                "</head>\r\n"           
                "<body onload=\"GetSwitchAnalogData()\">\r\n"  
                "<body style=background:#A0B0B0>"   
                "<center><h1>Arduino AJAX Input</h1>\r\n" 
                "<h4>(sensors: DS18B20 for temperature & DHT11 for humidity)</h4>"
                "<font color=white>"
                "<div id=\"sw_an_data\">\r\n"
                "</div>\r\n"  
                "<H4>Sketch by Nicu Florica aka niq_ro.<p />"
                "<a href=http://arduinotehniq.blogspot.com target=blank>http://arduinotehniq.blogspot.com</a>"  
    //            "<button type=\"button\" onclick=\"http://arduinotehniq.blogspot.com\">http://arduinotehniq.blogspot.com</button>"
                // http://www.computerhope.com/issues/ch000076.htm
    //            "<FORM><INPUT Type=\"BUTTON\" Value=\"http://arduinotehniq.blogspot.com\" Onclick=\"window.location.href='http://arduinotehniq.blogspot.com' target=_blanck\"></FORM>"
                "</body>\r\n"          
                "</html>\r\n"));               
               }
      
            
           delay(10);  
           connection->close();   
           } 
       }

                                                      
void checkSensors(){
  
                      sensors.requestTemperatures();     // Send the command to get temperatures
                      te = sensors.getTempCByIndex(0);
                      Serial.print("Temperature = ");   
                      Serial.print(te);   
                      Serial.println(" Celsius");                      
                   } 
/*
void masterswitchOn(){
                      digitalWrite(ms,HIGH); 
                     }
void masterswitchOff(){
                      digitalWrite(ms,LOW); 
                      }       
*/          
           

String sendData(String command, const int timeout, boolean debug)
     {
    String response = "";
    
 //   esp8266.print(command); // send the read character to the esp8266
       Serial2.print(command);
       
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
//      while(esp8266.available())
      while(Serial2.available())
      {
        
        // The esp has data so display its output to the serial window 
  //      char c = esp8266.read(); // read the next character.
        char c = Serial2.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}
