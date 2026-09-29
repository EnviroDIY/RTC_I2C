/* This is the class for SD2405 that can be used as part of the RTC_I2C library */

#ifndef _RTC_SD2405_H_
#define _RTC_SD2405_H_

#include <RTC_I2C.h>


/// 7-bit I2C slave address for the SD2405.
#define SD2405_ADDRESS 0x32
/// Second register; referred to as Second in the Real time clock register table (ADDRESS 0x00).
#define SD2405_CLOCKREG 0x00
/// Second alarm register; referred to as Second alarm in the Time alarm register (ADDRESS 0x07).
#define SD2405_ALARM 0x07
/// Control register 1 (CTR1); referred to as CTR1 in the Control register table (ADDRESS 0x0F).
#define SD2405_CONTROL 0x0F
/// Time trimming register; referred to as Time Trimming Register in documentation (ADDRESS 0x12).
#define SD2405_OFFSET 0x12

/// Weekday numbering used by the RTC: 0 through 6.
#define SD2405_WDAYBASE 0
/// The weekday register comes before the day-of-month register in the clock register sequence.
#define SD2405_WDAYFIRST true
/// The 24-hour-mode flag is required in bit 2 of the hour register.
#define SD2405_BIT7 (1 << 2)
/// Capabilities supported by the SD2405 implementation.
#define SD2405_CAP (RTC_CAP_32KHZ | RTC_CAP_1HZ | RTC_CAP_ALARM | RTC_CAP_HOURLY_ALARM | RTC_CAP_OFFSET)


class SD2405 : public RTC_I2C {
 public:
  SD2405(void) {
    _i2caddr = SD2405_ADDRESS;
    _clockreg = SD2405_CLOCKREG;
    _wdaybase = SD2405_WDAYBASE;
    _wdayfirst = SD2405_WDAYFIRST;
    _capabilities = SD2405_CAP;
    _bit7set = SD2405_BIT7;
  }
  bool init(BatteryMode mode = BatteryMode::LEVEL_SWITCHING) override;
  bool isValid(void) override;
  bool setAlarm(byte minute, byte hour) override; // here we can only set the alarm til next match
  bool setAlarm(byte minute) override;            // hourly alarm at a particular minute
  bool senseAlarm(void) override;
  bool clearAlarm(void) override;
  bool enableAlarm(void) override;
  bool disableAlarm(void) override;
  bool enable32kHz(void) override;
  bool disable32kHz(void) override;
  bool enable1Hz(void) override;
  bool disable1Hz(void) override;
  /**
   * @copydoc RTC_I2C::setOffset()
   *
   * This RTC has only one calibrated correction mode. Both `OffsetMode::FINE_OFFSET` and `OffsetMode::COARSE_OFFSET`
   * are treated identically.
   *
   * Negative values make the clock faster by roughly 3.051 ppm/LSB.
   * The range of the internal parameter goes from -64 to +63.
   */
  bool setOffset(int offset, OffsetMode mode = OffsetMode::FINE_OFFSET) override;
  unsigned int getOffset(void) override;
  String getManufacturer(void) override;
  String getModel(void) override;
};
#endif
