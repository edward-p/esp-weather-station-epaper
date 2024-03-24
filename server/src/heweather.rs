use dashmap::DashMap;
use derive_getters::Getters;
use rocket::serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug, Clone, Getters)]
#[serde(crate = "rocket::serde")]
pub struct HeLocation {
    name: String,
    id: String,
}

#[derive(Serialize, Deserialize, Debug, Getters)]
#[serde(crate = "rocket::serde")]
pub struct HeLocationResponse {
    location: Vec<HeLocation>,
}

pub struct LocationMap {
    pub map: DashMap<String, HeLocation>,
}

pub async fn get_location(
    key: &String,
    location: &String,
) -> Result<HeLocation, Box<dyn std::error::Error>> {
    let url = format!(
        "https://geoapi.qweather.com/v2/city/lookup?key={}&location={}",
        key, location
    );
    let body = reqwest::get(url).await?.text().await?;
    let v: HeLocationResponse = serde_json::from_str(&body)?;
    Ok(v.location.first().unwrap().clone())
}

#[derive(Serialize, Deserialize, Debug, Clone, Getters)]
#[serde(crate = "rocket::serde", rename_all = "camelCase")]
pub struct HeWeatherNow {
    obs_time: String,
    temp: String,
    feels_like: String,
    icon: String,
    text: String,
    wind360: String,
    wind_dir: String,
    wind_scale: String,
    wind_speed: String,
    humidity: String,
    precip: String,
    pressure: String,
    vis: String,
    cloud: String,
    dew: String,
}

#[derive(Serialize, Deserialize, Debug, Getters)]
#[serde(crate = "rocket::serde", rename_all = "camelCase")]
pub struct HeWeatherNowResponse {
    now: HeWeatherNow,
}

pub async fn get_weather_now(
    key: &String,
    location: &String,
    lang: &String,
) -> Result<HeWeatherNow, Box<dyn std::error::Error>> {
    let url: String = format!(
        "https://devapi.qweather.com/v7/weather/now?key={}&location={}&lang{}",
        key, location, lang
    );
    let body = reqwest::get(url).await?.text().await?;
    let v: HeWeatherNowResponse = serde_json::from_str(&body)?;
    Ok(v.now)
}

#[derive(Serialize, Deserialize, Debug, Clone, Getters)]
#[serde(crate = "rocket::serde", rename_all = "camelCase")]
pub struct HeAQI {
    pub_time: String,
    aqi: String,
    level: String,
    category: String,
    primary: String,
    pm10: String,
    pm2p5: String,
    no2: String,
    so2: String,
    co: String,
    o3: String,
}

#[derive(Serialize, Deserialize, Debug, Getters)]
#[serde(crate = "rocket::serde", rename_all = "camelCase")]
pub struct HeAQIResponse {
    now: HeAQI,
}

pub async fn get_aqi(
    key: &String,
    location: &String,
    lang: &String,
) -> Result<HeAQI, Box<dyn std::error::Error>> {
    let url: String = format!(
        "https://devapi.qweather.com/v7/air/now?key={}&location={}&lang{}",
        key, location, lang
    );
    let body = reqwest::get(url).await?.text().await?;
    let v: HeAQIResponse = serde_json::from_str(&body)?;
    Ok(v.now)
}

#[derive(Serialize, Deserialize, Debug, Clone, Getters)]
#[serde(crate = "rocket::serde", rename_all = "camelCase")]
pub struct HeWeatherData {
    fx_date: String,
    sunrise: String,
    sunset: String,
    moonrise: String,
    moonset: String,
    moon_phase: String,
    moon_phase_icon: String,
    temp_max: String,
    temp_min: String,
    icon_day: String,
    text_day: String,
    icon_night: String,
    text_night: String,
    wind360_day: String,
    wind_dir_day: String,
    wind_scale_day: String,
    wind_speed_day: String,
    wind360_night: String,
    wind_dir_night: String,
    wind_scale_night: String,
    wind_speed_night: String,
    humidity: String,
    precip: String,
    pressure: String,
    vis: String,
    cloud: String,
    uv_index: String,
}
#[derive(Serialize, Deserialize, Debug, Getters)]
#[serde(crate = "rocket::serde", rename_all = "camelCase")]
pub struct HeWeatherDataResponse {
    daily: [HeWeatherData; 3],
}

pub async fn get_weather_3d(
    key: &String,
    location: &String,
    lang: &String,
) -> Result<[HeWeatherData; 3], Box<dyn std::error::Error>> {
    let url: String = format!(
        "https://devapi.qweather.com/v7/weather/3d?key={}&location={}&lang{}",
        key, location, lang
    );
    let body = reqwest::get(url).await?.text().await?;
    let v: HeWeatherDataResponse = serde_json::from_str(&body)?;
    Ok(v.daily)
}
