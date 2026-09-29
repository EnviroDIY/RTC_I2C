#include <RTC_PCF8523.h>

// if Vbat disabled, connect to Vcc
void PCF8523::init(BatteryMode mode) {
  setRegister(PCF8523_CONTROL, 0b00010000); // initiate power-on reset by software
  setRegister(PCF8523_CONTROL + 1, 0);      // disable watchdog and countdown timers
  byte bsm_reg = 0;
  switch (mode) {
  case BatteryMode::SWITCHING_DISABLED: {
    bsm_reg = 0b01100000;
    break;
  }
  case BatteryMode::LEVEL_SWITCHING: {
    bsm_reg = 0b00000000;
    break;
  }
  case BatteryMode::DIRECT_SWITCHING: {
    bsm_reg = 0b00100000;
    break;
  }
  }
  setRegister(PCF8523_CONTROL + 2, bsm_reg); // switch mode and battery low detection
  setRegister(PCF8523_CLKOUT, 0b00111000);   // disable clock output
}

bool PCF8523::isValid(void) {
  return ((getRegister(PCF8523_CONTROL + 2) & 0b00000100) == 0) && // no battery low flag and
         ((getRegister(PCF8523_CONTROL) & 0b00100000) == 0) &&     // oscillator is running
         ((getRegister(PCF8523_CLOCKREG) & 0b10000000) == 0);      // OS flag cleared
}

void PCF8523::enableAlarm(void) {
  byte ctr = getRegister(PCF8523_CONTROL);
  setRegister(PCF8523_CONTROL, (ctr & 0b11111101) | 0b00000010);
}

void PCF8523::disableAlarm(void) {
  byte ctr = getRegister(PCF8523_CONTROL);
  setRegister(PCF8523_CONTROL, (ctr & 0b11111101) | 0b00000000);
}


void PCF8523::enable32kHz(void) {
  byte clkout = getRegister(PCF8523_CLKOUT);
  setRegister(PCF8523_CLKOUT, (clkout & 0b11000111) | 0b00000000);
}

void PCF8523::disable32kHz(void) {
  byte clkout = getRegister(PCF8523_CLKOUT);
  setRegister(PCF8523_CLKOUT, (clkout & 0b11000111) | 0b00111000);
}

void PCF8523::enable1Hz(void) {
  byte clkout = getRegister(PCF8523_CLKOUT);
  setRegister(PCF8523_CLKOUT, (clkout & 0b11000111) | 0b00110000);
}

void PCF8523::disable1Hz(void) {
  disable32kHz();
}

void PCF8523::setOffset(int offset, OffsetMode mode) {
  if (mode == OffsetMode::RAW_OFFSET)
    // put the raw value into the register
    setRegister(PCF8523_OFFSET, (offset & 0xFF));
  else {
    if (mode == OffsetMode::COARSE_OFFSET)
      // add and then divide to round the offset instead of truncating it
      offset = (offset + (offset > 0 ? +217 : -217)) / 434;
    else
      // add and then divide to round the offset instead of truncating it
      offset = (offset + (offset > 0 ? +203 : -203)) / 406;
    // force offset into range
    if (offset < -64)
      offset = -64;
    else if (offset > 63)
      offset = 63;
    // set the offset register with the calculated value and mode
    setRegister(PCF8523_OFFSET, ((offset & 0x7F) | (static_cast<byte>(mode) << 7)));
    // Serial.println(offset);
    // Serial.println(((offset&0x7F)|(mode<<7)));
  }
}

unsigned int PCF8523::getOffset(void) {
  return getRegister(PCF8523_OFFSET);
}

String PCF8523::getManufacturer(void) {
  return F("NXP");
}

String PCF8523::getModel(void) {
  return F("PCF8523");
}
