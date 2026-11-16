#include <RTC_RV3028U.h>

// get Unix time (from a Unix time counter)
timestamp_t RV3028U::getTime() {
  timestamp_t t = 0;
  readUnixTime(t);
  return t;
}

bool RV3028U::readUnixTime(timestamp_t &timestamp) {
  timestamp_t t1 = 0, t2;
  for (uint16_t attempts = 0; attempts < 256; ++attempts) {
    t2 = t1;
    _wire->beginTransmission(_i2caddr);
    _wire->write(RV3028_UCLOCK);
    if (_wire->endTransmission(false) != 0) return false;
    if (_wire->requestFrom(_i2caddr, (byte)4) != 4) return false;
    t1 = 0;
    for (byte i = 0; i < 4; i++) t1 = (t1 >> 8) | (((timestamp_t)_wire->read()) << 24);
    if (t1 == t2) {
      timestamp = t1;
      return true;
    }
  }
  return false;
}

bool RV3028U::getTime(tm &timeParts) {
  timestamp_t t;
  if (!readUnixTime(t)) return false;
  TimeUtils::fillTimeParts(t, 0, epochStart::unix_epoch, timeParts);
  return true;
}

bool RV3028U::setTime(timestamp_t t) {
  // Serial.println(static_cast<uint32_t>(t), HEX);
  byte control;
  if (!readRegister(RV3028_CONTROL + 1, control)) return false;
  tm timeParts;
  TimeUtils::fillTimeParts(t, 0, epochStart::unix_epoch, timeParts);
  _wire->beginTransmission(_i2caddr);
  _wire->write(RV3028_CLOCKREG);
  _wire->write(bin2bcd(timeParts.tm_sec));
  _wire->write(bin2bcd(timeParts.tm_min));
  _wire->write(bin2bcd(timeParts.tm_hour));
  _wire->write(bin2bcd(timeParts.tm_wday));
  _wire->write(bin2bcd(timeParts.tm_mday));
  _wire->write(bin2bcd(timeParts.tm_mon + 1));
  _wire->write(bin2bcd(timeParts.tm_year - 100));
  bool success = _wire->endTransmission() == 0;
  success &= setRegister(RV3028_CONTROL + 1, control | 0b1); // reset the shared counter chain before setting Unix time
  _wire->beginTransmission(_i2caddr);
  _wire->write(RV3028_UCLOCK);
  timestamp_t unixTime = t;
  for (byte i = 0; i < 4; i++) {
#if defined(ARDUINO_ARCH_NRF52840) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED)
    _wire->write(static_cast<int>(unixTime & 0xFF));
#else
    _wire->write(static_cast<uint32_t>(unixTime & 0xFF));
#endif
    unixTime = unixTime >> 8;
  }
  success &= (_wire->endTransmission() == 0);
  if (success) success &= clearErrorFlags();
  return success;
}

bool RV3028U::setTime(tm timeParts) {
  return setTime(TimeUtils::tmToEpochTime(timeParts).getTimestamp());
}

String RV3028U::getModel() {
  return F("RV3028U");
}
