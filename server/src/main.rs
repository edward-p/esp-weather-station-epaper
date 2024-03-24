use chrono::Utc;
use dashmap::DashMap;
use rocket::fairing::AdHoc;
use rocket::{serde::json::Json, State};

use eink_weather_server::config::Config;
use eink_weather_server::heweather::{get_aqi, get_weather_3d, get_weather_now};
use eink_weather_server::{
    heweather::{get_location, LocationMap},
    weather::Weather,
};
use figment::{
    providers::{Env, Format, Serialized, Toml},
    Figment, Profile,
};

#[macro_use]
extern crate rocket;

#[get("/time")]
async fn time() -> String {
    let now = Utc::now();
    format!("date: {}", now.format("%a, %d %b %Y %H:%M:%S GMT"))
}

#[get("/weather?<city>&<lang>&<client_name>")]
async fn weather(
    city: String,
    lang: Option<String>,
    client_name: Option<String>,
    location_map: &State<LocationMap>,
    config: &State<Config>,
) -> Option<Json<Weather>> {
    info!("city: {:?}", city);
    info!("lang: {:?}", lang);
    info!("client_name: {:?}", client_name);

    if city.is_empty() {
        return None;
    }

    let lang = lang.unwrap_or("zh".into());

    let key = config.heweather_key();
    let location = match location_map.map.get(&city) {
        Some(l) => l.clone(),
        None => {
            let l = get_location(key, &city).await.unwrap();
            location_map.map.insert(city, l.clone());
            l
        }
    };

    let now = get_weather_now(key, location.id(), &lang).await.unwrap();
    let aqi = get_aqi(key, location.id(), &lang).await.unwrap();
    let daily = get_weather_3d(key, location.id(), &lang).await.unwrap();
    Some(Json(Weather::new(
        &location,
        &aqi,
        &now,
        [&daily[0], &daily[1], &daily[2]],
    )))
}

#[rocket::main]
async fn main() -> Result<(), rocket::Error> {
    let figment = Figment::from(rocket::Config::default())
        .merge(Serialized::defaults(Config::default()))
        .merge(Toml::file("App.toml").nested())
        .merge(Env::prefixed("APP_").global())
        .select(Profile::from_env_or("APP_PROFILE", "default"));

    let _rocket = rocket::custom(figment)
        .manage(LocationMap {
            map: DashMap::new(),
        })
        .attach(AdHoc::config::<Config>())
        .mount("/", routes![time])
        .mount("/", routes![weather])
        .launch()
        .await?;

    Ok(())
}
