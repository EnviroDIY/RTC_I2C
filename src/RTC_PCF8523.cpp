#include <RTC_PCF8523.h>

bool PCF8523::init(bool reset_configuration, bool clear_flags) {
  bool success = true;
  if (reset_configuration) {
    success &= setRegister(PCF8523_CONTROL, 0b01011000); // initiate software reset with the documented 0x58 command
    success &= setRegister(PCF8523_CONTROL, 0);          // enable the clock and disable correction interrupts
    success &= setRegister(PCF8523_CONTROL + 1, 0);      // disable watchdog and countdown timers
    success &= setRegister(PCF8523_CLKOUT, 0b00111000);  // disable clock output
  }
  if (clear_flags) success &= clearErrorFlags();
  return success;
}

// if Vbat disabled, connect to Vcc
bool PCF8523::setBatteryMode(BatteryMode mode) {
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
  default:
    return false;
  }
  byte control3;
  if (!readRegister(PCF8523_CONTROL + 2, control3)) return false;
  byte requestedControl3 = (control3 & 0b00011111) | bsm_reg;
  if (requestedControl3 == control3) return true;
  return setRegister(PCF8523_CONTROL + 2, requestedControl3); // update battery mode without clearing status flags
}

bool PCF8523::isValid() {
  byte control1;
  byte control3;
  byte seconds;
  bool success = true;
  success &= readRegister(PCF8523_CONTROL + 2, control3) && ((control3 & 0b00000100) == 0); // battery OK
  success &= readRegister(PCF8523_CONTROL, control1) && ((control1 & 0b00100000) == 0);     // oscillator running
  success &= readRegister(PCF8523_CLOCKREG, seconds) && ((seconds & 0b10000000) == 0);      // OS flag cleared
  return success;
}

bool PCF8523::setCountdownAlarm(uint16_t timerValue, CountdownTimerFrequency timerFrequency) {
  if (timerValue == 0 || timerValue > 0xFF) return false;

  byte frequencyBits;
  switch (timerFrequency) {
  case CountdownTimerFrequency::HZ_4096:
    frequencyBits = 0;
    break;
  case CountdownTimerFrequency::HZ_64:
    frequencyBits = 1;
    break;
  case CountdownTimerFrequency::HZ_1:
    frequencyBits = 2;
    break;
  case CountdownTimerFrequency::PER_MINUTE:
    frequencyBits = 3;
    break;
  case CountdownTimerFrequency::PER_HOUR:
    frequencyBits = 7;
    break;
  default:
    return false;
  }

  byte timerControl;
  byte interruptControl;
  byte frequencyControl;
  if (!readRegister(PCF8523_CLKOUT, timerControl) || !readRegister(PCF8523_CONTROL + 1, interruptControl) ||
      !readRegister(PCF8523_TIMER_FREQUENCY, frequencyControl))
    return false;

  bool success = true;
  success &= setRegister(PCF8523_CLKOUT, (timerControl & 0b10111110) | 0b01000000); // stop timer B; use pulse mode
  success &= setRegister(PCF8523_CONTROL + 1,
                         (interruptControl & 0b00000110) | 0b11011000); // disable CTBIE and clear CTBF only
  success &= setRegister(PCF8523_TIMER_FREQUENCY, (frequencyControl & 0b11111000) | frequencyBits);
  success &= setRegister(PCF8523_TIMER_VALUE, static_cast<byte>(timerValue));
  return success;
}

bool PCF8523::enableCountdownAlarm() {
  byte timerControl;
  byte interruptControl;
  if (!readRegister(PCF8523_CLKOUT, timerControl) || !readRegister(PCF8523_CONTROL + 1, interruptControl)) return false;
  bool success = setRegister(PCF8523_CONTROL + 1,
                             (interruptControl & 0b00000110) | 0b11011001); // clear CTBF and enable CTBIE
  success &= setRegister(PCF8523_CLKOUT, timerControl | 0b01000001);        // pulse mode; start timer B
  return success;
}

bool PCF8523::disableCountdownAlarm() {
  byte timerControl;
  byte interruptControl;
  if (!readRegister(PCF8523_CLKOUT, timerControl) || !readRegister(PCF8523_CONTROL + 1, interruptControl)) return false;
  bool success = setRegister(PCF8523_CLKOUT, timerControl & 0b11111110); // stop timer B
  success &= setRegister(PCF8523_CONTROL + 1,
                         (interruptControl & 0b00000110) | 0b11111000); // preserve flags; disable CTBIE
  return success;
}

bool PCF8523::senseCountdownAlarm() {
  byte control;
  return readRegister(PCF8523_CONTROL + 1, control) && ((control & 0b00100000) != 0);
}

bool PCF8523::clearCountdownAlarm() {
  byte control;
  if (!readRegister(PCF8523_CONTROL + 1, control)) return false;
  return setRegister(PCF8523_CONTROL + 1, (control & 0b00000111) | 0b11011000); // clear CTBF only
}

bool PCF8523::enableAlarm() {
  byte ctr;
  if (!readRegister(PCF8523_CONTROL, ctr)) return false;
  return setRegister(PCF8523_CONTROL, (ctr & 0b11111101) | 0b00000010);
}

bool PCF8523::disableAlarm() {
  byte ctr;
  if (!readRegister(PCF8523_CONTROL, ctr)) return false;
  return setRegister(PCF8523_CONTROL, (ctr & 0b11111101) | 0b00000000);
}


bool PCF8523::enable32kHz() {
  byte clkout;
  if (!readRegister(PCF8523_CLKOUT, clkout)) return false;
  return setRegister(PCF8523_CLKOUT, (clkout & 0b11000111) | 0b00000000);
}

bool PCF8523::disable32kHz() {
  byte clkout;
  if (!readRegister(PCF8523_CLKOUT, clkout)) return false;
  return setRegister(PCF8523_CLKOUT, (clkout & 0b11000111) | 0b00111000);
}

bool PCF8523::enable1Hz() {
  byte clkout;
  if (!readRegister(PCF8523_CLKOUT, clkout)) return false;
  return setRegister(PCF8523_CLKOUT, (clkout & 0b11000111) | 0b00110000);
}

bool PCF8523::disable1Hz() {
  return disable32kHz();
}

bool PCF8523::setOffset(int offset, OffsetMode mode) {
  bool success = true;
  if (mode == OffsetMode::RAW_OFFSET)
    // put the raw value into the register
    success &= setRegister(PCF8523_OFFSET, (offset & 0xFF));
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
    success &=
      setRegister(PCF8523_OFFSET, ((offset & 0x7F) | (static_cast<byte>(mode) << 7))); // Serial.println(offset);
    // Serial.println(((offset&0x7F)|(mode<<7)));
  }
  return success;
}

unsigned int PCF8523::getOffset() {
  return getRegister(PCF8523_OFFSET);
}

String PCF8523::getManufacturer() {
  return F("NXP");
}

String PCF8523::getModel() {
  return F("PCF8523");
}

// cSpell:ignore  VBat
