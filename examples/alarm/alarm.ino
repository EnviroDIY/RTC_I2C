/** =========================================================================
 * @example{lineno} alarm.ino
 * @brief Demonstrates receiving periodic countdown and calendar alarm interrupts from an RV3032.
 *
 *  Steps for configuring an alarm:
 *  - Disable any existing alarms using rtc.disableAlarm() or rtc.disableCountdownAlarm()
 *  - Clear any existing alarm flags using rtc.clearAlarm() or rtc.clearCountdownAlarm()
 *  - Set the new alarm using rtc.setAlarm() or rtc.setCountdownAlarm() or one of the other alarm setting functions
 *  - Enable the alarm using rtc.enableAlarm() or rtc.enableCountdownAlarm()
 *
 * You should follow these steps whenever configuring a new alarm to ensure proper operation.  You generally cannot set
 * a new alarm while the alarm is active.  If you don't clear the previous alarm flag, the new alarm generally won't
 * trigger.
 * ======================================================================= */

#include <RTC_RV3032.h>

#if defined(ARDUINO_ARCH_AVR)
#include <EnableInterrupt.h>
#endif

/// The pin number for the alarm interrupt input.
const uint8_t alarmInterruptPin = 31;
/// The number of countdown alarm tests to run.
const uint8_t nCountdownTests = 10;
/// The number of every-second alarm tests to run.
const uint8_t nEverySecondTests = 15;
/// The number of seconds match alarm tests to run.
const uint8_t nSecondsMatchTests = 5;
/// The number of minutes match alarm tests to run.
const uint8_t nMinutesMatchTests = 3;
/// The number of exact match alarm tests to run.
const uint8_t nExactMatchTests = 3;

/// The countdown period in seconds.
const uint16_t countdownPeriodSeconds = 2;
/// The lead time in seconds for setting future alarms.
const uint8_t futureAlarmLeadSeconds = 5;

RV3032 rtc;

volatile uint8_t alarmInterruptCount = 0;

/**
 * ISR for handling clock alarm interrupts.
 * Increments the global alarm interrupt count.
 */
void clockAlarmISR() {
  ++alarmInterruptCount;
}

/**
 * Attach the clock alarm interrupt to the specified pin.
 * Configures the pin as an input with pull-up and sets up the interrupt using EnableInterrupt on AVR or attachInterrupt
 * otherwise.
 */
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
 * @param countdown True if reporting a countdown alarm, false for a calendar alarm.
 */
void reportAlarmInterrupts(uint8_t& reportedCount, bool countdown) {
  uint8_t currentCount = alarmInterruptCount;
  if (currentCount == reportedCount) return;

  // The RTC holds its interrupt output low until the calendar alarm flag is cleared.
  rtc.clearAlarm();
  while (reportedCount < currentCount) {
    ++reportedCount;
    Serial.println(countdown ? F("Countdown alarm detected") : F("Clock alarm detected"));
  }
}

/**
 * Run the alarm test.
 * Returns true if the test passes, false otherwise.
 * @param expectedCount The number of alarm interrupts expected.  The test will fail if the actual count does not match.
 * @param minimumTime The minimum time the test should take.  The test will fail if the expected count is reached too
 * quickly.
 * @param maximumTime The maximum time to wait for the alarm interrupts.  The test will fail if the expected count is
 * not reached within this time.
 * @param countdown True if reporting a countdown alarm, false for a calendar alarm.
 */
bool runAlarmTest(uint8_t expectedCount, unsigned long minimumTime, unsigned long maximumTime, bool countdown) {
  uint8_t reportedCount = 0;
  unsigned long startTime = millis();

  while (millis() - startTime < maximumTime && alarmInterruptCount < expectedCount) {
    reportAlarmInterrupts(reportedCount, countdown);
  }
  reportAlarmInterrupts(reportedCount, countdown);

  Serial.print(F("Received "));
  Serial.print(alarmInterruptCount);
  Serial.print(F(" alarm interrupts in "));
  Serial.print(millis() - startTime);
  Serial.print(F(" ms; expected "));
  Serial.print(expectedCount);
  Serial.print(F(" in "));
  Serial.print(minimumTime);
  Serial.print('-');
  Serial.print(maximumTime);
  Serial.println(F(" ms"));

  bool passed = alarmInterruptCount == expectedCount && (millis() - startTime >= minimumTime);
  Serial.println(passed ? F("PASS") : F("FAIL"));
  return passed;
}

