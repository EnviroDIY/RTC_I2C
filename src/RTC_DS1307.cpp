#include <RTC_DS1307.h>

bool DS1307::init(bool reset_configuration, bool clear_flags) {
  bool success = true;
  if (reset_configuration) success &= setRegister(DS1307_CONTROL, 0b00000011); // typical value after power-on
  if (clear_flags) success &= clearErrorFlags();
  return success;
}

bool DS1307::isValid() {
  byte seconds;
  bool success = readRegister(DS1307_CLOCKREG, seconds);
  return success && ((seconds & 0x80) == 0); // oscillator enabled
}

bool DS1307::clearErrorFlags() {
  // the seconds register; high bit is the oscillator halt bit
  byte seconds;
  // return false if read fails
  if (!readRegister(DS1307_CLOCKREG, seconds)) return false;
  // return true if the oscillator is already enabled/not halted
  if ((seconds & 0x80) == 0) return true; // 0b10000000
  // clear the clock-halt bit if it was set and return the write status
  return setRegister(DS1307_CLOCKREG, seconds & 0x7F); // 0b01111111
}

bool DS1307::enable32kHz() {
  return setRegister(DS1307_CONTROL, 0b00010011);
}

bool DS1307::disable32kHz() {
  return setRegister(DS1307_CONTROL, 0b00000000);
}

bool DS1307::enable1Hz() {
  return setRegister(DS1307_CONTROL, 0b00010000);
}

bool DS1307::disable1Hz() {
  return setRegister(DS1307_CONTROL, 0b00000000);
}

String DS1307::getManufacturer() {
  return F("Analog Devices");
}

String DS1307::getModel() {
  return F("DS1307");
}
