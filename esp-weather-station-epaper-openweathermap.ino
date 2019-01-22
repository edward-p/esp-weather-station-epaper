/*
   MIT License (MIT)

   Copyright (c) 2017 by Hui Lu

   Permission is hereby granted, free of charge, to any person obtaining a copy
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

   MIT License

   Copyright (c) 2019 Edward Pacman

   Permission is hereby granted, free of charge, to any person obtaining a copy
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

#include "waveshare/EPD_drive.h"
#include "waveshare/EPD_drive_gpio.h"
#include "waveshare/EPD_drive.cpp"
#include "waveshare/EPD_drive_gpio.cpp"

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <SPI.h>
#include <time.h>

#include <OpenWeatherMapCurrent.h>
#include <OpenWeatherMapForecast.h>
#include <SunMoonCalc.h>

#include "settings.h"
#include "bitmaps.h"
#include "lang.h"
ADC_MODE(ADC_VCC);

const String & getWindDirectionSign(float windDeg)
{
	double wd = windDeg;

	if (wd - 11.25 < 0)
		wd = wd - 11.25 + 360;
	else
		wd -= 11.25;
	return strWindDirection[(int)floor(wd / 22.5)];
}

const byte getMeteoconIcon(String iconText)
{
// clear sky
	if (iconText == "01d") return 12;
	if (iconText == "01n") return 25;
	// few clouds
	if (iconText == "02d") return 58;
	if (iconText == "02n") return 59;
// scattered clouds
	if (iconText == "03d" || iconText == "03n") return 54;
	// broken clouds
	if (iconText == "04d") return 38;
	if (iconText == "04n") return 39;
	//rain
	if (iconText == "09d" || iconText == "09n") return 19;
	// shower rain
	if (iconText == "10d" || iconText == "10n") return 40;
	// thunderstorm
	if (iconText == "11d" || iconText == "11n") return 26;
	// snow
	if (iconText == "13d" || iconText == "13n") return 16;
	// mist
	if (iconText == "50d" || iconText == "50n") return 33;
	// Nothing matched: N/A,use"°F"or"°C" inseted
	return IS_METRIC ? 48 : 47;
}

const int getwday(uint32_t epoch)
{

}
/***************************
 **************************/
String locID;
String apiKey;

OpenWeatherMapCurrentData currentWeather;
OpenWeatherMapForecastData forecasts[MAX_FORECASTS];

String lastUpdate = "--";
bool shouldsave = false;
bool updating = false; //is in updating progress

NTPClient ntpClient(ntpServer, UTC_OFFSET*3600);
struct tm nowInfo;

//Ticker ticker;
Ticker avoidstuck;
WaveShare_EPD EPD = WaveShare_EPD();

