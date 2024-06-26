/**The MIT License (MIT)

  Copyright (c) 2015 by Daniel Eichhorn

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  See more at http://blog.squix.ch
*/
#include "TimeClient.h"

TimeClient::TimeClient(float utcOffset, String Serverurl, int serverPort) {
  myUtcOffset = utcOffset;
  server = Serverurl;
  port = serverPort;
}

void TimeClient::updateTime() {
  WiFiClient client;
  const int httpPort = port;
  Serial.println("updating time");

  if (!client.connect(server, port)) {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  client.print(String("GET /time") + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "Connection: keep\r\n" +
               "\r\n");
  client.setNoDelay(false);
  Serial.println(">>>Response: ");
  while (client.connected()) {
    while (client.available()) {
      String line = client.readStringUntil('\n'); Serial.println(line);
      line.toUpperCase();
      // example:
      // date: Thu, 19 Nov 2015 20:25:40 GMT
      if (line.startsWith("DATE: ")) {

        int parsedHours = line.substring(23, 25).toInt();
        int parsedMinutes = line.substring(26, 28).toInt();
        int parsedSeconds = line.substring(29, 31).toInt();
        Serial.println(String(parsedHours) + ":" + String(parsedMinutes) + ":" + String(parsedSeconds));

        localEpoc = (parsedHours * 60 * 60 + parsedMinutes * 60 + parsedSeconds);
        Serial.println(localEpoc);

        localMillisAtUpdate = millis();
        client.stop();
        break;
      }

    }


  }
}

String TimeClient::getHours() {
  if (localEpoc == 0) {
    return "--";
  }
  int hours = ((getCurrentEpochWithUtcOffset()  % 86400L) / 3600) % 24;
  if (hours < 10) {
    return "0" + String(hours);
  }
  return String(hours); // print the hour (86400 equals secs per day)

}
String TimeClient::getMinutes() {
  if (localEpoc == 0) {
    return "--";
  }
  int minutes = ((getCurrentEpochWithUtcOffset() % 3600) / 60);
  if (minutes < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    return "0" + String(minutes);
  }
  return String(minutes);
}
String TimeClient::getSeconds() {
  if (localEpoc == 0) {
    return "--";
  }
  int seconds = getCurrentEpochWithUtcOffset() % 60;
  if ( seconds < 10 ) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    return "0" + String(seconds);
  }
  return String(seconds);
}

String TimeClient::getFormattedTime() {
  return getHours() + ":" + getMinutes() + ":" + getSeconds();
}

long TimeClient::getCurrentEpoch() {
  return localEpoc + ((millis() - localMillisAtUpdate) / 1000);
}

long TimeClient::getCurrentEpochWithUtcOffset() {
  return long(round(getCurrentEpoch() + 3600 * myUtcOffset + 86400L)) % 86400L;
}
