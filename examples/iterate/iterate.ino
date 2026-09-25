// Test sketch that iterates over a large set of RTCs and prints the time.

#include <RTC_I2C.h>
#include <RTC_DS1307.h>
#include <RTC_DS1337.h>
#include <RTC_DS3231.h>
#include <RTC_MCP79410.h>
#include <RTC_PCF8523.h>
#include <RTC_PCF8563.h>
#include <RTC_RS5C372.h>
#include <RTC_RV3028.h>
#include <RTC_RV3028U.h>
#include <RTC_RV3032.h>
#include <RTC_RV8523.h>
#include <RTC_RV8803.h>
#include <RTC_SD2405.h>

#define MAXRTC 13

DS1307 rtc0;
DS1337 rtc1;
DS3231 rtc2;
MCP79410 rtc3;
PCF8523 rtc4;
PCF8563 rtc5;
RS5C372 rtc6;
RV3028 rtc7;
RV3028U rtc8;
RV3032 rtc9;
RV8523 rtc10;
RV8803 rtc11;
SD2405 rtc12;

RTC_I2C *rtc[MAXRTC] = {&rtc0, &rtc1, &rtc2, &rtc3, &rtc4, &rtc5, &rtc6, &rtc7, &rtc8, &rtc9, &rtc10, &rtc11, &rtc12};
RTC_I2C *attachedRTC[MAXRTC] = {nullptr};
uint8_t n_attached = 0;


void setup(void) {
  Serial.begin(115200);
  Serial.println(F("Starting RTC iteration test...\n"));

  for (byte i = 0; i < MAXRTC; i++) {
    attachedRTC[i] = nullptr;
  }

  for (byte i = 0; i < MAXRTC; i++) {
    Serial.print(F("Initializing RTC"));
    Serial.print(i);
    Serial.print(F(": "));
    Serial.print(rtc[i]->getMakeModel());
    Serial.print(F(" at "));
    Serial.print(String(rtc[i]->getAddress(), HEX));
    Serial.println();
    bool success = rtc[i]->begin();
    Serial.print(F("    ..."));
    Serial.println(success ? "success" : "failure");
    if (success) {
      attachedRTC[n_attached++] = rtc[i];
    }
  }
  // Friday, September 25, 2026 at 7:05:00 PM UTC in Unix epoch time
  timestamp_t ts = 1790363100;
  tm timeParts;
  // convert to a tm object
  TimeUtils::utcTimeTToTm(TimeUtils::getTimeT(ts, 0, epochStart::unix_epoch), timeParts);

  for (byte i = 0; i < n_attached; i++) {
    attachedRTC[i]->setTime(timeParts);
    Serial.print(F("Set RTC"));
    Serial.print(i);
    Serial.print(F(": "));
    Serial.print(attachedRTC[i]->getMakeModel());
    Serial.print(F(" at "));
    Serial.print(String(attachedRTC[i]->getAddress(), HEX));
    Serial.print(" to ");
    showDate(timeParts);
    Serial.print(" ");
    showTime(timeParts);
    Serial.println();
  }
}

void loop(void) {
  Serial.println("\n---\n");
  tm timeParts;
  for (byte i = 0; i < n_attached; i++) {
    Serial.print(attachedRTC[i]->getMakeModel());
    Serial.print(F(" ("));
    Serial.print(i);
    Serial.print(F("): "));
    attachedRTC[i]->getTime(timeParts);
    showTime(timeParts);
    Serial.print(F("   "));
    showDate(timeParts);
    Serial.println();
  }
  delay(5000L);
}


void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void showTime(tm timeParts) {
  print2digits(timeParts.tm_hour);
  Serial.write(':');
  print2digits(timeParts.tm_min);
  Serial.write(':');
  print2digits(timeParts.tm_sec);
}


void showDate(tm timeParts) {
  Serial.print(timeParts.tm_mday);
  Serial.write('.');
  Serial.print(timeParts.tm_mon);
  Serial.write('.');
  Serial.print(1970 + timeParts.tm_year);
}