void saveConfigCallback()
{
	shouldsave = true;
}
void setup()
{

	Serial.begin(115200); Serial.println(); Serial.println();
	check_rtc_mem();
	pinMode(D3, INPUT);
	pinMode(CS, OUTPUT);
	pinMode(DC, OUTPUT);
	pinMode(RST, OUTPUT);
	pinMode(BUSY, INPUT);
	EEPROM.begin(41);
	SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
	SPI.begin();
	EPD.EPD_init_Part(); driver_delay_xms(DELAYTIME);
	/*************************************************
	   wifimanager
	*************************************************/
	//WiFi.begin(WIFI_SSID, WIFI_PWD);
	WiFiManagerParameter wp_locID("locID", "location id", "", 7);
	WiFiManagerParameter wp_apiKey("apiKey", "openweathermap api key", "", 32);

	WiFiManager wifiManager;
	if (read_config() == 126) {
		EPD.deepsleep(); ESP.deepSleep(60 * sleeptime * 1000000);
	}
	wifiManager.setConfigPortalTimeout(180);
	wifiManager.setAPCallback(configModeCallback);
	wifiManager.setSaveConfigCallback(saveConfigCallback);
	wifiManager.addParameter(&wp_locID);
	wifiManager.addParameter(&wp_apiKey);
	wifiManager.autoConnect("Weather widget");
	while (WiFi.status() != WL_CONNECTED) {
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
		EPD.deepsleep(); ESP.deepSleep(60 * sleeptime * 1000000);
	}
	ntpClient.begin();
	locID = wp_locID.getValue();
	apiKey = wp_apiKey.getValue();
	/*************************************************
	   EPPROM Write
	*************************************************/
	if (locID.length() != 0) {
		Serial.print("Writing EEPROM....");
		int i = 0;
		//write location ID
		EEPROM.write(i++, locID.length());
		for (int j = 0; j < locID.length(); i++, j++)
			EEPROM.write(i, locID[j]);
		//write API key
		EEPROM.write(i++, apiKey.length());
		for (int j = 0; j < apiKey.length(); i++, j++)
			EEPROM.write(i, apiKey[j]);
		EEPROM.commit();
		Serial.println("done");
	}
	/*************************************************
	   EPPROM Read
	*************************************************/
	int i = 0;
	//read location ID
	locID = "";
	for (int j = 0, length = EEPROM.read(i++); j < length; j++, i++)
		locID += (char)EEPROM.read(i);
	//read API key
	apiKey = "";
	for (int j = 0, length = EEPROM.read(i++); j < length; j++, i++)
		apiKey += (char)EEPROM.read(i);
	Serial.println(locID);
	Serial.println(apiKey);
	/*************************************************
	   update weather
	*************************************************/
	avoidstuck.attach(10, check);
	updating = true;
	updateData();
	updating = false;
	updatedisplay();
}
void check()
{
	if (updating == true) {
		EPD.deepsleep();
		ESP.deepSleep(60 * sleeptime * 1000000);
	}
	avoidstuck.detach();
	return;
}

void loop()
{
	if (digitalRead(D3) == LOW) {
		WiFiManager wifiManager;
		wifiManager.resetSettings();
		delay(200);
		ESP.restart();
	}
	EPD.deepsleep();
	ESP.deepSleep(60 * sleeptime * 1000000);
}

