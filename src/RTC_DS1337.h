/* This is the class for DS1337 that can be used as part of the RTC_I2C library */

#ifndef _RTC_DS1337_H_
#define _RTC_DS1337_H_

#include <RTC_I2C.h>


/// 7-bit I2C slave address for the DS1337.
#define DS1337_ADDRESS 0x68
/// Start of the time-of-day/date registers; the first clock register is at address 0x00.
#define DS1337_CLOCKREG 0x00
/// Start of Alarm 1 registers; referred to as the Alarm 1 time-of-day/date alarm registers in documentation (ADDRESSES
/// 0x07–0x0A).
#define DS1337_ALARM1 0x07
/// Control register; referred to as Control Register in documentation (ADDRESS 0x0E).
#define DS1337_CONTROL 0x0E
/// Status register; referred to as Status Register in documentation (ADDRESS 0x0F).
#define DS1337_STATUS 0x0F
/// No offset/calibration register is provided by the DS1337.
#define DS1337_OFFSET 0xFF
/// Weekday numbering used by the RTC: 1 through 7.
#define DS1337_WDAYBASE 1
/// The weekday register comes before the day-of-month register in the clock register sequence.
#define DS1337_WDAYFIRST true
/// No clock-register bit 7 must be forced when writing time.
#define DS1337_BIT7 0
/// Capabilities supported by the DS1337 implementation.
#define DS1337_CAP (RTC_CAP_32KHZ | RTC_CAP_1HZ | RTC_CAP_ALARM | RTC_CAP_HOURLY_ALARM)

/// The class for the [Analog Devices
/// DS1337](https://www.analog.com/media/en/technical-documentation/data-sheets/ds1337-ds1337c.pdf)
class DS1337 : public DSAlarm {
 public:
  DS1337(void) {
    _i2caddr = DS1337_ADDRESS;
    _clockreg = DS1337_CLOCKREG;
    _wdaybase = DS1337_WDAYBASE;
    _wdayfirst = DS1337_WDAYFIRST;
    _capabilities = DS1337_CAP;
    _bit7set = DS1337_BIT7;
  }
  bool init(BatteryMode mode = BatteryMode::LEVEL_SWITCHING) override;
  bool isValid(void) override;
  bool enable32kHz(void) override;
  bool disable32kHz(void) override;
  bool enable1Hz(void) override;
  bool disable1Hz(void) override;
  String getManufacturer(void) override;
  String getModel(void) override;
};
#endif
