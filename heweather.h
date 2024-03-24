/*
  heweather.h
*/
#pragma once
#include "JsonStreamingParser.h"
#include "JsonListener.h"


//JsonStreamingParser parser;
//heweatherListener listener;


class heweatherclient: public JsonListener {
  private:
    String currentKey ;
    String currentParent;
    String server;
    int port;
    const char* lang;
  public:
    heweatherclient(String Serverurl, int serverPort, const char* langstring);
    String aqi;
    String level;
    String category;
    String primary;
    String pm10;
    String pm2p5;
    String no2;
    String so2;
    String co;
    String o3;
    bool rain = 0;
    const char* client_name;

    String now_cond;
    String now_hum;
    String now_tmp;
    String now_feels_like;
    String now_cond_index;
    String now_dir;
    String now_sc;
    String now_txt;

    String today_cond_d;
    String today_cond_d_index;
    String today_cond_n;
    String today_cond_n_index;
    String today_tmp_max;
    String today_tmp_min;
    String today_txt_d;
    String today_txt_n;

    String tomorrow_cond_d;
    String tomorrow_cond_d_index;
    String tomorrow_cond_n;
    String tomorrow_cond_n_index;
    String tomorrow_tmp_max;
    String tomorrow_tmp_min;
    String tomorrow_txt_d;
    String tomorrow_txt_n;
    String message;

    String thedayaftertomorrow_cond_d;
    String thedayaftertomorrow_cond_d_index;
    String thedayaftertomorrow_cond_n;
    String thedayaftertomorrow_cond_n_index;
    String thedayaftertomorrow_tmp_max;
    String thedayaftertomorrow_tmp_min;
    String thedayaftertomorrow_txt_d;
    String thedayaftertomorrow_txt_n;
    // virtual void keys(String key);
    void update();
    String citystr; String date;
    String city;

    virtual void whitespace(char c);

    virtual void startDocument();

    virtual void key(String key);

    virtual void value(String value);

    virtual void endArray();

    virtual void endObject();

    virtual void endDocument();

    virtual void startArray();

    virtual void startObject();

};
