use derive_getters::Getters;
use rocket::serde::{Deserialize, Serialize};

#[derive(Debug, Deserialize, Serialize, Getters)]
#[serde(crate = "rocket::serde")]
pub struct Config {
    heweather_key: String,
}

impl Default for Config {
    fn default() -> Config {
        Config {
            heweather_key: "".into(),
        }
    }
}
