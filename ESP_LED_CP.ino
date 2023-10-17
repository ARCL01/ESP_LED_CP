#include <WiFi.h>
#include <Preferences.h>
#include <FastLED.h>
#include "time.h"

Preferences preferences;

WiFiServer server(80);

String header;
String timezone;
String solid_color;
String run;
String old_run;
String last_program = "";
const char* solid_color_hex;

const int led = 2;
int config_time = 10;
int timeout = 10000;
int clock_12_pos = 0;

bool loop_exit = true; //helper pro exitování smyček
bool config_timeout = true; //helper pro timeout na konfigurační mód

#define led 2

#define NUM_LEDS 60
#define DATA_PIN 13
CRGB leds[NUM_LEDS];

int delay_int;

const char* config_vars[7] = {"clock","solid_color","timezone=","solid_color=","left","right","run="}; //hledané proměnné v configu

const char* ssid = "Kuliferda 3";
const char* password = "pufiik2zaak";

const char* ntpServer = "pool.ntp.org";

void wipe_eeprom_variables() {
  preferences.begin("config", false);
  preferences.remove("timezone");
  preferences.end();
  Serial.println("EEPROM variables will be wiped.");
  Serial.println("For this program to function, you need to flash it again on the ESP without the \"wipe_eeprom_variables()\" being active.");
  Serial.println("The ESP will now restrat, until the function is disabled.");
  delay(5000);
  ESP.restart();
}

