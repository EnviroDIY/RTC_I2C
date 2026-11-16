#include <RTC_RV3028.h>

bool RV3028::init(bool reset_configuration, bool clear_flags) {
  bool success = true;
  if (reset_configuration) {
    success &= setRegister(RV3028_CONTROL, 0);     // clear control1 register
    success &= setRegister(RV3028_CONTROL + 1, 0); // clear control2 register
  }
  if (clear_flags) success &= clearErrorFlags();
  if (reset_configuration) {
    const byte clkoutPendingMask = 0x01;
    byte clkout;
    if (!readRegister(RV3028_CLKOUT, clkout))
      success = false;
    else if (clkout != 0b01000000 || (_pendingEEPROMWrites & clkoutPendingMask)) {
      bool persistenceSuccess = updateEEPROMByte(RV3028_CLKOUT, 0b01000000); // 32 KHz output by default, CLKOUT is off
      if (persistenceSuccess)
        _pendingEEPROMWrites &= ~clkoutPendingMask;
      else
        _pendingEEPROMWrites |= clkoutPendingMask;
      success &= persistenceSuccess;
    }
  }
  return success;
}

bool RV3028::setBatteryMode(BatteryMode mode) {
  byte bsm_reg;
  if (!readRegister(RV3028_BSM, bsm_reg)) return false;
  byte requested_bsm = bsm_reg & 0b11110011; // zero only the switching mode bits
  switch (mode) {
  case BatteryMode::SWITCHING_DISABLED: {
    // Switchover Disabled. – Default value on delivery
    break;
  }
  case BatteryMode::LEVEL_SWITCHING: {
    // Enables the Level Switching Mode (LSM).  Switchover when VDD < VTH:LSM (2.0 V) AND VBACKUP > VTH:LSM (2.0 V). Use
    // this with a standard coin cell battery.
    requested_bsm |= 0b1100;
    break;
  }
  case BatteryMode::DIRECT_SWITCHING: {
    // Enables the Direct Switching Mode (DSM).  Switchover when VDD < VBACKUP.  Slightly lower power consumption than
    // LSM.  Use this when charging a rechargeable battery.
    requested_bsm |= 0b0100;
    break;
  }
  default:
    return false;
  }
  const byte batteryModePendingMask = 0x02;
  if (requested_bsm == bsm_reg && !(_pendingEEPROMWrites & batteryModePendingMask)) return true;
  bool persistenceSuccess = updateEEPROMByte(RV3028_BSM, requested_bsm);
  if (persistenceSuccess)
    _pendingEEPROMWrites &= ~batteryModePendingMask;
  else
    _pendingEEPROMWrites |= batteryModePendingMask;
  return persistenceSuccess;
}

bool RV3028::isValid() {
  byte status;
  bool success = readRegister(RV3028_STATUS, status);
  success &= ((status & 0b1) == 0); // POR flag is cleared
  return success;
}

bool RV3028::clearErrorFlags() {
  byte status;
  if (!readRegister(RV3028_STATUS, status)) return false;
  if ((status & 0b1) == 0) return true;
  return setRegister(RV3028_STATUS, (status | 0b01111110) & 0b11111110); // clear only PORF
}

bool RV3028::setAlarm(AlarmMatch match, const AlarmTime &alarm) {
  byte minuteRegister = 0x80;
  byte hourRegister = 0x80;
  byte dayRegister = 0x80;
  bool setWeekdayMode = false;
  bool setDateMode = false;

  switch (match) {
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
    dayRegister = bin2bcd(alarm.weekday);
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
    byte control;
    if (!readRegister(RV3028_CONTROL, control)) return false;
    success &= setRegister(RV3028_CONTROL, setDateMode ? (control | 0x20) : (control & ~0x20));
  }
  success &= setRegister(RV3028_ALARM, minuteRegister);
  success &= setRegister(RV3028_ALARM + 1, hourRegister);
  success &= setRegister(RV3028_ALARM + 2, dayRegister);
  return success;
}


bool RV3028::enableAlarm() {
  byte ctr;
  if (!readRegister(RV3028_CONTROL + 1, ctr)) return false;
  return setRegister(RV3028_CONTROL + 1, ctr | 0b1000); // set the AIE bit
}

bool RV3028::disableAlarm() {
  byte ctr;
  if (!readRegister(RV3028_CONTROL + 1, ctr)) return false;
  return setRegister(RV3028_CONTROL + 1, (ctr & 0b11110111)); // clear AIE bit
}

bool RV3028::senseAlarm() {
  byte status;
  return readRegister(RV3028_STATUS, status) && ((status & 0b100) != 0);
}

bool RV3028::clearAlarm() {
  byte ctr;
  if (!readRegister(RV3028_STATUS, ctr)) return false;
  return setRegister(RV3028_STATUS, (ctr & 0b11111011));
}


