#include <RTC_RV3032.h>

bool RV3032::init(bool reset_configuration, bool clear_flags) {
  bool success = true;

  if (reset_configuration) {
    success &= setRegister(RV3032_CONTROL, 0);     // clear control1 register
    success &= setRegister(RV3032_CONTROL + 1, 0); // clear control2 register
    success &= setRegister(RV3032_CONTROL + 2, 0); // clear control3 register
    success &= setRegister(RV3032_CONTROL + 3, 0); // clear time-stamp register
    success &= setRegister(RV3032_CONTROL + 4, 0); // clear clock interrupt mask register
    success &= setRegister(RV3032_CONTROL + 5, 0); // clear EVI control register
    success &= setRegister(RV3032_CONTROL + 6, 0); // clear temperature threshold register
  }
  if (clear_flags) success &= clearErrorFlags(); // clear all flags
  if (reset_configuration) {
    success &= setEEPROMBackedRegister(RV3032_CLKOUT, 0); // set oscillator selector to 32 KHz crystal by default
    byte coe;
    if (!readRegister(RV3032_COE, coe))
      success = false;
    else
      success &= setEEPROMBackedRegister(RV3032_COE, coe | 0b01000000); // disable CLKOUT
  }
  return success;
}

bool RV3032::setBatteryMode(BatteryMode mode) {
  byte coe;
  if (!readRegister(RV3032_COE, coe)) return false;
  byte requested_coe = coe & 0b11001111; // zero only the switching mode bits
  switch (mode) {
  case BatteryMode::SWITCHING_DISABLED: {
    // Switchover Disabled. – Default value on delivery
    break;
  }
  case BatteryMode::LEVEL_SWITCHING: {
    // Enables the Level Switching Mode (LSM).  Switchover when VDD < VTH:LSM (2.0 V) AND VBACKUP > VTH:LSM (2.0 V). Use
    // this with a standard coin cell battery.
    requested_coe |= 0b100000;
    break;
  }
  case BatteryMode::DIRECT_SWITCHING: {
    // Enables the Direct Switching Mode (DSM).  Switchover when VDD < VBACKUP.  Slightly lower power consumption than
    // LSM.  Use this when charging a rechargeable battery.
    requested_coe |= 0b010000;
    break;
  }
  default:
    return false;
  }
  return setEEPROMBackedRegister(RV3032_COE, requested_coe);
}

bool RV3032::isValid() {
  byte status;
  bool success = readRegister(RV3032_STATUS, status);
  success &= ((status & 0b11) == 0); // voltage low and POR flags are cleared
  return success;
}

bool RV3032::clearErrorFlags() {
  byte status;
  if (!readRegister(RV3032_STATUS, status)) return false;
  if ((status & 0b11) == 0) return true;                  // voltage low and POR flags are cleared
  return setRegister(RV3032_STATUS, status & 0b11111100); // clear VLF and PORF
}

bool RV3032::setAlarm(AlarmMatch match, const AlarmTime &alarm) {
  byte minuteRegister = 0x80;
  byte hourRegister = 0x80;
  byte dateRegister = 0x80;

  switch (match) {
  case AlarmMatch::MATCH_MINUTES:
    minuteRegister = bin2bcd(alarm.minute);
    break;
  case AlarmMatch::MATCH_HOURS:
    minuteRegister = bin2bcd(alarm.minute);
    hourRegister = bin2bcd(alarm.hour);
    break;
  case AlarmMatch::MATCH_DAY_OF_MONTH:
    minuteRegister = bin2bcd(alarm.minute);
    hourRegister = bin2bcd(alarm.hour);
    dateRegister = bin2bcd(alarm.day);
    break;
  default:
    return false;
  }

  bool success = true;
  success &= setRegister(RV3032_ALARM, minuteRegister);
  success &= setRegister(RV3032_ALARM + 1, hourRegister);
  success &= setRegister(RV3032_ALARM + 2, dateRegister);
  return success;
}


bool RV3032::enableAlarm() {
  byte ctr;
  if (!readRegister(RV3032_CONTROL + 1, ctr)) return false;
  return setRegister(RV3032_CONTROL + 1, ctr | 0b1000); // set the AIE bit
}

bool RV3032::disableAlarm() {
  byte ctr;
  if (!readRegister(RV3032_CONTROL + 1, ctr)) return false;
  return setRegister(RV3032_CONTROL + 1, (ctr & 0b11110111)); // clear AIE bit
}

bool RV3032::senseAlarm() {
  byte status;
  return readRegister(RV3032_STATUS, status) && ((status & 0b1000) != 0);
}

bool RV3032::clearAlarm() {
  byte ctr;
  if (!readRegister(RV3032_STATUS, ctr)) return false;
  return setRegister(RV3032_STATUS, (ctr & 0b11110111));
}


