#include <RTC_RV8803.h>

bool RV8803::init(bool reset_configuration, bool clear_flags) {
  bool success = true;
  if (reset_configuration) success &= setRegister(RV8803_CONTROL, 0); // clear control register
  if (clear_flags) success &= clearErrorFlags();                      // clear all flags
  if (reset_configuration) success &= setRegister(RV8803_CLKOUT, 0);  // 32 KHz output by default
  return success;
}

bool RV8803::isValid() {
  byte status;
  return readRegister(RV8803_STATUS, status) && ((status & 0b11) == 0); // both voltage low flags are cleared
}

bool RV8803::clearErrorFlags() {
  byte status;
  if (!readRegister(RV8803_STATUS, status)) return false;
  if ((status & 0b11) == 0) return true;
  return setRegister(RV8803_STATUS, status & 0b11111100); // clear voltage low flags
}

// set & clear reset bit when setting time/date
bool RV8803::setTime(timestamp_t t) {
  tm timeParts;
  TimeUtils::fillTimeParts(t, 0, epochStart::unix_epoch, timeParts);
  return setTime(timeParts);
}

bool RV8803::setTime(tm timeParts) {
  byte control;
  if (!readRegister(RV8803_CONTROL, control)) return false;
  bool success = setRegister(RV8803_CONTROL, control | 1); // set RESET bit
  if (success) success = writeTimeRegisters(timeParts);
  success &= setRegister(RV8803_CONTROL, control & 0b11111110); // clear RESET bit
  if (success) success &= clearErrorFlags();
  return success;
}

bool RV8803::writeTimeRegisters(tm timeParts) {
  for (byte attempts = 0; attempts < 4; ++attempts) {
    _wire->beginTransmission(_i2caddr);
    _wire->write(RV8803_CLOCKREG);
    _wire->write(bin2bcd(timeParts.tm_sec));
    _wire->write(bin2bcd(timeParts.tm_min));
    _wire->write(bin2bcd(timeParts.tm_hour));
    _wire->write(1 << timeParts.tm_wday);
    _wire->write(bin2bcd(timeParts.tm_mday));
    _wire->write(bin2bcd(timeParts.tm_mon + 1));
    _wire->write(bin2bcd(timeParts.tm_year - 100));
    if (_wire->endTransmission() == 0 && finishWriteAccess(RV8803_CLOCKREG)) return true;
  }
  return false;
}

bool RV8803::setRegister(byte reg, byte val) {
  for (byte attempts = 0; attempts < 4; ++attempts) {
    _wire->beginTransmission(_i2caddr);
    _wire->write(reg);
    _wire->write(val);
    if (_wire->endTransmission() == 0 && finishWriteAccess(reg)) return true;
  }
  return false;
}

bool RV8803::finishWriteAccess(byte reg) {
  byte value;
  return readRegister(reg, value); // finish with a read operation and STOP, as required by affected silicon
}

timestamp_t RV8803::getTime() {
  timestamp_t unixTimestamp = 0;
  getTime(unixTimestamp);
  return unixTimestamp;
}

bool RV8803::getTime(timestamp_t &unixTimestamp) {
  tm timeParts;
  if (!getTime(timeParts)) return false;
  unixTimestamp = TimeUtils::tmToEpochTime(timeParts).getTimestamp();
  return true;
}

// Implement the time-reading procedure described in the application note, section 4.12.
bool RV8803::getTime(tm &timeParts) {
  tm timeParts1;
  if (!RTC_I2C::getTime(timeParts)) return false;
  if (timeParts.tm_sec == 59) { // be careful when we read 59 seconds because there could have been an increment
    if (!RTC_I2C::getTime(timeParts1)) return false;     // query again
    if (timeParts1.tm_sec != 59) timeParts = timeParts1; // otherwise the second reading must be OK
  }
  return true;
}

bool RV8803::setAlarm(AlarmMatch match, const AlarmTime &alarm) {
  byte minuteRegister = 0x80;
  byte hourRegister = 0x80;
  byte dayRegister = 0x80;
  bool setWeekdayMode = false;
  bool setDateMode = false;

  switch (match) {
  case AlarmMatch::MATCH_SECONDS:
    break;
  case AlarmMatch::MATCH_MINUTES:
    minuteRegister = bin2bcd(alarm.minute);
    break;
  case AlarmMatch::MATCH_HOURS:
    minuteRegister = bin2bcd(alarm.minute);
    hourRegister = bin2bcd(alarm.hour);
    break;
  case AlarmMatch::MATCH_DAY_OF_WEEK:
    minuteRegister = bin2bcd(alarm.minute);
    hourRegister = bin2bcd(alarm.hour);
    dayRegister = (byte)(1u << alarm.weekday);
    setWeekdayMode = true;
    break;
  case AlarmMatch::MATCH_DAY_OF_MONTH:
    minuteRegister = bin2bcd(alarm.minute);
    hourRegister = bin2bcd(alarm.hour);
    dayRegister = bin2bcd(alarm.day);
    setDateMode = true;
    break;
  default:
    return false;
  }

  bool success = true;
  if (setWeekdayMode || setDateMode) {
    byte extension;
    if (!readRegister(RV8803_CLKOUT, extension)) return false;
    success &= setRegister(RV8803_CLKOUT, setDateMode ? (extension | 0x40) : (extension & ~0x40));
  }
  success &= setRegister(RV8803_ALARM, minuteRegister);
  success &= setRegister(RV8803_ALARM + 1, hourRegister);
  success &= setRegister(RV8803_ALARM + 2, dayRegister);
  return success;
}


bool RV8803::enableAlarm() {
  byte ctr;
  if (!readRegister(RV8803_CONTROL, ctr)) return false;
  return setRegister(RV8803_CONTROL, ctr | 0b1000); // set the AIE bit
}

bool RV8803::disableAlarm() {
  byte ctr;
  if (!readRegister(RV8803_CONTROL, ctr)) return false;
  return setRegister(RV8803_CONTROL, (ctr & 0b11110111)); // clear AIE bit
}

bool RV8803::senseAlarm() {
  byte status;
  return readRegister(RV8803_STATUS, status) && ((status & 0b1000) != 0);
}

bool RV8803::clearAlarm() {
  byte ctr;
  if (!readRegister(RV8803_STATUS, ctr)) return false;
  return setRegister(RV8803_STATUS, (ctr & 0b11110111));
}


bool RV8803::enable32kHz() {
  byte clkout;
  if (!readRegister(RV8803_CLKOUT, clkout)) return false;
  return setRegister(RV8803_CLKOUT, (clkout | 0b1100));
}

bool RV8803::enable1Hz() {
  byte clkout;
  if (!readRegister(RV8803_CLKOUT, clkout)) return false;
  return setRegister(RV8803_CLKOUT, (clkout & 0b11110011) | 0b00001000);
}

bool RV8803::setOffset(int offset, OffsetMode mode) {
  bool success = true;
  if (mode != OffsetMode::RAW_OFFSET) {
    // Force the offset into range
    if (offset < 0)
      offset = offset - 12;
    else
      offset = offset + 12;
    offset = offset / 24;
    if (offset < -32)
      offset = -32;
    else if (offset > 31)
      offset = 31;
  }
  // set the offset register
  success &= setRegister(RV8803_OFFSET, (offset & 0x3F)); // Serial.println(offset);
  // Serial.println(offset&0x3F);
  return success;
}


unsigned int RV8803::getOffset() {
  return (getRegister(RV8803_OFFSET) & 0x3F);
}

String RV8803::getManufacturer() {
  return F("Micro Crystal");
}

String RV8803::getModel() {
  return F("RV8803");
}
