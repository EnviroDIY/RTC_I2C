#include <RTC_DS3231.h>

void DS3231::init(__attribute__((unused)) BatteryMode mode) {
  setRegister(DS3231_CONTROL,
              0b00000100);                // typical value after power-on, except for bit 2 (disables SQW), starts clock
  setRegister(DS3231_STATUS, 0b00000000); // clear OSF flag, clear alarm flags, disable 32 kHz output
}

bool DS3231::isValid(void) {
  return ((getRegister(DS3231_STATUS) & 0x80) == 0); // OSF bit cleared = oscillator enabled
}

void DS3231::enable32kHz(void) {
  byte stat = getRegister(DS3231_STATUS);
  setRegister(DS3231_STATUS, (stat & 0b11110111) | 0b00001000); // EN32kHz = 1
}

void DS3231::disable32kHz(void) {
  byte stat = getRegister(DS3231_STATUS);
  setRegister(DS3231_STATUS, (stat & 0b11110111) | 0b00000000); //  EN32kHz = 0
}

void DS3231::enable1Hz(void) {
  byte ctr = getRegister(DS3231_CONTROL);
  setRegister(DS3231_CONTROL, (ctr & 0b11100011) | 0b00000000); // RS1=0 RS=0 INTCN=0
}

void DS3231::disable1Hz(void) {
  byte ctr = getRegister(DS3231_CONTROL);
  setRegister(DS3231_CONTROL, (ctr & 0b11100011) | 0b00000100); // RS1=0 RS=0 INTCN=1 disables 1 Hz
}

int DS3231::getTemp(void) {
  byte temp = getRegister(DS3231_TEMPMSB);
  return (int8_t)temp;
}

void DS3231::setOffset(int offset, OffsetMode mode) {
  int timeout = 0;
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
  setRegister(DS3231_OFFSET, offset & 0xFF);
  while (timeout++ && getRegister(DS3231_STATUS) & 0b100); // wait for non-busy period
  setRegister(DS3231_CONTROL, getRegister(DS3231_CONTROL) | 0b100000);
}

unsigned int DS3231::getOffset(void) {
  return getRegister(DS3231_OFFSET);
}

String DS3231::getManufacturer(void) {
  return F("Analog Devices");
}

String DS3231::getModel(void) {
  return F("DS3231");
}

// cSpell:ignore TEMPMSB
