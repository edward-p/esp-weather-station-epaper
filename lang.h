#ifndef __LANG_H
#define __LANG_H

#define LANG 1  //1 English 2 Chinees
const String WDAY_NAMES[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
const String MONTH_NAMES[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

#if (LANG == 1)

const String OPEN_WEATHER_MAP_LANGUAGE = "en";
extern const String strWindDirection[] = {
	"N",
	"NNE",
	"NE",
	"ENE",
	"E",
	"ESE",
	"SE",
	"SSE",
	"S",
	"SSW",
	"SW",
	"WSW",
	"W",
	"WNW",
	"WN",
	"NWN"
};
const char* strHumidity = "HUM";
const char* strPressure = "Pressure";
const char* strLastUpdate = "Updated at";
const char* config_page_line1 = "Connect WIFI ";
const char* config_page_line2 = "Weather widget";
const char* config_page_line3 = "Open any website";
const char* config_page_line4 = "to enter config page";
const char* config_page_line5 = "Input wifi name , password";
const char* config_page_line6 = "and city for weather";
const char* config_timeout_line1 = "Credentials not valid or config timeout";
const char* config_timeout_line2 = "① Turn off the power";
const char* config_timeout_line3 = "② Hold reset for seconds";
const char* config_timeout_line4 = "③ Turn on the power";
#endif
#if (LANG == 2)
const String OPEN_WEATHER_MAP_LANGUAGE = "zh";
extern const String strWindDirection[] = {
	"北",
	"北东北",
	"东北",
	"东东北",
	"东",
	"东东南",
	"东南",
	"南东南",
	"南",
	"南西南",
	"西南",
	"西西南",
	"西",
	"西西北",
	"西北",
	"北西北"
};
const char* strHumidity = "湿度";
const char* strPressure = "气压";
const char* strLastUpdate = "更新于";
const char* config_page_line1 = "请用手机连接wifi";
const char* config_page_line2 = "weather widget";
const char* config_page_line3 = "浏览器打开任意网页";
const char* config_page_line4 = "进入配置页面";
const char* config_page_line5 = "输入用于天气更新的wifi";
const char* config_page_line6 = "名称，密码和城市(拼音)";
const char* config_timeout_line1 = "配置无效或配置超时";
const char* config_timeout_line2 = "① 关闭电源";
const char* config_timeout_line3 = "② 按住reset键数秒";
const char* config_timeout_line4 = "③ 开启电源重新进入配置模式";
#endif

#endif /* end of include guard: __LANG_H */
