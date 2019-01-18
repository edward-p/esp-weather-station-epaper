# esp8266-weather-station-epaper-OpenWeatherMap

Forked from [duck531a98/esp8266-weather-station-epaper](https://github.com/duck531a98/esp8266-weather-station-epaper) change weather source to OpenWeatherMap

## Weather Forecast Data

As your device will be displaying data from OpenWeatherMap you need an "API key" from them. It uniquely ties requests from your device(s) to your account and ensures that the number of requests remains within your allotted quota.

-   Go to <https://docs.thingpulse.com/go/create-openweathermap-key>.
-   Take note of the features in the "Free" column. By using the free plan you are limited to 60 - - calls per minute which is just enough for personal usage.
-   In the "Free" column click on "Get API key and Start". 
-   You need also get the city id of your location which can be found in url like:<https://openweathermap.org/city/2643743>. The `2643743` is the ID of London.

## Summary

Esp8266 is programed to display weather forecast on 2.9inch e-paper.
You can get a 2.9inch e-paper display in Waveshare's shop. Buy it on taobao.com if you are in China. <https://detail.tmall.com/item.htm?id=550690109675&spm=a1z09.2.0.0.nyL5N4&_u=q2skmgl30cb>

Esp8266 is in deep sleeping after update the weather forecast to save battery.

![](https://github.com/duck531a98/esp8266-weather-station-epaper/raw/master/pics/20170715_113425.jpg)

![](https://github.com/duck531a98/esp8266-weather-station-epaper/raw/master/pics/20170715_152231.jpg)

![](https://github.com/duck531a98/esp8266-weather-station-epaper/raw/master/pics/20170715_152306.jpg)

## Dependents

[json-streaming-parser](https://github.com/squix78/json-streaming-parser) by _Daniel Eichhorn_

[WiFiManager](https://github.com/tzapu/WiFiManager) by _tzapu_

[esp8266-weather-station](https://github.com/ThingPulse/esp8266-weather-station) by _ThingPulse_

## BOM

1.  2.9inch e-paper from Waveshare
2.  Nodemcu or Wemos or integrated PCB( gerber files in /PCB folder)
3.  Li-Po battery
4.  3d printed case(STL files in /3d folder still in revising )

## Uploading code to esp8266

Add esp8266 to boards manager in arduino ide. Follow this guide <https://github.com/esp8266/Arduino#installing-with-boards-manager>.
Don't forget to upload font files. Follow this guide to upload font files to spiff meomory. <https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#uploading-files-to-file-system>.

## Multi language support

modify the language in lang.h 
There is Strings for Chinese and English already.
You can add you own.
Weahter data supports zh,en,de,es,fr,it,jp,kr,ru,in,th

## Connect esp8266 to display module

-   BUSY---->gpio4
-   RST---->gpio2
-   DC---->gpio5
-   CS---->gpio15
-   CLK---->gpio14
-   DIN---->gpio13

Connect gpio16 to rst on esp8266 to wakeup esp8266 from deep sleeping with internal timer.

There will be two versions of my pcb. One without epaper driving components and One with driving components. I'm still testing them to make sure they are reliable. Also Mike is making that board, too.

## Font

_Hui Lu_ developped this tool to generate your own font (unicode ucs-2)
<https://github.com/duck531a98/font-generator>

Since arduino ide is compiling the code to utf-8 charset. There is a internal function to convert utf-8 to unicode ucs-2. Display strings typed in to arduino ide with function DrawUTF.

Font files should be uploaded into spiff memory of esp8266.

A full unicode ucs-s font with 16x16 size will be 2MB. So if you don't need to display multi language, just convert ascii characters.

## Low power issues

Since regulator AMS1117 and CP2102 have a quiescent current about several mA, so nodemcu is not suitable for battery powered.

A low quiescent current LDO is needed. I'm testing HT7333 now, see how long could my board works with a 400mAh battery. Esp8266 consumes about 20-25uA in deep sleeping mode.

## Thanks

Thanks to Mike Daniel Fred. They led me into the world of esp8266. Thanks to their work on esp8266 weahter stations, wifi manager, json parser and so on. Thanks to Mike, for now he's the only one which I could share and ask for help about electronic stuffs.
