// https://forum.arduino.cc/index.php?topic=353502.0
// https://github.com/anakod/ESP8266pro
// http://nicuflorica.blogspot.ro/
// http://arduinotehniq.blogspot.com/
// http://www.arduinotehniq.com/ - new site
// version 1m9b_shield by niq_ro with DHT22
// thx - Sorin from https://github.com/nekhbet

//#include <SoftwareSerial.h> 
#include <ESP8266pro.h> 
#include <ESP8266proServer.h> 
#include <DHT.h>
#include <EEPROM.h> // http://tronixstuff.com/2011/03/16/tutorial-your-arduinos-inbuilt-eeprom/


#define DHTPIN 8     // what pin we're connected the DHT output
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22 
DHT dht(DHTPIN, DHTTYPE);

// long int interval = 21600000; // 6 hours
//long int kil = 3600000; // 1 hour
long int taim = 0;
int taim2 = 0;
int taim3 = 0;
int taim4 = 0;
long int taim5 = 0;

//#define rezet 7   // pin for reset
float te;
int tz;
int teset01, teset02;

float teset;   // set  temperature
float teset2;       // value for numerical data
float dete = 0.5;  // hysteressis
#define releu 13  // define the pin for relay control

int citire = 0;   // for read sensor with pause

#define DEBUG true

//const int  RX=10;     
//const int  TX=11;     
const int  ts=9;     //temp sensor DS18B20
const int  il=13;    //indicator led
const int  ms=12;    //master switch relay

float t;            //temperature  
 
unsigned long previousMillis = 0;         //check sensors
const long interval = 60000;               //Sampling interval time mSec

const long COM_BAUD = 115200;                       //ESP UART baudrate
const String mode = "1";                           //1 - Client, 2 - Access Point, 3 - Both                             
const String ssid = "SSID";       //Network SSID
const String password = "password";              //Network Password

//SoftwareSerial esp8266(RX, TX); // RX, TX   

//ESP8266pro wifi(esp8266, Serial); 
ESP8266pro wifi(Serial2, Serial); 
ESP8266proServer server(wifi, onClientRequest); 
String requestPaths[ESP_MAX_CONNECTIONS];     