bool RV3032::enable32kHz() {
  bool success = true;
  byte coe;
  if (!readRegister(RV3032_COE, coe)) return false;
  success &= setEEPROMBackedRegister(RV3032_CLKOUT, 0);              // set 32kHz
  success &= setEEPROMBackedRegister(RV3032_COE, coe & ~0b01000000); // enable CLKOUT
  return success;
}

bool RV3032::disable32kHz() {
  bool success = true;
  byte coe;
  if (!readRegister(RV3032_COE, coe)) return false;
  success &= setEEPROMBackedRegister(RV3032_COE, coe | 0b01000000); // disable CLKOUT
  return success;
}

bool RV3032::enable1Hz() {
  bool success = true;
  byte coe;
  if (!readRegister(RV3032_COE, coe)) return false;
  success &= setEEPROMBackedRegister(RV3032_CLKOUT, 0b01100000);     // set 1Hz
  success &= setEEPROMBackedRegister(RV3032_COE, coe & ~0b01000000); // enable CLKOUT
  return success;
}

bool RV3032::disable1Hz() {
  return disable32kHz();
}

bool RV3032::getTemperature(float &temperatureC) {
  byte tempLSB;
  if (!readRegister(RV3032_BUSY, tempLSB)) return false;
  byte tempMSB;
  if (!readRegister(RV3032_TEMP, tempMSB)) return false;

  int16_t rawTemperature = (int16_t)(int8_t)tempMSB * 16 + ((tempLSB >> 4) & 0x0F);
  temperatureC = (float)rawTemperature * 0.0625f;
  return true;
}


bool RV3032::setOffset(int offset, OffsetMode mode) {
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
  success &= setEEPROMBackedRegister(RV3032_OFFSET, (offset & 0x3F)); // set the offset in the RAM mirror
  // Serial.println(offset);
  // Serial.println(offset&0x3F);
  return success;
}

unsigned int RV3032::getOffset() {
  return (getRegister(RV3032_OFFSET) & 0x3F);
}


bool RV3032::setEEPROMRefresh(byte control, bool enable) {
  return setRegister(RV3032_CONTROL, enable ? (control & ~0b00000100) : (control | 0b00000100));
}

bool RV3032::updateEEPROMByte(byte reg, byte value) {
  bool success = true;
  byte timeout = 0;
  byte control;
  if (!readRegister(RV3032_CONTROL, control)) return false;
  // disable automatic EEPROM refresh before writing
  if (!setEEPROMRefresh(control, false)) return false; // set EERD = 1
  if (!setRegister(reg, value)) {
    setEEPROMRefresh(control, true);
    return false;
  }
  // Write the address and data to the EEPROM
  if (!setRegister(RV3032_EEADDR, reg) || !setRegister(RV3032_EEDATA, value)) {
    // restore automatic EEPROM refresh before returning after failure
    setEEPROMRefresh(control, true);
    return false;
  }
  // wait for operation to complete
  while (++timeout && (getRegister(RV3032_BUSY) & 0b100)) { // busy with reading/writing EEPROM
    delay(2);                                               // wait 2 ms
  }
  if (!timeout) {
    // restore automatic EEPROM refresh before returning after failure
    setEEPROMRefresh(control, true);
    return false;
  }
  timeout = 0;
  if (!setRegister(RV3032_EECMD, 0x21)) { // write EEDATA to the EEPROM byte selected by EEADDR
    setEEPROMRefresh(control, true);
    return false;
  }
  while (++timeout && (getRegister(RV3032_BUSY) & 0b100)) { // busy with reading/writing EEPROM
    delay(10);                                              // wait 10 ms
  }
  if (!timeout) {
    // restore automatic EEPROM refresh before returning after failure
    setEEPROMRefresh(control, true);
    return false;
  }
  // restore automatic EEPROM refresh
  success &= setEEPROMRefresh(control, true); // set EERD = 0
  return success;
}

bool RV3032::setEEPROMBackedRegister(byte reg, byte value) {
  const byte pendingMask = (byte)(1u << (reg - RV3032_COE));
  byte currentValue;
  if (!readRegister(reg, currentValue)) return false;
  if (currentValue == value && !(_pendingEEPROMWrites & pendingMask)) return true;
  bool persistenceSuccess = updateEEPROMByte(reg, value);
  if (persistenceSuccess)
    _pendingEEPROMWrites &= ~pendingMask;
  else
    _pendingEEPROMWrites |= pendingMask;
  return persistenceSuccess;
}

String RV3032::getManufacturer() {
  return F("Micro Crystal");
}

String RV3032::getModel() {
  return F("RV3032");
}

// cSpell:ignore EEADDR EEDATA EECMD PORF
