#include <RTC_RV3028.h>

bool RV3028::init(BatteryMode mode) {
  bool success = true;
  success &= setRegister(RV3028_CONTROL, 0);         // clear control1 register
  success &= setRegister(RV3028_CONTROL + 1, 0);     // clear control2 register
  success &= setRegister(RV3028_STATUS, 0);          // clear all flags
  success &= setRegister(RV3028_CLKOUT, 0b01000000); // 32 KHz output by default, CLKOUT is off
  byte bsm_reg =
    getRegister(RV3028_BSM) & 0b11110011; // get the current BSM register and zero only the switching mode bits
  switch (mode) {
  case BatteryMode::SWITCHING_DISABLED: {
    // Switchover Disabled. – Default value on delivery
    bsm_reg |= 0;
    break;
  }
  case BatteryMode::LEVEL_SWITCHING: {
    // Enables the Level Switching Mode (LSM).  Switchover when VDD < VTH:LSM (2.0 V) AND VBACKUP > VTH:LSM (2.0 V). Use
    // this with a standard coin cell battery.
    bsm_reg |= 0b1100;
    break;
  }
  case BatteryMode::DIRECT_SWITCHING: {
    // Enables the Direct Switching Mode (DSM).  Switchover when VDD < VBACKUP.  Slightly lower power consumption than
    // LSM.  Use this when charging a rechargeable battery.
    bsm_reg |= 0b0100;
    break;
  }
  }
  success &= setRegister(RV3028_BSM, bsm_reg); // switching mode
  success &= updateEEPROMByte(RV3028_CLKOUT);
  success &= updateEEPROMByte(RV3028_BSM);
  return success;
}

bool RV3028::isValid() {
  return ((getRegister(RV3028_STATUS) & 0b1) == 0); // POR flag is cleared
}

bool RV3028::setAlarm(byte minute, byte hour) {
  bool success = true;
  success &= setRegister(RV3028_ALARM, bin2bcd(minute));   // set minute alarm
  success &= setRegister(RV3028_ALARM + 1, bin2bcd(hour)); // set hour alarm
  success &= setRegister(RV3028_ALARM + 2, 0x80);          // set weekday/day alarm to always
  return success;
}

bool RV3028::setAlarm(byte minute) {
  bool success = true;
  success &= setRegister(RV3028_ALARM, bin2bcd(minute)); // set minute alarm
  success &= setRegister(RV3028_ALARM + 1, 0x80);        // set hour alarm to always
  success &= setRegister(RV3028_ALARM + 2, 0x80);        // set weekday/day alarm to always
  return success;
}


bool RV3028::enableAlarm() {
  byte ctr = getRegister(RV3028_CONTROL + 1);
  return setRegister(RV3028_CONTROL + 1, ctr | 0b1000); // set the AIE bit
}

bool RV3028::disableAlarm() {
  byte ctr = getRegister(RV3028_CONTROL + 1);
  return setRegister(RV3028_CONTROL + 1, (ctr & 0b11110111)); // clear AIE bit
}

bool RV3028::senseAlarm() {
  return ((getRegister(RV3028_STATUS) & 0b100) != 0);
}

bool RV3028::clearAlarm() {
  byte ctr = getRegister(RV3028_STATUS);
  return setRegister(RV3028_STATUS, (ctr & 0b11111011));
}


bool RV3028::enable32kHz() {
  return setRegister(RV3028_CLKOUT, 0b11000000) & // enable CLKOUT 32kHz
         updateEEPROMByte(RV3028_CLKOUT);
}

bool RV3028::disable32kHz() {
  return setRegister(RV3028_CLKOUT, 0b01000000) & // disable CLKOUT
         updateEEPROMByte(RV3028_CLKOUT);
}

// use nINT as output since the CLICK board does not
// support the output of CLKOUT
bool RV3028::enable1Hz() {
  return setRegister(RV3028_CONTROL + 1, getRegister(RV3028_CONTROL) & ~0b00010000) &   // USEL = 0
         setRegister(RV3028_CONTROL + 1, getRegister(RV3028_CONTROL + 1) | 0b00100000); // UIE = 1
}

bool RV3028::disable1Hz() {
  return setRegister(RV3028_CONTROL + 1, getRegister(RV3028_CONTROL + 1) & ~0b00100000); // UIE = 0
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
  success &= setRegister(RV3028_OFFSET, (offset >> 1));
  success &= setRegister(RV3028_OFFSET + 1, (getRegister(RV3028_OFFSET + 1) & 0b01111111) |
                                              ((offset & 1) << 7)); // update the EEPROM with the new offset values
  success &= updateEEPROMByte(RV3028_OFFSET);
  success &= updateEEPROMByte(RV3028_OFFSET + 1);
  return success;
}

unsigned int RV3028::getOffset() {
  return ((((unsigned int)getRegister(RV3028_OFFSET)) << 1) | (getRegister(RV3028_OFFSET + 1) >> 7));
}


bool RV3028::updateEEPROMByte(byte reg) {
  bool success = true;
  byte timeout = 0;
  byte cnts = getRegister(reg);
  success &= setRegister(RV3028_CONTROL, getRegister(RV3028_CONTROL) | 0b1000); // set EERD = 1
  success &= setRegister(RV3028_EEADDR, reg);
  success &= setRegister(RV3028_EEDATA, cnts);
  while (++timeout && (getRegister(RV3028_STATUS) & 0b10000000)) { // busy with reading/writing EEPROM
    delay(20);                                                     // wait 20 ms
  }
  if (!timeout) {
    // Serial.println(F("Timeout in EEPROM wait"));
    return false;
  }
  timeout = 0;
  success &= setRegister(RV3028_EECMD, 0x21); // update EEPROM at EEADDR with value stored in EEADDR
  while (++timeout && (getRegister(RV3028_STATUS) & 0b10000000)) { // busy with reading/writing EEPROM
    delay(10);                                                     // wait 10 ms
  }
  if (!timeout) {
    // Serial.println(F("Timeout in EEPROM write"));
    return false;
  }
  success &= setRegister(RV3028_CONTROL, getRegister(RV3028_CONTROL) & ~0b00001000); // set EERD = 0
  return success;
}

String RV3028::getManufacturer() {
  return F("Micro Crystal");
}

String RV3028::getModel() {
  return F("RV3028");
}

// cSpell:ignore EEADDR EEDATA EECMD
