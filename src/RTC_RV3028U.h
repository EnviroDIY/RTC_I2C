/* This is the class for RV3028U (using only the Unix clock) that can be used as part of the RTC_I2C library */

#ifndef _RTC_RV3028U_H_
#define _RTC_RV3028U_H_

#include <RTC_RV3028.h>

/// Unix Time 0 register; the RV-3028-U Unix time counter begins at address 0x1B.
#define RV3028_UCLOCK 0x1B

/// Capabilities supported by the RV-3028-U implementation.
#define RV3028U_CAP                                                                                                    \
  (static_cast<byte>(RTCCapability::RTC_CAP_32KHZ) | static_cast<byte>(RTCCapability::RTC_CAP_1HZ) |                   \
   static_cast<byte>(RTCCapability::RTC_CAP_ALARM) | static_cast<byte>(RTCCapability::RTC_CAP_BATTERY) |               \
   static_cast<byte>(RTCCapability::RTC_CAP_OFFSET))


/// Class for the [Micro Crystal
/// RV-3028](https://www.microcrystal.com/fileadmin/Media/Products/RTC/Datasheet/RV-3028-C7.pdf), using the Unix clock.
class RV3028U : public RV3028 {
 public:
  /// Initializes the instance for the RV3028U hardware variant.
  RV3028U() {
    _i2caddr = RV3028_ADDRESS;
    _clockreg = RV3028_UCLOCK;
    _wdaybase = 0;
    _wdayfirst = 0;
    _capabilities = RV3028U_CAP;
    _supportedAlarmMatches = RV3028_ALARMS;
    _bit7set = RV3028_BIT7;
  }
  using RTC_I2C::setTime;
  /**
   * @brief Set both the Unix counter and the calendar counters used by the alarm.
   * @param t The Unix timestamp to write to the RTC.
   * @return `true` when the required I2C communication succeeds; otherwise `false`.
   */
  bool setTime(timestamp_t t) override;
  /**
   * @brief Set both the Unix counter and the calendar counters used by the alarm.
   * @param timeParts The calendar time to write to the RTC.
   * @return `true` when the required I2C communication succeeds; otherwise `false`.
   */
  bool setTime(tm timeParts) override;
  timestamp_t getTime() override;
  bool getTime(tm &timeParts) override;
  String getModel() override;

 private:
  bool readUnixTime(timestamp_t &timestamp);
};
#endif
