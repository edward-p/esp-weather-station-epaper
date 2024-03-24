use std::collections::HashMap;

use crate::heweather::{HeAQI, HeLocation, HeWeatherData, HeWeatherNow};
use log::info;
use rocket::serde::{Deserialize, Serialize};

lazy_static! {
    pub static ref ICON_MAP: HashMap<&'static str, &'static str> = {
        let mut m = HashMap::new();
        // https://dev.qweather.com/en/docs/resource/icons/#weather-icons
        // use http://javl.github.io/image2cpp/ to convert
        m.insert("100",	"0");	//Sunny
        m.insert("101",	"1");	//Cloudy
        m.insert("102",	"2");	//Few Clouds
        m.insert("103",	"3");	//Partly Cloudy
        m.insert("104",	"4");	//Overcast
        m.insert("150",	"5");	//Clear
        m.insert("151",	"6");	//Cloudy
        m.insert("152",	"7");	//Few Clouds
        m.insert("153",	"8");	//Partly Cloudy
        m.insert("300",	"9");	//Shower Rain
        m.insert("301",	"10");	//Heavy Shower Rain
        m.insert("302",	"11");	//Thundershower
        m.insert("303",	"12");	//Heavy Thunderstorm
        m.insert("304",	"13");	//Hail
        m.insert("305",	"14");	//Light Rain
        m.insert("306",	"15");	//Moderate Rain
        m.insert("307",	"16");	//Heavy Rain
        m.insert("308",	"17");	//Extreme Rain
        m.insert("309",	"18");	//Drizzle Rain
        m.insert("310",	"19");	//Rainstorm
        m.insert("311",	"20");	//Heavy Rainstorm
        m.insert("312",	"21");	//Severe Rainstorm
        m.insert("313",	"22");	//Freezing Rain
        m.insert("314",	"23");	//Light to Moderate Rain
        m.insert("315",	"24");	//Moderate to Heavy Rain
        m.insert("316",	"25");	//Heavy Rain to Rainstorm
        m.insert("317",	"26");	//Rainstorm to Heavy Rainstorm
        m.insert("318",	"27");	//Heavy to Severe Rainstorm
        m.insert("350",	"28");	//Shower Rain
        m.insert("351",	"29");	//Heavy Shower Rain
        m.insert("399",	"30");	//Rain
        m.insert("400",	"31");	//Light Snow
        m.insert("401",	"32");	//Moderate Snow
        m.insert("402",	"33");	//Heavy Snow
        m.insert("403",	"34");	//Snowstorm
        m.insert("404",	"35");	//Sleet
        m.insert("405",	"36");	//Rain and Snow
        m.insert("406",	"37");	//Shower Rain and Snow
        m.insert("407",	"38");	//Snow Flurry
        m.insert("408",	"39");	//Light to Moderate Snow
        m.insert("409",	"40");	//Moderate to Heavy Snow
        m.insert("410",	"41");	//Heavy Snow to Snowstorm
        m.insert("456",	"42");	//Shower Rain and Snow
        m.insert("457",	"43");	//Snow Flurry
        m.insert("499",	"44");	//Snow
        m.insert("500",	"45");	//Mist
        m.insert("501",	"46");	//Fog
        m.insert("502",	"47");	//Haze
        m.insert("503",	"48");	//Sand
        m.insert("504",	"49");	//Dust
        m.insert("507",	"50");	//Duststorm
        m.insert("508",	"51");	//Sandstorm
        m.insert("509",	"52");	//Dense Fog
        m.insert("510",	"53");	//Strong Fog
        m.insert("511",	"54");	//Moderate Haze
        m.insert("512",	"55");	//Heavy Haze
        m.insert("513",	"56");	//Severe Haze
        m.insert("514",	"57");	//Heavy Fog
        m.insert("515",	"58");	//Extra Heavy Fog
        m.insert("900",	"59");	//Hot
        m.insert("901",	"60");	//Cold
        m.insert("999",	"61");	//Unknown
        m
    };
}

#[derive(Serialize, Deserialize, Debug)]
#[serde(crate = "rocket::serde")]
pub struct WeatherNow {
    cond: String,
    cond_index: String,
    txt: String,
    hum: String,
    tmp: String,
    feels_like: String,
    dir: String,
    sc: String,
    sp: String,
}

impl From<&HeWeatherNow> for WeatherNow {
    fn from(value: &HeWeatherNow) -> Self {
        Self {
            cond: value.icon().clone(),
            cond_index: ICON_MAP.get(value.icon().as_str()).unwrap().to_string(),
            txt: value.text().clone(),
            hum: value.humidity().clone(),
            tmp: value.temp().clone(),
            feels_like: value.feels_like().clone(),
            dir: value.wind_dir().clone(),
            sc: value.wind_scale().clone(),
            sp: value.wind_speed().clone(),
        }
    }
}

#[derive(Serialize, Deserialize)]
#[serde(crate = "rocket::serde")]
pub struct WeatherData {
    cond_d: String,
    cond_d_index: String,
    cond_n: String,
    cond_n_index: String,
    txt_d: String,
    txt_n: String,
    tmpmin: String,
    tmpmax: String,
    dir_d: String,
    dir_n: String,
    sc_d: String,
    sc_n: String,
    sp_d: String,
    sp_n: String,
}

impl From<&HeWeatherData> for WeatherData {
    fn from(value: &HeWeatherData) -> Self {
        info!("icon_day: {}", value.icon_day());
        info!("icon_night: {}", value.icon_night());
        Self {
            cond_d: value.icon_day().clone(),
            cond_d_index: ICON_MAP
                .get(value.icon_day().as_str())
                .unwrap_or(&"61")
                .to_string(),
            cond_n: value.icon_night().clone(),
            cond_n_index: ICON_MAP
                .get(value.icon_night().as_str())
                .unwrap_or(ICON_MAP.get(value.icon_day().as_str()).unwrap_or(&"61"))
                .to_string(),
            txt_d: value.text_day().clone(),
            txt_n: value.text_night().clone(),
            tmpmin: value.temp_min().clone(),
            tmpmax: value.temp_max().clone(),
            dir_d: value.wind_dir_day().clone(),
            dir_n: value.wind_dir_night().clone(),
            sc_d: value.wind_scale_day().clone(),
            sc_n: value.wind_scale_night().clone(),
            sp_d: value.wind_speed_day().clone(),
            sp_n: value.wind_speed_night().clone(),
        }
    }
}

#[derive(Deserialize, Serialize)]
#[serde(crate = "rocket::serde")]
pub struct Weather {
    aqi: HeAQI,
    now: WeatherNow,
    today: WeatherData,
    tomorrow: WeatherData,
    thedayaftertomorrow: WeatherData,
    city: String,
}

impl Weather {
    pub fn new(
        location: &HeLocation,
        aqi: &HeAQI,
        now: &HeWeatherNow,
        daily: [&HeWeatherData; 3],
    ) -> Self {
        Self {
            aqi: aqi.clone(),
            city: location.name().clone(),
            now: now.into(),
            today: daily[0].into(),
            tomorrow: daily[1].into(),
            thedayaftertomorrow: daily[2].into(),
        }
    }
}