void updatedisplay()
{
	EPD.clearshadows(); EPD.clearbuffer(); EPD.fontscale = 1;

	EPD.SetFont(12); unsigned char code[] = { 0x00,  getMeteoconIcon(currentWeather.icon) }; EPD.DrawUnicodeStr(0, 0, 80, 80, 1, code);
	EPD.SetFont(13); unsigned char code2[] = { 0x00, getMeteoconIcon(currentWeather.icon) }; EPD.DrawUnicodeStr(0, 88, 32, 32, 1, code2);
	EPD.SetFont(3);
	EPD.DrawXbm_P(80, 5, 12, 12, (unsigned char *)city_icon); EPD.DrawUTF(80, 21, 12, 12, currentWeather.cityName); //city name
	EPD.DrawUTF(96, 72, 12, 12, (String)strHumidity); EPD.DrawUTF(96, 98, 12, 12, String(currentWeather.humidity) + "%");
	EPD.DrawUTF(112, 76, 12, 12, MONTH_NAMES[nowInfo.tm_mon]); EPD.DrawUTF(112, 96, 12, 12, String(nowInfo.tm_mday));

	EPD.DrawUTF(2, 121, 12, 12, WDAY_NAMES[nowInfo.tm_wday] + " " + String(currentWeather.temp, 1) + degreeSign);
	EPD.DrawUTF(17, 121, 12, 12, currentWeather.description + " " + getWindDirectionSign(currentWeather.windDeg) + "->" + String(currentWeather.windSpeed, 1) + "m/s");
	EPD.DrawXline(86, 295, 30);

	for (int i = 1, x = 31; i < 3; i++, x += 31) {
		int forecastWday = nowInfo.tm_wday + i;
		if (forecastWday >= 7)
			forecastWday -= 7;
		float minTempOfDay = 460;
		float maxTempOfDay = -460;
		for (int j = 0; j < MAX_FORECASTS; j++) {
			time_t obsTime = forecasts[j].observationTime + 3600 * UTC_OFFSET;
			struct tm * obsTimeInfo = localtime(&obsTime);
			Serial.println(String("nowInfo.tm_wday=" ) + nowInfo.tm_wday);
			Serial.println(String("obsTimeInfo->tm_wday=" ) + obsTimeInfo->tm_wday);
			if (obsTimeInfo->tm_wday == forecastWday) {
				if (forecasts[j].tempMin < minTempOfDay)
					minTempOfDay = forecasts[j].tempMin;
				if (forecasts[j].tempMax > maxTempOfDay)
					maxTempOfDay = forecasts[j].tempMax;
			}
		}
		for (int j = 0; j < MAX_FORECASTS; j++) {
			time_t obsTime = forecasts[j].observationTime + 3600 * UTC_OFFSET;
			struct tm * obsTimeInfo = localtime(&obsTime);
			Serial.println(String("nowInfo.tm_wday=" ) + nowInfo.tm_wday);
			Serial.println(String("obsTimeInfo->tm_wday=" ) + obsTimeInfo->tm_wday);
			if (obsTimeInfo->tm_wday == forecastWday) {
				EPD.SetFont(13); unsigned char code3[] = { 0x00, getMeteoconIcon(forecasts[i].icon) }; EPD.DrawUnicodeStr(x, 88, 32, 32, 1, code3);
				EPD.SetFont(3);
				EPD.DrawUTF(x + 2, 121, 12, 12, WDAY_NAMES[forecastWday] + " "  + String(minTempOfDay, 1) + degreeSign  + "~" + String(maxTempOfDay, 1) + degreeSign);
				EPD.DrawUTF(x + 17, 121, 12, 12, forecasts[i].description + " " + getWindDirectionSign(forecasts[i].windDeg) + "->" + String(forecasts[i].windSpeed, 1) + "m/s");
				EPD.DrawXline(86, 295, x + 30);
				break;
			}
		}
	}

	// EPD.DrawUTF(86,116,16,16,"RH:"+heweather.now_hum+"%"+" "+heweather.now_dir+heweather.now_sc);
	// EPD.DrawXbm_P(76,116,12,12,(unsigned char *)night);  EPD.DrawUTF(76,131,12,12,tonightstr+heweather.today_txt_n);

	EPD.SetFont(3);
	EPD.DrawYline(96, 127, 123);
	EPD.DrawYline(96, 127, 182);
	EPD.DrawUTF(96, 192, 12, 12, "OpenWeatherMap");                                 //OpenWeatherMap
	EPD.DrawUTF(112, 187, 12, 12, (String)strLastUpdate + " " + lastUpdate);        //last time updated
	EPD.DrawUTF(96, 128, 12, 12, (String)strPressure);
	EPD.DrawUTF(112, 128, 12, 12, String(currentWeather.pressure / 10.0, 1) + "kPa");

	EPD.SetFont(0x1);
	EPD.DrawUTF(96, 5, 32, 32, String(currentWeather.temp, 0) + "°"); //currentWeather
	EPD.DrawYline(96, 127, 67);
	dis_batt(3, 272);

	for (int i = 0; i < 1360; i++) EPD.EPDbuffer[i] = ~EPD.EPDbuffer[i];
	for (int i = 128 * 296 / 8 - 1; i >= 1360; i -= 16) {
		EPD.EPDbuffer[i] = ~EPD.EPDbuffer[i];
		EPD.EPDbuffer[i - 1] = ~EPD.EPDbuffer[i - 1];
		EPD.EPDbuffer[i - 2] = ~EPD.EPDbuffer[i - 2];
		EPD.EPDbuffer[i - 3] = ~EPD.EPDbuffer[i - 3];
	}
	for (int x = 92; x < 96; x++)
		EPD.DrawXline(85, 295, x);
	EPD.EPD_Dis_Part(0, 127, 0, 295, (unsigned char *)EPD.EPDbuffer, 1);
	//EPD.EPD_Dis_Full((unsigned char *)EPD.EPDbuffer,1);
	driver_delay_xms(DELAYTIME);

}


