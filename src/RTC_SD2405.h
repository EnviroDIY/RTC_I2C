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
#define SD2405_CAP                                                                                                     \
  (static_cast<byte>(RTCCapability::RTC_CAP_32KHZ) | static_cast<byte>(RTCCapability::RTC_CAP_1HZ) |                   \
   static_cast<byte>(RTCCapability::RTC_CAP_ALARM) | static_cast<byte>(RTCCapability::RTC_CAP_OFFSET))
/// Alarms supported by the SD2405 implementation.
#define SD2405_ALARMS                                                                                                  \
  static_cast<byte>(AlarmMatch::MATCH_SECONDS) | static_cast<byte>(AlarmMatch::MATCH_MINUTES) |                        \
    static_cast<byte>(AlarmMatch::MATCH_HOURS) | static_cast<byte>(AlarmMatch::MATCH_DAY_OF_WEEK) |                    \
    static_cast<byte>(AlarmMatch::MATCH_DAY_OF_MONTH) | static_cast<byte>(AlarmMatch::MATCH_DAY_OF_YEAR) |             \
    static_cast<byte>(AlarmMatch::MATCH_EXACT)

/// The class for the [DFRobot
/// SD2405](https://image.dfrobot.com/image/data/TOY0021/SD2405AL%20datasheet%20(Angelo%20v0.1).pdf)
class SD2405 : public RTC_I2C {
 public:
  /// Initializes the instance for the SD2405 hardware.
  SD2405() {
    _i2caddr = SD2405_ADDRESS;
    _clockreg = SD2405_CLOCKREG;
    _wdaybase = SD2405_WDAYBASE;
    _wdayfirst = SD2405_WDAYFIRST;
    _capabilities = SD2405_CAP;
    _supportedAlarmMatches = SD2405_ALARMS;
    _bit7set = SD2405_BIT7;
  }
  bool init(bool reset_configuration = true, bool clear_flags = true) override;
  bool isValid() override;
  bool clearErrorFlags() override;
  bool setAlarm(AlarmMatch match, const AlarmTime &alarm) override;
  bool senseAlarm() override;
  bool clearAlarm() override;
  bool enableAlarm() override;
  bool disableAlarm() override;
  bool enable32kHz() override;
  bool disable32kHz() override;
  bool enable1Hz() override;
  bool disable1Hz() override;
  /**
   * @copydoc RTC_I2C::setOffset()
   *
   * This RTC has only one calibrated correction mode. Both `OffsetMode::FINE_OFFSET` and `OffsetMode::COARSE_OFFSET`
   * are treated identically.
   *
   * Negative values make the clock faster by roughly 3.051 ppm/LSB.
   * The documented adjustment range goes from -62 to +63.
   */
  bool setOffset(int offset, OffsetMode mode = OffsetMode::FINE_OFFSET) override;
  unsigned int getOffset() override;
  String getManufacturer() override;
  String getModel() override;
};
#endif
