/* This is the class for DS3231 that can be used as part of the RTC_I2C library */

#ifndef _RTC_DS3231_H_
#define _RTC_DS3231_H_

#include <RTC_I2C.h>


/// 7-bit I2C slave address for the DS3231.
#define DS3231_ADDRESS 0x68
/// Start of the timekeeping registers; the first clock register is at address 0x00.
#define DS3231_CLOCKREG 0x00
/// Start of Alarm 1 registers; Alarm 1 occupies registers 0x07–0x0A.
#define DS3231_ALARM1 0x07
/// Control register; referred to as Control Register in documentation (ADDRESS 0x0E).
#define DS3231_CONTROL 0x0E
/// Status register; referred to as Status Register in documentation (ADDRESS 0x0F).
#define DS3231_STATUS 0x0F
/// Aging offset register; referred to as Crystal Aging Offset Register in documentation (ADDRESS 0x10).
#define DS3231_OFFSET 0x10
/// Temperature register upper byte; referred to as Temperature Register (Upper Byte) in documentation (ADDRESS 0x11).
#define DS3231_TEMPMSB 0x11
/// Weekday numbering used by the RTC: 1 through 7.
#define DS3231_WDAYBASE 1
/// The weekday register comes before the day-of-month register in the clock register sequence.
#define DS3231_WDAYFIRST true
/// No clock-register bit 7 must be forced when writing time.
#define DS3231_BIT7 0
/// Capabilities supported by the DS3231 implementation.
#define DS3231_CAP (RTC_CAP_32KHZ | RTC_CAP_1HZ | RTC_CAP_ALARM | RTC_CAP_HOURLY_ALARM | RTC_CAP_OFFSET | RTC_CAP_TEMP)


class DS3231 : public DSAlarm {
 public:
  DS3231(void) {
    _i2caddr = DS3231_ADDRESS;
    _clockreg = DS3231_CLOCKREG;
    _wdaybase = DS3231_WDAYBASE;
    _wdayfirst = DS3231_WDAYFIRST;
    _capabilities = DS3231_CAP;
    _bit7set = DS3231_BIT7;
  };
  void init(BatteryMode mode = BatteryMode::LEVEL_SWITCHING) override;
  bool isValid(void) override;
  void enable32kHz(void) override;
  void disable32kHz(void) override;
  void enable1Hz(void) override;
  void disable1Hz(void) override;
  int getTemp(void) override;
  /**
   * @copydocs RTC_I2C::setOffset()
   * This RTC has only one calibrated correction mode - an aging offset that is added to or subtracted from the
   * capacitance for temperature correction. This is called the fine offset. If `OffsetMode::COARSE_OFFSET` is used, the
   * input is interpreted as a calibrated offset in 0.01 ppm steps.
   *
   * After having changed the offset value, a conversion is triggered so that changes are immediately visible.
   *
   * Negative values make the clock faster by roughly 0.1 ppm/LSB; positive values make it slower.
   */
  void setOffset(int offset, OffsetMode mode = OffsetMode::FINE_OFFSET) override;
  unsigned int getOffset(void) override;
  String getManufacturer(void) override;
  String getModel(void) override;
};
#endif

// cSpell:ignore TEMPMSB
