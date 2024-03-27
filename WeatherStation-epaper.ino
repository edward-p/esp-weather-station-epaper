/**The MIT License (MIT)

  Copyright (c) 2017 by Hui Lu

  Permission i  s hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <JsonListener.h>
#include "Wire.h"
#include "TimeClient.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "heweather.h"
#include <EEPROM.h>
#include <SPI.h>
#include "FS.h"
#include "EPD_drive.h"
#include "EPD_drive_gpio.h"
#include "bitmaps.h"
#include "lang.h"
ADC_MODE(ADC_VCC);

struct ConfigStruct {
  char city[32];
  char server[258];
  int port;
};

/***************************
  Settings
 **************************/
//const char* WIFI_SSID = "";
//const char* WIFI_PWD = "";
uint64_t sleeptime = 60;   //updating interval 71min maximum
const float UTC_OFFSET = 8;
byte end_time = 1;          //time that stops to update weather forecast
byte start_time = 7;        //time that starts to update weather forecast
String server;
int port;
const char* client_name = "news"; //send message to weather station via duckduckweather.esy.es/client.php
//modify language in lang.h

/***************************
 **************************/
String city;
String lastUpdate = "--";
bool shouldsave = false;
bool updating = false; //is in updating progress
TimeClient* timeClient;
heweatherclient* heweather;

//Ticker ticker;
Ticker avoidstuck;
WaveShare_EPD EPD = WaveShare_EPD();

void saveConfigCallback () {
  shouldsave = true;
}
void setup() {

  Serial.begin(115200); Serial.println(); Serial.println();
  check_rtc_mem();
  pinMode(D3, INPUT);
  pinMode(CS, OUTPUT);
  pinMode(DC, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(BUSY, INPUT);
  EEPROM.begin(sizeof(ConfigStruct));
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  SPI.begin();
  SPIFFS.begin();
  EPD.EPD_init_Part(); driver_delay_xms(DELAYTIME);
  /*************************************************
    wifimanager
  *************************************************/
  //WiFi.begin(WIFI_SSID, WIFI_PWD);
  WiFiManagerParameter p_city("city", "city", "your city", 32);
  WiFiManagerParameter p_server("server", "server", "192.168.1.1", 256);
  WiFiManagerParameter p_port("port", "port", "80", 8);
  WiFiManager wifiManager;
  if (read_config() == 126)
  {
    EPD.deepsleep(); ESP.deepSleep(60 * sleeptime * 1000000, WAKE_RF_DISABLED);
  }
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&p_city);
  wifiManager.addParameter(&p_server);
  wifiManager.addParameter(&p_port);
  wifiManager.autoConnect("Weather widget");
  while (WiFi.status() != WL_CONNECTED)
  {
    always_sleep();
    Serial.println("failed to connect and hit timeout");
    EPD.clearshadows();
    EPD.clearbuffer();
    EPD.fontscale = 1;
    EPD.SetFont(3);
    EPD.DrawUTF(0, 0, 12, 12, config_timeout_line1);
    EPD.DrawUTF(18, 0, 12, 12, config_timeout_line2);
    EPD.DrawUTF(36, 0, 12, 12, config_timeout_line3);
    EPD.DrawUTF(52, 0, 12, 12, config_timeout_line4);
    EPD.EPD_Dis_Part(0, 127, 0, 295, (unsigned char *)EPD.EPDbuffer, 1);
    EPD.deepsleep(); ESP.deepSleep(60 * sleeptime * 1000000, WAKE_RF_DISABLED);
  }
  city = p_city.getValue();
  server = p_server.getValue();
  port = (int) String(p_port.getValue()).toInt();
  /*************************************************
    EPPROM
  *************************************************/
  if (city != "your city")
  {
    struct ConfigStruct config_s;
    city.toCharArray(config_s.city, sizeof(config_s.city));
    server.toCharArray(config_s.server, sizeof(config_s.server));
    config_s.port = port;
    EEPROM.put(0, config_s);
    EEPROM.commit();
  }

  struct ConfigStruct config_s;
  EEPROM.get(0, config_s);

  city = String(config_s.city);
  server = String(config_s.server);
  port = config_s.port;

  timeClient = new TimeClient(UTC_OFFSET, server, port);
  heweather = new heweatherclient(server, port, lang);
  heweather->city = city;
  heweather->client_name = client_name;
  /*************************************************
     update weather
  *************************************************/
  //heweather->city="huangdao";
  avoidstuck.attach(10, check);
  updating = true;
  updateData();
  updating = false;
  updatedisplay();

  free(heweather);
  free(timeClient);
}

void check()
{
  if (updating == true)
  {
    EPD.deepsleep();
    ESP.deepSleep(60 * sleeptime * 1000000, WAKE_RF_DISABLED);
  }
  avoidstuck.detach();
  return;
}

