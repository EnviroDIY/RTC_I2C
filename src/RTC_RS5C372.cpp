#include <RTC_RS5C372.h>

bool RS5C372::init(bool reset_configuration, bool clear_flags) {
  bool success = true;
  if (reset_configuration) {
    success &= setRegister(RS5C372_CONTROL1, 0b00100000); // INT(1Hz) and 32K to INTRB output, alarms to INTRA
    // Writing CONTROL2 clears XSTP, so preserve the validity flag when flag clearing was not requested.
    if (clear_flags) success &= setRegister(RS5C372_CONTROL2, 0b00101000); // ADJ = 0; use 24h and disable 32K
  }
  if (clear_flags) success &= clearErrorFlags();
  return success;
}

bool RS5C372::isValid() {
  byte control;
  return readRegister(RS5C372_CONTROL2, control) &&
         ((control & 0b00010000) == 0); // XSTP=0, otherwise clock is/was halted
}

bool RS5C372::clearErrorFlags() {
  byte control;
  if (!readRegister(RS5C372_CONTROL2, control)) return false;
  if ((control & 0b00010000) == 0) return true;
  return setRegister(RS5C372_CONTROL2, control & 0b11101111); // clear XSTP
}

bool RS5C372::enableAlarm() {
  byte ctr;
  if (!readRegister(RS5C372_CONTROL1, ctr)) return false;
  return setRegister(RS5C372_CONTROL1, (ctr | 0b10000000)); // set AALE bit
}

bool RS5C372::disableAlarm() {
  byte ctr;
  if (!readRegister(RS5C372_CONTROL1, ctr)) return false;
  return setRegister(RS5C372_CONTROL1, (ctr & ~0b10000000)); // reset AALE bit
}

bool RS5C372::setAlarm(AlarmMatch match, const AlarmTime &alarm) {
  byte weekdays;
  switch (match) {
  case AlarmMatch::MATCH_HOURS:
    weekdays = 0x7F;
    break;
  case AlarmMatch::MATCH_DAY_OF_WEEK:
    weekdays = (byte)(1u << alarm.weekday);
    break;
  default:
    return false;
  }

  bool success = true;
  success &= setRegister(RS5C372_ALARMMIN, bin2bcd(alarm.minute));
  success &= setRegister(RS5C372_ALARMHR, bin2bcd(alarm.hour));
  success &= setRegister(RS5C372_ALARMWDAYS, weekdays);
  return success;
}

bool RS5C372::senseAlarm() {
  byte control;
  return readRegister(RS5C372_CONTROL2, control) && ((control & 0b10) != 0);
}

bool RS5C372::clearAlarm() {
  byte ctr;
  if (!readRegister(RS5C372_CONTROL2, ctr)) return false;
  return setRegister(RS5C372_CONTROL2, (ctr & 0b11111101));
}


bool RS5C372::enable32kHz() {
  byte clkout;
  if (!readRegister(RS5C372_CONTROL2, clkout)) return false;
  return setRegister(RS5C372_CONTROL2, (clkout & ~0b00001000)); // set CLEN to 0
}

bool RS5C372::disable32kHz() {
  byte clkout;
  if (!readRegister(RS5C372_CONTROL2, clkout)) return false;
  return setRegister(RS5C372_CONTROL2, (clkout | 0b00001000)); // set CLEN to 1
}

bool RS5C372::enable1Hz() {
  byte clkout;
  if (!readRegister(RS5C372_CONTROL1, clkout)) return false;
  return setRegister(RS5C372_CONTROL1, (clkout & 0b11111000) | 0b00000011);
}

bool RS5C372::disable1Hz() {
  byte clkout;
  if (!readRegister(RS5C372_CONTROL1, clkout)) return false;
  return setRegister(RS5C372_CONTROL1, (clkout & 0b11111000));
}

bool RS5C372::setOffset(int offset, OffsetMode mode) {
  bool success = true;
  if (mode != OffsetMode::RAW_OFFSET) {
    // add and then divide to round the offset instead of truncating it
    offset = (offset + (offset > 0 ? +152 : -152)) / 305;
    // force offset into range
    if (offset < -64)
      offset = -64;
    else if (offset > 63)
      offset = 63;
  }
  // write the offset
  success &= setRegister(RS5C372_OFFSET, (offset & 0x7F)); // Serial.println(offset);
  // Serial.println(((offset&0x7F)|(mode<<7)));
  return success;
}

unsigned int RS5C372::getOffset() {
  return getRegister(RS5C372_OFFSET);
}

String RS5C372::getManufacturer() {
  return F("Ricoh");
}

String RS5C372::getModel() {
  return F("RS5C372");
}

// cSpell:ignore XSTP AALE
