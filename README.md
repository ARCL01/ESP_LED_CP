# ESP_LED_CP :bulb:
CP stands for **configuration program**.
This program allows you to control LEDs over a web server that is hosted on the ESP32 itself. I did this project when I didn't know that something like WLED existed. If you are asking yourself if my work is better, then no, but I think it's easier to understand and configure.

## Gettings started
### Installation
1. Download **ESP_LED_CP_x.x.x.zip** from releases
2. Edit the pre-configured WiFi password and other settings to your liking in the **.ino** file
3. Upload it to your ESP board
4. Type the IP given to your ESP in the browser and enjoy!

### Configuration
All configurable options **should be** at the top of the program. They **should be** well commented but, I know myself, and I know for sure, that I didn't even paid attention to this.
So, my solution to this. I will just list the important ones here.

```const char* ssid = "xxxx"```
- Here, you will type your WiFi SSID

```const char* password = "xxxx"```
- Here, you will type you WiFi password

```const char* config_vars[x] = {"xx","xx",...};```
- Here, you will type all the options you can configure through the website
- _This should change soon, I will replace this with more understandable options_

```#define NUM_LEDS xx```
- Here, you can define the length of the strip
  
```#define DATA_PIN xx```
- Here, you can define the number of pins to which are LEDs connected.

## Branches
There are two main branches in this repository.
- Main
- Experimental
   
I think the names are self-explanatory but, the **main** branch is for stable releases and **experimental** is for, well, experiments.
