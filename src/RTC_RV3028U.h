/* This is the class for RV3028U (using only the Unix clock) that can be used as part of the RTC_I2C library */

#ifndef _RTC_RV3028U_H_
#define _RTC_RV3028U_H_

#include <RTC_RV3028.h>

#define RV3028_UCLOCK 0x1B

#define RV3028U_CAP (RTC_CAP_32KHZ | RTC_CAP_1HZ | RTC_CAP_OFFSET)


class RV3028U : public RV3028 {
 public:
  RV3028U(void) {
    _i2caddr = RV3028_ADDRESS;
    _clockreg = RV3028_UCLOCK;
    _wdaybase = 0;
    _wdayfirst = 0;
    _capabilities = RV3028U_CAP;
    _bit7set = RV3028_BIT7;
  };
  void setAlarm(__attribute__((unused)) byte minute, __attribute__((unused)) byte hour) override {}
  bool senseAlarm(void) override { return false; }; // no-op for this RTC
  void clearAlarm(void) override {}                 // no-op for this RTC
  void enableAlarm(void) override {}                // no-op for this RTC
  void disableAlarm(void) override {}               // no-op for this RTC
  void setTime(timestamp_t t) override;
  void setTime(tm timeParts) override;
  timestamp_t getTime(bool blocking) override;
  void getTime(tm &timeParts, bool blocking) override;
  String getManufacturer(void) override;
  String getModel(void) override;
};
#endif
