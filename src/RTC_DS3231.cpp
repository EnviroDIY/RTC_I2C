#include <RTC_DS3231.h>

bool DS3231::init(bool reset_configuration, bool clear_flags) {
  bool success = true;
  if (reset_configuration)
    success &= setRegister(DS3231_CONTROL,
                           0b00000100); // typical value after power-on, except bit 2 disables SQW and starts clock
  if (clear_flags) success &= clearErrorFlags();
  return success;
}

bool DS3231::isValid() {
  byte status;
  return readRegister(DS3231_STATUS, status) && ((status & 0x80) == 0); // OSF bit cleared = oscillator enabled
}

bool DS3231::enable32kHz() {
  byte stat;
  if (!readRegister(DS3231_STATUS, stat)) return false;
  return setRegister(DS3231_STATUS, (stat & 0b11110111) | 0b00001000); // EN32kHz = 1
}

bool DS3231::disable32kHz() {
  byte stat;
  if (!readRegister(DS3231_STATUS, stat)) return false;
  return setRegister(DS3231_STATUS, (stat & 0b11110111) | 0b00000000); //  EN32kHz = 0
}

bool DS3231::enable1Hz() {
  byte ctr;
  if (!readRegister(DS3231_CONTROL, ctr)) return false;
  return setRegister(DS3231_CONTROL, (ctr & 0b11100011) | 0b00000000); // RS1=0 RS=0 INTCN=0
}

bool DS3231::disable1Hz() {
  byte ctr;
  if (!readRegister(DS3231_CONTROL, ctr)) return false;
  return setRegister(DS3231_CONTROL, (ctr & 0b11100011) | 0b00000100); // RS1=0 RS=0 INTCN=1 disables 1 Hz
}

bool DS3231::getTemperature(float &temperatureC) {
  byte tempMSB;
  byte tempLSB;
  if (!readRegister(DS3231_TEMPMSB, tempMSB)) return false;
  if (!readRegister(DS3231_TEMPMSB + 1, tempLSB)) return false;
  temperatureC = (float)(int8_t)tempMSB + (float)((tempLSB >> 6) & 0x03) * 0.25f;
  return true;
}

bool DS3231::startTemperatureConversion() {
  byte status;
  if (!readRegister(DS3231_STATUS, status)) return false;
  if (status & 0b00000100) return true; // conversion is already in progress

  byte control;
  if (!readRegister(DS3231_CONTROL, control)) return false;
  return setRegister(DS3231_CONTROL, control | 0b00100000); // set CONV bit
}

bool DS3231::setOffset(int offset, OffsetMode mode) {
  bool success = true;
  byte timeout = 0;
  if (mode != OffsetMode::RAW_OFFSET) {
    //  If the user didn't specify that this is a raw offset, assume they gave an offset in the more common format of
    //  0.01 ppm steps.
    if (offset < 0)
      offset = (offset - 5) / 10;
    else
      offset = (offset + 5) / 10;
    // force the offset to be in the supported range
    if (offset < -128)
      offset = -128;
    else if (offset > 127)
      offset = 127;
  }
  success &= setRegister(DS3231_OFFSET, offset & 0xFF);
  while (++timeout && (getRegister(DS3231_STATUS) & 0b100)); // wait for non-busy period
  if (!timeout) return false;
  byte control;
  if (!readRegister(DS3231_CONTROL, control)) return false;
  success &= setRegister(DS3231_CONTROL, control | 0b100000);
  return success;
}

unsigned int DS3231::getOffset() {
  return getRegister(DS3231_OFFSET);
}

String DS3231::getManufacturer() {
  return F("Analog Devices");
}

String DS3231::getModel() {
  return F("DS3231");
}

// cSpell:ignore TEMPMSB
