#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "heweather.h"

heweatherclient::heweatherclient(String Serverurl, int serverPort, const char* langstring)
{
  server = Serverurl;
  port = serverPort;
  lang = langstring;
}

void heweatherclient::whitespace(char c) {
  // Serial.println("whitespace");
}

void heweatherclient::startDocument() {
  //Serial.println("start document");
}

void heweatherclient::key(String key) {
  currentKey = key;

  // Serial.println("key: " + key);
}

void heweatherclient::value(String value) {

  if (currentParent == "aqi")
  {
    if (currentKey == "aqi") aqi = value;
    if (currentKey == "level") level = value;
    if (currentKey == "category") category = value;
    if (currentKey == "primary") primary = value;
    if (currentKey == "pm10") pm10 = value;
    if (currentKey == "pm2p5") pm2p5 = value;
    if (currentKey == "no2") no2= value;
    if (currentKey == "so2") so2 = value;
    if (currentKey == "co") co = value;
    if (currentKey == "o3") o3 = value;
  }
  
  if (currentParent == "now")
  {
    if (currentKey == "cond") now_cond = value;
    if (currentKey == "cond_index")
    {
      now_cond_index = value;
      if (value.toInt() >= 19 && value.toInt() <= 40)  rain = 1;
    }
    if (currentKey == "hum") now_hum = value;
    if (currentKey == "tmp") now_tmp = value;
    if (currentKey == "feels_like") now_feels_like = value;
    if (currentKey == "dir") now_dir = value;
    if (currentKey == "sc") now_sc = value;
    if (currentKey == "txt") now_txt = value;
  }
  if (currentParent == "today")
  {
    if (currentKey == "cond_d") today_cond_d = value;
    if (currentKey == "cond_d_index") {
      today_cond_d_index = value;
      if (value.toInt() >= 19 && value.toInt() <= 40)  rain = 1;
    }
    if (currentKey == "cond_n") today_cond_n = value;
    if (currentKey == "cond_n_index") {
      today_cond_n_index = value;
      if (value.toInt() >= 19 && value.toInt() <= 40)  rain = 1;
    }
    if (currentKey == "tmpmax") today_tmp_max = value;
    if (currentKey == "tmpmin") today_tmp_min = value;
    if (currentKey == "txt_d") today_txt_d = value;
    if (currentKey == "txt_n") today_txt_n = value;
  }

  if (currentParent == "tomorrow")
  {
    if (currentKey == "cond_d") tomorrow_cond_d = value;
    if (currentKey == "cond_d_index") {
      tomorrow_cond_d_index = value;
      if (value.toInt() >= 19 && value.toInt() <= 40)  rain = 1;
    }

    if (currentKey == "cond_n") tomorrow_cond_n = value;
    if (currentKey == "cond_n_index") tomorrow_cond_n_index = value;
    if (currentKey == "tmpmax") tomorrow_tmp_max = value;
    if (currentKey == "tmpmin") tomorrow_tmp_min = value;
    if (currentKey == "txt_d") tomorrow_txt_d = value;
    if (currentKey == "txt_n") tomorrow_txt_n = value;
  }

  if (currentParent == "thedayaftertomorrow")
  {
    if (currentKey == "cond_d") thedayaftertomorrow_cond_d = value;
    if (currentKey == "cond_d_index") {
      thedayaftertomorrow_cond_d_index = value;
      if (value.toInt() >= 19 && value.toInt() <= 40)  rain = 1;
    }

    if (currentKey == "cond_n") thedayaftertomorrow_cond_n = value;
    if (currentKey == "cond_n_index") thedayaftertomorrow_cond_n_index = value;
    if (currentKey == "tmpmax") thedayaftertomorrow_tmp_max = value;
    if (currentKey == "tmpmin") thedayaftertomorrow_tmp_min = value;
    if (currentKey == "txt_d") thedayaftertomorrow_txt_d = value;
    if (currentKey == "txt_n") thedayaftertomorrow_txt_n = value;
  }
  
  if (currentKey = "message") message = value;
  //Serial.println("value: " + value);
}

void heweatherclient::endArray() {
  //Serial.println("end array. ");
}

void heweatherclient::endObject() {
  // Serial.println("end object. ");
  currentParent = "";
}

void heweatherclient::endDocument() {
  //Serial.println("end document. ");
}

void heweatherclient::startArray() {
  // Serial.println("start array. ");
}

void heweatherclient::startObject() {
  // Serial.println("start object. ");
  currentParent = currentKey;
}
void heweatherclient::update()
{
  rain = 0;
  JsonStreamingParser parser;
  parser.setListener(this);
  WiFiClient client;
  const int httpPort = port;
  if (!client.connect(server, port)) {
    Serial.println("connection failed");
    return;
  }

  Serial.print("Requesting URL: ");

  // This will send the request to the server
  client.print(String("GET /weather?city=") + city + "&lang=" + lang + "&client_name=" + client_name + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "Connection: close\r\n" +
               "\r\n" );


  int pos = 0;
  boolean isBody = false;
  char c;

  int size = 0;
  client.setNoDelay(false);
  while (client.connected()) {
    Serial.println(client.available());
    delay(500);
    while (client.available()) {
      c = client.read();
      Serial.print(c);
      if (c == '{' || c == '[') {
        isBody = true;
      }
      if (isBody) {
        parser.parse(c);
      }
    }
  }
}