void loop() {

  if (digitalRead(D3) == LOW)
  {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    ESP.reset();
  }

  EPD.deepsleep();
  ESP.deepSleep(60 * sleeptime * 1000000, WAKE_RF_DISABLED);

}

void updatedisplay()
{

  EPD.clearshadows(); EPD.clearbuffer(); EPD.fontscale = 1;

  boolean isNight = timeClient->getHours().toInt() >= 19;
  EPD.SetFont(12); unsigned char code[] = {0x00, heweather->now_cond_index.toInt()}; EPD.DrawUnicodeStr(0, 16, 80, 80, 1, code);
  EPD.SetFont(13); unsigned char code2[] = {0x00, isNight ? heweather->today_cond_n_index.toInt() : heweather->today_cond_d_index.toInt()}; EPD.DrawUnicodeStr(0, 113, 32, 32, 1, code2);
  EPD.SetFont(13); unsigned char code3[] = {0x00, heweather->tomorrow_cond_d_index.toInt()}; EPD.DrawUnicodeStr(32, 113, 32, 32, 1, code3);
  EPD.SetFont(13); unsigned char code4[] = {0x00, heweather->thedayaftertomorrow_cond_d_index.toInt()}; EPD.DrawUnicodeStr(64, 113, 32, 32, 1, code4);
  EPD.DrawXline(114, 295, 32);
  EPD.DrawXline(114, 295, 64);
  EPD.DrawXline(114, 295, 96);

  EPD.SetFont(3);
  Serial.println("heweather->city");
  Serial.println(heweather->city);
  EPD.DrawXbm_P(80, 5, 12, 12, (unsigned char *)city_icon); EPD.DrawUTF(80, 21, 12, 12, heweather->city); //城市名
  EPD.DrawUTF(80, 70, 12, 12, heweather->now_txt); //当前天气
  EPD.DrawUTF(98, 70, 12, 12, "RH " + heweather->now_hum + "%");
  EPD.DrawUTF(112, 70, 12, 12, "FL " + heweather->now_feels_like + "°C");
  EPD.DrawUTF(112, 70, 12, 12, heweather->date.substring(5, 10));
  EPD.DrawUTF(4, 145, 12, 12, (String)todaystr + " " + heweather->today_tmp_min + "°C~" + heweather->today_tmp_max + "°C");
  EPD.DrawUTF(18, 145, 12, 12, heweather->today_txt_d + "/" + heweather->today_txt_n);
  EPD.DrawUTF(36, 145, 12, 12, (String)tomorrowstr + " " + heweather->tomorrow_tmp_min + "°C~" + heweather->tomorrow_tmp_max + "°C");
  EPD.DrawUTF(50, 145, 12, 12, heweather->tomorrow_txt_d + "/" + heweather->tomorrow_txt_n);
  EPD.DrawUTF(66, 145, 12, 12, (String)thedayaftertomorrowstr + " " + heweather->thedayaftertomorrow_tmp_min + "°C~" + heweather->thedayaftertomorrow_tmp_max + "°C");
  EPD.DrawUTF(80, 145, 12, 12, heweather->thedayaftertomorrow_txt_d + "/" + heweather->thedayaftertomorrow_txt_n);
  EPD.DrawXbm_P(106, 116, 12, 12, (unsigned char *)aqi_icon); EPD.DrawUTF(106, 131, 12, 12, heweather->category + "[" + heweather->aqi + "]" + ", pm10[" + heweather->pm10 + "], pm2_5[" + heweather->pm2p5 + "]");
  //  EPD.DrawUTF(86,116,16,16,"RH:"+heweather->now_hum+"%"+" "+heweather->now_dir+heweather->now_sc);
  //  EPD.DrawXbm_P(76,116,12,12,(unsigned char *)night);  EPD.DrawUTF(76,131,12,12,tonightstr+heweather->today_txt_n);

  //  EPD.DrawXbm_P(112, 116, 12, 12, (unsigned char *)message);/
  //  EPD.DrawUTF(112, 131, 12, 12, heweather->message);/
  EPD.SetFont(0x2); EPD.DrawUTF(3, 245, 10, 10, lastUpdate); //updatetime

  EPD.SetFont(0x1);
  EPD.DrawUTF(96, 5, 32, 32, heweather->now_tmp + "°C"); //天气实况温度
  EPD.DrawYline(96, 127, 67);
  dis_batt(3, 272);


  for (int i = 0; i < 1808; i++) EPD.EPDbuffer[i] = ~EPD.EPDbuffer[i];
  EPD.EPD_Dis_Part(0, 127, 0, 295, (unsigned char *)EPD.EPDbuffer, 1);
  //EPD.EPD_Dis_Full((unsigned char *)EPD.EPDbuffer,1);
  driver_delay_xms(DELAYTIME);


}



