/** =========================================================================
 * @example{lineno} alarm.ino
 * @brief Demonstrates receiving periodic countdown and calendar alarm interrupts from an RV3032.
 * ======================================================================= */

#include <RTC_RV3032.h>

#if defined(ARDUINO_ARCH_AVR)
#include <EnableInterrupt.h>
#endif

const uint8_t alarmInterruptPin = 31;

const uint8_t expectedCountdownAlarms = 10;
const uint16_t countdownAlarmPeriodSeconds = 2;
const unsigned long countdownTestTimeoutMs = 21500UL;

const uint8_t expectedSecondAlarms = 15;
const unsigned long secondTestDurationMs = 15000UL;

const uint8_t futureAlarmTestCount = 5;
const uint8_t futureAlarmLeadSeconds = 5;
const unsigned long futureSecondsAlarmTimeoutMs = 7000UL;

const uint8_t expectedMinuteAlarms = 2;
const unsigned long minuteTestDurationMs = 120000UL;
const unsigned long futureMinutesAlarmTimeoutMs = 70000UL;

RV3032 rtc;

volatile uint8_t alarmInterruptCount = 0;

/**
 * ISR for handling clock alarm interrupts.
 */
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

/**
 * Reset the alarm interrupt count to zero.
 */
void resetAlarmInterruptCount() {
  noInterrupts();
  alarmInterruptCount = 0;
  interrupts();
}

/**
 * Report any calendar alarm interrupts that have occurred since the last report.
 * @param reportedCount The number of calendar alarm interrupts that have already been reported.
 */
void reportAlarmInterrupts(uint8_t& reportedCount) {
  uint8_t currentCount = alarmInterruptCount;
  if (currentCount == reportedCount) return;

  // The RTC holds its interrupt output low until the calendar alarm flag is cleared.
  rtc.clearAlarm();
  while (reportedCount < currentCount) {
    ++reportedCount;
    Serial.println(F("Clock alarm detected"));
  }
}

/**
 * Report any countdown alarm interrupts that have occurred since the last report.
 * @param reportedCount The number of countdown alarm interrupts that have already been reported.
 */
void reportCountdownAlarmInterrupts(uint8_t& reportedCount) {
  uint8_t currentCount = alarmInterruptCount;
  if (currentCount == reportedCount) return;

  rtc.clearCountdownAlarm();
  while (reportedCount < currentCount) {
    ++reportedCount;
    Serial.println(F("Countdown alarm detected"));
  }
}

/**
 * Run the countdown alarm test.
 * Returns true if the test passes, false otherwise.
 */
bool runCountdownAlarmTest() {
  resetAlarmInterruptCount();
  if (!rtc.disableCountdownAlarm() || !rtc.clearCountdownAlarm() ||
      !rtc.setCountdownAlarm(countdownAlarmPeriodSeconds, CountdownTimerFrequency::HZ_1) ||
      !rtc.enableCountdownAlarm()) {
    Serial.println(F("Could not configure the countdown alarm"));
    return false;
  }

  uint8_t reportedCount = 0;
  unsigned long startTime = millis();
  while (alarmInterruptCount < expectedCountdownAlarms && millis() - startTime < countdownTestTimeoutMs) {
    reportCountdownAlarmInterrupts(reportedCount);
  }

  rtc.disableCountdownAlarm();
  rtc.clearCountdownAlarm();
  reportCountdownAlarmInterrupts(reportedCount);

  Serial.print(F("Received "));
  Serial.print(alarmInterruptCount);
  Serial.print(F(" countdown interrupts; expected "));
  Serial.println(expectedCountdownAlarms);

  bool passed = alarmInterruptCount == expectedCountdownAlarms;
  Serial.println(passed ? F("PASS") : F("FAIL"));
  return passed;
}

/**
 * Run the alarm test for the specified timeout and expected count.
 * Returns true if the test passes, false otherwise.
 * @param timeoutMs The maximum time to wait for the alarm interrupts.
 * @param expectedCount The number of alarm interrupts expected.
 */
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

/**
 * Run the future minutes alarm test.
 * Returns true if the test passes, false otherwise.
 * @param testNumber The test number for reporting purposes.
 */
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

/**
 * Run the future seconds alarm test.
 * Returns true if the test passes, false otherwise.
 * @param testNumber The test number for reporting purposes.
 */
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


/** ===========================================================================
 * Setup function for the alarm interrupt example.
 * Initializes the RTC, attaches the alarm interrupt, and runs various alarm tests.
 =========================================================================== */
void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.print(rtc.getMakeModel());
  Serial.println(F(" alarm interrupt example\n"));

  if (!rtc.begin() || !rtc.init()) {
    Serial.print(F("Cannot access the "));
    Serial.print(rtc.getMakeModel());
    Serial.print(F(" at "));
    Serial.println(rtc.getAddress());
    while (true);
  }

  attachClockAlarmInterrupt();

  bool countdownTestPassed = true;
  if (!rtc.supports(RTCCapability::RTC_CAP_COUNTDOWN)) {
    Serial.println(F("/nCountdown alarm not supported"));
  } else {
    Serial.print(F("\nTesting a periodic two-second countdown alarm "));
    Serial.print(futureAlarmTestCount);
    Serial.println(F(" times"));
    countdownTestPassed = runCountdownAlarmTest();
  }

  bool secondTestPassed = true;
  if (!rtc.supports(AlarmMatch::MATCH_SECONDS)) {
    Serial.println(F("\nEvery-second alarm not supported"));
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
    Serial.println(F("\nSeconds matching alarm not supported for future second matching"));
  } else {
    Serial.println(F("\nTesting an alarm set 5 seconds in the future 5 times"));
    for (uint8_t testNumber = 1; testNumber <= futureAlarmTestCount; ++testNumber) {
      futureSecondsTestsPassed &= runFutureSecondsAlarmTest(testNumber);
    }
  }

  bool everyMinuteTestPassed = true;
  if (!rtc.supports(AlarmMatch::MATCH_SECONDS)) {
    Serial.println(F("\nSeconds matching alarm not supported for every-minute alarm"));
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
    Serial.println(F("\nMinutes matching alarm not supported for future minute matching"));
  } else {
    Serial.println(F("\nTesting an alarm set for the next minute"));
    for (uint8_t testNumber = 1; testNumber <= futureAlarmTestCount; ++testNumber) {
      futureMinutesTestsPassed &= runFutureMinutesAlarmTest(testNumber);
    }
  }

  Serial.println();
  Serial.println(countdownTestPassed && secondTestPassed && everyMinuteTestPassed && futureSecondsTestsPassed &&
                     futureMinutesTestsPassed ?
                   F("All alarm tests passed") :
                   F("One or more tests failed"));
}

void loop() {}
