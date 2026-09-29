#include <RTC_SD2405.h>

bool SD2405::init(__attribute__((unused)) BatteryMode mode) {
  bool success = true;
  success &= setRegister(SD2405_CONTROL + 1, 0x80); // unlock RTC
  success &= setRegister(SD2405_CONTROL, 0x84);     // unlock RTC
  success &= setRegister(SD2405_CONTROL + 2, 0x00);
  return success;
}

bool SD2405::isValid(void) {
  return ((getRegister(SD2405_CONTROL) & 0b1) == 0); // power on bit (not sure, but believe that = 0 means valid)
}

bool SD2405::setAlarm(byte minute, byte hour) {
  bool success = true;
  success &= setRegister(SD2405_ALARM + 1, bin2bcd(minute)); // set second alarm
  success &= setRegister(SD2405_ALARM + 2, bin2bcd(hour));   // set hour alarm
  success &= setRegister(SD2405_ALARM + 7, 0b110);           // hours and minute need to match
  return success;
}

bool SD2405::setAlarm(byte minute) {
  return setRegister(SD2405_ALARM + 1, bin2bcd(minute)) & // set second alarm
         setRegister(SD2405_ALARM + 7, 0b010);            // minute needs to match
}


bool SD2405::enableAlarm(void) {
  byte ctr = getRegister(SD2405_CONTROL + 1);
  return setRegister(SD2405_CONTROL + 1, (ctr & 0b11001101) | 0b00010010); // set the INTAE bit and INTS1/INTS=01
}

bool SD2405::disableAlarm(void) {
  byte ctr = getRegister(SD2405_CONTROL + 1);
  return setRegister(SD2405_CONTROL + 1, (ctr & 0b11111101)); // clear INTAE bit
}

bool SD2405::senseAlarm(void) {
  return ((getRegister(SD2405_CONTROL) & 0b00100000) != 0);
}

bool SD2405::clearAlarm(void) {
  byte ctr = getRegister(SD2405_ALARM + 3);
  return setRegister(SD2405_ALARM + 3, (ctr & 0b11011111));
}


bool SD2405::enable32kHz(void) {
  return setRegister(SD2405_CONTROL + 1,
                     (getRegister(SD2405_CONTROL + 1) & 0b11001110) | 0b00100001) &                // enable SQW output
         setRegister(SD2405_CONTROL + 2, (getRegister(SD2405_CONTROL + 2) & 0b11110000) | 0b0001); // 32kHz
}

bool SD2405::disable32kHz(void) {
  return setRegister(SD2405_CONTROL + 1, getRegister(SD2405_CONTROL + 1) & ~0b00110001); // disable SQW and alarm INT
}

bool SD2405::enable1Hz(void) {
  return setRegister(SD2405_CONTROL + 1,
                     (getRegister(SD2405_CONTROL + 1) & 0b11001110) | 0b00100001) &                // enable SQW output
         setRegister(SD2405_CONTROL + 2, (getRegister(SD2405_CONTROL + 2) & 0b11110000) | 0b1010); // 1 Hz
}


bool SD2405::disable1Hz(void) {
  return disable32kHz();
}


bool SD2405::setOffset(int offset, OffsetMode mode) {
  bool success = true;
  if (mode != OffsetMode::RAW_OFFSET) {
    // add and then divide to round the offset instead of truncating it
    offset = (offset + (offset > 0 ? +152 : -152)) / 305;
    // force the offset into range
    if (offset < -62)
      offset = -62;
    else if (offset > 63)
      offset = 63;
  }
  // set the offset register
  success &= setRegister(SD2405_OFFSET, (offset & 0x7F)); // Serial.println(offset);
  return success;
}

unsigned int SD2405::getOffset(void) {
  return (getRegister(SD2405_OFFSET) & 0x7F);
}

String SD2405::getManufacturer(void) {
  return F("DFRobot");
}

String SD2405::getModel(void) {
  return F("SD2405");
}

// cSpell:ignore INTAE