void updateData() {

  timeClient->updateTime();
  heweather->update();
  lastUpdate = timeClient->getHours() + ":" + timeClient->getMinutes();
  byte rtc_mem[4]; rtc_mem[0] = 126;
  byte Hours = timeClient->getHours().toInt();
  Serial.println("hour");
  Serial.print(Hours);
  if (Hours == end_time)
  {
    if ((start_time - end_time) < 0)  rtc_mem[1] = (24 - Hours + start_time) * 60 / sleeptime;
    else rtc_mem[1] = (start_time - Hours) * 60 / sleeptime;
    ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
    Serial.println("rtc_mem[1]");
    Serial.println(rtc_mem[1]);
  }

  Serial.print("heweather->rain"); Serial.print(heweather->rain); Serial.print("\n");
  //delay(1000);
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //进入配置模式

  EPD.clearshadows(); EPD.clearbuffer(); EPD.fontscale = 1;
  EPD.SetFont(3);
  EPD.fontscale = 1;
  EPD.DrawXbm_P(6, 80, 32, 32, one);
  EPD.DrawXbm_P(42, 80, 32, 32, two);
  EPD.DrawXbm_P(79, 80, 32, 32, three);
  EPD.fontscale = 1;
  EPD.DrawUTF(6, 112, 12, 12, config_page_line1);
  EPD.DrawUTF(22, 112, 12, 12, config_page_line2);
  EPD.DrawXline(80, 295, 39);
  EPD.DrawUTF(42, 112, 12, 12, config_page_line3);
  EPD.DrawUTF(58, 112, 12, 12, config_page_line4);
  EPD.DrawXline(80, 295, 76);
  EPD.DrawUTF(79, 112, 12, 12, config_page_line5);
  EPD.DrawUTF(94, 112, 12, 12, config_page_line6);
  EPD.DrawXbm_P(6, 0, 70, 116, phone);
  EPD.EPD_Dis_Part(0, 127, 0, 295, (unsigned char *)EPD.EPDbuffer, 1);
  driver_delay_xms(DELAYTIME);
}
void dis_batt(int16_t x, int16_t y)
{
  /*attention! calibrate it yourself */
  float voltage;
  voltage = (float)ESP.getVcc() / 1000;

  float batt_voltage = voltage;

  Serial.println((String)"batt:" + batt_voltage);
  /*attention! calibrate it yourself */
  if (batt_voltage <= 3.15)  {
    EPD.clearbuffer();
    EPD.DrawXbm_P(39, 98, 100, 50, (unsigned char *)needcharge);
    always_sleep();
  }
  if (batt_voltage > 3.15 && batt_voltage <= 2.95)  EPD.DrawXbm_P(x, y, 20, 10, (unsigned char *)batt_1);
  if (batt_voltage > 3.25 && batt_voltage <= 3.0)  EPD.DrawXbm_P(x, y, 20, 10, (unsigned char *)batt_2);
  if (batt_voltage > 3.3 && batt_voltage <= 3.05)  EPD.DrawXbm_P(x, y, 20, 10, (unsigned char *)batt_3);
  if (batt_voltage > 3.35 && batt_voltage <= 3.1)  EPD.DrawXbm_P(x, y, 20, 10, (unsigned char *)batt_4);
  if (batt_voltage > 3.4)  EPD.DrawXbm_P(x, y, 20, 10, (unsigned char *)batt_5);
}
unsigned long read_config()
{
  byte rtc_mem[4];
  ESP.rtcUserMemoryRead(4, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
  Serial.println("first time to run check config");
  return rtc_mem[2];

}
unsigned long always_sleep()
{
  byte rtc_mem[4];
  ESP.rtcUserMemoryRead(4, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
  if (rtc_mem[2] != 126)
  {
    Serial.println("first time to run check config");
    rtc_mem[2] = 126;
    ESP.rtcUserMemoryWrite(4, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
  }
  return 0;
}
void check_rtc_mem()
{
  /*
    rtc_mem[0] sign for first run
    rtc_mem[1] how many hours left
  */
  byte rtc_mem[4];
  ESP.rtcUserMemoryRead(0, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
  if (rtc_mem[0] != 126)
  {
    Serial.println("first time to run");
    rtc_mem[0] = 126;
    rtc_mem[1] = 0;
    ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
  }
  else
  {
    if (rtc_mem[1] > 0)
    {
      rtc_mem[1]--;
      Serial.println("don't need to update weather");
      Serial.println(rtc_mem[1]);
      ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
      EPD.deepsleep();
      ESP.deepSleep(60 * sleeptime * 1000000, WAKE_RF_DISABLED);
    }
  }

}
