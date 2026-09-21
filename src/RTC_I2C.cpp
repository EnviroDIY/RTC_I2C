#include "RTC_I2C.h"


// try to establish connection to RTC after RTC specific parameters have been set
bool RTC_I2C::begin(TwoWire *wi) {
  if (_i2caddr == 0 || wi == NULL) return false;
  if (_started) return true;
  _wire = wi;
  _wire->begin();
  _wire->beginTransmission(_i2caddr);
  if (_wire->endTransmission() != 0) return false;
  _started = true;
  return true;
}

// set time from Unix time
void RTC_I2C::setTime(time_t t) {
  tm timeParts;
  gmtime_r(&t, &timeParts);
  setTime(timeParts);
}

// set time from a time record
void RTC_I2C::setTime(tm timeParts) {
  _wire->beginTransmission(_i2caddr);
  _wire->write((_capabilities & RTC_CAP_SREGADDR) ? (_clockreg << 4) : _clockreg);
  _wire->write(bin2bcd(timeParts.tm_sec) | ((_bit7set & 1) ? 0x80 : 0));
  _wire->write(bin2bcd(timeParts.tm_min) | ((_bit7set & (1 << 1)) ? 0x80 : 0));
  _wire->write(bin2bcd(timeParts.tm_hour) | ((_bit7set & (1 << 2)) ? 0x80 : 0));
  if (!_wdayfirst) _wire->write(bin2bcd(timeParts.tm_mday) | ((_bit7set & (1 << 4)) ? 0x80 : 0));
  _wire->write((_wdaybase < 2 ? bin2bcd(timeParts.tm_wday - 1 + _wdaybase) : (1 << (timeParts.tm_wday - 1))) |
               ((_bit7set & (1 << 3)) ? 0x80 : 0));
  if (_wdayfirst) _wire->write(bin2bcd(timeParts.tm_mday) | ((_bit7set & (1 << 4)) ? 0x80 : 0));
  _wire->write(bin2bcd(timeParts.tm_mon) | ((_bit7set & (1 << 5)) ? 0x80 : 0));
  _wire->write(bin2bcd(timeParts.tm_year - 30)); // readjust to 2000 instead of 1970!
  _wire->endTransmission();
}

// get Unix time
time_t RTC_I2C::getTime(bool blocking) {
  tm timeParts;
  getTime(timeParts, blocking);
  return mktime(&timeParts);
}

// get time as time record
void RTC_I2C::getTime(tm &timeParts, bool blocking) {
  int timeout = 0;
  byte sec;
  timeParts = tm{0, 0, 0, 0, 0, 0, 0, 0, 0};

  if (blocking) {
    sec = getRegister(_clockreg);
    while (++timeout &&
           sec == getRegister((_capabilities & RTC_CAP_SREGADDR) ? (_clockreg << 4) :
                                                                   _clockreg)); // wait until next second is reached
  }
  _wire->beginTransmission(_i2caddr);
  _wire->write((_capabilities & RTC_CAP_SREGADDR) ? (_clockreg << 4) : _clockreg);
  if (_wire->endTransmission(false) != 0) return;
  if (_wire->requestFrom(_i2caddr, (byte)7) != 7) return;
  timeParts.tm_sec = bcd2bin(_wire->read() & 0x7F);
  timeParts.tm_min = bcd2bin(_wire->read() & 0x7F);
  timeParts.tm_hour = bcd2bin(_wire->read() & 0x3F);
  if (!_wdayfirst) timeParts.tm_mday = bcd2bin(_wire->read() & 0x3F);
  timeParts.tm_wday = (_wdaybase < 2 ? (bcd2bin(_wire->read() & 0x07)) - _wdaybase + 1 : decodewday(_wire->read()));
  if (_wdayfirst) timeParts.tm_mday = bcd2bin(_wire->read() & 0x3F);
  timeParts.tm_mon = bcd2bin(_wire->read() & 0x1F);
  timeParts.tm_year = bcd2bin(_wire->read()) + 30; // rebase to 1970!
}

byte RTC_I2C::decodewday(byte bits) {
  for (byte res = 1; res < 8; res++) {
    if (bits & 1) return res;
    bits = bits >> 1;
  }
  return 1; // default value
}