void setup() {
  pinMode(led,OUTPUT);
  preferences.begin("config", false); 
  timezone = preferences.getString("timezone","UTC");
  preferences.end();
  Serial.begin(115200);
  //wipe_eeprom_variables();

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected, using IP:");
  Serial.println(WiFi.localIP());
  Serial.println("Configuration server is running");
  server.begin();
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    int start_time_client = millis();
    int current_time_client =  millis();
    Serial.println("New Client.");
    config_timeout = false;
    String LocalIP = WiFi.localIP().toString();
    String currentLine = "";
    // čeká než se připojí klient, pokud je klient připojen déle než 5 vteřni tak začne čekat na další klienty
    while (client.connected() && current_time_client <= start_time_client + 5000) {
      current_time_client =  millis();
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            //čeká na příchozí url
            if (header.indexOf("/config") >= 0) {
              digitalWrite(led,HIGH);
              delay(200);
              digitalWrite(led,LOW);
              delay(200);
              for (int index = 0;index != 7;index++){
                if(header.indexOf(config_vars[index]) >= 0 && index == 0) {
                  Serial.println("<*> 'clock' configuration entry found");
                  last_program = "clock";
                }
                else if(header.indexOf(config_vars[index]) >= 0 && index == 1) {
                  Serial.println("<*> 'solid_color' configuration entry found");
                  last_program = "solid_color";
                }
                else if(header.indexOf(config_vars[index]) >= 0 && index == 2) {
                  Serial.print("<*> 'timezone' configuration variable found: ");
                  // Odděluje od stringu hodnotu za =, aby to fungovalo tak hledá ve stringu = ale nehledá jej od začátku stringu ale od hodnoty v arrayi,
                  // k tomu musím přičíst 1 aby tam to = nebylo, potom je to omezené z druhé strany /,
                  // zase je to udělané tak aby to lomítko nehledal od začátku ale podle hodnoty v arrayi. 
                  timezone = header.substring(header.indexOf("=",header.indexOf(config_vars[index])) + 1, header.indexOf("/",header.indexOf(config_vars[index])));
                  Serial.println(timezone);
                  preferences.begin("config", false); 
                  if (timezone == preferences.getString("timezone","UTC")) {
                    Serial.println("<*> The variable saved in EEPROM is already set to: '" + timezone + "'");
                    preferences.end();
                  }
                  else {
                    preferences.putString("timezone", timezone);
                    preferences.end();
                    Serial.println("<*> Variables successfully saved to EEPROM");
                  }
                }
                else if(header.indexOf(config_vars[index]) >= 0 && index == 3) {
                  Serial.print("<*> 'solid_color' configuration variable found: ");
                  solid_color = "0X" + header.substring(header.indexOf("=",header.indexOf(config_vars[index])) + 1, header.indexOf("/",header.indexOf(config_vars[index])));
                  Serial.println(solid_color);
                  solid_color_hex = solid_color.c_str();
                }
                else if (header.indexOf(config_vars[index]) >= 0 && index == 4) {
                  Serial.print("<*> 'left' configuration variable found:");
                  clock_12_pos -= 1;
                }
                else if (header.indexOf(config_vars[index]) >= 0 && index == 5) {
                  Serial.print("<*> 'right' configuration variable found:");
                  clock_12_pos += 1;
                }
                else if (header.indexOf(config_vars[index]) >= 0 && index == 6) {
                  Serial.print("<*> 'run' configuration variable found: ");
                  run = header.substring(header.indexOf("=",header.indexOf(config_vars[index])) + 1, header.indexOf("/",header.indexOf(config_vars[index])));
                  Serial.println(run);
                }
                else {
                  Serial.print("<!> couldn't find variable for entry: ");
                  Serial.println(config_vars[index]);
                }
              }
            }
            client.println("<!DOCTYPE html>");
            client.println("<html lang='en'>");
            client.println("<head>");
            client.println("    <meta charset='UTF-8'>");
            client.println("    <meta name='viewport' content='width=device-width, initial-scale=1.0'>");
            client.println("    <title>Document</title>");
            client.println("    <link rel='preconnect' href='https://fonts.googleapis.com'>");
            client.println("    <link rel='preconnect' href='https://fonts.gstatic.com' crossorigin>");
            client.println("    <link rel='stylesheet' href='https://fonts.googleapis.com/css2?family=Noto+Sans+Mono&display=swap'>");
            client.println("</head>");
            client.println("<body onload='check_options(); set_program();' style='color: white; background-color: black;  display: flex; flex-wrap: wrap; flex-direction: column; font-family: \"Noto Sans Mono\", monospace; align-items: center;'>");
            client.println("    <select id='programs' onchange='check_options()' style='background-color: black; color: white; height: 3rem; text-align: center; margin-bottom: 2rem; font-family: \"Noto Sans Mono\", monospace; border: solid 0.1rem white; border-radius: 0.5rem; width: 50%;'>");
            client.println("        <option value='clock'>clock</option>");
            client.println("        <option value='solid_color'>solid_color</option>");
            client.println("    </select>");
            client.println("    <div id='solid_color_options' style='display: none; text-align: center;'>");
            client.println("        <h1>--solid color options--</h1>");
            client.println("        <label for='solid_color'>color:</label><br>");
            client.println("        <input type='color' id='solid_color' value='#"+ solid_color.substring(2) +"'style=' width: 10rem; height: 3rem; border: solid 0.1rem white; border-radius: 0.5rem; background-color: transparent; margin-top: 0.5rem;'><br>");
            client.println("        <input type='button' onclick='solid_color_apply()' value='--apply--' style='background-color: black; color: white; height: 3rem; width: 10rem; font-family: \"Noto Sans Mono\", monospace; border: solid 0.1rem white; border-radius: 0.5rem; margin-top: 3rem;'>");
            client.println("    </div>");
            client.println("    <div id='clock_options' style='display: none; text-align: center; '>");
            client.println("        <h1>--clock options--</h1>");
            client.println("        <label for='timezone'>hour difference from GMT time (add +/- before the number): </label><br>");
            client.println("        <input type='text' id='timezone' value='"+ timezone +"'style=' background-color: black; color: white; border: solid 0.1rem white; border-radius: 0.5rem; font-family: \"Noto Sans Mono\", monospace; margin-bottom: 2rem; margin-top: 0.5rem; height: 2.5rem;'><br>");
            client.println("        <label for='timezone'>Move the 12 hour mark on the clock: </label><br>");
            client.println("        <input type='button' id='clock_move_left' value='<<' onclick='clock_move_left()' style='background-color: black; color: white; height: 2rem; width: 3rem; border: solid 0.1rem white; border-radius: 0.5rem; font-family: \"Noto Sans Mono\", monospace; margin-top: 0.5rem;'>");
            client.println("        <input type='button' id='clock_move_right' value='>>' onclick='clock_move_right()' style='background-color: black; color: white; height: 2rem; width: 3rem; border: solid 0.1rem white; border-radius: 0.5rem; font-family: \"Noto Sans Mono\", monospace; margin-top: 0.5rem;'><br>");
            client.println("        <input type='button' onclick='clock_apply() ' value='--apply--' style='background-color: black; color: white; height: 3rem; width: 10rem; font-family: \"Noto Sans Mono\", monospace; border: solid 0.1rem white; border-radius: 0.5rem; margin-top: 3rem;'>");
            client.println("    </div>");
            client.println("</body>");
            client.println("<script>");
            client.println("    function set_program(last_program = '"+ last_program +"') {");
            client.println("       if (last_program == 'clock') {");
            client.println("          document.getElementById('programs').value = 'clock'");
            client.println("          document.getElementById('clock_options').style.display = 'block'");
            client.println("          document.getElementById('solid_color_options').style.display = 'none'");
            client.println("      }");
            client.println("      else if (last_program == 'solid_color') {");
            client.println("          document.getElementById('programs').value = 'solid_color'");
            client.println("          document.getElementById('solid_color_options').style.display = 'block'");
            client.println("          document.getElementById('clock_options').style.display = 'none'");
            client.println("      }");
            client.println("    }");
            client.println("    function check_options() {");
            client.println("        if (document.getElementById('programs').value == 'solid_color') {");
            client.println("            console.log('solid_color');");
            client.println("            document.getElementById('solid_color_options').style.display = 'block'");
            client.println("            document.getElementById('clock_options').style.display = 'none'");
            client.println("        }");
            client.println("        else if (document.getElementById('programs').value == 'clock') {");
            client.println("            console.log('clock');");
            client.println("            document.getElementById('clock_options').style.display = 'block'");
            client.println("            document.getElementById('solid_color_options').style.display = 'none'");
            client.println("        }");
            client.println("    }");
            client.println("    function clock_move_left() {");
            client.println("        window.open('http://"+ LocalIP + "/config/clock/left/config_end','_self');");
            client.println("    }");
            client.println("    function clock_move_right() {");
            client.println("        window.open('http://"+ LocalIP + "/config/clock/right/config_end','_self');");
            client.println("    }");
            client.println("    function clock_apply() {");
            client.println("        last_program = 'clock'");
            client.println("        window.open('http://"+ LocalIP +"/config/clock/timezone='+ document.getElementById('timezone').value +'/run='+ document.getElementById('programs').value +'/config_end','_self');");
            client.println("    }");
            client.println("    function solid_color_apply() {");
            client.println("        last_program = 'solid_color'");
            client.println("        window.open('http://"+ LocalIP + "/config/solid_color/solid_color='+ document.getElementById('solid_color').value.substring(1) +'/run='+ document.getElementById('programs').value +'/config_end','_self');");
            client.println("    }");
            client.println("</script>");
            client.println();
            break;
          } 
          else {
            currentLine = "";
          }
        } 
        else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  if(run == "clock" ) {
    if (old_run != "clock") {
      for(int x=0; x < NUM_LEDS; x++) {
        leds[x] = 0x000000;
        leds[NUM_LEDS-x] = 0x000000;
        FastLED.show();
        delay(10);
      }
    }
    old_run = "clock";
    configTime(timezone.toInt()*3600, 3600, ntpServer);
    Serial.println("<*> Clock program is running");
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("<!> Failed to obtain time");
      return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    int next_led = 0;
    float hours_to_min = timeinfo.tm_hour * 60; //hodiny převedené na minuty
    float minutes = timeinfo.tm_min;
    float seconds_to_min = timeinfo.tm_sec / 60; //sekundy převedené na minuty
    float plus_led_time = 1440 / NUM_LEDS; //čas který iběhne než se rosvítí jedna ledka
    float time_in_mins_fl = hours_to_min + minutes + seconds_to_min;
    int time_in_mins_in = hours_to_min + minutes + seconds_to_min;

    int difference = (time_in_mins_fl * 100) - (time_in_mins_in * 100); 

    Serial.print("<*> Time in minutes: ");
    Serial.println(time_in_mins_fl);
    Serial.print("<*> LEDs turned on: ");
    Serial.println(time_in_mins_fl/plus_led_time);
    Serial.print("<*> Time difference from GMT time: ");
    Serial.println(timezone);

    if (hours_to_min == 0 && minutes == 0){
      for (int x=0; x < NUM_LEDS; x++) {
        leds[x] = 0x000000;
        leds[NUM_LEDS-x] = 0x000000;
        FastLED.show();
        delay(10);
      }
    }

    for (int x=0; x < time_in_mins_fl/plus_led_time; x++) {
      leds[NUM_LEDS-x] = 0x0b2752;
      next_led = NUM_LEDS - x - 1;
      FastLED.show();
      delay(10);
    }
    for (int x=255;x >= 0;x--) {
      leds[next_led] = CHSV(32, 255, x);
      FastLED.show();
      if (x==0) {
        for (int x=0;x <= 255;x++) {
          leds[next_led] = CHSV(32, 255, x);
          FastLED.show();
        }
      }
    }
  }
  else if (run == "solid_color") {
    if (old_run != "solid_color") {
      for(int x=0; x < NUM_LEDS; x++) {
        leds[x] = 0x000000;
        leds[NUM_LEDS-x] = 0x000000;
        FastLED.show();
        delay(10);
      }
    }
    old_run = "solid_color";
    Serial.println("<*> Solid_color program is running");
    for (int x=0; x < NUM_LEDS; x++) {
      leds[x] = strtol(solid_color_hex, NULL, 0);
      leds[NUM_LEDS-x] = strtol(solid_color_hex, NULL, 0);
      FastLED.show();
      delay(10);
    }
  }
}
