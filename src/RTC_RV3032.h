/* This is the class for RV3032 that can be used as part of the RTC_I2C library */

#ifndef _RTC_RV3032_H_
#define _RTC_RV3032_H_

#include <RTC_I2C.h>


#define RV3032_ADDRESS 0x51  // I2C address for RV3032
#define RV3032_CLOCKREG 0x01 // Clock register (that is where the seconds start)
#define RV3032_ALARM 0x08    // Alarm minutes register
#define RV3032_STATUS 0x0D   // Status register
#define RV3032_CONTROL 0x10  // Control register
#define RV3032_TEMP 0x0F     // Temperature register
#define RV3032_BUSY 0x0E     // contains EE busy bit
#define RV3032_COE 0xC0      // Clockout enable & BSM register
#define RV3032_CLKOUT 0xC3   // Clockout register
#define RV3032_OFFSET 0xC1   // Offset register
#define RV3032_EECMD 0x3F    // EEPROM command
#define RV3032_EEDATA 0x3E   // value for  EEPROM data transfer
#define RV3032_EEADDR 0x3D   // address for EEPROM data transfer
#define RV3032_BIT7 0
#define RV3032_WDAYBASE 0     // wday range from 0 to 6,
#define RV3032_WDAYFIRST true // wday comes after day of month in clock reg
#define RV3032_CAP (RTC_CAP_32KHZ | RTC_CAP_1HZ | RTC_CAP_ALARM | RTC_CAP_HOURLY_ALARM | RTC_CAP_OFFSET | RTC_CAP_TEMP)


class RV3032 : public RTC_I2C {
 public:
  RV3032(void) {
    _i2caddr = RV3032_ADDRESS;
    _clockreg = RV3032_CLOCKREG;
    _wdaybase = RV3032_WDAYBASE;
    _wdayfirst = RV3032_WDAYFIRST;
    _capabilities = RV3032_CAP;
    _bit7set = RV3032_BIT7;
  };
  void init(BatteryMode mode = BatteryMode::LSM) override;
  bool isValid(void) override;
  void setAlarm(byte minute, byte hour) override;
  void setAlarm(byte minute) override;
  bool senseAlarm(void) override;
  void clearAlarm(void) override;
  void enableAlarm(void) override;
  void disableAlarm(void) override;
  void enable32kHz(void) override;
  void disable32kHz(void) override;
  void enable1Hz(void) override;
  void disable1Hz(void) override;
  void setOffset(int offset, OffsetMode mode = OffsetMode::EVERY_MINUTE) override;
  unsigned int getOffset(void) override;
  String getManufacturer(void) override;
  String getModel(void) override;


 protected:
  void updateEEPROMByte(byte reg);
};
#endif

// cSpell:ignore EECMD EEDATA EEADDR