// set one RTC register
void RTC_I2C::setRegister(byte reg, byte val) {
  //Serial.print(F("setReg(0x")); Serial.print(reg,HEX); Serial.print(F(")=0b")); Serial.println(val,BIN);
  _wire->beginTransmission(_i2caddr);
  _wire->write((_capabilities & RTC_CAP_SREGADDR) ? (reg << 4) : reg);
  _wire->write(val);
  _wire->endTransmission();
  //Serial.println(F("Verify:")); Serial.println(getRegister(reg),BIN);
}

// get one RTC register
byte RTC_I2C::getRegister(byte reg) {
  byte res;
  //Serial.print(F("getReg(0x")); Serial.print(reg,HEX); Serial.print(F(")=0b"));
  _wire->beginTransmission(_i2caddr);
  _wire->write((_capabilities & RTC_CAP_SREGADDR) ? (reg << 4) : reg);
  if (_wire->endTransmission(false) != 0) return 0xFF;
  if (_wire->requestFrom(_i2caddr, (byte)1) != 1) return 0xFF;
  res = _wire->read();
  //Serial.println(res,BIN);
  return res;
}

// Alarm functions for all the Analog Devices RTCs with DS prefix

// Common registers
#define DSALARM_ALARM1 0x07  // start of alarm 1 register (seconds)
#define DSALARM_CONTROL 0x0E // Control register
#define DSALARM_STATUS 0x0F  // Status register

void DSAlarm::setAlarm(byte minute, byte hour) {
  setRegister(DSALARM_ALARM1, 0x00);                // clear seconds alarm
  setRegister(DSALARM_ALARM1 + 1, bin2bcd(minute)); // set minute alarm
  setRegister(DSALARM_ALARM1 + 2, bin2bcd(hour));   // set hour alarm
  setRegister(DSALARM_ALARM1 + 3, 0x80);            // set day alarm to always
}

void DSAlarm::setAlarm(byte minute) {
  setRegister(DSALARM_ALARM1, 0x00);                // clear seconds alarm
  setRegister(DSALARM_ALARM1 + 1, bin2bcd(minute)); // set minute alarm
  setRegister(DSALARM_ALARM1 + 2, 0x80);            // set hour alarm to always
  setRegister(DSALARM_ALARM1 + 3, 0x80);            // set day alarm to always
}

void DSAlarm::enableAlarm(void) {
  byte ctr = getRegister(DSALARM_CONTROL);
  setRegister(DSALARM_CONTROL, (ctr & 0b11111110) | 0b00000001);
}

void DSAlarm::disableAlarm(void) {
  byte ctr = getRegister(DSALARM_CONTROL);
  setRegister(DSALARM_CONTROL, (ctr & 0b11111110) | 0b00000000);
}

bool DSAlarm::senseAlarm(void) {
  return getRegister(DSALARM_STATUS) & 0x01;
}

void DSAlarm::clearAlarm(void) {
  byte ctr = getRegister(DSALARM_STATUS);
  setRegister(DSALARM_STATUS, (ctr & 0b11111110) | 0b00000000);
}


// Alarm functions for all the NXP RTCs with PCF prefix

// Common registers
#define PCFALARM_STATUS 0x01 // Control register

void PCFAlarm::setAlarm(byte minute, byte hour) {
  setRegister(_clockreg + 7, bin2bcd(minute)); // set minute alarm
  setRegister(_clockreg + 8, bin2bcd(hour));   // set hour alarm
  setRegister(_clockreg + 9, 0x80);            // set day alarm to always
  setRegister(_clockreg + 10, 0x80);           // set weekday alarm to always
}

void PCFAlarm::setAlarm(byte minute) {
  setRegister(_clockreg + 7, bin2bcd(minute)); // set minute alarm
  setRegister(_clockreg + 8, 0x80);            // set hour alarm to always
  setRegister(_clockreg + 9, 0x80);            // set day alarm to always
  setRegister(_clockreg + 10, 0x80);           // set weekday alarm to always
}

bool PCFAlarm::senseAlarm(void) {
  return ((getRegister(PCFALARM_STATUS) & 0b1000) != 0);
}

void PCFAlarm::clearAlarm(void) {
  byte ctr = getRegister(PCFALARM_STATUS);
  setRegister(PCFALARM_STATUS, (ctr & 0b11110111) | 0b00000000);
}

// Manufacturer and model information functions
String RTC_I2C::getMakeModel(void) {
  return String(getManufacturer()) + " " + String(getModel());
}