/**
 * Run the countdown alarm test.
 * Returns true if the test passes, false otherwise.
 */
bool runCountdownAlarmTest(uint8_t nTests = nCountdownTests, uint16_t countdownSeconds = countdownPeriodSeconds) {
  // Print a message
  Serial.print(F("\nTesting a periodic "));
  Serial.print(countdownSeconds);
  Serial.print(F(" second countdown alarm "));
  Serial.print(nTests);
  Serial.println(F(" times"));

  // Figure out the duration for the countdown alarm; at count*frequency, at most almost a full extra countdown period
  unsigned long minDuration = (nTests * countdownSeconds * 1000UL);
  unsigned long maxDuration = minDuration + (countdownSeconds * 1000U * 0.95);

  // reset the alarm interrupt count before starting the test
  resetAlarmInterruptCount();
  // set up the alarm
  if (!rtc.disableCountdownAlarm() || !rtc.clearCountdownAlarm() ||
      !rtc.setCountdownAlarm(countdownSeconds, CountdownTimerFrequency::HZ_1) || !rtc.enableCountdownAlarm()) {
    Serial.println(F("Could not configure the countdown alarm"));
    return false;
  }

  bool retVal = runAlarmTest(nTests, minDuration, maxDuration, true);

  // Disable the alarm after the test
  rtc.disableCountdownAlarm();
  rtc.clearCountdownAlarm();

  return retVal;
}

/**
 * Set an alarm for a specific number of seconds in the future and test it.
 * Returns true if the test passes, false otherwise.
 * @param nTests The number of seconds match tests to run.
 * @param leadSeconds The number of seconds to add to the current time for the future alarm.
 */
bool runMatchSecondsAlarmTest(uint8_t nTests = nSecondsMatchTests, uint8_t leadSeconds = futureAlarmLeadSeconds) {
  Serial.print(F("\nTesting an alarm set "));
  Serial.print(leadSeconds);
  Serial.print(F(" seconds in the future "));
  Serial.print(nTests);
  Serial.println(F(" times"));

  // Figure out the test duration for the future alarm; at least leadSeconds per test plus a small buffer
  unsigned long minDuration = (leadSeconds - 1) * 1000UL;
  unsigned long maxDuration = leadSeconds * 1000UL + 1000UL;


  int8_t rep = nTests;
  bool retVal = true;

  while (rep-- > 0 && retVal) {
    // get the current time from the RTC
    tm currentTime;
    if (!rtc.getTime(currentTime)) {
      Serial.println(F("Could not read the time before setting the alarm"));
      retVal = false;
      continue;
    }

    // calculate a new alarm second based on the current time and the lead time for the future alarm
    uint8_t alarmSecond = currentTime.tm_sec + leadSeconds;
    if (alarmSecond >= 60) {
      alarmSecond -= 60;
    }

    // reset the interrupt count
    resetAlarmInterruptCount();

    // - Disable any existing alarms
    // - Clear any existing alarm flags
    // - Set the new alarm
    // - Enable the alarm
    if (!rtc.disableAlarm() || !rtc.clearAlarm() || !rtc.setMinuteAlarm(alarmSecond) || !rtc.enableAlarm()) {
      Serial.println(F("Could not configure the matching seconds alarm"));
      retVal = false;
    }

    Serial.print(F("Matching seconds test "));
    Serial.print(nTests - rep - 1);
    Serial.print(F(" currently "));
    Serial.print(currentTime.tm_year + 1900);
    Serial.print('-');
    Serial.print(currentTime.tm_mon + 1);
    Serial.print('-');
    Serial.print(currentTime.tm_mday);
    Serial.print(' ');
    Serial.print(currentTime.tm_hour);
    Serial.write(':');
    if (currentTime.tm_min < 10) Serial.write('0');
    Serial.print(currentTime.tm_min);
    Serial.write(':');
    if (currentTime.tm_sec < 10) Serial.write('0');
    Serial.print(currentTime.tm_sec);
    Serial.print(F("; waiting for "));
    Serial.print(currentTime.tm_year + 1900);
    Serial.print('-');
    Serial.print(currentTime.tm_mon + 1);
    Serial.print('-');
    Serial.print(currentTime.tm_mday);
    Serial.print(' ');
    Serial.print(currentTime.tm_hour);
    Serial.write(':');
    if (currentTime.tm_min < 10) Serial.write('0');
    Serial.print(currentTime.tm_min);
    Serial.write(':');
    if (alarmSecond < 10) Serial.write('0');
    Serial.println(alarmSecond);

    retVal = runAlarmTest(1, minDuration, maxDuration, false);
  }
  // disable the alarm after the test
  rtc.disableAlarm();
  rtc.clearAlarm();

  return retVal;
}

