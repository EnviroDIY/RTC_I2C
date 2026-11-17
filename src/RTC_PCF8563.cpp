#include <RTC_PCF8563.h>


bool PCF8563::init(bool reset_configuration, bool clear_flags) {
  bool success = true;
  if (reset_configuration) {
    success &= setRegister(PCF8563_CONTROL, 0);
    success &= setRegister(PCF8563_CONTROL + 1, 0);
    success &= setRegister(PCF8563_CLKOUT, 0);
  }
  if (clear_flags) success &= clearErrorFlags();
  return success;
}

bool PCF8563::isValid() {
  byte control;
  byte seconds;
  bool success = true;
  success &= readRegister(PCF8563_CONTROL, control) && ((control & 0b00100000) == 0);
  //^ STOP not asserted
  success &= readRegister(PCF8563_CLOCKREG, seconds) && ((seconds & 0b10000000) == 0);
  //^ VL (low voltage) bit not asserted
  return success;
}

bool PCF8563::setCountdownAlarm(uint16_t timerValue, CountdownTimerFrequency timerFrequency) {
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
  default:
    return false;
  }

  byte control;
  if (!readRegister(PCF8563_CONTROL + 1, control)) return false;
  bool success = setRegister(PCF8563_TIMER_CONTROL, frequencyBits); // stop the timer and select its source clock
  success &= setRegister(PCF8563_CONTROL + 1,
                         (control & 0b00000010) | 0b00011000); // pulse mode; disable TIE and clear TF only
  success &= setRegister(PCF8563_TIMER_VALUE, static_cast<byte>(timerValue));
  return success;
}

bool PCF8563::enableCountdownAlarm() {
  byte timerControl;
  byte interruptControl;
  if (!readRegister(PCF8563_TIMER_CONTROL, timerControl) || !readRegister(PCF8563_CONTROL + 1, interruptControl))
    return false;
  bool success = setRegister(PCF8563_CONTROL + 1,
                             (interruptControl & 0b00000010) | 0b00011001);                // clear TF and enable TIE
  success &= setRegister(PCF8563_TIMER_CONTROL, (timerControl & 0b00000011) | 0b10000000); // start timer
  return success;
}

bool PCF8563::disableCountdownAlarm() {
  byte timerControl;
  byte interruptControl;
  if (!readRegister(PCF8563_TIMER_CONTROL, timerControl) || !readRegister(PCF8563_CONTROL + 1, interruptControl))
    return false;
  bool success = setRegister(PCF8563_TIMER_CONTROL, timerControl & 0b00000011); // stop timer
  success &= setRegister(PCF8563_CONTROL + 1,
                         (interruptControl & 0b00000010) | 0b00011100); // preserve flags; disable TIE
  return success;
}

bool PCF8563::senseCountdownAlarm() {
  byte control;
  return readRegister(PCF8563_CONTROL + 1, control) && ((control & 0b00000100) != 0);
}

bool PCF8563::clearCountdownAlarm() {
  byte control;
  if (!readRegister(PCF8563_CONTROL + 1, control)) return false;
  return setRegister(PCF8563_CONTROL + 1, (control & 0b00010011) | 0b00001000); // clear TF only
}

bool PCF8563::enableAlarm() {
  byte ctr;
  if (!readRegister(PCF8563_CONTROL + 1, ctr)) return false;
  return setRegister(PCF8563_CONTROL + 1, (ctr & 0b11110001) | 0b00001110);
}

bool PCF8563::disableAlarm() {
  byte ctr;
  if (!readRegister(PCF8563_CONTROL + 1, ctr)) return false;
  return setRegister(PCF8563_CONTROL + 1, (ctr & 0b11110001) | 0b00001100);
}


bool PCF8563::enable32kHz() {
  return setRegister(PCF8563_CLKOUT, 0b10000000);
}

bool PCF8563::disable32kHz() {
  return setRegister(PCF8563_CLKOUT, 0);
}

bool PCF8563::enable1Hz() {
  return setRegister(PCF8563_CLKOUT, 0b10000011);
}

bool PCF8563::disable1Hz() {
  return disable32kHz();
}

String PCF8563::getManufacturer() {
  return F("NXP");
}

String PCF8563::getModel() {
  return F("PCF8563");
}