void updateData()
{
	Serial.print("Syncing time from ntp server...");
	ntpClient.update();
	Serial.println();
	Serial.println(String("time synced at ") + ntpClient.getFormattedTime());
	time_t now = ntpClient.getRawTime();
	struct tm* timeInfo = localtime(&now);
	nowInfo.tm_sec = timeInfo->tm_sec;
	nowInfo.tm_min = timeInfo->tm_min;
	nowInfo.tm_hour = timeInfo->tm_hour;
	nowInfo.tm_mday = timeInfo->tm_mday;
	nowInfo.tm_mon = timeInfo->tm_mon;
	nowInfo.tm_year = timeInfo->tm_year;
	nowInfo.tm_wday = timeInfo->tm_wday;
	nowInfo.tm_yday = timeInfo->tm_yday;
	nowInfo.tm_isdst = timeInfo->tm_isdst;


	byte rtc_mem[4]; rtc_mem[0] = 126;
	byte Hours = ntpClient.getHours().toInt();
	Serial.println("hour");
	Serial.print(Hours);

	// long sleep during the none update time
	if (Hours == end_time) {
		if ((start_time - end_time) < 0)
			rtc_mem[1] = (24 - Hours + start_time) * 60 / sleeptime;
		else
			rtc_mem[1] = (start_time - Hours) * 60 / sleeptime;
		ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
		Serial.println("rtc_mem[1]");
		Serial.println(rtc_mem[1]);
	}

	Serial.println("Updating conditions...");
	OpenWeatherMapCurrent *currentWeatherClient = new OpenWeatherMapCurrent();
	currentWeatherClient->setMetric(IS_METRIC);
	currentWeatherClient->setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
	currentWeatherClient->updateCurrentById(&currentWeather, apiKey, locID);
	delete currentWeatherClient;
	currentWeatherClient = nullptr;

	if (LANG == 1) //upercase the first character
		currentWeather.description[0] -= 32;

//Update weather
	Serial.println("Updating forecasts...");
	OpenWeatherMapForecast *forecastClient = new OpenWeatherMapForecast();
	forecastClient->setMetric(IS_METRIC);
	forecastClient->setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
	uint8_t allowedHours[] = { 0, 3, 6, 8, 12, 15, 18, 21 };

	forecastClient->setAllowedHours(allowedHours, sizeof(allowedHours));
	forecastClient->updateForecastsById(forecasts, apiKey, locID, MAX_FORECASTS);
	delete forecastClient;
	forecastClient = nullptr;

	for (int i = 0; i < MAX_FORECASTS; i++)
		if (LANG == 1) //upercase the first character
			forecasts[i].description[0] -= 32;

	lastUpdate = ntpClient.getFormattedTime();
}

void configModeCallback(WiFiManager *myWiFiManager)
{
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

	/*attention! calibrate it yourself */
	if (batt_voltage <= 2.9) {
		EPD.clearbuffer();
		EPD.DrawXbm_P(39, 98, 100, 50, (unsigned char *)needcharge);
		always_sleep();
	}
	if (batt_voltage > 2.9 && batt_voltage <= 2.95) EPD.DrawXbm_P(x, y, 20, 10, (unsigned char *)batt_1);
	if (batt_voltage > 2.95 && batt_voltage <= 3.0) EPD.DrawXbm_P(x, y, 20, 10, (unsigned char *)batt_2);
	if (batt_voltage > 3.0 && batt_voltage <= 3.05) EPD.DrawXbm_P(x, y, 20, 10, (unsigned char *)batt_3);
	if (batt_voltage > 3.05 && batt_voltage <= 3.1) EPD.DrawXbm_P(x, y, 20, 10, (unsigned char *)batt_4);
	if (batt_voltage > 3.1) EPD.DrawXbm_P(x, y, 20, 10, (unsigned char *)batt_5);
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
	if (rtc_mem[2] != 126) {
		Serial.println("first time to run check config");
		rtc_mem[2] = 126;
		ESP.rtcUserMemoryWrite(4, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
	}

}
void check_rtc_mem()
{
	/*
	   rtc_mem[0] sign for first run
	   rtc_mem[1] how many hours left
	 */
	byte rtc_mem[4];

	ESP.rtcUserMemoryRead(0, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
	if (rtc_mem[0] != 126) {
		Serial.println("first time to run");
		rtc_mem[0] = 126;
		rtc_mem[1] = 0;
		ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
	}else {
		if (rtc_mem[1] > 0) {
			rtc_mem[1]--;
			Serial.println("don't need to update weather");
			Serial.println(rtc_mem[1]);
			ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtc_mem, sizeof(rtc_mem));
			EPD.deepsleep();
			ESP.deepSleep(60 * sleeptime * 1000000);
		}
	}
}