void setup() {
  //  digitalWrite(rezet, HIGH); //We need to set it HIGH immediately on boot
  // pinMode(rezet,OUTPUT);     //We can declare it an output ONLY AFTER it's HIGH
           // (( HACKHACKHACKHACK ))

  dht.begin();   
  //   pinMode(7, INPUT);        // switch is attached to Arduino pin 7
  //   pinMode(8, INPUT);        // switch is attached to Arduino pin 8 
  pinMode(ts, INPUT);
  pinMode(ms, OUTPUT);
  pinMode(il, OUTPUT);     
  //    esp8266.begin(COM_BAUD);
  Serial2.begin(COM_BAUD);
  Serial.begin(19200);    //for debug

  pinMode(releu, OUTPUT);

  //  sendData("AT+RESTORE\r\n",5000,DEBUG); // reset module
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

  digitalWrite(releu,LOW);   

  digitalWrite(il,HIGH);   
  delay(1000);   
  digitalWrite(il,LOW);

/*
EEPROM.write(600,23);   // write set temoerature
*/
/*
EEPROM.write(601,23);
EEPROM.write(602,5);
*/

// teset = EEPROM.read(600);  // read set temperature
  teset01 = EEPROM.read(601);
  teset02 = EEPROM.read(602);
  teset = 10*teset01 + teset02;
  teset = teset/10;
}
    
    void loop() {                                                                                  // Process incoming requests   
                 server.processRequests();
                 delay(100);  
            
 
/*  
 if(taim > kil){
  Serial.println("RESET!");
  digitalWrite(rezet, LOW); //Pulling the RESET pin LOW triggers the reset.
  }     
*/

// if webpage is without client(s)
 if (citire == 0)   // must read temperature 
 {
 taim5 = millis();  // note time reading
 faranet (teset, dete);  // read temperature and control relay)
 citire = 1;        // note that (temperature was read)
 }
 if (millis() > taim5 + 15000) // if time between sensor reading is too lonf 
{
 citire = 0;        // must read again the temperature
 Serial.println("!");
}
/*
Serial.print("citire = ");  
Serial.print(citire);
Serial.print("   taim5 = ");
Serial.print(taim5);
Serial.print("  millis() = ");
Serial.println(millis());
*/
                  }  // end of main loop

        
        void onClientRequest(ESP8266proConnection* connection, char* buffer, int length, boolean completed) {
      Serial.print(buffer);
      if (strncmp(buffer, "GET ", 4) == 0)
      {    
        // We found GET HTTP request

        char* p = strstr(buffer + 4, " ");     
        *p = '\0'; // erase string after path     
        requestPaths[connection->getId()] = (String)((char*)buffer + 4);
      }      
            if (completed)
      {
        String path = requestPaths[connection->getId()];
      // Thermostat temperature path
        if (path.indexOf("change") > 0)
        {
          Serial.println(" AJAX request received: change thermostat temperature");
          Serial.println("-");
          Serial.println(path);

          // Set the new temperature
  //         if (path.indexOf("&done") > 0)
 //         {
            int index1;
            int index2;
            
            String test;
            String test01, test02;
            
            test01 = path.substring(17, 19);
            test02 = path.substring(20, 21);
            Serial.print("value = ");
            Serial.print(test01);
            Serial.print(",");
            Serial.println(test02);
            String new_value;
        //    teset = test.toInt();
        //    EEPROM.write(600,teset);
            teset01 = test01.toInt();
            teset02 = test02.toInt();
            teset = 10*teset01 + teset02;
            teset = teset/10;
            EEPROM.write(601,teset01);
            EEPROM.write(602,teset02);
            Serial.print(" / ");
            Serial.println(teset);
            
            /*
            index1 = path.indexOf('new_temp=');
            tz = index1;
            index2 = path.indexOf('&done');
            new_value = path.substring(index1+9, index2);
            Serial.print("index1: ");
              Serial.println(index1);
            Serial.print("index2: ");
              Serial.println(index2);
            Serial.println(" New value from the form: ");
            Serial.println(new_value);
            */
 //         }
        }
        else if (path.indexOf("ajax_switch&nocache=") > 0)      
        {
          Serial.println(" AJAX request received: ajax switch"); 
          int h = dht.readHumidity();
          te = dht.readTemperature();  
          int te2 = te*10;
          connection->send(F("<h2>humidity = "));
          connection->send(String(h));
          connection->send(F(" % RH <br> temperature = "));
          if (te2 > 0)
          {
            connection->send(F("+"));
          }
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

          connection->send(F("<font color=gray>"
          "set temperature = "));
   //       teset = tz;
          teset2 = teset;     
          if (teset > 0)
          {
            connection->send(F("+"));
          } 
          if (teset < 0)
          {
            teset2 = -teset;
            connection->send(F("-"));
          }
          connection->send(String(teset2,1));
          connection->send(F("<sup>o</sup>C"));
          //    connection->send(String(tz));

          connection->send(F("<br>Temperature is "));
          if (te > teset + dete/2) 
          {
            Serial.println("Temperature is high..");
            connection->send(F("<font color=red> HIGH !"));
            digitalWrite(releu,HIGH);   // cooler is ON (powered)
          }
          if (te < teset - dete/2) 
          {
            Serial.println("Temperature is too low.. ");
            connection->send(F("<font color=blue> LOW !"));
            digitalWrite(releu,LOW);   // cooler is ON (powered)
          }

          if ((te >= teset - dete/2) and (te <= teset + dete/2))
          {
            Serial.println("Temperature is ok.. ");
            connection->send(F("<font color=green> OK !"));
          }


          // time untill reset
          //       connection->send(F("<br>I'll reset in "));
          connection->send(F("<font color=gray> <br>Up-time = "));
          //       taim2 = (kil-taim)/1000; // all seconds
          taim = millis(); 
          taim2 = taim/60000; // all minutes
          taim3 = taim2/60;  // hours
          taim4 = taim2 - 60*taim3;  // rest of minutes
          connection->send(String(taim3));
          if (taim4 < 10) connection->send(F(":0"));
          else connection->send(F(":"));
          connection->send(String(taim4));
          connection->send(F(" hours.."));            
        }            
        else {
//    else if (path == "/"){        
        connection->send(F(
        "HTTP/1.1 200 OK\r\n\r\n"
        "<!DOCTYPE html>\r\n"
        "<html>\r\n"         
        "<head>\r\n"                  
        "<title>AJAX Thermostat with Arduino & ESP8266-12E shield</title>\r\n" 
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
        "setTimeout('GetSwitchAnalogData()', 13000);\r\n"
        "}\r\n"
        "function set_thermostat_temperature() {\r\n"
        "var nocache = \"?nocache=\" + Math.random() * 1000000;\r\n"
        "var request = new XMLHttpRequest();\r\n"
        //"var new_temperature = document.getElementById(\"js-temperature-value\");\r\n"
        "var new_temperature = document.getElementById(\"js-temperature-value\").value;\r\n"
        "request.onreadystatechange = function() {\r\n"
        "if (this.readyState == 4) {\r\n"
        "if (this.status == 200) {\r\n"
        "}}}\r\n"
     //   "request.open(\"GET\", \"change\" + nocache + \"&new_temp=\" + new_temperature + \"&done\", true);\r\n"
        "request.open(\"GET\", \"change?new_temp=\" + new_temperature, true);\r\n"
        "request.send(null);\r\n"
        "}\r\n"
        "</script>\r\n"                       
        "</head>\r\n"));   
        connection->send(F(        
        "<body onload=\"GetSwitchAnalogData()\">\r\n"  
        "<body style=background:lightblue>"   
        "<center><h1>AJAX Thermostat</br>with Arduino &</br>ESP8266-12E shield</h1>\r\n" 
        "<h4>(DHT22 as sensor for temperature & humidity)"
        "<div id=\"sw_an_data\">\r\n"
        "</div>\r\n"  
        "<fieldset>"
        "<legend>You will set temperature at</legend>"
        "<select name=\"temperature\" id=\"js-temperature-value\">"
        "<option value='21.0'>21.0</option>"
        "<option value='21.5'>21.5</option>"
        "<option value='22.0'>22.0</option>"
        "<option value='22.5'>22.5</option>"
        "<option value='23.0'>23.0</option>"
        "<option value='23.5'>23.5</option>"
        "<option value='24.0'>24.0</option>"
        "<option value='24.5'>24.5</option>"
        "<option value='25.0'>25.0</option>"
        "</select>"
        "</fieldset>"                         
        "<input type=\"button\" onclick=\"set_thermostat_temperature();\" value=\"Submit\">"
        "<H4>Sketch by Nicu Florica aka niq_ro.<p />"
        "<a href=http://arduinotehniq.blogspot.com target=blank>http://arduinotehniq.blogspot.com</a>"  
        "<p><br>version. 1.9a - 23.11.2015, Craiova</br>"
        "</body>\r\n"          
        "</html>\r\n"));               
        }


        delay(10);  
        connection->close();   
           } 

  
}    // end of void

                                                      


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

void faranet (float tiset, float deti)
{
Serial.println("no web client........");
//int h = dht.readHumidity();
float te5 = dht.readTemperature();  
Serial.print("t = ");
Serial.print(te5);
Serial.println("gr.C");
if (te5 > tiset + deti/2) 
{
  Serial.println("Temperature is high.. cooler is ON");
  digitalWrite(releu,HIGH);   // cooler is ON (powered)
}
if (te5 < tiset - deti/2) 
{
  Serial.println("Temperature is too low.. cooler is OFF");
  digitalWrite(releu,LOW);   // cooler is ON (powered)
}

if ((te5 >= tiset - deti/2) and (te5 <= tiset + deti/2))
{
  Serial.println("Temperature is ok.. ");
}
}
