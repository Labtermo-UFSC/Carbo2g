#include <Arduino.h>
#include <WiFi.h>
#include <esp_wpa2.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include "secrets.h"
#include "config.h"
#include "MHZ19.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ------------------------ DEFINES ------------------------ //
#define timeoutUntillResetSeconds  60
#define timeToUpdateGsheetsSeconds 60

// ------------------ Gsheets Credentials ------------------ //
// Gscript ID (googleScriptID) is in secrets.h
String GOOGLE_SCRIPT_ID = googleScriptID;
String GOOGLE_SHEET_NAME = sheetName;
String SENSOR_ID = sensorID;

// ------------------------ DISPLAY ------------------------ //
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ------------------------ MHZ19 -------------------------- //
#define GlobalRXMhz19 16
#define GlobalTXMhz19 17
#define MHZBAUDRATE 9600 
MHZ19 mhz; 

// ------------------------ VARIABLES ---------------------- //


// ------------------ Function Prototypes ------------------ //
void checkConnection(int16_t timeout);
String IpAddress2String(const IPAddress& ipAddress);
void setupWiFi(bool IsWAP2Enterprise);
void setupDisplay();
void setupMHZ19();
void updateScreen(int16_t CO2Unlimited);

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  setupDisplay();
  setupMHZ19();
  setupWiFi(true);

}

void loop()
{
  vTaskDelay((timeToUpdateGsheetsSeconds * 1000) / portTICK_PERIOD_MS);
  int16_t CO2Unlimited = mhz.getCO2(true, true);
  int16_t CO2limited = mhz.getCO2(false, true);
  float Temperature = mhz.getTemperature(true);
  updateScreen(CO2Unlimited);
  Serial.println(String("CO2 Unlimited:" + String(CO2Unlimited) + "  CO2 Limited:" + String(CO2limited) + "  Temperature:" + String(Temperature)));
  checkConnection(timeoutUntillResetSeconds);
  String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+"sheet="+GOOGLE_SHEET_NAME;
  urlFinal = urlFinal + "&value0=" + String(CO2Unlimited) + "&value1=" + String(CO2limited) + "&value2=" + String(Temperature);
  urlFinal = urlFinal + "&rssi=" + String(WiFi.RSSI()) + "&ipaddr=" + IpAddress2String(WiFi.localIP()) + "&ssid=" + String(WiFi.SSID()); 
  Serial.print("POST data to spreadsheet:");
  Serial.println(urlFinal);
  HTTPClient http;
  http.begin(urlFinal.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET(); 
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);
  //---------------------------------------------------------------------
  //getting response from google sheet
  String payload;
  if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);    
  }
  //---------------------------------------------------------------------
  http.end();
}

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}

void checkConnection(int16_t timeout){
  int timeoutCounter = 0;
  if (WiFi.status() != WL_CONNECTED)
  {
    // Tryy to reconnect
    WiFi.begin(ssid);
    Serial.printf("Reconnecting to WiFi: %s ", ssid);
    while (WiFi.status() != WL_CONNECTED)
    { 
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      Serial.print(F("."));
      timeoutCounter++;;
      if (timeoutCounter >= timeout)
      { //x seconds timeout - reset board
        ESP.restart();
      }
    }
    Serial.println(F(" connected!"));
  }
}

void setupWiFi(bool IsWAP2Enterprise){
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  if (IsWAP2Enterprise)
  {
    Serial.println("Using WPA2 Enterprise");
    //esp_wifi_set_mac(ESP_IF_WIFI_STA, &masterCustomMac[0]);
    Serial.print("MAC >> ");
    Serial.println(WiFi.macAddress());
    Serial.printf("Connecting to WiFi: %s ", ssid);
    //esp_wifi_sta_wpa2_ent_set_ca_cert((uint8_t *)incommon_ca, strlen(incommon_ca) + 1);
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)identity1, strlen(identity1));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)identity1, strlen(identity1));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password1, strlen(password1));
    esp_wifi_sta_wpa2_ent_enable();
    WiFi.begin(ssid);
  }
  else
  {
    Serial.println("Using Normal Network");
    WiFi.begin(ssid2, password2);
  }
  int timeoutCounter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Conectando");
    display.println(String("   " + String(timeoutCounter) + "/" + String(timeoutUntillResetSeconds)));
    display.display();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.print(F("."));
    timeoutCounter++;
    if (timeoutCounter >= timeoutUntillResetSeconds)
    { //after 30 seconds timeout - reset board
      ESP.restart();
    }
  }
  Serial.println(F(" connected!"));
  Serial.print(F("IP address set: "));
  Serial.println(WiFi.localIP()); //print LAN IP
  display.clearDisplay();
  display.println("Conectado!");
  display.display();
  vTaskDelay(10000 / portTICK_PERIOD_MS);
}

void setupDisplay(){
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    ESP.restart();
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.display();
}

void setupMHZ19(){
  Serial2.begin(MHZBAUDRATE, SERIAL_8N1, GlobalRXMhz19, GlobalTXMhz19); // Serial2 is the MH-Z19 serial port
  mhz.begin(Serial2);
  mhz.autoCalibration(true);
  char myVersion[4];
  mhz.getVersion(myVersion);
  Serial.print("\nFirmware Version: ");
  for(byte i = 0; i < 4; i++)
  {
    Serial.print(myVersion[i]);
    if(i == 1)
      Serial.print(".");
  }
   Serial.println("");
   Serial.print("Range: ");
   Serial.println(mhz.getRange());
   Serial.print("Background CO2: ");
   Serial.println(mhz.getBackgroundCO2());
   Serial.print("Temperature Cal: ");
   Serial.println(mhz.getTempAdjustment());
   Serial.print("ABC Status: "); mhz.getABC() ? Serial.println("ON") :  Serial.println("OFF");
}

void updateScreen(int16_t CO2Unlimited){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Carbo2g-" + SENSOR_ID);
  display.println("CO2: " + String(CO2Unlimited));
  display.println("RAND: " + String(random(100)));
  display.display();
}