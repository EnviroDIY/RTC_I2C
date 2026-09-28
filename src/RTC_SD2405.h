/* This is the class for SD2405 that can be used as part of the RTC_I2C library */

#ifndef _RTC_SD2405_H_
#define _RTC_SD2405_H_

#include <RTC_I2C.h>


#define SD2405_ADDRESS 0x32  // I2C address for SD2405
#define SD2405_CLOCKREG 0x00 // Clock register (that is where the seconds start)
#define SD2405_ALARM 0x07    // Alarm seconds register
#define SD2405_CONTROL 0x0F  // Control register
#define SD2405_OFFSET 0x12   // Offset register

#define SD2405_WDAYBASE 0     // wday range from 0 to 6
#define SD2405_WDAYFIRST true // wday comes before day of month in clock reg
#define SD2405_BIT7 (1 << 2)  // The 24H flag!
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
  };
  void init(BatteryMode mode = BatteryMode::LSM) override;
  bool isValid(void) override;
  void setAlarm(byte minute, byte hour) override; // here we can only set the alarm til next match
  void setAlarm(byte minute) override;            // hourly alarm at a particular minute
  bool senseAlarm(void) override;
  void clearAlarm(void) override;
  void enableAlarm(void) override;
  void disableAlarm(void) override;
  void enable32kHz(void) override;
  void disable32kHz(void) override;
  void enable1Hz(void) override;
  void disable1Hz(void) override;
  void setOffset(int offset, byte mode = 1) override;
  unsigned int getOffset(void) override;
  String getManufacturer(void) override;
  String getModel(void) override;
};
#endif
