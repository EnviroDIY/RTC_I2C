#include <RTC_SD2405.h>

bool SD2405::init(bool reset_configuration, bool clear_flags) {
  byte control;
  if (!readRegister(SD2405_CONTROL, control)) return false;
  byte control2 = 0;
  if (!reset_configuration && !readRegister(SD2405_CONTROL + 1, control2)) return false;
  bool success = setRegister(SD2405_CONTROL + 1, control2 | 0x80); // unlock RTC
  success &= setRegister(SD2405_CONTROL, control | 0x84);          // unlock RTC without clearing status flags
  if (clear_flags) success &= clearErrorFlags();
  if (reset_configuration) success &= setRegister(SD2405_CONTROL + 2, 0x00);
  return success;
}

bool SD2405::isValid() {
  byte control;
  return readRegister(SD2405_CONTROL, control) &&
         ((control & 0b1) == 0); // RTCF=0 means no oscillator failure has been detected
}

bool SD2405::clearErrorFlags() {
  byte control;
  if (!readRegister(SD2405_CONTROL, control)) return false;
  if ((control & 0b1) == 0) return true;
  return setRegister(SD2405_CONTROL, control & 0b11111110); // clear RTCF
}

bool SD2405::setAlarm(AlarmMatch match, const AlarmTime &alarm) {
  byte matchBits;
  switch (match) {
  case AlarmMatch::MATCH_SECONDS:
    matchBits = 0x01;
    break;
  case AlarmMatch::MATCH_MINUTES:
    matchBits = 0x03;
    break;
  case AlarmMatch::MATCH_HOURS:
    matchBits = 0x07;
    break;
  case AlarmMatch::MATCH_DAY_OF_WEEK:
    matchBits = 0x0F;
    break;
  case AlarmMatch::MATCH_DAY_OF_MONTH:
    matchBits = 0x17;
    break;
  case AlarmMatch::MATCH_DAY_OF_YEAR:
    matchBits = 0x37;
    break;
  case AlarmMatch::MATCH_EXACT:
    matchBits = 0x77;
    break;
  default:
    return false;
  }

  bool success = true;
  success &= setRegister(SD2405_ALARM, bin2bcd(alarm.second));
  success &= setRegister(SD2405_ALARM + 1, bin2bcd(alarm.minute));
  success &= setRegister(SD2405_ALARM + 2, bin2bcd(alarm.hour));
  success &= setRegister(SD2405_ALARM + 3, 1 << (alarm.weekday + _wdaybase));
  success &= setRegister(SD2405_ALARM + 4, bin2bcd(alarm.day));
  success &= setRegister(SD2405_ALARM + 5, bin2bcd(alarm.month));
  success &= setRegister(SD2405_ALARM + 6, bin2bcd((byte)(alarm.year - 2000)));
  success &= setRegister(SD2405_ALARM + 7, matchBits);
  return success;
}

bool SD2405::setCountdownAlarm(uint16_t timerValue, CountdownTimerFrequency timerFrequency) {
  if (timerValue == 0 || timerValue > 0xFF) return false;

  byte frequencyBits;
  switch (timerFrequency) {
  case CountdownTimerFrequency::HZ_4096:
    frequencyBits = 0;
    break;
  case CountdownTimerFrequency::HZ_64:
    frequencyBits = 1;
    break;
  case CountdownTimerFrequency::HZ_1:
    frequencyBits = 2;
    break;
  case CountdownTimerFrequency::PER_MINUTE:
    frequencyBits = 3;
    break;
  default:
    return false;
  }

  byte control2;
  byte control3;
  if (!readRegister(SD2405_CONTROL + 1, control2) || !readRegister(SD2405_CONTROL + 2, control3)) return false;

  bool success = setRegister(SD2405_CONTROL + 1, control2 & 0b11111011); // disable INTDE
  success &= clearCountdownAlarm();
  success &= setRegister(SD2405_CONTROL + 2, (control3 & 0b11001111) | (frequencyBits << 4));
  success &= setRegister(SD2405_TIMER, static_cast<byte>(timerValue));
  return success;
}

