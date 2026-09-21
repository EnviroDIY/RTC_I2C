// Simple sketch for testing basic set/get methods

#include <RTC_DS3231.h>

DS3231 rtc;

void setup(void) {
  tm timeParts, timeParts1;
  Serial.begin(115200);
  while (!Serial);
  Serial.println(F("\n\rRTC Test"));

  if (!rtc.begin()) {
    Serial.println(F("Cannot access RTC"));
    while (1);
  }
  rtc.init();
  timeParts = tm{1, 1, 1, 1, 2, 3, 55};
  rtc.setTime(timeParts);
  rtc.getTime(timeParts1);
  if (mktime(&timeParts) != mktime(&timeParts1)) {
    Serial.println(F("Not able to set RTC"));
    while (1);
  }
  delay(5100);
  rtc.getTime(timeParts1);
  if (mktime(&timeParts) + 5 != mktime(&timeParts1)) {
    Serial.println(F("RTC does not advance"));
    while (1);
  }
  Serial.println(F("Everything in order"));
}

void loop() {}
