/** =========================================================================
 * @example{lineno} alarm.ino
 * @brief Demonstrates receiving every-second and every-minute DS3231 alarm interrupts.
 * ======================================================================= */

#include <RTC_DS3231.h>

#if defined(ARDUINO_ARCH_AVR)
#include <EnableInterrupt.h>
#endif

const uint8_t alarmInterruptPin = 31;
const uint8_t expectedSecondAlarms = 15;
const uint8_t expectedMinuteAlarms = 2;
const uint8_t futureAlarmTestCount = 5;
const uint8_t futureAlarmLeadSeconds = 5;

const unsigned long secondTestDurationMs = 15000UL;
const unsigned long minuteTestDurationMs = 120000UL;
const unsigned long futureSecondsAlarmTimeoutMs = 7000UL;
const unsigned long futureMinutesAlarmTimeoutMs = 70000UL;

DS3231 rtc;

volatile uint8_t alarmInterruptCount = 0;

void clockAlarmISR() {
  ++alarmInterruptCount;
}

void attachClockAlarmInterrupt() {
  pinMode(alarmInterruptPin, INPUT_PULLUP);
#if defined(ARDUINO_ARCH_AVR)
  enableInterrupt(alarmInterruptPin, clockAlarmISR, FALLING);
#else
  attachInterrupt(digitalPinToInterrupt(alarmInterruptPin), clockAlarmISR, FALLING);
#endif
}

void resetAlarmInterruptCount() {
  noInterrupts();
  alarmInterruptCount = 0;
  interrupts();
}

void reportAlarmInterrupts(uint8_t& reportedCount) {
  uint8_t currentCount = alarmInterruptCount;
  if (currentCount == reportedCount) return;

  // The DS3231 holds INT/SQW low until the Alarm 1 flag is cleared.
  rtc.clearAlarm();
  while (reportedCount < currentCount) {
    ++reportedCount;
    Serial.println(F("Clock alarm detected"));
  }
}

bool runAlarmTest(unsigned long timeoutMs, uint8_t expectedCount) {
  uint8_t reportedCount = 0;
  unsigned long startTime = millis();

  while (millis() - startTime < timeoutMs) {
    reportAlarmInterrupts(reportedCount);
  }

  rtc.disableAlarm();
  rtc.clearAlarm();
  reportAlarmInterrupts(reportedCount);

  Serial.print(F("Received "));
  Serial.print(alarmInterruptCount);
  Serial.print(F(" alarm interrupts; expected "));
  Serial.println(expectedCount);

  bool passed = alarmInterruptCount == expectedCount;
  Serial.println(passed ? F("PASS") : F("FAIL"));
  return passed;
}

bool runFutureMinutesAlarmTest(uint8_t testNumber) {
  tm currentTime;
  if (!rtc.disableAlarm() || !rtc.clearAlarm() || !rtc.getTime(currentTime)) {
    Serial.println(F("Could not read the time before setting the alarm"));
    return false;
  }
  Serial.print(F("Current time: "));
  Serial.print(currentTime.tm_hour);
  Serial.write(':');
  if (currentTime.tm_min < 10) Serial.write('0');
  Serial.print(currentTime.tm_min);
  Serial.write(':');
  if (currentTime.tm_sec < 10) Serial.write('0');
  Serial.println(currentTime.tm_sec);

  uint8_t alarmSecond = 0;
  uint8_t alarmMinute = currentTime.tm_min + 1;
  uint8_t alarmHour = currentTime.tm_hour;
  if (alarmMinute >= 60) {
    alarmMinute = 0;
    alarmHour = (alarmHour + 1) % 24;
  }

  resetAlarmInterruptCount();
  if (!rtc.setDailyAlarm(alarmHour, alarmMinute, alarmSecond) || !rtc.enableAlarm()) {
    Serial.println(F("Could not configure the future alarm"));
    return false;
  }

  Serial.print(F("Future alarm test "));
  Serial.print(testNumber);
  Serial.print(F(": waiting for "));
  Serial.print(alarmHour);
  Serial.write(':');
  if (alarmMinute < 10) Serial.write('0');
  Serial.print(alarmMinute);
  Serial.write(':');
  if (alarmSecond < 10) Serial.write('0');
  Serial.println(alarmSecond);

  uint8_t reportedCount = 0;
  unsigned long startTime = millis();
  while (alarmInterruptCount == 0 && millis() - startTime < futureMinutesAlarmTimeoutMs) {
    reportAlarmInterrupts(reportedCount);
  }

  rtc.disableAlarm();
  rtc.clearAlarm();
  reportAlarmInterrupts(reportedCount);

  bool passed = alarmInterruptCount == 1;
  Serial.println(passed ? F("PASS") : F("FAIL"));
  return passed;
}