/**
 * Set an alarm for the next minute break and test if it triggers correctly.
 * Returns true if the test passes, false otherwise.
 * @param nTests The test number for reporting purposes.
 */
bool runMatchMinutesAlarmTest(uint8_t nTests) {
  Serial.print(F("\nTesting a match minutes alarm "));
  Serial.print(nTests);
  Serial.println(F(" times"));

  int8_t rep = nTests;
  bool retVal = true;

  while (rep-- > 0 && retVal) {
    // get the current time from the RTC
    tm currentTime;
    if (!rtc.getTime(currentTime)) {
      Serial.println(F("Could not read the time before setting the alarm"));
      retVal = false;
      continue;
    }

    // calculate a new alarm second based on the current time and the lead time for the future alarm
    uint8_t alarmMinute = currentTime.tm_min + 1;
    if (alarmMinute >= 60) {
      alarmMinute -= 60;
    }

    // reset the interrupt count
    resetAlarmInterruptCount();
    // set up the alarm
    if (!rtc.disableAlarm() || !rtc.clearAlarm() || !rtc.setHourlyAlarm(alarmMinute) || !rtc.enableAlarm()) {
      Serial.println(F("Could not configure the matching minutes alarm"));
      retVal = false;
    }

    Serial.print(F("Matching minutes alarm test "));
    Serial.print(nTests - rep - 1);
    Serial.print(F(" currently "));
    Serial.print(currentTime.tm_year + 1900);
    Serial.print('-');
    Serial.print(currentTime.tm_mon + 1);
    Serial.print('-');
    Serial.print(currentTime.tm_mday);
    Serial.print(' ');
    Serial.print(currentTime.tm_hour);
    Serial.write(':');
    if (currentTime.tm_min < 10) Serial.write('0');
    Serial.print(currentTime.tm_min);
    Serial.write(':');
    if (currentTime.tm_sec < 10) Serial.write('0');
    Serial.print(currentTime.tm_sec);
    Serial.print(F("; waiting for "));
    Serial.print(currentTime.tm_year + 1900);
    Serial.print('-');
    Serial.print(currentTime.tm_mon + 1);
    Serial.print('-');
    Serial.print(currentTime.tm_mday);
    Serial.print(' ');
    Serial.print(currentTime.tm_hour);
    Serial.write(':');
    if (alarmMinute < 10) Serial.write('0');
    Serial.print(alarmMinute);
    Serial.println(":00");

    retVal = runAlarmTest(1, (59L - currentTime.tm_sec) * 1000L, 61000L, false);
  }
  // disable the alarm after the test
  rtc.disableAlarm();
  rtc.clearAlarm();

  return retVal;
}


/**
 * Set a monthly alarm for the the current day and hour and next minute break and test if it triggers correctly.
 * Returns true if the test passes, false otherwise.
 * @param nTests The number of times to run the test.
 * @warning The calculation of the next time to run the test is extremely simplistic.  It doesn't handle day/month/year
 * at all.
 */
