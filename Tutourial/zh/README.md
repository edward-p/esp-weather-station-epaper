## 基本环境搭建（导入ESP8266支持到Arduino）

[传送门-------->](http://www.windworkshop.cn/?p=758)

**ps:本人当前使用的是2.5.0-beta2版的ESP8266环境，不保证其他版本也能正常编译运行**

## 导入ESP8266FS工具

下载 [ESP8266FS-0.3.0.zip](https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.3.0/ESP8266FS-0.3.0.zip)

解压后放到 `我的文档\Arduino\tools\ESP8266FS\tool\esp8266fs.jar`

重启Arduino，应该能看到Tools菜单里面多了`ESP8266 Sketch Data Upload`

## 安装依赖库

打开`Tools`->`Manage Libraries..`依次搜索和安装：

-   json-streaming-parser
-   WiFiManager
-   esp8266-weather-station

检查库冲突，已知冲突的库如下：

-   `NTPClient`与`esp8266-weather-station`冲突，请删除`我的文档\Arduino\libraries\NTPClient`

## 上传

下载源码

-   下载最新的源代码 [openweathermap.zip](https://github.com/edward-p/esp-weather-station-epaper-openweathermap/archive/openweathermap.zip)
-   解压后用Arduino打开`esp-weather-station-epaper-openweathermap.ino`

进入刷写模式

-   关闭电源
-   按住内侧按钮不放，同时打开电源
-   连接数据线

选择开发板

-   `Tools`->`Board`->`NodeMCU 1.0`

-   `Tools`->`Flash Size`->`4M (3M SPIFFS)`

-   `Tools`->`Port`->`COM#`（依照你的端口）

上传字库

-   `Tools`->`ESP8266 Sketch Data Upload`

上传主程序

-   关闭电源
-   按住内侧按钮不放，同时打开电源
-   点击按钮 :arrow_right:

## 语言设置

`lang.h`

-   第1行:`#define LANG 1` 1表示英文，2表示中文

## 更新周期设置

`settings.h`

-   第7行`sleeptime`，更新周期，单位：分钟（最大71分钟）

-   第8行`end_time`，更新结束时刻，默认为1点

-   第9行`start_time`，更新开始时间，默认为7点

**ps:每次修改完后需要重新上传主程序**

## 重新设置地点、Wifi等

-   关闭电源
-   打开电源后，再按住内侧按钮不放
-   直到屏幕上出现配置提示

## 天气apiKey，Location ID获取

由于你的设备将要从OpenWeatherMap获取并显示天气数据，你需要一个OpenWeatherMap的“API key”，

-   请前往 <https://home.openweathermap.org/users/sign_up> 注册一个OpenWeatherMap帐号（因为人机验证是google提供，所以。。）
-   完成注册后你可以再<https://home.openweathermap.org/api_keys> 查看你的“API key”，他是一串32个字符的十六进制字符串
-   Location ID可以搜到城市之后在浏览器地址栏查看，比如<https://openweathermap.org/city/2643743>. `2643743` 就是伦敦的 Location ID.
-   免费用户的API一分钟请求次数不超过60次，是绝对够用的。
