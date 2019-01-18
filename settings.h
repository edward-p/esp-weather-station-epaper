/***************************
   Settings
 **************************/
#pragma once

#define UTC_OFFSET +8
const int sleeptime = 30;                       //updating interval 71min maximum
byte end_time = 1;                              //time that stops to update weather forecast
byte start_time = 7;                            //time that starts to update weather forecast
const char* ntpServer = "ntp1.aliyun.com";      //ntp server

// values in metric or imperial system?
const boolean IS_METRIC = true;
const String degreeSign = (IS_METRIC ? "°C" : "°F");
const uint8_t MAX_FORECASTS = 5;
