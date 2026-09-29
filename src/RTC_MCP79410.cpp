#include <RTC_MCP79410.h>

bool MCP79410::init(__attribute__((unused)) BatteryMode mode) {
  return setRegister(MCP79410_CONTROL, 0x80); // 0b10000000
}

bool MCP79410::isValid(void) {
  return ((getRegister(MCP79410_STATUS) & 0b100000) != 0); // oscillator is running
}

bool MCP79410::setTime(tm timeParts) {
  bool success = setRegister(MCP79410_CLOCKREG, 0x00); // disable oscillator
  int timeout = 0;
  while (++timeout && getRegister(MCP79410_CLOCKREG + 3) & 0b100000) {
    // wait for OSCON to become zero
  }
  _wire->beginTransmission(_i2caddr);
  _wire->write(MCP79410_CLOCKREG);
  _wire->write(bin2bcd(timeParts.tm_sec));  // seconds after the minute
  _wire->write(bin2bcd(timeParts.tm_min));  // minutes after the hour
  _wire->write(bin2bcd(timeParts.tm_hour)); // hours since midnight
  _wire->write((bin2bcd(timeParts.tm_wday + _wdaybase)) | 0b1000);
  // ^^ day of the week, adjusted from tm's 0-6 numbering to whatever the RTC uses as its _wdaybase
  // XOR with 0b1000 to set the VBATEN enable bit so that the thing can run on batteries
  _wire->write(bin2bcd(timeParts.tm_mday));       // day of the month
  _wire->write(bin2bcd(timeParts.tm_mon + 1));    // month of the year, zero to 1 indexed
  _wire->write(bin2bcd(timeParts.tm_year - 100)); // years since 1900 (as in tm structure) converted to years since 2000
  success &= (_wire->endTransmission() == 0);
  success &= setRegister(MCP79410_CLOCKREG, bin2bcd(timeParts.tm_sec) | 0x80); // now enable oscillator!
  return success;
}

// set time from Unix time
bool MCP79410::setTime(timestamp_t t) {
  tm timeParts;
  TimeUtils::fillTimeParts(t, 0, epochStart::unix_epoch, timeParts);
  return setTime(timeParts);
}


bool MCP79410::setAlarm(byte minute, byte hour) {
  bool success = true;
  tm timeParts;
  time_t t;
  if (!getTime(timeParts)) return false; // current time
  if (!((timeParts.tm_min < minute && timeParts.tm_hour == hour) ||
        (timeParts.tm_hour < hour))) { // alarm should be next day
    t = TimeUtils::tmToEpochTime(timeParts).getTimestamp() + SECONDS_IN_DAY;
    TimeUtils::fillTimeParts(t, 0, epochStart::unix_epoch, timeParts);
  }
  success &= setRegister(MCP79410_ALARM, bin2bcd(0));          // set second alarm
  success &= setRegister(MCP79410_ALARM + 1, bin2bcd(minute)); // set minute alarm
  success &= setRegister(MCP79410_ALARM + 2, bin2bcd(hour));   // set hour alarm
  success &=
    setRegister(MCP79410_ALARM + 3, 0x70 | bin2bcd(timeParts.tm_wday));   // set weekday alarm and set match condition
  success &= setRegister(MCP79410_ALARM + 4, bin2bcd(timeParts.tm_mday)); // set day of month
  success &= setRegister(MCP79410_ALARM + 5, bin2bcd(timeParts.tm_mon + 1)); // set day of month
  return success;
}

bool MCP79410::setAlarm(byte minute) {
  return setRegister(MCP79410_ALARM + 1, bin2bcd(minute)) & // set minute alarm
         setRegister(MCP79410_ALARM + 3, 0x10);             // set match condition to minutes must match
}

bool MCP79410::enableAlarm(void) {
  byte ctr = getRegister(MCP79410_CONTROL);
  return setRegister(MCP79410_CONTROL, ctr | 0b10000); // set the ALM0 bit
}

bool MCP79410::disableAlarm(void) {
  byte ctr = getRegister(MCP79410_CONTROL + 1);
  return setRegister(MCP79410_CONTROL, (ctr & 0b11101111)); // clear ALM0 bit
}

bool MCP79410::senseAlarm(void) {
  return ((getRegister(MCP79410_ALARM + 3) & 0b1000) != 0);
}

bool MCP79410::clearAlarm(void) {
  byte ctr = getRegister(MCP79410_ALARM + 3);
  return setRegister(MCP79410_ALARM + 3, (ctr & 0b11110111));
}


bool MCP79410::enable32kHz(void) {
  return setRegister(MCP79410_CONTROL, (getRegister(MCP79410_CONTROL) & 0b10111100) | 0b1000011); // enable SQW 32 kHz
}

bool MCP79410::disable32kHz(void) {
  return setRegister(MCP79410_CONTROL, getRegister(MCP79410_CONTROL) & ~0b01000000); // disable SQW
}

bool MCP79410::enable1Hz(void) {
  return setRegister(MCP79410_CONTROL, (getRegister(MCP79410_CONTROL) & 0b10111000) | 0b1000000); // set 1Hz
}

bool MCP79410::disable1Hz(void) {
  return disable32kHz();
}

String MCP79410::getManufacturer(void) {
  return F("Microchip");
}

String MCP79410::getModel(void) {
  return F("MCP79410");
}


bool MCP79410::setOffset(int offset, OffsetMode mode) {
  bool success = true;
  bool sign = false;
  switch (mode) {
  case OffsetMode::RAW_OFFSET: {
    // with a raw offset, directly write the given value to the offset register
    success &= setRegister(MCP79410_OFFSET, offset & 0xFF);
    break;
  }
  case OffsetMode::FINE_OFFSET: {
    // force the offset into supported range
    if (offset < -127)
      offset = -127;
    else if (offset > 127)
      offset = 127;
    // split the offset into sign and magnitude
    if (offset < 0) {
      sign = true;
      offset = -offset;
    }
    offset = (offset + 50) / 100;
    // Serial.println(sign);
    // Serial.println(offset);
    success &= setRegister(MCP79410_OFFSET, (sign << 7) | offset);
    break;
  }
  default:
    break;
  }
  success &= setRegister(MCP79410_CONTROL, getRegister(MCP79410_CONTROL) & 0b11111011); // clear RS2 to use fine trim
  return success;
}

unsigned int MCP79410::getOffset(void) {
  return getRegister(MCP79410_OFFSET);
}