bool runFutureSecondsAlarmTest(uint8_t testNumber) {
  tm currentTime;
  if (!rtc.disableAlarm() || !rtc.clearAlarm() || !rtc.getTime(currentTime)) {
    Serial.println(F("Could not read the time before setting the alarm"));
    return false;
  }

  uint8_t alarmSecond = currentTime.tm_sec + futureAlarmLeadSeconds;
  uint8_t alarmMinute = currentTime.tm_min;
  uint8_t alarmHour = currentTime.tm_hour;
  if (alarmSecond >= 60) {
    alarmSecond -= 60;
    ++alarmMinute;
  }
  if (alarmMinute >= 60) {
    alarmMinute = 0;
    alarmHour = (alarmHour + 1) % 24;
  }

  resetAlarmInterruptCount();
  if (!rtc.setDailyAlarm(alarmHour, alarmMinute, alarmSecond) || !rtc.enableAlarm()) {
    Serial.println(F("Could not configure the future alarm"));
    return false;
  }

  Serial.print(F("Future alarm test "));
  Serial.print(testNumber);
  Serial.print(F(": waiting for "));
  Serial.print(alarmHour);
  Serial.write(':');
  if (alarmMinute < 10) Serial.write('0');
  Serial.print(alarmMinute);
  Serial.write(':');
  if (alarmSecond < 10) Serial.write('0');
  Serial.println(alarmSecond);

  uint8_t reportedCount = 0;
  unsigned long startTime = millis();
  while (alarmInterruptCount == 0 && millis() - startTime < futureSecondsAlarmTimeoutMs) {
    reportAlarmInterrupts(reportedCount);
  }

  rtc.disableAlarm();
  rtc.clearAlarm();
  reportAlarmInterrupts(reportedCount);

  bool passed = alarmInterruptCount == 1;
  Serial.println(passed ? F("PASS") : F("FAIL"));
  return passed;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.print(rtc.getMakeModel());
  Serial.println(F(" alarm interrupt example"));

  if (!rtc.begin() || !rtc.init()) {
    Serial.print(F("Cannot access the "));
    Serial.print(rtc.getMakeModel());
    Serial.print(F(" at "));
    Serial.println(rtc.getAddress());
    while (true);
  }

  attachClockAlarmInterrupt();

  bool secondTestPassed = true;
  if (!rtc.supports(AlarmMatch::MATCH_SECONDS)) {
    Serial.println(F("Every-second alarm not supported"));
  } else {
    Serial.println(F("\nTesting an alarm every second for 15 seconds"));
    resetAlarmInterruptCount();
    if (!rtc.disableAlarm() || !rtc.clearAlarm() || !rtc.setSecondAlarm() || !rtc.enableAlarm()) {
      Serial.println(F("Could not configure the every-second alarm"));
      while (true);
    }
    secondTestPassed = runAlarmTest(secondTestDurationMs, expectedSecondAlarms);
  }

  bool futureSecondsTestsPassed = true;
  if (!rtc.supports(AlarmMatch::MATCH_SECONDS)) {
    Serial.println(F("Seconds matching alarm not supported for future second matching"));
  } else {
    Serial.println(F("\nTesting an alarm set 5 seconds in the future 5 times"));
    for (uint8_t testNumber = 1; testNumber <= futureAlarmTestCount; ++testNumber) {
      futureSecondsTestsPassed &= runFutureSecondsAlarmTest(testNumber);
    }
  }

  bool everyMinuteTestPassed = true;
  if (!rtc.supports(AlarmMatch::MATCH_SECONDS)) {
    Serial.println(F("Seconds matching alarm not supported for every-minute alarm"));
  } else {
    Serial.println(F("\nTesting an alarm every minute for 2 minutes"));
    resetAlarmInterruptCount();
    if (!rtc.setMinuteAlarm(0) || !rtc.enableAlarm()) {
      Serial.println(F("Could not configure the every-minute alarm"));
      while (true);
    }
    everyMinuteTestPassed = runAlarmTest(minuteTestDurationMs, expectedMinuteAlarms);
  }


  bool futureMinutesTestsPassed = true;
  if (!rtc.supports(AlarmMatch::MATCH_MINUTES)) {
    Serial.println(F("Minutes matching alarm not supported for future minute matching"));
  } else {
    Serial.println(F("\nTesting an alarm set for the next minute"));
    for (uint8_t testNumber = 1; testNumber <= futureAlarmTestCount; ++testNumber) {
      futureMinutesTestsPassed &= runFutureMinutesAlarmTest(testNumber);
    }
  }

  Serial.println();
  Serial.println(secondTestPassed && everyMinuteTestPassed && futureSecondsTestsPassed && futureMinutesTestsPassed ?
                   F("All alarm tests passed") :
                   F("One or more tests failed"));
}

void loop() {}
