#include <RTC_RV3032.h>

bool RV3032::init(BatteryMode mode) {
  bool success = true;
  success &= setRegister(RV3032_CONTROL, 0);     // clear control1 register
  success &= setRegister(RV3032_CONTROL + 1, 0); // clear control2 register
  success &= setRegister(RV3032_CONTROL + 2, 0); // clear control3 register
  success &= setRegister(RV3032_CONTROL + 3, 0); // clear time-stamp register
  success &= setRegister(RV3032_CONTROL + 4, 0); // clear clock interrupt mask register
  success &= setRegister(RV3032_CONTROL + 5, 0); // clear EVI control register
  success &= setRegister(RV3032_CONTROL + 6, 0); // clear temperature threshold register
  success &= setRegister(RV3032_STATUS, 0);      // clear all flags
  success &= setRegister(RV3032_CLKOUT, 0);      // Set the oscillator selector to 32 KHz crystal by default
  byte coe_bsm_reg = 0b01000000;                 // disable CLKOUT
  switch (mode) {
  case BatteryMode::SWITCHING_DISABLED: {
    // Switchover Disabled. – Default value on delivery
    coe_bsm_reg |= 0;
    break;
  }
  case BatteryMode::LEVEL_SWITCHING: {
    // Enables the Level Switching Mode (LSM).  Switchover when VDD < VTH:LSM (2.0 V) AND VBACKUP > VTH:LSM (2.0 V). Use
    // this with a standard coin cell battery.
    coe_bsm_reg |= 0b100000;
    break;
  }
  case BatteryMode::DIRECT_SWITCHING: {
    // Enables the Direct Switching Mode (DSM).  Switchover when VDD < VBACKUP.  Slightly lower power consumption than
    // LSM.  Use this when charging a rechargeable battery.
    coe_bsm_reg |= 0b010000;
    break;
  }
  }
  success &= setRegister(RV3032_COE, coe_bsm_reg); // disable CLKOUT & set switching mode
  success &= updateEEPROMByte(RV3032_COE);
  return success;
}

bool RV3032::isValid() {
  return ((getRegister(RV3032_STATUS) & 0b11) == 0); // both voltage low and POR flags are cleared
}

bool RV3032::setAlarm(byte minute, byte hour) {
  bool success = true;
  success &= setRegister(RV3032_ALARM, bin2bcd(minute));   // set minute alarm
  success &= setRegister(RV3032_ALARM + 1, bin2bcd(hour)); // set hour alarm
  success &= setRegister(RV3032_ALARM + 2, 0x80);          // set date alarm to always
  return success;
}

bool RV3032::setAlarm(byte minute) {
  bool success = true;
  success &= setRegister(RV3032_ALARM, bin2bcd(minute)); // set minute alarm
  success &= setRegister(RV3032_ALARM + 1, 0x80);        // set hour alarm to always
  success &= setRegister(RV3032_ALARM + 2, 0x80);        // set date alarm to always
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
  success &= setRegister(RV3032_CLKOUT, 0); // set 32kHz
  success &= updateEEPROMByte(RV3032_CLKOUT);
  success &= setRegister(RV3032_COE, coe & ~0b01000000); // enable CLKOUT
  success &= updateEEPROMByte(RV3032_COE);
  return success;
}

bool RV3032::disable32kHz() {
  bool success = true;
  byte coe;
  if (!readRegister(RV3032_COE, coe)) return false;
  success &= setRegister(RV3032_COE, coe | 0b01000000); // disable CLKOUT
  success &= updateEEPROMByte(RV3032_COE);
  return success;
}

bool RV3032::enable1Hz() {
  bool success = true;
  byte coe;
  if (!readRegister(RV3032_COE, coe)) return false;
  success &= setRegister(RV3032_CLKOUT, 0b01100000); // set 1Hz
  success &= updateEEPROMByte(RV3032_CLKOUT);
  success &= setRegister(RV3032_COE, coe & ~0b01000000); // enable CLKOUT
  success &= updateEEPROMByte(RV3032_COE);
  return success;
}

bool RV3032::disable1Hz() {
  return disable32kHz();
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
  success &= setRegister(RV3032_OFFSET, (offset & 0x3F)); // set the offset in the RAM mirror
  success &= updateEEPROMByte(RV3032_OFFSET);
  // Serial.println(offset);
  // Serial.println(offset&0x3F);
  return success;
}

unsigned int RV3032::getOffset() {
  return (getRegister(RV3032_OFFSET) & 0x3F);
}


bool RV3032::updateEEPROMByte(byte reg) {
  bool success = true;
  byte timeout = 0;
  byte cnts = getRegister(reg);
  success &= setRegister(RV3032_CONTROL, getRegister(RV3032_CONTROL) | 0b100); // set EERD = 1
  success &= setRegister(RV3032_EEADDR, reg);
  success &= setRegister(RV3032_EEDATA, cnts);
  while (++timeout && (getRegister(RV3032_BUSY) & 0b100)) { // busy with reading/writing EEPROM
    delay(2);                                               // wait 2 ms
  }
  if (!timeout) {
    setRegister(RV3032_CONTROL, getRegister(RV3032_CONTROL) & ~0b00000100);
    //^ set EERD = 0 (EEPROM Memory Refresh Disable, disables auto-refresh)
    return false;
  }
  timeout = 0;
  success &= setRegister(RV3032_EECMD, 0x21);               // update EEPROM at EEADDR with value stored in EEADDR
  while (++timeout && (getRegister(RV3032_BUSY) & 0b100)) { // busy with reading/writing EEPROM
    delay(10);                                              // wait 10 ms
  }
  if (!timeout) {
    setRegister(RV3032_CONTROL, getRegister(RV3032_CONTROL) & ~0b00000100);
    //^ set EERD = 0 (re-enables auto refresh)
    return false;
  }
  success &= setRegister(RV3032_CONTROL, getRegister(RV3032_CONTROL) & ~0b00000100); // set EERD = 0
  return success;
}

String RV3032::getManufacturer() {
  return F("Micro Crystal");
}

String RV3032::getModel() {
  return F("RV3032");
}

// cSpell:ignore EEADDR EEDATA EECMD