bool runMatchDayOfMonthAlarmTest(uint8_t nTests) {
  Serial.print(F("\nTesting a monthly match alarm set "));
  if (rtc.supports(AlarmMatch::MATCH_SECONDS)) {
    Serial.print(futureAlarmLeadSeconds);
    Serial.print(F(" seconds in the future "));
  } else {
    Serial.print(F("at the next minute break "));
  }
  Serial.print(nTests);
  Serial.println(F(" times"));

  // Figure out the test duration for the future alarm
  unsigned long singleTestDuration = 61000UL;
  if (rtc.supports(AlarmMatch::MATCH_SECONDS)) {
    singleTestDuration = futureAlarmLeadSeconds * 1000UL + 1000UL;
  }

  int8_t rep = nTests;
  bool retVal = true;

  while (rep-- > 0 && retVal) {
    // get the current time from the RTC
    tm currentTime;
    if (!rtc.getTime(currentTime)) {
      Serial.println(F("Could not read the time before setting the alarm"));
      retVal = false;
      continue;
    }

    // calculate a new alarm second based on the current time and the lead time for the future alarm

    uint8_t alarmSecond = 0;
    uint8_t alarmMinute = currentTime.tm_min;
    uint8_t alarmHour = currentTime.tm_hour;
    // If we can match seconds, calculate the alarm second based on the current time and lead time
    // That will make the test go much faster since we don't have to wait for the next minute break.
    if (rtc.supports(AlarmMatch::MATCH_SECONDS)) {
      alarmSecond = currentTime.tm_sec + futureAlarmLeadSeconds;
    }
    // If the alarm second exceeds 59, or we don't support matching seconds, roll over to the next minute
    if (alarmSecond >= 60 && rtc.supports(AlarmMatch::MATCH_SECONDS)) {
      alarmSecond -= 60;
      ++alarmMinute;
    } else if (!rtc.supports(AlarmMatch::MATCH_SECONDS)) {
      ++alarmMinute;
    }
    // If the alarm minute exceeds 59, roll over to the next hour
    if (alarmMinute >= 60) {
      alarmMinute = 0;
      alarmHour = (alarmHour + 1) % 24;
    }
    // NOTE: This simplistic calculation does not handle day/month/year rollovers at all.
    // Don't run the test at midnight.

    // reset the interrupt count
    resetAlarmInterruptCount();

    // - Disable any existing alarms
    // - Clear any existing alarm flags
    // - Set the new alarm
    // - Enable the alarm
    if (!rtc.disableAlarm() || !rtc.clearAlarm() ||
        !rtc.setMonthlyAlarm(currentTime.tm_mday, alarmHour, alarmMinute, alarmSecond) || !rtc.enableAlarm()) {
      Serial.println(F("Could not configure the future alarm"));
      retVal = false;
    }

    Serial.print(F("Future alarm test "));
    Serial.print(nTests - rep - 1);
    Serial.print(F(" currently "));
    Serial.print(currentTime.tm_year + 1900);
    Serial.print('-');
    Serial.print(currentTime.tm_mon + 1);
    Serial.print('-');
    Serial.print(currentTime.tm_mday);
    Serial.print(' ');
    Serial.print(currentTime.tm_hour);
    Serial.write(':');
    if (currentTime.tm_min < 10) Serial.write('0');
    Serial.print(currentTime.tm_min);
    Serial.write(':');
    if (currentTime.tm_sec < 10) Serial.write('0');
    Serial.print(currentTime.tm_sec);
    Serial.print(F("; waiting for "));
    Serial.print(currentTime.tm_year + 1900);
    Serial.print('-');
    Serial.print(currentTime.tm_mon + 1);
    Serial.print('-');
    Serial.print(currentTime.tm_mday);
    Serial.print(' ');
    Serial.print(alarmHour);
    Serial.write(':');
    if (alarmMinute < 10) Serial.write('0');
    Serial.print(alarmMinute);
    Serial.write(':');
    if (alarmSecond < 10) Serial.write('0');
    Serial.println(alarmSecond);

    if (rtc.supports(AlarmMatch::MATCH_SECONDS)) {
      unsigned long minDuration = nTests * futureAlarmLeadSeconds * 1000UL;
      unsigned long maxDuration = minDuration + (futureAlarmLeadSeconds * 1000U * 0.95);
      retVal = runAlarmTest(1, minDuration, maxDuration, false);
    } else {
      retVal = runAlarmTest(1, (59L - currentTime.tm_sec) * 1000L, 61000L, false);
    }
  }
  // disable the alarm after the test
  rtc.disableAlarm();
  rtc.clearAlarm();

  return retVal;
}

