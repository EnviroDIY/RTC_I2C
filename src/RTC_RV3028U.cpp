#include <RTC_RV3028U.h>

// get Unix time (from a Unix time counter)
time_t RV3028U::getTime(bool blocking) {
  time_t t1 = 0, t2;
  int timeout = 0;
  do {
    t2 = t1;
    if (blocking) {
      t1 = getRegister(RV3028_UCLOCK);
      while (++timeout && t1 == getRegister(RV3028_UCLOCK)); // wait until next second is reached
    }
    _wire->beginTransmission(_i2caddr);
    _wire->write(RV3028_UCLOCK);
    if (_wire->endTransmission(false) != 0) return 0;
    if (_wire->requestFrom(_i2caddr, (byte)4) != 4) return 0;
    t1 = 0;
    for (byte i = 0; i < 4; i++) t1 = (t1 >> 8) | (((time_t)_wire->read()) << 24);
    if (blocking) return t1;
  } while (t1 != t2);
  return t1;
}

void RV3028U::getTime(tm &timeParts, bool blocking) {
  time_t t = getTime(blocking);
  gmtime_r(&t, &timeParts);
}

void RV3028U::setTime(time_t t) {
  // Serial.println(static_cast<uint32_t>(t), HEX);
  setRegister(RV3028_CONTROL + 1, getRegister(RV3028_CONTROL + 1) | 0b1); // reset counter chain in clock
  _wire->beginTransmission(_i2caddr);
  _wire->write(RV3028_UCLOCK);
  for (byte i = 0; i < 4; i++) {
#if defined(ARDUINO_ARCH_NRF52840)
    _wire->write(static_cast<int>(t & 0xFF));
#else
    _wire->write(static_cast<uint32_t>(t & 0xFF));
#endif
    t = t >> 8;
  }
  _wire->endTransmission();
}

void RV3028U::setTime(tm timeParts) {
  setTime(mktime(&timeParts));
}

String RV3028U::getManufacturer(void) {
  return F("Micro Crystal");
}

String RV3028U::getModel(void) {
  return F("RV3028U");
}