bool RV3028::enable32kHz() {
  return updateEEPROMByte(RV3028_CLKOUT, 0b11000000); // enable CLKOUT 32kHz
}

bool RV3028::disable32kHz() {
  return updateEEPROMByte(RV3028_CLKOUT, 0b01000000); // disable CLKOUT
}

// use nINT as output since the CLICK board does not
// support the output of CLKOUT
bool RV3028::enable1Hz() {
  bool success = true;
  byte control1;
  byte control2;
  if (!readRegister(RV3028_CONTROL, control1) || !readRegister(RV3028_CONTROL + 1, control2)) return false;
  success &= setRegister(RV3028_CONTROL, control1 & ~0b00010000);
  //^ Clear USEL (Update Interrupt Select) bit to select 1 Hz output
  success &= setRegister(RV3028_CONTROL + 1, control2 | 0b00100000);
  // Set UIE (Periodic Time Update Interrupt Enable) bit so the interrupt is generated
  return success;
}

bool RV3028::disable1Hz() {
  byte control;
  if (!readRegister(RV3028_CONTROL + 1, control)) return false;
  return setRegister(RV3028_CONTROL + 1, control & ~0b00100000); // UIE = 0
}


// negative values make the clock faster by 0.9537 ppm/LSB
// The range of the internal parameter goes from -256 to +255.
// This means that possible values for offset range from -243.2 ppm to +244.1 ppm.
bool RV3028::setOffset(int offset, OffsetMode mode) {
  bool success = true;
  if (mode != OffsetMode::RAW_OFFSET) {
    // Force the offset into range
    if (offset < 0)
      offset = offset - 47;
    else
      offset = offset + 47;
    offset = offset / 95;
    if (offset < -256)
      offset = -256;
    else if (offset > 255)
      offset = 255;
    // Serial.println(offset);
  }
  // set the offset registers
  byte offsetLow;
  if (!readRegister(RV3028_OFFSET + 1, offsetLow)) return false;
  byte requestedOffsetLow = (offsetLow & 0b01111111) | ((offset & 1) << 7);
  success &= updateEEPROMByte(RV3028_OFFSET, offset >> 1);
  success &= updateEEPROMByte(RV3028_OFFSET + 1, requestedOffsetLow);
  return success;
}

unsigned int RV3028::getOffset() {
  return ((((unsigned int)getRegister(RV3028_OFFSET)) << 1) | (getRegister(RV3028_OFFSET + 1) >> 7));
}


bool RV3028::setEEPROMRefresh(byte control, bool enable) {
  return setRegister(RV3028_CONTROL, enable ? (control & ~0b00001000) : (control | 0b00001000));
}

bool RV3028::updateEEPROMByte(byte reg, byte value) {
  bool success = true;
  byte timeout = 0;
  byte control;
  if (!readRegister(RV3028_CONTROL, control)) return false;
  // disable automatic EEPROM refresh before writing
  if (!setEEPROMRefresh(control, false)) return false; // set EERD = 1
  if (!setRegister(reg, value)) {
    setEEPROMRefresh(control, true);
    return false;
  }
  // Write the address and data to the EEPROM
  if (!setRegister(RV3028_EEADDR, reg) || !setRegister(RV3028_EEDATA, value)) {
    // restore automatic EEPROM refresh before returning after failure
    setEEPROMRefresh(control, true);
    return false;
  }
  // wait for operation to complete
  while (++timeout && (getRegister(RV3028_STATUS) & 0b10000000)) { // busy with reading/writing EEPROM
    delay(20);                                                     // wait 20 ms
  }
  if (!timeout) {
    // Serial.println(F("Timeout in EEPROM wait"));
    // restore automatic EEPROM refresh before returning after failure
    setEEPROMRefresh(control, true);
    return false;
  }
  timeout = 0;
  if (!setRegister(RV3028_EECMD, 0x00) || // required first command
      !setRegister(RV3028_EECMD, 0x21)) { // write EEDATA to the byte selected by EEADDR
    setEEPROMRefresh(control, true);
    return false;
  }
  while (++timeout && (getRegister(RV3028_STATUS) & 0b10000000)) { // busy with reading/writing EEPROM
    delay(10);                                                     // wait 10 ms
  }
  if (!timeout) {
    // Serial.println(F("Timeout in EEPROM write"));
    // restore automatic EEPROM refresh before returning after failure
    setEEPROMRefresh(control, true);
    return false;
  }
  // restore automatic EEPROM refresh
  success &= setEEPROMRefresh(control, true); // set EERD = 0
  return success;
}

String RV3028::getManufacturer() {
  return F("Micro Crystal");
}

String RV3028::getModel() {
  return F("RV3028");
}

// cSpell:ignore EEADDR EEDATA EECMD PORF