bool SD2405::enableCountdownAlarm() {
  byte control;
  if (!readRegister(SD2405_CONTROL + 1, control)) return false;
  bool success = clearCountdownAlarm();
  success &= setRegister(SD2405_CONTROL + 1, (control & 0b11001111) | 0b00110100); // select countdown INT; enable INTDE
  return success;
}

bool SD2405::disableCountdownAlarm() {
  byte control;
  if (!readRegister(SD2405_CONTROL + 1, control)) return false;
  byte requestedControl = control & 0b11111011;                                      // disable INTDE
  if ((requestedControl & 0b00110000) == 0b00110000) requestedControl &= 0b11001111; // disconnect countdown INT
  return setRegister(SD2405_CONTROL + 1, requestedControl);
}

bool SD2405::senseCountdownAlarm() {
  byte control;
  return readRegister(SD2405_CONTROL, control) && ((control & 0b00010000) != 0);
}

bool SD2405::clearCountdownAlarm() {
  byte control;
  if (!readRegister(SD2405_CONTROL, control)) return false;
  return setRegister(SD2405_CONTROL, control & 0b11101111); // clear INTDF
}


bool SD2405::enableAlarm() {
  byte ctr;
  if (!readRegister(SD2405_CONTROL + 1, ctr)) return false;
  return setRegister(SD2405_CONTROL + 1, (ctr & 0b11001101) | 0b00010010); // set the INTAE bit and INTS1/INTS=01
}

bool SD2405::disableAlarm() {
  byte ctr;
  if (!readRegister(SD2405_CONTROL + 1, ctr)) return false;
  return setRegister(SD2405_CONTROL + 1, (ctr & 0b11111101)); // clear INTAE bit
}

bool SD2405::senseAlarm() {
  byte control;
  return readRegister(SD2405_CONTROL, control) && ((control & 0b00100000) != 0);
}

bool SD2405::clearAlarm() {
  byte ctr;
  if (!readRegister(SD2405_CONTROL, ctr)) return false;
  return setRegister(SD2405_CONTROL, (ctr & 0b11011111)); // clear INTAF
}


bool SD2405::enable32kHz() {
  byte control2;
  byte control3;
  if (!readRegister(SD2405_CONTROL + 1, control2) || !readRegister(SD2405_CONTROL + 2, control3)) return false;
  bool success = setRegister(SD2405_CONTROL + 1, (control2 & 0b11001110) | 0b00100001); // enable SQW output
  success &= setRegister(SD2405_CONTROL + 2, (control3 & 0b11110000) | 0b0001);         // 32kHz
  return success;
}

bool SD2405::disable32kHz() {
  byte control;
  if (!readRegister(SD2405_CONTROL + 1, control)) return false;
  return setRegister(SD2405_CONTROL + 1, control & ~0b00110001); // disable frequency output and INT selection
}

bool SD2405::enable1Hz() {
  byte control2;
  byte control3;
  if (!readRegister(SD2405_CONTROL + 1, control2) || !readRegister(SD2405_CONTROL + 2, control3)) return false;
  bool success = setRegister(SD2405_CONTROL + 1, (control2 & 0b11001110) | 0b00100001); // enable SQW output
  success &= setRegister(SD2405_CONTROL + 2, (control3 & 0b11110000) | 0b1010);         // 1 Hz
  return success;
}


bool SD2405::disable1Hz() {
  return disable32kHz();
}


bool SD2405::setOffset(int offset, OffsetMode mode) {
  bool success = true;
  if (mode != OffsetMode::RAW_OFFSET) {
    // add and then divide to round the offset instead of truncating it
    offset = (offset + (offset > 0 ? +152 : -152)) / 305;
    // force the offset into range
    if (offset < -62)
      offset = -62;
    else if (offset > 63)
      offset = 63;
  }
  // set the offset register
  success &= setRegister(SD2405_OFFSET, (offset & 0x7F)); // Serial.println(offset);
  return success;
}

unsigned int SD2405::getOffset() {
  return (getRegister(SD2405_OFFSET) & 0x7F);
}

String SD2405::getManufacturer() {
  return F("DFRobot");
}

String SD2405::getModel() {
  return F("SD2405");
}

// cSpell:ignore INTAE
