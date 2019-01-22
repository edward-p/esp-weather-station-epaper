#ifndef  __WEATHERDATA_H
#define __WEATHERDATA_H

typedef struct {
	String description;
	String icon;
	String cityName;
	float temp;
	float windDeg;
	float windSpeed;
	uint16_t pressure;
	uint8_t humidity;
} currentweather_t;

typedef struct {
	int wday;
	String icon;
	String description;
	float dayMaxTemp;
	float dayMinTemp;
	float windDeg;
	float windSpeed;
} forecasts_t;

#endif /* end of include guard: __WEATHERDATA_H */
