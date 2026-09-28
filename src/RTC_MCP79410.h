/* This is the class for MCP79410 that can be used as part of the RTC_I2C library */

#ifndef _RTC_MCP79410_H_
#define _RTC_MCP79410_H_

#include <RTC_I2C.h>


#define MCP79410_ADDRESS 0x6F   // I2C address for MCP79410
#define MCP79410_CLOCKREG 0x00  // Clock register (that is where the seconds start)
#define MCP79410_ALARM 0x0A     // Alarm seconds register
#define MCP79410_STATUS 0x03    // Status register
#define MCP79410_CONTROL 0x07   // Control register
#define MCP79410_OFFSET 0x08    // Offset register
#define MCP79410_WDAYBASE 1     // wday range from 1 to 7,
#define MCP79410_WDAYFIRST true // wday comes after day of month in clock reg
#define MCP79410_BIT7 0
#define MCP79410_CAP (RTC_CAP_32KHZ | RTC_CAP_1HZ | RTC_CAP_ALARM | RTC_CAP_HOURLY_ALARM | RTC_CAP_OFFSET)


class MCP79410 : public RTC_I2C {
 public:
  MCP79410(void) {
    _i2caddr = MCP79410_ADDRESS;
    _clockreg = MCP79410_CLOCKREG;
    _wdaybase = MCP79410_WDAYBASE;
    _wdayfirst = MCP79410_WDAYFIRST;
    _capabilities = MCP79410_CAP;
    _bit7set = MCP79410_BIT7;
  };
  void init(BatteryMode mode = BatteryMode::LSM) override;
  bool isValid(void) override;
  void setTime(timestamp_t t) override;
  void setTime(tm timeParts) override;
  void setAlarm(byte minute, byte hour) override; // here we can only set the alarm til next match
  void setAlarm(byte minute) override;
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