/** ===========================================================================
 * Setup function for the alarm interrupt example.
 * Initializes the RTC, attaches the alarm interrupt, and runs various alarm tests.
 =========================================================================== */
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 10000L);
  Serial.println();
  Serial.print(rtc.getMakeModel());
  Serial.println(F(" alarm interrupt example\n"));

  if (!rtc.begin() || !rtc.init()) {
    Serial.print(F("Cannot access the "));
    Serial.print(rtc.getMakeModel());
    Serial.print(F(" at "));
    Serial.println(rtc.getAddress());
    while (true); // do nothing if the RTC cannot be accessed
  }

  // Attach the interrupt to the RTC alarm pin
  attachClockAlarmInterrupt();

  // run a countdown alarm test
  bool countdownTestPassed = true;
  if (!rtc.supports(RTCCapability::RTC_CAP_COUNTDOWN)) {
    Serial.println(F("\nCountdown alarm not supported"));
  } else {
    countdownTestPassed = runCountdownAlarmTest(nCountdownTests, countdownPeriodSeconds);
  }

  // run an every-second alarm test
  // for an alarm every second:
  // - use the setSecondAlarm() function
  // - the RTC must support the EVERY_SECOND match type
  bool everySecondPassed = true;
  if (!rtc.supports(AlarmMatch::EVERY_SECOND)) {
    Serial.println(F("\nEvery-second alarm not supported"));
  } else {
    Serial.print(F("\nTesting an alarm every second for "));
    Serial.print(nEverySecondTests);
    Serial.println(F(" seconds"));
    resetAlarmInterruptCount();
    if (!rtc.disableAlarm() || !rtc.clearAlarm() || !rtc.setSecondAlarm() || !rtc.enableAlarm()) {
      Serial.println(F("Could not configure the every-second alarm"));
      while (true);
    }
    everySecondPassed =
      runAlarmTest(nEverySecondTests, ((nEverySecondTests - 1) * 1000UL), nEverySecondTests * 1000UL + 950UL, false);
  }

  // run an matching-seconds alarm test
  // for an alarm every minute when the seconds match:
  // - use the setMinuteAlarm() function
  // - the RTC must support the MATCH_SECONDS match type
  // NOTE: In this test, the alarm is set for a several seconds in the future and then unset and reset a few seconds
  // later. The total time is faster than 1 alarm/minute
  bool matchSecondsPassed = true;
  if (!rtc.supports(AlarmMatch::MATCH_SECONDS)) {
    Serial.println(F("\nSeconds matching alarm not supported"));
  } else {
    matchSecondsPassed = runMatchSecondsAlarmTest(nSecondsMatchTests, futureAlarmLeadSeconds);
  }

  // run an matching-minutes alarm test
  // for an alarm every minute when the seconds match:
  // - use the setMinuteAlarm() function
  // - the RTC must support the MATCH_SECONDS match type
  // NOTE: in this test, the alarm is set for a the very next minute then unset and reset for one minute later.
  // We're not waiting hours here.
  bool matchMinutesPassed = true;
  if (!rtc.supports(AlarmMatch::MATCH_MINUTES)) {
    Serial.println(F("\nMinutes matching alarm not supported"));
  } else {
    matchMinutesPassed &= runMatchMinutesAlarmTest(nMinutesMatchTests);
  }

  // run an day of month match alarm test setting the full date and time based on the current date and time
  // this is the largest commonly supported alarm match type
  // NOTE: in this test, the alarm is set for a the very next minute then unset and reset for either 5s or one minute
  // later.
  bool exactMatchPassed = true;
  if (!rtc.supports(AlarmMatch::MATCH_DAY_OF_MONTH)) {
    Serial.println(F("\nExact match alarm not supported"));
  } else {
    exactMatchPassed &= runMatchDayOfMonthAlarmTest(nExactMatchTests);
  }

  Serial.println();
  Serial.println(countdownTestPassed && everySecondPassed && matchSecondsPassed && matchMinutesPassed &&
                     exactMatchPassed ?
                   F("All alarm tests passed") :
                   F("One or more tests failed"));
}

void loop() {}
